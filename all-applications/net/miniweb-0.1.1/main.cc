/**************************************************************************

    This is a test program for the OXTextEdit widget.
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

#include <X11/keysym.h>

#include <xclass/utils.h>
#include <xclass/version.h>
#include <xclass/OXMsgBox.h>
#include <xclass/OXMainFrame.h>
#include <xclass/OXComboBox.h>
#include <xclass/OXFileDialog.h>
#include <xclass/OXAboutDialog.h>

#include "OXWebHtml.h"
#include "main.h"

#include "tb-open.xpm"
#include "tb-save.xpm"
#include "tb-print.xpm"
#include "tb-left.xpm"
#include "tb-right.xpm"

SToolBarData tb_data[] = {
  { "tb-open.xpm",  tb_open_xpm,  "Open File",     BUTTON_NORMAL, M_FILE_OPEN,     NULL },
  { "tb-save.xpm",  tb_save_xpm,  "Save File",     BUTTON_NORMAL, M_FILE_SAVE,     NULL },
  { "tb-print.xpm", tb_print_xpm, "Print",         BUTTON_NORMAL, M_FILE_PRINT,    NULL },
  { "",             NULL,         0,               0,             -1,              NULL },
  { "tb-left.xpm",  tb_left_xpm,  "Previous Page", BUTTON_NORMAL, M_VIEW_PREVPAGE, NULL },
  { "tb-right.xpm", tb_right_xpm, "Next Page",     BUTTON_NORMAL, M_VIEW_NEXTPAGE, NULL },
  { NULL,           NULL,         NULL,            0,             0,               NULL }
};

char *filetypes[] = { "HTML files",     "*.htm|*.html",
                      "All files",      "*",
                      NULL,             NULL };


//----------------------------------------------------------------------

int main(int argc, char **argv) {

  OXClient *clientX = new OXClient;

  char *fname = NULL;
  if (argc > 1) fname = argv[1];

  OXMain *mainWindow = new OXMain(clientX->GetRoot(), fname);
  mainWindow->MapWindow();

  clientX->Run();
}


//----------------------------------------------------------------------

OXMain::OXMain(const OXWindow *p, char *fname) :
  OXMainFrame(p, 100, 100) {

  _filename = NULL;
  _lastUrl = NULL;

  //---- menu bar

  _menuBarLayout = new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 0, 0, 1, 1);
  _menuBarItemLayout = new OLayoutHints(LHINTS_TOP | LHINTS_LEFT, 0, 4, 0, 0);

  _menuFile = new OXPopupMenu(_client->GetRoot());
  _menuFile->AddEntry(new OHotString("&Open..."), M_FILE_OPEN);
  _menuFile->AddEntry(new OHotString("Save &as..."), M_FILE_SAVE);
  _menuFile->AddSeparator();
  _menuFile->AddEntry(new OHotString("E&xit"), M_FILE_EXIT);
  _menuFile->Associate(this);
  _menuFile->DisableEntry(M_FILE_SAVE);

  _menuEdit = new OXPopupMenu(_client->GetRoot());
  _menuEdit->AddEntry(new OHotString("&Copy"), M_EDIT_COPY);
  _menuEdit->AddEntry(new OHotString("&Paste"), M_EDIT_PASTE);
  _menuEdit->Associate(this);

  _menuView = new OXPopupMenu(_client->GetRoot());
  _menuView->AddEntry(new OHotString("&Toolbar"), M_VIEW_TOOLBAR);
  _menuView->AddEntry(new OHotString("Status &Bar"), M_VIEW_STATUSBAR);
  _menuView->AddSeparator();
  _menuView->AddEntry(new OHotString("&Underline links"), M_VIEW_UNDERLINE);
  _menuView->AddSeparator();
  _menuView->AddEntry(new OHotString("Page &Source..."), M_VIEW_SOURCE);
  _menuView->Associate(this);
  _menuView->CheckEntry(M_VIEW_TOOLBAR);
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

  //---- toolbar

  _toolBarSep = new OXHorizontal3dLine(this);

  _toolBar = new OXToolBar(this);
  _toolBar->AddButtons(tb_data);

  AddFrame(_toolBarSep, new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X));
  AddFrame(_toolBar, new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 
                                      0, 0, 3, 3));

  _toolBar->AddFrame(new OXLabel(_toolBar, new OString("Location:")),
                     new OLayoutHints(LHINTS_LEFT | LHINTS_CENTER_Y,
                                      10, 0, 0, 0));

  _location = new OXComboBox(_toolBar, "http://localhost/", C_LOCATION);
  _toolBar->AddFrame(_location, new OLayoutHints(LHINTS_LEFT | LHINTS_CENTER_Y,
                                                 5, 0, 0, 0));
  _location->Associate(this);
  _location->Resize(300, _location->GetDefaultHeight());

  //---- html widget

  _viewLayout = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y);

  _htmlview = new OXWebHtml(this, 10, 10, -1);
  _htmlview->Associate(this);
  AddFrame(_htmlview, _viewLayout);

  //---- status bar

  _statusLayout = new OLayoutHints(LHINTS_BOTTOM | LHINTS_EXPAND_X, 0, 0, 3, 0);

  _statusBar = new OXStatusBar(this);
  AddFrame(_statusBar, _statusLayout);

  _loadDoc = fname;

  _menuView->CheckEntry(M_VIEW_UNDERLINE);
  _htmlview->UnderlineLinks(1);

  prev.clear();
  next.clear();

  UpdateButtons();

  for (int i = 0; i < NUM_RECENT; ++i) _history[i] = NULL;

  _geom.w = 600;
  _geom.h = 450;
  _geom.x = (_client->GetDisplayWidth() - _geom.w) / 2;
  _geom.y = (_client->GetDisplayWidth() - _geom.h) / 2;

  MapSubwindows();

  ReadIniFile();

  SetWindowName("Untitled");

  SetWMGravity(StaticGravity);
  Resize(_geom.w, _geom.h);
  Move(_geom.x, _geom.y);
  SetWMPosition(_geom.x, _geom.y);
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
  for (int i = 0; i < prev.size(); ++i) delete[] prev[i];
  for (int i = 0; i < next.size(); ++i) delete[] next[i];
  if (_lastUrl) delete[] _lastUrl;
  for (int i = 0; i < NUM_RECENT; ++i) if (_history[i]) delete[] _history[i];
}

int OXMain::CloseWindow() {
  SaveIniFile();
  return OXMainFrame::CloseWindow();
}

int OXMain::HandleMapNotify(XMapEvent *event) {

  if (_loadDoc) {
    OHtmlUri uri(_loadDoc);
    LoadDoc(&uri);
  }

  return OXMainFrame::HandleMapNotify(event);
}


void OXMain::LoadDoc(OHtmlUri *uri) {
  FILE *f;
  char buf[1024], content[300];
  const char *filename;
  int retc;

  char *urlstr = uri->BuildUri();
  _location->SetText(urlstr);
  delete[] urlstr;

  // first, check whether this URL just points to an anchor inside
  // the currently loaded document

  OHtmlUri baseUri(_htmlview->GetBaseUri());
  if (!uri->EqualsUri(&baseUri,
                      URI_SCHEME_MASK | URI_AUTH_MASK | URI_PATH_MASK |
                      URI_QUERY_MASK)) {

    // we need to load this document

    strcpy(content, "text/html");

    if (uri->zScheme && strcasecmp(uri->zScheme, "http") == 0) {
      filename = "/tmp/"APP_NAME".html.tmp";

      char *url = uri->BuildUri();

      try {
        retc = _htmlview->HttpFetch(url, filename, 0, 0, content);
      } catch (OConnectException &e) {
        sprintf(buf, "Failed to donwload \"%s\": %s", url, e.GetMessage());
        new OXMsgBox(_client->GetRoot(), this, new OString("Connect Error"),
                     new OString(buf), MB_ICONSTOP, ID_OK);
        delete[] url;
        return;
      }
      if (retc != 200) {
        sprintf(buf, "Failed to donwload \"%s\": error %d", url, retc);
        new OXMsgBox(_client->GetRoot(), this, new OString("HTTP error"),
                     new OString(buf), MB_ICONSTOP, ID_OK);
        unlink(filename);
        delete[] url;
        return;
      }
      delete[] url;
    } else {
      if (!uri->zScheme) uri->zScheme = StrDup("file");
      if (*uri->zPath != '/') {
        getcwd(buf, 1024);
        strcat(buf, "/");
        strcat(buf, uri->zPath);
        delete[] uri->zPath;
        uri->zPath = StrDup(buf);
      }
      filename = uri->zPath;
    }

    f = fopen(filename, "r");
    if (f) {
      char *url = uri->BuildUri();

      if (_filename) delete[] _filename;
      _filename = StrDup(filename);

      _htmlview->Clear();
      _htmlview->SetBaseUri(url);

      if (strcmp(content, "text/plain") == 0) {
        _htmlview->ParseText("<html><pre>\n");
      }

      while (1) {
        if (fgets(buf, 1024, f) == 0) break;
        _htmlview->ParseText(buf);
      }
      fclose(f);

      if (strcmp(content, "text/plain") == 0) {
        _htmlview->ParseText("</pre></html>\n");
      }

      _htmlview->Layout();

      char *docName = _htmlview->GetDocName();
      SetWindowName(docName ? docName : filename);

      delete[] url;

    } else {
      sprintf(buf, "Could not open \"%s\"", filename);
      new OXMsgBox(_client->GetRoot(), this, new OString("Test"),
                   new OString(buf), MB_ICONSTOP, ID_OK);
      return;
    }
  }

  if (uri->zFragment && *uri->zFragment) {
    // go to the anchor
    _htmlview->GotoAnchor(uri->zFragment);
  } else {
    _htmlview->ScrollToPosition(OPosition(0, 0));
  }

  // add the url to the stack of visited pages

  if (_lastUrl) {
    prev.push_back(_lastUrl);
    for (int i = 0; i < next.size(); ++i) delete[] next[i];
    next.clear();
    UpdateButtons();
  }
  _lastUrl = uri->BuildUri();

  // tell the OXHtml widget this page has been visited

  _htmlview->AddToVisited(_lastUrl);
}

int OXMain::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;
  OComboBoxMessage *cmsg = (OComboBoxMessage *) msg;
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

            case M_VIEW_TOOLBAR:
              DoToggleToolBar();
              break;

            case M_VIEW_STATUSBAR:
              DoToggleStatusBar();
              break;

            case M_VIEW_PREVPAGE:
              DoPrevPage();
              break;

            case M_VIEW_NEXTPAGE:
              DoNextPage();
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

    case MSG_COMBOBOX:
      switch (msg->action) {
        case MSG_TEXTCHANGED:
          // keysym == 0 happens on drop-down listbox selection
          if (cmsg->keysym == XK_Return || cmsg->keysym == 0) {
            const char *urlstr = _location->GetText();
            AddToHistory(urlstr);
            OHtmlUri uri(urlstr);
            LoadDoc(&uri);
          }
          break;

      }
      break;

    case MSG_HTML:
      switch (msg->action) {
        case MSG_CLICK:
          if (hmsg->uri) {
            OHtmlUri URI(hmsg->uri);
            LoadDoc(&URI);
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
  if (fi.filename) {
    OHtmlUri uri(fi.filename);
    LoadDoc(&uri);
  }
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

  te->AutoUpdate(False);
  te->InsertText(_htmlview->GetText());
  te->AutoUpdate(True);

  tr->SetWindowName(_filename);

  tr->MapSubwindows();
  tr->Layout();

  tr->CenterOnParent();
  tr->MapWindow();
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

void OXMain::DoToggleUnderlineLinks() {
  if (_menuView->IsEntryChecked(M_VIEW_UNDERLINE)) {
    _menuView->UnCheckEntry(M_VIEW_UNDERLINE);
    _htmlview->UnderlineLinks(0);
  } else {
    _menuView->CheckEntry(M_VIEW_UNDERLINE);
    _htmlview->UnderlineLinks(1);
  }
}

void OXMain::DoPrevPage() {
  if (prev.size() > 0) {
    int i = prev.size() - 1;
    char *url = prev[i];
    if (_lastUrl) next.push_back(_lastUrl);
    _lastUrl = NULL;
    prev.erase(prev.begin() + i);
    OHtmlUri uri(url);
    LoadDoc(&uri);
    delete[] url;
  } else {
    XBell(GetDisplay(), 0);
  }
  UpdateButtons();
}

void OXMain::DoNextPage() {
  if (next.size() > 0) {
    int i = next.size() - 1;
    char *url = next[i];
    if (_lastUrl) prev.push_back(_lastUrl);
    _lastUrl = NULL;
    next.erase(next.begin() + i);
    OHtmlUri uri(url);
    LoadDoc(&uri);
    delete[] url;
  } else {
    XBell(GetDisplay(), 0);
  }
  UpdateButtons();
}

void OXMain::UpdateButtons() {
  if (prev.size() > 0)
    tb_data[4].button->Enable();
  else
    tb_data[4].button->Disable();
  if (next.size() > 0)
    tb_data[5].button->Enable();
  else
    tb_data[5].button->Disable();
}

void OXMain::DoAbout() {
  OAboutInfo info;
  
  info.wname = "About "APP_NAME;
  info.title = APP_NAME" version "APP_VERSION"\n"
               "A mini web browser and HTML viewer\n"
               "Compiled with xclass version "XCLASS_VERSION;
  info.copyright = "Copyright © 2002-2003, Héctor Peraza.";
  info.text = "This program is free software; you can redistribute it "
              "and/or modify it under the terms of the GNU "
              "General Public License.\n\n"
              "http://xclass.sourceforge.net";

  new OXAboutDialog(_client->GetRoot(), this, &info);
}

//----------------------------------------------------------------------

void OXMain::ReadIniFile() {
  char *inipath, line[1024], arg[256];

  inipath = GetResourcePool()->FindIniFile(APP_NAME"rc", INI_READ);
  if (!inipath) return;

  OIniFile ini(inipath, INI_READ);

  while (ini.GetNext(line)) {

    if (strcasecmp(line, "defaults") == 0) {
      if (ini.GetItem("geometry", arg)) {
        if (sscanf(arg, "%d %d (%d x %d)",
                        &_geom.x, &_geom.y,
                        &_geom.w, &_geom.h) == 4) {
          if (_geom.w < 10 || _geom.w > 32000 ||
              _geom.h < 10 || _geom.h > 32000) {
            _geom.w = 600;
            _geom.h = 450;
          }
        } else {
          _geom.w = 600;
          _geom.h = 450;
          _geom.x = (_client->GetDisplayWidth() - _geom.w) / 2;
          _geom.y = (_client->GetDisplayWidth() - _geom.h) / 2;
        }
      }
      if (!ini.GetBool("show toolbar", true)) {
        HideFrame(_toolBar);
        HideFrame(_toolBarSep);
        _menuView->UnCheckEntry(M_VIEW_TOOLBAR);
      }
      if (!ini.GetBool("show status bar", true)) {
        HideFrame(_statusBar);
        _menuView->UnCheckEntry(M_VIEW_STATUSBAR);
      }

    } else if (strcasecmp(line, "history") == 0) {
      char tmp[50];

      for (int i = NUM_RECENT-1; i >= 0; --i) {
        sprintf(tmp, "url%d", i+1);
        if (ini.GetItem(tmp, arg)) AddToHistory(arg);
      }

    }
  }

  delete[] inipath;
}

void OXMain::SaveIniFile() {
  char *inipath, tmp[256];

  inipath = GetResourcePool()->FindIniFile(APP_NAME"rc", INI_WRITE);
  if (!inipath) return;

  OIniFile ini(inipath, INI_WRITE);

  ini.PutNext("defaults");
  ini.PutBool("show toolbar", _toolBar->IsVisible());
  ini.PutBool("show status bar", _statusBar->IsVisible());
#if 0
  sprintf(tmp, "%d %d (%d x %d)", _x, _y, _w, _h);
#else
  int x, y;
  Window wdummy;
  XTranslateCoordinates(GetDisplay(), _id, _client->GetRoot()->GetId(),
                        0, 0, &x, &y, &wdummy);
  sprintf(tmp, "%d %d (%d x %d)", x, y, _w, _h);
#endif
  ini.PutItem("geometry", tmp);
  ini.PutNewLine();

  ini.PutNext("history");
  for (int i = 0; i < NUM_RECENT; ++i) {
    if (_history[i]) {
      sprintf(tmp, "url%d", i+1);
      ini.PutItem(tmp, _history[i]);
    }
  }
  ini.PutNewLine();

  delete[] inipath;
}

//----------------------------------------------------------------------

void OXMain::AddToHistory(const char *url) {
  int i;

  if (!url) return;

  // first, see if the file is already there

  for (i = 0; i < NUM_RECENT; ++i) {
    if (_history[i] && (strcmp(url, _history[i]) == 0)) break;
  }

  if (i == 0) {

    return; // nothing to do, the file already was the most recent

  } else if (i < NUM_RECENT) {

    // the file was there, move it to the top of the list

    char *tmp = _history[i];

    for ( ; i > 0; --i) _history[i] = _history[i-1];
    _history[0] = tmp;

  } else {

    // new file: shift all the entries down and add the url to the head

    if (_history[NUM_RECENT-1]) delete[] _history[NUM_RECENT-1];
    for (i = NUM_RECENT-1; i > 0; --i)
      _history[i] = _history[i-1];
    _history[0] = StrDup(url);

  }

  UpdateHistory();
}

void OXMain::UpdateHistory() {
  int  i;
  char tmp[PATH_MAX];

  _location->RemoveAllEntries();

  if (!_history[0]) return;

  for (i = 0; i < NUM_RECENT; ++i) {
    if (_history[i])
      _location->AddEntry(new OString(_history[i]), i);
  }
  _location->Select(0);
}
