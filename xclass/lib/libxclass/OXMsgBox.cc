/**************************************************************************

    This file is part of xclass, a Win95-looking GUI toolkit.
    Copyright (C) 1996, 1997 David Barth, Hector Peraza.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

**************************************************************************/

#include <xclass/utils.h>
#include <xclass/OXClient.h>
#include <xclass/OString.h>
#include <xclass/OXTextButton.h>
#include <xclass/OXMsgBox.h>

#include "icons/mb-stop.s.xpm"
#include "icons/mb-question.s.xpm"
#include "icons/mb-exclamation.s.xpm"
#include "icons/mb-asterisk.s.xpm"

//-----------------------------------------------------------------

OXMsgBox::OXMsgBox(const OXWindow *p, const OXWindow *main,
                   OString *title, OString *msg, const OPicture *icon,
                   int buttons, int *ret_code,
                   unsigned long options) :
  OXTransientFrame(p, main, 10, 10, options) {

    _MsgBox(main, title, msg, icon, buttons, ret_code);
}

OXMsgBox::OXMsgBox(const OXWindow *p, const OXWindow *main,
                   OString *title, OString *msg, int icon,
                   int buttons, int *ret_code,
                   unsigned long options) :
  OXTransientFrame(p, main, 10, 10, options) {
    const OPicture *icon_pic;

    switch (icon) {
      case MB_ICONSTOP:
        icon_pic = _client->GetPicture("mb-stop.s.xpm", XCP_mb_stop);
        break;

      case MB_ICONQUESTION:
        icon_pic = _client->GetPicture("mb-question.s.xpm", XCP_mb_question);
        break;

      case MB_ICONEXCLAMATION:
        icon_pic = _client->GetPicture("mb-exclamation.s.xpm", XCP_mb_exclamation);
        break;

      case MB_ICONASTERISK:
        icon_pic = _client->GetPicture("mb-asterisk.s.xpm", XCP_mb_asterisk);
        break;

      default:
        icon_pic = NULL;
        break;
    }

    _MsgBox(main, title, msg, icon_pic, buttons, ret_code);
}

