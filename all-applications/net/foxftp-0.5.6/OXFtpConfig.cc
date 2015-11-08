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

#include <xclass/ODimension.h>
#include <xclass/OIniFile.h>
#include <xclass/OLayout.h>
#include <xclass/OMimeTypes.h>
#include <xclass/OX3dLines.h>
#include <xclass/OXButton.h>
#include <xclass/OXTextEntry.h>
#include <xclass/OXFSDDListBox.h>
#include <xclass/OXTab.h>
#include <xclass/OXLabel.h>
#include <xclass/OXGroupFrame.h>
#include <xclass/OXSpinner.h>
#include <xclass/O2ColumnsLayout.h>

#include "OFtpConfig.h"
#include "OXFtpConfig.h"


#define ICON_FOLDER      0
#define ICON_APP         1
#define ICON_DOC         2
#define ICON_LINK        3
 
#define ICON_BIG         0
#define ICON_SMALL       1
 
struct _default_icon {
  char  *picname_prefix;
  const OPicture *icon[2];
};
 
struct _default_icon default_icon[] = {
  { "folder",  { NULL, NULL } },
  { "app",     { NULL, NULL } },
  { "doc",     { NULL, NULL } },
  { "slink",   { NULL, NULL } },
  { NULL,      { NULL, NULL } }
};

extern OMimeTypes *MimeTypeList;


//----------------------------------------------------------------------

