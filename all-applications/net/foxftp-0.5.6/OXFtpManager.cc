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

#include <string.h>
#include <unistd.h>

#include <vector>

#include <xclass/utils.h>
#include <xclass/OIniFile.h>
#include <xclass/OX3dLines.h>
#include <xclass/OXCanvas.h>
#include <xclass/OXListView.h>
#include <xclass/OXLabel.h>
#include <xclass/OXTextEdit.h>
#include <xclass/OXTextEntry.h>
#include <xclass/OXSecretTextEntry.h>
#include <xclass/OXCheckButton.h>
#include <xclass/OXListTree.h>
#include <xclass/O2ColumnsLayout.h>

#include "TDList.h"
#include "OPackedMatrixLayout.h"
#include "OFile.h"
#include "OFtpConfig.h"
#include "OFtpMessage.h"
#include "OXFtpManager.h"
#include "main.h"


char *foxftp_filetypes[] = {
  "fOXFtp ini files", "*.ini",
  "All files",        "*",
  NULL,               NULL
};


//----------------------------------------------------------------------

OBookmark::OBookmark(const char *name, const char *description,
                     OSiteConfig *info) {
  _info = info;
  _name = NULL;
  _description = new OString(description);
  SetName(name);
}

OBookmark::OBookmark(const char *name, OString *description,
                     OSiteConfig *info) {
  _info = info;
  _name = NULL;
  _description = new OString(description);
  SetName(name);
}

OBookmark::OBookmark(const OBookmark &item) {
  _info = NULL;
  _description = new OString(item._description);
  SetName(item._name);
  SetSiteConfig(item._info);
}

OBookmark::~OBookmark() {
  if (_name) delete[] _name;
  if (_description) delete _description;
  if (_info) delete _info;
}

void OBookmark::SetName(const char *name) {
  if (_name) delete []_name;
  _name = NULL;
  if (name) _name = StrDup(name);
}

void OBookmark::SetDescription(const char *description) {
  if (_description) delete _description;
  _description = NULL;
  if (description) _description = new OString(description);
}

void OBookmark::SetDescription(OString *description) {
  if (_description) delete _description;
  _description = NULL;
  if (description) _description = new OString(description);
}

void OBookmark::SetSiteConfig(OSiteConfig *info) {
  if (_info) delete _info;
  _info = new OSiteConfig(info);
}


//----------------------------------------------------------------------

OBookmarkList::OBookmarkList(char *path, TDDLList<OBookmark*> *list) {
  _path = NULL;
  _list = NULL;
  SetPath(path);
  SetList(list);
}

OBookmarkList::~OBookmarkList() {
  if (_path) delete[] _path;
  if (_list) delete _list;
}

void OBookmarkList::SetPath(char *path) {
  if (_path) delete []_path;
  _path = NULL;
  if (path) _path = StrDup(path);
}

void OBookmarkList::SetList(TDDLList<OBookmark*> *list) {
  if (_list) delete _list;
  _list = NULL;
  if (list) _list = list;
}


//----------------------------------------------------------------------

