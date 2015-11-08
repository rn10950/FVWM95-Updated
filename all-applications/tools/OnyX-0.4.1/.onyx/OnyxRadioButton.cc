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

#include "OnyxRadioButton.h"

extern MessageCenter *CentralMessageCenter;


//----------------------------------------------------------------------

RedirectedRadioButton::RedirectedRadioButton(const OXWindow *p, OHotString *s, int ID) :
    OXRadioButton(p, s, ID) {
  InternalID = ID;
  Clicked = 0;
  AddInput(StructureNotifyMask | ButtonPressMask | ButtonReleaseMask |
           PointerMotionMask | ExposureMask | KeyPressMask | KeyReleaseMask);
}

int RedirectedRadioButton::HandleCrossing(XCrossingEvent *event) {
  if (event->type == LeaveNotify) {
    if (!*ExternalStatus) {
      Clicked = 0;
      OXRadioButton::HandleCrossing(event);
    }
  } else {
    Clicked = 1;
    OXRadioButton::HandleCrossing(event);
  }
  return True;
}

int RedirectedRadioButton::HandleMotion(XMotionEvent *event) {
  OnyxMessage omsg;

  OXRadioButton::HandleMotion(event);

  omsg.onyx_type = RadioButtonType;
  omsg.id = InternalID;
  omsg.action = PointerMoved;
  omsg.x = event->x;
  omsg.y = event->y;
  omsg.x_root = event->x_root;
  omsg.y_root = event->y_root;

  return CentralMessageCenter->ProcessMessage(&omsg);
}

int RedirectedRadioButton::HandleButton(XButtonEvent *event) {
  OnyxMessage omsg;

  omsg.onyx_type = RadioButtonType;
  omsg.id = InternalID;

  omsg.button = event->button;
  omsg.x = event->x;
  omsg.y = event->y;
  omsg.x_root = event->x_root;
  omsg.y_root = event->y_root;

  if (event->type == ButtonPress) {
    if(event->button == 1) {
      OXRadioButton::HandleButton(event);
    }
    Clicked = 1;
    *ExternalStatus = GetState();
    omsg.action = ButtonPressed;
    CentralMessageCenter->ProcessMessage(&omsg);
  } else {
    if ((Clicked) && (event->button == 1)) {
      OXRadioButton::HandleButton(event);
      *ExternalStatus = GetState();
      omsg.action = ButtonReleased;
      CentralMessageCenter->ProcessMessage(&omsg);
      omsg.action = ButtonClicked;
      CentralMessageCenter->ProcessMessage(&omsg);
    } else {
      *ExternalStatus = GetState();
      omsg.action = ButtonReleased;
      CentralMessageCenter->ProcessMessage(&omsg);
    }
    Clicked = 0;
  }
  return True;
}

int RedirectedRadioButton::HandleKey(XKeyEvent *event) {
  OnyxMessage omsg;

  omsg.onyx_type = RadioButtonType;
  omsg.id = InternalID;

  omsg.keysym = XKeycodeToKeysym(GetDisplay(), event->keycode, 0);

  if (event->type == KeyPress) {
    omsg.action = KeyPressed;
    CentralMessageCenter->ProcessMessage(&omsg);
  } else {
    omsg.action = KeyReleased;
    CentralMessageCenter->ProcessMessage(&omsg);
  }
  return True;
}

RedirectedRadioButton::~RedirectedRadioButton() {
}


//----------------------------------------------------------------------

OnyxRadioButton::OnyxRadioButton() : OnyxObject("OnyxRadioButton") {
  RadioButtonExists = 0;
  Width = 100;
  Height = 100;
  X = 0;
  Y = 15;
  IsChecked = False;
}

int OnyxRadioButton::Create() {
  OnyxObject::Create();
  if (!RadioButtonExists) {
    InternalFrame = new OXCompositeFrame(Parent, Width, Height);
    InternalRadioButton = new RedirectedRadioButton(InternalFrame, new OHotString(Text), ID);
    InternalRadioButton->ExternalStatus = &IsChecked;
    InternalRadioButton->SetState(IsChecked);
    InternalFrame->AddFrame(InternalRadioButton, new OLayoutHints(LHINTS_EXPAND_ALL));
    if (!IsEnabled) {
      Disable();
    }
    InternalRadioButton->SetFont(ApplicationClient()->GetFont(Font));
    InternalFrame->MapSubwindows();
    InternalFrame->Layout();
    InternalFrame->Move(X, Y);
    RadioButtonExists++;
  }
  return RadioButtonExists;
}

void OnyxRadioButton::Check() {
  IsChecked = 1;
  InternalRadioButton->SetState(IsChecked);
}

void OnyxRadioButton::UnCheck() {
  IsChecked = 0;
  InternalRadioButton->SetState(IsChecked);
}

void OnyxRadioButton::Enable() {
  OnyxObject::Enable();
  if (IsChecked) {
    InternalRadioButton->SetState(1);
  } else {
    InternalRadioButton->SetState(2);
  }
}

void OnyxRadioButton::Disable() {
  OnyxObject::Disable();
  InternalRadioButton->SetState(3);
}

void OnyxRadioButton::Update() {
  OnyxObject::Update();
  InternalRadioButton->SetText(new OHotString(Text));
  InternalRadioButton->SetFont(ApplicationClient()->GetFont(Font));
  InternalRadioButton->SetState(IsChecked);
  ApplicationClient()->NeedRedraw(InternalRadioButton);
}

void OnyxRadioButton::Toggle() {
  IsChecked = !IsChecked;
  InternalRadioButton->SetState(IsChecked);
}

void OnyxRadioButton::Move(int newX, int newY) {
  OnyxObject::Move(newX, newY);
  InternalFrame->Move(newX, newY);
}

void OnyxRadioButton::Resize(int newWidth, int newHeight) {
  OnyxObject::Resize(newWidth, newHeight);
  InternalFrame->Resize(newWidth, newHeight);
}

void OnyxRadioButton::MoveResize(int newX, int newY, int newWidth, int newHeight) {
  OnyxObject::MoveResize(newX, newY, newWidth, newHeight);
  InternalFrame->MoveResize(newX, newY, newWidth, newHeight);
}

OnyxRadioButton::~OnyxRadioButton() {
  InternalFrame->DestroyWindow();
  // the delete below frees InternalRadioButton
  delete InternalFrame;
}
