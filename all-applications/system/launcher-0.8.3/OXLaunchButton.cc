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

#include <xclass/OString.h>
#include <xclass/OPicture.h>
#include <xclass/OResourcePool.h>
#include <xclass/OXCanvas.h>

#include "OXLaunchButton.h"

OXFont *OXLaunchButton::_defaultFont = NULL;


//----------------------------------------------------------------------

OXLaunchButton::OXLaunchButton(const OXWindow *p, const OPicture *pic,
                 OHotString *label, int ID, unsigned int option) :
  OXButton(p, ID, option) {

  if (_defaultFont == NULL) {
    _defaultFont = (OXFont *) _client->GetResourcePool()->GetIconFont();
  }

  _font = _defaultFont;

  XGCValues gval;
  unsigned long gmask = GCForeground | GCBackground | GCFont;

  _backPixel = _client->GetColorByName("#808080");

  _hilitePixel = _client->GetHilite(_backPixel);
  _shadowPixel = _client->GetShadow(_backPixel);

  gval.font = _font->GetId();
  gval.background = _backPixel;
  gval.foreground = _blackPixel;
  _gc = (OXGC *) _client->GetGC(_id, gmask, &gval);

  _normalPic = _onPic = _disabledPic = pic;
  _label = label;
  _prevstate = _state = BUTTON_UP;
  _type = BUTTON_NORMAL;
  _mouseOn = 0;

  _showLabel = True;
  _labelColor = _blackPixel;

  SetUpSize();

  // Button1 is already grabbed (OXButton)

  XGrabButton(GetDisplay(), Button3, AnyModifier, _id, False,
              ButtonPressMask | ButtonReleaseMask |
              EnterWindowMask | LeaveWindowMask,
              GrabModeAsync, GrabModeAsync, None, None);
}

OXLaunchButton::~OXLaunchButton() {
  _client->FreeGC(_gc);
  if (_label) delete _label;
  _client->FreeColor(_hilitePixel);
  _client->FreeColor(_shadowPixel);
}

void OXLaunchButton::_DoRedraw() {
  int x, y;
  int options;

  ClearWindow();
  options = GetOptions();

  const OPicture *pic = _normalPic;
  if (!IsEnabled()) {
    pic = _disabledPic;
  } else if (_mouseOn == True) {
    pic = _onPic;
  }
  if (pic == NULL) {
    pic = _normalPic;
  }
  if (_state == BUTTON_DOWN || _state == BUTTON_ENGAGED)
    ; // do nothing
  else if (_mouseOn == True && IsEnabled())
    ChangeOptions(RAISED_FRAME);
  else
    ChangeOptions(CHILD_FRAME);

#if 0
  OXFrame::DrawBorder();
#else
  unsigned long fgp = _gc->GetForeground();
  switch (_options & (RAISED_FRAME | SUNKEN_FRAME)) {
  case SUNKEN_FRAME:
    _gc->SetForeground(_shadowPixel);
    DrawLine(_gc->GetGC(), 0, 0, _w-2, 0);
    DrawLine(_gc->GetGC(), 0, 0, 0, _h-2);
    _gc->SetForeground(_hilitePixel);
    DrawLine(_gc->GetGC(), 0, _h-1, _w-1, _h-1);
    DrawLine(_gc->GetGC(), _w-1, _h-1, _w-1, 0);
    break;

  case RAISED_FRAME:
    _gc->SetForeground(_hilitePixel);
    DrawLine(_gc->GetGC(), 0, 0, _w-2, 0);
    DrawLine(_gc->GetGC(), 0, 0, 0, _h-2);
    _gc->SetForeground(_shadowPixel);
    DrawLine(_gc->GetGC(), 0, _h-1, _w-1, _h-1);
    DrawLine(_gc->GetGC(), _w-1, _h-1, _w-1, 0);
    break;

/*
  default:
    _gc->SetForeground(_backPixel);
    DrawLine(_gc->GetGC(), 0, 0, _w-2, 0);
    DrawLine(_gc->GetGC(), 0, 0, 0, _h-2);
    DrawLine(_gc->GetGC(), 0, _h-1, _w-1, _h-1);
    DrawLine(_gc->GetGC(), _w-1, _h-1, _w-1, 0);
    break;
*/
  }
  _gc->SetForeground(fgp);
#endif
  ChangeOptions(options);

  if (pic != NULL) {
    x = (_w - pic->GetWidth()) >> 1;
    y = 2;
    if (_label == NULL) y = 3;
    if (_state == BUTTON_ENGAGED || _state == BUTTON_DOWN) { ++x; ++y; }
    pic->Draw(GetDisplay(), _id, _gc->GetGC(), x, y);
  }
  if (_label && _showLabel) {
    OFontMetrics fm;
    _font->GetFontMetrics(&fm);
    x = (_w - _tw) >> 1;
    y = (_h - 3 - fm.descent);
    if (_state == BUTTON_ENGAGED || _state == BUTTON_DOWN) { ++x; ++y; }
    unsigned long fg = _gc->GetForeground();
    if (!IsEnabled()) {
      _gc->SetForeground(_hilitePixel);
      _label->Draw(GetDisplay(), _id, _gc->GetGC(), x+1, y+1);
      _gc->SetForeground(_shadowPixel);
      _label->Draw(GetDisplay(), _id, _gc->GetGC(), x, y);
    } else {
      _gc->SetForeground(_labelColor);
      _label->Draw(GetDisplay(), _id, _gc->GetGC(), x, y);
    }
    _gc->SetForeground(fg);
  }
}

