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

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <assert.h>

#include <xclass/utils.h>
#include <xclass/OXFrame.h>
#include <xclass/OXClient.h>
#include <xclass/OXDragWindow.h>

#include <X11/cursorfont.h>
#include <X11/extensions/shape.h>


Cursor OXDragWindow::_defaultCursor = None;


//---------------------------------------------------------------------------

OXDragWindow::OXDragWindow(const OXWindow *p, Pixmap pic, Pixmap mask,
                           unsigned int options, unsigned long back) :
  OXFrame(p, 32, 32, options, back) {

  if (_defaultCursor == None) {
    _defaultCursor = XCreateFontCursor(GetDisplay(), XC_top_left_arrow);
  }

  _pic = pic;
  _mask = mask;

  XSetWindowAttributes wattr;
  unsigned long wmask;

  wmask = CWOverrideRedirect | CWSaveUnder;
  wattr.save_under = True;
  wattr.override_redirect = True;
    
  XChangeWindowAttributes(GetDisplay(), _id, wmask, &wattr);

  Window root;
  int x, y;
  unsigned int bw, d;

  XGetGeometry(GetDisplay(), _pic, &root, &x, &y, &_pw, &_ph, &bw, &d);

  wmask = CWOverrideRedirect;
  wattr.override_redirect = True;

  // This input window is used to make the dragging smoother when using
  // highly complicated shapped windows (like labels and semitransparent
  // icons), for some obscure reason most of the motion events get lost 
  // while the pointer is over the shaped window.

  _input = XCreateWindow(GetDisplay(), _client->GetRoot()->GetId(),
                         0, 0, _w, _h, 0, 0,
                         InputOnly, CopyFromParent,
                         wmask, &wattr);

  Resize(GetDefaultSize());

  XShapeCombineMask(GetDisplay(), _id, ShapeBounding, 0, 0, _mask, ShapeSet);

  XDefineCursor(GetDisplay(), _id, _defaultCursor);
}

OXDragWindow::~OXDragWindow() {
  XDestroyWindow(GetDisplay(), _input);
}

void OXDragWindow::_Moved() {
  OXFrame::_Moved();
  XMoveWindow(GetDisplay(), _input, _x, _y);
}

void OXDragWindow::_Resized() {
  OXFrame::_Resized();
  XResizeWindow(GetDisplay(), _input, _w, _h);
}

void OXDragWindow::MapWindow() {
  OXFrame::MapWindow();
  XMapWindow(GetDisplay(), _input);
}

void OXDragWindow::UnmapWindow() {
  OXFrame::UnmapWindow();
  XUnmapWindow(GetDisplay(), _input);
}

void OXDragWindow::RaiseWindow() {
  OXFrame::RaiseWindow();
  XRaiseWindow(GetDisplay(), _input);
}

void OXDragWindow::LowerWindow() {
  XLowerWindow(GetDisplay(), _input);
  OXFrame::LowerWindow();
}

void OXDragWindow::MapRaised() {
  OXFrame::MapRaised();
  XMapRaised(GetDisplay(), _input);
}

void OXDragWindow::Layout() {
  XShapeCombineMask(GetDisplay(), _id, ShapeBounding, 0, 0, _mask, ShapeSet);
}

void OXDragWindow::_DoRedraw() {
  XCopyArea(GetDisplay(), _pic, _id, _bckgndGC, 0, 0, _pw, _ph, 0, 0);
}
