/**************************************************************************

    This file is part of xclass, a Win95-looking GUI toolkit.
    Copyright (C) 1996, 1997 Ricky Ralston, Hector Peraza.

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

#ifndef __OXSLIST_H
#define __OXSLIST_H


#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>

#include <xclass/utils.h>
#include <xclass/OBaseObject.h>

#undef NULL
#define NULL 0

//------ A node object for the ODList and OXSList classes

class OXSNode : public OBaseObject {
public:
  int id;
  OXSNode *prev, *next;
  XPointer data;
};


//------ Double-linked list of OXSNode objects

class ODList : public OBaseObject {
public:
  ODList(XPointer data);
  virtual ~ODList();

  virtual OXSNode *Add(int id, XPointer data);
  virtual Bool Remove(int id);
  virtual OXSNode *GetNode(int id);
  virtual Bool SetFirst(int id);
  virtual Bool SetLast(int id);
  const OXSNode *GetHead() const { return _head; }
  const OXSNode *GetTail() const { return _tail; }
  int NoOfItems() const { return _nentries; }

protected:
  Bool _Link(OXSNode *item, Bool toend = True);
  Bool _Unlink(OXSNode *item);

  int _nentries;
  OXSNode *_head, *_tail;
  XPointer _data;
};


//------ Double-linked list that uses the X context manager
//       to speed-up searches

class OXSList : public ODList {
public:
  OXSList(Display *dpy, XPointer data);
  virtual ~OXSList();

  virtual OXSNode *Add(int id, XPointer data);
  virtual Bool Remove(int id);
  virtual OXSNode *GetNode(int id);

protected:
  Display *_dpy;
  XContext _context;
};


#endif  // __OXSLIST_H
