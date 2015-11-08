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

#include <xclass/utils.h>
#include <xclass/ODimension.h>
#include <xclass/OResourcePool.h>
#include <xclass/OXTip.h>

#include <X11/extensions/shape.h>

//----------------------------------------------------------------------

OXTip::OXTip(const OXWindow *p, OString *text) :
  OXCompositeFrame(p, 10, 10, HORIZONTAL_FRAME | RAISED_FRAME) {

  XSetWindowAttributes attr;
  unsigned long mask;

  mask = CWOverrideRedirect | CWSaveUnder;
  attr.override_redirect = True;
  attr.save_under = True;

  XChangeWindowAttributes(GetDisplay(), _id, mask, &attr);

  _bg = _client->GetResourcePool()->GetTipBgndColor();
  _fg = _client->GetResourcePool()->GetTipFgndColor();

  _useShadow = True;

  _pclip = None;
  _pmask = None;

  SetBackgroundColor(_bg);

  _label = new OXLabel(this, text);
  _label->SetBackgroundColor(_bg);
  _label->SetTextColor(_fg);
  //_label->SetFont(...);


  AddFrame(_label, _ll = new OLayoutHints(LHINTS_LEFT | LHINTS_TOP,
                                          2, 3, 0, 0));
  MapSubwindows();

  if (_useShadow) {
    ODimension size = GetDefaultSize();
    Resize(size.w + 4, size.h + 4);
    SetupShadow();
  } else {
    Resize(GetDefaultSize());
  }
}

OXTip::~OXTip() {
  delete _ll;
  XDestroyWindow(GetDisplay(), _id);
  if (_pclip != None) XFreePixmap(GetDisplay(), _pclip);
  if (_pmask != None) XFreePixmap(GetDisplay(), _pmask);
}

void OXTip::DrawBorder() {
  if (_useShadow) {
    DrawLine(_blackGC, 0, 0, _w-2, 0);
    DrawLine(_blackGC, 0, 0, 0, _h-2);
  } else {
    DrawLine(_shadowGC, 0, 0, _w-2, 0);
    DrawLine(_shadowGC, 0, 0, 0, _h-2);
  }
  DrawLine(_blackGC,  0, _h-1, _w-1, _h-1);
  DrawLine(_blackGC,  _w-1, _h-1, _w-1, 0);
}

void OXTip::SetText(OString *text) {
  _label->SetText(text);
  if (_useShadow) {
    ODimension size = GetDefaultSize();
    Resize(size.w + 4, size.h + 4);
    SetupShadow();
  } else {
    Resize(GetDefaultSize());
  }
}

void OXTip::Show(int x, int y) {
  Move(x, y);
  MapWindow();
  RaiseWindow();
}

void OXTip::Hide() {
  UnmapWindow();
}

void OXTip::SetupShadow() {
  Window Root = _client->GetRoot()->GetId();
  XGCValues gcval;
  unsigned long gcmask;

  if (_pclip != None) XFreePixmap(GetDisplay(), _pclip);
  if (_pmask != None) XFreePixmap(GetDisplay(), _pmask);

  ODimension size = GetDefaultSize();

  _pmask = XCreatePixmap(GetDisplay(), Root, size.w + 4, size.h + 4, 1);
  _pclip = XCreatePixmap(GetDisplay(), Root, size.w + 4, size.h + 4, 1);

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

  XFillRectangle(GetDisplay(), _pmask, gc0, 0, 0, size.w + 4, size.h + 4);
  XFillRectangle(GetDisplay(), _pmask, cgc, 3, 3, size.w + 4, size.h + 4);
  XFillRectangle(GetDisplay(), _pmask, gc1, 0, 0, size.w + 1, size.h + 1);

  XFillRectangle(GetDisplay(), _pclip, gc0, 0, 0, size.w + 4, size.h + 4);
  XFillRectangle(GetDisplay(), _pclip, gc1, 1, 1, size.w - 1, size.h - 1);

  XShapeCombineMask(GetDisplay(), _id, ShapeBounding, 0, 0, _pmask, ShapeSet);
  XShapeCombineMask(GetDisplay(), _id, ShapeClip,     0, 0, _pclip, ShapeSet);

  XFreeGC(GetDisplay(), gc0);
  XFreeGC(GetDisplay(), gc1);
  XFreeGC(GetDisplay(), cgc);
}

void OXTip::Reconfig() {

  _bg = _client->GetResourcePool()->GetTipBgndColor();
  _fg = _client->GetResourcePool()->GetTipFgndColor();

  SetBackgroundColor(_bg);

  _label->SetBackgroundColor(_bg);
  _label->SetTextColor(_fg);
  //_label->SetFont(...);
  // resize, etc. if font changed

  _label->Reconfig();
  NeedRedraw(True);
}
