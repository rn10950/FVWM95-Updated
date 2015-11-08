/**************************************************************************

    This is an example xclass application.
    Copyright (C) 2000, 2001, Hector Peraza.

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
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <sys/stat.h>

#include <xclass/OXMsgBox.h>
#include <xclass/OXFileDialog.h>
#include <xclass/OXAboutDialog.h>
#include <xclass/version.h>

#include "main.h"

#include "tb-open.xpm"
#include "tb-save.xpm"
#include "tb-print.xpm"

#include "menudef.h"
#include "toolbardef.h"

char *filetypes[] = { "All files",      "*",
                      "Text files",     "*.txt",
                      "Document files", "*.doc",
                      "HTML files",     "*.html",
                      NULL,             NULL };

//----------------------------------------------------------------------

int main(int argc, char **argv) {
  OXClient *clientX = new OXClient(argc, argv);

  OXMain *mainw = new OXMain(clientX->GetRoot(), 10, 10);
  mainw->Resize(500, 350);

  if (argc > 1) mainw->ReadFile(argv[1]);

  mainw->MapWindow();

  clientX->Run();

  exit(0);
}

OXMain::OXMain(const OXWindow *p, int w, int h) :
  OXMainFrame(p, w, h) {

  _menuBarLayout = new OLayoutHints(LHINTS_TOP | LHINTS_LEFT | LHINTS_EXPAND_X, 
                                    0, 0, 1, 1);
  _menuBarItemLayout = new OLayoutHints(LHINTS_TOP | LHINTS_LEFT,
                                    0, 4, 0, 0);

  _menuFile = _MakePopup(&file_popup);
  _menuEdit = _MakePopup(&edit_popup);
  _menuView = _MakePopup(&view_popup);
  _menuHelp = _MakePopup(&help_popup);

  _menuFile->Associate(this);
  _menuEdit->Associate(this);
  _menuView->Associate(this);
  _menuHelp->Associate(this);

  //------ menu bar

  _menuBar = new OXMenuBar(this, 1, 1, HORIZONTAL_FRAME);
  _menuBar->AddPopup(new OHotString("&File"), _menuFile, _menuBarItemLayout);
  _menuBar->AddPopup(new OHotString("&Edit"), _menuEdit, _menuBarItemLayout);
  _menuBar->AddPopup(new OHotString("&View"), _menuView, _menuBarItemLayout);
  _menuBar->AddPopup(new OHotString("&Help"), _menuHelp, _menuBarItemLayout);

  AddFrame(_menuBar, _menuBarLayout);

  //---- toolbar

  _toolBarSep = new OXHorizontal3dLine(this);

  _toolBar = new OXToolBar(this);
  _toolBar->AddButtons(tb_data);

  AddFrame(_toolBarSep, new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X));
  AddFrame(_toolBar, new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 
                                      0, 0, 3, 3));

  //------ Canvas frame

  _canvas = new OXCanvas(this, 10, 10, SUNKEN_FRAME | DOUBLE_BORDER);
  _container = new OXCompositeFrame(_canvas->GetViewPort(), 640, 480,
                                    OWN_BKGND, _whitePixel);
  _canvas->SetContainer(_container);

  AddFrame(_canvas, new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y));

  //------ status bar

  _statusBar = new OXStatusBar(this);
  AddFrame(_statusBar, new OLayoutHints(LHINTS_BOTTOM | LHINTS_EXPAND_X,
                                     0, 0, 3, 0));

  SetWindowTitle(NULL);
  SetClassHints("ExampleApp", "ExampleApp");

  MapSubwindows();
}

OXMain::~OXMain() {
  delete _menuBarLayout;
  delete _menuBarItemLayout;

  delete _menuFile;
  delete _menuEdit;
  delete _menuView;
  delete _menuHelp;
}

int OXMain::CloseWindow() {
  return OXMainFrame::CloseWindow();
}

OXPopupMenu *OXMain::_MakePopup(struct _popup *p) {

  OXPopupMenu *popup = new OXPopupMenu(_client->GetRoot());

  for (int i=0; p->popup[i].name != NULL; ++i) {
    if (strlen(p->popup[i].name) == 0) {
      popup->AddSeparator();
    } else {
      if (p->popup[i].popup_ref == NULL) {
        popup->AddEntry(new OHotString(p->popup[i].name), p->popup[i].id);
      } else {
        struct _popup *p1 = p->popup[i].popup_ref;
        popup->AddPopup(new OHotString(p->popup[i].name), p1->ptr);
      }
      if (p->popup[i].state & MENU_DISABLED) popup->DisableEntry(p->popup[i].id);
      if (p->popup[i].state & MENU_CHECKED) popup->CheckEntry(p->popup[i].id);
      if (p->popup[i].state & MENU_RCHECKED) popup->RCheckEntry(p->popup[i].id,
                                                                p->popup[i].id,
                                                                p->popup[i].id);
    }
  }
  p->ptr = popup;

  return popup;
}

int OXMain::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;

  switch (msg->type) {

    case MSG_MENU:
    case MSG_BUTTON:
      switch (msg->action) {

        case MSG_CLICK:
          switch (wmsg->id) {

            //--------------------------------------- File

            case M_FILE_OPEN:
              DoOpen();
              break;

            case M_FILE_SAVE:
              DoSave(NULL);
              break;

            case M_FILE_SAVEAS:
              DoSave(NULL);
              break;

            case M_FILE_PRINT:
              DoPrint();
              break;

            case M_FILE_EXIT:
              CloseWindow();
              break;

            //--------------------------------------- Edit

            case M_EDIT_CUT:
              break;

            case M_EDIT_COPY:
              break;

            case M_EDIT_PASTE:
              break;

            //--------------------------------------- View

            case M_VIEW_TOOLBAR:
              DoToggleToolBar();
              break;

            case M_VIEW_STATUSBAR:
              DoToggleStatusBar();
              break;

            //--------------------------------------- Help

            case M_HELP_CONTENTS:
              break;

            case M_HELP_SEARCH:
              break;

            case M_HELP_ABOUT:
              DoAbout();
              break;

            default:
              break;

          } // switch (wmsg->id)
          break;

        default:
          break;

      } // switch (msg->action)
      break;

    default:
      break;

  } // switch (msg->type)

  return True;
}

//----------------------------------------------------------------------

void OXMain::SetWindowTitle(char *title) {
  static char *pname = "Test application";

  if (title) {
    char *wname = new char[strlen(title) + strlen(pname) + 4];
    sprintf(wname, "%s - %s", pname, title);
    SetWindowName(wname);
    delete wname;
  } else {
    SetWindowName(pname);
  }
}

void OXMain::UpdateStatus() {
  char tmp[1024];

  strcpy(tmp, "Ready");
  _statusBar->SetText(0, new OString(tmp));
}

//----------------------------------------------------------------------

void OXMain::ReadFile(char *fname) {
  FILE *fp;

  if (!fname) return;

  if ((fp = fopen(fname, "r")) == NULL) {

    char tmp[PATH_MAX];
    sprintf(tmp, "Could not open input file \"%s\": %s.",
                 fname, strerror(errno));
    new OXMsgBox(_client->GetRoot(), this, new OString("File Open"),
                 new OString(tmp), MB_ICONSTOP, ID_OK);
    return;

  } else {

    // Add your code to process the file here

    fclose(fp);
  }
}

void OXMain::WriteFile(char *fname) {
  FILE *fp;

  if (!fname) return;

  if ((fp = fopen(fname, "w")) == NULL) {

    char tmp[PATH_MAX];
    sprintf(tmp, "Could not create output file \"%s\": %s.",
                 fname, strerror(errno));
    new OXMsgBox(_client->GetRoot(), this, new OString("File Open"),
                 new OString(tmp), MB_ICONSTOP, ID_OK);
    return;

  } else {

    // Add your code to save the file here

    fclose(fp);
  }
}

void OXMain::DoOpen() {
  OFileInfo fi;
  FILE *fp;

  fi.MimeTypesList = NULL;
  fi.file_types = filetypes;

  new OXFileDialog(_client->GetRoot(), this, FDLG_OPEN, &fi);
  if (fi.filename) {
    ReadFile(fi.filename);
  }
}

void OXMain::DoSave(char *fname) {
  int retc;
  OFileInfo fi;
  FILE *fp;

  fi.MimeTypesList = NULL;
  fi.file_types = filetypes;

  new OXFileDialog(_client->GetRoot(), this, FDLG_SAVE, &fi);
  if (fi.filename) {

    // check whether the file already exists, and ask
    // permission to overwrite if so.

    if (access(fi.filename, F_OK) == 0) {
      new OXMsgBox(_client->GetRoot(), this,
            new OString("Save"),
            new OString("A file with the same name already exists. Overwrite?"),
            MB_ICONQUESTION, ID_YES | ID_NO, &retc);
      if (retc == ID_NO) return;
    }

    WriteFile(fi.filename);
  }
}

void OXMain::DoPrint() {

  // Put your printing code here...

}

void OXMain::DoToggleToolBar() {
  if (_toolBar->IsVisible()) {
    HideFrame(_toolBar);
    HideFrame(_toolBarSep);
    _menuView->UnCheckEntry(M_VIEW_TOOLBAR);
  } else {
    ShowFrame(_toolBar);
    ShowFrame(_toolBarSep);
    _menuView->CheckEntry(M_VIEW_TOOLBAR);
  }
}

void OXMain::DoToggleStatusBar() {
  if (_statusBar->IsVisible()) {
    HideFrame(_statusBar);
    _menuView->UnCheckEntry(M_VIEW_STATUSBAR);
  } else {
    ShowFrame(_statusBar);
    _menuView->CheckEntry(M_VIEW_STATUSBAR);
  }
}

void OXMain::DoAbout() {
  OAboutInfo info;
  
  info.wname = "About Test Application";
  info.title = "Test Application\n"
               "Compiled with xclass version "XCLASS_VERSION;
  info.copyright = "Copyright © 1998-2000 by the fOX Project Team.";
  info.text = "This program is free software; you can redistribute it "
              "and/or modify it under the terms of the GNU "
              "General Public License.\n\n"
              "http://xclass.sourceforge.net";

  new OXAboutDialog(_client->GetRoot(), this, &info);
}
