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
#include <sys/types.h>
#include <sys/stat.h>

#include <xclass/utils.h>
#include <xclass/OResourcePool.h>
#include <xclass/OMimeTypes.h>
#include <xclass/OX3dLines.h>
#include <xclass/OXButton.h>
#include <xclass/OXListView.h>
#include <xclass/OXMenu.h>
#include <xclass/OXMsgBox.h>
#include <xclass/OXToolBar.h>
#include <xclass/OXToolBarButton.h>
#include <xclass/OXAboutDialog.h>
#include <xclass/OXStatusBar.h>

#include "TDList.h"
#include "OFile.h"
#include "OFileSystem.h"
#include "OFtpConfig.h"
#include "OFtpMessage.h"
#include "OFtp.h"
#include "OLocal.h"
#include "OXFtpManager.h"
#include "OXSiteView.h"
#include "OXFtpConfig.h"
#include "OXFtpDialogs.h"
#include "OXConnectDialog.h"
#include "OXFtpTransfer.h"

#include "main.h"

static struct _popup file_popup = {
  NULL, {
  { "Ne&w",                  M_FILE_NEW,        0, NULL },
  { "&Open",                 M_FILE_OPEN,       0, NULL },
  { "Op&en With...",         M_FILE_OPEN_WITH,  0, NULL },
  { "",                      -1,                0, NULL },
  { "&Quick Connect...",     M_FILE_QUICK,      0, NULL },
  { "Connec&t...",           M_FILE_CONNECT,    0, NULL },
  { "Reconnect",             M_FILE_RECONNECT,  0, NULL },
  { "D&isconnect",           M_FILE_DISCONNECT, 0, NULL },
  { "&Close",                M_FILE_CLOSE,      0, NULL },
  { "",                      -1,                0, NULL },
  { "C&hange Directory...",  M_FILE_CWD,        0, NULL },
  { "Rena&me",               M_FILE_RENAME,     0, NULL },
  { "P&roperties...",        M_FILE_PROPS,      0, NULL },
  { "",                      -1,                0, NULL },
  { "Down&load",             M_FILE_DOWNLOAD,   0, NULL },
  { "&Upload",               M_FILE_UPLOAD,     0, NULL },
  { "",                      -1,                0, NULL },
  { "E&xit",                 M_FILE_EXIT,       0, NULL },
  { NULL,                    -1,                0, NULL } }
};

static struct _popup edit_popup = {
  NULL, {
  { "&Copy",             M_EDIT_COPY,          0, NULL },
  { "&Paste",            M_EDIT_PASTE,         0, NULL },
  { "",                  -1,                   0, NULL },
  { "Select &All",       M_EDIT_SELECT_ALL,    0, NULL },
  { "&Invert Selection", M_EDIT_INVERT_SELECT, 0, NULL },
  { NULL,                -1,                   0, NULL } }
};

static struct _popup arrange_popup = {
  NULL, {
  { "By &Name",  M_VIEW_ARRANGE_BYNAME, 0, NULL },
  { "By &Type",  M_VIEW_ARRANGE_BYTYPE, 0, NULL },
  { "By &Size",  M_VIEW_ARRANGE_BYSIZE, 0, NULL },
  { "By &Date",  M_VIEW_ARRANGE_BYDATE, 0, NULL },
  { NULL,        -1,                    0, NULL } }
};

static struct _popup view_popup = {
  NULL, {
  { "&Toolbar",            M_VIEW_TOOLBAR,     0, NULL },
  { "Show button l&abels", M_VIEW_SHOW_LABELS, 0, NULL },
  { "Status &Bar",         M_VIEW_STATUSBAR,   0, NULL },
  { "",                    -1,                 0, NULL },
  { "Lar&ge Icons",        M_VIEW_LARGE,       0, NULL },
  { "S&mall Icons",        M_VIEW_SMALL,       0, NULL },
  { "&List",               M_VIEW_LIST,        0, NULL },
  { "&Details",            M_VIEW_DETAIL,      0, NULL },
  { "",                    -1,                 0, NULL },
  { "Arrange &Icons",      M_VIEW_ARRANGE,     0, &arrange_popup },
  { "&Show Dot Files",     M_VIEW_DOT_FILES,   0, NULL },
  { "",                    -1,                 0, NULL },
  { "Sto&p",               M_VIEW_STOP,        0, NULL },
  { "Re&fresh",            M_VIEW_REFRESH,     0, NULL },
  { NULL,                  -1,                 0, NULL } }
};

