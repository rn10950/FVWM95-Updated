/**************************************************************************
 
    This file is part of xclass.
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


#include <stdio.h>

#include <xclass/OXCompositeFrame.h>
#include <xclass/utils.h>

#include "OPackedMatrixLayout.h"


//----------------------------------------------------------------------

// Arranges frames in a packed matrix-like way.
// This manager provides:
//  - a column number (0 means unlimited)
//  - a row number (0 means unlimited)
//  - horizontal & vertical separators
//
// Notes:
//  - If both column and row are fixed values, any remaining frames
//    outside the count won't be managed
//  - Unlimited rows means the frame can expand downward (the default
//    behaviour in most UI)
//  - Both unlimied rows and columns is undefined (read: will crash 
//    the algorithm ;-)
//  - With fixed dimensions, frames are always arranged in rows.
//    That is: 1st frame is at position (0,0), next one is at row(0),
//    column(1) and so one...
//  - When specifying one dimension as unlimited (i.e. row = 0 or 
//    column = 0) the frames are arranged according to the direction
//    of the fixed dimension

OPackedMatrixLayout::OPackedMatrixLayout(class OXCompositeFrame *main,
                                         int r, int c, int s, int h) { 
  _main = main;
  _list = _main->GetListAddr();
  sep   = s;
  hints = h;
  rows  = r;
  columns = c;
}

ODimension OPackedMatrixLayout::GetDefaultSize() const {
  ODimension csize;
  SListFrameElt *ptr;
  int colcount = 0, rowcount = 0;
  int maxw[16], maxh[16];

  // init the max row and columns fields

  memset(&maxw, 0, 16 * sizeof(int));
  memset(&maxh, 0, 16 * sizeof(int));

  // calc max width in each column and max height in each row

  for (ptr = *_list; ptr != NULL; ptr = ptr->next) {
    csize = ptr->frame->GetDefaultSize();
    maxw[colcount] = max(maxw[colcount], csize.w);
    maxh[rowcount] = max(maxh[rowcount], csize.h);
    if (rows == 0) {
      colcount = (colcount + 1) % columns;
      if (colcount == 0) rowcount++;
    } else {
      rowcount = (rowcount + 1) % rows;
      if (rowcount == 0) {
        colcount++;
        if (columns != 0 && colcount > columns) break;
      }
    }
  }
  
  // sum up the widths and the heights and don't forget the separator

  ODimension size(sep, sep);
  int i;

  if (colcount == 0) colcount = columns;
  for (i = 0; i < colcount; i++) size.w += maxw[i] + sep;

  if (rowcount == 0) rowcount = rows;
  for (i = 0; i < rowcount; i++) size.h += maxh[i] + sep;

  return size;
}

void OPackedMatrixLayout::Layout() {
  ODimension csize;
  SListFrameElt *ptr;
  int x = sep, y = sep;
  int rowcount = 0, colcount = 0;
  int maxw[16], maxh[16];

  // init the max row and columns fields

  memset(&maxw, 0, 16 * sizeof(int));
  memset(&maxh, 0, 16 * sizeof(int));

  // calc max width in each column and max height in each row

  for (ptr = *_list; ptr != NULL; ptr = ptr->next) {
    csize = ptr->frame->GetDefaultSize();
    maxw[colcount] = max(maxw[colcount], csize.w);
    maxh[rowcount] = max(maxh[rowcount], csize.h);
    if (rows == 0) {
      colcount = (colcount + 1) % columns;
      if (colcount == 0) rowcount++;
    } else {
      rowcount = (rowcount + 1) % rows;
      if (rowcount == 0) {
        colcount++;
        if (columns != 0 && colcount > columns) break;
      }
    }
  }

  // move each frame to the new position and layout the frame

  rowcount = colcount = 0;
  for (ptr = *_list; ptr != NULL; ptr = ptr->next) {
    ODimension size = ptr->frame->GetDefaultSize();
    ptr->frame->MoveResize(x, y, size.w, size.h);
    ptr->frame->Layout();
    if (rows == 0) {
      x += maxw[colcount++] + sep;
      colcount %= columns;
      if (colcount == 0) {
        x = sep;
        y += maxh[rowcount++] + sep;
      }
    } else {
      y += maxh[rowcount++] + sep;
      rowcount %= rows;
      if (rowcount == 0) {
        y = sep;
        x += maxw[colcount++] + sep;
        if (columns != 0 && colcount > columns) return;
      }
    }
  }
}
