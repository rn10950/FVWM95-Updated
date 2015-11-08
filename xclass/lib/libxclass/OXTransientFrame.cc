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

#include <X11/Xlib.h>
#include <X11/Xutil.h>
   
#include <xclass/utils.h>
#include <xclass/OXClient.h>
#include <xclass/OXTransientFrame.h>


//----------------------------------------------------------------

OXTransientFrame::OXTransientFrame(const OXWindow *p, const OXWindow *main,
                                   int w, int h, unsigned long options) :
  OXMainFrame(p, w, h, options) {

  _main = (OXMainFrame *) main;
  if (main) {
    XSetTransientForHint(GetDisplay(), _id, main->GetId());
    ((OXMainFrame *)main)->RegisterTransient(this);
  }
}

OXTransientFrame::~OXTransientFrame() {
  if (_main) ((OXMainFrame *)_main)->UnregisterTransient(this);
}

// Position transient frame centered relative to the parent frame.
// If _main is NULL (i.e. OXTransientFrame is acting just like a
// OXMainFrame) and croot is True, the window will be centered on
// the root window, otherwise no action is taken and the default
// wm placement will be used.

void OXTransientFrame::CenterOnParent(int croot) {
  int ax, ay;
  Window wdummy;

  int dw = _client->GetDisplayWidth();
  int dh = _client->GetDisplayHeight();

  if (_main) {

    XTranslateCoordinates(GetDisplay(),
                          _main->GetId(), GetParent()->GetId(),
                          (_main->GetWidth() - _w) >> 1,
                          (_main->GetHeight() - _h) >> 1,
                          &ax, &ay, &wdummy);
    if (ax < 10) ax = 10; else if (ax + _w + 10 > dw) ax = dw - _w - 10;
    if (ay < 20) ay = 20; else if (ay + _h + 50 > dh) ay = dh - _h - 50;

  } else if (croot) {

    ax = (dw - _w) >> 1;
    ay = (dh - _h) >> 1;

  } else {

    return;

  }

  Move(ax, ay);
  SetWMPosition(ax, ay);
}
