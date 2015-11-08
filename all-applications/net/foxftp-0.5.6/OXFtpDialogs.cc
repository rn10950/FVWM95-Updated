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

#include <xclass/OXMenu.h>
#include <xclass/OXScrollBar.h>
#include <xclass/OXTextEntry.h>
#include <xclass/OXLabel.h>
#include <xclass/OXIcon.h>
#include <xclass/OXGroupFrame.h>
#include <xclass/OPicture.h>
#include <xclass/OX3dLines.h>
#include <xclass/OXTextButton.h>
#include <xclass/OXCheckButton.h>
#include <xclass/OXRadioButton.h>
#include <xclass/OXSecretTextEntry.h>
#include <xclass/O2ColumnsLayout.h>

#include "OFtpConfig.h"
#include "OXFtpDialogs.h"

#include "icons/login-key.xpm"


//----------------------------------------------------------------------

OXPasswdDialog::OXPasswdDialog(const OXWindow *p, const OXWindow *main,
                         int w, int h, int *ret_val, OSiteConfig *ret_site,
                         unsigned long options) :
  OXTransientFrame(p, main, w, h, options) {

  _ret_val = ret_val;
  *_ret_val = false;
  _ret_site = ret_site;

  _l1 = new OLayoutHints(LHINTS_NORMAL, 3, 3, 3, 3);
  _l2 = new OLayoutHints(LHINTS_CENTER_X | LHINTS_CENTER_Y, 5, 5, 0, 0);
  _l3 = new OLayoutHints(LHINTS_TOP | LHINTS_RIGHT, 3, 3, 3, 3);
  _l4 = new OLayoutHints(LHINTS_EXPAND_X, 3, 3, 3, 3);

  _frame1 = new OXCompositeFrame(this, 1, 1, HORIZONTAL_FRAME);

  _connectPicture = _client->GetPicture("login-key.xpm", login_key_xpm);
  _connectIcon = new OXIcon(_frame1, _connectPicture, 32, 32);

  _frame2 = new OXCompositeFrame(_frame1, 1, 1);

  _textLabel = new OXLabel(_frame2, 
                   new OString("Enter the password to log on this site"));

  _frame3 = new OXCompositeFrame(_frame2, 1, 1);
  _frame3->SetLayoutManager(new O2ColumnsLayout(_frame3, 5, 1));

  _loginLabel = new OXLabel(_frame3, new OString("Login: "));
  _loginEntry = new OXTextEntry(_frame3, new OTextBuffer(128), 11);

  _passwdLabel = new OXLabel(_frame3, new OString("Password: "));
  _passwdEntry = new OXSecretTextEntry(_frame3, new OTextBuffer(128), 12);

  _loginEntry->Resize(150, _loginEntry->GetDefaultHeight());
  _passwdEntry->Resize(150, _passwdEntry->GetDefaultHeight());
  _loginEntry->Associate(this);
  _passwdEntry->Associate(this);

  _frame3->AddFrame(_loginLabel, NULL);
  _frame3->AddFrame(_loginEntry, NULL);
  _frame3->AddFrame(_passwdLabel, NULL);
  _frame3->AddFrame(_passwdEntry, NULL);

  _frame2->AddFrame(_textLabel, new OLayoutHints(LHINTS_CENTER_X));
  _frame2->AddFrame(_frame3, _l1);

  _frame1->AddFrame(_connectIcon, _l2);
  _frame1->AddFrame(_frame2, _l1);

  _frame4 = new OXCompositeFrame(this, 1, 1, HORIZONTAL_FRAME | FIXED_WIDTH);
  _tbOk = new OXTextButton(_frame4, new  OHotString("&OK"), 1);
  _tbCancel = new OXTextButton(_frame4, new  OHotString("&Cancel"), 2);
  _tbOk->Associate(this);
  _tbCancel->Associate(this);
  _frame4->AddFrame(_tbOk, _l4);
  _frame4->AddFrame(_tbCancel, _l4);

  int bwidth = max(_tbOk->GetDefaultWidth(), _tbCancel->GetDefaultWidth());
  _frame4->Resize(bwidth * 2 + 20, _frame4->GetDefaultHeight());

  if (ret_site->_login)
    _loginEntry->AddText(0, ret_site->_login);
  
  AddFrame(_frame1, _l1);
  AddFrame(_frame4, _l3);

  SetDefaultAcceptButton(_tbOk);
  SetDefaultCancelButton(_tbCancel);
  SetFocusOwner(_loginEntry);

  SetMWMHints(MWM_DECOR_ALL | MWM_DECOR_MAXIMIZE | MWM_DECOR_MENU,
              MWM_FUNC_ALL | MWM_FUNC_MAXIMIZE | MWM_FUNC_RESIZE,
              MWM_INPUT_MODELESS);

  MapSubwindows();
  Resize(GetDefaultSize());
  CenterOnParent();

  char buf[128];
  sprintf(buf, "Login to %s", ret_site->_site);
  SetWindowName(buf);
  MapWindow();

  _client->WaitFor(this);
}