OXFtpConfig::OXFtpConfig(const OXWindow *p, const OXWindow *main, 
                char *homepath, int w, int h, unsigned long options) :
  OXTransientFrame(p, main, w, h, options) {

  OXTab *_tab;
  OXCompositeFrame *_frame, *_fbutton, *_subFrame, *_subFrame2;
  OXGroupFrame *_groupFrame;
  OXLabel *_label1, *_label2, *_label3;
  OXTextButton *_bsave, *_bapply, *_bclose;

  //--- load default pictures...
 
  for (int i = 0; default_icon[i].picname_prefix != NULL; ++i) {
    char *name;
 
    name = new char[20];
    sprintf(name, "%s.s.xpm", default_icon[i].picname_prefix);
    if ((default_icon[i].icon[0] = _client->GetPicture(name)) == NULL)
      FatalError("Pixmap not found: %s", name);
 
    name = new char[20];
    sprintf(name, "%s.t.xpm", default_icon[i].picname_prefix);
    if ((default_icon[i].icon[1] = _client->GetPicture(name)) == NULL)
      FatalError("Pixmap not found: %s", name);

  }

  _l1 = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_TOP, 0, 0, 5, 0);
  _l2 = new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 3, 3, 1, 1);
  _l3 = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y, 2, 2, 2, 2);
  _l4 = new OLayoutHints(LHINTS_NORMAL, 5, 2, 2, 2);
  _l5 = new OLayoutHints(LHINTS_TOP | LHINTS_RIGHT, 0, 0, 4, 2);
  _l6 = new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 2, 2, 0, 0);

  _tab = new OXTab(this, 10, 10);

  //--- General

  _frame = _tab->AddTab(new OString("General"));
  _groupFrame = new OXGroupFrame(_frame, new OString("Transfer options"));
  _subFrame = new OXCompositeFrame(_groupFrame, 1, 1);
  _subFrame->SetLayoutManager(new O2ColumnsLayout(_subFrame, 5, 4));
  _label1 = new OXLabel(_subFrame, new OString("Transfer type:"));
  _label2 = new OXLabel(_subFrame, new OString("If file exist:"));
  _transferModeLb = new OXDDListBox(_subFrame, 100);
  _overwriteModeLb = new OXDDListBox(_subFrame, 200);
  _transferModeLb->AddEntry(new OString("Ask"), 101);
  _transferModeLb->AddEntry(new OString("ASCII"), 102);
  _transferModeLb->AddEntry(new OString("Binary"), 103);
  _overwriteModeLb->AddEntry(new OString("Ask"), 201);
  _overwriteModeLb->AddEntry(new OString("Overwrite"), 202);
  _transferModeLb->Resize(150, _transferModeLb->GetDefaultHeight());
  _overwriteModeLb->Resize(150, _overwriteModeLb->GetDefaultHeight());
  _transferModeLb->Associate(this);
  _overwriteModeLb->Associate(this);
  _subFrame->AddFrame(_label1, NULL);
  _subFrame->AddFrame(_transferModeLb, NULL);
  _subFrame->AddFrame(_label2, NULL);
  _subFrame->AddFrame(_overwriteModeLb, NULL);
  _groupFrame->AddFrame(_subFrame, _l1);
  _frame->AddFrame(_groupFrame, _l2);

  _groupFrame = new OXGroupFrame(_frame, new OString("Preferences"));
  _disCloseCb = new OXCheckButton(_groupFrame, new OHotString("&Disconnect on close"), 250);
  _closeDisCb = new OXCheckButton(_groupFrame, new OHotString("&Close window on disconnect"), 251);
  _disCloseCb->Associate(this);
  _closeDisCb->Associate(this);
  _subFrame = new OXCompositeFrame(_groupFrame, 1, 1); 
  _subFrame->SetLayoutManager(new O2ColumnsLayout(_subFrame, 5, 4));
  _label1 = new OXLabel(_subFrame, new OString("Anonymous passwd:"));
  _label2 = new OXLabel(_subFrame, new OString("Download directory:"));
  _anoPwdTe = new OXTextEntry(_subFrame, NULL, 300);
  _downDirTe = new OXTextEntry(_subFrame, NULL, 301);
  _anoPwdTe->Resize(150, _anoPwdTe->GetDefaultHeight());
  _downDirTe->Resize(150, _downDirTe->GetDefaultHeight());
  _subFrame->AddFrame(_label1, NULL);
  _subFrame->AddFrame(_anoPwdTe, NULL);
  _subFrame->AddFrame(_label2, NULL);
  _subFrame->AddFrame(_downDirTe, NULL);
  _groupFrame->AddFrame(_disCloseCb, _l4);
  _groupFrame->AddFrame(_closeDisCb, _l4);
  _groupFrame->AddFrame(_subFrame, _l1);
  _frame->AddFrame(_groupFrame, _l2);

  _groupFrame = new OXGroupFrame(_frame, new OString("Site retry"));
  _subFrame = new OXCompositeFrame(_groupFrame, 1, 1);
  _subFrame->SetLayoutManager(new O2ColumnsLayout(_subFrame, 5, 4));
  _label1 = new OXLabel(_subFrame, new OString("Number of retries:"));
  _label2 = new OXLabel(_subFrame, new OString("Delay (sec):"));
  _siteRetrySp = new OXSpinner(_subFrame, "", 350);
  _siteRetrySp->Resize(80, _siteRetrySp->GetDefaultHeight());
  _siteRetrySp->SetPercent(False);
  _siteRetrySp->SetRange(0, 5, 1);
  _retryDelaySp = new OXSpinner(_subFrame, "", 351);
  _retryDelaySp->Resize(80, _retryDelaySp->GetDefaultHeight());
  _retryDelaySp->SetPercent(False);
  _retryDelaySp->SetRange(5, 60, 5);
  _subFrame->AddFrame(_label1, NULL);
  _subFrame->AddFrame(_siteRetrySp, NULL);
  _subFrame->AddFrame(_label2, NULL);
  _subFrame->AddFrame(_retryDelaySp, NULL);
  _groupFrame->AddFrame(_subFrame, _l1);
  _frame->AddFrame(_groupFrame, _l2);

  //--- File Types

  /* ... grabbed from explorer ... */
  _frame = _tab->AddTab(new OString("File Types"));

  _l7 = new OLayoutHints(LHINTS_LEFT | LHINTS_TOP, 10, 10, 10, 0);
  _l8 = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y, 10, 10, 0, 10);
  _l9 = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_BOTTOM, 0, 0, 2, 0);
  _l10 = new OLayoutHints(LHINTS_LEFT | LHINTS_EXPAND_Y, 0, 5, 2, 2);
  _l11 = new OLayoutHints(LHINTS_LEFT | LHINTS_EXPAND_Y, 10, 5, 2, 2);
  _l12 = new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 0, 0, 5, 0);

  _frame->AddFrame(new OXLabel(_frame,
                        new OString("Registered file types:")), _l7);

  _subFrame = new OXHorizontalFrame(_frame, 10, 10);
  _frame->AddFrame(_subFrame, _l8);

  _subFrame2 = new OXVerticalFrame(_subFrame, 10, 10);
  _subFrame->AddFrame(_subFrame2, _l3);
  _subFrame2->AddFrame(_mimeTypeLb = new OXListBox(_subFrame2, 801), _l3);
  _subFrame2->AddFrame(_groupFrame = new OXGroupFrame(_subFrame2,
                       new OString("File type details")), _l9);

  _frame = new OXHorizontalFrame(_groupFrame, 10, 10, FIXED_HEIGHT);
  _subFrame = new OXHorizontalFrame(_groupFrame, 10, 10, FIXED_HEIGHT);

  _mimeIcon = new OXIcon(_frame,
                         default_icon[ICON_APP].icon[ICON_BIG], 32, 32);
  _mimeTypeLa = new OXLabel(_frame, new OString(""));
  _frame->AddFrame(_mimeIcon, _l10);
  _frame->AddFrame(new OXLabel(_frame, new OString("Extension:")), _l11);
  _frame->AddFrame(_mimeTypeLa, _l10);

  _mimeActionLa = new OXLabel(_subFrame, new OString(""));
  _label1 = new OXLabel(_subFrame, new OString("Opens with:"));
  _subFrame->AddFrame(_label1, _l10);
  _subFrame->AddFrame(_mimeActionLa, _l10);

  _frame->Resize(_frame->GetDefaultWidth(), 38);
  _subFrame->Resize(_frame->GetDefaultWidth(), _label1->GetDefaultHeight()+6);

  _groupFrame->AddFrame(_frame, _l12);
  _groupFrame->AddFrame(_subFrame, _l12);

  UpdateMimeTypeLB();
  _mimeTypeLb->Associate(this);
  _mimeTypeLb->Select(500);

  //--- Firewall

  _frame = _tab->AddTab(new OString("Advanced"));
  _groupFrame = new OXGroupFrame(_frame, new OString("Firewall"));
  _subFrame = new OXCompositeFrame(_groupFrame, 1, 1);
  _subFrame->SetLayoutManager(new O2ColumnsLayout(_subFrame, 5, 4));
  _label1 = new OXLabel(_subFrame, new OString("Type:"));
  _proxyModeLb = new OXDDListBox(_subFrame, 400);
  _proxyModeLb->AddEntry(new OString("None"), 401);
  _proxyModeLb->AddEntry(new OString("OPEN host"), 402);
  _proxyModeLb->AddEntry(new OString("USER user@host:port"), 403);
  _proxyModeLb->AddEntry(new OString("USER user@host port"), 404);
  _proxyModeLb->Resize(150, _proxyModeLb->GetDefaultHeight());
  _proxyModeLb->Associate(this);
  _label2 = new OXLabel(_subFrame, new OString("Hostname:"));
  _label3 = new OXLabel(_subFrame, new OString("Port:"));
  _proxyHostTe = new OXTextEntry(_subFrame, NULL, 450);
  _proxyPortTe = new OXTextEntry(_subFrame, NULL, 451);
  _proxyHostTe->Resize(150, _proxyHostTe->GetDefaultHeight());
  _proxyPortTe->Resize(150, _proxyPortTe->GetDefaultHeight());
  _subFrame->AddFrame(_label1, NULL);
  _subFrame->AddFrame(_proxyModeLb, NULL);
  _subFrame->AddFrame(_label2, NULL);
  _subFrame->AddFrame(_proxyHostTe, NULL);
  _subFrame->AddFrame(_label3, NULL);
  _subFrame->AddFrame(_proxyPortTe, NULL);
  _groupFrame->AddFrame(_subFrame, _l1);
  _frame->AddFrame(_groupFrame, _l2);

  _groupFrame = new OXGroupFrame(_frame, new OString("Transfer"));
  _subFrame = new OXCompositeFrame(_groupFrame, 1, 1);
  _subFrame->SetLayoutManager(new O2ColumnsLayout(_subFrame, 5, 4));
  _label1 = new OXLabel(_subFrame, new OString("Receive buffer:"));
  _label2 = new OXLabel(_subFrame, new OString("Send buffer:"));
  _label3 = new OXLabel(_subFrame, new OString("Cache dir:"));
  _recvBufferTe = new OXTextEntry(_subFrame, NULL, 501);
  _sendBufferTe = new OXTextEntry(_subFrame, NULL, 502);
  _cacheDirTe = new OXTextEntry(_subFrame, NULL, 503);
  _recvBufferTe->Resize(150, _recvBufferTe->GetDefaultHeight());
  _sendBufferTe->Resize(150, _sendBufferTe->GetDefaultHeight());
  _cacheDirTe->Resize(150, _cacheDirTe->GetDefaultHeight());
  _subFrame->AddFrame(_label1, NULL);
  _subFrame->AddFrame(_recvBufferTe, NULL);
  _subFrame->AddFrame(_label2, NULL);
  _subFrame->AddFrame(_sendBufferTe, NULL);
  _subFrame->AddFrame(_label3, NULL);
  _subFrame->AddFrame(_cacheDirTe, NULL);
  _groupFrame->AddFrame(_subFrame, _l1);
  _frame->AddFrame(_groupFrame, _l2);

  _fbutton = new OXCompositeFrame(this, 1, 1, HORIZONTAL_FRAME | FIXED_WIDTH);
  _bsave = new OXTextButton(_fbutton, new OHotString("&Save"), 1000);
  _bapply = new OXTextButton(_fbutton, new OHotString("&Apply"), 1001);
  _bclose = new OXTextButton(_fbutton, new OHotString("Cl&ose"), 1002);
  _bsave->Associate(this);
  _bapply->Associate(this);
  _bclose->Associate(this);
  _fbutton->AddFrame(_bsave, _l6);
  _fbutton->AddFrame(_bapply, _l6);
  _fbutton->AddFrame(_bclose, _l6);

  SetDefaultAcceptButton(_bsave);
  SetDefaultCancelButton(_bclose);

  int width = _bsave->GetDefaultWidth();
  width = max(width, _bapply->GetDefaultWidth());
  width = max(width, _bclose->GetDefaultWidth());
  _fbutton->Resize((width + 20) * 3, _fbutton->GetDefaultHeight());

  AddFrame(_tab, _l3);
  AddFrame(_fbutton, _l5);

  _genConfig = new OGeneralConfig;
  _currentFile = new char[strlen(homepath) + 12];
  sprintf(_currentFile, "%s/foxftp.ini", homepath);
  ReadFile(_currentFile);
  SetWidgets();

  SetMWMHints(MWM_DECOR_ALL | MWM_DECOR_MAXIMIZE | MWM_DECOR_MENU,
              MWM_FUNC_ALL | MWM_FUNC_MAXIMIZE | MWM_FUNC_RESIZE,
              MWM_INPUT_MODELESS);
  MapSubwindows();
  Resize(GetDefaultSize());
  SetWindowName("General Options");
}