OXBookmarkProp::OXBookmarkProp(const OXWindow *p, const OXWindow *main,
                               int w, int h, OBookmark *bmark, int *rtc,
                               unsigned long options) :
  OXTransientFrame(p, main, w, h, options) {

  _l1 = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y, 5, 5, 5, 5);
  _l2 = new OLayoutHints(LHINTS_NORMAL, 40, 2, 8, 2);
  _l3 = new OLayoutHints(LHINTS_NORMAL, 50, 2, 2, 2);
  _l4 = new OLayoutHints(LHINTS_NORMAL, 2, 2, 2, 2);
  _l41 = new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 2, 2, 2, 2);
  _l42 = new OLayoutHints(LHINTS_TOP | LHINTS_CENTER_Y, 2, 2, 2, 2);
  _l5 = new OLayoutHints(LHINTS_TOP | LHINTS_RIGHT, 2, 2, 2, 2);

  _tab = new OXTab(this, 10, 10);
  _tabFrame1 = _tab->AddTab(new OString("General"));
  _tabFrame2 = _tab->AddTab(new OString("Site"));
  _tabFrame3 = _tab->AddTab(new OString("Advanced"));

  //---- General

  _tabFrame1->ChangeOptions(VERTICAL_FRAME);
  _tabFrame1->SetLayoutManager(new OPackedMatrixLayout(_tabFrame1, 0, 2, 4));
  _lname = new OXLabel(_tabFrame1, new OString("Name:"));
  _tename = new OXTextEntry(_tabFrame1, new OTextBuffer(128), 11);
  _tename->Resize(200, _tename->GetDefaultHeight());
  _ldes = new OXLabel(_tabFrame1, new OString("Description:"));
  _teddes = new OXTextEdit(_tabFrame1, 200, 150, 12);

  _tabFrame1->AddFrame(_lname);
  _tabFrame1->AddFrame(_tename);
  _tabFrame1->AddFrame(_ldes);
  _tabFrame1->AddFrame(_teddes);

  //---- Site

  _tabFrame2->ChangeOptions(VERTICAL_FRAME);
  _mframe = new OXCompositeFrame(_tabFrame2, 1, 1);
  _mframe->SetLayoutManager(new O2ColumnsLayout(_mframe, 5, 4));
  _lhost = new OXLabel(_mframe, new OString("Hostname:"));
  _tehost = new OXTextEntry(_mframe, new OTextBuffer(128), 21);
  _lport = new OXLabel(_mframe, new OString("Port:"));
  _teport = new OXTextEntry(_mframe, new OTextBuffer(128), 22);
  _linitd = new OXLabel(_mframe, new OString("Initial Dir:"));
  _teipath = new OXTextEntry(_mframe, new OTextBuffer(128), 23);
  _tehost->Resize(200, _tehost->GetDefaultHeight());
  _teport->Resize(200, _teport->GetDefaultHeight());
  _teipath->Resize(200, _teipath->GetDefaultHeight());
  _mframe->AddFrame(_lhost);
  _mframe->AddFrame(_tehost);
  _mframe->AddFrame(_lport);
  _mframe->AddFrame(_teport);
  _mframe->AddFrame(_linitd);
  _mframe->AddFrame(_teipath);

  _gblogin = new OXGroupFrame(_tabFrame2, new OString("Login Information"));
  _cbanonymous = new OXCheckButton(_gblogin, new OHotString("Use anonymous"), 30);
  _cbanonymous->Associate(this);
  _mframe2 = new OXCompositeFrame(_gblogin, 1, 1);
  _mframe2->SetLayoutManager(new O2ColumnsLayout(_mframe2, 5, 4));
  _llogin = new OXLabel(_mframe2, new OString("Login:"));
  _telogin = new OXTextEntry(_mframe2, new OTextBuffer(128), 31);
  _lpasswd = new OXLabel(_mframe2, new OString("Password:"));
  _tepasswd = new OXSecretTextEntry(_mframe2, new OTextBuffer(128), 32);
  _laccount = new OXLabel(_mframe2, new OString("Account:"));
  _teaccount = new OXTextEntry(_mframe2, new OTextBuffer(128), 33);
  _telogin->Resize(180, _telogin->GetDefaultHeight());
  _tepasswd->Resize(180, _tepasswd->GetDefaultHeight());
  _teaccount->Resize(180, _teaccount->GetDefaultHeight());
  _mframe2->AddFrame(_llogin);
  _mframe2->AddFrame(_telogin);
  _mframe2->AddFrame(_lpasswd);
  _mframe2->AddFrame(_tepasswd);
  _mframe2->AddFrame(_laccount);
  _mframe2->AddFrame(_teaccount);
  _gblogin->AddFrame(_cbanonymous, _l4);
  _gblogin->AddFrame(_mframe2, _l4);
  _tabFrame2->AddFrame(_mframe, _l1);
  _tabFrame2->AddFrame(_gblogin, _l4);

  //---- Advanced

  _tabFrame3->ChangeOptions(VERTICAL_FRAME);
  _cball = new OXCheckButton(_tabFrame3, new OHotString("Show all entries"), 41);
  _cbpassive = new OXCheckButton(_tabFrame3, new OHotString("Use passive mode (PASV)"), 42);
  _cbproxy = new OXCheckButton(_tabFrame3, new OHotString("Use firewall"), 43);
  _cblog = new OXCheckButton(_tabFrame3, new OHotString("Log commands"), 44);
  _cbalive = new OXCheckButton(_tabFrame3, new OHotString("Keep connection alive"), 45);
  _cbalive->Associate(this);
  OXCompositeFrame *_f1 = new OXCompositeFrame(_tabFrame3, 1, 1, HORIZONTAL_FRAME);
  _f1->AddFrame(new OXLabel(_f1, new OString("Timeout:")), _l42);
  _tealive = new OXTextEntry(_f1, NULL, 46);
  _tealive->Resize(100, _tealive->GetDefaultHeight());
  _f1->AddFrame(_tealive, _l42);

  _lupload = new OXLabel(_tabFrame3, new OString("Upload files"));
  _lbupload = new OXDDListBox(_tabFrame3, 47);
  _lbupload->AddEntry(new OString("Case sensitive"), 51);
  _lbupload->AddEntry(new OString("Uppercase"), 52);
  _lbupload->AddEntry(new OString("Lowercase"), 53);
  _lbupload->Resize(200, _lbupload->GetDefaultHeight());
  _tabFrame3->AddFrame(_cball, _l2);
  _tabFrame3->AddFrame(_cbpassive, _l2);
  _tabFrame3->AddFrame(_cbproxy, _l2);
  _tabFrame3->AddFrame(_cblog, _l2);
  _tabFrame3->AddFrame(_cbalive, _l2);
  _tabFrame3->AddFrame(_f1, _l3);
  _tabFrame3->AddFrame(_lupload, _l2);
  _tabFrame3->AddFrame(_lbupload, _l3);

  //---- OK and Cancel buttons

  _bframe = new OXCompositeFrame(this, 1, 1, HORIZONTAL_FRAME | FIXED_WIDTH);
  _tbOk = new OXTextButton(_bframe, new  OHotString("&OK"), 1);
  _tbCancel = new OXTextButton(_bframe, new  OHotString("&Cancel"), 2);
  _tbOk->Associate(this);
  _tbCancel->Associate(this);
  _bframe->AddFrame(_tbOk, _l41);
  _bframe->AddFrame(_tbCancel, _l41);

  int bwidth = max(_tbOk->GetDefaultWidth(), _tbCancel->GetDefaultWidth());
  _bframe->Resize(bwidth * 2 + 30, _bframe->GetDefaultHeight());

  AddFrame(_tab, _l1);
  AddFrame(_bframe, _l5);

  _rtc = rtc;
  *_rtc = false;
  _bmark = bmark;
  if (strcmp(_bmark->GetName(), "New Bookmark") == 0) {
    SetWindowName("New Bookmark");
  } else {
    SetWindowName("Edit Bookmark");
  }

  //---- init controls...

  _teddes->InsertText(_bmark->GetDescription());

  OSiteConfig *info = _bmark->GetSiteConfig();

  if (_bmark->GetName()) _tename->AddText(0, _bmark->GetName());
  if (info->_site) _tehost->AddText(0, info->_site);

  if (info->_useanonymous) {
    _telogin->AddText(0, "anonymous");
    _telogin->Disable();
    _tepasswd->AddText(0, "guest@");
    _cbanonymous->SetState(BUTTON_DOWN);
  } else {
    if (info->_login) _telogin->AddText(0, info->_login);
    if (info->_passwd) _tepasswd->AddText(0, info->_passwd);
  }

  if (info->_ipath) _teipath->AddText(0, info->_ipath);

  char buf[16];
  sprintf(buf, "%d", info->_port);
  _teport->AddText(0, buf);

  if (info->_showdot) _cball->SetState(BUTTON_DOWN);
  if (info->_useproxy) _cbproxy->SetState(BUTTON_DOWN);
  if (info->_dolog) _cblog->SetState(BUTTON_DOWN);
  if (info->_usepassive) _cbpassive->SetState(BUTTON_DOWN);

  sprintf(buf, "%d", info->_keepalive);
  _tealive->AddText(0, buf);
  if (info->_keepalive > 0) {
    _cbalive->SetState(BUTTON_DOWN);
  } else {
    _tealive->Disable();
  }

  if (info->_uploadmode == 1)
    _lbupload->Select(51);
  else if (info->_uploadmode == 2)
    _lbupload->Select(52);
  else
    _lbupload->Select(53);

  SetDefaultAcceptButton(_tbOk);
  SetDefaultCancelButton(_tbCancel);
  SetFocusOwner(_tename);

  SetMWMHints(MWM_DECOR_ALL | MWM_DECOR_MAXIMIZE | MWM_DECOR_MENU,
            MWM_FUNC_ALL | MWM_FUNC_MAXIMIZE | MWM_FUNC_RESIZE,
            MWM_INPUT_MODELESS);

  MapSubwindows();
  Resize(GetDefaultSize());
  CenterOnParent();
  MapWindow();

  _client->WaitFor(this);
}