static struct _popup options_popup = {
  NULL, {
  { "&Session Configuration...", M_OPTIONS_SESSION, 0 },
  { "&Global Configuration...",  M_OPTIONS_GLOBAL,  0 },
  { NULL,                        -1,                0, NULL } }
};

static struct _popup window_popup = {
  NULL, {
  { "&Cascade",           M_WINDOW_CASCADE,      0, NULL },
  { "Tile &Horizontally", M_WINDOW_HORIZONTALLY, 0, NULL },
  { "Tile &Vertically",   M_WINDOW_VERTICALLY,   0, NULL },
  { "",                   -1,                    0, NULL },
  { "Arrange &Icons",     M_WINDOW_ICONS,        0, NULL },
  { NULL,                 -1,                    0, NULL } }
};

static struct _popup help_popup = {
  NULL, {
  { "&Contents...", M_HELP_CONTENTS, MENU_DISABLED, NULL },
  { "&Search...",   M_HELP_SEARCH,   MENU_DISABLED, NULL },
  { "",             -1,              0,             NULL },
  { "&About...",    M_HELP_ABOUT,    0,             NULL },
  { NULL,           -1,              0,             NULL } }
};

#include "icons/tb-connect.xpm"
#include "icons/tb-disconnect.xpm"
#include "icons/tb-quick.xpm"
#include "icons/tb-reconnect.xpm"
#include "icons/tb-bigicons.xpm"
#include "icons/tb-copy.xpm"
#include "icons/tb-delete.xpm"
#include "icons/tb-details.xpm"
#include "icons/tb-list.xpm"
#include "icons/tb-refresh.xpm"
#include "icons/tb-paste.xpm"
#include "icons/tb-prop.xpm"
#include "icons/tb-smicons.xpm"
#include "icons/tb-stop.xpm"
#include "icons/tb-uplevel.xpm"

#include "icons/tbu-disconnect.xpm"
#include "icons/tbu-reconnect.xpm"
#include "icons/tbu-bigicons.xpm"
#include "icons/tbu-copy.xpm"
#include "icons/tbu-delete.xpm"
#include "icons/tbu-details.xpm"
#include "icons/tbu-list.xpm"
#include "icons/tbu-refresh.xpm"
#include "icons/tbu-paste.xpm"
#include "icons/tbu-prop.xpm"
#include "icons/tbu-smicons.xpm"
#include "icons/tbu-stop.xpm"
#include "icons/tbu-uplevel.xpm"

#include "icons/bm-file.xpm"


