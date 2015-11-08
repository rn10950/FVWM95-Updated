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

#ifndef __OTIMER_H
#define __OTIMER_H
  

#include <sys/time.h>

#include <xclass/OBaseObject.h>
#include <xclass/OXWindow.h>
#include <xclass/OXSList.h>
#include <xclass/OXClient.h>
  
class OXSTimerList;


//--- Only single-shot timers by now...

class OTimer : public OBaseObject {
protected:
  static int id_count;

public:
  OTimer(OComponent *c, unsigned long msecs);
  virtual ~OTimer();

  friend class OXClient;
  friend class OXSTimerList;

protected:
  int _id;
  OXClient *_client;
  OComponent *_c;
  struct timeval _tm;
};


//--- This list needs to be sorted by timeout values

class OXSTimerList : public OXSList {
public:
  OXSTimerList(Display *dpy, XPointer data) : OXSList(dpy, data) {}

  virtual OXSNode *Add(int id, XPointer data);
};


#endif  // __OTIMER_H
