/**************************************************************************

    This file is part of notepad, a simple text editor.
    Copyright (C) 1997-2004, Harald Radke, Hector Peraza.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

**************************************************************************/

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <time.h>

#include <X11/Xatom.h>
#include <X11/keysym.h>

#include <xclass/OXMsgBox.h>
#include <xclass/OXFileDialog.h>
#include <xclass/OXFontDialog.h>
#include <xclass/OXAboutDialog.h>
#include <xclass/version.h>

#include "main.h"
#include "OXDialogs.h"

char *filetypes[] = {
  "All files", "*",
  "Document files", "*.doc",
  "Text files", "*.txt",
  NULL, NULL
};

//----------------------------------------------------------------------

OXClient *clientX;

search_struct search;

int main(int argc, char **argv) {
  clientX = new OXClient(argc, argv);

  search.direction = True;
  search.caseSensitive = False;
  search.buffer = NULL;

  OXMain *mainWindow = new OXMain(clientX->GetRoot(), 10, 10);

  mainWindow->Resize(640, 400);
  mainWindow->MapWindow();
  if (argc > 1)
    mainWindow->LoadFile(argv[1]);

  clientX->Run();
}

OXMain::OXMain(const OXWindow *p, int w, int h) : OXMainFrame(p, w, h) {

  _helpWindow = 0;

  _menuBarLayout = new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 0, 0, 1, 1);
  _menuBarItemLayout = new OLayoutHints(LHINTS_TOP | LHINTS_LEFT, 0, 4, 0, 0);

  _menuFile = new OXPopupMenu(_client->GetRoot());
  _menuFile->AddEntry(new OHotString("&New"), M_FILE_NEW);
  _menuFile->AddSeparator();
  _menuFile->AddEntry(new OHotString("&Open..."), M_FILE_OPEN);
  _menuFile->AddEntry(new OHotString("&Save"), M_FILE_SAVE);
  _menuFile->AddEntry(new OHotString("Save &as..."), M_FILE_SAVEAS);
  _menuFile->AddSeparator();
  _menuFile->AddEntry(new OHotString("&Print..."), M_FILE_PRINT);
  _menuFile->AddSeparator();
  _menuFile->AddEntry(new OHotString("E&xit"), M_FILE_EXIT);

  _menuEdit = new OXPopupMenu(_client->GetRoot());
  _menuEdit->AddEntry(new OHotString("&Undo\tCtrl+Z"), M_EDIT_UNDO);
  _menuEdit->AddSeparator();
  _menuEdit->AddEntry(new OHotString("Cu&t\tCtrl+X"), M_EDIT_CUT);
  _menuEdit->AddEntry(new OHotString("&Copy\tCtrl+C"), M_EDIT_COPY);
  _menuEdit->AddEntry(new OHotString("&Paste\tCtrl+V"), M_EDIT_PASTE);
  _menuEdit->AddEntry(new OHotString("De&lete\tDel"), M_EDIT_DELETE);
  _menuEdit->AddSeparator();
  _menuEdit->AddEntry(new OHotString("Select &all"), M_EDIT_SELECTALL);
  _menuEdit->AddEntry(new OHotString("Time/&Date\tF5"), M_EDIT_TIMEDATE);
  _menuEdit->AddSeparator();
  _menuEdit->AddEntry(new OHotString("Set &Font..."), M_EDIT_SELFONT);
  _menuEdit->AddEntry(new OHotString("Set Ta&b Width..."), M_EDIT_SETTABS);

  _menuEdit->DisableEntry(M_EDIT_CUT);
  _menuEdit->DisableEntry(M_EDIT_COPY);
  _menuEdit->DisableEntry(M_EDIT_DELETE);
  _menuEdit->DisableEntry(M_EDIT_UNDO);
  _menuEdit->DisableEntry(M_EDIT_PASTE);

  _menuSearch = new OXPopupMenu(_client->GetRoot());
  _menuSearch->AddEntry(new OHotString("&Find..."), M_SEARCH_FIND);
  _menuSearch->AddEntry(new OHotString("Find &next\tF3"), M_SEARCH_FINDNEXT);
  _menuSearch->AddSeparator();
  _menuSearch->AddEntry(new OHotString("&Goto line..."), M_SEARCH_GOTO);

  _menuHelp = new OXPopupMenu(_client->GetRoot());
  _menuHelp->AddEntry(new OHotString("&Help\tF1"), M_HELP_CONTENTS);
  _menuHelp->AddSeparator();
  _menuHelp->AddEntry(new OHotString("&About..."), M_HELP_ABOUT);

  _menuFile->Associate(this);
  _menuEdit->Associate(this);
  _menuSearch->Associate(this);
  _menuHelp->Associate(this);

  _menuBar = new OXMenuBar(this, 1, 1, HORIZONTAL_FRAME);
  _menuBar->AddPopup(new OHotString("&File"), _menuFile, _menuBarItemLayout);
  _menuBar->AddPopup(new OHotString("&Edit"), _menuEdit, _menuBarItemLayout);
  _menuBar->AddPopup(new OHotString("&Search"), _menuSearch, _menuBarItemLayout);
  _menuBar->AddPopup(new OHotString("&Help"), _menuHelp, _menuBarItemLayout);

  AddFrame(_menuBar, _menuBarLayout);

  _te = new OXTextEdit(this, 10, 10, 1);
  _te->Associate(this);
  AddFrame(_te, new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y));

  _status = new OXStatusBar(this);
  AddFrame(_status, new OLayoutHints(LHINTS_BOTTOM | LHINTS_EXPAND_X, 0, 0, 3, 0));

  SetClassHints("Notepad", "Notepad");
  SetWindowName("Untitled - Notepad");

  filename = StrDup("Untitled");
  printer = StrDup("lp");
  printProg = StrDup("lpr");

  const OXFont *f = _te->GetFont();
  _fontName = new OString(f->NameOfFont());

  SetFocusOwner(_te);

  MapSubwindows();

  int keycode;

  keycode = XKeysymToKeycode(GetDisplay(), XK_F1);
  XGrabKey(GetDisplay(), keycode, AnyModifier, _id, True,
           GrabModeAsync, GrabModeAsync);

  keycode = XKeysymToKeycode(GetDisplay(), XK_F3);
  XGrabKey(GetDisplay(), keycode, AnyModifier, _id, True,
           GrabModeAsync, GrabModeAsync);

  keycode = XKeysymToKeycode(GetDisplay(), XK_F5);
  XGrabKey(GetDisplay(), keycode, AnyModifier, _id, True,
           GrabModeAsync, GrabModeAsync);

  timer = new OTimer(this, 100);

  _exiting = False;
}

