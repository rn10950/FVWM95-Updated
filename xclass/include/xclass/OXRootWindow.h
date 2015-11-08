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

#ifndef __OXROOTWINDOW_H
#define __OXROOTWINDOW_H

#include <X11/Xlib.h>

#include <xclass/utils.h>
#include <xclass/OXWindow.h>


//----------------------------------------------------------------------

class OXRootWindow : public OXWindow {
public:
  OXRootWindow(OXClient *c, Window id, OXWindow *parent = NULL);

  virtual int HandleEvent(XEvent *event);
  virtual int HandlePropertyChange(XPropertyEvent *event);
};


#endif  // __OXROOTWINDOW_H
