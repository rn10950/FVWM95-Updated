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
#include <xclass/OXGroupFrame.h>
#include <xclass/OResourcePool.h>
#include <xclass/OXFont.h>
#include <xclass/OGC.h>
#include <xclass/OString.h>


const OXFont *OXGroupFrame::_defaultFont;
const OXGC *OXGroupFrame::_defaultGC;
int   OXGroupFrame::_init = False;

//-----------------------------------------------------------------

OXGroupFrame::OXGroupFrame(const OXWindow *p, OString *title,
                           unsigned int options, unsigned long back) :
  OXCompositeFrame(p, 1, 1, options, back) {

    if (!_init) {
      _defaultFont = GetResourcePool()->GetDefaultFont();
      _defaultGC = GetResourcePool()->GetFrameGC();
      _init = True;
    }

    _text = title;  
    _font = _defaultFont;
    _normGC = (OXGC *) _defaultGC;

    _bw = _font->TextHeight() + 1;

    _old_insets = True;
    _ComputeInsets();
}

OXGroupFrame::~OXGroupFrame() {
  delete _text;
  if (_normGC != _defaultGC) delete _normGC;
  if (_font != _defaultFont) _client->FreeFont((OXFont *) _font);
}

void OXGroupFrame::OldInsets(int onoff) {
  if (_old_insets != onoff) {
    _old_insets = onoff;
    _ComputeInsets();
  }
}

void OXGroupFrame::DrawBorder() {
  int x, y, tw, l, t, r, b, gl, gr, sep;
  OFontMetrics fm;

  tw = _font->TextWidth(_text->GetString(), _text->GetLength());
  _font->GetFontMetrics(&fm);

  l = 0;
  t = (_font->TextHeight() + 2) >> 1;
  r = _w - 1;
  b = _h - 1;

  sep = 3;
  gl = 5 + sep;
  gr = gl + tw + (sep << 1);

  DrawLine(_shadowGC,  l,   t,   gl,  t);
  DrawLine(_hilightGC, l+1, t+1, gl,  t+1);
  DrawLine(_shadowGC,  gr,  t,   r-1, t);
  DrawLine(_hilightGC, gr,  t+1, r-2, t+1);

  DrawLine(_shadowGC,  r-1, t,   r-1, b-1);
  DrawLine(_hilightGC, r,   t,   r,   b);

  DrawLine(_shadowGC,  r-1, b-1, l,   b-1);
  DrawLine(_hilightGC, r,   b,   l,   b);

  DrawLine(_shadowGC,  l,   b-1, l,   t);
  DrawLine(_hilightGC, l+1, b-2, l+1, t+1);

  x = gl + sep;
  y = fm.ascent + 1;

  if (!IsEnabled()) {
    unsigned long forecolor = _normGC->GetForeground();

    _normGC->SetForeground(_hilitePixel);
    _text->Draw(GetDisplay(), _id, _normGC->GetGC(), x+1, y+1);
    _normGC->SetForeground(_shadowPixel);
    _text->Draw(GetDisplay(), _id, _normGC->GetGC(), x, y);
    _normGC->SetForeground(forecolor);
  } else {
    _text->Draw(GetDisplay(), _id, _normGC->GetGC(), x, y);
  }
}

void OXGroupFrame::_Enable(int onoff) {
  OXFrame::_Enable(onoff);
  NeedRedraw();
}

void OXGroupFrame::_ComputeInsets() {
  if (_old_insets) {
    _insets = OInsets(_bw, _bw, _bw, _bw);
  } else {
    _insets = OInsets(2, 2, _font->TextHeight() + 1, 2);
  }
}
