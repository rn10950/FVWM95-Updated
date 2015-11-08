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

#include <xclass/OXTextEntry.h>
#include <xclass/OXLabel.h>
#include <xclass/OXIcon.h>
#include <xclass/OPicture.h>
#include <xclass/OX3dLines.h>
#include <xclass/OXTextButton.h>
#include <xclass/OXCheckButton.h>
#include <xclass/OXSecretTextEntry.h>
#include <xclass/O2ColumnsLayout.h>

#include "OFtpConfig.h"
#include "OXConnectDialog.h"

#include "icons/connect-dialog.xpm"


//----------------------------------------------------------------------

OXConnectDialog::OXConnectDialog(const OXWindow *p, const OXWindow *main,
                         int w, int h, int *ret_val, OSiteConfig *ret_site,
                         unsigned long options) :
  OXTransientFrame(p, main, w, h, options) {

  char buf[16];
  _ret_val = ret_val;
  *_ret_val = false;
  _ret_site = ret_site;

  _l1 = new OLayoutHints(LHINTS_NORMAL, 3, 3, 3, 3);
  _l2 = new OLayoutHints(LHINTS_CENTER_X | LHINTS_CENTER_Y, 5, 5, 0, 0);
  _l3 = new OLayoutHints(LHINTS_TOP | LHINTS_RIGHT, 3, 3, 6, 3);
  _l4 = new OLayoutHints(LHINTS_NORMAL, 0, 0, 2, 3);
  _l5 = new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X);
  _l6 = new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 5, 0, 0, 0);
  _l7 = new OLayoutHints(LHINTS_TOP | LHINTS_RIGHT, 0, 5, 0, 0);

  _frame = new OXCompositeFrame(this, 1, 1);
  _allFrame = new OXCompositeFrame(_frame, 1, 1, HORIZONTAL_FRAME);
  _connectPicture = _client->GetPicture("connect-dialog.xpm",
                                        connect_dialog_xpm);

  _connectIcon = new OXIcon(_allFrame, _connectPicture, 32, 32);

  _infoFrame = new OXCompositeFrame(_allFrame, 1, 1);
  _infoFrame->SetLayoutManager(new O2ColumnsLayout(_infoFrame, 5, 1));

  _siteLabel = new OXLabel(_infoFrame, new OString("Site: "));
  _siteEntry = new OXTextEntry(_infoFrame, new OTextBuffer(128), 10);

  _loginLabel = new OXLabel(_infoFrame, new OString("Login: "));
  _loginEntry = new OXTextEntry(_infoFrame, new OTextBuffer(128), 11);

  _passwdLabel = new OXLabel(_infoFrame, new OString("Password: "));
  _passwdEntry = new OXSecretTextEntry(_infoFrame, new OTextBuffer(128), 12);

  _portLabel = new OXLabel(_infoFrame, new OString("Port: "));
  _portEntry = new OXTextEntry(_infoFrame, new OTextBuffer(128), 13);

  _siteEntry->Resize(150, _siteEntry->GetDefaultHeight());
  _loginEntry->Resize(150, _loginEntry->GetDefaultHeight());
  _passwdEntry->Resize(150, _passwdEntry->GetDefaultHeight());
  _portEntry->Resize(150, _portEntry->GetDefaultHeight());

  _siteEntry->Associate(this);
  _loginEntry->Associate(this);
  _passwdEntry->Associate(this);
  _portEntry->Associate(this);

  _infoFrame->AddFrame(_siteLabel, NULL);
  _infoFrame->AddFrame(_siteEntry, NULL);
  _infoFrame->AddFrame(_loginLabel, NULL);
  _infoFrame->AddFrame(_loginEntry, NULL);
  _infoFrame->AddFrame(_passwdLabel, NULL);
  _infoFrame->AddFrame(_passwdEntry, NULL);
  _infoFrame->AddFrame(_portLabel, NULL);
  _infoFrame->AddFrame(_portEntry, NULL);

  _allFrame->AddFrame(_connectIcon, _l2);
  _allFrame->AddFrame(_infoFrame, _l1);

  _buttonFrame = new OXCompositeFrame(this, 1, 1, HORIZONTAL_FRAME | FIXED_WIDTH);
  _tbOk = new OXTextButton(_buttonFrame, new  OHotString("&OK"), 1);
  _tbCancel = new OXTextButton(_buttonFrame, new  OHotString("&Cancel"), 2);
  _tbOk->Associate(this);
  _tbCancel->Associate(this);
  _buttonFrame->AddFrame(_tbOk, _l6);
  _buttonFrame->AddFrame(_tbCancel, _l6);

  int width = max(_tbOk->GetDefaultWidth(), _tbCancel->GetDefaultWidth());
  _buttonFrame->Resize((width + 20) * 2, _buttonFrame->GetDefaultHeight());

  SetDefaultAcceptButton(_tbOk);
  SetDefaultCancelButton(_tbCancel);

  OXVerticalFrame *cframe = new OXVerticalFrame(_frame, 10, 10);

  _useAnonymous = new OXCheckButton(cframe,
                      new OHotString("Use &anonymous ftp"), 20);
  _useProxy = new OXCheckButton(cframe, new OHotString("Use &proxy"), 21);
  _useAnonymous->Associate(this);
  _useProxy->Associate(this);
  cframe->AddFrame(_useAnonymous, _l4);
  cframe->AddFrame(_useProxy, _l4);

  _frame->AddFrame(_allFrame, _l1);
  _frame->AddFrame(cframe, _l7);

  if (ret_site->_site)
    _siteEntry->AddText(0, ret_site->_site);

  sprintf(buf, "%d", ret_site->_port);
  _portEntry->AddText(0, buf);

  if (_ret_site->_useproxy)
    _useProxy->SetState(BUTTON_DOWN);

  if (_ret_site->_useanonymous) {
    _useAnonymous->SetState(BUTTON_DOWN);
    _loginEntry->Disable();
    _passwdEntry->Disable();
  } else {
    if (ret_site->_login)
      _loginEntry->AddText(0, ret_site->_login);
    if (ret_site->_passwd)
      _passwdEntry->AddText(0, ret_site->_passwd);
  }

  AddFrame(_frame, _l1);
  AddFrame(new OXHorizontal3dLine(this), _l5);
  AddFrame(_buttonFrame, _l3);

  SetFocusOwner(_siteEntry);

  SetMWMHints(MWM_DECOR_ALL | MWM_DECOR_MAXIMIZE | MWM_DECOR_MENU,
            MWM_FUNC_ALL | MWM_FUNC_MAXIMIZE | MWM_FUNC_RESIZE,
            MWM_INPUT_MODELESS);

  MapSubwindows();
  Resize(GetDefaultSize());
  CenterOnParent();
  SetWindowName("Quick Connect");
  MapWindow();

  _client->WaitFor(this);
}
 
