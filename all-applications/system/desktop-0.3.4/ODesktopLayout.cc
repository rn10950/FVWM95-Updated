/**************************************************************************

    This file is part of Xclass95, a Win95-looking GUI toolkit.
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
#include <xclass/OXCompositeFrame.h>

#include "ODesktopLayout.h"


//-----------------------------------------------------------------

ODesktopLayout::ODesktopLayout(OXCompositeFrame *main, OXClient *c, int sep) {
  _main   = main;
  _client = c;
  _sep    = sep;
  _list   = _main->GetListAddr();
}

void ODesktopLayout::Layout() {
  SListFrameElt *ptr;
  int x, y, max_height;
  int xw, yw;
  ODimension csize, max_osize(0,0),
             msize(_client->GetDisplayWidth(), _client->GetDisplayHeight());
  unsigned int hints;

  for (ptr=*_list; ptr != NULL; ptr=ptr->next) {
    csize = ptr->frame->GetDefaultSize();
    max_osize.w = max(max_osize.w, csize.w);
    max_osize.h = max(max_osize.h, csize.h);
  }

  max_height = max(msize.h, max_osize.h + (_sep << 1));

  x = _sep; y = _sep << 1;
  for (ptr=*_list; ptr != NULL; ptr=ptr->next) {

    hints = ptr->layout->GetLayoutHints();
    csize = ptr->frame->GetDefaultSize();

    if (hints & LHINTS_CENTER_X)
      xw = x + ((max_osize.w - csize.w) >> 1);
    else if (hints & LHINTS_RIGHT)
      xw = x + max_osize.w - csize.w;
    else // defaults to LHINTS_LEFT
      xw = x;

    if (hints & LHINTS_CENTER_Y)
      yw = y + ((max_osize.h - csize.h) >> 1);
    else if (hints & LHINTS_BOTTOM)
      yw = y + max_osize.h - csize.h;
    else // defaults to LHINTS_TOP
      yw = y;

    ptr->frame->Move(xw, yw);
    y += max_osize.h + _sep + (_sep >> 1);

    if (y + max_osize.h > max_height) {
      y = _sep << 1;
      x += max_osize.w + _sep;
    }

  }
}

ODimension ODesktopLayout::GetDefaultSize() const {
  SListFrameElt *ptr;
  int x, y;
  ODimension max_size, max_osize(0,0),
             msize(_client->GetDisplayWidth(), _client->GetDisplayHeight());

  for (ptr=*_list; ptr != NULL; ptr=ptr->next) {
    max_size = ptr->frame->GetDefaultSize();
    max_osize.w = max(max_osize.w, max_size.w);
    max_osize.h = max(max_osize.h, max_size.h);
  }

  max_size.h = max(msize.h, max_osize.h + (_sep << 1));

  x = _sep; y = _sep << 1;
  for (ptr=*_list; ptr != NULL; ptr=ptr->next) {
    y += max_osize.h + _sep + (_sep >> 1);
    if (y + max_osize.h > max_size.h) {
      y = _sep << 1;
      x += max_osize.w + _sep;
    }
  }
  if (y != (_sep << 1)) x += max_osize.w + _sep;
  max_size.w = max(x, msize.w);

  return max_size;
}