OXFtpConfig::~OXFtpConfig() {
  delete _l1; delete _l2; delete _l3; delete _l4; delete _l5; delete _l6;
  if (_currentFile) delete[] _currentFile;
}

void OXFtpConfig::OpenWindow() {
  CenterOnParent();
  MapWindow();
  _client->WaitForUnmap(this);
}

int OXFtpConfig::CloseWindow() {
  // This window is persistent, so just hide it
  UnmapWindow();
  return False;
}

void OXFtpConfig::UpdateMimeTypeLB() {
  OMime *MimeRunner;
  int i;

  _mimeTypeLb->RemoveAllEntries();

  _mimeTypeLb->AddEntry(new OXTreeLBEntry(_mimeTypeLb->GetContainer(),
                      new OString("Application"),
                      default_icon[ICON_APP].icon[ICON_SMALL], 500, NULL),
                new OLayoutHints(LHINTS_LEFT | LHINTS_TOP,
                      3, 0, 0, 0));
  _mimeTypeLb->AddEntry(new OXTreeLBEntry(_mimeTypeLb->GetContainer(),
                      new OString("File folder"),
                      default_icon[ICON_FOLDER].icon[ICON_SMALL], 501, NULL),
                new OLayoutHints(LHINTS_LEFT | LHINTS_TOP,
                      3, 0, 0, 0));

  MimeRunner = NULL;
  i = 0;

  while ((MimeRunner = MimeTypeList->NextMime(MimeRunner))) {
    const OPicture *pic;
    char  fname[50];
    OString *descr;

    pic = MimeTypeList->GetIcon(MimeRunner, True);
    if (!pic) pic = default_icon[ICON_DOC].icon[ICON_SMALL];

    if (MimeRunner->description) {
      descr = new OString(MimeRunner->description);
    } else {
      char default_descr[1000];
      // generate default description
      sprintf(default_descr, "Files matching %s", MimeRunner->pattern);
      descr = new OString(default_descr);
    }

    // Fake string value /********/
    sprintf(fname, "%ld", (long) MimeRunner);

    _mimeTypeLb->AddEntry(new OXTreeLBEntry(_mimeTypeLb->GetContainer(),
                        // new OString(ftypes[i].ft), 
                        descr, pic, 503+i, new OString(fname)),
                  new OLayoutHints(LHINTS_LEFT | LHINTS_TOP,
                        3, 0, 0, 0));
    i++;
  }
}

