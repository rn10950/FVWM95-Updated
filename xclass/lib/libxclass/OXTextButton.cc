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
#include <xclass/OXTextButton.h>
#include <xclass/OXClient.h>
#include <xclass/OResourcePool.h>
#include <xclass/OXFont.h>
#include <xclass/OGC.h>

#include <X11/keysym.h>

const OXFont *OXTextButton::_defaultFont;
const OXGC *OXTextButton::_defaultGC;
int   OXTextButton::_init = False;


//-----------------------------------------------------------------

OXTextButton::OXTextButton(const OXWindow *p, OString *s, int ID,
                           unsigned int options)
  : OXButton(p, ID, options) {
    int hotchar;

    if (!_init) {
      const OResourcePool *res = _client->GetResourcePool();

      _defaultFont = res->GetDefaultFont();
      _defaultGC = res->GetFrameGC();

      _init = True;
    }

    _widgetType = "OXTextButton";

    _text = s;
    _align = TEXT_CENTER_X | TEXT_CENTER_Y;
    _justify = TEXT_JUSTIFY_LEFT;
    _textFlags = 0;
    _wrapLength = -1;
    _hkeycode = 0;

    _ml = _mr = _mt = _mb = 0;

    _font = _defaultFont;
    _normGC = (OXGC *) _defaultGC;

    if (_text) {
      _tl = _font->ComputeTextLayout(_text->GetString(), _text->GetLength(),
                                     _wrapLength, _justify, _textFlags,
                                     &_tw, &_th);
    } else {
      _tl = NULL;
      _tw = _th = 0;
    }

    OXMainFrame *main = (OXMainFrame *) _toplevel;

    if (s && ((hotchar = s->GetHotChar()) != 0)) {
      if ((_hkeycode = XKeysymToKeycode(GetDisplay(), hotchar)) != 0) {
        if (main) {
          main->BindKey(this, _hkeycode, Mod1Mask);
          main->BindKey(this, _hkeycode, Mod1Mask | Mod2Mask);         
          main->BindKey(this, _hkeycode, Mod1Mask | LockMask);
          main->BindKey(this, _hkeycode, Mod1Mask | Mod2Mask | LockMask);
        }
      }
    }

    if (main) main->RegisterButton(this);
}

OXTextButton::~OXTextButton() {
  if (_hkeycode) {
    const OXMainFrame *main = (OXMainFrame *) _toplevel;
    if (main) {
      main->RemoveBind(this, _hkeycode, Mod1Mask);
      main->RemoveBind(this, _hkeycode, Mod1Mask | Mod2Mask);
      main->RemoveBind(this, _hkeycode, Mod1Mask | LockMask);
      main->RemoveBind(this, _hkeycode, Mod1Mask | Mod2Mask | LockMask);
    }
  }
  if (_text) delete _text;
  if (_tl) delete _tl;
  if (_normGC != _defaultGC) delete _normGC;
  if (_font != _defaultFont) _client->FreeFont((OXFont *) _font);

  OXMainFrame *main = (OXMainFrame *) _toplevel;
  if (main) main->UnregisterButton(this);
}

void OXTextButton::SetText(OString *new_text) {
  // TODO: change key binding to the new hot char
  if (new_text) {
    if (_text) delete _text;
    _text = new_text;
    Layout();
  }
}

void OXTextButton::SetFont(const OXFont *f) {
  if (f) {
    const OXFont *oldfont = _font;
    _font = f;
    if (_normGC == _defaultGC) _normGC = new OXGC(_defaultGC);
    _normGC->SetFont(_font->GetId());
    if (oldfont != _defaultFont) _client->FreeFont((OXFont *) oldfont);
    Layout();
  }
}

void OXTextButton::SetTextColor(unsigned int color) {
  if (_normGC == _defaultGC) _normGC = new OXGC(_defaultGC);
  _normGC->SetForeground(color);
  NeedRedraw(False);
}

void OXTextButton::Layout() {
  if (_tl) delete _tl;
  if (_text) {
    _tl = _font->ComputeTextLayout(_text->GetString(), _text->GetLength(),
                                   _wrapLength, _justify, _textFlags,
                                   &_tw, &_th);
  } else {
    _tl = NULL;
    _tw = _th = 0;
  }
  NeedRedraw(True);
}

void OXTextButton::_DoRedraw() {
  int x, y;
  
  OXFrame::_DoRedraw();

  if (_align & TEXT_LEFT)
    x = _ml + 4;
  else if (_align & TEXT_RIGHT)
    x = _w - _tw - _mr - 4;
  else
    x = (_w - _tw + _ml - _mr) >> 1;

  if (_align & TEXT_TOP)
    y = _mt + 3;
  else if (_align & TEXT_BOTTOM)
    y = _h - _th - _mb - 3;
  else
    y = (_h - _th + _mt - _mb) >> 1;

  if (_state == BUTTON_DOWN || _state == BUTTON_ENGAGED) { ++x; ++y; }

  if (_state == BUTTON_ENGAGED) {
    FillRectangle(_hibckgndGC->GetGC(), 2, 2, _w-4, _h-4);
    DrawLine(_hilightGC, 2, 2, _w-3, 2);
  }

  if (!_text) return;

  if (!IsEnabled()) {
    unsigned long forecolor = _normGC->GetForeground();

    _normGC->SetForeground(_hilitePixel);
    _tl->DrawText(GetDisplay(), _id, _normGC->GetGC(), x+1, y+1, 0, -1);
    _tl->UnderlineChar(GetDisplay(), _id, _normGC->GetGC(), x+1, y+1,
                       _text->GetHotIndex());
    _normGC->SetForeground(_shadowPixel);
    _tl->DrawText(GetDisplay(), _id, _normGC->GetGC(), x, y, 0, -1);
    _tl->UnderlineChar(GetDisplay(), _id, _normGC->GetGC(), x, y, 
                       _text->GetHotIndex());
    _normGC->SetForeground(forecolor);
  } else {
    _tl->DrawText(GetDisplay(), _id, _normGC->GetGC(), x, y, 0, -1);
    _tl->UnderlineChar(GetDisplay(), _id, _normGC->GetGC(), x, y, 
                       _text->GetHotIndex());
  }

  if (HasFocus()) {
    DrawRectangle(GetResourcePool()->GetFocusHiliteGC()->GetGC(),
                  _bw+1, _bw+1, _w-(_bw<<1)-3, _h-(_bw<<1)-3);
  }
}

int OXTextButton::HandleKey(XKeyEvent *event) {
  int click = False;

  if (!IsEnabled()) return True;

  if ((event->keycode == _hkeycode) ||
      (XLookupKeysym(event, 0) == XK_space)) {
    if (event->type == KeyPress) {
      if (TakesFocus()) RequestFocus();
      if (_state == BUTTON_ENGAGED) return True;
      SetState(BUTTON_DOWN);
    } else { // KeyRelease
      if (_state == BUTTON_ENGAGED /*&& !allowRelease*/) return True;
      click = (_state == BUTTON_DOWN);
      if (click && _type)
        SetState(BUTTON_ENGAGED);
      else
        SetState(BUTTON_UP);
    }
    if (click) {
      OButtonMessage message(MSG_BUTTON, MSG_CLICK, _widgetID);
      SendMessage(_msgObject, &message);
    }
  }

  return True;
}
