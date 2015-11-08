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

#ifndef __OFILEHANDLER_H
#define __OFILEHANDLER_H
  
#include <stdio.h>

#include <xclass/OBaseObject.h>
#include <xclass/OComponent.h>
#include <xclass/OXSList.h>
#include <xclass/OXClient.h>
  

#define  XCM_READABLE    (1<<0)
#define  XCM_WRITABLE    (1<<1)
#define  XCM_EXCEPTION   (1<<2)


//----------------------------------------------------------------------

class OFileHandler : public OBaseObject {
protected:
  static int id_count;

public:
  OFileHandler(OComponent *c, int fd, unsigned int event_mask);
  ~OFileHandler();

  int GetFd() const { return _fd; }
  int GetEventMask() const { return _eventMask; }
  
  void SetEventMask(unsigned int event_mask) { _eventMask = event_mask; } 

  friend class OXClient;
  friend class OXSList;

protected:
  int _id;
  OXClient *_client;
  OComponent *_c;
  int _fd;
  unsigned int _eventMask;    // mask of desired events
};


#endif  // __OFILEHANDLER_H