OXBookmarkProp::~OXBookmarkProp() {
  delete _l1;
  delete _l2;
  delete _l3;
  delete _l4;
  delete _l41;
  delete _l42;
  delete _l5;
}

int OXBookmarkProp::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;

  switch (msg->type) {
    case MSG_BUTTON:
    case MSG_MENU:
      switch (msg->action) {
        case MSG_CLICK:
          switch (wmsg->id) {
            case 1: {
              *_rtc = true;
              // copy all
              OSiteConfig *info = _bmark->GetSiteConfig();
              _bmark->SetName((char*)_tename->GetString());
              _bmark->SetDescription(_teddes->GetText()->GetString());
              info->SetSite((char*)_tehost->GetString());
              if (_cbanonymous->GetState() == BUTTON_DOWN) {
                info->SetLogin("anonymous");
                info->SetPasswd("guest@");
                info->_useanonymous = true;
              } else {
                info->SetLogin((char*)_telogin->GetString());
                info->SetPasswd((char*)_tepasswd->GetString());
                info->_useanonymous = false;
              }
              info->SetInitialPath((char*)_teipath->GetString());
              info->_port = atoi(_teport->GetString());
              if (_cbproxy->GetState() == BUTTON_DOWN) {
                info->_useproxy = true;
              } else {
                info->_useproxy = false;
              }
              if (_cblog->GetState() == BUTTON_DOWN) {
                info->_dolog = true;
              } else {
                info->_dolog = false;
              }
              if (_cball->GetState() == BUTTON_DOWN) {
                info->_showdot = true;
              } else {
                info->_showdot = false;
              }
              if (_cbpassive->GetState() == BUTTON_DOWN) {
                info->_usepassive = true;
              } else {
                info->_usepassive = false;
              }
              if (_cbalive->GetState() == BUTTON_DOWN) {
                info->_keepalive = atoi(_tealive->GetString());
              } else {
                info->_keepalive = 0;
              }
              int id = _lbupload->GetSelected();
              if (id >= 51 && id <= 53)
                info->_uploadmode = id - 50;
              else
                info->_uploadmode = 1;
              CloseWindow();
              break;
            }

            case 2:
              *_rtc = false;
              CloseWindow();
              break;
          }
          break;
      }
      break;

    case MSG_CHECKBUTTON:
      switch (msg->action) {
        case MSG_CLICK:
          switch (wmsg->id) {
            case 30: {
              OSiteConfig *info = _bmark->GetSiteConfig();
              _telogin->Clear();
              _tepasswd->Clear();
              _client->NeedRedraw(_telogin);
              _client->NeedRedraw(_tepasswd);
              if (_cbanonymous->GetState() == BUTTON_DOWN) {
                _telogin->Disable();
              } else {
                if (info->_login)
                  _telogin->AddText(0, info->_login);
                if (info->_passwd)
                  _tepasswd->AddText(0, info->_passwd); 
                _telogin->Enable();
              }
              break;
            }

            case 45: {
              _client->NeedRedraw(_tealive);
              if (_cbalive->GetState() == BUTTON_DOWN) {
                _tealive->Enable();
              } else {
                _tealive->Disable();
              }
              break;
            }
          }
          break;
      }
      break;
  }

  return true;
}


//----------------------------------------------------------------------

class OXFolderProp : public OXTransientFrame {
public:
  OXFolderProp(const OXWindow *p, const OXWindow *main, int w, int h,
               char *name, int *rtc, unsigned long options);
  virtual ~OXFolderProp();

  virtual int ProcessMessage(OMessage *);

protected:
  char *_name;
  int *_rtc;

  OXCompositeFrame *_bframe;
  OXTextButton *_tbOk, *_tbCancel;
  OXLabel *_lname;
  OXTextEntry *_tename;
  OLayoutHints *_l1, *_l2, *_l3;
};

