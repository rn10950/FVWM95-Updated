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

#include <unistd.h>
#include <sys/time.h>

#include <xclass/utils.h>
#include <xclass/OTimer.h>


int OTimer::id_count = 0;

OTimer::OTimer(OComponent *c, unsigned long msecs) {

  if (c != NULL) {

    _c = c;
    _client = c->_client;
    _id = ++id_count;

    gettimeofday(&_tm, NULL);
    _tm.tv_sec  += msecs / 1000;
    _tm.tv_usec += (msecs % 1000) * 1000;
    if (_tm.tv_usec >= 1000000) {
        _tm.tv_usec -= 1000000;
        _tm.tv_sec++;
    }
    _client->RegisterTimer(this);

  } else {
    FatalError("Attempt to attach timer to a non-existing object");
  }
}

OTimer::~OTimer() {
  _client->UnregisterTimer(this);
}

// Add the timer to the list in the correct position,
// ordered by the expiration time.

OXSNode *OXSTimerList::Add(int id, XPointer data) {
  Remove(id);  // remove existing node, if any...

  OXSNode *e = new OXSNode;

  if (!e) return NULL;

  e->next = NULL;
  e->prev = NULL;
  e->id   = id;
  e->data = data;

  if (_head == NULL) {

    _head = _tail = e;

  } else {

    OTimer *t, *ct;
    OXSNode *ptr;

    t = (OTimer *) data;
    for (ptr=_head; ptr!=NULL; ptr=ptr->next) {
      ct = (OTimer *) ptr->data;
      if ( (ct->_tm.tv_sec > t->_tm.tv_sec) ||
          ((ct->_tm.tv_sec == t->_tm.tv_sec) && 
           (ct->_tm.tv_usec > t->_tm.tv_usec))) break;
    }

    // add `e' before `ptr'
    if (ptr == NULL) {

      // add at the end
      _tail->next = e;
      e->prev = _tail;
      _tail = e;

    } else if (ptr == _head) {  // or (ptr->prev == NULL)

      ptr->prev = e;
      e->next = _head;
      _head = e;

    } else {

      e->next = ptr;
      e->prev = ptr->prev;
      ptr->prev->next = e;
      ptr->prev = e;
    }
  }

  _nentries++;
  XSaveContext(_dpy, e->id, _context, (XPointer) e);

  return e;
}
