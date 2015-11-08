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
#include <xclass/OXCompositeFrame.h>


//--------------------------------------------------------------------

// Arranges frames in a matrix-like way
// This manager provides :
// - a column number (0 means unlimited)
// - a row number (0 means unlimited)
// - horizontal & vertical separators
//
// Notes : If both column and row are fixed values, any remaining frames
//         outside the count won't be managed
//         Unlimited rows means the frame can expand downward (the default
//         behaviour in most UI)
//         Both unlimied rows and columns is undefined (read: will crash 
//         the algorithm ;-)
//         With fixed dimensions, frames are always arranged in rows.
//         That is: 1st frame is at position (0,0), next one is at row(0),
//         column(1) and so one...
//         When specifying one dimension as unlimited (i.e. row=0 or 
//         column=0) the frames are arranged according to the direction
//         of the fixed dimension

OMatrixLayout::OMatrixLayout(class OXCompositeFrame *main,
                             int r, int c, int s, int h) { 
  _main = main;
  _list = _main->GetListAddr();
  sep   = s;
  hints = h;
  rows  = r;
  columns = c;
}

ODimension OMatrixLayout::GetDefaultSize() const {
  ODimension size, csize, maxsize(0,0);
  SListFrameElt *ptr;
  int count = 0;

  for (ptr=*_list; ptr != NULL; ptr=ptr->next) {
    count++;
    csize = ptr->frame->GetDefaultSize();
    maxsize.w = max(maxsize.w, csize.w);
    maxsize.h = max(maxsize.h, csize.h);
  }

  if (rows == 0) {
    size.w = columns * (maxsize.w + sep) + sep;
    size.h = ((count - 1) / columns + 1) * (maxsize.h + sep) + sep;
  } else if (columns == 0) {
    size.w = ((count - 1) / rows + 1) * (maxsize.w + sep) + sep;
    size.h = rows * (maxsize.h + sep) + sep;
  } else {
    size.w = columns * (maxsize.w + sep) + sep;
    size.h = rows * (maxsize.h + sep) + sep;
  }
  return size;
}

void OMatrixLayout::Layout() {
  ODimension csize, maxsize(0,0);
  SListFrameElt *ptr;
  int x = sep, y = sep;
  int rowcount = rows, colcount = columns;

  for (ptr=*_list; ptr != NULL; ptr=ptr->next) {
    csize = ptr->frame->GetDefaultSize();
    maxsize.w = max(maxsize.w, csize.w);
    maxsize.h = max(maxsize.h, csize.h);
  }

  for (ptr=*_list; ptr != NULL; ptr=ptr->next) {
    ODimension size = ptr->frame->GetDefaultSize();
    ptr->frame->MoveResize(x, y, size.w, size.h);
    ptr->frame->Layout();

    if (columns == 0) {
      y += maxsize.h + sep;
      rowcount--;
      if (rowcount <= 0) {
        rowcount = rows;
        y = sep; x += maxsize.w + sep;
      }
    } else if (rowcount == 0) {
      x += maxsize.w + sep;
      colcount--;
      if (colcount <= 0) {
        colcount = columns;
        x = sep; y += maxsize.h + sep;
      }
    } else {
      x += maxsize.w + sep;
      colcount--;
      if (colcount <= 0) {
        colcount = rows; rowcount--;
        x = sep; y += maxsize.h + sep;
        if (rowcount <= 0) return;
      }
    }
  }
}