OXFolderProp::OXFolderProp(const OXWindow *p, const OXWindow *main, int w, int h,
               char *name, int *rtc,
               unsigned long options = MAIN_FRAME | VERTICAL_FRAME) :
  OXTransientFrame(p, main, w, h, options) {

  _l1 = new OLayoutHints(LHINTS_CENTER_X, 5, 5, 4, 4);
  _l2 = new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 2, 2, 2, 2);
  _l3 = new OLayoutHints(LHINTS_TOP | LHINTS_RIGHT, 2, 2, 2, 2);

  _lname = new OXLabel(this, new OString("Enter new folder name:"));
  _tename = new OXTextEntry(this, new OTextBuffer(128), 11);
  _tename->Resize(250, _tename->GetDefaultHeight());

  _bframe = new OXCompositeFrame(this, 1, 1, HORIZONTAL_FRAME | FIXED_WIDTH);
  _tbOk = new OXTextButton(_bframe, new  OHotString("&OK"), 1);
  _tbCancel = new OXTextButton(_bframe, new  OHotString("&Cancel"), 2);
  _tbOk->Associate(this);
  _tbCancel->Associate(this);
  _bframe->AddFrame(_tbOk, _l2);
  _bframe->AddFrame(_tbCancel, _l2);

  int bwidth = max(_tbOk->GetDefaultWidth(), _tbCancel->GetDefaultWidth());
  _bframe->Resize(bwidth * 2 + 30, _bframe->GetDefaultHeight());

  AddFrame(_lname, _l1);
  AddFrame(_tename, _l1);
  AddFrame(_bframe, _l3);

  _rtc = rtc;
  *_rtc = false;
  _name = name;
  if (strcmp(_name, "New Folder") == 0) {
    SetWindowName("New Folder");
  } else {
    SetWindowName("Rename Folder");
  }
  _tename->AddText(0, _name);

  SetDefaultAcceptButton(_tbOk);
  SetDefaultCancelButton(_tbCancel);
  SetFocusOwner(_tename);

  SetMWMHints(MWM_DECOR_ALL | MWM_DECOR_MAXIMIZE | MWM_DECOR_MENU,
            MWM_FUNC_ALL | MWM_FUNC_MAXIMIZE | MWM_FUNC_RESIZE,
            MWM_INPUT_MODELESS);

  MapSubwindows();
  Resize(GetDefaultSize());
  CenterOnParent();
  MapWindow();

  _client->WaitFor(this);
}

OXFolderProp::~OXFolderProp() {
  delete _l1;
  delete _l2;
  delete _l3;
}

int OXFolderProp::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;

  switch (msg->type) {
    case MSG_BUTTON:
      switch (msg->action) {
        case MSG_CLICK:
          switch (wmsg->id) {
            case 1:
              *_rtc = true;
              strcpy(_name, _tename->GetString());
              CloseWindow();
              break;
            case 2:
              *_rtc = false;
              CloseWindow();
              break;
          }
          break;
      }
      break;

  }

  return True;
}


//----------------------------------------------------------------------

#include "icons/bm-file.xpm"

#define M_SAVE_BOOKMARK		101
#define M_CONNECT		102
#define M_CLOSE			103

#define M_NEW_BOOKMARK		201
#define M_EDIT_BOOKMARK		202
#define M_COPY_BOOKMARK		203
#define M_DELETE_BOOKMARK	204
#define M_NEW_FOLDER		205
#define M_RENAME_FOLDER		206
#define M_DELETE_FOLDER		207


//----------------------------------------------------------------------

