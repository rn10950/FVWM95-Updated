/**************************************************************************

    This file is part of xclass, a Win95-looking GUI toolkit.
    Copyright (C) 1996, 1997 David Barth, Ricky Ralston, Hector Peraza.

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

#ifndef __OIDLEHANDLER_H
#define __OIDLEHANDLER_H
  

#include <stdio.h>

#include <xclass/OBaseObject.h>
#include <xclass/OXWindow.h>
#include <xclass/OXSList.h>
#include <xclass/OXClient.h>
  

//----------------------------------------------------------------------

class OIdleHandler : public OBaseObject {
private:
  static int _id_count;

public:
  OIdleHandler(OComponent *c);
  ~OIdleHandler();

  int GetId() const { return _id; }

  friend class OXClient;
  friend class OXSList;

protected:
  int _id;
  OXClient *_client;
  OComponent *_c;
};


#endif  // __OIDLEHANDLER_H
