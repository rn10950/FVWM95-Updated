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

#ifndef __OFOCUSMANAGER_H
#define __OFOCUSMANAGER_H

#include <X11/Xlib.h>

#include <xclass/OBaseObject.h>
#include <xclass/OXFrame.h>

class OXFrame;
class OXClient;
class OXCompositeFrame;


//----------------------------------------------------------------------

class OFocusManager : public OBaseObject {
public:
  OFocusManager(OXClient *client, OXFrame *focusRoot);

  OXFrame *GetFocusOwner();
  void SetFocusOwner(OXFrame *f);
  int  FocusNext(OXFrame *f);
  int  FocusNext() { return FocusNext(GetFocusOwner()); }
  int  FocusPrevious(OXFrame *f);
  int  FocusPrevious() { return FocusPrevious(GetFocusOwner()); }
  int  AssignFocus(OXFrame *f);
  int  FocusForward(OXCompositeFrame *f);
  int  FocusBackward(OXCompositeFrame *f);
  int  FocusCurrent() { return AssignFocus(GetFocusOwner()); }

protected:
  OXFrame *_focusRoot, *_focusOwner;
  OXClient *_client;
  XID _focusOwnerId;
};


#endif  // __OFOCUSMANAGER_H
