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
#include <xclass/OResourcePool.h>
#include <xclass/OXLabel.h>
#include <xclass/OXFont.h>
#include <xclass/OGC.h>
#include <xclass/OXClient.h>
#include <xclass/OString.h>

const OXFont *OXLabel::_defaultFont;
const OXGC *OXLabel::_defaultGC;
int OXLabel::_init = False;

//-----------------------------------------------------------------

OXLabel::OXLabel(const OXWindow *p, OString *text, OXFont *font,
                 unsigned int options, unsigned long back) :
    OXFrame(p, 1, 1, options, back) {

    if (!_init) {
      _defaultFont = GetResourcePool()->GetDefaultFont();
      _defaultGC = GetResourcePool()->GetFrameGC();
      _init = True;
    }

    _text  = text;
    _align = TEXT_CENTER_X | TEXT_CENTER_Y;
    _justify = TEXT_JUSTIFY_LEFT;
    _textFlags = 0;
    _3dStyle = LABEL_NORMAL;
    _wrapLength = -1;

    if (font) {
      _font = font;
      _normGC = new OXGC(_defaultGC);
      _normGC->SetFont(_font->GetId());
    } else {
      _font = _defaultFont;
      _normGC = (OXGC *) _defaultGC;
    }

    _ml = _mr = _mt = _mb = 0;

    _tl = _font->ComputeTextLayout(_text->GetString(), _text->GetLength(),
                                   _wrapLength, _justify, _textFlags,
                                   &_tw, &_th);
}

OXLabel::~OXLabel() {
  if (_text) delete _text;
  delete _tl;
  if (_normGC != _defaultGC) delete _normGC;
  if (_font != _defaultFont) _client->FreeFont((OXFont *) _font);
}

void OXLabel::SetText(OString *new_text) {
  if (new_text) {
    if (_text) delete _text;
    _text = new_text;
    Layout();
  }
}

void OXLabel::SetFont(const OXFont *f) {
  if (f) {
    const OXFont *oldfont = _font;
    _font = f;
    if (_normGC == _defaultGC) _normGC = new OXGC(_defaultGC);
    _normGC->SetFont(_font->GetId());
    if (oldfont != _defaultFont) _client->FreeFont((OXFont *) oldfont);
    Layout();
  }
}

void OXLabel::SetTextColor(unsigned int color) {
  if (_normGC == _defaultGC) _normGC = new OXGC(_defaultGC);
  _normGC->SetForeground(color);
  NeedRedraw(False);
}

void OXLabel::Set3DStyle(int style) {
  _3dStyle = style;
  NeedRedraw();
}

void OXLabel::Layout() {
  delete _tl;
  _tl = _font->ComputeTextLayout(_text->GetString(), _text->GetLength(),
                                 _wrapLength, _justify, _textFlags,
                                 &_tw, &_th);
  NeedRedraw();
}

void OXLabel::_DoRedraw() {
  int x, y;

  OXFrame::_DoRedraw();
    
  if (_align & TEXT_LEFT)
    x = _ml;
  else if (_align & TEXT_RIGHT)
    x = _w - _tw - _mr;
  else
    x = (_w - _tw + _ml - _mr) >> 1;

  if (_align & TEXT_TOP)
    y = _mt;
  else if (_align & TEXT_BOTTOM)
    y = _h - _th - _mb;
  else
    y = (_h - _th + _mt - _mb) >> 1;


  switch (_3dStyle) {
    case LABEL_RAISED:
    case LABEL_SUNKEN:
      {
      unsigned long forecolor = _normGC->GetForeground();
      unsigned long hi = _whitePixel;
      unsigned long sh = forecolor; //_shadowPixel;

      if (_3dStyle == LABEL_RAISED) { unsigned long t = hi; hi = sh; sh = t; }

      _normGC->SetForeground(hi);
      _DrawText(x+1, y+1);
      _normGC->SetForeground(sh);
      _DrawText(x, y);
      _normGC->SetForeground(forecolor);
      }
      break;

    case LABEL_SHADOW:
      {
      unsigned long forecolor = _normGC->GetForeground();

      _normGC->SetForeground(_shadowPixel);
      _DrawText(x+1, y+1);
      _normGC->SetForeground(forecolor);
      _DrawText(x, y);
      }
      break;

    default:
    case LABEL_NORMAL:
      _DrawText(x, y);
      break;
  }
}

void OXLabel::_DrawText(int x, int y) {
  _tl->DrawText(GetDisplay(), _id, _normGC->GetGC(), x, y, 0, -1);
  _tl->UnderlineChar(GetDisplay(), _id, _normGC->GetGC(), x, y, 
                     _text->GetHotIndex());
}
