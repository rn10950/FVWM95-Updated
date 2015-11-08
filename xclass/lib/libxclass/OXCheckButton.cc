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
#include <xclass/OXCheckButton.h>
#include <xclass/OXClient.h>
#include <xclass/OResourcePool.h>
#include <xclass/OXFont.h>
#include <xclass/OGC.h>
#include <xclass/OString.h>
#include <xclass/OPicture.h>

#include <X11/keysym.h>

#include "icons/chkbutton.on.xpm"
#include "icons/chkbutton.off.xpm"
#include "icons/chkbutton.ond.xpm"
#include "icons/chkbutton.offd.xpm"

const OPicture *OXCheckButton::_on;
const OPicture *OXCheckButton::_off;
const OPicture *OXCheckButton::_ond;
const OPicture *OXCheckButton::_offd;
int   OXCheckButton::_init = False;


//-----------------------------------------------------------------

OXCheckButton::OXCheckButton(const OXWindow *p, OString *s, int ID,
                             unsigned int option)
  : OXTextButton(p, s, ID, option) {

    if (!_init) {
      _on = _client->GetPicture("chkbutton.on.xpm", XCP_chkbutton_on);
      _off = _client->GetPicture("chkbutton.off.xpm", XCP_chkbutton_off);
      _ond = _client->GetPicture("chkbutton.ond.xpm", XCP_chkbutton_ond);
      _offd = _client->GetPicture("chkbutton.offd.xpm", XCP_chkbutton_offd);

      if (!_on || !_off || !_ond || !_offd)
        FatalError("OXCheckButton: bad or missing pixmaps");

      _init = True;
    }

    _widgetType = "OXCheckButton";

    OXMainFrame *main = (OXMainFrame *) _toplevel;
    if (main) main->UnregisterButton(this);
}

ODimension OXCheckButton::GetDefaultSize() const {
  int w = (_tw == 0) ? _offd->GetWidth() : _tw + _offd->GetWidth() + 9;
  int h = (_th == 0) ? _offd->GetHeight() : _th + 2;                      
  return ODimension(w, h);           
}

void OXCheckButton::_SetState(int state) {
  if (state != _state) {
    _state = state;
    NeedRedraw(False);
  }
}

int OXCheckButton::HandleKey(XKeyEvent *event) {
  int click = False;

  if (!IsEnabled()) return True;

  if ((event->keycode == _hkeycode) ||
      (XLookupKeysym(event, 0) == XK_space)) {
    if (event->type == KeyPress) {
      if (TakesFocus()) RequestFocus();
      _down = True;
      _prevstate = _state;
    } else { // KeyRelease
      if (_down)
        _SetState((_prevstate == BUTTON_UP) ? BUTTON_DOWN : BUTTON_UP);
      _down = False;
      click = (_state != _prevstate);
    }
    NeedRedraw(False);
    if (click) {
      OButtonMessage message(MSG_CHECKBUTTON, MSG_CLICK, _widgetID);
      SendMessage(_msgObject, &message);
    }
  }

  return True;
}

int OXCheckButton::HandleButton(XButtonEvent *event) {
  int click = False;

  if (!IsEnabled()) return True;

  if (event->button != Button1) return False;

  if ((event->type == ButtonPress) && TakesFocus())
    RequestFocus();

  if (event->type == ButtonPress) {
    _down = True;
    _prevstate = _state;
  } else { // ButtonRelease
    if (_down)
      _SetState((_prevstate == BUTTON_UP) ? BUTTON_DOWN : BUTTON_UP);
    _down = False;
    click = (_state != _prevstate);
  }
  NeedRedraw(False);
  if (click) {
    OButtonMessage message(MSG_CHECKBUTTON, MSG_CLICK, _widgetID);
    SendMessage(_msgObject, &message);
  }

  return True;
}

int OXCheckButton::HandleCrossing(XCrossingEvent *event) {

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

void OXCheckButton::_DoRedraw() {
  int tx, ty, y0, cw;
  
  OXFrame::_DoRedraw();

/*
  cw = 13;
  y0 = (_h - cw) >> 1;

  DrawLine(_shadowGC, 0, y0, cw-2, y0);
  DrawLine(_shadowGC, 0, y0, 0, y0+cw-2);
  DrawLine(_blackGC, 1, y0+1, cw-3, y0+1);
  DrawLine(_blackGC, 1, y0+1, 1, y0+cw-3);

  DrawLine(_hilightGC, 0, y0+cw-1, cw-1, y0+cw-1);
  DrawLine(_hilightGC, cw-1, y0+cw-1, cw-1, y0);
  DrawLine(_bckgndGC,  2, y0+cw-2, cw-2, y0+cw-2);
  DrawLine(_bckgndGC,  cw-2, y0+2, cw-2, y0+cw-2);

  FillRectangle(_whiteGC, 2, y0+2, cw-4, cw-4);

  if (_state == BUTTON_DOWN) {
    XSegment seg[6];

    int l = 2;
    int t = y0+2; 

    seg[0].x1 = 1+l; seg[0].y1 = 3+t; seg[0].x2 = 3+l; seg[0].y2 = 5+t;
    seg[1].x1 = 1+l; seg[1].y1 = 4+t; seg[1].x2 = 3+l; seg[1].y2 = 6+t;
    seg[2].x1 = 1+l; seg[2].y1 = 5+t; seg[2].x2 = 3+l; seg[2].y2 = 7+t;
    seg[3].x1 = 3+l; seg[3].y1 = 5+t; seg[3].x2 = 7+l; seg[3].y2 = 1+t;
    seg[4].x1 = 3+l; seg[4].y1 = 6+t; seg[4].x2 = 7+l; seg[4].y2 = 2+t;
    seg[5].x1 = 3+l; seg[5].y1 = 7+t; seg[5].x2 = 7+l; seg[5].y2 = 3+t;

    DrawSegments(_blackGC, seg, 6);
  }

  tx = 20;
  ty = 2;

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

*/


  tx = 20;
  ty = (_h - _th) >> 1;

  cw = 13;
//  y0 = ty + ((_th - cw) >> 1);
  y0 = (_th == 0) ? 0 : ty +1;

  ClearArea(0, y0, cw, cw);

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
