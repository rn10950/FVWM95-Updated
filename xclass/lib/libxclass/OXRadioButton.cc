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
#include <xclass/OXMainFrame.h>
#include <xclass/OXRadioButton.h>
#include <xclass/OXClient.h>
#include <xclass/OResourcePool.h>
#include <xclass/OXFont.h>
#include <xclass/OGC.h>
#include <xclass/OString.h>
#include <xclass/OPicture.h>

#include <X11/keysym.h>

#include "icons/rbutton.on.xpm"
#include "icons/rbutton.off.xpm"
#include "icons/rbutton.ond.xpm"
#include "icons/rbutton.offd.xpm"


const OPicture *OXRadioButton::_on;
const OPicture *OXRadioButton::_off;
const OPicture *OXRadioButton::_ond;
const OPicture *OXRadioButton::_offd;
int   OXRadioButton::_init = False;


//-----------------------------------------------------------------

OXRadioButton::OXRadioButton(const OXWindow *p, OString *s, int ID,
                             unsigned int option)
  : OXTextButton(p, s, ID, option) {

    if (!_init) {
      _on = _client->GetPicture("rbutton.on.xpm", XCP_rbutton_on);
      _off = _client->GetPicture("rbutton.off.xpm", XCP_rbutton_off);
      _ond = _client->GetPicture("rbutton.ond.xpm", XCP_rbutton_ond);
      _offd = _client->GetPicture("rbutton.offd.xpm", XCP_rbutton_offd);

      if (!_on || !_off || !_ond || !_offd)
        FatalError("OXRadioButton: missing required pixmaps");

      _init = True;
    }

    _widgetType = "OXRadioButton";

    OXMainFrame *main = (OXMainFrame *) _toplevel;
    if (main) main->UnregisterButton(this);
}

ODimension OXRadioButton::GetDefaultSize() const {
  int w = (_tw == 0) ? _offd->GetWidth() : _tw + _offd->GetWidth() + 10;
  int h = (_th == 0) ? _offd->GetHeight() : _th + 2;
  return ODimension(w, h);
}

void OXRadioButton::_SetState(int state) {
  if (state != _state) {
    _state = state;
    NeedRedraw(False);
  }
}

int OXRadioButton::HandleKey(XKeyEvent *event) {
  int click = False;

  if (!IsEnabled()) return True;

  if ((event->keycode == _hkeycode) ||
      (XLookupKeysym(event, 0) == XK_space)) {
    if (event->type == KeyPress) {
      if (TakesFocus()) RequestFocus();
      _down = True;
      _prevstate = _state;
    } else { // KeyRelease
      _SetState(_down ? BUTTON_DOWN : _prevstate);
      _down = False;
      click = (_state != _prevstate);
    }
    NeedRedraw(False);
    if (click) {
      OButtonMessage message(MSG_RADIOBUTTON, MSG_CLICK, _widgetID);
      SendMessage(_msgObject, &message);
    }
  }

  return True;
}

int OXRadioButton::HandleButton(XButtonEvent *event) {
  int click = False;

  if (!IsEnabled()) return True;

  if (event->button != Button1) return False;

  if ((event->type == ButtonPress) && TakesFocus())
    RequestFocus();

  // We don't need to check the button number as XGrabButton will
  // only allow button1 events
  if (event->type == ButtonPress) {
    _down = True;
    _prevstate = _state;
  } else { // ButtonRelease
    _SetState(_down ? BUTTON_DOWN : _prevstate);
    _down = False;
    click = (_state != _prevstate);
  }
  NeedRedraw(False);
  if (click) {
    OButtonMessage message(MSG_RADIOBUTTON, MSG_CLICK, _widgetID);
    SendMessage(_msgObject, &message);
  }
  return True;
}

int OXRadioButton::HandleCrossing(XCrossingEvent *event) {

  if (!IsEnabled()) return True;
  if (event->state != Button1Mask) return True;

  if (event->type == EnterNotify) {
    _down = True;
  } else {
    _down = False;
  }
  NeedRedraw(False);

  return True;
}

void OXRadioButton::_DoRedraw() {
  int tx, ty, y0, pw;

  OXFrame::_DoRedraw();

  tx = 20;
  ty = (_h - _th) >> 1;

  pw = 12;
//  y0 = ty + ((_th - pw) >> 1);
  y0 = (_th == 0) ? 0 : ty +1;

  ClearArea(0, y0, pw, pw);

  if (!IsEnabled()) {
    _offd->Draw(GetDisplay(), _id, _normGC->GetGC(), 0, y0);
  } else if (_state == BUTTON_DOWN) {
    if (_down)
      _ond->Draw(GetDisplay(), _id, _normGC->GetGC(), 0, y0);
    else
      _on->Draw(GetDisplay(), _id, _normGC->GetGC(), 0, y0);
  } else {
    if (_down)
      _offd->Draw(GetDisplay(), _id, _normGC->GetGC(), 0, y0);
    else
      _off->Draw(GetDisplay(), _id, _normGC->GetGC(), 0, y0);
  }

  if (!_text) return;

  if (!IsEnabled()) {
    unsigned long forecolor = _normGC->GetForeground();

    _normGC->SetForeground(_hilitePixel);
    _tl->DrawText(GetDisplay(), _id, _normGC->GetGC(), tx+1, ty+1, 0, -1);
    _tl->UnderlineChar(GetDisplay(), _id, _normGC->GetGC(), tx+1, ty+1, 
                       _text->GetHotIndex());
    _normGC->SetForeground(_shadowPixel);
    _tl->DrawText(GetDisplay(), _id, _normGC->GetGC(), tx, ty, 0, -1);
    _tl->UnderlineChar(GetDisplay(), _id, _normGC->GetGC(), tx, ty, 
                       _text->GetHotIndex());
    _normGC->SetForeground(forecolor);
  } else {
    _tl->DrawText(GetDisplay(), _id, _normGC->GetGC(), tx, ty, 0, -1);
    _tl->UnderlineChar(GetDisplay(), _id, _normGC->GetGC(), tx, ty, 
                       _text->GetHotIndex());
  }

  if (HasFocus()) {
    DrawRectangle(GetResourcePool()->GetFocusHiliteGC()->GetGC(),
                  tx-1, ty, _tw+1, _th);
  }
}
