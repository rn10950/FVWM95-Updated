/**************************************************************************

    This file is part of NRX mail, a mail client using the XClass95 toolkit.
    Copyright (C) 1998 by Harald Radke.                 

    This application is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This application is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

**************************************************************************/

#include <stdio.h>
#include <unistd.h>

#include <xclass/OXFileDialog.h>

#include "OXViewSrc.h"
#include "OXPrintBox.h"


//----------------------------------------------------------------------

OXViewSrc::OXViewSrc(const OXWindow *p, const OXWindow *main, int w,
		     int h, char *printercommand, char *printer,
		     OText *text, unsigned long options) : 
  OXTransientFrame(p, main, w, h, options) {

  _printer = new char[strlen(printer) + 1];
  strcpy(_printer, printer);
  _printercommand = new char[strlen(printercommand) + 1];
  strcpy(_printercommand, printercommand);

  int i, ax, ay;
  Window wdummy;

  InitMenu();

  tv = new OXTextEdit(this, 560, 300, 1);
  tv->SetReadOnly(True);
  AddFrame(tv, new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y));
#if 0
  tv->SetText(text);
#else
  {
    int ln;
    tv->AutoUpdate(False);
    for (ln = 0; ln < text->RowCount(); ++ln) {
      const char *str = text->GetLine(OPosition(0, ln));
      if (str) {
        tv->InsertText(str);
        tv->InsertText("\n");
      }
    }
    tv->SetCursorPosition(OPosition(0, 0), True);
    tv->AutoUpdate(True);
  }
#endif

  SetFocusOwner(tv);

  SetMWMHints(MWM_DECOR_ALL | MWM_DECOR_MAXIMIZE | MWM_DECOR_MENU,
	      MWM_FUNC_ALL | MWM_FUNC_MAXIMIZE | MWM_FUNC_RESIZE,
	      MWM_INPUT_MODELESS);
  MapSubwindows();
  // position relative to the parent's window
  XTranslateCoordinates(GetDisplay(),
			main->GetId(), GetParent()->GetId(),
			(((OXFrame *) main)->GetWidth() - _w) >> 1,
			(((OXFrame *) main)->GetHeight() - _h) >> 1,
			&ax, &ay, &wdummy);
  Move(ax, ay);
  Layout();
  SetWindowName("View Mail Source");
  MapWindow();
}

OXViewSrc::~OXViewSrc() {
  delete[] _printer;
  delete[] _printercommand;
  delete _menuFile;
  delete _menuEdit;
  delete _menuHelp;
  delete _menuBarItemLayout;
  delete _menuBarLayout;
}

void OXViewSrc::InitMenu() {

  _menuBarLayout = new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 0, 0, 1, 1);
  _menuBarItemLayout = new OLayoutHints(LHINTS_TOP | LHINTS_LEFT, 0, 4, 0, 0);

  _menuFile = new OXPopupMenu(_client->GetRoot());
  _menuFile->AddEntry(new OHotString("&Save Mail Source..."), MVS_FILE_SAVE);
  _menuFile->AddEntry(new OHotString("&Print Mail Source..."), MVS_FILE_PRINT);
  _menuFile->AddSeparator();
  _menuFile->AddEntry(new OHotString("&Close"), MVS_FILE_EXIT);

  _menuEdit = new OXPopupMenu(_client->GetRoot());
  _menuEdit->AddEntry(new OHotString("&Copy"), MVS_EDIT_COPY);
  _menuEdit->DisableEntry(MVS_EDIT_COPY);
  _menuEdit->AddEntry(new OHotString("&Select all"), MVS_EDIT_SELECTALL);
