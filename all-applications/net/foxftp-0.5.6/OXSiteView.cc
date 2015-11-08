/**************************************************************************

    This file is part of foxftp, a ftp client for fOX.
    Copyright (C) 1998, 1999, G. Matzka.

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
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <xclass/OResourcePool.h>
#include <xclass/OFileHandler.h>
#include <xclass/OMimeTypes.h>
#include <xclass/OString.h>
#include <xclass/OXLabel.h>
#include <xclass/OXIcon.h>
#include <xclass/OXTransientFrame.h>
#include <xclass/OXFileList.h>
#include <xclass/OXMsgBox.h>
#include <xclass/OXClient.h>
#include <xclass/OXListTree.h>
#include <xclass/OXResizer.h>

#include "TDList.h"
#include "OFile.h"
#include "OFileSystem.h"
#include "OFtpConfig.h"
#include "OFtpFileHandler.h"
#include "OFtpMessage.h"
#include "OXFtpDialogs.h"
#include "OXLogView.h"
#include "OXProperties.h"
#include "OXSiteView.h"

#include "icons/hdisk.t.xpm"

//---------------------------------------------------------------------

#define M_FC_COPY		200
#define M_FC_PASTE		201
#define M_FC_DELETE		202
#define M_FC_RENAME		203
#define M_FC_PROPS_SELECT	204

#define M_FC_VIEW		210
#define M_FC_ARRANGE		211
#define M_FC_REFRESH		212
#define M_FC_PROPS_UNSELECT	213

#define M_LT_OPEN		220
#define M_LT_COPY		221
#define M_LT_PASTE		222
#define M_LT_DELETE		223
#define M_LT_RENAME		224
#define M_LT_PROPERTIES		225

#define M_VIEW_LARGE            233
#define M_VIEW_SMALL            234
#define M_VIEW_LIST             235
#define M_VIEW_DETAIL           236
#define M_VIEW_ARRANGE_BYNAME   241
#define M_VIEW_ARRANGE_BYTYPE   242
#define M_VIEW_ARRANGE_BYSIZE   243
#define M_VIEW_ARRANGE_BYDATE   244


//----------------------------------------------------------------------

OXSiteView::OXSiteView(OXMdiMainFrame *p, OMimeTypes *mime_types,
                       OFileSystem *base, int w, int h,
                       unsigned int options, unsigned long back)
 : OXMdiFrame(p, w, h, options, back) {

  _base = base;
  _genConfig = NULL;
  _siteConfig = NULL;
  _connectName = NULL;
  _alive_timeout = NULL;
  _retry_timeout = NULL;
  _retry_count = 0;
  _dirhandler = NULL;
  _waitCursor = GetResourcePool()->GetWaitCursor();

  _cache = new TDDLList<ODir *>();
  SetSiteLock(false);

  _recyclepic = _client->GetPicture("mrecycle.xpm");

  ChangeOptions(VERTICAL_FRAME);

  _selectMenu = new OXPopupMenu(_client->GetRoot());
  _selectMenu->AddPopup(new OHotString("&Transfer To"), p->GetWinListMenu());
  _selectMenu->AddSeparator();
  _selectMenu->AddEntry(new OHotString("C&opy"), M_FC_COPY);
  _selectMenu->AddEntry(new OHotString("P&aste"), M_FC_PASTE);
  _selectMenu->AddEntry(new OHotString("&Delete"), M_FC_DELETE);
  _selectMenu->AddEntry(new OHotString("Rena&me"), M_FC_RENAME);
  _selectMenu->AddSeparator();
  _selectMenu->AddEntry(new OHotString("&Properties..."), M_FC_PROPS_SELECT);
  _selectMenu->Associate(this);

  _arrangeMenu = new OXPopupMenu(_client->GetRoot());
  _arrangeMenu->AddEntry(new OHotString("By &Name"), M_VIEW_ARRANGE_BYNAME);
  _arrangeMenu->AddEntry(new OHotString("By &Type"), M_VIEW_ARRANGE_BYTYPE);
  _arrangeMenu->AddEntry(new OHotString("By &Size"), M_VIEW_ARRANGE_BYSIZE);
  _arrangeMenu->AddEntry(new OHotString("By &Date"), M_VIEW_ARRANGE_BYDATE);

  _viewMenu = new OXPopupMenu(_client->GetRoot());
  _viewMenu->AddEntry(new OHotString("Lar&ge Icons"), M_VIEW_LARGE);
  _viewMenu->AddEntry(new OHotString("S&mall Icons"), M_VIEW_SMALL);
  _viewMenu->AddEntry(new OHotString("&List"), M_VIEW_LIST);
  _viewMenu->AddEntry(new OHotString("&Details"), M_VIEW_DETAIL);

  _unselectMenu = new OXPopupMenu(_client->GetRoot());
  _unselectMenu->AddPopup(new OHotString("&View"), _viewMenu);
  _unselectMenu->AddPopup(new OHotString("&Arrange Icons"), _arrangeMenu);
  _unselectMenu->AddSeparator();
  _unselectMenu->AddEntry(new OHotString("P&aste"), M_FC_PASTE);
  _unselectMenu->AddEntry(new OHotString("&Refresh"), M_FC_REFRESH);
  _unselectMenu->AddSeparator();
  _unselectMenu->AddEntry(new OHotString("&Properties..."), M_FC_PROPS_UNSELECT);
  _unselectMenu->Associate(this);

  _treeMenu = new OXPopupMenu(_client->GetRoot());
  _treeMenu->AddEntry(new OHotString("&Open"), M_LT_OPEN);
  _treeMenu->AddPopup(new OHotString("&Transfer"), p->GetWinListMenu());
  _treeMenu->AddSeparator();
  _treeMenu->AddEntry(new OHotString("C&opy"), M_LT_COPY);
  _treeMenu->AddEntry(new OHotString("P&aste"), M_LT_PASTE);
  _treeMenu->AddEntry(new OHotString("&Delete"), M_LT_DELETE);
  _treeMenu->AddEntry(new OHotString("Rena&me"), M_LT_RENAME);
  _treeMenu->AddSeparator();
  _treeMenu->AddEntry(new OHotString("&Properties..."), M_LT_PROPERTIES);
  _treeMenu->Associate(this);

  _hframe = new OXCompositeFrame(this, 1, 1, HORIZONTAL_FRAME);
  _lt = new OXListTree(_hframe, 10, 10, -1, SUNKEN_FRAME | DOUBLE_BORDER);
  _lt->Associate(this);

  _lv = new OXFileList(_hframe, -1, mime_types, "*", 100, 100);
  _lv->AutoRefresh(False);
  _lv->Associate(this);
  ////(_lv->GetViewPort())->SetBackgroundColor(_whitePixel);

  _vres = new OXVerticalResizer(_hframe, 2, 4);
  _vres->SetPrev(_lt, 50);
  _vres->SetNext(_lv, 100);
  _hframe->AddFrame(_lt, new OLayoutHints(LHINTS_NORMAL | LHINTS_EXPAND_Y));
  _hframe->AddFrame(_vres, new OLayoutHints(LHINTS_EXPAND_Y));
  _hframe->AddFrame(_lv, new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y));

  _log = new OXLogView(this, 1, 1, SUNKEN_FRAME | DOUBLE_BORDER);
  _base->SetLogObject(_log);

  SetViewMode(LV_LARGE_ICONS);
  SetSortMode(SORT_BY_NAME);

  _hres = new OXHorizontalResizer(this, 4, 2);
  _hres->SetPrev(_hframe, 150);
  _hres->SetNext(_log, 50);

  AddFrame(_hframe, new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y));
  AddFrame(_hres, new OLayoutHints(LHINTS_EXPAND_X));
  AddFrame(_log, new OLayoutHints(LHINTS_EXPAND_X));

  XGrabButton(GetDisplay(), Button3, AnyModifier, _id, false, 
              ButtonPressMask | ButtonReleaseMask |
              EnterWindowMask | LeaveWindowMask,
              GrabModeAsync, GrabModeAsync, None, None);

  SetSiteConfig(NULL);
  MapSubwindows();

  _log->Resize(1, 100);
  _lt->Resize(150, 0);

  Layout();
}

OXSiteView::~OXSiteView() {
  SetAliveTimer(false);
  RemoveFrame(_log);
  delete _log;
  _lv->Clear(); ////_fc->RemoveAll();
  delete _cache;
  delete _selectMenu; delete _unselectMenu; delete _treeMenu;
  delete _base;
  if (_connectName) delete[] _connectName;
  if (_siteConfig) delete _siteConfig;
  if (_dirhandler) delete[] _dirhandler;
  _client->FreePicture(_recyclepic);
  if (_alive_timeout) delete _alive_timeout;
  if (_retry_timeout) delete _retry_timeout;
}

void OXSiteView::SetSiteLock(bool lock) {
  _lock = lock;
  if (_lock) {
    SetAliveTimer(false);
    XDefineCursor(GetDisplay(), _id, _waitCursor);
  } else {
    SetAliveTimer(true);
    XDefineCursor(GetDisplay(), _id, None);
  }
  XFlush(GetDisplay());
}

int OXSiteView::HandleTimer(OTimer *t) {
  if (t == _alive_timeout) {
    if (IsConnected()) _base->Noop();
    SetAliveTimer(true);
    return true;
  } else if (t == _retry_timeout) {
    Connect();
    return true;
  }
  return false;
}

void OXSiteView::SetAliveTimer(bool set) {
  if (_alive_timeout) {
    delete _alive_timeout;
    _alive_timeout = NULL;
  }
  if (set && _siteConfig && _siteConfig->_keepalive > 0) {
    _alive_timeout = new OTimer(this, _siteConfig->_keepalive * 1000);
  }
}

void OXSiteView::SetRetryTimer(bool set) {
  if (_retry_timeout) {
    delete _retry_timeout;
    _retry_timeout = NULL;
  }
  if (set && _retry_count < _genConfig->_site_retry) {
    _retry_count++;
    _retry_timeout = new OTimer(this, _genConfig->_retry_delay * 1000);
  } else {
    _retry_count = 0;
  }
}

void OXSiteView::UpdateLists(ODir *delem) {
  char *p, buf[PATH_MAX];
  OFile *felem;

  _lv->Clear();
  p = delem->GetPath();
  if (p[0] != '/')
    sprintf(buf, "%s/%s", _connectName, delem->GetPath());
  else
    sprintf(buf, "%s%s", _connectName, delem->GetPath());
  SetWindowName(buf);

  UpdateTree(delem->GetPath());
  for (int i = 0; i < delem->GetList()->GetSize(); i++) {
    felem = delem->GetList()->GetAt(i+1);
    if (S_ISDIR(felem->_type)) {
      sprintf(buf, "%s/%s", delem->GetPath(), felem->_name);
      UpdateTree(buf);
    }
    AddFile(felem);
  }
  SetSortMode(_sortMode);
  _lv->Layout();
}

int OXSiteView::HandleFileEvent(OFileHandler *fh, unsigned int mask) {
  if (fh == _dirhandler && mask == XCM_READABLE) {
    OFtpFileHandler *fs = (OFtpFileHandler*)fh;
    OHandle *tmp = fs->GetHandle();
    // dir
    OFile *felem;
    if (_base->GetFile(tmp, felem)) {
      if (strcmp(felem->_name, ".") == 0 ||
         strcmp(felem->_name, "..") == 0) {
        delete felem;
      } else {
        fs->GetDir()->GetList()->Add(felem);
      }
    } else {
      _base->Close(tmp);
      _cache->Add(fs->GetDir());
      UpdateLists(fs->GetDir());
      SetSiteLock(false);
      delete _dirhandler;
      _dirhandler = NULL;
      OStatusMessage m1(STATUS_MESSAGE, STATUS_CHANGED, "Reading directory finished");
      SendMessage(GetTopLevel(), &m1);
    }
  }

  return true;
}

int OXSiteView::HandleButton(XButtonEvent *event) {
  if (event->button != Button3 && event->type != ButtonPress)
    return true;

  int x = event->x;
  int y = event->y;
  if (y < 0)
    return true;
  if (y > _hres->GetY()) {
    if (_log->IsMapped())
      return _log->HandleButton(event);
    return true;
  }
  if (x >= _lt->GetX() && x < _vres->GetX()) {
    _treeMenu->PlaceMenu(event->x_root, event->y_root, true, true);
  } else {
    if (_lv->NumSelected() > 0) {
      _selectMenu->PlaceMenu(event->x_root, event->y_root, true, true);
    } else {
      _unselectMenu->PlaceMenu(event->x_root, event->y_root, true, true);
    }
  }
//  _transferMenu->DisableEntry(_parent->GetId());
//  _main->GetWinListPopupMenu()->DisableEntry(_parent->GetId());
//    _main->GetWinListPopupMenu()->EnableEntry(_parent->GetId());
  return true;
}

void OXSiteView::SetViewMode(int mode) {
  _viewMode = mode;
  _lv->SetViewMode(mode);
  int id = M_VIEW_LARGE;
  switch (mode) {
    case LV_LARGE_ICONS:  id = M_VIEW_LARGE;  break;
    case LV_SMALL_ICONS:  id = M_VIEW_SMALL;  break;
    case LV_LIST:         id = M_VIEW_LIST;   break;
    case LV_DETAILS:      id = M_VIEW_DETAIL; break;
  }
  _viewMenu->RCheckEntry(id, M_VIEW_LARGE, M_VIEW_DETAIL);
}

void OXSiteView::SetSortMode(int mode) {
  _sortMode = mode;
  _lv->Sort(_sortMode);
  int id = M_VIEW_ARRANGE_BYNAME;
  switch (mode) {
    case SORT_BY_NAME: id = M_VIEW_ARRANGE_BYNAME; break;
    case SORT_BY_TYPE: id = M_VIEW_ARRANGE_BYTYPE; break;
    case SORT_BY_DATE: id = M_VIEW_ARRANGE_BYDATE; break;
    case SORT_BY_SIZE: id = M_VIEW_ARRANGE_BYSIZE; break;
  }
  _arrangeMenu->RCheckEntry(id, M_VIEW_ARRANGE_BYNAME, M_VIEW_ARRANGE_BYDATE);
}

void MapPath(const char *in, char *out) {
  char *p, tmp[PATH_MAX];

  strcpy(tmp, in);
  p = strtok(tmp, "/\\");
  if (p && in[0] == '/') {
    if (p[1] == ':') {
      sprintf(out, "%s\\", p);
    } else {
      sprintf(out, "/%s/", p);
    }
    do {
      p = strtok(NULL, "/\\");
      if (p) {
        strcat(out, p);
        strcat(out, "/");
      }
    } while (p);
    out[strlen(out)-1] = '\0';
  } else {
    strcpy(out, in);  
  }
}

OListTreeItem *OXSiteView::FindItemInLt(const char *path) {
  OListTreeItem *i1, *i2;
  char *p, buf[PATH_MAX];

  i1 = _lt->GetFirstItem();
  strcpy(buf, path);
  if (buf[0] == '/') {
    p = &buf[1];
  } else {
    p = &buf[3];
  }
  p = strtok(p, "/\\");
  if (p) {
    do {
      i2 = _lt->FindChildByName(i1, p);
      i1 = i2;
    } while ((p = strtok(NULL, "/\\")) != NULL);
  }
  return i1;
}

void OXSiteView::UpdateTree(const char *path) {
  OListTreeItem *root;
  OListTreeItem *i1, *i2;
  char *p;
  char buf[PATH_MAX], buf2[256];

  MapPath(path, buf);
  if (buf[0] == '/') {
    strcpy(buf2, "/");
    p = &buf[1];
  } else {
    buf[0] = toupper(buf[0]);
    strncpy(buf2, buf, 3);
    buf2[3] = 0;
    p = &buf[3];
  }
  root = _lt->GetFirstItem();
  while (root) {
    if (strcmp(root->text, buf2) == 0) break;
    root = root->nextsibling;
  }
  if (root == NULL) {
    root = _lt->AddItem(NULL, buf2,
                        _client->GetPicture("hdisk.t.xpm", hdisk_t_xpm),
                        _client->GetPicture("hdisk.t.xpm", hdisk_t_xpm));
    _lt->OpenNode(root);
  }
  i1 = root;
  p = strtok(p, "/\\");
  if (p) {
    do {
      i2 = _lt->FindChildByName(i1, p);
      if (!i2) {
        i2 = _lt->AddItem(i1, p, NULL, NULL);
        _lt->OpenNode(i2);
        _lt->SortChildren(i1);
      }
      i1 = i2;
    } while ((p = strtok(NULL, "/\\")) != NULL);
  }
  //_lt->SortChildren(root);
  //_lt->Sort(root);
  _client->NeedRedraw(_lt);
}

int OXSiteView::CloseWindow() {
  int force = false;
  if (IsConnected()) {
    if (_genConfig->_disconnect_close) force = true;
  }
  if (Disconnect(force) == false) {
    return False;
  }
  SetSiteLock(false);
  return OXMdiFrame::CloseWindow();
}

void OXSiteView::ChangeDirectory(const char *path) {
  if (_lock) return;
  if (Chdir(path)) {
    DisplayDirectory();
  }
}

int OXSiteView::Chdir(const char *path) {
  char buf[PATH_MAX];
  int rtc = true;
  if (_lock) return true;
  if (IsConnected()) {
    MapPath(path, buf);
    SetSiteLock(true);
    rtc = _base->Chdir(buf);
    SetSiteLock(false);
  }
  return rtc;
}

void OXSiteView::AddFile(OFile *elem) {
  const OPicture *pic, *lpic, *spic, *slpic;

  if (_siteConfig->_showdot == false && elem->_name[0] == '.') {
    return;
  } else if (S_ISDIR(elem->_type) || _lv->FileMatch(elem->_name) != 0) {
    _lv->GetFilePictures(&pic, &lpic, elem->_type,
                         elem->_is_link, elem->_name, false);
    _lv->GetFilePictures(&spic, &slpic, elem->_type,
                         elem->_is_link, elem->_name, true);
    std::vector<OString *> names;
    names.push_back(new OString(elem->_name));
    names.push_back(new OString(""));  // as required by OXFileList
    names.push_back(new OString(_lv->AttributeString(elem->_type,
                                                     elem->_is_link)));
    names.push_back(new OString(_lv->SizeString(elem->_size)));
    names.push_back(new OString(elem->_user));
    names.push_back(new OString(elem->_group));
    names.push_back(new OString(_lv->TimeString(elem->_time)));
    _lv->AddItem(new OFtpItem(_lv, -1, pic, lpic, spic, slpic,
                              names, elem, _viewMode));
  }
}

void OXSiteView::DisplayDirectory(int force) {
  char path[PATH_MAX];
  ODir *celem;
  int i;

  if (_lock) return;
  if (_base->GetState() != FS_STATE_READY) return;
  if (_base->Pwd(path, PATH_MAX) != true) return;

  OStatusMessage m1(STATUS_MESSAGE, STATUS_CHANGED, "Reading directory");
  SendMessage(GetTopLevel(), &m1);

  for (celem = NULL, i = 0; i < _cache->GetSize(); i++) {
    celem = _cache->GetAt(i+1);
    if (strcmp(celem->GetPath(), path) == 0) {
      if (force) {
        _cache->Remove(i+1);
        celem = NULL;
      }
      break;
    }
    celem = NULL;
  }
  if (celem) {
    UpdateLists(celem);
  } else {
    int mode = MODE_TRANS_IMAGE;
    if (_siteConfig->_usepassive) {
      mode |= MODE_CONN_PASSIVE;
    } else {
      mode |= MODE_CONN_PORT;
    }
    OHandle *handle;
    if (_siteConfig->_showdot) {
      mode |= MODE_DIR_NLST;
      handle = _base->OpenDir(path, "-al", mode);
    } else {
      mode |= MODE_DIR_LIST;
      handle = _base->OpenDir(path, "", mode);
    }
    if (handle) {
      SetSiteLock(true);
      _dirhandler = new OFtpFileHandler(this, XCM_READABLE, handle,
                    new ODir(path, new TDDLList<OFile *>()));
    }
  }
}

OHandle *OXSiteView::OpenFile(const char *file, int mode) {
  OHandle *handle = NULL;
  char tmp[PATH_MAX];
  if (IsConnected()) {
    if (_siteConfig->_usepassive) {
      mode = (mode & ~MODE_CONN_PORT) | MODE_CONN_PASSIVE;
    } else {
      mode = (mode & ~MODE_CONN_PASSIVE) | MODE_CONN_PORT;
    }
    SetSiteLock(true);
    strcpy(tmp, file);
    if (mode & MODE_TRANS_PUT) {
      if (_siteConfig->_uploadmode == 2) {
        for (int i = 0; i < strlen(file); i++)
          tmp[i] = toupper(file[i]);
      } else if (_siteConfig->_uploadmode == 3) {
        for (int i = 0; i < strlen(file); i++)
          tmp[i] = tolower(file[i]);
      }
    }
    handle = _base->OpenFile(tmp, mode);
    if (handle == NULL)
      SetSiteLock(false);
  }
  return handle;
}

int OXSiteView::CloseFile(OHandle *handle) {
  int rtc = true;
  if (IsConnected()) {
    rtc = _base->Close(handle);
    SetSiteLock(false);
  }
  return rtc;
}

int OXSiteView::Mkdir(char *dir) {
  int rtc = true;
  if (IsConnected()) {
    SetSiteLock(true);
    rtc = _base->Mkdir(dir);
    SetSiteLock(false);
  }
  return rtc;
}

void OXSiteView::SetGeneralConfig(OGeneralConfig *genConfig) {
  _genConfig = genConfig;
}

void OXSiteView::SetSiteConfig(OSiteConfig *siteConfig) {
  if (_siteConfig) {
	delete _siteConfig;
	_siteConfig = NULL;
  }
  if (siteConfig) {
    _siteConfig = new OSiteConfig(siteConfig);
  }
  if (_siteConfig && _siteConfig->_dolog) {
    _base->SetLogObject(_log);
    ShowFrame(_log);
    ShowFrame(_hres);
    _hres->SetState(RESIZER_ACTIVE);
  } else {
    _base->SetLogObject(NULL);
    HideFrame(_log);
    HideFrame(_hres);
    _hres->SetState(RESIZER_INACTIVE);
  }
  Layout();
}

bool OXSiteView::IsConnected() {
  return (_base->GetState() == FS_STATE_READY) ? true : false;
}

int OXSiteView::Connect() {
  int rtc;
  char buf[PATH_MAX], buf2[PATH_MAX];

  OStatusMessage m1(STATUS_MESSAGE, STATUS_CHANGED, "Connecting to site");
  if (_base->NeedConnect()) {
    if (_siteConfig == NULL || _genConfig == NULL)
      return false;

    SendMessage(GetTopLevel(), &m1);
    if (_siteConfig->_useanonymous) {
      _siteConfig->SetLogin("anonymous");
      if (_genConfig->_anonymous_pw)
        _siteConfig->SetPasswd(_genConfig->_anonymous_pw);
      else
        _siteConfig->SetPasswd("guest@host.com");
    } else {
      if (!_siteConfig->_passwd || strlen(_siteConfig->_passwd) == 0) {
        new OXPasswdDialog(_client->GetRoot(), GetTopLevel(), 400, 200, 
                        &rtc, _siteConfig);
        if (rtc == false) {
          m1.SetText("Connect aborted");
          SendMessage(GetTopLevel(), &m1);
	  SetWindowName(m1.str);
          return false;
        }
      }
    }

    if (_genConfig->_proxy_mode != 1 && _siteConfig->_useproxy) {
      sprintf(buf, "%s:%d", _genConfig->_proxy, _genConfig->_proxyport);
    } else {
      sprintf(buf, "%s:%d", _siteConfig->_site, _siteConfig->_port);
    }
    sprintf(buf2, "Connecting to %s", buf);
    m1.SetText(buf2);
    SendMessage(GetTopLevel(), &m1);
    rtc = _base->Connect(buf);
    if (rtc != true) {
      m1.SetText("Connect failed");
      SendMessage(GetTopLevel(), &m1);
      SetWindowName(m1.str);
      SetRetryTimer(true);
      return false;
    }
    if (_genConfig->_proxy_mode != 1 && _siteConfig->_useproxy) {
      if (_genConfig->_proxy_mode == 3)
        sprintf(buf, "%s@%s:%d", _siteConfig->_login,
                _siteConfig->_site, _siteConfig->_port);
      else
        sprintf(buf, "%s@%s %d", _siteConfig->_login,
                _siteConfig->_site, _siteConfig->_port);

      sprintf(buf2, "Login as %s", buf);
      m1.SetText(buf2);
      SendMessage(GetTopLevel(), &m1);
      rtc = _base->Login(buf, _siteConfig->_passwd);
    } else {
      sprintf(buf2, "Login as %s", _siteConfig->_login);
      m1.SetText(buf2);
      SendMessage(GetTopLevel(), &m1);
      rtc = _base->Login(_siteConfig->_login, _siteConfig->_passwd);
    }
    if (rtc != true) {
      m1.SetText("Login failed");
      SendMessage(GetTopLevel(), &m1);
      SetWindowName(m1.str);
      _base->Quit();
      SetRetryTimer(true);
      return false;
    }
    if (_siteConfig->_port != 21)
      sprintf(buf, "ftp://%s:%d", _siteConfig->_site, _siteConfig->_port);
    else
      sprintf(buf, "ftp://%s", _siteConfig->_site);
  } else {
    sprintf(buf, "ftp://localhost");
    _siteConfig->_keepalive = 0;
  }
  _connectName = StrDup(buf);
  SetWindowName(buf);
  SetAliveTimer(true);
  SetRetryTimer(false);
  m1.SetText("Connection established");
  SendMessage(GetTopLevel(), &m1);

  _sys_type = 1;
  if (_base->SysType(buf, PATH_MAX) == 1) {
    if (strcmp(buf, "WIN32") == 0)
      _sys_type = 2;
  }
  if (_siteConfig->_ipath)
    Chdir(_siteConfig->_ipath);

  DisplayDirectory();
  return true;
}

int OXSiteView::Reconnect() {
  OStatusMessage m1(STATUS_MESSAGE, STATUS_CHANGED, "Reconnecting to site");
  SendMessage(GetTopLevel(), &m1);
  if (Disconnect(true)) {
    return Connect();
  }
  return true;
}

int OXSiteView::Disconnect(int force) {
  int rtc = false;
  if (_base->NeedConnect() == false) {
    new OXMsgBox(_client->GetRoot(), GetTopLevel(), new OString("fOX Ftp"),
                 new OString("You can't close window for local machine"), MB_ICONEXCLAMATION,
                 ID_OK, &rtc);
    return false;
  } else if (IsConnected()) {
    if (force == false) {
      new OXMsgBox(_client->GetRoot(), GetTopLevel(), new OString("fOX Ftp"),
                   new OString("Do you want co close the connection?"), MB_ICONQUESTION,
                   ID_YES | ID_NO, &rtc);
      if (rtc == ID_NO)
        return false;
    }
    rtc = true;
  }
  if (rtc) {
    OStatusMessage m1(STATUS_MESSAGE, STATUS_CHANGED, "Disconnecting from site");
    SendMessage(GetTopLevel(), &m1);
    SetWindowName("not connected");
    SetAliveTimer(false);
    _base->Quit();
    _lv->Clear(); ////->RemoveAll();
    OListTreeItem *h;
    while ((h = _lt->GetFirstItem()) != NULL) {
      _lt->DeleteItem(h);
    }
    _client->NeedRedraw(_lt);
  }
  return true;
}

void OXSiteView::ShowProperties() {
  if (_lv->NumSelected() != 0) {
    int rtc;
    OFtpItem *f;
    char tmp[PATH_MAX];
    bool local = true;

    if (_base->NeedConnect()) local = false;

    std::vector<OItem *> items = _lv->GetSelectedItems();
    _base->Pwd(tmp, PATH_MAX);
    for (int i = 0; i < items.size(); ++i) {
      f = (OFtpItem *) items[i];
      new OXFtpPropertiesDialog(_client->GetRoot(), GetTopLevel(), &rtc,
                                tmp, f->GetFileInfo(), f->GetFilePicture(0),
                                local);
      if (rtc == ID_CANCEL)
        break;
    }
    if (rtc == ID_OK)
      DisplayDirectory(true);
  }
}

void OXSiteView::SelectAll() {
  _lv->SelectAll();
}

void OXSiteView::InvertSelection() {
  _lv->InvertSelection();
}

void OXSiteView::DoDelete() {
  int rtc = ID_NO;
  TDDLList<ODir *> *list;

  if (_lock) return;
  list = CreateFileListFromFc();
  if (list->GetSize() > 0) {
    OStatusMessage m1(STATUS_MESSAGE, STATUS_CHANGED, "");
    Delete(rtc, list, NULL);
    if (rtc == ID_CANCEL) 
      m1.SetText("Delete canceled");
    else
      m1.SetText("Delete finished");
    SendMessage(GetTopLevel(), &m1);
    DisplayDirectory(true);
  }
  delete list;
}

void OXSiteView::Delete(int &rtc, TDDLList<ODir *> *dlist, ODir *delem) {
  ODir *dtmp;
  TDDLList<OFile *> *flist;
  char buf[256], buf2[256];

  if (!dlist) return;

  if (delem == NULL) {
    delem = dlist->GetHead();
  }
  flist = delem->GetList();

  for (int j = 0; j < flist->GetSize(); j++) {
    OFile *felem = flist->GetAt(j + 1);
    if (S_ISDIR(felem->_type)) {
      if (rtc != ID_YESALL) {
        // confirm delete for files in directory %s
        sprintf(buf, "Examine files in directory \"%s\"?", felem->_name);
        new OXMsgBox(_client->GetRoot(), GetTopLevel(),
                     new OString("Confirm examine"), new OString(buf), _recyclepic,
                     ID_YES | ID_YESALL | ID_NO | ID_CANCEL, &rtc);
      }
      if (rtc == ID_CANCEL)
        return;
      if (rtc == ID_YES || rtc == ID_YESALL) {
        int i;
        char *p = strrchr(felem->_name, '/');
        if (p)
          sprintf(buf, "%s/%s", delem->GetPath(), &p[1]);
        else
          sprintf(buf, "%s/%s", delem->GetPath(), felem->_name);
        for (i = 0; i < dlist->GetSize(); i++) {
          dtmp = dlist->GetAt(i+1);
          if (strcmp(dtmp->GetPath(), buf) == 0) {
            break;
          }
          dtmp = NULL;
        }
        if (dtmp) {
          Delete(rtc, dlist, dtmp);
          if (rtc == ID_CANCEL)
            return;
          if (_base->Rmdir(dtmp->GetPath())) {
            sprintf(buf2, "Directory %s deleted", buf);
          } else {
            sprintf(buf2, "Directory %s delete failed", buf);
          }
          OStatusMessage m1(STATUS_MESSAGE, STATUS_CHANGED, buf2);
          SendMessage(GetTopLevel(), &m1);
          OListTreeItem *item = FindItemInLt(dtmp->GetPath());
          if (item) _lt->DeleteItem(item);
          for (i = 0; i < dlist->GetSize(); i++) {
            dtmp = dlist->GetAt(i+1);
            if (strcmp(dtmp->GetPath(), buf) == 0) {
              dlist->Remove(i+1);
              break;
            }
          }
        }
      }
    } else {
      if (rtc != ID_YESALL) {
        int buttons;
        if (j == flist->GetSize() - 1) {
          // last element or single file
          buttons = ID_YES | ID_NO;
        } else {
          buttons = ID_YES | ID_YESALL | ID_NO | ID_CANCEL;
        }
        sprintf(buf, "Are you sure you want to delete file \"%s\"?", felem->_name);
        new OXMsgBox(_client->GetRoot(), GetTopLevel(),
                     new OString("Confirm file delete"), new OString(buf), _recyclepic,
                     buttons, &rtc);
      }
      if (rtc == ID_CANCEL)
        return;
      else if (rtc == ID_YES || rtc == ID_YESALL) {
        char *p = strrchr(felem->_name, '/');
        if (p)
          sprintf(buf, "%s/%s", delem->GetPath(), &p[1]);
        else
          sprintf(buf, "%s/%s", delem->GetPath(), felem->_name);
        if (_base->Delete(buf)) {
          sprintf(buf2, "File %s deleted", buf);
        } else {
          sprintf(buf2, "File %s delete failed", buf);
        }
        OStatusMessage m1(STATUS_MESSAGE, STATUS_CHANGED, buf2);
        SendMessage(GetTopLevel(), &m1);
      }
    }
  }
  return;
}

TDDLList<ODir *> *OXSiteView::CreateFileList() {
  return CreateFileListFromFc();
}

TDDLList<ODir *> *OXSiteView::CreateFileListFromFc() {
  char path[PATH_MAX], buf[PATH_MAX];
  OFtpItem *item;
  OFile *felem;

  if (_lock) return NULL;
  if (_base->Pwd(path, PATH_MAX) != true)
    return NULL;

  OStatusMessage m1(STATUS_MESSAGE, STATUS_CHANGED, "Creating file list");
  SendMessage(GetTopLevel(), &m1);

  TDDLList<ODir *> *list = new TDDLList<ODir *>();
  TDDLList<OFile *> *flist = new TDDLList<OFile *>();
  ODir *delem = new ODir(path, flist);
  list->Add(delem);

  std::vector<OItem *> items = _lv->GetSelectedItems();

  for (int i = 0; i < items.size(); ++i) {
    item = (OFtpItem *) items[i];
    felem = item->GetFileInfo();
    flist->Add(new OFile(felem));
    if (S_ISDIR(felem->_type)) {
      sprintf(buf, "%s/%s", path, felem->_name);
      CreateSubFolderList(buf, felem->_name, list);
    }
  }

  return list;
}

TDDLList<ODir *> *OXSiteView::CreateFileListFromLt() {
  char tmp[PATH_MAX], path[PATH_MAX];
  OListTreeItem *h;

  if (_lock) return NULL;

  OStatusMessage m1(STATUS_MESSAGE, STATUS_CHANGED, "Creating file list");
  SendMessage(GetTopLevel(), &m1);

  TDDLList<ODir *> *list = new TDDLList<ODir *>();
  if ((h = _lt->GetSelected()) != NULL) {
    if (h->parent) {
      _lt->GetPathnameFromItem(h->parent, tmp);
      MapPath(tmp, path);
      ODir *delem = new ODir(path, new TDDLList<OFile *>());
      OFile *felem = new OFile();
      felem->SetName(h->text);
      felem->_type = S_IFDIR;
      delem->GetList()->Add(felem);
      list->Add(delem);
    }
    _lt->GetPathnameFromItem(h, tmp);
    MapPath(tmp, path);
    CreateSubFolderList(path, h->text, list);
  }
  return list;
}

int OXSiteView::CreateSubFolderList(char *abs_path, char *rel_path,
                                    TDDLList<ODir *> *list) { 
  char buf[PATH_MAX];
  OFile *felem;

  TDDLList<OFile *> *flist = new TDDLList<OFile *>();
  ODir *delem = new ODir(abs_path, flist);
  list->Add(delem);

  int mode = MODE_TRANS_IMAGE;
  if (_siteConfig->_usepassive) {
    mode |= MODE_CONN_PASSIVE;
  } else {
    mode |= MODE_CONN_PORT;
  }
  OHandle *handle;
  if (_siteConfig->_showdot) {
    mode |= MODE_DIR_NLST;
    handle = _base->OpenDir(abs_path, "-al", mode);
  } else {
    mode |= MODE_DIR_LIST;
    handle = _base->OpenDir(abs_path, "", mode);
  }
  if (handle) {
    while (_base->GetFile(handle, felem)) {
      if (strcmp(felem->_name, ".") == 0 ||
         strcmp(felem->_name, "..") == 0) {
         delete felem;
         continue;
      }
      sprintf(buf, "%s/%s", rel_path, felem->_name);
      felem->SetName(buf);
      flist->Add(felem);
    }
    _base->Close(handle);
  }
  for (int i = 0; i < flist->GetSize(); i++) {
    felem = flist->GetAt(i + 1);
    if (S_ISDIR(felem->_type)) {
      char *p = strrchr(felem->_name, '/');
      if (p)
        sprintf(buf, "%s/%s", delem->GetPath(), &p[1]);
      else
        sprintf(buf, "%s/%s", delem->GetPath(), felem->_name);
      UpdateTree(buf);
      CreateSubFolderList(buf, felem->_name, list);
    }
  }
  return true;
}

int OXSiteView::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;
  OItemViewMessage *ivmsg = (OItemViewMessage *) msg;

  switch (msg->type) {
    case MSG_MENU:
      switch (msg->action) {
        case MSG_CLICK:
          switch (wmsg->id) {
            case M_FC_REFRESH: {
              DisplayDirectory(true);
              break;
            }
            case M_FC_RENAME: {
              if (_lv->NumSelected() == 1) {
                OFtpItem *f;
                char tmp[PATH_MAX];
                std::vector<OItem *> items = _lv->GetSelectedItems();
                f = (OFtpItem *) items[0];
                if (f->EditName(tmp)) {
                  if (strlen(tmp) != 0 &&
                     strcmp(tmp, f->GetName()->GetString()) != 0) {
                    if (_base->Rename(f->GetName()->GetString(), tmp)) {
                      OStatusMessage m1(STATUS_MESSAGE, STATUS_CHANGED, "Rename successful");
                      SendMessage(GetTopLevel(), &m1);
                      DisplayDirectory(true);
                    } else {
                      int rtc;
                      new OXMsgBox(_client->GetRoot(), GetTopLevel(),
                                   new OString("fOX FTP"),
                                   new OString("Rename of file failed"), MB_ICONSTOP,
                                   ID_OK, &rtc);
                    }
                  }
                }
              }
              break;
            }
            case M_FC_DELETE: {
              DoDelete();
              break;
            }
            case M_FC_PROPS_SELECT: {
              ShowProperties();
              break;
            }

            case M_LT_OPEN: {
              OListTreeItem *h;
              if ((h = _lt->GetSelected()) != NULL) {
                char tmp[2048];
                _lt->GetPathnameFromItem(h, tmp);
                _lt->OpenNode(h);
               ChangeDirectory(tmp);
              }
              break;
            }
            case M_LT_DELETE: {
              int rtc = ID_NO;
              if (_lock == false) {
                TDDLList<ODir *> *list = CreateFileListFromLt();
                if (list->GetSize() > 0) {
                  Delete(rtc, list, NULL);
                  DisplayDirectory(true);
                }
                delete list;
              }
              break;
            }
            case M_LT_RENAME: {
              OListTreeItem *h;
              if ((h = _lt->GetSelected()) != NULL) {
                //char tmp[2048];
                //if (h->EditName(tmp)) {
                //  printf("NAME CHANGED TO:%s\n", tmp);
                //}
              }
              break;
            }

            case M_VIEW_LARGE:
            case M_VIEW_SMALL:
            case M_VIEW_LIST:
            case M_VIEW_DETAIL:
            case M_VIEW_ARRANGE_BYNAME:
            case M_VIEW_ARRANGE_BYTYPE:
            case M_VIEW_ARRANGE_BYSIZE:
            case M_VIEW_ARRANGE_BYDATE: {
              OWidgetMessage newmsg(MSG_MENU, MSG_CLICK, wmsg->id - 200);
              SendMessage(GetTopLevel(), &newmsg);
              break;
            }

            default: {
              OXMdiFrame *tmp = _main->GetMdiFrame(wmsg->id);
              if (tmp) {
                OTransferMessage msg(FTP_TRANSFER, FTP_START, 
                                     this, (OXSiteView*)tmp);
                SendMessage(GetTopLevel(), &msg);
              }
              break;	
            }
          }
      }
      break;

    case MSG_LISTVIEW:
      switch (msg->action) {
        case MSG_DBLCLICK:
          if (ivmsg->button == Button1) {
            int sel;
            std::vector<OItem *> items;
            OFtpItem *f;

            if ((sel = _lv->NumSelected()) == 1) {
              items = _lv->GetSelectedItems();
              f = (OFtpItem *) items[0];
              if (S_ISDIR(f->GetFileInfo()->_type) ||
                 S_ISLNK(f->GetFileInfo()->_type)) {
                ChangeDirectory(f->GetName()->GetString());
              }
            }
          }
          break;

        default:
          break;
      }
      break;

    case MSG_LISTTREE:
      _client->NeedRedraw(_lt);
      switch (msg->action) {
        case MSG_CLICK: {
          OListTreeItem *h;
          if ((h = _lt->GetSelected()) != NULL) {
            char tmp[2048];
            _lt->GetPathnameFromItem(h, tmp);
            //_lt->OpenNode(h);
            ChangeDirectory(tmp);
          }
          break;
        }
      }
      break;
  }

  return true;
}