void OXFtpConfig::SetWidgets() {
  char buf[32];

  if (_genConfig->_disconnect_close)
    _disCloseCb->SetState(BUTTON_DOWN);

  if (_genConfig->_close_disconnect)
    _closeDisCb->SetState(BUTTON_DOWN);

  if (_genConfig->_anonymous_pw)
    _anoPwdTe->AddText(0, _genConfig->_anonymous_pw);

  if (_genConfig->_download_dir)
    _downDirTe->AddText(0, _genConfig->_download_dir);

  if (_genConfig->_cache_dir)
    _cacheDirTe->AddText(0, _genConfig->_cache_dir);

  sprintf(buf, "%d", _genConfig->_recvbuffer);
  _recvBufferTe->AddText(0, buf);

  sprintf(buf, "%d", _genConfig->_sendbuffer);
  _sendBufferTe->AddText(0, buf);

  _transferModeLb->Select(_genConfig->_transfer_type + 100);
  _overwriteModeLb->Select(_genConfig->_exist_mode + 200);
  _proxyModeLb->Select(_genConfig->_proxy_mode + 400);

  if (_genConfig->_proxy_mode != 1) {
    if (_genConfig->_proxy)
      _proxyHostTe->AddText(0, _genConfig->_proxy);
    sprintf(buf, "%d", _genConfig->_proxyport);
    _proxyPortTe->AddText(0, buf);
  } else {
    _proxyHostTe->Disable();
    _proxyPortTe->Disable();
  }

  _siteRetrySp->SetValue(_genConfig->_site_retry);
  _retryDelaySp->SetValue(_genConfig->_retry_delay);
}