OXConnectDialog::~OXConnectDialog() {
   _client->FreePicture(_connectPicture);
  delete _l1; delete _l2; delete _l3; delete _l4;
  delete _l5; delete _l6; delete _l7;
}
 
int OXConnectDialog::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;

  switch (msg->type) {
    case MSG_BUTTON:
      switch (msg->action) {
        case MSG_CLICK:
          switch (wmsg->id) {
            case 1: {
              *_ret_val = true;
              _ret_site->SetSite((char*)_siteEntry->GetString());
              _ret_site->SetLogin((char*)_loginEntry->GetString());
              _ret_site->SetPasswd((char*)_passwdEntry->GetString());
              _ret_site->_port = atoi(_portEntry->GetString());
              CloseWindow();
              break;
	    }

            case 2:
              *_ret_val = false;
              CloseWindow();
              break;
          }
          break;
      }
      break;

    case MSG_CHECKBUTTON:
    case MSG_RADIOBUTTON:
      switch (msg->action) {
        case MSG_CLICK:
          switch (wmsg->id) {
            case 20:
              if (_useAnonymous->GetState() == BUTTON_DOWN) {
                _loginEntry->Clear();
                _passwdEntry->Clear();
                _loginEntry->Disable();
                _passwdEntry->Disable();
                _ret_site->_useanonymous = true;
              } else {
                if (_ret_site->_login)
                  _loginEntry->AddText(0, _ret_site->_login);
                if (_ret_site->_passwd)
                  _passwdEntry->AddText(0, _ret_site->_passwd);
                _loginEntry->Enable();
                _passwdEntry->Enable();
                _ret_site->_useanonymous = false;
              }
              break;

            case 21:
              if (_useProxy->GetState() == BUTTON_DOWN) {
                _ret_site->_useproxy = true;
              } else {
                _ret_site->_useproxy = false;
              }
              break;
          }
          break;
      }
      break;

  }

  return True;
}
