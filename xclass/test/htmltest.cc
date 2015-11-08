/**************************************************************************

    This is a test program for the OXHtml widget.
    Copyright (C) 2000, Hector Peraza.

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

#include <xclass/utils.h>
#include <xclass/version.h>
#include <xclass/OXMsgBox.h>
#include <xclass/OXMainFrame.h>
#include <xclass/OXMenu.h>
#include <xclass/OXHtml.h>
#include <xclass/OHtmlUri.h>
#include <xclass/OXTextEdit.h>
#include <xclass/OXStatusBar.h>
#include <xclass/OXFileDialog.h>
#include <xclass/OXAboutDialog.h>


#define M_FILE_OPEN       101
#define M_FILE_SAVE       102
#define M_FILE_PRINT      103
#define M_FILE_EXIT       104

#define M_EDIT_CUT        201
#define M_EDIT_COPY       202
#define M_EDIT_PASTE      203

#define M_VIEW_STATUSBAR  301
#define M_VIEW_UNDERLINE  302
#define M_VIEW_SOURCE     303

#define M_HELP_CONTENTS   401
#define M_HELP_SEARCH     402
#define M_HELP_ABOUT      403


char *filetypes[] = { "HTML files",     "*.htm|*.html",
                      "All files",      "*",
                      NULL,             NULL };

//----------------------------------------------------------------------

class OXMain : public OXMainFrame {
public:
  OXMain(const OXWindow *p, char *fname);
  ~OXMain();

  virtual int ProcessMessage(OMessage *msg);

  void DoOpen();
  void DoViewSource();
  void DoToggleStatusBar();
  void DoToggleUnderlineLinks();
  void DoAbout();

protected:
  void LoadDoc(char *filename);

  OXHtml *_htmlview;
  OXMenuBar *_menuBar;
  OXPopupMenu *_menuFile, *_menuEdit, *_menuView, *_menuHelp;
  OXStatusBar *_statusBar;
  OLayoutHints *_menuBarLayout, *_menuBarItemLayout,
               *_viewLayout, *_statusLayout;

  char *_filename;
};


//----------------------------------------------------------------------

int main(int argc, char **argv) {

  OXClient *clientX = new OXClient;

  char *fname = NULL;
  if (argc > 1) fname = argv[1];

  OXMain *mainWindow = new OXMain(clientX->GetRoot(), fname);
  mainWindow->MapWindow();

  clientX->Run();
}


OXMain::OXMain(const OXWindow *p, char *fname) :
  OXMainFrame(p, 100, 100) {

  _filename = NULL;

  //---- menu bar

  _menuBarLayout = new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 0, 0, 1, 1);
  _menuBarItemLayout = new OLayoutHints(LHINTS_TOP | LHINTS_LEFT, 0, 4, 0, 0);

  _menuFile = new OXPopupMenu(_client->GetRoot());
  _menuFile->AddEntry(new OHotString("&Open..."), M_FILE_OPEN);
  _menuFile->AddSeparator();
  _menuFile->AddEntry(new OHotString("E&xit"), M_FILE_EXIT);
  _menuFile->Associate(this);

  _menuEdit = new OXPopupMenu(_client->GetRoot());
  _menuEdit->AddEntry(new OHotString("&Copy"), M_EDIT_COPY);
  _menuEdit->AddEntry(new OHotString("&Paste"), M_EDIT_PASTE);
  _menuEdit->Associate(this);

  _menuView = new OXPopupMenu(_client->GetRoot());
  _menuView->AddEntry(new OHotString("Status &Bar"), M_VIEW_STATUSBAR);
  _menuView->AddSeparator();
  _menuView->AddEntry(new OHotString("&Underline links"), M_VIEW_UNDERLINE);
  _menuView->AddSeparator();
  _menuView->AddEntry(new OHotString("Page &Source..."), M_VIEW_SOURCE);
  _menuView->Associate(this);
  _menuView->CheckEntry(M_VIEW_STATUSBAR);

  _menuHelp = new OXPopupMenu(_client->GetRoot());
  _menuHelp->AddEntry(new OHotString("&Contents"), M_HELP_CONTENTS);
  _menuHelp->AddEntry(new OHotString("&Search..."), M_HELP_SEARCH);
  _menuHelp->AddSeparator();
  _menuHelp->AddEntry(new OHotString("&About"), M_HELP_ABOUT);
  _menuHelp->Associate(this);

  _menuBar = new OXMenuBar(this, 1, 1, HORIZONTAL_FRAME);
  _menuBar->AddPopup(new OHotString("&File"), _menuFile, _menuBarItemLayout);
  _menuBar->AddPopup(new OHotString("&Edit"), _menuEdit, _menuBarItemLayout);
  _menuBar->AddPopup(new OHotString("&View"), _menuView, _menuBarItemLayout);
  _menuBar->AddPopup(new OHotString("&Help"), _menuHelp, _menuBarItemLayout);

  AddFrame(_menuBar, _menuBarLayout);

  //---- html widget

  _viewLayout = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y);

  _htmlview = new OXHtml(this, 10, 10, -1);
  _htmlview->Associate(this);
  AddFrame(_htmlview, _viewLayout);

  //---- status bar

  _statusLayout = new OLayoutHints(LHINTS_BOTTOM | LHINTS_EXPAND_X, 0, 0, 3, 0);

  _statusBar = new OXStatusBar(this);
  AddFrame(_statusBar, _statusLayout);

  SetWindowName("Untitled");

  if (fname) LoadDoc(fname);

  MapSubwindows();
  Resize(560, 400);
  Layout();
}

OXMain::~OXMain() {
  delete _menuFile;
  delete _menuView;
  delete _menuBarLayout;
  delete _menuBarItemLayout;
  delete _viewLayout;
  delete _statusLayout;
  if (_filename) delete _filename;
}

void OXMain::LoadDoc(char *filename) {
  FILE *f;
  char buf[1024];

  //printf("loading %s\n", filename);

  f = fopen(filename, "r");
  if (f) {

    if (_filename) delete[] _filename;
    _filename = StrDup(filename);

    _htmlview->Clear();

    OHtmlUri uri;
    uri.zScheme = StrDup("file");
    if (filename[0] == '/') {
      strcpy(buf, filename);
    } else {
      getcwd(buf, 1024);
      strcat(buf, "/");
      strcat(buf, filename);
    }
    uri.zPath = StrDup(buf);

    char *url = uri.BuildUri();
    _htmlview->SetBaseUri(url);
    delete[] url;

    while (1) {
      if (fgets(buf, 1024, f) == 0) break;
      _htmlview->ParseText(buf);
    }
    fclose(f);

    _htmlview->Layout();

    SetWindowName(filename);

  } else {

    sprintf(buf, "Could not open \"%s\"", filename);
    new OXMsgBox(_client->GetRoot(), this, new OString("Test"),
                 new OString(buf), MB_ICONSTOP, ID_OK);

  }
}

int OXMain::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;
  OHtmlMessage *hmsg = (OHtmlMessage *) msg;

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

            case M_FILE_EXIT:
              CloseWindow();
              break;

            //--------------------------------------- Edit

            case M_EDIT_COPY:
              break;

            case M_EDIT_PASTE:
              break;

            //--------------------------------------- View

            case M_VIEW_STATUSBAR:
              DoToggleStatusBar();
              break;

            case M_VIEW_UNDERLINE:
              DoToggleUnderlineLinks();
              break;

            case M_VIEW_SOURCE:
              DoViewSource();
              break;

            //--------------------------------------- Help

            case M_HELP_CONTENTS:
              break;

            case M_HELP_SEARCH:
              break;

            case M_HELP_ABOUT:
              DoAbout();
              break;

          }
          break;
      }
      break;

    case MSG_HTML:
      switch (msg->action) {
        case MSG_CLICK:
          if (hmsg->uri) {
            OHtmlUri URI(hmsg->uri);
            if (URI.zFragment && *URI.zFragment) {
              if (strcasecmp(URI.zScheme, "file") == 0) {
                OHtmlUri baseURI(_htmlview->GetBaseUri());
                if (strcmp(URI.zPath, baseURI.zPath) != 0)
                  LoadDoc(URI.zPath);
              }
              _htmlview->GotoAnchor(URI.zFragment);
            } else {
              if (strcasecmp(URI.zScheme, "file") == 0)
                LoadDoc(URI.zPath);
            }
          }
          break;

        case MSG_SELECT:
          if (hmsg->uri)
            _statusBar->SetText(0, new OString(hmsg->uri));
          else
            _statusBar->SetText(0, new OString(""));
          break;
      }
      break;

    default:
      break;

  }

  return True;
}

void OXMain::DoOpen() {
  OFileInfo fi;
  char buf[1024];

  fi.MimeTypesList = NULL;
  fi.file_types = filetypes;
  new OXFileDialog(_client->GetRoot(), this, FDLG_OPEN, &fi);
  if (fi.filename) LoadDoc(fi.filename);
}

void OXMain::DoViewSource() {
  FILE *f;
  char buf[1024];

  if (!_filename) return;

  OXTransientFrame *tr = new OXTransientFrame(_client->GetRoot(), this,
                                              600, 350);

  OXTextEdit *te = new OXTextEdit(tr, 10, 10, -1);
  te->SetReadOnly(True);

  tr->AddFrame(te, new OLayoutHints(LHINTS_EXPAND_ALL));

#if 1
  te->AutoUpdate(False);
  f = fopen(_filename, "r");
  if (f) {
    while (1) {
      if (fgets(buf, 1024, f) == 0) break;
      te->InsertText(buf);
    }
    fclose(f);
  } else {
    sprintf(buf, "Could not open \"%s\"", _filename);
    new OXMsgBox(_client->GetRoot(), this, new OString("Test"),
                 new OString(buf), MB_ICONSTOP, ID_OK);
  }
  te->AutoUpdate(True);
#else
  te->LoadFromFile(_filename);
#endif

  tr->SetWindowName(_filename);

  tr->MapSubwindows();
  tr->Layout();

  tr->CenterOnParent();
  tr->MapWindow();
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

void OXMain::DoToggleUnderlineLinks() {
  if (_menuView->IsEntryChecked(M_VIEW_UNDERLINE)) {
    _menuView->UnCheckEntry(M_VIEW_UNDERLINE);
    _htmlview->UnderlineLinks(0);
  } else {
    _menuView->CheckEntry(M_VIEW_UNDERLINE);
    _htmlview->UnderlineLinks(1);
  }
}

void OXMain::DoAbout() {
  OAboutInfo info;
  
  info.wname = "About HTML viewer";
  info.title = "HTML viewer\n"
               "Compiled with xclass version "XCLASS_VERSION;
  info.copyright = "Copyright © 2002, Héctor Peraza.";
  info.text = "This program is free software; you can redistribute it "
              "and/or modify it under the terms of the GNU "
              "General Public License.\n\n"
              "http://xclass.sourceforge.net";

  new OXAboutDialog(_client->GetRoot(), this, &info);
}
