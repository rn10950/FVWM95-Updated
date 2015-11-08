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

#include "OnyxTimer.h"

extern MessageCenter *CentralMessageCenter;


//----------------------------------------------------------------------

RedirectedTimerFrame::RedirectedTimerFrame(const OXWindow *p,
  int w, int h, int ID, int *Interval) : OXCompositeFrame(p, w, h) {
    InternalID = ID;
    InternalInterval = Interval;
    InternalTimer = NULL;
}

int RedirectedTimerFrame::HandleTimer(OTimer *t) {
  if (t != InternalTimer) {
    return False;
  } else {
    delete InternalTimer;
    if (TimerHaltRequested) {
      InternalTimer = NULL;
      TimerHaltRequested = 0;
    } else {
      InternalTimer = new OTimer(this, *InternalInterval);
      OnyxMessage omsg;
      omsg.onyx_type = NoType;
      omsg.id = InternalID;
      omsg.action = TimerElapsed;
      CentralMessageCenter->ProcessMessage(&omsg);
    }
  }
  return True;
}

void RedirectedTimerFrame::StartTimer() {
  TimerHaltRequested = 0;
  if (InternalTimer == NULL) {
    InternalTimer = new OTimer(this, *InternalInterval);
  }
}

void RedirectedTimerFrame::StopTimer() {
  TimerHaltRequested = 1;
}

RedirectedTimerFrame::~RedirectedTimerFrame() {
}


//--------------------------------------------------------------------

OnyxTimer::OnyxTimer() : OnyxObject("OnyxTimer") {
  Interval = 0;
}

int OnyxTimer::Create() {
  OnyxObject::Create();
  InternalFrame = new RedirectedTimerFrame(Parent, 1, 1, ID, &Interval);
  return True;
}

void OnyxTimer::Start() {
  InternalFrame->StartTimer();
}

void OnyxTimer::Stop() {
  InternalFrame->StopTimer();
}

OnyxTimer::~OnyxTimer() {
  delete InternalFrame;
}
