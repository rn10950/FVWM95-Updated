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

#ifndef __ONYXRADIOBUTTON_H
#define __ONYXRADIOBUTTON_H

#include <xclass/OXCompositeFrame.h>
#include <xclass/OXRadioButton.h>

#include "OnyxBase.h"


#define RadioButtonType MSG_RADIOBUTTON  /* Define RadioButton Type */


//----------------------------------------------------------------------

class RedirectedRadioButton : public OXRadioButton {
public:
  RedirectedRadioButton(const OXWindow *p, OHotString *s, int ID);
  virtual ~RedirectedRadioButton();

  virtual int HandleButton(XButtonEvent *event);
  virtual int HandleKey(XKeyEvent *event);
  virtual int HandleMotion(XMotionEvent *event);
  virtual int HandleCrossing(XCrossingEvent *event);

  int *ExternalStatus;

private:
  int Clicked;
  int InternalID;
};


//----------------------------------------------------------------------

class OnyxRadioButton : public OnyxObject {
public:
  OnyxRadioButton();
  virtual ~OnyxRadioButton();

  virtual int  Create();
  virtual void Move(int newX, int newY);
  virtual void Resize(int newWidth, int newHeight);
  virtual void MoveResize(int newX, int newY, int newWidth, int newHeight);
  virtual void Check();
  virtual void UnCheck();
  virtual void Toggle();
  virtual void Enable();
  virtual void Disable();
  virtual void Update();

protected:
  OXCompositeFrame *InternalFrame;
  RedirectedRadioButton *InternalRadioButton;
  int RadioButtonExists;
};


#endif  // __ONYXRADIOBUTTON_H
