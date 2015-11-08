/**************************************************************************

    This file is part of xclass.
    Copyright (C) 2004, Hector Peraza.                 

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
#include <xclass/OXToolBarButton.h>
#include <xclass/OResourcePool.h>
#include <xclass/OXHelpDoc.h>
#include <xclass/OXHelpPopup.h>
#include <xclass/OXHelpWindow.h>

#include "icons/help-contents.xpm"
#include "icons/help-prev.xpm"
#include "icons/help-next.xpm"

#define HELP_CONTENTS    101
#define HELP_BACK        102
#define HELP_FORWARD     103


static SToolBarDataEx buttons[] = {
  { "help-contents.xpm", help_contents_xpm, NULL, NULL, NULL, NULL,
    "Contents", "Go to the Table of Contents", BUTTON_NORMAL, HELP_CONTENTS, NULL },
  { "help-prev.xpm", help_prev_xpm, NULL, NULL, NULL, NULL,
    "Back", "Go to Previous Page", BUTTON_NORMAL, HELP_BACK, NULL },
  { "help-next.xpm", help_next_xpm, NULL, NULL, NULL, NULL,
    "Forward", "Go to Next Page", BUTTON_NORMAL, HELP_FORWARD, NULL },
  { NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, 0, 0, NULL }
};


//----------------------------------------------------------------------

OXHelpWindow::OXHelpWindow(const OXWindow *p, const OXWindow *t,
                           int w, int h, const char *rootfile,
                           const char *curfile, const char *appname) :
  OXTransientFrame(p, t, 100, 100) {

  _filename = NULL;
  _lastUrl = NULL;

  _appName = StrDup(appname);

  //---- toolbar

  _toolBar = new OXToolBar(this);

  _toolBar->AddButtons(buttons, True, True);
  _cnt = buttons[0].button;
  _bck = buttons[1].button;
  _fwd = buttons[2].button;

  AddFrame(_toolBar, new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X));

  //---- html widget

  _viewLayout = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y);

  _hview = new OXHelpDoc(this, 10, 10, -1);
  _hview->Associate(this);
  AddFrame(_hview, _viewLayout);

  _hpopup = new OXHelpPopup(_client->GetRoot());

  if (rootfile)
    _rootfile = StrDup(rootfile);
  else
    _rootfile = NULL;

  if (curfile) 
    _loadDoc = _client->GetResourcePool()->FindHelpFile(curfile, _appName);
  else
    _loadDoc = _client->GetResourcePool()->FindHelpFile(rootfile, _appName);

  _hview->UnderlineLinks(1);

  prev.clear();
  next.clear();

  UpdateButtons();

  for (int i = 0; i < HELP_NUM_RECENT; ++i) _history[i] = NULL;

  _geom.w = w;
  _geom.h = h;
  //_geom.x = (_client->GetDisplayWidth() - _geom.w) / 2;
  _geom.x = _client->GetDisplayWidth() - _geom.w - 15;
  _geom.y = (_client->GetDisplayHeight() - _geom.h) / 2;

  MapSubwindows();

  //ReadIniFile();

  SetWindowName("Untitled");

  SetWMGravity(StaticGravity);
  Resize(_geom.w, _geom.h);
  Move(_geom.x, _geom.y);
  SetWMPosition(_geom.x, _geom.y);
  Layout();
}

OXHelpWindow::~OXHelpWindow() {
  delete _viewLayout;
  if (_filename) delete[] _filename;
  if (_rootfile) delete[] _rootfile;
  if (_loadDoc) delete[] _loadDoc;
  delete[] _appName;
  for (int i = 0; i < prev.size(); ++i) delete[] prev[i];
  for (int i = 0; i < next.size(); ++i) delete[] next[i];
  if (_lastUrl) delete[] _lastUrl;
  for (int i = 0; i < HELP_NUM_RECENT; ++i) if (_history[i]) delete[] _history[i];
  delete _hpopup;
}

int OXHelpWindow::CloseWindow() {
  //SaveIniFile();

  if (_msgObject) {
    OWidgetMessage msg(MSG_HELP, MSG_CLOSE, -1);
    SendMessage(_msgObject, &msg);
  }

  return OXTransientFrame::CloseWindow();
}

int OXHelpWindow::HandleMapNotify(XMapEvent *event) {

  if (_loadDoc) {
    OHtmlUri uri(_loadDoc);
    LoadDoc(&uri, _hview);
  }

  return OXTransientFrame::HandleMapNotify(event);
}

void OXHelpWindow::LoadDoc(OHtmlUri *uri, OXHelpDoc *dst) {
  FILE *f;
  char buf[1024];
  const char *filename;

  char *urlstr = uri->BuildUri();
  delete[] urlstr;

  // first, check whether this URL just points to an anchor inside
  // the currently loaded document

  OHtmlUri baseUri(_hview->GetBaseUri());  // use dst??
  if (!uri->EqualsUri(&baseUri,
                      URI_SCHEME_MASK | URI_AUTH_MASK | URI_PATH_MASK |
                      URI_QUERY_MASK)) {

    // load the document

    if (uri->zScheme && strcasecmp(uri->zScheme, "http") == 0) {
      new OXMsgBox(_client->GetRoot(), this,
                   new OString("Help Browser Error"),
                   new OString("Invalid http link in help file"),
                   MB_ICONSTOP, ID_OK);
      return;
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

      dst->Clear();
      dst->SetBaseUri(url);

      while (1) {
        if (fgets(buf, 1024, f) == 0) break;
        dst->ParseText(buf);
      }
      fclose(f);

      dst->Layout();

      if (dst == _hview) {
        char *docName = _hview->GetDocName();
        SetWindowName(docName ? docName : filename);
      }

      delete[] url;

    } else {
      sprintf(buf, "Could not open \"%s\"", filename);
      new OXMsgBox(_client->GetRoot(), this, new OString("Help Browser Error"),
                   new OString(buf), MB_ICONSTOP, ID_OK);
      return;
    }
  }

  if (uri->zFragment && *uri->zFragment) {
    // go to the anchor
    dst->GotoAnchor(uri->zFragment);
  } else {
    dst->ScrollToPosition(OPosition(0, 0));
  }

  if (dst != _hview) return;

  // add the url to the stack of visited pages

  if (_lastUrl) {
    prev.push_back(_lastUrl);
    for (int i = 0; i < next.size(); ++i) delete[] next[i];
    next.clear();
    UpdateButtons();
  }
  _lastUrl = uri->BuildUri();

  // tell the OXHtml widget this page has been visited

  _hview->AddToVisited(_lastUrl);
}

int OXHelpWindow::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;
  OHtmlMessage *hmsg = (OHtmlMessage *) msg;

  switch (msg->type) {
    case MSG_MENU:
    case MSG_BUTTON:
      switch (msg->action) {
        case MSG_CLICK:
          switch (wmsg->id) {

            //--------------------------------------- File

            case HELP_CONTENTS:
              DoContents();
              break;

            case HELP_BACK:
              DoPrevPage();
              break;

            case HELP_FORWARD:
              DoNextPage();
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
            if (strcasecmp(URI.zScheme, "popup") == 0) {
              LoadDoc(&URI, _hpopup->GetDoc());
              _hpopup->Relayout();
              _hpopup->Show(hmsg->x_root - 100, hmsg->y_root + 10, True);
            } else {
              LoadDoc(&URI, _hview);
            }
          }
          break;

      }
      break;

    default:
      break;

  }

  return True;
}

void OXHelpWindow::DoContents() {
  if (_rootfile) {
    if (_loadDoc) delete[] _loadDoc;
    _loadDoc = _client->GetResourcePool()->FindHelpFile(_rootfile, _appName);
    if (_loadDoc) {
      OHtmlUri uri(_loadDoc);
      LoadDoc(&uri, _hview);
    }
  } else {
    XBell(GetDisplay(), 0);
  }
  UpdateButtons();
}

void OXHelpWindow::DoPrevPage() {
  if (prev.size() > 0) {
    int i = prev.size() - 1;
    char *url = prev[i];
    if (_lastUrl) next.push_back(_lastUrl);
    _lastUrl = NULL;
    prev.erase(prev.begin() + i);
    OHtmlUri uri(url);
    LoadDoc(&uri, _hview);
    delete[] url;
  } else {
    XBell(GetDisplay(), 0);
  }
  UpdateButtons();
}

void OXHelpWindow::DoNextPage() {
  if (next.size() > 0) {
    int i = next.size() - 1;
    char *url = next[i];
    if (_lastUrl) prev.push_back(_lastUrl);
    _lastUrl = NULL;
    next.erase(next.begin() + i);
    OHtmlUri uri(url);
    LoadDoc(&uri, _hview);
    delete[] url;
  } else {
    XBell(GetDisplay(), 0);
  }
  UpdateButtons();
}

void OXHelpWindow::DoSetCurrent(const char *file) {
  if (file) {
    if (_loadDoc) delete[] _loadDoc;
    _loadDoc = _client->GetResourcePool()->FindHelpFile(file, _appName);
    if (_loadDoc) {
      OHtmlUri uri(_loadDoc);
      LoadDoc(&uri, _hview);
    }
  } else {
    DoContents();
  }
  UpdateButtons();
}

void OXHelpWindow::UpdateButtons() {
  if (prev.size() > 0)
    _bck->Enable();
  else
    _bck->Disable();
  if (next.size() > 0)
    _fwd->Enable();
  else
    _fwd->Disable();
}

//----------------------------------------------------------------------

void OXHelpWindow::ReadIniFile() {
  char *inipath, line[1024], arg[256];

  inipath = GetResourcePool()->FindIniFile("xchelprc", INI_READ);
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
    }

  }

  delete[] inipath;
}

void OXHelpWindow::SaveIniFile() {
  char *inipath, tmp[256];

  inipath = GetResourcePool()->FindIniFile("xchelprc", INI_WRITE);
  if (!inipath) return;

  OIniFile ini(inipath, INI_WRITE);

  ini.PutNext("defaults");
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

  delete[] inipath;
}

//----------------------------------------------------------------------

void OXHelpWindow::AddToHistory(const char *url) {
  int i;

  if (!url) return;

  // first, see if the file is already there

  for (i = 0; i < HELP_NUM_RECENT; ++i) {
    if (_history[i] && (strcmp(url, _history[i]) == 0)) break;
  }

  if (i == 0) {

    return; // nothing to do, the file already was the most recent

  } else if (i < HELP_NUM_RECENT) {

    // the file was there, move it to the top of the list

    char *tmp = _history[i];

    for ( ; i > 0; --i) _history[i] = _history[i-1];
    _history[0] = tmp;

  } else {

    // new file: shift all the entries down and add the url to the head

    if (_history[HELP_NUM_RECENT-1]) delete[] _history[HELP_NUM_RECENT-1];
    for (i = HELP_NUM_RECENT-1; i > 0; --i)
      _history[i] = _history[i-1];
    _history[0] = StrDup(url);

  }

}