void OXFtpConfig::GetWidgets() {

  if (_disCloseCb->GetState() == BUTTON_DOWN)
    _genConfig->_disconnect_close = true;
  else
    _genConfig->_disconnect_close = false;

  if (_closeDisCb->GetState() == BUTTON_DOWN)
    _genConfig->_close_disconnect = true;
  else
    _genConfig->_close_disconnect = false;

  _genConfig->_site_retry = (int) _siteRetrySp->GetValue();

  if (_genConfig->_site_retry < 0 || _genConfig->_site_retry > 5)
    _genConfig->_site_retry = 0;

  _genConfig->_retry_delay = (int)_retryDelaySp->GetValue();

  if (_genConfig->_retry_delay < 5 || _genConfig->_retry_delay > 60)
    _genConfig->_retry_delay = 5;

  _genConfig->SetAnonymousPasswd((char*)_anoPwdTe->GetString());
  _genConfig->SetDownloadDir((char*)_downDirTe->GetString());
  _genConfig->SetCacheDir((char*)_cacheDirTe->GetString());

  _genConfig->_transfer_type = _transferModeLb->GetSelected() - 100;
  if (_genConfig->_transfer_type < 1 || _genConfig->_transfer_type > 2) {
    _genConfig->_transfer_type = 1;
  }

  _genConfig->_exist_mode = _overwriteModeLb->GetSelected() - 200;
  if (_genConfig->_exist_mode < 1 || _genConfig->_exist_mode > 2) {
    _genConfig->_exist_mode = 1;
  }

  _genConfig->_proxy_mode = _proxyModeLb->GetSelected() - 400;
  if (_genConfig->_proxy_mode < 1 || _genConfig->_proxy_mode > 4) {
    _genConfig->_proxy_mode = 1;
  }

  if (_genConfig->_proxy_mode == 1) {
    _genConfig->SetProxyHost(NULL);
    _genConfig->_proxyport = 0;
  } else {
    _genConfig->SetProxyHost((char*)_proxyHostTe->GetString());
    _genConfig->_proxyport = atoi(_proxyPortTe->GetString());
  }

  _genConfig->_recvbuffer = atoi(_recvBufferTe->GetString());
  _genConfig->_sendbuffer = atoi(_sendBufferTe->GetString());
}

