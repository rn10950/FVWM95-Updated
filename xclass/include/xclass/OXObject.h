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

#ifndef __OXOBJECT_H
#define __OXOBJECT_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <xclass/utils.h>
#include <xclass/OComponent.h>
#include <xclass/OXClient.h>


//----------------------------------------------------------------------
// X-Window object base class, this class groups all objects
// having a X Window ID, like Windows, Pixmaps, Fonts, GCs, etc.

class OXObject : public OComponent {
public:
  OXObject() { _id = None; }

  // Access functions:
  const XID GetId() const { return _id; } 
  Display *GetDisplay() const { return _client->GetDisplay(); }

protected:
  XID _id;                     // X-Window identifier
};


#endif  // __OXOBJECT_H