OXMain::~OXMain() {
  delete _menuBarLayout;
  delete _menuBarItemLayout;
  delete _menuFile;
  delete _menuEdit;
  delete _menuSearch;
  delete _menuHelp;
}

void OXMain::LoadFile(char *fname) {
  OFileInfo fi;
  char *p, tmp[1024];

  fi.filename = NULL;

  if (fname == NULL) {
    fi.MimeTypesList = NULL;
    fi.file_types = filetypes;
    fi.file_types_index = 2;

    new OXFileDialog(_client->GetRoot(), this, FDLG_OPEN, &fi);
    fname = fi.filename;
  }

  if (fname) {
    if (!_te->LoadFromFile(fname)) {
      sprintf(tmp, "Error opening file \"%s\"", fname);
      new OXMsgBox(_client->GetRoot(), this, new OString("Notepad"),
		   new OString(tmp), MB_ICONSTOP, ID_OK);

      if (fi.filename) delete[] fi.filename;
    } else {
      if ((p = strrchr(fname, '/')) == NULL) {
	p = fname;
      } else {
	++p;
      }
      sprintf(tmp, "%s: %d lines read.", p, _te->GetNumberOfItems());
      _status->SetText(0, new OString(tmp));
      delete[] filename;
      filename = StrDup(p);
      sprintf(tmp, "%s - Notepad", p);
      SetWindowName(tmp);
    }
  }
  _te->Layout();
}

