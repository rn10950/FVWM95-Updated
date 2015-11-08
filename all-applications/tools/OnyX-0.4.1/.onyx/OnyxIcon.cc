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

#include <X11/X.h>
#include <X11/extensions/shape.h>

#include "OnyxIcon.h"

extern MessageCenter *CentralMessageCenter;


//----------------------------------------------------------------------

RedirectedIcon::RedirectedIcon(const OXWindow *p, const OPicture *pic,
    int w, int h, int ID) : OXIcon(p, pic, w, h) {
  InternalID = ID;

  AddInput(StructureNotifyMask | ButtonPressMask | ButtonReleaseMask |
           PointerMotionMask | KeyPressMask | KeyReleaseMask);
}

int RedirectedIcon::HandleButton(XButtonEvent *event) {
  OnyxMessage omsg;

  OXIcon::HandleButton(event);

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

int RedirectedIcon::HandleKey(XKeyEvent *event) {
  OnyxMessage omsg;

  OXIcon::HandleKey(event);

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

int RedirectedIcon::HandleMotion(XMotionEvent *event) {
  OnyxMessage omsg;

  OXIcon::HandleMotion(event);

  omsg.onyx_type = ContainerType;
  omsg.id = InternalID;
  omsg.action = PointerMoved;
  omsg.x = event->x;
  omsg.y = event->y;
  omsg.x_root = event->x_root;
  omsg.y_root = event->y_root;

  return CentralMessageCenter->ProcessMessage(&omsg);
}

void RedirectedIcon::SetupShape(int onoff) {
  if (onoff && _pic) {
    if (_pic->GetMask()) {
      XShapeCombineMask(GetDisplay(), _id, ShapeBounding,
                        _insets.t, _insets.b, _pic->GetMask(), ShapeSet);
    } else {
      XRectangle rect;

      rect.x = 0;
      rect.y = 0;
      rect.width = _pic->GetWidth();
      rect.height = _pic->GetHeight();
      XShapeCombineRectangles(GetDisplay(), _id, ShapeBounding,
                              _insets.t, _insets.b, &rect, 1, ShapeSet, Unsorted);
    }
  } else {
    XShapeCombineMask(GetDisplay(), _id, ShapeBounding,
                      _insets.t, _insets.b, None, ShapeSet);
  }
}

RedirectedIcon::~RedirectedIcon() {
}

//----------------------------------------------------------------------

OnyxIcon::OnyxIcon() : OnyxObject("OnyxIcon") {
  IconExists = 0;
  Width = 32;
  Height = 32;
  IsShaped = 0;
}

int OnyxIcon::Create() {
  OnyxObject::Create();
  if (!IconExists) {
    InternalPicture = ApplicationPicturePool()->GetPicture(Picture);
    InternalIcon = new RedirectedIcon(Parent, InternalPicture, Width, Height, ID);
    InternalIcon->SetBorderStyle(BorderStyle);
    InternalIcon->Move(X, Y);
    InternalIcon->SetupShape(IsShaped);
    IconExists++;
  }
  return IconExists;
}

void OnyxIcon::Move(int newX, int newY) {
  OnyxObject::Move(newX, newY);
  InternalIcon->Move(newX, newY);
}

void OnyxIcon::Resize(int newWidth, int newHeight) {
  OnyxObject::Resize(newWidth, newHeight);
  InternalIcon->Resize(newWidth, newHeight);
  InternalIcon->SetupShape(IsShaped);
}

void OnyxIcon::MoveResize(int newX, int newY, int newWidth, int newHeight) {
  OnyxObject::MoveResize(newX, newY, newWidth, newHeight);
  InternalIcon->MoveResize(newX, newY, newWidth, newHeight);
  InternalIcon->SetupShape(IsShaped);
}

void OnyxIcon::Update() {
  OnyxObject::Update();
  if (InternalPicture) ApplicationPicturePool()->FreePicture(InternalPicture);
  InternalPicture = ApplicationPicturePool()->GetPicture(Picture);
  InternalIcon->SetPicture(InternalPicture);
  InternalIcon->SetBorderStyle(BorderStyle);
  InternalIcon->SetupShape(IsShaped);
  ApplicationClient()->NeedRedraw(InternalIcon);
}

OnyxIcon::~OnyxIcon() {
  InternalIcon->DestroyWindow();
  delete InternalIcon;
}
