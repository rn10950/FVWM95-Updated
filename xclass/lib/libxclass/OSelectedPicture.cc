/**************************************************************************

    This file is part of xclass, a Win95-looking GUI toolkit.
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

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <xclass/utils.h>
#include <xclass/OXClient.h>
#include <xclass/OXWindow.h>
#include <xclass/OResourcePool.h>
#include <xclass/OSelectedPicture.h>
#include <xclass/OGC.h>


OXGC *OSelectedPicture::_selectedGC = NULL;


//---------------------------------------------------------------------------

OSelectedPicture::OSelectedPicture(const OXClient *client, const OPicture *p) :
  OPicture(NULL) {

  XGCValues gcv;
  unsigned  long gcm;

  _client = client;
  Display *dpy = _client->GetDisplay();
  Window Root  = _client->GetRoot()->GetId();

  if (!_selectedGC) {
    gcm = GCForeground | GCBackground | GCGraphicsExposures |
            GCFillStyle | GCStipple;
    gcv.foreground = _client->GetResourcePool()->GetSelectedBgndColor();
    gcv.background = _client->GetResourcePool()->GetBlackColor();
//    gcv.foreground = _client->GetResourcePool()->GetSelectedFgndColor();
//    gcv.background = _client->GetResourcePool()->GetSelectedBgndColor();
    gcv.graphics_exposures = False;
    gcv.fill_style = FillStippled;
    gcv.stipple = _client->GetResourcePool()->GetCheckeredBitmap();
    _selectedGC = new OXGC(dpy, Root, gcm, &gcv);
  }

  int w = p->GetWidth();
  int h = p->GetHeight();

  _pic  = XCreatePixmap(dpy, Root, w, h, _client->GetDisplayDepth());
  _mask = p->GetMask();

  _attributes.width  = w;
  _attributes.height = h;

  XCopyArea(dpy, p->GetPicture(), _pic, _selectedGC->GetGC(), 0, 0, w, h, 0, 0);

  gcm = GCClipMask | GCClipXOrigin | GCClipYOrigin;
  gcv.clip_mask = p->GetMask();
  gcv.clip_x_origin = 0;
  gcv.clip_y_origin = 0;
  XChangeGC(dpy, _selectedGC->GetGC(), gcm, &gcv);

  XFillRectangle(dpy, _pic, _selectedGC->GetGC(), 0, 0, w, h);

  gcm = GCClipMask;
  gcv.clip_mask = None;
  XChangeGC(dpy, _selectedGC->GetGC(), gcm, &gcv);
}

OSelectedPicture::~OSelectedPicture() {
  XFreePixmap(_client->GetDisplay(), _pic);
}
