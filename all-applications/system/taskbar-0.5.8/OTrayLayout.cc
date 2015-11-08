/**************************************************************************

    This file is part of taskbar.
    Copyright (C) 2000, Hector Peraza.

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

#include <xclass/OXFrame.h>

#include "OTrayLayout.h"


#define MIN_BUTTON_WIDTH 32
#define MAX_BUTTON_WIDTH 180


//----------------------------------------------------------------------

OTrayLayout::OTrayLayout(OXCompositeFrame *main,
                         int rowh, int rows, int sep) {
  _main = main;
  _list = _main->GetListAddr();
  _sep = sep;
  _rows = rows;
  _rowHeight = rowh;
}

ODimension OTrayLayout::GetDefaultSize() const {
  ODimension size, msize = _main->GetSize();

  size.w = msize.w;
  size.h = 4;

  for (ptr=*_list, count = 0; ptr != NULL; ptr=ptr->next) {
    if (ptr->frame->IsVisible()) {
      size.w += ptr->frame->GetDefaultSize().w + _sep;
    }
  }

//  if (_rows == 1)
    size.h = _rowHeight;
//  else
//    size.h = _rows * (_rowHeight + _sep) - _sep;

  return size;
}

void OTrayLayout::Layout() {
  SListFrameElt *ptr;
  int x, y;
  ODimension msize = _main->GetSize();

  x = 2;
  for (ptr=*_list; ptr != NULL; ptr=ptr->next) {
    if (ptr->frame->IsVisible()) {
      ODimension size = ptr->frame->GetDefaultSize();
      y = (msize.h - size.h) >> 2;
      ptr->frame->MoveResize(x, y, size.w, _rowHeight-2);
      x += size.w + _sep;
    }
  }
}
