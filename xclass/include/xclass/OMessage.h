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

#ifndef __OMESSAGE_H
#define __OMESSAGE_H

#include <xclass/OBaseObject.h>
#include <xclass/OMessageCodes.h>


//----------------------------------------------------------------------

// This is the message base class, it just implements a null message.
// All other messages extend this class.

class OMessage : public OBaseObject {
public:
  OMessage(int msgType = MSG_EMPTY, int msgAction = MSG_NULL) {
    type   = msgType;
    action = msgAction;
  }

  int type, action;
};

// This is a generic widget message. Simple widgets like OXMenu, OXButton,
// etc. would make direct use of this one...

class OWidgetMessage : public OMessage {
public:
  OWidgetMessage(int typ, int act = MSG_NULL, int wid = -1) : 
    OMessage(typ, act) { id = wid; }

  int id;
};


#endif  // __OMESSAGE_H
