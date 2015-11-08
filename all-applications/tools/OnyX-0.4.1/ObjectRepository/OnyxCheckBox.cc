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

#include "OnyxCheckBox.h"

extern MessageCenter *CentralMessageCenter;


//----------------------------------------------------------------------

RedirectedCheckBox::RedirectedCheckBox(const OXWindow *p, OHotString *s, int ID) :
  OXCheckButton(p, s, ID) {
  InternalID = ID;
  Clicked = 0;
  AddInput(StructureNotifyMask | ButtonPressMask | ButtonReleaseMask |
           PointerMotionMask | ExposureMask | KeyPressMask | KeyReleaseMask);
}

int RedirectedCheckBox::HandleCrossing(XCrossingEvent *event) {
  if (event->type == LeaveNotify) {
    if (!*ExternalStatus) {
      Clicked = 0;
      OXCheckButton::HandleCrossing(event);
    }
  } else {
    Clicked = 1;
    OXCheckButton::HandleCrossing(event);
  }
  return True;
}

int RedirectedCheckBox::HandleMotion(XMotionEvent *event) {
  OnyxMessage omsg;

  OXCheckButton::HandleMotion(event);

  omsg.onyx_type = CheckBoxType;
  omsg.id = InternalID;
  omsg.action = PointerMoved;
  omsg.x = event->x;
  omsg.y = event->y;
  omsg.x_root = event->x_root;
  omsg.y_root = event->y_root;

  return CentralMessageCenter->ProcessMessage(&omsg);
}

int RedirectedCheckBox::HandleButton(XButtonEvent *event) {
  OnyxMessage omsg;

  omsg.onyx_type = CheckBoxType;
  omsg.id = InternalID;

  omsg.button = event->button;
  omsg.x = event->x;
  omsg.y = event->y;
  omsg.x_root = event->x_root;
  omsg.y_root = event->y_root;

  if (event->type == ButtonPress) {
    if (event->button == 1) {
      OXCheckButton::HandleButton(event);
    }
    Clicked = 1;
    *ExternalStatus = GetState();
    omsg.action = ButtonPressed;
    CentralMessageCenter->ProcessMessage(&omsg);
  } else {
    if ((Clicked) && (event->button == 1)) {
      OXCheckButton::HandleButton(event);
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

int RedirectedCheckBox::HandleKey(XKeyEvent *event) {
  OnyxMessage omsg;

  omsg.onyx_type = CheckBoxType;
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

RedirectedCheckBox::~RedirectedCheckBox() {
}


//----------------------------------------------------------------------

OnyxCheckBox::OnyxCheckBox() : OnyxObject("OnyxCheckBox") {
  CheckBoxExists = 0;
  Width = 100;
  Height = 100;
  X = 0;
  Y = 0;      ////15;
  IsChecked = False;
}

int OnyxCheckBox::Create() {
  OnyxObject::Create();
  if (!CheckBoxExists) {
    InternalFrame = new OXCompositeFrame(Parent, Width, Height);
    InternalCheckBox = new RedirectedCheckBox(InternalFrame, new OHotString(Text), ID);
    InternalCheckBox->ExternalStatus = &IsChecked;
    InternalCheckBox->SetState(IsChecked);
    InternalFrame->AddFrame(InternalCheckBox, new OLayoutHints(LHINTS_EXPAND_ALL));
    if (!IsEnabled) {
      Disable();
    }
    InternalCheckBox->SetFont(ApplicationClient()->GetFont(Font));
    InternalFrame->MapSubwindows();
    InternalFrame->Layout();
    InternalFrame->Move(X, Y);
    CheckBoxExists++;
/*    InternalCheckBox->Associate(CentralMessageCenter);*/
  }
  return CheckBoxExists;
}

void OnyxCheckBox::Check() {
  IsChecked = 1;
  InternalCheckBox->SetState(IsChecked);
}

void OnyxCheckBox::UnCheck() {
  IsChecked = 0;
  InternalCheckBox->SetState(IsChecked);
}

void OnyxCheckBox::Enable() {
  OnyxObject::Enable();
  if (IsChecked) {
    InternalCheckBox->SetState(1);
  } else {
    InternalCheckBox->SetState(2);
  }
}

void OnyxCheckBox::Disable() {
  OnyxObject::Disable();
  InternalCheckBox->SetState(3);
}

void OnyxCheckBox::Update() {
  OnyxObject::Update();
  InternalCheckBox->SetText(new OHotString(Text));
  InternalCheckBox->SetFont(ApplicationClient()->GetFont(Font));
  InternalCheckBox->SetState(IsChecked);
  InternalCheckBox->NeedRedraw();
}

void OnyxCheckBox::Toggle() {
  IsChecked = !IsChecked;
  InternalCheckBox->SetState(IsChecked);
}

void OnyxCheckBox::Move(int newX, int newY) {
  OnyxObject::Move(newX, newY);
  InternalFrame->Move(newX, newY);
}

void OnyxCheckBox::Resize(int newWidth, int newHeight) {
  OnyxObject::Resize(newWidth, newHeight);
  InternalFrame->Resize(newWidth, newHeight);
}

void OnyxCheckBox::MoveResize(int newX, int newY, int newWidth, int newHeight) {
  OnyxObject::MoveResize(newX, newY, newWidth, newHeight);
  InternalFrame->MoveResize(newX, newY, newWidth, newHeight);
}

OnyxCheckBox::~OnyxCheckBox() {
  InternalFrame->DestroyWindow();
  // this deletes InternalCheckBox:
  delete InternalFrame;
}
