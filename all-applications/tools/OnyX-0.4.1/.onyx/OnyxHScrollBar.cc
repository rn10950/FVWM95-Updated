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

#include "OnyxHScrollBar.h"

extern MessageCenter *CentralMessageCenter;


//----------------------------------------------------------------------

RedirectedHScrollFrame::RedirectedHScrollFrame(const OXWindow *p, int w, int h,
  int ID, RedirectedHScrollBar **SB) : OXCompositeFrame(p, w, h) {

  AddInput(StructureNotifyMask | ButtonPressMask | ButtonReleaseMask |
           PointerMotionMask | KeyPressMask | KeyReleaseMask |
           EnterWindowMask | LeaveWindowMask | FocusChangeMask);
  ScrollBar = SB;
}

int RedirectedHScrollFrame::HandleKey(XKeyEvent *event) {
  return (*ScrollBar)->HandleKey(event);
}

int RedirectedHScrollFrame::HandleButton(XButtonEvent *event) {
  return (*ScrollBar)->HandleButton(event);
}
         
int RedirectedHScrollFrame::HandleMotion(XMotionEvent *event) {
  return (*ScrollBar)->HandleMotion(event);
}

RedirectedHScrollFrame::~RedirectedHScrollFrame() {
}


//----------------------------------------------------------------------

RedirectedHScrollBar::RedirectedHScrollBar(const OXWindow *p, int w, int h,
  int ID, int *Pos) : OXHScrollBar(p, w, h) {

  InternalID = ID;
  Position = Pos;
}

int RedirectedHScrollBar::HandleKey(XKeyEvent *event) {
  OnyxMessage omsg;

  omsg.onyx_type = HorizontalScrollType;
  omsg.id = InternalID;

  omsg.keysym = XKeycodeToKeysym(GetDisplay(), event->keycode, 0);

  OXHScrollBar::HandleKey(event);

  if (event->type == KeyPress) {
    omsg.action = KeyPressed;
    CentralMessageCenter->ProcessMessage(&omsg);
  } else {
    omsg.action = KeyReleased;
    CentralMessageCenter->ProcessMessage(&omsg);
  }
  return True;
}

int RedirectedHScrollBar::HandleButton(XButtonEvent *event) {
  OnyxMessage omsg;

  omsg.onyx_type = HorizontalScrollType;
  omsg.id = InternalID;

  omsg.button = event->button;
  omsg.x = event->x;
  omsg.y = event->y;
  omsg.x_root = event->x_root;
  omsg.y_root = event->y_root;

  OXHScrollBar::HandleButton(event);

  if (event->type == ButtonPress) {
    omsg.action = ButtonPressed;
    CentralMessageCenter->ProcessMessage(&omsg);
  } else {
    omsg.action = ButtonReleased;
    CentralMessageCenter->ProcessMessage(&omsg);
  }

  return True;
}

int RedirectedHScrollBar::HandleMotion(XMotionEvent *event) {
  OnyxMessage omsg;

  OXHScrollBar::HandleMotion(event);

  omsg.onyx_type = HorizontalScrollType;
  omsg.id = InternalID;
  omsg.action = PointerMoved;
  omsg.x = event->x;
  omsg.y = event->y;
  omsg.x_root = event->x_root;
  omsg.y_root = event->y_root;

  return CentralMessageCenter->ProcessMessage(&omsg);
}

int RedirectedHScrollBar::ProcessMessage(OMessage *msg) {
  OnyxMessage omsg;

  omsg.onyx_type = HorizontalScrollType;
  omsg.id = InternalID;

  *Position = GetPosition();
  omsg.position = *Position;

  if (msg->type == MSG_SLIDERTRACK) {
    omsg.action = ScrollBarMoved;
    CentralMessageCenter->ProcessMessage(&omsg);
  } else if (msg->type == MSG_SLIDERPOS) {
    omsg.action = ScrollBarPositioned;
    CentralMessageCenter->ProcessMessage(&omsg);
  } else {
    if (msg->type == MSG_LINEUP) {
      omsg.action = LineScrolled;
      omsg.direction = 1;
    } else if (msg->type == MSG_LINEDOWN) {
      omsg.action = LineScrolled;
      omsg.direction = 0;
    } else if (msg->type == MSG_PAGEUP) {
      omsg.action = PageScrolled;
      omsg.direction = 1;
    } else {
      omsg.action = PageScrolled;
      omsg.direction = 0;
    }
    CentralMessageCenter->ProcessMessage(&omsg);
  }

  return True;
}

RedirectedHScrollBar::~RedirectedHScrollBar() {
}


//----------------------------------------------------------------------

OnyxHScrollBar::OnyxHScrollBar() : OnyxObject("OnyxHScrollBar") {
  ScrollBarExists = 0;
  Width = 100;
  Height = 16;
  X = 0;
  Y = 15;
  Range = 100;
  PageSize = 25;
  Position = 0;
}

int OnyxHScrollBar::Create() {
  OnyxObject::Create();
  if (!ScrollBarExists) {
    InternalFrame = new RedirectedHScrollFrame(Parent, Width, Height, ID,
                                               &InternalScrollBar);
    InternalScrollBar = new RedirectedHScrollBar(InternalFrame, Width, Height,
                                                 ID, &Position);
    InternalFrame->AddFrame(InternalScrollBar, new OLayoutHints(LHINTS_EXPAND_ALL));
    InternalFrame->MapSubwindows();
    InternalFrame->Layout();   
    InternalFrame->Move(X, Y);
    InternalScrollBar->Associate(InternalScrollBar);
    InternalScrollBar->SetRange(Range, PageSize, False);
    InternalScrollBar->SetPosition(Position, False);

    ScrollBarExists++;
  }

  return ScrollBarExists;
}

void OnyxHScrollBar::Move(int newX, int newY) {
  OnyxObject::Move(newX, newY);
  InternalFrame->Move(newX, newY);
}

void OnyxHScrollBar::Resize(int newWidth, int newHeight) {
  OnyxObject::Resize(newWidth, newHeight);
  InternalFrame->Resize(newWidth, newHeight);
}

void OnyxHScrollBar::MoveResize(int newX, int newY, int newWidth, int newHeight) {
  OnyxObject::MoveResize(newX, newY, newWidth, newHeight);
  InternalFrame->MoveResize(newX, newY, newWidth, newHeight);
}

void OnyxHScrollBar::ChangeRange(int NewRange) {
  Range = NewRange;
  InternalScrollBar->SetRange(Range, PageSize);
}

void OnyxHScrollBar::ChangePageSize(int NewPageSize) {
  PageSize = NewPageSize;
  InternalScrollBar->SetRange(Range, PageSize);
}

void OnyxHScrollBar::ChangeRangeAndPageSize(int NewRange, int NewPageSize) {
  Range = NewRange;
  PageSize = NewPageSize;
  InternalScrollBar->SetRange(Range, PageSize);
}

void OnyxHScrollBar::SetPosition(int NewPosition) {
  InternalScrollBar->SetPosition(NewPosition);
  // Just to ensure that the position took we won't just set it ourselves.
  Position = InternalScrollBar->GetPosition();
}

void OnyxHScrollBar::Update() {
  OnyxObject::Update();
  InternalScrollBar->NeedRedraw();
}

OnyxHScrollBar::~OnyxHScrollBar() {
  InternalFrame->DestroyWindow();
  delete InternalFrame;
}
