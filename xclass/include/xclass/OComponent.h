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

#ifndef __OCOMPONENT_H
#define __OCOMPONENT_H

#include <X11/Xlib.h>
#include <xclass/OMessage.h>

class OTimer;
class OFileHandler;
class OIdleHandler;
class OXClient;


//----------------------------------------------------------------------
// xclass generic object, objects derivated from this can 
// communicate with each other.

class OComponent : public OBaseObject {
public:
  OComponent() : _client(0), _msgObject(0) {}
  OComponent(OXClient *c) : _client(c), _msgObject(0) {}
  OComponent(const OComponent *p) : _client(p->_client), _msgObject(p) {}

  void Associate(const OComponent *c) { _msgObject = c; }

  virtual void SendMessage(OMessage *msg) {
    SendMessage(_msgObject, msg);
  }
  virtual void SendMessage(const OComponent *obj, OMessage *msg) {
    if (obj) ((OComponent *)obj)->ProcessMessage(msg);
  }
  virtual int ProcessMessage(OMessage *msg) { return False; }

  virtual int HandleTimer(OTimer *) { return False; }
  virtual int HandleFileEvent(OFileHandler *, unsigned int) { return False; }
  virtual int HandleIdleEvent(OIdleHandler *) { return False; }

protected:
  OXClient *_client;           // pointer to the X client

  friend class OXClient;
  friend class OTimer;
  friend class OFileHandler;
  friend class OIdleHandler;

  const OComponent *_msgObject;
};


#endif  // __OCOMPONENT_H
