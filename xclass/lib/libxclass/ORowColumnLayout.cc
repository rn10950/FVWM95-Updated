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

void ORowLayout::Layout() {
  SListFrameElt *ptr;
  ODimension size;
  OInsets ins = _main->GetInsets();
  int x = ins.l, y = ins.t;
 
  for (ptr = *_list; ptr != NULL; ptr = ptr->next) {
    if (ptr->frame->IsVisible()) {
      size = ptr->frame->GetDefaultSize();
      ptr->frame->MoveResize(x, y, size.w, size.h);
      ptr->frame->Layout();
      x += size.w + sep;
    }
  }
}

ODimension ORowLayout::GetDefaultSize() const {
  ODimension size(0,0), dsize, msize = _main->GetSize();
  OInsets ins = _main->GetInsets();
  SListFrameElt *ptr;
  int options = _main->GetOptions();
  
  if ((options & FIXED_HEIGHT) && (options & FIXED_WIDTH))
    return msize;
 
  for (ptr = *_list; ptr != NULL; ptr = ptr->next) {
    if (ptr->frame->IsVisible()) {
      dsize   = ptr->frame->GetDefaultSize();
      size.h  = max(size.h, dsize.h);
      size.w += dsize.w + sep;
    }
  }

  size.h += ins.t + ins.b;
  size.w += ins.l + ins.r;
  size.w -= sep;

  if (options & FIXED_HEIGHT) size.h = msize.h;
  if (options & FIXED_WIDTH)  size.w = msize.w;

  return size;
}

//--------------------------------------------------------------------

void OColumnLayout::Layout() {
  SListFrameElt *ptr;
  ODimension size;
  OInsets ins = _main->GetInsets();
  int x = ins.l, y = ins.t;
 
  for (ptr = *_list; ptr != NULL; ptr = ptr->next) {
    if (ptr->frame->IsVisible()) {
      size = ptr->frame->GetDefaultSize();
      ptr->frame->MoveResize(x, y, size.w, size.h);
      ptr->frame->Layout();
      y += size.h + sep;
    }
  }
}

ODimension OColumnLayout::GetDefaultSize() const {
  ODimension size(0,0), dsize, msize = _main->GetSize();
  OInsets ins = _main->GetInsets();
  SListFrameElt *ptr;
  int options = _main->GetOptions();
  
  if (options & FIXED_HEIGHT && options & FIXED_WIDTH)
    return msize;

  for (ptr = *_list; ptr != NULL; ptr = ptr->next) {
    if (ptr->frame->IsVisible()) {
      dsize   = ptr->frame->GetDefaultSize();
      size.h += dsize.h + sep;
      size.w  = max(size.w, dsize.w);
    }
  }

  size.h += ins.t + ins.b;
  size.h -= sep;
  size.w += ins.l + ins.r;

  if (options & FIXED_HEIGHT) size.h = msize.h;
  if (options & FIXED_WIDTH)  size.w = msize.w;

  return size;
}
