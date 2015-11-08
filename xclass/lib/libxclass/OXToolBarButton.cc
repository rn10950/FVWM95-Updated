/**************************************************************************

    This file is part of xclass.
    Copyright (C) 1998, 1999, G. Matzka, H. Peraza.

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
#include <xclass/OXToolBarButton.h>

OXFont *OXToolBarButton::_defaultFont = NULL;


//----------------------------------------------------------------------

OXToolBarButton::OXToolBarButton(const OXWindow *p, const OPicture *pic,
                 OHotString *label, int ID, unsigned int option) :
  OXButton(p, ID, option) {

  if (_defaultFont == NULL) {
    _defaultFont = _client->GetFont("helvetica -10");
  }

  _font = _defaultFont;

  XGCValues gval;
  unsigned long gmask = GCForeground | GCBackground | GCFont;

  _backPixel = _defaultFrameBackground;

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

  SetUpSize();
}

OXToolBarButton::~OXToolBarButton() {
  _client->FreeGC(_gc);
  if (_label) delete _label;
  _client->FreeColor(_hilitePixel);
  _client->FreeColor(_shadowPixel);
}

void OXToolBarButton::_DoRedraw() {
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

  OXButton::DrawBorder();
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
      _gc->SetForeground(_blackPixel);
      _label->Draw(GetDisplay(), _id, _gc->GetGC(), x, y);
    }
    _gc->SetForeground(fg);
  }
}

int OXToolBarButton::HandleCrossing(XCrossingEvent *event) {
  OXButton::HandleCrossing(event);
  if (event->type == LeaveNotify) {
    _mouseOn = False;
  } else if (event->type == EnterNotify) {
    _mouseOn = True;
  }
  if (IsEnabled()) NeedRedraw();
  return True;
}

ODimension OXToolBarButton::GetDefaultSize() const {
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

  if (_options & FIXED_WIDTH) size.w = _w;
  if (_options & FIXED_HEIGHT) size.h = _h;

  return size;
}

void OXToolBarButton::SetUpSize() {
  _tw = _th = 0;
  if (_label && _showLabel) {
    _tw = _font->XTextWidth(_label->GetString(), _label->GetLength());
    _th = _font->TextHeight();
  }
}

void OXToolBarButton::SetLabel(OHotString *label) {
  if (_label) delete _label;
  _label = label;
  SetUpSize();
  NeedRedraw();
}

void OXToolBarButton::ShowLabel(int onoff) {
  if (_showLabel != onoff) {
    _showLabel = onoff;
    SetUpSize();
    NeedRedraw();
  }
}

void OXToolBarButton::SetNormalPic(const OPicture *pic) {
  _normalPic = pic;
  NeedRedraw();
}

void OXToolBarButton::SetOnPic(const OPicture *pic) {
  _onPic = pic;
  NeedRedraw();
}

void OXToolBarButton::SetDisabledPic(const OPicture *pic) {
  _disabledPic = pic;
  if (!IsEnabled()) NeedRedraw();
}
