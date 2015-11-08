/**************************************************************************

    This file is part of xcgview, a xclass port of Ghostview 1.5
    Copyright (C) 1998 Hector Peraza.

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

#include <xclass/ODimension.h>
#include <xclass/OXClient.h>
#include <xclass/OString.h>
#include <xclass/OGC.h>
#include <xclass/OResourcePool.h>
#include <xclass/utils.h>

#include "OXPageLBEntry.h"


//-----------------------------------------------------------------

OXPageLBEntry::OXPageLBEntry(const OXWindow *p, OString *s, int ID) :
  OXTextLBEntry(p, s, ID) {

  _flags = 0;
}

void OXPageLBEntry::SetMark(int mark_type, int mode) {
  if (mode)
    _flags |= mark_type;
  else
    _flags &= ~mark_type;

  NeedRedraw();
}

void OXPageLBEntry::_DoRedraw() {
  int x, y;

  x = _w - _tw - 6;
  y = (_h - _th) >> 1;

  if (_active) {
    SetBackgroundColor(_defaultSelectedBackground);
    ClearWindow();
    _normGC->SetForeground(_selPixel);
  } else {
    SetBackgroundColor(_defaultDocumentBackground);
    ClearWindow();
    _normGC->SetForeground(_defaultDocumentForeground);
  }

  _text->Draw(GetDisplay(), _id, _normGC->GetGC(), x, y + _ta);
  if (_flags & PLBE_CHECKMARK) DrawCheckMark(_normGC->GetGC());
  if (_flags & PLBE_RADIOMARK) DrawRCheckMark(_normGC->GetGC());

  if (_focused) {
    DrawRectangle(GetResourcePool()->GetDocumentBckgndGC()->GetGC(),  // ==!== doc back color
                  0, 0, _w-1, _h-1);
    DrawRectangle(GetResourcePool()->GetFocusHiliteGC()->GetGC(),
                  0, 0, _w-1, _h-1);
  }
}

void OXPageLBEntry::DrawCheckMark(GC gc) {
  XSegment seg[6];

  int l = 2;
  int t = (_h - 8) >> 1;

  seg[0].x1 = 1+l; seg[0].y1 = 3+t; seg[0].x2 = 3+l; seg[0].y2 = 5+t;
  seg[1].x1 = 1+l; seg[1].y1 = 4+t; seg[1].x2 = 3+l; seg[1].y2 = 6+t;
  seg[2].x1 = 1+l; seg[2].y1 = 5+t; seg[2].x2 = 3+l; seg[2].y2 = 7+t;
  seg[3].x1 = 3+l; seg[3].y1 = 5+t; seg[3].x2 = 7+l; seg[3].y2 = 1+t;
  seg[4].x1 = 3+l; seg[4].y1 = 6+t; seg[4].x2 = 7+l; seg[4].y2 = 2+t;
  seg[5].x1 = 3+l; seg[5].y1 = 7+t; seg[5].x2 = 7+l; seg[5].y2 = 3+t;

  DrawSegments(gc, seg, 6);
}

void OXPageLBEntry::DrawRCheckMark(GC gc) {
  XSegment seg[5];

  int t = (_h - 5) >> 1;
  int l = _w - 7;

  seg[0].x1 = 1+l; seg[0].y1 = 0+t; seg[0].x2 = 3+l; seg[0].y2 = 0+t;
  seg[1].x1 = 0+l; seg[1].y1 = 1+t; seg[1].x2 = 4+l; seg[1].y2 = 1+t;
  seg[2].x1 = 0+l; seg[2].y1 = 2+t; seg[2].x2 = 4+l; seg[2].y2 = 2+t;
  seg[3].x1 = 0+l; seg[3].y1 = 3+t; seg[3].x2 = 4+l; seg[3].y2 = 3+t;
  seg[4].x1 = 1+l; seg[4].y1 = 4+t; seg[4].x2 = 3+l; seg[4].y2 = 4+t;

  DrawSegments(gc, seg, 5);
}