void OXMsgBox::_MsgBox(const OXWindow *main,
                       OString *title, OString *msg, const OPicture *icon,
                       int buttons, int *ret_code) {
  int nb, width;

  Yes = YesAll = No = OK = Apply =
  Retry = Ignore = Cancel = Close = Dismiss = NULL;
  Icon = NULL;
  _ret_code = ret_code;
  nb = width = 0;

  //--- create the buttons

  ButtonFrame = new OXHorizontalFrame(this, 60, 20, FIXED_WIDTH);
  L1 = new OLayoutHints(LHINTS_CENTER_Y | LHINTS_EXPAND_X,
                        3, 3, 0, 0);

  buttons &= (ID_YES | ID_YESALL | ID_NO | ID_OK | ID_APPLY |
              ID_RETRY | ID_IGNORE | ID_CANCEL | ID_CLOSE | ID_DISMISS);
  if (buttons == 0) buttons = ID_DISMISS;

  if (buttons & ID_YES) {
    Yes = new OXTextButton(ButtonFrame, new OHotString("&Yes"), ID_YES);
    Yes->Associate(this);
    ButtonFrame->AddFrame(Yes, L1);
    width = max(width, Yes->GetDefaultWidth()); ++nb;
  }

  if (buttons & ID_YESALL) {
    YesAll = new OXTextButton(ButtonFrame, new OHotString("Yes &All"),
                              ID_YESALL);
    YesAll->Associate(this);
    ButtonFrame->AddFrame(YesAll, L1);
    width = max(width, YesAll->GetDefaultWidth()); ++nb;
  }

  if (buttons & ID_NO) {
    No = new OXTextButton(ButtonFrame, new OHotString("&No"), ID_NO);
    No->Associate(this);
    ButtonFrame->AddFrame(No, L1);
    width = max(width, No->GetDefaultWidth()); ++nb;
  }

  if (buttons & ID_OK) {
    OK = new OXTextButton(ButtonFrame, new OHotString("&OK"), ID_OK);
    OK->Associate(this);
    ButtonFrame->AddFrame(OK, L1);
    width = max(width, OK->GetDefaultWidth()); ++nb;
  }

  if (buttons & ID_APPLY) {
    Apply = new OXTextButton(ButtonFrame, new OHotString("&Apply"), ID_APPLY);
    Apply->Associate(this);
    ButtonFrame->AddFrame(Apply, L1);
    width = max(width, Apply->GetDefaultWidth()); ++nb;
  }

  if (buttons & ID_RETRY) {
    Retry = new OXTextButton(ButtonFrame, new OHotString("&Retry"), ID_RETRY);
    Retry->Associate(this);
    ButtonFrame->AddFrame(Retry, L1);
    width = max(width, Retry->GetDefaultWidth()); ++nb;
  }

  if (buttons & ID_IGNORE) {
    Ignore = new OXTextButton(ButtonFrame, new OHotString("&Ignore"), ID_IGNORE);
    Ignore->Associate(this);
    ButtonFrame->AddFrame(Ignore, L1);
    width = max(width, Ignore->GetDefaultWidth()); ++nb;
  }

  if (buttons & ID_CANCEL) {
    Cancel = new OXTextButton(ButtonFrame, new OHotString("&Cancel"), ID_CANCEL);
    Cancel->Associate(this);
    ButtonFrame->AddFrame(Cancel, L1);
    width = max(width, Cancel->GetDefaultWidth()); ++nb;
  }

  if (buttons & ID_CLOSE) {
    Close = new OXTextButton(ButtonFrame, new OHotString("C&lose"), ID_CLOSE);
    Close->Associate(this);
    ButtonFrame->AddFrame(Close, L1);
    width = max(width, Close->GetDefaultWidth()); ++nb;
  }

  if (buttons & ID_DISMISS) {
    Dismiss = new OXTextButton(ButtonFrame, new OHotString("&Dismiss"), ID_DISMISS);
    Dismiss->Associate(this);
    ButtonFrame->AddFrame(Dismiss, L1);
    width = max(width, Dismiss->GetDefaultWidth()); ++nb;
  }

  //--- place buttons at the bottom

  L2 = new OLayoutHints(LHINTS_BOTTOM | LHINTS_CENTER_X, 0, 0, 5, 5);
  AddFrame(ButtonFrame, L2);

  //--- Keep the buttons centered and with the same width

  ButtonFrame->Resize((width + 20) * nb, GetDefaultHeight());

  IconFrame = new OXHorizontalFrame(this, 60, 20);

  L3 = new OLayoutHints(LHINTS_CENTER_Y | LHINTS_LEFT,
                        2, 2, 2, 2);
    
  if (icon) {
    Icon = new OXIcon(IconFrame, icon,
                 icon->GetWidth(), icon->GetHeight());
    IconFrame->AddFrame(Icon, L3);
  }
    
  Label = new OXLabel(IconFrame, msg);
  L4 = new OLayoutHints(LHINTS_CENTER_Y | LHINTS_LEFT | LHINTS_EXPAND_X,
                        4, 2, 2, 2);
  L5 = new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 10, 10, 7, 2);
    
  IconFrame->AddFrame(Label, L4);
  AddFrame(IconFrame, L5);

  MapSubwindows();
    
  Resize(GetDefaultSize());

  //---- position relative to the parent's window

  CenterOnParent();

  //---- make the message box non-resizable

  SetWMSize(_w, _h);
  SetWMSizeHints(_w, _h, _w, _h, 0, 0);

  //---- set names

  SetWindowName((char *) title->GetString());
  SetIconName((char *) title->GetString());
  SetClassHints("XCLASS", "MsgBox");

  SetMWMHints(MWM_DECOR_ALL | MWM_DECOR_RESIZEH | MWM_DECOR_MAXIMIZE |
                              MWM_DECOR_MINIMIZE | MWM_DECOR_MENU,
              MWM_FUNC_ALL | MWM_FUNC_RESIZE | MWM_FUNC_MAXIMIZE |
                             MWM_FUNC_MINIMIZE,
              MWM_INPUT_MODELESS);
                          
  MapWindow();
  _client->WaitFor(this);
}

OXMsgBox::~OXMsgBox() {
  delete L1;
  delete L2;
  delete L3;
  delete L4;
  delete L5;
}

int OXMsgBox::CloseWindow() {
  if (_ret_code) *_ret_code = ID_CLOSE;
  return OXTransientFrame::CloseWindow();
}

int OXMsgBox::ProcessMessage(OMessage *msg) {
  OButtonMessage *bmsg;

  switch (msg->type) {
  case MSG_BUTTON:
    switch (msg->action) {
    case MSG_CLICK:
      bmsg = (OButtonMessage *) msg;
      if (_ret_code) *_ret_code = bmsg->id;
      delete this;
      break;

    default:
      break;
    }
    break;
  }
  return True;
}