OXPasswdDialog::~OXPasswdDialog() {
  delete _l1;
  delete _l2;
  delete _l3;
  delete _l4;
  _client->FreePicture(_connectPicture);
}

int OXPasswdDialog::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;

  switch (msg->type) {
    case MSG_BUTTON:
    case MSG_CHECKBUTTON:
    case MSG_RADIOBUTTON:
      switch (msg->action) {
        case MSG_CLICK:
          switch (wmsg->id) {
            case 1:
              *_ret_val = true;
              _ret_site->SetLogin(
                      (char*)_loginEntry->GetString());
              _ret_site->SetPasswd(
                      (char*)_passwdEntry->GetString());
              CloseWindow();
              break;

            case 2:
              *_ret_val = false;
              CloseWindow();
              break;
          }
          break;
      }
      break;

  }

  return true;
}


//----------------------------------------------------------------------

OXSetDirectory::OXSetDirectory(const OXWindow *p, const OXWindow *main,
               int w, int h, char *path, int *rtc, unsigned long options) :
  OXTransientFrame(p, main, w, h, options) {

  _l1 = new OLayoutHints(LHINTS_CENTER_X, 5, 5, 4, 4);
  _l2 = new OLayoutHints(LHINTS_EXPAND_X, 2, 2, 2, 2);
  _l3 = new OLayoutHints(LHINTS_TOP | LHINTS_RIGHT, 2, 2, 2, 2);

  _lpath = new OXLabel(this, new OString("Type new directory to change to"));
  _tepath = new OXTextEntry(this, new OTextBuffer(128), 11);
  _tepath->Resize(250, _tepath->GetDefaultHeight());

  _bframe = new OXCompositeFrame(this, 1, 1, HORIZONTAL_FRAME | FIXED_WIDTH);
  _tbOk = new OXTextButton(_bframe, new  OHotString("&OK"), 1);
  _tbCancel = new OXTextButton(_bframe, new  OHotString("&Cancel"), 2);
  _tbOk->Associate(this);
  _tbCancel->Associate(this);
  _bframe->AddFrame(_tbOk, _l2);
  _bframe->AddFrame(_tbCancel, _l2);

  int bwidth = max(_tbOk->GetDefaultWidth(), _tbCancel->GetDefaultWidth());
  _bframe->Resize(bwidth * 2 + 20, _bframe->GetDefaultHeight());

  AddFrame(_lpath, _l1);
  AddFrame(_tepath, _l1);
  AddFrame(_bframe, _l3);

  _rtc = rtc;
  *_rtc = false;
  _path = path;

  SetDefaultAcceptButton(_tbOk);
  SetDefaultCancelButton(_tbCancel);
  SetFocusOwner(_tepath);

  SetWindowName("Set Directory");
  SetMWMHints(MWM_DECOR_ALL | MWM_DECOR_MAXIMIZE | MWM_DECOR_MENU,
            MWM_FUNC_ALL | MWM_FUNC_MAXIMIZE | MWM_FUNC_RESIZE,
            MWM_INPUT_MODELESS);

  MapSubwindows();
  Resize(GetDefaultSize());
  CenterOnParent();
  MapWindow();

  _client->WaitFor(this);
}

OXSetDirectory::~OXSetDirectory() {
  delete _l1;
  delete _l2;
  delete _l3;
}