OXFtpManager::OXFtpManager(const OXWindow *p, const OXWindow *main,
                         char *homepath, int w, int h,
                         unsigned long options) :
  OXTransientFrame(p, main, w, h, options) {

  ChangeOptions(VERTICAL_FRAME);

  _mainWindow = main;

  _bookmarks = new TDDLList<OBookmarkList*>();
  _currentBmList = NULL;

  _l1 = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y, 2, 2, 2, 2);
  _l2 = new OLayoutHints(LHINTS_EXPAND_X, 2, 2, 2, 2);
  _l3 = new OLayoutHints(LHINTS_EXPAND_Y, 2, 2, 2, 2);
  _l4 = new OLayoutHints(LHINTS_TOP | LHINTS_RIGHT, 2, 2, 2, 2);
  _l5 = new OLayoutHints(LHINTS_EXPAND_Y | LHINTS_RIGHT, 2, 2, 16, 2);
  _l6 = new OLayoutHints(LHINTS_NORMAL, 4, 4, 2, 2);
  _l7a = new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 4, 4, 2, 2);
  _l7b = new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 4, 4, 8, 2);
  _l8 = new OLayoutHints(LHINTS_NORMAL, 2, 2, 2, 2);

  _frame1 = new OXCompositeFrame(this, 1, 1, HORIZONTAL_FRAME);
  _toolBarSep = new OXHorizontal3dLine(this);
  _fbutton = new OXCompositeFrame(this, 1, 1, HORIZONTAL_FRAME | FIXED_WIDTH);

  _frame4 = new OXCompositeFrame(_frame1, 1, 1, VERTICAL_FRAME);
  _lt = new OXListTree(_frame4, 150, 200, -1, SUNKEN_FRAME | DOUBLE_BORDER);
  _lt->Associate(this);
  _frame4->AddFrame(new OXLabel(_frame4, new OString("Folders:")), _l6);
  _frame4->AddFrame(_lt, _l3);

  _frame2 = new OXCompositeFrame(_frame1, 200, 150, VERTICAL_FRAME);
  _lv = new OXListView(_frame2, 250, 150, -1);
  _lv->Associate(this);
  ////(_lv->GetViewPort())->SetBackgroundColor(_whitePixel);
  _lv->SetViewMode(LV_LIST /*LV_SMALL_ICONS*/);
  _gfdetails = new OXGroupFrame(_frame2, new OString("Details"));

  _lhost = new OXLabel(_gfdetails, new OString(""));
  _ldetails = new OXLabel(_gfdetails, new OString(""));
  _gfdetails->AddFrame(_lhost, _l8);
  _gfdetails->AddFrame(_ldetails, _l8);

  _frame2->AddFrame(new OXLabel(_frame2, new OString("Bookmarks:")), _l6);
  _frame2->AddFrame(_lv, _l1);
  _frame2->AddFrame(_gfdetails, _l2);
  
  _frame3 = new OXCompositeFrame(_frame1, 1, 1, VERTICAL_FRAME);
  _bnewbm = new OXTextButton(_frame3, new OHotString("&New Bookmark"), M_NEW_BOOKMARK);
  _beditbm = new OXTextButton(_frame3, new OHotString("&Edit Bookmark"), M_EDIT_BOOKMARK);
  _bcopybm = new OXTextButton(_frame3, new OHotString("&Copy Bookmark"), M_COPY_BOOKMARK);
  _bdeletebm = new OXTextButton(_frame3, new OHotString("&Delete Bookmark"), M_DELETE_BOOKMARK);
  _bnewfd = new OXTextButton(_frame3, new OHotString("New &Folder"), M_NEW_FOLDER);
  _brenamefd = new OXTextButton(_frame3, new OHotString("&Rename Folder"), M_RENAME_FOLDER);
  _bdeletefd = new OXTextButton(_frame3, new OHotString("Dele&te Folder"), M_DELETE_FOLDER);
  _bnewbm->Associate(this);
  _beditbm->Associate(this);
  _bcopybm->Associate(this);
  _bdeletebm->Associate(this);
  _bnewfd->Associate(this);
  _brenamefd->Associate(this);
  _bdeletefd->Associate(this);
  _frame3->AddFrame(_bnewbm, _l7b);
  _frame3->AddFrame(_beditbm, _l7a);
  _frame3->AddFrame(_bcopybm, _l7a);
  _frame3->AddFrame(_bdeletebm, _l7a);
  _frame3->AddFrame(_bnewfd, _l7b);
  _frame3->AddFrame(_brenamefd, _l7a);
  _frame3->AddFrame(_bdeletefd, _l7a);

  _frame1->AddFrame(_frame4, _l3);
  _frame1->AddFrame(_frame2, _l1);
  _frame1->AddFrame(_frame3, _l5);
 
  _bsave = new OXTextButton(_fbutton, new OHotString("&Save"), M_SAVE_BOOKMARK);
  _bconnect = new OXTextButton(_fbutton, new OHotString("C&onnect"), M_CONNECT);
  _bclose = new OXTextButton(_fbutton, new OHotString("C&lose"), M_CLOSE);
  _bsave->Associate(this);
  _bconnect->Associate(this);
  _bclose->Associate(this);
  _fbutton->AddFrame(_bsave, _l7a);
  _fbutton->AddFrame(_bconnect, _l7a);
  _fbutton->AddFrame(_bclose, _l7a);

  int bwidth = _bsave->GetDefaultWidth();
  bwidth = max(bwidth, _bconnect->GetDefaultWidth());
  bwidth = max(bwidth, _bclose->GetDefaultWidth());

  _fbutton->Resize(bwidth * 3 + 50, _fbutton->GetDefaultHeight());

  AddFrame(_frame1, _l1);
  AddFrame(_toolBarSep, _l2);
  AddFrame(_fbutton, _l4);

  _currentFile = new char[strlen(homepath) + 15];
  sprintf(_currentFile, "%s/bookmarks.ini", homepath);
  ReadFile(_currentFile);

  SetControls();

  SetDefaultAcceptButton(_bsave);
  SetDefaultCancelButton(_bclose);
  SetFocusOwner(_lv);

  SetWindowName("Connect");

  SetMWMHints(MWM_DECOR_ALL | MWM_DECOR_MAXIMIZE | MWM_DECOR_MENU,
              MWM_FUNC_ALL | MWM_FUNC_MAXIMIZE | MWM_FUNC_RESIZE,
              MWM_INPUT_MODELESS);

  MapSubwindows();
  Resize(GetDefaultSize());
}

OXFtpManager::~OXFtpManager() {
  delete _l1;
  if (_currentFile) delete[] _currentFile;
  if (_currentBmList) delete _currentBmList;  /**/
}

void OXFtpManager::SetControls() {
  _bnewfd->Enable();
  if (_lt->GetSelected()) {
    _brenamefd->Enable();
    _bdeletefd->Enable();
    _bnewbm->Enable();
  } else {
    _brenamefd->Disable();
    _bdeletefd->Disable();
    _bnewbm->Disable();
  }
  if (_lv->NumSelected() == 1) {
    _beditbm->Enable();
    _bcopybm->Enable();
    _bconnect->Enable();
  } else {
    _beditbm->Disable();
    _bcopybm->Disable();
    _bconnect->Disable();
  }
  if (_lv->NumSelected() > 0) {
    _bdeletebm->Enable();
  } else {
    _bdeletebm->Disable();
  }
}

void OXFtpManager::OpenWindow() {
  CenterOnParent();
  MapWindow();
  _client->WaitForUnmap(this);
}

int OXFtpManager::CloseWindow() {
  // This window is persistent, so just hide it
  UnmapWindow();
  return False;
}

OBookmarkList *OXFtpManager::FindBookmarkList(char *path) {
  OBookmarkList *blist;

  for (int i = 0; i < _bookmarks->GetSize(); i++) {
    blist = _bookmarks->GetAt(i + 1);
    if (strcmp(blist->GetPath(), path) == 0) {
      return blist;
      break;
    }
  }
  return NULL;
}

void OXFtpManager::UpdateBookmarkList(OBookmarkList *blist) {
  OBookmark *bmark;
  const OPicture *pic = _client->GetPicture("bm-file.xpm", bm_file_xpm);

  _lv->Clear(); //_lv->RemoveAll();
  if (blist) {
    for (int j = 0; j < blist->GetList()->GetSize(); j++) {
      bmark = blist->GetList()->GetAt(j + 1);
      std::vector<OString *> names;
      names.push_back(new OString(bmark->GetName()));
      _lv->AddItem(new OBookmarkItem(_lv, -1, names, bmark, pic, pic));
    }
  }
  _lv->Layout();
  _currentBmList = blist;
  SetControls();
}

