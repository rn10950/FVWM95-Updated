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

#include "OnyxComboBox.h"

extern MessageCenter *CentralMessageCenter;


//----------------------------------------------------------------------

RedirectedComboBoxFrame::RedirectedComboBoxFrame(const OXWindow *p,
                         int w, int h, int ID, char *txt, int *Enabled) :
  OXCompositeFrame(p, w, h) {

  InternalID = ID;
  Text = txt;

  AddInput(StructureNotifyMask | ButtonPressMask | ButtonReleaseMask |
           PointerMotionMask);

  IsEnabled = Enabled;
}

void RedirectedComboBoxFrame::ConnectBox(RedirectedComboBox *Bx) {
  Box = Bx;
}

int RedirectedComboBoxFrame::HandleButton(XButtonEvent *event) {
  OnyxMessage omsg;

  omsg.onyx_type = ComboBoxType;
  omsg.id = InternalID;

  omsg.button = event->button;
  omsg.x = event->x;
  omsg.y = event->y;
  omsg.x_root = event->x_root;
  omsg.y_root = event->y_root;

  OXCompositeFrame::HandleButton(event);

  if (event->type == ButtonPress) {
    omsg.action = ButtonPressed;
    CentralMessageCenter->ProcessMessage(&omsg);
  } else {
    omsg.action = ButtonReleased;
    CentralMessageCenter->ProcessMessage(&omsg);
  }

  return True;
}

int RedirectedComboBoxFrame::HandleMotion(XMotionEvent *event) {
  OnyxMessage omsg;

  OXCompositeFrame::HandleMotion(event);

  omsg.onyx_type = ComboBoxType;
  omsg.id = InternalID;
  omsg.action = PointerMoved;
  omsg.x = event->x;
  omsg.y = event->y;
  omsg.x_root = event->x_root;
  omsg.y_root = event->y_root;

  return CentralMessageCenter->ProcessMessage(&omsg);
}

int RedirectedComboBoxFrame::ProcessMessage(OMessage *msg) {
  OnyxMessage omsg;
  OComboBoxMessage *cmsg;

  omsg.onyx_type = ComboBoxType;
  omsg.id = InternalID;

  if (msg->action == MSG_TEXTCHANGED) {
    strcpy(Text, Box->GetText());
    omsg.action = TextChanged;
    CentralMessageCenter->ProcessMessage(&omsg);
    cmsg = (OComboBoxMessage *) msg;
    if (cmsg->keysym == XK_Return) {
      omsg.action = EnterPressed;
      CentralMessageCenter->ProcessMessage(&omsg);
    }
  } else if (msg->action == MSG_CLICK) {
    strcpy(Text, Box->GetText());
    omsg.action = ItemSelected;
    omsg.item_number = Box->GetSelected();
    CentralMessageCenter->ProcessMessage(&omsg);
  }

  return True;
}

RedirectedComboBoxFrame::~RedirectedComboBoxFrame() {
}


//----------------------------------------------------------------------

RedirectedComboBox::RedirectedComboBox(const OXWindow *p, char *text,
  int ID, int *FocusPreference, int *Pseudo, int *RdOnly) : 
    OXComboBox(p, text, ID) {

  InternalID = ID;
  Clicked = 0;
  ReleaseFocus = FocusPreference;
  PseudoFocus = Pseudo;
  ReadOnly = RdOnly;

  AddInput(StructureNotifyMask | ButtonPressMask |  ButtonReleaseMask |
           PointerMotionMask | ExposureMask | KeyPressMask | KeyReleaseMask |
           EnterWindowMask | LeaveWindowMask);
}

int RedirectedComboBox::HandleFocusChange(XFocusChangeEvent *event) {
  if (!*PseudoFocus) {
    OXComboBox::HandleFocusChange(event);
  }
  return True;
}

int RedirectedComboBox::HandleCrossing(XCrossingEvent *event) {
  if (event->type == LeaveNotify) {
    Clicked = 0;
    if (*ReleaseFocus) {  /* If QuickFocusRelease desired then give up the focus. */
      XSetInputFocus(GetDisplay(), PointerRoot, None, CurrentTime);
    }
  } else {
    Clicked = 1;
  }
  return OXComboBox::HandleCrossing(event);
}

int RedirectedComboBox::HandleMotion(XMotionEvent *event) {
  OnyxMessage omsg;

  OXComboBox::HandleMotion(event); 

  omsg.onyx_type = ComboBoxType;
  omsg.id = InternalID;
  omsg.action = PointerMoved;
  omsg.x = event->x;
  omsg.y = event->y;
  omsg.x_root = event->x_root;
  omsg.y_root = event->y_root;

  return CentralMessageCenter->ProcessMessage(&omsg);
}

int RedirectedComboBox::HandleButton(XButtonEvent *event) {
  OnyxMessage omsg;

  omsg.onyx_type = ComboBoxType;
  omsg.id = InternalID;

  omsg.button = event->button;
  omsg.x = event->x;
  omsg.y = event->y;
  omsg.x_root = event->x_root;
  omsg.y_root = event->y_root;

  OXComboBox::HandleButton(event);

  if (event->type == ButtonPress) {
    Clicked = 1;
    omsg.action = ButtonPressed;
    CentralMessageCenter->ProcessMessage(&omsg);
  } else {
    omsg.action = ButtonReleased;
    CentralMessageCenter->ProcessMessage(&omsg);
    if (Clicked) {
      omsg.action = ButtonClicked;
      CentralMessageCenter->ProcessMessage(&omsg);
    }
  }

  return True;
}