int OXSetDirectory::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;

  switch (msg->type) {
    case MSG_BUTTON:
    case MSG_MENU:
      switch (msg->action) {
        case MSG_CLICK:
          switch (wmsg->id) {
            case 1:
              *_rtc = true;
              strcpy(_path, _tepath->GetString());
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

  return true;
}


//----------------------------------------------------------------------

#define M_TE_OK     20
#define M_TE_CANCEL 21

#include "icons/ascii-trans.xpm"
#include "icons/binary-trans.xpm"

OXChooseType::OXChooseType(const OXWindow *p, const OXWindow *main,
                    int w, int h, int *rtc, unsigned long options) :
  OXTransientFrame(p, main, w, h, options) {
 
  *rtc = -1;
  _rtc = rtc;
 
  _l1 = new OLayoutHints(LHINTS_NORMAL, 3, 3, 1, 1);
  _l2 = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y);
  _l3 = new OLayoutHints(LHINTS_TOP | LHINTS_RIGHT, 3, 3, 3, 3);
  _l4 = new OLayoutHints(LHINTS_LEFT | LHINTS_CENTER_Y, 3, 3, 3, 3);
 
  OXButton *okTb, *cancelTb;
  OXCompositeFrame *frame;

  _asciiPic = _client->GetPicture("ascii-trans.xpm", ascii_trans_xpm);
  _binPic = _client->GetPicture("binary-trans.xpm", binary_trans_xpm);

  frame = new OXCompositeFrame(this, 10, 10, HORIZONTAL_FRAME);
  _binIcon = new OXIcon(frame, _binPic, 32, 32);
  _binRb = new OXRadioButton(frame, new OHotString("Binary transfer"), 10);
  _binRb->Associate(this);
  frame->AddFrame(_binIcon, _l1);
  frame->AddFrame(_binRb, _l4);
  AddFrame(frame, _l2);

  frame = new OXCompositeFrame(this, 10, 10, HORIZONTAL_FRAME);
  _asciiIcon = new OXIcon(frame, _asciiPic, 32, 32);
  _asciiRb = new OXRadioButton(frame, new OHotString("ASCII transfer"), 11);
  _asciiRb->Associate(this);
  frame->AddFrame(_asciiIcon, _l1);
  frame->AddFrame(_asciiRb, _l4);
  AddFrame(frame, _l2);

  _binRb->SetState(BUTTON_DOWN);
  _asciiRb->SetState(BUTTON_UP);
 
  frame = new OXCompositeFrame(this, 10, 10, HORIZONTAL_FRAME);
  okTb = new OXTextButton(frame, new  OHotString("&OK"), M_TE_OK);
  cancelTb = new OXTextButton(frame, new  OHotString("&Cancel"), M_TE_CANCEL);
  okTb->Associate(this);
  cancelTb->Associate(this);
  frame->AddFrame(okTb, _l1);
  frame->AddFrame(cancelTb, _l1);

  AddFrame(new OXHorizontal3dLine(this), _l2);
  AddFrame(frame, _l3);

  SetWindowName("Choose Transfer Type");

  SetMWMHints(MWM_DECOR_ALL | MWM_DECOR_MAXIMIZE | MWM_DECOR_MENU,
            MWM_FUNC_ALL | MWM_FUNC_MAXIMIZE | MWM_FUNC_RESIZE,
            MWM_INPUT_MODELESS);

  MapSubwindows();
  Resize(GetDefaultSize());
  CenterOnParent();

  MapWindow();

  _client->WaitFor(this);
}

OXChooseType::~OXChooseType() {
  delete _l1;
  delete _l2;
  delete _l3;
  delete _l4;
  _client->FreePicture(_asciiPic);
  _client->FreePicture(_binPic);
}
 
int OXChooseType::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;

  switch (msg->type) {
    case MSG_BUTTON:
      switch (msg->action) {
        case MSG_CLICK:
          switch (wmsg->id) {
            case M_TE_OK:
              CloseWindow();
              break;
 
            case M_TE_CANCEL:
              *_rtc = -1;
              CloseWindow();
              break;
          }
          break;
      }
      break;

    case MSG_RADIOBUTTON:
      switch (msg->action) {
        case MSG_CLICK:
          if (*_rtc != wmsg->id) {
            *_rtc = wmsg->id;
            if (wmsg->id == 10) {
              _asciiRb->SetState(BUTTON_UP);
              _binRb->SetState(BUTTON_DOWN);
            } else {
              _binRb->SetState(BUTTON_UP);
              _asciiRb->SetState(BUTTON_DOWN);
            }
          }
          break;
      }
  }

  return true;
}
