/**************************************************************************

    This file is part of xclass.
    Copyright (C) 1996, 1997 David Barth, Hector Peraza.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

**************************************************************************/

#include <xclass/OXStatusBar.h>


//-------------------------------------------------------------------

OXStatusBar::OXStatusBar(const OXWindow *p, int w, int h,
              unsigned int options, unsigned long back) :
  OXHorizontalFrame(p, w, h, options, back) {

    _nlabels = 1;

    OXStatusLabel *l = new OXStatusLabel(this, new OString(""), 100);
    l->ChangeOptions(l->GetOptions() | SUNKEN_FRAME | FIXED_WIDTH);
    l->SetMargins(4, 4, 1, 1);
    l->SetTextAlignment(TEXT_LEFT);
    AddFrame(l, new OLayoutHints(LHINTS_LEFT | LHINTS_EXPAND_X | LHINTS_TOP));
}

OXStatusBar::~OXStatusBar() {
  SListFrameElt *ptr;

  for (ptr = _flist; ptr; ptr = ptr->next) delete ptr->layout;
}

void OXStatusBar::AddLabel(int width, int hints) {
  SListFrameElt *ptr;

  for (ptr = _flist; ptr; ptr = ptr->next) {
    OXStatusLabel *l = (OXStatusLabel *) ptr->frame;
    if (ptr->layout->GetLayoutHints() & LHINTS_EXPAND_X) {
      int lm, rm;

      delete ptr->layout;
      lm = rm = 0;
      if (l->_hints & LHINTS_LEFT) rm = 3; else lm = 3;
      ptr->layout = new OLayoutHints(l->_hints | LHINTS_TOP, lm, rm, 0, 0);
    }
  }
  OXStatusLabel *l = new OXStatusLabel(this, new OString(""), width, hints);
  l->ChangeOptions(l->GetOptions() | SUNKEN_FRAME | FIXED_WIDTH);
  l->SetMargins(4, 4, 1, 1);
  l->SetTextAlignment(TEXT_LEFT);
  AddFrame(l, new OLayoutHints(LHINTS_LEFT | LHINTS_EXPAND_X | LHINTS_TOP));
  _nlabels++;
  Layout();
  MapSubwindows();
}

void OXStatusBar::SetText(int num, OString *text) {
  int i;
  SListFrameElt *ptr;

  for (i = 0, ptr = _flist; ptr; ++i, ptr = ptr->next) {
    if (i == num) {
      OXStatusLabel *l = (OXStatusLabel *) ptr->frame;
      l->SetText(text);
      Layout();
      break;
    }
  }
}

void OXStatusBar::SetWidth(int num, int width) {
  int i;
  SListFrameElt *ptr;

  for (i = 0, ptr = _flist; ptr; ++i, ptr = ptr->next) {
    if (i == num) {
      OXStatusLabel *l = (OXStatusLabel *) ptr->frame;
      l->_width = width;
      Layout();
      break;
    }
  }
}

void OXStatusBar::SetAlignment(int num, int hints) {
  int i;
  SListFrameElt *ptr;

  for (i = 0, ptr = _flist; ptr; ++i, ptr = ptr->next) {
    if (i == num) {
      OXStatusLabel *l = (OXStatusLabel *) ptr->frame;
      l->_hints = hints;
      if ((ptr->layout->GetLayoutHints() & LHINTS_EXPAND_X) == 0) {
        int lm, rm;

        delete ptr->layout;
        lm = rm = 0;
        if (l->_hints & LHINTS_LEFT) rm = 3; else lm = 3;
        ptr->layout = new OLayoutHints(l->_hints | LHINTS_TOP, lm, rm, 0, 0);
        Layout();
      }
      break;
    }
  }
}