void OXMain::SaveFile(char *fname) {
  char *p, tmp[1024];

  if (!_te->SaveToFile(fname)) {
    sprintf(tmp, "Error saving file \"%s\"", fname);
    new OXMsgBox(_client->GetRoot(), this, new OString("Notepad"),
		 new OString(tmp), MB_ICONSTOP, ID_OK);
    return;
  }

  if ((p = strrchr(fname, '/')) == NULL) {
    p = fname;
  } else {
    ++p;
  }
  sprintf(tmp, "%s: %d lines written.", p, _te->GetNumberOfItems());
  _status->SetText(0, new OString(tmp));

  sprintf(tmp, "%s - Notepad", p);
  SetWindowName(tmp);
}

// shouldn't we create a backup file?

void OXMain::SaveFileAs() {
  OFileInfo fi;

  fi.MimeTypesList = NULL;
  fi.file_types = filetypes;
  fi.file_types_index = 0;
  new OXFileDialog(_client->GetRoot(), this, FDLG_SAVE, &fi);
  if (fi.filename) {
    SaveFile(fi.filename);
    delete[] filename;
    filename = StrDup(fi.filename);
  }
}


int OXMain::IsSaved() {
  int ret;
  char tmp[1024];

  sprintf(tmp, "The text has been modified. Do you want to save the changes?");

  if (!_te->TextChanged()) {
    return ID_NO;
  } else {
    new OXMsgBox(_client->GetRoot(), this, new OString("Notepad"),
		 new OString(tmp), MB_ICONEXCLAMATION,
		 ID_YES | ID_NO | ID_CANCEL, &ret);
    return ret;
  }
}

void OXMain::Print() {
  int  i, n, ret;
  char tmp[1024];
  OTextLine *ln;

  new OXPrintBox(clientX->GetRoot(), this, 400, 150,
                 &printer, &printProg, &ret);

  if (ret) {

    sprintf(tmp, "%s -P%s", printProg, printer);
    FILE *p = popen(tmp, "w");

    if (p == NULL) {
      sprintf(tmp, "Print command failed: %s", strerror(errno));
      new OXMsgBox(_client->GetRoot(), this, new OString("Print"),
                   new OString(tmp), MB_ICONSTOP, ID_OK, &ret);
      return;
    }

    n = _te->GetNumberOfItems() - 1;
    for (i = 0; i < n; ++i) {
      ln = (OTextLine *) _te->GetItem(i);
      fprintf(p, "%s\n", ln->GetString());
    }
    ln = (OTextLine *) _te->GetItem(n);
    if (ln->GetTextLength() > 0)
      fprintf(p, "%s\n", ln->GetString());

    pclose(p);

    sprintf(tmp, "Printed: %s", filename);
    _status->SetText(0, new OString(tmp));
  }
}

int OXMain::CloseWindow() {
  if (_exiting) {
    XBell(GetDisplay(), 0);
    return False;
  }
  _exiting = True;
  switch (IsSaved()) {
  case ID_CLOSE:
  case ID_CANCEL:
    break;
  case ID_YES:
    if (strcmp(filename, "Untitled") == 0)
      SaveFileAs();
    else
      SaveFile(filename);
    if (_te->TextChanged()) break;
  case ID_NO:
    if (_helpWindow) _helpWindow->CloseWindow();
    return OXMainFrame::CloseWindow();
  }
  _exiting = False;
  return False;
}

