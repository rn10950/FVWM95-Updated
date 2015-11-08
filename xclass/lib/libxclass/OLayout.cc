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

OVerticalLayout::OVerticalLayout(class OXCompositeFrame *main) { 
  _main = main; 
  _list = _main->GetListAddr();
}

void OVerticalLayout::Layout() {
  SListFrameElt *ptr;
  OLayoutHints *layout;
  int nb_expand = 0;
  int size_expand, esize_expand, rem_expand, tmp_expand = 0;
  int top, bottom;
  unsigned long hints;
  unsigned int extra_space;
  int exp = 0;
  int exp_max = 0;
  int remain;
  int x = 0, y = 0;
  OInsets ins = _main->GetInsets();
  ODimension size, csize;
  ODimension msize = _main->GetSize();
  int pad_left, pad_top, pad_right, pad_bottom;

  if (*_list == NULL) return;

  top = ins.t;
  bottom = msize.h - ins.b;
  remain = msize.h - ins.t - ins.b;

  for (ptr = *_list; ptr != NULL; ptr = ptr->next) {

    if (ptr->frame->IsVisible()) {
      layout = ptr->layout;
      size = ptr->frame->GetDefaultSize();
      size.h += layout->GetPadTop() + layout->GetPadBottom();
      hints = layout->GetLayoutHints();
      if ((hints & LHINTS_EXPAND_Y) || (hints & LHINTS_CENTER_Y)) {
	nb_expand++;
	exp += size.h;
	exp_max = max(exp_max, size.h);
      } else
	remain -= size.h;
    }
  }

  if (nb_expand) {
    size_expand = remain / nb_expand;
    if (size_expand < exp_max)
      esize_expand = (remain - exp) / nb_expand; // defined only if needed
    rem_expand = remain % nb_expand;
  }

  for (ptr = *_list; ptr != NULL; ptr = ptr->next) {

    if (ptr->frame->IsVisible()) {

      hints = (layout = ptr->layout)->GetLayoutHints();
      csize      = ptr->frame->GetDefaultSize();
      pad_left   = layout->GetPadLeft();
      pad_top    = layout->GetPadTop();
      pad_right  = layout->GetPadRight();
      pad_bottom = layout->GetPadBottom();

      if (hints & LHINTS_RIGHT)
	x = msize.w - ins.r - csize.w - pad_right;
      else if (hints & LHINTS_CENTER_X)
	x = (msize.w - csize.w) >> 1; 
      else // defaults to LHINTS_LEFT
	x = pad_left + ins.l;
      
      if (hints & LHINTS_EXPAND_X) {
	size.w = msize.w - ins.l - ins.r - pad_left - pad_right;
	x = pad_left + ins.l;
      } else {
	size.w = csize.w;
      }

      if (hints & LHINTS_EXPAND_Y) {
//	if (size_expand >= exp_max)
	  size.h = size_expand - pad_top - pad_bottom;
//	else
//	  size.h = csize.h + esize_expand;
	tmp_expand += rem_expand;
	if (tmp_expand >= nb_expand) {
	  size.h++;
	  tmp_expand -= nb_expand;
	}
      } else {
	size.h = csize.h;
	if (hints & LHINTS_CENTER_Y) {
	  if (size_expand >= exp_max)
	    extra_space = (size_expand - pad_top - pad_bottom - size.h) >> 1;
	  else
	    extra_space = (esize_expand) >> 1;
	  y += extra_space;
	  top += extra_space;
	}
      }

      if (hints & LHINTS_BOTTOM) {
	y = bottom - size.h - pad_bottom;
	bottom -= size.h + pad_top + pad_bottom;
      } else { // LHINTS_TOP by default
	y = top + pad_top;
	top += size.h + pad_top + pad_bottom;
      }

      if (hints & LHINTS_CENTER_Y)
	top += extra_space;

      ptr->frame->MoveResize(x, y, size.w, size.h);
    }
  }
}

ODimension OVerticalLayout::GetDefaultSize() const {
  SListFrameElt *ptr;
  ODimension size(0,0), msize = _main->GetSize(), csize;
  OInsets ins = _main->GetInsets();
  int options = _main->GetOptions();

  if ((options & FIXED_WIDTH) && (options & FIXED_HEIGHT))
    return msize;

  for (ptr = *_list; ptr != NULL; ptr = ptr->next) {
    if (ptr->frame->IsVisible()) {
      csize = ptr->frame->GetDefaultSize();
      size.w = max(size.w, csize.w + ptr->layout->GetPadLeft() +
                                     ptr->layout->GetPadRight());
      size.h += csize.h + ptr->layout->GetPadTop() +
                          ptr->layout->GetPadBottom();
    }
  }

  size.w += ins.l + ins.r;
  size.h += ins.t + ins.b;

  if (options & FIXED_WIDTH) size.w = msize.w;
  if (options & FIXED_HEIGHT) size.h = msize.h;

  return size;
}

