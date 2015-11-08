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

#ifndef __OX3DLINES_H
#define __OX3DLINES_H

#include <X11/Xlib.h>

#include <xclass/utils.h>
#include <xclass/OXFrame.h>
#include <xclass/OXWindow.h>


//----------------------------------------------------------------------

class OXHorizontal3dLine : public OXFrame {
public:
  OXHorizontal3dLine(const OXWindow *p, int w = 4, int h = 2,
            unsigned int options = CHILD_FRAME,
            unsigned long back = _defaultFrameBackground) :
    OXFrame(p, w, h, options, back) {}

    virtual void DrawBorder() {
      DrawLine(_shadowGC,  0, 0, _w-2, 0);
      DrawLine(_hilightGC, 0, 1, _w-1, 1);
      DrawLine(_hilightGC, _w-1, 0, _w-1, 1);
    }
};


class OXVertical3dLine : public OXFrame {
public:
  OXVertical3dLine(const OXWindow *p, int w = 2, int h = 4,
            unsigned int options = CHILD_FRAME,
            unsigned long back = _defaultFrameBackground) :
    OXFrame(p, w, h, options, back) {}

    virtual void DrawBorder() {
      DrawLine(_shadowGC,  0, 0, 0, _h-2);
      DrawLine(_hilightGC, 1, 0, 1, _h-1);
      DrawLine(_hilightGC, 0, _h-1, 1, _h-1);
    }
};

#endif  // __OX3DLINES_H
