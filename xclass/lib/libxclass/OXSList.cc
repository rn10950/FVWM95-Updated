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

#include <xclass/utils.h>
#include <xclass/OXSList.h>


//-------------------------------------------------------------------

ODList::ODList(XPointer data) {
  _nentries = 0;
  _head     = _tail = NULL;
  _data     = data;
}

ODList::~ODList() {
  OXSNode *e = _head, *next;

  while (e) {
    next = e->next;
    delete e;
    e = next;
  }
}

OXSNode *ODList::Add(int id, XPointer data) {
  Remove(id);  // remove existing node, if any...

  OXSNode *item = new OXSNode;

  if (!item) return NULL;

  item->id   = id;
  item->data = data;

  _Link(item, True);

  return item;
}

Bool ODList::Remove(int id) {
  OXSNode *item;

  item = GetNode(id);
  if (!item) return False;

  _Unlink(item);
  delete item;

  return True;
}

Bool ODList::SetFirst(int id) {
  OXSNode *item;

  item = GetNode(id);
  if (!item) return False;

  _Unlink(item);
  _Link(item, False);

  return True;
}

Bool ODList::SetLast(int id) {
  OXSNode *item;

  item = GetNode(id);
  if (!item) return False;

  _Unlink(item);
  _Link(item, True);

  return True;
}

Bool ODList::_Link(OXSNode *item, Bool toend) {

  if (!item) return False;

  item->next = NULL;
  item->prev = NULL;

  if (_head == NULL) {

    _head = _tail = item;

  } else {
    if (toend) {

      // add to the end...
      _tail->next = item;
      item->prev = _tail;
      _tail = item;

    } else {

      // add to the beginning...
      _head->prev = item;
      item->next = _head;
      _head = item;

    }
  }
  _nentries++;

  return True;
}

Bool ODList::_Unlink(OXSNode *item) {
  OXSNode *e;

  if (!item) return False;

  if (item->prev == NULL) {

    _head = item->next;
    if (_head == NULL)
      _tail = NULL;
    else
      _head->prev = NULL;

  } else if (item->next == NULL) {

    e = item->prev;
    e->next = NULL;
    _tail = e;

  } else {

    e = item->prev;
    e->next = item->next;
    e = item->next;
    e->prev = item->prev;

  }

  _nentries--;

  return True;
}

OXSNode *ODList::GetNode(int id) {
  OXSNode *item;

  for (item = _head; item != NULL; item = item->next)
    if (item->id == id) return item;

  return NULL;
}


//-------------------------------------------------------------------

OXSList::OXSList(Display *dpy, XPointer data) : ODList(data) {
  _dpy      = dpy;
  _context  = XUniqueContext();
}

OXSList::~OXSList() {
  OXSNode *e = _head;

  while (e) {
    XDeleteContext(_dpy, e->id, _context);
    e = e->next;
  }
}

OXSNode *OXSList::Add(int id, XPointer data) {
  Remove(id);  // remove existing node, if any...

  OXSNode *e = new OXSNode;

  if (!e) return NULL;

  e->id   = id;
  e->data = data;

  _Link(e, True);
  XSaveContext(_dpy, e->id, _context, (XPointer) e);

  return e;
}

Bool OXSList::Remove(int id) {
  OXSNode *item;

  if ((XFindContext(_dpy, id, _context, (XPointer *)&item)) == XCNOENT)
    return False;

  _Unlink(item);
  XDeleteContext(_dpy, id, _context);
  delete item;

  return True;
}

OXSNode *OXSList::GetNode(int id) {
  OXSNode *item;

  if ((XFindContext(_dpy, id, _context, (XPointer *)&item)) == XCNOENT)
    return NULL;

  return item;
}