int OXMain::HandleKey(XKeyEvent *event) {
  if (event->type == KeyPress) {
    int keysym = XKeycodeToKeysym(GetDisplay(), event->keycode, 0);
    switch (keysym) {
      case XK_F1:
        ShowHelp();
        break;

      case XK_F3:
        Search(True);
        break;

      case XK_F5:
        TimeDate();
        break;

      default:
        return OXMainFrame::HandleKey(event);
    }
    return True;
  }
  return OXMainFrame::HandleKey(event);
}

void OXMain::Clear() {
  _te->Clear();
  delete[] filename;
  filename = StrDup("Untitled");
  SetWindowName("Untitled - Notepad");
  _status->SetText(0, new OString("New File"));
}

void OXMain::Search(int ret) {
  char tmp[1024];

  if ((ret == False) || (search.buffer == NULL))
    new OXSearchBox(clientX->GetRoot(), this, 400, 150, &search, &ret);

  if (ret) {
    if (!(_te->Search(search.buffer, search.direction, search.caseSensitive))) {
      sprintf(tmp, "Couldn't find \"%s\" ", search.buffer);
      new OXMsgBox(_client->GetRoot(), this, new OString("Notepad"),
		   new OString(tmp), MB_ICONEXCLAMATION, ID_OK);
    } else {
      _menuEdit->EnableEntry(M_EDIT_CUT);
      _menuEdit->EnableEntry(M_EDIT_COPY);
      _menuEdit->EnableEntry(M_EDIT_DELETE);
    }
  }
}

void OXMain::Goto() {
  long ret;

  new OXGotoBox(clientX->GetRoot(), this, 400, 150, &ret);

  if (ret >= 0)
    _te->SetCursorPosition(OPosition(0, ret-1), True);
}

void OXMain::TimeDate() {
  time_t now = time(NULL);
  char *tstr = ctime(&now);
  if (tstr[strlen(tstr)-1] == '\n') tstr[strlen(tstr)-1] = '\0';
  _te->InsertText(tstr);
}

void OXMain::ShowHelp() {
  if (!_helpWindow) {
    _helpWindow = new OXHelpWindow(_client->GetRoot(), NULL, 600, 650,
                                   "index.html", NULL, "notepad");
    _helpWindow->Associate(this);
  }
  _helpWindow->MapRaised();
}

void OXMain::About() {
  OAboutInfo info;

  info.wname = "About fOX Notepad";
  info.title = "fOX Notepad version " NOTEPAD_VERSION "\n"
               "A simple text editor.\n"
               "Compiled with xclass version "XCLASS_VERSION;
  info.copyright = "Copyright © 1997-2004 by H. Radke and H. Peraza";
  info.text = "This program is free software; you can redistribute it "
              "and/or modify it under the terms of the GNU "
              "General Public License.\n\n"
              "http://xclass.sourceforge.net";

  new OXAboutDialog(_client->GetRoot(), this, &info);
}

int OXMain::HandleTimer(OTimer *t) {
  if (t != timer) return True;
  if (XGetSelectionOwner(GetDisplay(), XA_PRIMARY) == None)
    _menuEdit->DisableEntry(M_EDIT_PASTE);
  else
    _menuEdit->EnableEntry(M_EDIT_PASTE);
  delete timer;
  timer = new OTimer(this, 100);
  return True;
}

