/**************************************************************************

    This file is part of OnyX, a visual development environment
    using the xclass toolkit. Copyright (C) 2004, Hector Peraza.

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

#include <X11/X.h>
#include <X11/keysym.h>

#include "OnyxSpinButton.h"

extern MessageCenter *CentralMessageCenter;

static int ButtonIsDown;  //==!==


//----------------------------------------------------------------------

RedirectedSpinner::RedirectedSpinner(const OXWindow *p, int ID) :
  OXSpinner(p, "", ID) {
  InternalID = ID;
  Clicked = 0;
  ButtonIsDown = 0;
  AddInput(StructureNotifyMask | KeyPressMask| KeyReleaseMask |
           ButtonPressMask | ButtonReleaseMask |
           PointerMotionMask | ExposureMask |
           EnterWindowMask | LeaveWindowMask);
}

int RedirectedSpinner::HandleCrossing(XCrossingEvent *event) {
  if (event->type == LeaveNotify) {
    Clicked = 0;
  } else {
    Clicked = 1;
  }
  return OXSpinner::HandleCrossing(event);
}

int RedirectedSpinner::HandleMotion(XMotionEvent *event) {
  OnyxMessage omsg;

  if (ButtonIsDown) {
    OXSpinner::HandleMotion(event);
  }

  omsg.onyx_type = SpinButtonType;
  omsg.id = InternalID;
  omsg.action = PointerMoved;
  omsg.x = event->x;
  omsg.y = event->y;
  omsg.x_root = event->x_root;
  omsg.y_root = event->y_root;

  return CentralMessageCenter->ProcessMessage(&omsg);
}

int RedirectedSpinner::HandleButton(XButtonEvent *event) {
  OnyxMessage omsg;

  omsg.onyx_type = SpinButtonType;
  omsg.id = InternalID;

  omsg.button = event->button;
  omsg.x = event->x;
  omsg.y = event->y;
  omsg.x_root = event->x_root;
  omsg.y_root = event->y_root;

  OXSpinner::HandleButton(event);

  if (event->type == ButtonPress) {
    ButtonIsDown = 1;
    Clicked = 1;
    omsg.action = ButtonPressed;
    CentralMessageCenter->ProcessMessage(&omsg);
  } else {
    ButtonIsDown = 0;
    omsg.action = ButtonReleased;
    CentralMessageCenter->ProcessMessage(&omsg);
    if (Clicked) {
      omsg.action = ButtonClicked;
      CentralMessageCenter->ProcessMessage(&omsg);
    }
  }
  return True;
}

int RedirectedSpinner::HandleKey(XKeyEvent *event) {
  OnyxMessage omsg;

  omsg.onyx_type = SpinButtonType;
  omsg.id = InternalID;

  omsg.keysym = XKeycodeToKeysym(GetDisplay(), event->keycode, 0);

  if (event->type == KeyPress) {
    OXSpinner::HandleKey(event);
    omsg.action = KeyPressed;
    CentralMessageCenter->ProcessMessage(&omsg);
    if (omsg.keysym == XK_Return) {
      omsg.action = EnterPressed;
      CentralMessageCenter->ProcessMessage(&omsg);
    }
  } else {
    omsg.action = KeyReleased;
    CentralMessageCenter->ProcessMessage(&omsg);
  }
  return True;
}

int RedirectedSpinner::ProcessMessage(OMessage *msg) {
  OnyxMessage omsg;

  omsg.onyx_type = SpinButtonType;
  omsg.id = InternalID;

  if (msg->type == MSG_SPINNER && msg->action == MSG_VALUECHANGED) {
    omsg.action = ValueChanged;
    CentralMessageCenter->ProcessMessage(&omsg);
  } else {
    OXSpinner::ProcessMessage(msg);
  }

  return True;
}


//----------------------------------------------------------------------

OnyxSpinButton::OnyxSpinButton() : OnyxObject("OnyxSpinButton") {
  SpinButtonExists = 0;
  Width = 100;
  Height = 10;
  X = 0;
  Y = 15;
  Size = 255;
}

int OnyxSpinButton::Create() {
  OnyxObject::Create();
  if (!SpinButtonExists) {
    InternalSpinner = new RedirectedSpinner(Parent, ID);
    InternalSpinner->MoveResize(X, Y, Width, Height);
    InternalSpinner->MapSubwindows();
    InternalSpinner->Layout();
    SpinButtonExists++;
    // Note the message loop below!
    InternalSpinner->Associate(InternalSpinner);
  }
  return SpinButtonExists;
}

void OnyxSpinButton::Move(int newX, int newY) {
  OnyxObject::Move(newX, newY);
  InternalSpinner->Move(newX, newY);
}

void OnyxSpinButton::Resize(int newWidth, int newHeight) {
  OnyxObject::Resize(newWidth, newHeight);
  InternalSpinner->Resize(newWidth, newHeight);
}

void OnyxSpinButton::MoveResize(int newX, int newY, int newWidth, int newHeight) {
  OnyxObject::MoveResize(newX, newY, newWidth, newHeight);
  InternalSpinner->MoveResize(newX, newY, newWidth, newHeight);
}

void OnyxSpinButton::Update() {
  InternalSpinner->NeedRedraw();
}

OnyxSpinButton::~OnyxSpinButton() {
  InternalSpinner->DestroyWindow();
  delete InternalSpinner;
}
