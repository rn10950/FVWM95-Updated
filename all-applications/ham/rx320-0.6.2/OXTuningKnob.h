/**************************************************************************

    This file is part of rx320, a control program for the Ten-Tec RX320
    receiver. Copyright (C) 2000-2004, Hector Peraza.

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

#ifndef __OXTUNINGKNOB_H
#define __OXTUNINGKNOB_H

#include <xclass/OXFrame.h>
#include <xclass/OXWidget.h>
#include <xclass/OXIcon.h>
#include <xclass/OPicture.h>
#include <xclass/OMessage.h>


#define MSG_TUNING_KNOB  (MSG_USERMSG+1122)
#define MSG_UP           1
#define MSG_DOWN         2


//----------------------------------------------------------------------

class OXTuningKnob : public OXFrame, public OXWidget {
public:
  OXTuningKnob(const OXWindow *p, int id = -1);
  virtual ~OXTuningKnob();
  
  void StepKnob(int step);

  virtual ODimension GetDefaultSize() const 
    { return ODimension(_knob->GetWidth(), _knob->GetHeight()); }
    
  virtual int HandleButton(XButtonEvent *event);
  virtual int HandleMotion(XMotionEvent *event);
  
protected:
  virtual void _DoRedraw();

  const OPicture *_knob, *_hole;
  int _a;
};


#endif  // __OXTUNINGKNOB_H