int RedirectedComboBox::HandleKey(XKeyEvent *event) {
  OnyxMessage omsg;

  omsg.onyx_type = ComboBoxType;
  omsg.id = InternalID;

  omsg.keysym = XKeycodeToKeysym(GetDisplay(), event->keycode, 0);

  if (event->type == KeyPress) {
    if (!*ReadOnly) {  // TODO: propagate F4 and up/down even if ReadOnly
      OXComboBox::HandleKey(event);
    }
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
 
OnyxComboBox::OnyxComboBox() : OnyxObject("OnyxComboBox") {
  ComboBoxExists = 0;
  Width = 100;
  Height = 100;
  X = 0;
  Y = 15;
  TotalEntries = 0;
  ReadOnly = False;
  QuickFocusRelease = False;
  PseudoFocus = False;
  Size = 255;
}

int OnyxComboBox::Create() {
  OnyxObject::Create();
  if (!ComboBoxExists) {
    InternalFrame = new RedirectedComboBoxFrame(Parent, Width, Height, ID,
                                                Text, &IsEnabled);
    InternalComboBox = new RedirectedComboBox(InternalFrame, Text, ID,
                                              &QuickFocusRelease,
                                              &PseudoFocus, &ReadOnly);
    InternalComboBox->Resize(Width, Height);
    InternalFrame->AddFrame(InternalComboBox, new OLayoutHints(LHINTS_EXPAND_ALL));
    InternalFrame->MapSubwindows();
    InternalFrame->Layout();
    InternalFrame->Move(X, Y);
    InternalComboBox->Associate(InternalFrame);
    InternalFrame->ConnectBox(InternalComboBox);

    //strcpy(Text, InternalComboBox->GetText());

    ComboBoxExists++;
  }

  return ComboBoxExists;
}

void OnyxComboBox::AddEntry(char *text) {
  InternalComboBox->AddEntry(new OString(text), TotalEntries++);
}

void OnyxComboBox::SelectEntry(int Entry) {
  InternalComboBox->Select(Entry);
}

#if 0
void OnyxComboBox::SelectEntry(char *text) {
  struct OnyxDropText *temp;

  temp = TextList;
  while (temp) {
    if (!strcmp(text, temp->Text)) {
      InternalTextBuffer->Clear();
      InternalTextBuffer->AddText(0, temp->Text);
      InternalTextEntry->NeedRedraw();
      InternalComboBox->Select(temp->Number);
      temp = NULL;
    } else {
      temp = temp->Next;
    }
  }
}
#endif

void OnyxComboBox::RemoveEntry(int Entry) {
  InternalComboBox->RemoveEntry(Entry);
}

#if 0
void OnyxComboBox::RemoveEntry(char *text) {
  struct OnyxDropText *temp, *prev;
  int Entry, Found;

  prev = NULL;
  temp = TextList;

  Found = False;
  while (temp) {
    if (!strcmp(text, temp->Text)) {   // If this is the requested entry
      Found = True;                    // Entry was located.
      Entry = temp->Number;            // Save number of entry.
      if (prev) {
        prev->Next = temp->Next;       // Remove node frop middle of list.
      } else {
        TextList = temp->Next;         // prev is null. Then move head of list.
      }

      delete[] temp->Text;             // Free space allocated for text.
      delete temp;                     // Free space allocated for structure.
      break;                           // Jump out of while loop.
    } else {
      // Otherwise advance temp to the next node
      // with prev trailing right behind.
      prev = temp;
      temp = temp->Next;
    }
  }

  if (Found) {
    InternalComboBox->RemoveEntry(Entry);
  }
}
#endif

void OnyxComboBox::ClearAllEntries() {
#if 0
  struct OnyxDropText *temp, *prev;

  prev = TextList;
  temp = TextList;

  while (temp) {
    temp = temp->Next;    // Advance temp tp the next entry.
    InternalComboBox->RemoveEntry(prev->Number);  // Remove from visible list.
    delete[] prev->Text;  // Free space allocated for text.
    delete prev;          // Free space allocated for structure.
    prev = temp;          // Set to null to jump out of while loop.
  }
  TextList = NULL;
  EndOfList = NULL;
#else
  InternalComboBox->RemoveAllEntries();
#endif
  TotalEntries = 0;
}

void OnyxComboBox::Move(int newX, int newY) {
  OnyxObject::Move(newX, newY);
  InternalFrame->Move(newX, newY);
}

void OnyxComboBox::Resize(int newWidth, int newHeight) {
  OnyxObject::Resize(newWidth, newHeight);
  InternalFrame->Resize(newWidth, newHeight);
}

void OnyxComboBox::MoveResize(int newX, int newY, int newWidth, int newHeight) {
  OnyxObject::MoveResize(newX, newY, newWidth, newHeight);
  InternalFrame->MoveResize(newX, newY, newWidth, newHeight);
}

void OnyxComboBox::AddText(char *TextToAdd) {
  OXTextEntry *te = InternalComboBox->GetTextEntry();
  te->Clear();
  te->AddText(0, TextToAdd);
}
        
void OnyxComboBox::AppendText(char *TextToAppend) {
  OXTextEntry *te = InternalComboBox->GetTextEntry();
  te->AddText(te->GetTextLength(), TextToAppend);
}
 
void OnyxComboBox::ClearText() {
  OXTextEntry *te = InternalComboBox->GetTextEntry();
  te->Clear();
}

void OnyxComboBox::Update() {
  OnyxObject::Update();
  InternalComboBox->NeedRedraw();
}
        
OnyxComboBox::~OnyxComboBox() {
  InternalFrame->DestroyWindow();
  delete InternalFrame;
}