static SToolBarDataEx ftp_tb[] = {
  { "tb-quick.xpm", tb_quick_xpm, NULL, NULL, NULL, NULL, "Quick",
    "Quick Connect", BUTTON_NORMAL, M_FILE_QUICK, NULL },
  { "tb-connect.xpm", tb_connect_xpm,  NULL, NULL, NULL, NULL,
    "Connect", "Connect To Site", BUTTON_NORMAL, M_FILE_CONNECT, NULL },
  { "tb-reconnect.xpm", tb_reconnect_xpm,  NULL, NULL, "tbu-reconnect.xpm", tbu_reconnect_xpm,
    "Reconnect", "Reconnect Site", BUTTON_NORMAL, M_FILE_RECONNECT, NULL },
  { "tb-disconnect.xpm", tb_disconnect_xpm, NULL, NULL, "tbu-disconnect.xpm", tbu_disconnect_xpm,
    "Disconnect", "Disconnect Site", BUTTON_NORMAL, M_FILE_DISCONNECT,  NULL },
  { "", NULL,  NULL, NULL, NULL, NULL, NULL, NULL, -1,  -1,  NULL },
  { "tb-copy.xpm", tb_copy_xpm, NULL, NULL, "tbu-copy.xpm", tbu_copy_xpm,
    "Copy", "Copy to Clipboard", BUTTON_NORMAL, M_EDIT_COPY,  NULL },
  { "tb-paste.xpm", tb_paste_xpm, NULL, NULL, "tbu-paste.xpm", tbu_paste_xpm,
    "Paste", "Paste from Clipboard", BUTTON_NORMAL, M_EDIT_PASTE, NULL },
  { "", NULL,  NULL, NULL, NULL, NULL, NULL, NULL, -1,  -1,  NULL },
  { "tb-refresh.xpm", tb_refresh_xpm, NULL, NULL, "tbu-refresh.xpm", tbu_refresh_xpm,
    "Refresh", "Refresh List", BUTTON_NORMAL, M_VIEW_REFRESH, NULL },
  { "tb-stop.xpm", tb_stop_xpm, NULL, NULL, "tbu-stop.xpm", tbu_stop_xpm,
    "Stop", "Stop Transfer", BUTTON_NORMAL, M_VIEW_STOP, NULL },
  { "", NULL,  NULL, NULL, NULL, NULL, NULL, NULL, -1,  -1,  NULL },
  { "tb-delete.xpm", tb_delete_xpm, NULL, NULL, "tbu-delete.xpm", tbu_delete_xpm,
    "Delete", "Delete Selected", BUTTON_NORMAL, M_EDIT_DELETE, NULL },
  { "tb-prop.xpm", tb_prop_xpm, NULL, NULL, "tbu-prop.xpm", tbu_prop_xpm,
    "Properties", "Show Properties", BUTTON_NORMAL, M_FILE_PROPS, NULL },
  { "", NULL,  NULL, NULL, NULL, NULL, NULL, NULL, -1,  -1,  NULL },
  { "tb-uplevel.xpm", tb_uplevel_xpm, NULL, NULL, "tbu-uplevel.xpm", tbu_uplevel_xpm,
    "Up", "Up one level", BUTTON_NORMAL, M_UP_ONE_LEVEL, NULL },
  { "", NULL,  NULL, NULL, NULL, NULL, NULL, NULL, -1,  -1,  NULL },
  { "tb-bigicons.xpm", tb_bigicons_xpm, NULL, NULL, "tbu-bigicons.xpm", tbu_bigicons_xpm,
    "Large", "Large icons", BUTTON_STAYDOWN, M_VIEW_LARGE, NULL },
  { "tb-smicons.xpm", tb_smicons_xpm, NULL, NULL, "tbu-smicons.xpm", tbu_smicons_xpm,
    "Small", "Small icons", BUTTON_STAYDOWN, M_VIEW_SMALL, NULL },
  { "tb-list.xpm", tb_list_xpm, NULL, NULL, "tbu-list.xpm", tbu_list_xpm,
    "List", "List icons", BUTTON_STAYDOWN, M_VIEW_LIST, NULL },
  { "tb-details.xpm", tb_details_xpm, NULL, NULL, "tbu-details.xpm", tbu_details_xpm,
    "Details", "Details list", BUTTON_STAYDOWN, M_VIEW_DETAIL, NULL },
  { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, NULL }
};


//----------------------------------------------------------------------

OMimeTypes *MimeTypeList;
char homepath[PATH_MAX];

int main(int argc, char **argv) {
  struct stat statbuf;

  OXClient *clientX = new OXClient;
  // ==!== clientX = new OXFtpClient;

  sprintf(homepath, "%s/.foxftp", getenv("HOME"));
  if (stat(homepath, &statbuf) != 0) {
    if (mkdir(homepath, S_IRWXU) != 0)
      strcpy(homepath, ".");
  }
  MimeTypeList = clientX->GetResourcePool()->GetMimeTypes();
 
  OXFtpMain *mainWindow = new OXFtpMain(clientX->GetRoot(), homepath, 10, 10);
  mainWindow->MoveResize(100, 100, 700, 500);
  mainWindow->SetWMPosition(100, 100);
  mainWindow->MapWindow();

  clientX->Run();

  delete clientX;
  return 0;
}