int OXMain::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;
  OItemViewMessage *vmsg = (OItemViewMessage *) msg;
  int tabw;
  char tmp[1024];

  switch (msg->type) {
  case MSG_TEXTEDIT:
    switch (msg->action) {
    case MSG_CURSORPOS:
      sprintf(tmp, "Line: %d Column: %d", vmsg->pos.y + 1, vmsg->pos.x + 1);
      _status->SetText(0, new OString(tmp));
      break;

    case MSG_SELCHANGED:
      if (_te->HasSelection()) {
	_menuEdit->EnableEntry(M_EDIT_CUT);
	_menuEdit->EnableEntry(M_EDIT_COPY);
	_menuEdit->EnableEntry(M_EDIT_DELETE);
      } else {
	_menuEdit->DisableEntry(M_EDIT_CUT);
	_menuEdit->DisableEntry(M_EDIT_COPY);
	_menuEdit->DisableEntry(M_EDIT_DELETE);
      }
      break;

    case MSG_CLICK:
      if (vmsg->button == Button3)
        _menuEdit->PlaceMenu(vmsg->pos.x, vmsg->pos.y, True, True);
      break;
    }
    break;

  case MSG_MENU:
  case MSG_BUTTON:
    switch (msg->action) {
    case MSG_CLICK:
      switch (wmsg->id) {

      //------------------------------- File

      case M_FILE_NEW:
	switch (IsSaved()) {
        case ID_CLOSE:
	case ID_CANCEL:
	  break;
	case ID_YES:
	  if (strcmp(filename, "Untitled") == 0)
	    SaveFileAs();
	  else
	    SaveFile(filename);
          if (_te->TextChanged()) break;
	case ID_NO:
	  Clear();
	}
	break;

      case M_FILE_OPEN:
	switch (IsSaved()) {
        case ID_CLOSE:
	case ID_CANCEL:
	  break;
	case ID_YES:
	  if (strcmp(filename, "Untitled") == 0)
	    SaveFileAs();
	  else
	    SaveFile(filename);
          if (_te->TextChanged()) break;
	case ID_NO:
	  LoadFile();
	  break;
	}
	break;

      case M_FILE_SAVE:
	if (strcmp(filename, "Untitled") == 0)
	  SaveFileAs();
	else
	  SaveFile(filename);
	break;

      case M_FILE_SAVEAS:
	SaveFileAs();
	break;

      case M_FILE_PRINT:
	Print();
	break;

      case M_FILE_EXIT:
        CloseWindow();
	break;

      //------------------------------- Edit

      case M_EDIT_CUT:
	if (_te->Cut())
	  _menuEdit->EnableEntry(M_EDIT_PASTE);
	break;

      case M_EDIT_COPY:
	if (_te->Copy())
	  _menuEdit->EnableEntry(M_EDIT_PASTE);
	break;

      case M_EDIT_PASTE:
	_te->Paste();
	break;

      case M_EDIT_DELETE:
	_te->DeleteSelection();
	break;

      case M_EDIT_SELECTALL:
	_te->SelectAll();
        if (_te->HasSelection()) {
	  _menuEdit->EnableEntry(M_EDIT_CUT);
	  _menuEdit->EnableEntry(M_EDIT_COPY);
	  _menuEdit->EnableEntry(M_EDIT_DELETE);
	}
	break;

      case M_EDIT_TIMEDATE:
        TimeDate();
        break;

      case M_EDIT_SELFONT:
        new OXFontDialog(_client->GetRoot(), this, _fontName);
        _te->SetFont(_client->GetFont(_fontName->GetString()));
        break;

      case M_EDIT_SETTABS:
        tabw = _te->GetTabWidth();
        new OXSetTabsBox(clientX->GetRoot(), this, 400, 150, &tabw);
        if (tabw >= 0) {
          if (tabw < 1) tabw = 1; else if (tabw > 100) tabw = 100;
          _te->SetTabWidth(tabw);
        }
        break;

      //------------------------------- Search

      case M_SEARCH_FIND:
	Search(False);
	break;

      case M_SEARCH_FINDNEXT:
	Search(True);
	break;

      case M_SEARCH_GOTO:
	Goto();
        break;

      //------------------------------- Help

      case M_HELP_CONTENTS:
        ShowHelp();
        break;

      case M_HELP_ABOUT:
        About();
        break;

      default:
	break;

      }
      break;

    default:
      break;

    }
    break;

  case MSG_HELP:
    if (msg->action == MSG_CLOSE) _helpWindow = 0;
    break;

  default:
    break;

  }

  return True;
}