//  _menuEdit->AddEntry(new OHotString("&Search..."), MVS_EDIT_SEARCH);
//  _menuEdit->AddEntry(new OHotString("&Search again"), MVS_EDIT_SEARCHAGAIN);

  _menuHelp = new OXPopupMenu(_client->GetRoot());
  _menuHelp->AddEntry(new OHotString("&Contents..."), MVS_HELP_CONTENTS);
  _menuHelp->AddEntry(new OHotString("&Search..."), MVS_HELP_SEARCH);
  _menuHelp->AddSeparator();
  _menuHelp->AddEntry(new OHotString("&About..."), MVS_HELP_ABOUT);

  _menuHelp->DisableEntry(MVS_HELP_CONTENTS);
  _menuHelp->DisableEntry(MVS_HELP_SEARCH);

  _menuFile->Associate(this);
  _menuEdit->Associate(this);
  _menuHelp->Associate(this);

  _menuBar = new OXMenuBar(this, 1, 1, HORIZONTAL_FRAME);
  _menuBar->AddPopup(new OHotString("&File"), _menuFile, _menuBarItemLayout);
  _menuBar->AddPopup(new OHotString("&Edit"), _menuEdit, _menuBarItemLayout);
  _menuBar->AddPopup(new OHotString("&Help"), _menuHelp, _menuBarItemLayout);

  AddFrame(_menuBar, _menuBarLayout);
}

void OXViewSrc::SaveSource() {
  char *file;

  static const char *filetypes[8] = {
    "All files", "*",
    "Document files", "*.doc",
    "Text files", "*.txt",
    NULL, NULL
  };

  OFileInfo fi;
  char *p, tmp[1024];

  fi.MimeTypesList = NULL;
  fi.file_types = (char **) filetypes;

  new OXFileDialog(_client->GetRoot(), this, FDLG_SAVE, &fi);

  if (fi.filename) {
    file = new char[strlen(fi.ini_dir) + strlen(fi.filename) + 2];
    sprintf(file, "%s/%s", fi.ini_dir, fi.filename);
    if (!tv->SaveToFile(file)) {
      sprintf(tmp, "Error saving mail to file \"%s\"", file);
      new OXMsgBox(_client->GetRoot(), this, new OString("NRX MAIL"),
		   new OString(tmp), MB_ICONSTOP, ID_OK);
      return;
    }
  }
}

void OXViewSrc::PrintSource() {
  int ret;
  long count;
  char tmp[1024];
  char home[1024];

  new OXPrintBox(_client->GetRoot(), this, 400, 150, &_printer,
		 &_printercommand, &ret);

  if (ret == True) {
    sprintf(home, "%s/%s", getenv("HOME"), "/.nrxmail-print");
    tv->SaveToFile(home);
    sprintf(tmp, "%s -P%s %s", _printercommand, _printer, home); //, filename);
    FILE *p = popen(tmp, "w");
    pclose(p);
    unlink(home);
  }
}

int OXViewSrc::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;
////  OViewMessage *vmsg = (OViewMessage *) msg;
  const char *string;
  char *tmp;

  switch (msg->type) {
/*
  case MSG_VIEW:
    switch (msg->action) {
    case VIEW_ISMARKED:
      if (vmsg->sel == True)
	_menuEdit->EnableEntry(MVS_EDIT_COPY);
      else
	_menuEdit->DisableEntry(MVS_EDIT_COPY);
      break;

    case VIEW_CLICK3:
      _menuEdit->PlaceMenu(vmsg->xroot, vmsg->yroot, True, True);
      break;
    }
    break;
*/

  case MSG_MENU:
    switch (msg->action) {
    case MSG_CLICK:
      switch (wmsg->id) {
      case MVS_FILE_SAVE:
	SaveSource();
	break;

      case MVS_FILE_PRINT:
	PrintSource();
	break;

      case MVS_FILE_EXIT:
	CloseWindow();
	break;

      case MVS_EDIT_COPY:
	tv->Copy();
	break;

      case MVS_EDIT_SELECTALL:
	tv->SelectAll();
	_menuEdit->EnableEntry(MVS_EDIT_COPY);
	break;

//         case MVS_EDIT_SEARCH:
//         break;
//         case MVS_EDIT_SEARCHAGAIN:
//         break;

      case MVS_HELP_CONTENTS:
	break;

      case MVS_HELP_ABOUT:
	break;
      }
      break;

    default:
      break;
    }
  }

  return True;
}