//----------------------------------------------------------------------

OXFtpMain::OXFtpMain(const OXWindow *p, char *homepath, int w, int h) :
  OXMainFrame(p, w, h, VERTICAL_FRAME) {

  _clean_timer = NULL;
  _homepath = StrDup(homepath);

  _l1 = new OLayoutHints(LHINTS_NORMAL | LHINTS_EXPAND_X);
  _l2 = new OLayoutHints(LHINTS_NORMAL, 0, 4, 0, 0);
  _l3= new OLayoutHints(LHINTS_NORMAL | LHINTS_EXPAND_X); //, 4, 4, 4, 4);
  _l4 = new OLayoutHints(LHINTS_EXPAND_Y | LHINTS_EXPAND_X); //, 2, 2, 2, 2);
  _l5 = new OLayoutHints(LHINTS_BOTTOM | LHINTS_EXPAND_X, 0, 0, 2, 0);

  _menuFile = _MakePopup(&file_popup);
  _menuEdit = _MakePopup(&edit_popup);
  _menuSort = _MakePopup(&arrange_popup);
  _menuView = _MakePopup(&view_popup);
  _menuView->CheckEntry(M_VIEW_TOOLBAR);
  _menuView->CheckEntry(M_VIEW_STATUSBAR);
  _menuOptions = _MakePopup(&options_popup);
  _menuWindow = _MakePopup(&window_popup);
  _menuHelp = _MakePopup(&help_popup);

  _menuFile->Associate(this);
  _menuEdit->Associate(this);
  _menuView->Associate(this);
  _menuOptions->Associate(this);
  _menuWindow->Associate(this);
  _menuHelp->Associate(this);

  _menuBar = new OXMdiMenuBar(this, 10, 10);
  _mainFrame = new OXMdiMainFrame(this, _menuBar, 10, 10);

  _menuBar->AddPopup(new OHotString("&File"), _menuFile, _l2);
  _menuBar->AddPopup(new OHotString("&Edit"), _menuEdit, _l2);
  _menuBar->AddPopup(new OHotString("&View"), _menuView, _l2);
  _menuBar->AddPopup(new OHotString("&Options"), _menuOptions, _l2);
  _menuBar->AddPopup(new OHotString("&Window"), _menuWindow, _l2);
  _menuBar->AddPopup(new OHotString("&Help"), _menuHelp, _l2);

  _menuWindow->AddSeparator();
  _menuWindow->AddPopup(new OHotString("Windows"), _mainFrame->GetWinListMenu());

  _showLabels = False;

  _toolBarSep = new OXHorizontal3dLine(this);
  _toolBar = new OXToolBar(this);
  _toolBar->AddButtons(ftp_tb, _showLabels);

  _statusBar = new OXStatusBar(this);

  AddFrame(_menuBar, _l1);
  AddFrame(_toolBarSep, _l1);
  AddFrame(_toolBar, _l3);
  AddFrame(_mainFrame, _l4);
  AddFrame(_statusBar, _l5);

  _bmpic = _client->GetPicture("bm-file.xpm", bm_file_xpm);

  _manager = new OXFtpManager(_client->GetRoot(), this, _homepath, 1, 1);
  _config = new OXFtpConfig(_client->GetRoot(), this, _homepath, 1, 1);
  _genConfig = _config->GetGeneralConfig();
  _quickConfig = new OSiteConfig();
  _quickConfig->_dolog = true;

  OSiteConfig siteConfig;
  siteConfig.SetSite("localhost");
  siteConfig._dolog = false;
  _local = new OXSiteView(_mainFrame, MimeTypeList, new OLocal(), 520, 350);
  _local->SetWindowIcon(_bmpic);
  _local->SetGeneralConfig(_genConfig);
  _local->SetSiteConfig(&siteConfig);
  _local->Layout();
  _local->Connect();
  _remote = NULL;

  SetWindowName("fOX FTP V"FOXFTP_VERSION);
  SetClassHints("foxftp", "foxftp");
  _statusBar->SetText(0, new OString("fOX FTP V"FOXFTP_VERSION));

  SetViewById(M_VIEW_LARGE, _local);
  SetSortById(M_VIEW_ARRANGE_BYTYPE, _local);

  MapSubwindows();
  Layout();
}

