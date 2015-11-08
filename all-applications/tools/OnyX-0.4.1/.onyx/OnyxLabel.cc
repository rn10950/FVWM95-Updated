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

#include <stdio.h>

#include <X11/X.h>

#include "OnyxLabel.h"

extern MessageCenter *CentralMessageCenter;


//----------------------------------------------------------------------

RedirectedLabel::RedirectedLabel(const OXWindow *p, OString *text, int ID) :
    OXLabel(p, text) {
  InternalID = ID;
  AddInput(StructureNotifyMask | ButtonPressMask | ButtonReleaseMask |
           PointerMotionMask | KeyPressMask | KeyReleaseMask);
}

void RedirectedLabel::SetupAlignment(int horiz, int vert) {
  int alignment;

  switch (horiz) {
    default:
    case AlignCenter: alignment = TEXT_CENTER_X; break;
    case AlignLeft:   alignment = TEXT_LEFT; break;
    case AlignRight:  alignment = TEXT_RIGHT; break;
  }
  switch (vert) {
    default:
    case AlignCenter: alignment |= TEXT_CENTER_Y; break;
    case AlignTop:    alignment |= TEXT_TOP; break;
    case AlignBottom: alignment |= TEXT_BOTTOM; break;
  }

  SetTextAlignment(alignment);
  NeedRedraw();
}

int RedirectedLabel::HandleKey(XKeyEvent *event) {
  OnyxMessage omsg;

  OXLabel::HandleKey(event);

  omsg.onyx_type = ContainerType;
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

int RedirectedLabel::HandleButton(XButtonEvent *event) {
  OnyxMessage omsg;

  OXLabel::HandleButton(event);

  omsg.onyx_type = ContainerType;
  omsg.id = InternalID;

  omsg.button = event->button;
  omsg.x = event->x;
  omsg.y = event->y;
  omsg.x_root = event->x_root;
  omsg.y_root = event->y_root;

  if (event->type == ButtonPress) {
    omsg.action = ButtonPressed;
    CentralMessageCenter->ProcessMessage(&omsg);
  } else {
    omsg.action = ButtonReleased;
    CentralMessageCenter->ProcessMessage(&omsg);
  }

  return True;
}

int RedirectedLabel::HandleMotion(XMotionEvent *event) {
  OnyxMessage omsg;

  OXLabel::HandleMotion(event);

  omsg.onyx_type = ContainerType;
  omsg.id = InternalID;
  omsg.action = PointerMoved;
  omsg.x = event->x;
  omsg.y = event->y;
  omsg.x_root = event->x_root;
  omsg.y_root = event->y_root;

  return CentralMessageCenter->ProcessMessage(&omsg);
}

RedirectedLabel::~RedirectedLabel() {
}


//----------------------------------------------------------------------

OnyxLabel::OnyxLabel() : OnyxObject("OnyxLabel") {
  LabelExists = 0;
  Width = 100;
  Height = 100;
  X = 0;
  Y = 15;
}

int OnyxLabel::Create() {
  OnyxObject::Create();
  if (!LabelExists) {
    InternalLabel = new RedirectedLabel(Parent, new OString(Text), ID);
    InternalLabel->SetBorderStyle(BorderStyle);
    InternalLabel->SetFont(ApplicationClient()->GetFont(Font));
    InternalLabel->SetupAlignment(HorizontalAlignment, VerticalAlignment);
    InternalLabel->MoveResize(X, Y, Width, Height);
    LabelExists++;
  }
  return LabelExists;
}

void OnyxLabel::Move(int newX, int newY) {
  OnyxObject::Move(newX, newY);
  InternalLabel->Move(newX, newY);
}

void OnyxLabel::Resize(int newWidth, int newHeight) {
  OnyxObject::Resize(newWidth, newHeight);
  InternalLabel->Resize(newWidth, newHeight);
}

void OnyxLabel::MoveResize(int newX, int newY, int newWidth, int newHeight) {
  OnyxObject::MoveResize(newX, newY, newWidth, newHeight);
  InternalLabel->MoveResize(newX, newY, newWidth, newHeight);
}

void OnyxLabel::Update() {
  OnyxObject::Update();
  InternalLabel->SetBorderStyle(BorderStyle);
  InternalLabel->SetText(new OString(Text));
  InternalLabel->SetFont(ApplicationClient()->GetFont(Font));
  InternalLabel->SetupAlignment(HorizontalAlignment, VerticalAlignment);
  InternalLabel->NeedRedraw();
}

OnyxLabel::~OnyxLabel() {
  InternalLabel->DestroyWindow();
  delete InternalLabel;
}
