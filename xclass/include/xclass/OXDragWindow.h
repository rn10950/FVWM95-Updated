/**************************************************************************

    This file is part of xclass.
    Copyright (C) 1996-2000 Hector Peraza.

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

#ifndef __OXDRAGWINDOW_H
#define __OXDRAGWINDOW_H

#include <X11/Xlib.h>

#include <xclass/OXFrame.h>


//----------------------------------------------------------------------

class OXDragWindow : public OXFrame {
protected:
  static Cursor _defaultCursor;

public:
  OXDragWindow(const OXWindow *p, Pixmap pic, Pixmap mask,
               unsigned int options = CHILD_FRAME,
               unsigned long back = _whitePixel);
  virtual ~OXDragWindow();

  virtual ODimension GetDefaultSize() const { return ODimension(_pw, _ph); }

  virtual void MapWindow();
  virtual void UnmapWindow();
  virtual void RaiseWindow();
  virtual void LowerWindow();
  virtual void MapRaised();

  virtual void Layout();

  int HasWindow(Window w) const { return (w == _id || w == _input); }

protected:
  virtual void _Moved();
  virtual void _Resized();
  virtual void _DoRedraw();

  Window _input;
  Pixmap _pic, _mask;
  unsigned int _pw, _ph;
};


#endif  // __OXDRAGWINDOW