int OXFtpConfig::ReadFile(char *file) {
  char *p, line[256];
  int rtc;
  FILE *fp;
 
  if ((fp = fopen(file, "r")) == NULL) return false;
  fclose(fp);

  OIniFile ini(file, INI_READ);

  do {
    if ((rtc = ini.GetNext(line)) != 1) break;
  } while (strcmp(line, "foxftpglobals"));

  if (rtc) {
    p = ini.GetItem("proxy", line);
    if (p)
      _genConfig->SetProxyHost(p);
    p = ini.GetItem("anonymouspw", line);
    if (p)
      _genConfig->SetAnonymousPasswd(p);
    p = ini.GetItem("downloaddir", line);
    if (p)
      _genConfig->SetDownloadDir(p);
    p = ini.GetItem("cachedir", line);
    if (p)
      _genConfig->SetCacheDir(p);
    p = ini.GetItem("proxyport", line);
    if (p)
      _genConfig->_proxyport = atoi(p);
    p = ini.GetItem("recvbuffer", line);
    if (p)
      _genConfig->_recvbuffer = atoi(p);
    p = ini.GetItem("sendbuffer", line);
    if (p)
      _genConfig->_sendbuffer = atoi(p);
    p = ini.GetItem("transfer_type", line);
    if (p)
      _genConfig->_transfer_type = atoi(p);
    p = ini.GetItem("exist_mode", line);
    if (p)
      _genConfig->_exist_mode = atoi(p);
    p = ini.GetItem("proxy_mode", line);
    if (p)
      _genConfig->_proxy_mode = atoi(p);
    p = ini.GetItem("disconnect_close", line);
    if (p)
      _genConfig->_disconnect_close = atoi(p);
    p = ini.GetItem("close_disconnect", line);
    if (p)
      _genConfig->_close_disconnect = atoi(p);
    p = ini.GetItem("site_retry", line);
    if (p)
      _genConfig->_site_retry = atoi(p);
    p = ini.GetItem("retry_delay", line);
    if (p)
      _genConfig->_retry_delay = atoi(p);
  }

  return true;
}

int OXFtpConfig::WriteFile(char *file) {
  char buf[256];
  OIniFile ini(file, INI_WRITE);
 
  ini.PutNext("foxftpglobals");

  if (_genConfig->_proxy)
    ini.PutItem("proxy", _genConfig->_proxy);

  if (_genConfig->_anonymous_pw)
    ini.PutItem("anonymouspw", _genConfig->_anonymous_pw);

  if (_genConfig->_download_dir)
    ini.PutItem("downloaddir", _genConfig->_download_dir);

  if (_genConfig->_cache_dir)
    ini.PutItem("cachedir", _genConfig->_cache_dir);

  sprintf(buf, "%d", _genConfig->_proxyport);
  ini.PutItem("proxyport", buf);

  sprintf(buf, "%d", _genConfig->_recvbuffer);
  ini.PutItem("recvbuffer", buf);

  sprintf(buf, "%d", _genConfig->_sendbuffer);
  ini.PutItem("sendbuffer", buf);

  sprintf(buf, "%d", _genConfig->_transfer_type);
  ini.PutItem("transfer_type", buf);

  sprintf(buf, "%d", _genConfig->_exist_mode);
  ini.PutItem("exist_mode", buf);

  sprintf(buf, "%d", _genConfig->_proxy_mode);
  ini.PutItem("proxy_mode", buf);

  sprintf(buf, "%d", _genConfig->_disconnect_close);
  ini.PutItem("disconnect_close", buf);

  sprintf(buf, "%d", _genConfig->_close_disconnect);
  ini.PutItem("close_disconnect", buf);

  sprintf(buf, "%d", _genConfig->_site_retry);
  ini.PutItem("site_retry", buf);

  sprintf(buf, "%d", _genConfig->_retry_delay);
  ini.PutItem("retry_delay", buf);

  ini.PutNewLine();

  return true;
}

int OXFtpConfig::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;
  OListBoxMessage *lbmsg = (OListBoxMessage *) msg;
  char buf[32];

  switch (msg->type) {
    case MSG_BUTTON:
      switch (msg->action) {
        case MSG_CLICK:
          switch (wmsg->id) {
            case 1001:
              GetWidgets();
              CloseWindow();
              break;

            case 1000:
              GetWidgets();
              WriteFile(_currentFile);
              break;

            case 1002:
              CloseWindow();
              break;

          }
          break;
      }
      break;

    case MSG_DDLISTBOX:
      if ((lbmsg->action == MSG_CLICK) && (lbmsg->id == 400)) {
        if (lbmsg->entryID == 401) {
          _proxyHostTe->Disable();
          _proxyPortTe->Disable();
          _proxyHostTe->Clear();
          _proxyPortTe->Clear();
        } else if (_proxyHostTe->IsEnabled()) {
          _proxyHostTe->Enable();
          _proxyPortTe->Enable();
          if (_genConfig->_proxy)
            _proxyHostTe->AddText(0, _genConfig->_proxy);
          sprintf(buf, "%d", _genConfig->_proxyport);
          _proxyPortTe->AddText(0, buf);
        }
      }
      break;
  }

  return true;
}
