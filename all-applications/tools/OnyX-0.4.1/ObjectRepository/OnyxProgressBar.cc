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

#include "OnyxProgressBar.h"

extern MessageCenter *CentralMessageCenter;


//----------------------------------------------------------------------

RedirectedProgressBar::RedirectedProgressBar(const OXWindow *p,
                                             int w, int h, int ID) :
  OXProgressBar(p, w, h, ID) {

  InternalID = ID;
  AddInput(StructureNotifyMask | KeyPressMask| KeyReleaseMask |
           ButtonPressMask | ButtonReleaseMask |
           PointerMotionMask | ExposureMask);
}

int RedirectedProgressBar::HandleMotion(XMotionEvent *event) {
  OnyxMessage omsg;

  OXProgressBar::HandleMotion(event);

  omsg.onyx_type = ContainerType;
  omsg.id = InternalID;
  omsg.action = PointerMoved;
  omsg.x = event->x;
  omsg.y = event->y;
  omsg.x_root = event->x_root;
  omsg.y_root = event->y_root;

  return CentralMessageCenter->ProcessMessage(&omsg);
}

int RedirectedProgressBar::HandleButton(XButtonEvent *event) {
  OnyxMessage omsg;

  omsg.onyx_type = ContainerType;
  omsg.id = InternalID;

  omsg.button = event->button;
  omsg.x = event->x;
  omsg.y = event->y;
  omsg.x_root = event->x_root;
  omsg.y_root = event->y_root;

  OXProgressBar::HandleButton(event);

  if (event->type == ButtonPress) {
    omsg.action = ButtonPressed;
    CentralMessageCenter->ProcessMessage(&omsg);
  } else {
    omsg.action = ButtonReleased;
    CentralMessageCenter->ProcessMessage(&omsg);
  }
  return True;
}

int RedirectedProgressBar::HandleKey(XKeyEvent *event) {
  OnyxMessage omsg;

  omsg.onyx_type = ContainerType;
  omsg.id = InternalID;

  omsg.keysym = XKeycodeToKeysym(GetDisplay(), event->keycode, 0);

  if (event->type == KeyPress) {
    OXProgressBar::HandleKey(event);
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


//----------------------------------------------------------------------

OnyxProgressBar::OnyxProgressBar() : OnyxObject("OnyxProgressBar") {
  ProgressBarExists = 0;
  Width = 100;
  Height = 10;
  X = 0;
  Y = 15;
  Min = 0;
  Max = 100;
  Level = 25;
  ShowPercent = False;
  Size = 255;
  CurrentColor = ApplicationClient()->GetColorByName("#000080");
}

int OnyxProgressBar::Create() {
  OnyxObject::Create();
  if (!ProgressBarExists) {
    InternalProgressBar = new RedirectedProgressBar(Parent, Width, Height, ID);
    InternalProgressBar->SetBorderStyle(BorderStyle);
    InternalProgressBar->SetColor(CurrentColor);
    InternalProgressBar->SetRange(Min, Max);
    InternalProgressBar->SetPosition(Level);
    InternalProgressBar->ShowPercentage(ShowPercent);
    InternalProgressBar->Move(X, Y);
    InternalProgressBar->MapSubwindows();
    InternalProgressBar->Layout();
    ProgressBarExists++;
  }
  return ProgressBarExists;
}

void OnyxProgressBar::Move(int newX, int newY) {
  OnyxObject::Move(newX, newY);
  InternalProgressBar->Move(newX, newY);
}

void OnyxProgressBar::Resize(int newWidth, int newHeight) {
  OnyxObject::Resize(newWidth, newHeight);
  InternalProgressBar->Resize(newWidth, newHeight);
}

void OnyxProgressBar::MoveResize(int newX, int newY, int newWidth, int newHeight) {
  OnyxObject::MoveResize(newX, newY, newWidth, newHeight);
  InternalProgressBar->MoveResize(newX, newY, newWidth, newHeight);
}

void OnyxProgressBar::Update() {
  InternalProgressBar->SetBorderStyle(BorderStyle);
  InternalProgressBar->SetRange(Min, Max);
  InternalProgressBar->SetPosition(Level);
  InternalProgressBar->ShowPercentage(ShowPercent);
  InternalProgressBar->NeedRedraw();
}

void OnyxProgressBar::SetColor(const char *color) {
  ApplicationClient()->FreeColor(CurrentColor);
  CurrentColor = ApplicationClient()->GetColorByName(color);
  InternalProgressBar->SetColor(CurrentColor);
}

void OnyxProgressBar::SetColor(int r, int g, int b) {
  char tmp[20];

  sprintf(tmp, "#%02x%02x%02x", r, g, b);
  SetColor(tmp);
}


OnyxProgressBar::~OnyxProgressBar() {
  InternalProgressBar->DestroyWindow();
  delete InternalProgressBar;
  ApplicationClient()->FreeColor(CurrentColor);
}