int OXFtpManager::ReadFile(char *file) {
  char line[256];
  int rtc;
  FILE *fp;

  if ((fp = fopen(file, "r")) == NULL) return false;
  fclose(fp); 

  OIniFile ini(file, INI_READ);
  OListTreeItem *root;
  root = _lt->GetFirstItem();
  if (root) {
    _lt->DeleteItem(root);
    _bookmarks->Erase();
  }

  do {
    if ((rtc = ini.GetNext(line)) != 1) break;
  } while (strcmp(line, "globals"));

  if (rtc) {
    ini.GetItem("version", line);

    ReadFolder("root", line, &ini);
    root = _lt->AddItem(NULL, line, _client->GetPicture("hdisk.t.xpm"),
                        _client->GetPicture("hdisk.t.xpm"));
    _lt->OpenNode(root);
    ReadSubFolders("root", root, &ini);
  }

  return rtc;
}

int OXFtpManager::ReadSubFolders(char *name, OListTreeItem *parent,
                                 OIniFile *ini) {
  char *p, buf[32], line[256];
  int rtc, item = 1;
  OBookmark *bmark;
  OBookmarkList *blist;
  OListTreeItem *curr;

  TDDLList<OBookmark*> *list = new TDDLList<OBookmark*>();
  do {
    ini->Rewind();
    do {
      if ((rtc = ini->GetNext(line)) != 1) break;
    } while (strcmp(line, name));
    if (rtc) {
      sprintf(buf, "item%d", item);
      p = ini->GetItem(buf, line);
      if (p) {
        if (strncmp(line, "Item", 4) == 0) {
          // parse name
          bmark = new OBookmark(NULL, "", new OSiteConfig());
          ReadBookmark(line, bmark, ini);     
          list->Add(bmark);
        } else if (strncmp(line, "Folder", 5) == 0) {
          // parse name
          ReadFolder(line, buf, ini);
          curr = _lt->AddItem(parent, buf, NULL, NULL); 
          rtc = ReadSubFolders(line, curr, ini);
        }
      }
      item++;
    }
  } while (p && rtc);

  _lt->GetPathnameFromItem(parent, line);
  blist = new OBookmarkList(line, list);
  _bookmarks->Add(blist);

  return rtc;
}

int OXFtpManager::ReadFolder(char *name, char *folder, OIniFile *ini) {
  char *p, line[256];
  int rtc;

  do {
    if ((rtc = ini->GetNext(line)) != 1) break;
  } while (strcmp(line, name));

  if (rtc) {
    p = ini->GetItem("name", line);
    if (p) strcpy(folder, line);
  }

  return true;
}

int OXFtpManager::ReadBookmark(char *name, OBookmark *bmark, OIniFile *ini) {
  char *p, line[PATH_MAX];
  int rtc;

  do {
    if ((rtc = ini->GetNext(line)) != 1) break;
  } while (strcmp(line, name));

  if (rtc) {
    p = ini->GetItem("name", line);
    if (p) {
      bmark->SetName(line);
      p = ini->GetItem("description", line);
      if (p) {
        OTextBuffer tb(50, 20);
        tb.Clear();
        char buf[2];

        buf[1] = '\0';
      
        for (p = line; *p; ++p) {
          if (*p == '\\') {
            ++p;
            if (!*p) {
              break;
            } else if (*p == 'n') {
              tb.Append("\n");
              continue;
            } else if (*p == 'r') {
              continue;
            }
          }
          buf[0] = *p;
          tb.Append(buf);
        }
        bmark->SetDescription(tb.GetString());
      }
      OSiteConfig *info = bmark->GetSiteConfig();
      p = ini->GetItem("site", line);
      if (p)
        info->SetSite(line);
      p = ini->GetItem("login", line);
      if (p)
        info->SetLogin(line);
      p = ini->GetItem("passwd", line);
      if (p && strlen(p) != 0) {
        char *pp = info->Decrypt(line, info->_login);
        info->SetPasswd(pp);
        delete[] pp;
      }
      p = ini->GetItem("ipath", line);
      if (p)
        info->SetInitialPath(line);
      p = ini->GetItem("port", line);
      if (p)
        info->_port = atoi(line);
      p = ini->GetItem("uploadmode", line);
      if (p)
        info->_uploadmode = atoi(line);
      p = ini->GetItem("keepalive", line);
      if (p)
        info->_keepalive = atoi(line);
      p = ini->GetItem("passive", line);
      if (p)
        info->_usepassive = atoi(line);
      p = ini->GetItem("anonymous", line);
      if (p)
        info->_useanonymous = atoi(line);
      p = ini->GetItem("proxy", line);
      if (p)
        info->_useproxy = atoi(line);
      p = ini->GetItem("dolog", line);
      if (p)
        info->_dolog = atoi(line);
      p = ini->GetItem("showall", line);
      if (p)
        info->_showdot = atoi(line);
    }
  }
  return true;
}

int OXFtpManager::WriteFile(char *file) {
  OIniFile ini(file, INI_WRITE);

  ini.PutNext("globals");
  ini.PutItem("version", FOXFTP_VERSION);
  ini.PutNewLine();

  int folders, books;
  folders = books = 1;

  OListTreeItem *root = _lt->GetFirstItem();
  WriteSubFolders("root", root, folders, books, &ini);

  return true;
}

int OXFtpManager::WriteSubFolders(char *name, OListTreeItem *parent,
                                  int &folders, int &books, OIniFile *ini) {
  char buf1[32];
  char tmp[PATH_MAX];
  OListTreeItem *item;
  int folders2, books2;

  folders2 = folders;
  books2 = books;

  // write the folder list
  ini->PutNext(name);
  WriteFolder(name, parent, folders, books, ini);
  _lt->GetPathnameFromItem(parent, tmp);
  OBookmarkList *blist = FindBookmarkList(tmp);
  if (blist) {
    TDDLList<OBookmark *> *list = blist->GetList();
    OBookmark *bmark;
    for (int i = 1; i <= list->GetSize(); i++) {
      bmark = list->GetAt(i);
      // WRITE BOOKMARKS
      sprintf(buf1, "Item%d", books2++);
      ini->PutNext(buf1);
      WriteBookmark(bmark, ini);
    }
  }

  item = parent->firstchild;
  while (item) {
    // WRITE FOLDERS
    sprintf(buf1, "Folder%d", folders2++);
    WriteSubFolders(buf1, item, folders, books, ini);
    item = item->nextsibling;
  }

  return true;
}

