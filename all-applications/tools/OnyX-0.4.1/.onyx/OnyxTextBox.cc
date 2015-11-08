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
#include <X11/keysym.h>

#include "OnyxTextBox.h"

extern MessageCenter *CentralMessageCenter;

static int ButtonIsDown;  //==!==


//----------------------------------------------------------------------

RedirectedTextEntry::RedirectedTextEntry(const OXWindow *p,
    char *str, int ID, int *FocusPreference, int *Pseudo) :
  OXTextEntry(p) {
  InternalID = ID;
  Clicked = 0;
  ButtonIsDown = 0;
  Text = str;
  AddInput(StructureNotifyMask | KeyPressMask| KeyReleaseMask |
           ButtonPressMask | ButtonReleaseMask |
           PointerMotionMask | ExposureMask |
           EnterWindowMask | LeaveWindowMask |
           FocusChangeMask);
  ReleaseFocus = FocusPreference;
  PseudoFocus = Pseudo;
}

int RedirectedTextEntry::HandleCrossing(XCrossingEvent *event) {
  if (event->type == LeaveNotify) {
    Clicked = 0;
    if (*ReleaseFocus) {
      /* If QuickFocusRelease desired then give up the focus. */
      XSetInputFocus(GetDisplay(), PointerRoot, None, CurrentTime);
    }
  } else {
    Clicked = 1;
  }
  return OXTextEntry::HandleCrossing(event);
}

int RedirectedTextEntry::HandleMotion(XMotionEvent *event) {
  OnyxMessage omsg;

  if (ButtonIsDown) {
    OXTextEntry::HandleMotion(event);
  }

  omsg.onyx_type = TextBoxType;
  omsg.id = InternalID;
  omsg.action = PointerMoved;
  omsg.x = event->x;
  omsg.y = event->y;
  omsg.x_root = event->x_root;
  omsg.y_root = event->y_root;

  return CentralMessageCenter->ProcessMessage(&omsg);
}

int RedirectedTextEntry::HandleButton(XButtonEvent *event) {
  OnyxMessage omsg;

  omsg.onyx_type = TextBoxType;
  omsg.id = InternalID;

  omsg.button = event->button;
  omsg.x = event->x;
  omsg.y = event->y;
  omsg.x_root = event->x_root;
  omsg.y_root = event->y_root;

  OXTextEntry::HandleButton(event);

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

int RedirectedTextEntry::HandleKey(XKeyEvent *event) {
  OnyxMessage omsg;

  omsg.onyx_type = TextBoxType;
  omsg.id = InternalID;

  omsg.keysym = XKeycodeToKeysym(GetDisplay(), event->keycode, 0);

  if (event->type == KeyPress) {
    OXTextEntry::HandleKey(event);
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

int RedirectedTextEntry::ProcessMessage(OMessage *msg) {
  OnyxMessage omsg;

  omsg.onyx_type = TextBoxType;
  omsg.id = InternalID;

  if (msg->type == MSG_TEXTENTRY && msg->action == MSG_TEXTCHANGED) {
    strcpy(Text, GetString());
    omsg.action = TextChanged;
    CentralMessageCenter->ProcessMessage(&omsg);
  }
  return True;
}

int RedirectedTextEntry::HandleFocusChange(XFocusChangeEvent *event) {
  if (!*PseudoFocus) {
    /* If pseudo focus isn't on then pass real event. */
    OXTextEntry::HandleFocusChange(event);
  }
  return True;
}


//----------------------------------------------------------------------

OnyxTextBox::OnyxTextBox() : OnyxObject("OnyxTextBox") {
  TextBoxExists = 0;
  Width = 100;
  Height = 10;
  X = 0;
  Y = 15;
  Size = 255;
  QuickFocusRelease = False;
  PseudoFocus = False;
}

int OnyxTextBox::Create() {
  OnyxObject::Create();
  if (!TextBoxExists) {
    InternalFrame = new OXCompositeFrame(Parent, Width, Height);
    InternalTextEntry = new RedirectedTextEntry(InternalFrame, Text, ID,
                                        &QuickFocusRelease, &PseudoFocus);
    InternalTextEntry->AddText(0, Text, strlen(Text), True);
    InternalFrame->AddFrame(InternalTextEntry, new OLayoutHints(LHINTS_EXPAND_ALL));
    InternalFrame->MapSubwindows();
    InternalFrame->Layout();
    InternalFrame->Move(X, Y);
    TextBoxExists++;
/*    InternalTextEntry->Associate(CentralMessageCenter);*/
    // Note the message loop below!!!
    InternalTextEntry->Associate(InternalTextEntry);
  }
  return TextBoxExists;
}

void OnyxTextBox::Move(int newX, int newY) {
  OnyxObject::Move(newX, newY);
  InternalFrame->Move(newX, newY);
}

void OnyxTextBox::Resize(int newWidth, int newHeight) {
  OnyxObject::Resize(newWidth, newHeight);
  InternalFrame->Resize(newWidth, newHeight);
}

void OnyxTextBox::MoveResize(int newX, int newY, int newWidth, int newHeight) {
  OnyxObject::MoveResize(newX, newY, newWidth, newHeight);
  InternalFrame->MoveResize(newX, newY, newWidth, newHeight);
}

void OnyxTextBox::AddText(char *TextToAdd) {
  InternalTextEntry->Clear();
  InternalTextEntry->AddText(0, TextToAdd, strlen(TextToAdd), True);
}

void OnyxTextBox::AppendText(char *TextToAppend) {
  InternalTextEntry->AddText(InternalTextEntry->GetTextLength(),
                             TextToAppend, strlen(TextToAppend), True);
}

void OnyxTextBox::ClearText() {
  InternalTextEntry->Clear(True);
}

void OnyxTextBox::Update() {
  InternalTextEntry->Clear();
  InternalTextEntry->AddText(0, Text);
  InternalTextEntry->NeedRedraw();
}

OnyxTextBox::~OnyxTextBox() {
  InternalFrame->DestroyWindow();
  delete InternalFrame;
}
