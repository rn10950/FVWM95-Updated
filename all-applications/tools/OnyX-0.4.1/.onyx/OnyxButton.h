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

#ifndef __ONYXBUTTON_H
#define __ONYXBUTTON_H

#include <xclass/OPicture.h>
#include <xclass/OXCompositeFrame.h>
#include <xclass/OXTextButton.h>
#include <xclass/OXPictureButton.h>

#include "OnyxBase.h"


#define ButtonType MSG_BUTTON     /* Define Button Type */


//----------------------------------------------------------------------

class RedirectedButtonFrame : public OXCompositeFrame {
public:
  RedirectedButtonFrame(const OXWindow *p, int w, int h, int ID);

  virtual int ProcessMessage(OMessage *msg);

private:
  int InternalID;
};

class RedirectedTextButton : public OXTextButton {
public:
  RedirectedTextButton(const OXWindow *p, OHotString *text, int ID,
                       int *IsSticky, int *IsToggle, int *DisableStopsClick);
  virtual ~RedirectedTextButton();

  virtual int HandleButton(XButtonEvent *event);
  virtual int HandleKey(XKeyEvent *event);
  virtual int HandleCrossing(XCrossingEvent *event);
  virtual int HandleMotion(XMotionEvent *event);

  virtual void AlterState(int NewState);

private:
  int InternalID;
  int *ButtonIsSticky;
  int *ButtonIsToggle;
  int *DisableClick;
  int State;
  int Clicked;
};

class RedirectedPictureButton : public OXPictureButton {
public:
  RedirectedPictureButton(const OXWindow *p, const OPicture *pic, int ID,
                          int *IsSticky, int *IsToggle, int *DisableStopsClick);
  virtual ~RedirectedPictureButton();

  virtual int HandleButton(XButtonEvent *event);
  virtual int HandleKey(XKeyEvent *event);
  virtual int HandleCrossing(XCrossingEvent *event);
  virtual int HandleMotion(XMotionEvent *event);

  virtual void AlterState(int NewState);

private:
  int InternalID;
  int *ButtonIsSticky;
  int *ButtonIsToggle;
  int *DisableClick;
  int State;
  int Clicked;
};


//----------------------------------------------------------------------

class OnyxButton : public OnyxObject {
public:
  OnyxButton();
  virtual ~OnyxButton();

  virtual int  Create();
  virtual void Move(int newX, int newY);
  virtual void MoveResize(int newX, int newY, int newWidth, int newHeight);
  virtual void Resize(int newWidth, int newHeight);
  virtual void Disable();
  virtual void Enable();
  virtual void Update();
  virtual void Pop();
  virtual void Push();

  int IsSticky;
  int IsToggle;
  int DisableStopsClick;
  int PictureFound;

protected:
  RedirectedButtonFrame *InternalFrame;
  RedirectedTextButton *InternalButton;
  RedirectedPictureButton *InternalPictureButton;
  const OPicture *InternalPicture;
  int ButtonExists;
  int IsTextButton;
};


#endif  // __ONYXBUTTON_H