int OXFtpManager::WriteFolder(char *name, OListTreeItem *parent, 
                                  int &folders, int &books, OIniFile *ini) {
  char buf1[32], buf2[32];
  char tmp[PATH_MAX];
  int i;

  ini->PutItem("name", parent->text);
  _lt->GetPathnameFromItem(parent, tmp);
  OBookmarkList *blist = FindBookmarkList(tmp);
  if (blist) {
    for (i = 1; i <= blist->GetList()->GetSize(); i++) {
      sprintf(buf1, "item%d", i);
      sprintf(buf2, "Item%d", books++);
      ini->PutItem(buf1, buf2);
    }
  }
  OListTreeItem *item = parent->firstchild;
  while (item) {
    sprintf(buf1, "item%d", i++);
    sprintf(buf2, "Folder%d", folders++);
    ini->PutItem(buf1, buf2);
    item = item->nextsibling;
  }
  ini->PutNewLine();

  return true;
}

int OXFtpManager::WriteBookmark(OBookmark *bmark, OIniFile *ini) {
  char buf[8];

  OSiteConfig *info = bmark->GetSiteConfig();
  ini->PutItem("name", bmark->GetName());

  OTextBuffer tb(50, 20);
  tb.Clear();
  if (bmark->GetDescription()) {
    const char *str = bmark->GetDescription()->GetString();
    char buf[2];

    buf[1] = '\0';
      
    for ( ; *str; ++str) {
      if (*str == '\\') {
        tb.Append("\\\\");
      } else if (*str == '\n') {
        tb.Append("\\n");
      } else if (*str == '\r') {
        continue;
      } else {
        buf[0] = *str;
        tb.Append(buf);
      }
    }
  }
  ini->PutItem("description", (char *) tb.GetString());

  ini->PutItem("site", info->_site);

  if (info->_useanonymous == false) {
    ini->PutItem("login", info->_login);
    if (info->_passwd) {
      char *p = info->Encrypt(info->_passwd, info->_login);
      ini->PutItem("passwd", p);
      delete[] p;
    }
  }

  if (info->_ipath)
    ini->PutItem("ipath", info->_ipath);

  sprintf(buf, "%d", info->_port);
  ini->PutItem("port", buf);

  sprintf(buf, "%d", info->_uploadmode);
  ini->PutItem("uploadmode", buf);

  sprintf(buf, "%d", info->_keepalive);
  ini->PutItem("keepalive", buf);

  sprintf(buf, "%d", info->_usepassive);
  ini->PutItem("passive", buf);

  sprintf(buf, "%d", info->_useanonymous);
  ini->PutItem("anonymous", buf);

  sprintf(buf, "%d", info->_useproxy);
  ini->PutItem("proxy", buf);

  sprintf(buf, "%d", info->_dolog);
  ini->PutItem("dolog", buf);

  sprintf(buf, "%d", info->_showdot);
  ini->PutItem("showall", buf);

  ini->PutNewLine();

  return true;
}

void OXFtpManager::DoSaveBookmarks() {
  WriteFile(_currentFile);
}

void OXFtpManager::DoNewBookmark() {
  int rtc;

  OBookmark *bmark = new OBookmark("New Bookmark", "", new OSiteConfig());
  new OXBookmarkProp(_client->GetRoot(), this, 400, 200, bmark, &rtc); 

  if (rtc && _currentBmList) {
    _currentBmList->GetList()->Add(bmark);
    UpdateBookmarkList(_currentBmList);
  } else {
    delete bmark;
  }
  SetControls();
}

void OXFtpManager::DoEditBookmark() {
  int rtc;

  if (_lv->NumSelected() == 1) {
    std::vector<OItem *> items;
    items = _lv->GetSelectedItems();
    OBookmarkItem *f = (OBookmarkItem *) items[0];
    OBookmark *bmark = f->GetBookmark();
    new OXBookmarkProp(_client->GetRoot(), this, 400, 200, bmark, &rtc);
    UpdateBookmarkList(_currentBmList);
  }
}

void OXFtpManager::DoCopyBookmark() {
  int rtc;

  if (_lv->NumSelected() == 1) {
    std::vector<OItem *> items;
    items = _lv->GetSelectedItems();
    OBookmarkItem *f = (OBookmarkItem *) items[0];
    OBookmark *copy = f->GetBookmark();
    OBookmark *bmark = new OBookmark("New Bookmark",
                               copy->GetDescription(),
                               new OSiteConfig(copy->GetSiteConfig()));
    new OXBookmarkProp(_client->GetRoot(), this, 400, 200, bmark, &rtc);
    if (rtc && _currentBmList) {
      _currentBmList->GetList()->Add(bmark);
      UpdateBookmarkList(_currentBmList);
    } else {
      delete bmark;
    }
  }
}

void OXFtpManager::DoDeleteBookmark() {
  OBookmarkItem *f;
  std::vector<OItem *> items = _lv->GetSelectedItems();
  int i, j;

  for (i = 0; i < items.size(); ++i) {
    f = (OBookmarkItem *) items[i];
    if (f) {
      // remove f from _currentBmList
      TDDLList<OBookmark*> *blist = _currentBmList->GetList();
      OBookmark *bmark = f->GetBookmark();
      for (j = 1; j <= blist->GetSize(); j++) {
        if (bmark == blist->GetAt(j)) {
          blist->Remove(j);
          break;
        }
      }
    }
  }
  UpdateBookmarkList(_currentBmList);
}

