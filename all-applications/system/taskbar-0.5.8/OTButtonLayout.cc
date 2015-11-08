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

#include <xclass/OXCompositeFrame.h>

#include "OTButtonLayout.h"


#define MIN_BUTTON_WIDTH 32
#define MAX_BUTTON_WIDTH 180


//----------------------------------------------------------------------

OTButtonLayout::OTButtonLayout(OXCompositeFrame *main,
                               int rowh, int rows, int sep) {
  _main = main;
  _list = _main->GetListAddr();
  _sep = sep;
  _rows = (rows < 1) ? 1 : rows;
  _rowHeight = rowh;
}

ODimension OTButtonLayout::GetDefaultSize() const {
  ODimension size, msize = _main->GetSize();

  size.w = msize.w;

  if (_rows == 1)
    size.h = _rowHeight;
  else
    size.h = _rows * (_rowHeight + _sep) - _sep;

  return size;
}

void OTButtonLayout::Layout() {
  SListFrameElt *ptr;
  int count, tw, x, y, n;
  ODimension msize = _main->GetSize();

  for (ptr=*_list, count = 0; ptr != NULL; ptr=ptr->next) {
    if (ptr->frame->IsVisible()) ++count;
  }

  if (count == 0) {
    tw = msize.w;
  } else if (_rows == 1) {
    tw = msize.w / count;
  } else {
    tw = msize.w / ((count / _rows) + 1);
  }

  if (tw > MAX_BUTTON_WIDTH) tw = MAX_BUTTON_WIDTH;
  if (tw < MIN_BUTTON_WIDTH) tw = MIN_BUTTON_WIDTH;

  x = y = 0;
  n = 1;

  for (ptr=*_list; ptr != NULL; ptr=ptr->next) {
    if (ptr->frame->IsVisible()) {
      if ((x + tw > msize.w) && (n < _rows)) {
        x = 0;
        y += _rowHeight + _sep;
        ++n;
      }
      ptr->frame->MoveResize(x, y, tw-_sep-1, _rowHeight);
      x += tw;
    }
  }
}
