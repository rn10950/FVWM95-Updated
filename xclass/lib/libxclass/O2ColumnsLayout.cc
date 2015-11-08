/**************************************************************************

    This file is part of xclass, a Win95-looking GUI toolkit.
    Copyright (C) 1996-2000 David Barth, Hector Peraza.

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

#include <xclass/O2ColumnsLayout.h>

//----------------------------------------------------------------------

O2ColumnsLayout::O2ColumnsLayout(OXCompositeFrame *main, int hsep, int vsep) {
  _main = main;
  _list = _main->GetListAddr();
  _hsep = hsep;
  _vsep = vsep;
}

void O2ColumnsLayout::Layout() {
  ODimension csize, c1size(0,0), c2size(0,0);
  OInsets ins = _main->GetInsets();
  int x = ins.l, y = ins.t, h, count = 0;
  SListFrameElt *ptr;

  for (ptr = *_list; ptr != NULL; ptr = ptr->next) {
    ++count;
    csize = ptr->frame->GetDefaultSize();
    c1size.w = max(c1size.w, csize.w);
    c1size.h = max(c1size.h, csize.h);
    ptr = ptr->next;
    if (ptr == NULL) break;
    csize = ptr->frame->GetDefaultSize();
    c2size.w = max(c2size.w, csize.w);
    c2size.h = max(c2size.h, csize.h);
  }

  h = max(c1size.h, c2size.h);

  for (ptr = *_list; ptr != NULL; ptr = ptr->next) {
    csize = ptr->frame->GetDefaultSize();
    ptr->frame->MoveResize(x, y+((h-csize.h)>>1), csize.w, csize.h);
    ptr->frame->Layout();
    ptr = ptr->next;
    if (ptr == NULL) break;
    csize = ptr->frame->GetDefaultSize();
    ptr->frame->MoveResize(x+_hsep+c1size.w, y+((h-csize.h)>>1), csize.w, csize.h);
    ptr->frame->Layout();
    y += h+_vsep;
  }
}

ODimension O2ColumnsLayout::GetDefaultSize() const {
  ODimension size, csize, c1size(0,0), c2size(0,0);
  OInsets ins = _main->GetInsets();
  int count = 0;
  SListFrameElt *ptr;

  for (ptr = *_list; ptr != NULL; ptr = ptr->next) {
    ++count;
    csize = ptr->frame->GetDefaultSize();
    c1size.w = max(c1size.w, csize.w);
    c1size.h = max(c1size.h, csize.h);
    ptr = ptr->next;
    if (ptr == NULL) break;
    csize = ptr->frame->GetDefaultSize();
    c2size.w = max(c2size.w, csize.w);
    c2size.h = max(c2size.h, csize.h);
  }

  size.w = c1size.w + _hsep + c2size.w + ins.l + ins.r;
  size.h = (max(c1size.h, c2size.h) + _vsep) * count + ins.t + ins.b;

  return size;
}