int OXLaunchButton::HandleCrossing(XCrossingEvent *event) {
  OXButton::HandleCrossing(event);
  if (event->type == LeaveNotify) {
    _mouseOn = False;
  } else if (event->type == EnterNotify) {
    _mouseOn = True;
  }
  if (IsEnabled()) NeedRedraw();
  return True;
}

int OXLaunchButton::HandleButton(XButtonEvent *event) {
  if (event->button == Button3) {
    if (event->type == ButtonRelease) {
      OContainerMessage msg(MSG_BUTTON, MSG_SELECT, _widgetID,
                            event->button, 0, 0, event->x_root, event->y_root);
      SendMessage(_msgObject, &msg);
    }
    return True;
  } 
  return OXButton::HandleButton(event);
}

ODimension OXLaunchButton::GetDefaultSize() const {
  ODimension size(_tw, 0);
  if (_normalPic) {
    size.w = max(size.w, _normalPic->GetWidth());
    size.h = max(size.h, _normalPic->GetHeight());
  }
  if (_onPic) {
    size.w = max(size.w, _onPic->GetWidth());
    size.h = max(size.h, _onPic->GetHeight());
  }
  if (_disabledPic) {
    size.w = max(size.w, _disabledPic->GetWidth());
    size.h = max(size.h, _disabledPic->GetHeight());
  }
  size.w += 6;
  size.h += _th + 6;
  return size;
}

void OXLaunchButton::SetUpSize() {
  _tw = _th = 0;
  if (_label && _showLabel) {
    _tw = _font->XTextWidth(_label->GetString(), _label->GetLength());
    _th = _font->TextHeight();
  }
}

void OXLaunchButton::SetLabel(OHotString *label) {
  if (_label) delete _label;
  _label = label;
  SetUpSize();
  NeedRedraw();
}

void OXLaunchButton::SetLabelColor(unsigned long color) {
  _labelColor = color;
  NeedRedraw();
}

void OXLaunchButton::ShowLabel(int onoff) {
  if (_showLabel != onoff) {
    _showLabel = onoff;
    SetUpSize();
    NeedRedraw();
  }
}

void OXLaunchButton::SetNormalPic(const OPicture *pic) {
  _normalPic = pic;
  NeedRedraw();
}

void OXLaunchButton::SetOnPic(const OPicture *pic) {
  _onPic = pic;
  NeedRedraw();
}

void OXLaunchButton::SetDisabledPic(const OPicture *pic) {
  _disabledPic = pic;
  if (!IsEnabled()) NeedRedraw();
}