void OXFtpManager::DoNewFolder() {
  int rtc;
  char tmp[PATH_MAX];
  OListTreeItem *h;

  strcpy(tmp, "New Folder");
  new OXFolderProp(_client->GetRoot(), this, 400, 200, tmp, &rtc);
  if (rtc) {
    if ((h = _lt->GetSelected()) != NULL && h->parent != NULL) {
      h = _lt->AddItem(h->parent, tmp, NULL, NULL);
    } else {
      h = _lt->AddItem(_lt->GetFirstItem(), tmp, NULL, NULL);
    }
    _lt->OpenNode(h);
    _lt->GetPathnameFromItem(h, tmp);
    OBookmarkList *blist = new OBookmarkList(tmp, new TDDLList<OBookmark*>());
    _bookmarks->Add(blist);
    _client->NeedRedraw(_lt);    
  }
  SetControls();
}

void OXFtpManager::DoRenameFolder() {
  int rtc;
  OListTreeItem *h;
  OBookmarkList *blist;
  char tmp[PATH_MAX], old[PATH_MAX];

  if ((h = _lt->GetSelected()) != NULL) {
    strcpy(tmp, h->text);
    new OXFolderProp(_client->GetRoot(), this, 400, 200, tmp, &rtc);
    if (rtc) {
      _lt->GetPathnameFromItem(h, old);
      h->Rename(tmp);
      _client->NeedRedraw(_lt);
      // must rename all subfolders in the bookmark list - get new name first
      _lt->GetPathnameFromItem(h, tmp);
      int len = strlen(old);
      for (int i = 0; i < _bookmarks->GetSize(); i++) {
        blist = _bookmarks->GetAt(i + 1);
        if (strncmp(blist->GetPath(), old, len) == 0) {
          char *p, line[PATH_MAX];
          p = blist->GetPath() + len;
          strcpy(line, tmp);
          strcat(line, p);
          blist->SetPath(line);
        }
      }
    }
  }
}

void OXFtpManager::DoDeleteFolder() {
  OListTreeItem *h;
  char path[PATH_MAX], line[PATH_MAX];

  if ((h = _lt->GetSelected()) != NULL) {
    OBookmarkList *blist;
    line[0] = '\0';
    do {
      sprintf(path, "/%s%s", h->text, line);
      strcpy(line, path);
      h = h->parent;
    } while (h);
    for (int i = 0; i < _bookmarks->GetSize(); i++) {
      blist = _bookmarks->GetAt(i + 1);
      if (strcmp(blist->GetPath(), path) == 0) {
        _bookmarks->Remove(i + 1);
        break;
      }
    }
    _lt->DeleteItem(_lt->GetSelected());
    UpdateBookmarkList(NULL);
    _client->NeedRedraw(_lt);
  }
}

void OXFtpManager::DoConnect() {
  if (_lv->NumSelected() == 1) {
    std::vector<OItem *> items;

    items = _lv->GetSelectedItems();
    OBookmarkItem *f = (OBookmarkItem *) items[0];
    OBookmark *tmp = f->GetBookmark();

    // SEND A MESSAGE
    OSiteMessage message(SITE_MESSAGE, SITE_CONNECT, tmp->GetSiteConfig());
    SendMessage(_mainWindow, &message);
  }
}

int OXFtpManager::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;

  switch (msg->type) {
    case MSG_LISTTREE:
      _client->NeedRedraw(_lt);
      switch (msg->action) {
        case MSG_CLICK: {
          OListTreeItem *h;
          if ((h = _lt->GetSelected()) != NULL) {
            char tmp[PATH_MAX];
            _lt->GetPathnameFromItem(h, tmp);
	    //_lt->OpenNode(h);
            UpdateBookmarkList(FindBookmarkList(tmp));
          } else {
            SetControls();
          }
          break;
        }
      }
      break;

    case MSG_LISTVIEW:
      switch (msg->action) {
        case MSG_CLICK: 
          if (_lv->NumSelected() == 1) {
            std::vector<OItem *> items;
            items = _lv->GetSelectedItems();
            OBookmarkItem *f = (OBookmarkItem *) items[0];
            OBookmark *tmp = f->GetBookmark();
            char buf[PATH_MAX];
            sprintf(buf, "Host: %s", tmp->GetSiteConfig()->_site);
            _lhost->SetText(new OString(buf));

            if (tmp->GetDescription()) {
              OTextBuffer tb(50, 20);
              tb.Clear();
              const char *str = tmp->GetDescription()->GetString();
              char buf[2];

              buf[1] = '\0';
              for ( ; *str && *str != '\n'; ++str) {
                buf[0] = *str;
                tb.Append(buf);
              }
              if (*str++ == '\n')
                if (*str) tb.Append("...");

              _ldetails->SetText(new OString(tb.GetString()));
            }
            _frame2->Layout();
          }
          SetControls();
          break;

        case MSG_DBLCLICK:
          DoConnect();
          break;
      }
      break;

    case MSG_BUTTON:
      switch (msg->action) {
        case MSG_CLICK:
          switch (wmsg->id) {
            case M_SAVE_BOOKMARK:
              DoSaveBookmarks();
              break;

            case M_CONNECT:
              DoConnect();
              break;

            case M_CLOSE:
              CloseWindow();
              break;

            case M_NEW_BOOKMARK:
              DoNewBookmark();
              break;

            case M_EDIT_BOOKMARK:
              DoEditBookmark();
              break;

            case M_COPY_BOOKMARK:
              DoCopyBookmark();
              break;

            case M_DELETE_BOOKMARK:
              DoDeleteBookmark();
              break;

            case M_NEW_FOLDER:
              DoNewFolder();
              break;

            case M_RENAME_FOLDER:
              DoRenameFolder();
              break;

            case M_DELETE_FOLDER:
              DoDeleteFolder();
              break;

          }
          break;
      }
      break;

    default:
      break;

  }
  return true;
}
