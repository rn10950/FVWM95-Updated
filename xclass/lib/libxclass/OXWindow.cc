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

#include <xclass/utils.h>
#include <xclass/OXWindow.h>

OXWindow::OXWindow(const OXWindow *p, int x, int y,
		   int w, int h, int border,
		   int depth,
		   unsigned int clss,
		   Visual *visual,
		   unsigned long vmask,
		   XSetWindowAttributes *attr) {
  if (p != NULL) {
    _client = p->_client;
    _parent = p;
    _toplevel = p->_toplevel;
    _id = XCreateWindow(GetDisplay(), _parent->_id,
                        x, y, max(w, 1), max(h, 1), max(border, 0),
                        depth, clss, visual, vmask, attr);
    _windowExists = (_id != None);
    _client->RegisterWindow(this);
    _needRedraw = False;
    _clearBgnd = True;   // False;  OXScrollBarElt...
  } else {
    FatalError("OXWindow: Failed to create window!");
  }
}

OXWindow::OXWindow(OXClient *c, Window id, OXWindow *parent) {
  _client = c;
  _id     = id;
  _parent = parent;
  _toplevel = parent ? parent->_toplevel : (OXWindow *) NULL;
  if (id != None) _client->RegisterWindow(this);
  _needRedraw = False;
  _clearBgnd = False;
}

OXWindow::~OXWindow() {
  _client->UnregisterWindow(this);
}

void OXWindow::Move(int x, int y) {
  XMoveWindow(GetDisplay(), _id, x, y);
}

void OXWindow::Resize(int w, int h) {
  XResizeWindow(GetDisplay(), _id, max(w, 1), max(h, 1));
}

void OXWindow::MoveResize(int x, int y, int w, int h) {
  XMoveResizeWindow(GetDisplay(), _id, x, y, max(w, 1), max(h, 1));
}

int OXWindow::IsMapped() {
  XWindowAttributes xwattr;

  XGetWindowAttributes(GetDisplay(), _id, &xwattr);
  return (xwattr.map_state != IsUnmapped);
}

void OXWindow::ReparentWindow(const OXWindow *p, int x, int y) {
  if (p == _parent) return;
  XReparentWindow(GetDisplay(), _id, p->GetId(), x, y);
  _parent = p;
  _toplevel = p->GetTopLevel();
}