void OHorizontalLayout::Layout() {
  SListFrameElt *ptr;
  OLayoutHints *layout;
  int nb_expand = 0;
  int left, right;
  unsigned long hints;
  unsigned int extra_space;
  int exp = 0;
  int exp_max = 0;
  int size_expand, esize_expand, rem_expand, tmp_expand = 0;
  int remain;
  int x = 0, y = 0;
  OInsets ins = _main->GetInsets();
  ODimension size, csize;
  ODimension msize = _main->GetSize();
  int pad_left, pad_top, pad_right, pad_bottom;

  if (*_list == NULL) return;

  left   = ins.l;
  right  = msize.w - ins.r;
  remain = msize.w - ins.l - ins.r;

  for (ptr = *_list; ptr != NULL; ptr = ptr->next) {

    if (ptr->frame->IsVisible()) {
      layout  = ptr->layout;
      size    = ptr->frame->GetDefaultSize();
      size.w += layout->GetPadLeft() + layout->GetPadRight();
      hints = layout->GetLayoutHints();
      if ((hints & LHINTS_EXPAND_X) || (hints & LHINTS_CENTER_X)) {
	nb_expand++;
	exp += size.w;
	exp_max = max(exp_max, size.w);
      } else
	remain -= size.w;
    }
  }

  if (nb_expand) {
    size_expand = remain / nb_expand;
    if (size_expand < exp_max)
      esize_expand = (remain - exp) / nb_expand; // defined only if needed
    rem_expand = remain % nb_expand;
  }

  for (ptr = *_list; ptr != NULL; ptr = ptr->next) {

    if (ptr->frame->IsVisible()) {

      hints = (layout = ptr->layout)->GetLayoutHints();
      csize      = ptr->frame->GetDefaultSize();
      pad_left   = layout->GetPadLeft();
      pad_top    = layout->GetPadTop();
      pad_right  = layout->GetPadRight();
      pad_bottom = layout->GetPadBottom();

      if (hints & LHINTS_BOTTOM)
	y = msize.h - ins.b - csize.h - pad_bottom;
      else if (hints & LHINTS_CENTER_Y)
	y = (msize.h - csize.h) >> 1;
      else // LHINTS_TOP by default
	y = pad_top + ins.t;

      if (hints & LHINTS_EXPAND_Y) {
	size.h = msize.h - ins.t - ins.b - pad_top - pad_bottom;
	y = pad_top + ins.t;
      } else {
	size.h = csize.h;
      }

      if (hints & LHINTS_EXPAND_X) {
//	if (size_expand >= exp_max)
	  size.w = size_expand - pad_left - pad_right;
//	else
//	  size.w = csize.w + esize_expand;
	tmp_expand += rem_expand;
	if (tmp_expand >= nb_expand) {
	  size.w++;
	  tmp_expand -= nb_expand;
	}
      } else {
	size.w = csize.w;
	if (hints & LHINTS_CENTER_X) {
	  if (size_expand >= exp_max)
	    extra_space = (size_expand - pad_left - pad_right - size.w) >> 1;
	  else
	    extra_space = esize_expand >> 1;
	  x += extra_space;
	  left += extra_space;
	}
      }

      if (hints & LHINTS_RIGHT) {
	x = right - size.w - pad_right;
	right -= size.w + pad_left + pad_right;
      } else { // defaults to LHINTS_LEFT
	x = left + pad_left;
	left += size.w + pad_left + pad_right;
      }

      if (hints & LHINTS_CENTER_X)
	left += extra_space;

      ptr->frame->MoveResize(x, y, size.w, size.h);
    }
  }
}

ODimension OHorizontalLayout::GetDefaultSize() const {
  SListFrameElt *ptr;
  ODimension size(0,0), msize = _main->GetSize(), csize;
  OInsets ins = _main->GetInsets();
  int options = _main->GetOptions();

  if ((options & FIXED_WIDTH) && (options & FIXED_HEIGHT))
    return msize;

  for (ptr = *_list; ptr != NULL; ptr = ptr->next) {
    if (ptr->frame->IsVisible()) {
      csize = ptr->frame->GetDefaultSize();
      size.w += csize.w + ptr->layout->GetPadLeft() +
                          ptr->layout->GetPadRight();
      size.h = max(size.h, csize.h + ptr->layout->GetPadTop() +
                                     ptr->layout->GetPadBottom());
    }
  }

  size.w += ins.l + ins.r;
  size.h += ins.t + ins.b;

  if (options & FIXED_WIDTH) size.w = msize.w;
  if (options & FIXED_HEIGHT) size.h = msize.h;

  return size;
}