OXFtpMain::~OXFtpMain() {
  if (_homepath) delete[] _homepath;
  if (_quickConfig) delete _quickConfig;
  delete _l1; delete _l2; delete _l3; delete _l4; delete _l5;
  delete _menuFile; delete _menuEdit; delete _menuView; delete _menuSort;
  delete _menuOptions; delete _menuWindow; delete _menuHelp;
  _client->FreePicture(_bmpic);
}

int OXFtpMain::HandleTimer(OTimer *t) {
  if (t != _clean_timer)
    return false;

  delete _clean_timer;
  _clean_timer = NULL;
  _statusBar->SetText(0, new OString(""));
  _client->FlushRedraw();
  return true;
}

OXPopupMenu *OXFtpMain::_MakePopup(struct _popup *p) {
 
  OXPopupMenu *popup = new OXPopupMenu(_client->GetRoot());
 
  for (int i = 0; p->popup[i].name != NULL; ++i) {
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

void OXFtpMain::SetViewByMode(int new_mode) {
  int i, lv, bnum;
  switch (new_mode) {
    default: new_mode = LV_LARGE_ICONS;
    case LV_LARGE_ICONS:  bnum = 16; lv = M_VIEW_LARGE;  break;
    case LV_SMALL_ICONS:  bnum = 17; lv = M_VIEW_SMALL;  break;
    case LV_LIST:         bnum = 18; lv = M_VIEW_LIST;   break;
    case LV_DETAILS:      bnum = 19; lv = M_VIEW_DETAIL; break;
  }
  _menuView->RCheckEntry(lv, M_VIEW_LARGE, M_VIEW_DETAIL);
  for (i = 16; i <= 19; ++i)
    ftp_tb[i].button->SetState((i == bnum) ? BUTTON_ENGAGED : BUTTON_UP);
}

void OXFtpMain::SetSortByMode(int new_mode) {
  int smode;
  switch (new_mode) {
    default: new_mode = SORT_BY_NAME;
    case SORT_BY_NAME: smode = M_VIEW_ARRANGE_BYNAME; break;
    case SORT_BY_TYPE: smode = M_VIEW_ARRANGE_BYTYPE; break;
    case SORT_BY_DATE: smode = M_VIEW_ARRANGE_BYDATE; break;
    case SORT_BY_SIZE: smode = M_VIEW_ARRANGE_BYSIZE; break;
  }
  _menuSort->RCheckEntry(smode, M_VIEW_ARRANGE_BYNAME, M_VIEW_ARRANGE_BYDATE);
}

void OXFtpMain::SetViewById(int id, OXSiteView *site) {
  int i, lv, bnum;
  switch (id) {
    default: id = M_VIEW_LARGE;
    case M_VIEW_LARGE:  bnum = 16; lv = LV_LARGE_ICONS; break;
    case M_VIEW_SMALL:  bnum = 17; lv = LV_SMALL_ICONS; break;
    case M_VIEW_LIST:   bnum = 18; lv = LV_LIST;        break;
    case M_VIEW_DETAIL: bnum = 19; lv = LV_DETAILS;     break;
  }
  _menuView->RCheckEntry(id, M_VIEW_LARGE, M_VIEW_DETAIL);
  for (i = 16; i <= 19; ++i)
    ftp_tb[i].button->SetState((i == bnum) ? BUTTON_ENGAGED : BUTTON_UP);

  if (site && lv != site->GetViewMode())
    site->SetViewMode(lv);
}

void OXFtpMain::SetSortById(int id, OXSiteView *site) {
  int smode;
  switch (id) {
    default: id = M_VIEW_ARRANGE_BYNAME;
    case M_VIEW_ARRANGE_BYNAME: smode = SORT_BY_NAME; break;
    case M_VIEW_ARRANGE_BYTYPE: smode = SORT_BY_TYPE; break;
    case M_VIEW_ARRANGE_BYDATE: smode = SORT_BY_DATE; break;
    case M_VIEW_ARRANGE_BYSIZE: smode = SORT_BY_SIZE; break;
  }
  _menuSort->RCheckEntry(id, M_VIEW_ARRANGE_BYNAME, M_VIEW_ARRANGE_BYDATE);
  if (site && smode != site->GetSortMode()) {
    site->SetSortMode(smode);
  }
}

void OXFtpMain::SetControls() {
  OXSiteView *site = (OXSiteView *)_mainFrame->GetCurrent();

  _menuEdit->DisableEntry(M_EDIT_COPY);
  ftp_tb[5].button->Disable();

  _menuView->DisableEntry(M_VIEW_STOP);
  ftp_tb[9].button->Disable();

  _menuView->EnableEntry(M_VIEW_REFRESH);
  ftp_tb[8].button->Enable();

  _menuFile->EnableEntry(M_FILE_CWD);
  ftp_tb[14].button->Enable();

  if (site == NULL || site == _local) {
    _menuFile->DisableEntry(M_FILE_RECONNECT);
    _menuFile->DisableEntry(M_FILE_DISCONNECT);
    _menuFile->DisableEntry(M_FILE_CLOSE);
    ftp_tb[2].button->Disable();
    ftp_tb[3].button->Disable();
  } else {
    _menuFile->EnableEntry(M_FILE_RECONNECT);
    _menuFile->EnableEntry(M_FILE_CLOSE);
    ftp_tb[2].button->Enable();
    if (site->IsConnected()) {
      _menuFile->EnableEntry(M_FILE_DISCONNECT);
      ftp_tb[3].button->Enable();
    } else {
      _menuFile->DisableEntry(M_FILE_DISCONNECT);
      ftp_tb[3].button->Disable();
      _menuView->DisableEntry(M_VIEW_REFRESH);
      ftp_tb[8].button->Disable();
      _menuFile->DisableEntry(M_FILE_CWD);
      ftp_tb[14].button->Disable();
    }
  }
}

void OXFtpMain::DoQuickConnect() {
  int rtc; 

  new OXConnectDialog(_client->GetRoot(), this, 400, 200, &rtc, _quickConfig);
  if (rtc == 0) return;
  DoConnect(_quickConfig);
}

void OXFtpMain::DoConnect(OSiteConfig *siteConfig) {
  _remote = new OXSiteView(_mainFrame, MimeTypeList, new OFtp(), 520, 350);
  _remote->SetWindowIcon(_bmpic);
  _remote->SetGeneralConfig(_genConfig);
  _remote->SetSiteConfig(siteConfig);
  _remote->Layout();
  _remote->Connect();
}

void OXFtpMain::DoReConnect() {
  OXMdiFrame *curr = _mainFrame->GetCurrent();
  if (curr) {
    OXSiteView *site = (OXSiteView *) curr;
    site->Reconnect();
  }
  SetControls();
}

void OXFtpMain::DoDisConnect() {
  OXMdiFrame *curr = _mainFrame->GetCurrent();

  if (curr) {
    OXSiteView *site = (OXSiteView *) curr;
    site->Disconnect(true);
    if (_genConfig->_close_disconnect)
      site->CloseWindow();  // delete site;
  }
  SetControls();
}

void OXFtpMain::DoSetDirectory() {
  int rtc;
  char buf[PATH_MAX];
  OXMdiFrame *curr = _mainFrame->GetCurrent();

  if (curr) {
    OXSiteView *site = (OXSiteView *) curr;
    new OXSetDirectory(_client->GetRoot(), this, 400, 200, buf, &rtc);
    if (rtc)
      site->ChangeDirectory(buf);
  }
}

void OXFtpMain::DoStartFtp(OXSiteView *from, OXSiteView *to) {

  if (from == NULL || to == NULL || from == to) return;

  TDDLList<ODir *> *list = from->CreateFileList();
  new OXFtpTransfer(_client->GetRoot(), this, 100, 100, from, to, list);
}

int OXFtpMain::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;
  OXSiteView *site = (OXSiteView *) _mainFrame->GetCurrent();

  switch (msg->type) {
    case MSG_BUTTON:
    case MSG_MENU:
      switch (msg->action) {
        case MSG_CLICK:
          switch (wmsg->id) {
            case M_HELP_ABOUT: {
                OAboutInfo info;

                info.wname = "About fOX FTP";
                info.title = "fOX FTP Client V"FOXFTP_VERSION;
                info.copyright = "Copyright © 1998-1999 by G. Matzka";
                info.text = "This program is free software; you can redistribute it "
                            "and/or modify it under the terms of the GNU "
                            "General Public License.\n\n"
                            "mgerald@foxproject.org";
                new OXAboutDialog(_client->GetRoot(), this, &info);
              }
              break;

            case M_OPTIONS_SESSION:
              if (site) {
                int rtc;
                char buf[256];

                OSiteConfig *config = new OSiteConfig(site->GetSiteConfig());
                sprintf(buf, "This is an %sactive connection to '%s'",
                            site->IsConnected() ? "" : "in", config->_site);
                OBookmark *bmark = new OBookmark(config->_site, buf, config);
                new OXBookmarkProp(_client->GetRoot(), this, 1, 1, bmark, &rtc);

                if (rtc) {
                  rtc = ID_YES;
                  if (site->IsConnected()) {
                    new OXMsgBox(_client->GetRoot(), this, new OString("fOX Ftp"),
			   new OString("You are about to change the configuration\n"
                                       "of a connected site. Continue?"),
			   MB_ICONQUESTION, ID_YES | ID_NO, &rtc);
                  }
                  if (rtc == ID_YES)
                    site->SetSiteConfig(config);
                }

                delete bmark;
              }
              break;

            case M_OPTIONS_GLOBAL:
              _config->OpenWindow();
              break;

            case M_FILE_DOWNLOAD:
              _remote = (OXSiteView *) _mainFrame->GetCurrent();
              DoStartFtp(_remote, _local);
              break;

            case M_FILE_UPLOAD:
              _remote = (OXSiteView *) _mainFrame->GetCurrent();
              DoStartFtp(_local, _remote);
              break;

            case M_FILE_QUICK:
              DoQuickConnect();
              break;

            case M_FILE_CONNECT:
              _manager->OpenWindow();
              break;

            case M_FILE_RECONNECT:
              DoReConnect();
              break;

            case M_FILE_DISCONNECT:
              DoDisConnect();
              break;

            case M_UP_ONE_LEVEL:
              if (site) site->ChangeDirectory("..");
              break;

            case M_FILE_PROPS:
              if (site) site->ShowProperties();
              break;

            case M_FILE_CLOSE:
#if 0
              if (site) site->CloseWindow();
#else
              _mainFrame->Close(site);
#endif
              break;

            case M_FILE_EXIT:
              CloseWindow();
              break;

            case M_WINDOW_HORIZONTALLY:
              _mainFrame->TileHorizontal();
              break;

            case M_WINDOW_VERTICALLY:
              _mainFrame->TileVertical();
              break;

            case M_WINDOW_CASCADE:
              _mainFrame->Cascade();
              break;

            case M_WINDOW_ICONS:
              _mainFrame->ArrangeMinimized();
              break;

            case M_EDIT_DELETE:
              if (site) site->DoDelete();
              break;

            case M_FILE_CWD:
              DoSetDirectory();
              break;

            case M_EDIT_SELECT_ALL:
              if (site) site->SelectAll();
              break;

            case M_EDIT_INVERT_SELECT:
              if (site) site->InvertSelection();
              break;

            case M_VIEW_TOOLBAR:
              if (_toolBar->IsVisible()) {
                HideFrame(_toolBar);
                HideFrame(_toolBarSep);
                _menuView->UnCheckEntry(M_VIEW_TOOLBAR);
                _menuView->DisableEntry(M_VIEW_SHOW_LABELS);
              } else {
                ShowFrame(_toolBarSep);
                ShowFrame(_toolBar);
                _menuView->CheckEntry(M_VIEW_TOOLBAR);
                _menuView->EnableEntry(M_VIEW_SHOW_LABELS);
              }
              break;

            case M_VIEW_SHOW_LABELS: {
                int i;

                _showLabels = !_showLabels;

                if (_showLabels)
                  _menuView->CheckEntry(M_VIEW_SHOW_LABELS);
                else
                  _menuView->UnCheckEntry(M_VIEW_SHOW_LABELS);

                for (i = 0; ftp_tb[i].pixmap_name != NULL; ++i) {
                  if (ftp_tb[i].button)
                    ftp_tb[i].button->ShowLabel(_showLabels);
                }
                // this is a kludge...
                _toolBar->Resize(_toolBar->GetDefaultWidth(), 1);
                Layout();
              }
              break;

            case M_VIEW_STATUSBAR:
              if (_statusBar->IsVisible()) {
                HideFrame(_statusBar);
                _menuView->UnCheckEntry(M_VIEW_STATUSBAR);
              } else {
                ShowFrame(_statusBar);
                _menuView->CheckEntry(M_VIEW_STATUSBAR);
              }
              break;

            case M_VIEW_ARRANGE_BYNAME:
            case M_VIEW_ARRANGE_BYTYPE:
            case M_VIEW_ARRANGE_BYSIZE:
            case M_VIEW_ARRANGE_BYDATE:
              SetSortById(wmsg->id, site);
              break;

            case M_VIEW_LARGE:
            case M_VIEW_SMALL:
            case M_VIEW_LIST:
            case M_VIEW_DETAIL:
              SetViewById(wmsg->id, site);
              break;

            case M_VIEW_REFRESH:
              if (site) site->DisplayDirectory(true);
              break;

            case M_VIEW_DOT_FILES:
              if (site) {
                OSiteConfig *info = site->GetSiteConfig();
                if (info) {
                  if (info->_showdot) {
                    info->_showdot = false;
                    _menuView->UnCheckEntry(M_VIEW_DOT_FILES);
                  } else {
                    info->_showdot = true;
                    _menuView->CheckEntry(M_VIEW_DOT_FILES);
                  }
                  site->ChangeDirectory(".");
                }
              }
              break;

            default:
              _mainFrame->SetCurrent(wmsg->id);
              break;
          }
          break;
      }
      break;

    case SITE_MESSAGE:
      switch (msg->action) {
        case SITE_CONNECT:
          OSiteMessage *m = (OSiteMessage *) msg;
          DoConnect(m->info);
          break;
      }
      break;

    case STATUS_MESSAGE:
      switch (msg->action) {
        case STATUS_CHANGED:
          OStatusMessage *m = (OStatusMessage *) msg;
          _statusBar->SetText(0, new OString(m->str));
          _client->FlushRedraw();
          if (_clean_timer) delete _clean_timer;
          _clean_timer = new OTimer(this, 5000);
          break;
      }
      break;

    case FTP_TRANSFER:
      switch (msg->action) {
        case FTP_START:
          OXSiteView *from = ((OTransferMessage *)msg)->from;
          OXSiteView *to = ((OTransferMessage *)msg)->to;
          if (from != NULL && to != NULL) {
            if (from == to) {
              int rtc;
              new OXMsgBox(_client->GetRoot(), this, new OString("fOX Ftp"),
			   new OString("Source and target are the same!"),
			   MB_ICONEXCLAMATION, ID_OK, &rtc);
            } else {
              DoStartFtp(from, to);
            }
          }
          break;
      }
      break;
      
    case MSG_MDI:
      switch (msg->action) {
        case MDI_CURRENT:
          SetControls();
          if (site) {
            SetViewByMode(site->GetViewMode());
            SetSortByMode(site->GetSortMode());
            OSiteConfig *info = site->GetSiteConfig();
            if (info && info->_showdot) {
              _menuView->CheckEntry(M_VIEW_DOT_FILES);
            } else {
              _menuView->UnCheckEntry(M_VIEW_DOT_FILES);
            }
          } else {
            SetViewById(M_VIEW_LARGE, NULL);
            SetSortById(M_VIEW_ARRANGE_BYNAME, NULL);
          }
          break;
      }
      break;
  }

  return true;
}
