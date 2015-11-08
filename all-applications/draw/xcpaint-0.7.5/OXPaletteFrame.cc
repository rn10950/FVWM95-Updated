/**************************************************************************

    This file is part of xcpaint, a XPM pixmap editor.
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

#include <stdlib.h>
#include <stdio.h>

#include "OColorTable.h"
#include "OXPaletteFrame.h"
#include "OXPaintCanvas.h"

#include "weave.xpm"


//---------------------------------------------------------------------

OXPaletteFrame::OXPaletteFrame(const OXWindow *p, int w, int h, 
                               unsigned int options, unsigned long back) :
  OXCompositeFrame(p, w, h, options, back) {

    SetLayoutManager(new OMatrixLayout(this, 2, 0, 0, 0));

    _tpic = _client->GetPicture("weave.xpm", weave_xpm);

    XGrabButton(GetDisplay(), AnyButton, AnyModifier, _id, True,
                ButtonPressMask | ButtonReleaseMask /*| PointerMotionMask*/,
                GrabModeAsync, GrabModeAsync, None, None);
}

OXPaletteFrame::~OXPaletteFrame() {
}

void OXPaletteFrame::UpdateColors(const OColorTable *ct) {
  int i;
  const OColorInfo **ci = ct->GetTable();

  _RemoveEntries();

  if (ct->IsBTree()) {
    // color table is a B-tree
    _AddBTreeEntries(*ci);
  } else {
    // color table is an array
    for (i = 0; i <= 256; ++i) if (ci[i]) _AddEntry(ci[i]);
  }

  Layout();
  MapSubwindows();
}

void OXPaletteFrame::_AddEntry(const OColorInfo *ci) {
  unsigned int pixel;
  OXColorEntry *f;
  char tmp[256];

  if (ci) {
    pixel = ci->pixel;
    f = new OXColorEntry(this, 13, 13, SUNKEN_FRAME | OWN_BKGND,
                         0, pixel);
    if (pixel == TRANSPARENT(GetDisplay(), _client->GetScreenNumber()))
      f->SetBackgroundPixmap(_tpic->GetPicture());
    else
      f->SetBackgroundColor(pixel);
    if (ci->c_name)
      strcpy(tmp, ci->c_name);
    else
      sprintf(tmp, "Pixel value = %ld", ci->pixel);
    f->SetTip(tmp);
    AddFrame(f, NULL);
  }
}

void OXPaletteFrame::_AddBTreeEntries(const OColorInfo *root) {
  if (root) {
    _AddEntry(root);
    _AddBTreeEntries(root->_left);
    _AddBTreeEntries(root->_right);
  }
}

void OXPaletteFrame::_RemoveEntries() {
  OXFrame *f;
  SListFrameElt *ptr, *next;

  ptr = _flist;
  while (ptr) {
    next = ptr->next;
    f = ptr->frame;
    f->DestroyWindow();
    RemoveFrame(f);
    delete f;
    ptr = next;
  }
  _flist = NULL;
  //Layout();
}

int OXPaletteFrame::HandleButton(XButtonEvent *event) {
  SListFrameElt *ptr;
  OXColorEntry *e;

  if (event->type == ButtonPress) {
    for (ptr = _flist; ptr != NULL; ptr = ptr->next) {   
      e = (OXColorEntry *) ptr->frame;
      if (e->GetId() == event->subwindow) {
        OWidgetMessage message(MSG_PALETTE, MSG_CLICK, e->ID());
        SendMessage(_msgObject, &message);
      }
    }
  }
  return True;
}

unsigned long OXPaletteFrame::GetColorByIndex(int ix) {
  SListFrameElt *ptr;
  OXColorEntry *e;
  int i;

  for (ptr = _flist, i = 0; ptr != NULL; ptr = ptr->next, ++i) {
    if (i == ix) {
      e = (OXColorEntry *) ptr->frame;
      return e->ID();
    }
  }
  return TRANSPARENT(GetDisplay(), _client->GetScreenNumber());
}
