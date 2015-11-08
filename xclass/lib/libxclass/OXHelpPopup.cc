/**************************************************************************

    This file is part of xclass.
    Copyright (C) 2004, Hector Peraza.                 

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

#include <xclass/utils.h>
#include <xclass/ODimension.h>
#include <xclass/OResourcePool.h>
#include <xclass/OXHelpPopup.h>
#include <xclass/OXHelpDoc.h>

#include <X11/extensions/shape.h>


#define SHADOW_WIDTH  6


//----------------------------------------------------------------------

OXHelpPopup::OXHelpPopup(const OXWindow *p) :
  OXCompositeFrame(p, 10, 10, HORIZONTAL_FRAME | RAISED_FRAME) {

  XSetWindowAttributes attr;
  unsigned long mask;

  mask = CWOverrideRedirect | CWSaveUnder;
  attr.override_redirect = True;
  attr.save_under = True;

  XChangeWindowAttributes(GetDisplay(), _id, mask, &attr);

  _useShadow = True;

  _pclip = None;
  _pmask = None;

  _doc = new OXHelpDoc(this, 400, 400, CHILD_FRAME);
  _doc->ChangeOptions(OWN_BKGND);
  _doc->SetScrollOptions(CANVAS_NO_SCROLL);
  _doc->UnderlineLinks(True);

  AddFrame(_doc, _ll = new OLayoutHints(LHINTS_NORMAL, 1, 0, 1, 0));
  MapSubwindows();

  if (_useShadow) {
    ODimension size = GetDefaultSize();
    Resize(size.w + SHADOW_WIDTH + 1, size.h + SHADOW_WIDTH + 1);
    SetupShadow(size);
  } else {
    Resize(GetDefaultSize());
  }

  _waiting = False;
  AddInput(StructureNotifyMask);
}

OXHelpPopup::~OXHelpPopup() {
  delete _ll;
  DestroyWindow();
  if (_pclip != None) XFreePixmap(GetDisplay(), _pclip);
  if (_pmask != None) XFreePixmap(GetDisplay(), _pmask);
}

int OXHelpPopup::HandleButton(XButtonEvent *event) {
  if (_waiting) {
    XUngrabPointer(GetDisplay(), CurrentTime);
    Hide();
    _waiting = False;
    return True;
  }
  return OXCompositeFrame::HandleButton(event);
}

void OXHelpPopup::DrawBorder() {
  if (_useShadow) {
    DrawLine(_blackGC, 0, 0, _w-2, 0);
    DrawLine(_blackGC, 0, 0, 0, _h-2);
    DrawRectangle(_shadowGC, 1, 1, _w - SHADOW_WIDTH - 3, _h - SHADOW_WIDTH - 3);
  } else {
    DrawLine(_shadowGC, 0, 0, _w-2, 0);
    DrawLine(_shadowGC, 0, 0, 0, _h-2);
  }
  DrawLine(_blackGC,  0, _h-1, _w-1, _h-1);
  DrawLine(_blackGC,  _w-1, _h-1, _w-1, 0);
}

void OXHelpPopup::Relayout() {
  _doc->Layout();
  if (_useShadow) {
#if 0
    ODimension size = GetDefaultSize();
#else
    ODimension size = _doc->GetVirtualSize();
    size.w += HTML_INDENT/4;
    size.h += HTML_INDENT/4;
    _doc->Resize(size.w, size.h);
    size.w += 3;
    size.h += 3;
#endif
    Resize(size.w + SHADOW_WIDTH + 1, size.h + SHADOW_WIDTH + 1);
    SetupShadow(size);
  } else {
    Resize(GetDefaultSize());
  }
}

void OXHelpPopup::Show(int x, int y, int wa) {
  Move(x, y);
  MapWindow();
  RaiseWindow();
  if (wa) {
    _waiting = True;
    XGrabPointer(GetDisplay(), _id, False,
                 ButtonPressMask,
                 GrabModeAsync, GrabModeAsync, None,
                 None, CurrentTime);
    _client->WaitForUnmap(this);
  }
}

void OXHelpPopup::Hide() {
  UnmapWindow();
}

void OXHelpPopup::SetupShadow(ODimension size) {
  Window Root = _client->GetRoot()->GetId();
  XGCValues gcval;
  unsigned long gcmask;

  if (_pclip != None) XFreePixmap(GetDisplay(), _pclip);
  if (_pmask != None) XFreePixmap(GetDisplay(), _pmask);

  _pmask = XCreatePixmap(GetDisplay(), Root,
                         size.w + SHADOW_WIDTH + 1,
                         size.h + SHADOW_WIDTH + 1,
                         1);

  _pclip = XCreatePixmap(GetDisplay(), Root,
                         size.w + SHADOW_WIDTH + 1,
                         size.h + SHADOW_WIDTH + 1,
                         1);

  gcmask = GCForeground | GCBackground | GCFillStyle | GCStipple;
  gcval.foreground = 1;
  gcval.background = 0;
  gcval.fill_style = FillStippled;
  gcval.stipple = _client->GetResourcePool()->GetCheckeredBitmap();
  GC cgc = XCreateGC(GetDisplay(), _pmask, gcmask, &gcval);

  gcmask = GCForeground | GCBackground | GCFillStyle;
  gcval.fill_style = FillSolid;
  gcval.foreground = 0;
  gcval.background = 0;
  GC gc0 = XCreateGC(GetDisplay(), _pmask, gcmask, &gcval);

  gcval.foreground = 1;
  gcval.background = 1;
  GC gc1 = XCreateGC(GetDisplay(), _pmask, gcmask, &gcval);

  XFillRectangle(GetDisplay(), _pmask, gc0,
                 0, 0,
                 size.w + SHADOW_WIDTH + 1, size.h + SHADOW_WIDTH + 1);

  XFillRectangle(GetDisplay(), _pmask, cgc,
                 SHADOW_WIDTH, SHADOW_WIDTH,
                 size.w + SHADOW_WIDTH + 1, size.h + SHADOW_WIDTH + 1);

  XFillRectangle(GetDisplay(), _pmask, gc1,
                 0, 0,
                 size.w + 1, size.h + 1);

  XFillRectangle(GetDisplay(), _pclip, gc0,
                 0, 0,
                 size.w + SHADOW_WIDTH + 1, size.h + SHADOW_WIDTH + 1);

  XFillRectangle(GetDisplay(), _pclip, gc1,
                 1, 1,
                 size.w - 1, size.h - 1);

  XShapeCombineMask(GetDisplay(), _id, ShapeBounding, 0, 0, _pmask, ShapeSet);
  XShapeCombineMask(GetDisplay(), _id, ShapeClip,     0, 0, _pclip, ShapeSet);

  XFreeGC(GetDisplay(), gc0);
  XFreeGC(GetDisplay(), gc1);
  XFreeGC(GetDisplay(), cgc);
}
