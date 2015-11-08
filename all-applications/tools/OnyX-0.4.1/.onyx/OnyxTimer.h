/**************************************************************************

    This file is part of OnyX, a visual development environment
    using the xclass toolkit. Copyright (C) 1997, 1998 Frank Hall.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

**************************************************************************/

#ifndef __ONYXTIMER_H
#define __ONYXTIMER_H

#include <xclass/OTimer.h>
#include <xclass/OXCompositeFrame.h>

#include "OnyxBase.h"


#define TimerElapsed 1      // Define Timer Elapsed Event ID


//----------------------------------------------------------------------

class RedirectedTimerFrame : public OXCompositeFrame {
public:
  RedirectedTimerFrame(const OXWindow *p, int w, int h, int ID, int *Interval);
  virtual ~RedirectedTimerFrame();

  virtual int HandleTimer(OTimer *t);

  virtual void StartTimer();
  virtual void StopTimer();

private:
  int InternalID;
  int *InternalInterval;
  int TimerHaltRequested;
  OTimer *InternalTimer;
};


//----------------------------------------------------------------------

class OnyxTimer : public OnyxObject {
public:
  OnyxTimer();
  virtual ~OnyxTimer();

  virtual int  Create();
  virtual void Start();
  virtual void Stop();

  int Interval;

protected:
  RedirectedTimerFrame *InternalFrame;
};


#endif  // __ONYXTIMER_H
