/**************************************************************************

    This file is part of xclauncher.
    Copyright (C) 1998-2002 Hector Peraza.

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
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "OXRenameDlg.h"


//---------------------------------------------------------------------

OXRenameDlg::OXRenameDlg(const OXWindow *p, const OXWindow *main,
                         OString *wname, OString *title, OString *text,
                         OString *msg, int *ret_code,
                         unsigned long options) :
  OXTransientFrame(p, main, 10, 10, options) {
    int width = 0;

    _ret_code = ret_code;
    _msg = msg;

    L1 = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_TOP, 3, 3, 5, 5);

    //--- create the label for the top text

    L3 = new OLayoutHints(LHINTS_CENTER_X | LHINTS_TOP, 5, 5, 5, 0);
    AddFrame(new OXLabel(this, title), L3);

    //--- create the frame for the message entry box

    OXCompositeFrame *hframe = new OXHorizontalFrame(this, 0, 0, 0);
    msge = new OXTextEntry(hframe, new OTextBuffer(100));
    msge->Resize(250, msge->GetDefaultHeight());

    if (_msg) msge->AddText(0, _msg->GetString());

    L4 = new OLayoutHints(LHINTS_CENTER_Y | LHINTS_LEFT, 0, 5, 5, 5);
    L5 = new OLayoutHints(LHINTS_RIGHT | LHINTS_EXPAND_X | LHINTS_CENTER_Y);

    hframe->AddFrame(new OXLabel(hframe, new OString(text)), L4);
    hframe->AddFrame(msge, L5);

    AddFrame(hframe, L1);
    AddFrame(new OXHorizontal3dLine(this), L1);

    //--- create the buttons and button frame

    OXCompositeFrame *ButtonFrame = new OXHorizontalFrame(this,
                                                    60, 20, FIXED_WIDTH);

    Ok = new OXTextButton(ButtonFrame, new OHotString("&OK"), ID_OK);
    Ok->SetDefault();
    Ok->Associate(this);
    ButtonFrame->AddFrame(Ok, L1);
    width = max(width, Ok->GetDefaultWidth());

    Cancel = new OXTextButton(ButtonFrame, new OHotString("&Cancel"), ID_CANCEL);
    Cancel->Associate(this);
    ButtonFrame->AddFrame(Cancel, L1);
    width = max(width, Cancel->GetDefaultWidth());

    SetDefaultAcceptButton(Ok);
    SetDefaultCancelButton(Cancel);

    //--- place buttons at the bottom

    L2 = new OLayoutHints(LHINTS_BOTTOM | LHINTS_CENTER_X);
    AddFrame(ButtonFrame, L2);

    //--- Keep the buttons centered and with the same width

    ButtonFrame->Resize((width + 20) * 2, GetDefaultHeight());

    SetFocusOwner(msge);

    MapSubwindows();
    Resize(GetDefaultSize());

    CenterOnParent();

    //---- make the dialog box non-resizable

    SetWMSize(_w, _h);
    SetWMSizeHints(_w, _h, _w, _h, 0, 0);

    //---- set names

    SetWindowName(wname->GetString());
    SetIconName(wname->GetString());
    SetClassHints("XCLASS", "dialog");

    SetMWMHints(MWM_DECOR_ALL | MWM_DECOR_RESIZEH | MWM_DECOR_MAXIMIZE |
                                MWM_DECOR_MINIMIZE | MWM_DECOR_MENU,
                MWM_FUNC_ALL | MWM_FUNC_RESIZE | MWM_FUNC_MAXIMIZE |
                               MWM_FUNC_MINIMIZE,
                MWM_INPUT_MODELESS);

    MapWindow();
    _client->WaitFor(this);
}

OXRenameDlg::~OXRenameDlg() {
  delete L1;
  delete L2;
  delete L3;
  delete L4;
  delete L5;
}

int OXRenameDlg::CloseWindow() {
  if (_ret_code) *_ret_code = ID_CANCEL;
  return OXTransientFrame::CloseWindow();
}

int OXRenameDlg::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;

  switch (msg->type) {
  case MSG_BUTTON:
    switch (msg->action) {
    case MSG_CLICK:
      if (_ret_code) *_ret_code = wmsg->id;
      if (_msg) {
        _msg->Clear();
        _msg->Append(msge->GetString());
      }
      delete this;
      break;

    default:
      break;
    }
    break;
  }

  return True;
}
