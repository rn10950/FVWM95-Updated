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

#ifndef __ONYXCHECKBOX_H
#define __ONYXCHECKBOX_H

#include <xclass/OXCompositeFrame.h>
#include <xclass/OXCheckButton.h>

#include "OnyxBase.h"


#define CheckBoxType MSG_CHECKBUTTON	/* Define CheckBox Type */


//----------------------------------------------------------------------

class RedirectedCheckBox : public OXCheckButton {
public:
  RedirectedCheckBox(const OXWindow *p, OHotString *s, int ID);
  virtual ~RedirectedCheckBox();

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

class OnyxCheckBox : public OnyxObject {
public:
  OnyxCheckBox();
  virtual ~OnyxCheckBox();

  virtual int  Create();
  virtual void Move(int newX, int newY);
  virtual void Resize(int newWidth, int newHeight);
  virtual void MoveResize(int newX, int newY, int newWidth, int newHeight);
  virtual void Enable();
  virtual void Disable();
  virtual void Update();
  virtual void Check();
  virtual void UnCheck();
  virtual void Toggle();

protected:
  OXCompositeFrame *InternalFrame;
  RedirectedCheckBox *InternalCheckBox;
  int CheckBoxExists;
};


#endif  // __ONYXCHECKBOX_H
