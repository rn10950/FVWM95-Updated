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

#include "OnyxListBox.h"

extern MessageCenter *CentralMessageCenter;


//----------------------------------------------------------------------

RedirectedListBoxFrame::RedirectedListBoxFrame(const OXWindow *p,
                           int w, int h, int ID, int *Enabled, char *Txt) :
   OXCompositeFrame(p, w, h) {

  InternalID = ID;

  AddInput(StructureNotifyMask | ButtonPressMask | ButtonReleaseMask |
           PointerMotionMask | KeyPressMask | KeyReleaseMask | ExposureMask);

  IsEnabled = Enabled;
  Text = Txt;
}

#if 0
RedirectedListBoxFrame::HandleEvent(XEvent *event) {
  struct OnyxListText *temp;
  int Entry;
  
  if (*IsEnabled) {
    Box->HandleEvent(event);
  }

  if (event->type == 33) {
    Entry = Box->GetSelected();
    temp = *TextList;
    while (temp) {
      if (temp->Number == Entry) {
        strcpy(Text, temp->Text);
        temp = NULL;
      } else {
        temp = temp->Next;
      }
    }
    CentralMessageCenter->ProcessMessage(MK_MSG(CommandGroup,ListBoxType),InternalID,ItemSelected,Entry);
  }
  OXCompositeFrame::HandleEvent(event);

}
#endif

void RedirectedListBoxFrame::ConnectBox(RedirectedListBox *Bx) {
  Box = Bx;
}

int RedirectedListBoxFrame::HandleButton(XButtonEvent *event) {
  OnyxMessage omsg;

  omsg.onyx_type = ListBoxType;
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

int RedirectedListBoxFrame::HandleKey(XKeyEvent *event) {
  OnyxMessage omsg;

  omsg.onyx_type = ListBoxType;
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

int RedirectedListBoxFrame::HandleMotion(XMotionEvent *event) {
  OnyxMessage omsg;

  OXCompositeFrame::HandleMotion(event);

  omsg.onyx_type = ListBoxType;
  omsg.id = InternalID;
  omsg.action = PointerMoved;
  omsg.x = event->x;
  omsg.y = event->y;
  omsg.x_root = event->x_root;
  omsg.y_root = event->y_root;

  return CentralMessageCenter->ProcessMessage(&omsg);
}

void RedirectedListBoxFrame::ConnectText(struct OnyxListText **TL) {
  TextList = TL;
}


RedirectedListBoxFrame::~RedirectedListBoxFrame() {
}


//----------------------------------------------------------------------

RedirectedListBox::RedirectedListBox(const OXWindow *p, int ID) :
  OXListBox(p, ID) {

  InternalID = ID;
  AddInput(StructureNotifyMask | ButtonPressMask | ButtonReleaseMask |
           PointerMotionMask | KeyPressMask | KeyReleaseMask | ExposureMask);
}
 
int RedirectedListBox::HandleKey(XKeyEvent *event) {
  OnyxMessage omsg;

  omsg.onyx_type = ListBoxType;
  omsg.id = InternalID;

  omsg.keysym = XKeycodeToKeysym(GetDisplay(), event->keycode, 0);

  OXListBox::HandleKey(event);

  if (event->type == KeyPress) {
    omsg.action = KeyPressed;
    CentralMessageCenter->ProcessMessage(&omsg);
  } else {
    omsg.action = KeyReleased;
    CentralMessageCenter->ProcessMessage(&omsg);
  }
  return True;
}

int RedirectedListBox::HandleButton(XButtonEvent *event) {
  OnyxMessage omsg;

  omsg.onyx_type = ListBoxType;
  omsg.id = InternalID;

  omsg.button = event->button;
  omsg.x = event->x;
  omsg.y = event->y;
  omsg.x_root = event->x_root;
  omsg.y_root = event->y_root;

  OXListBox::HandleButton(event);

  if (event->type == ButtonPress) {
    omsg.action = ButtonPressed;
    CentralMessageCenter->ProcessMessage(&omsg);
  } else {
    omsg.action = ButtonReleased;
    CentralMessageCenter->ProcessMessage(&omsg);
  }
  return True;
}


//----------------------------------------------------------------------

OnyxListBox::OnyxListBox() : OnyxObject("OnyxListBox") {
  ListBoxExists = 0;
  Width = 100;
  Height = 100;
  X = 0;
  Y = 15;
  TotalEntries = 0;
  TextList = NULL;
  EndOfList = NULL;
}

int OnyxListBox::Create() {
  OnyxObject::Create();
  if (!ListBoxExists) {
    InternalFrame = new RedirectedListBoxFrame(Parent, Width, Height, ID,
                                               &IsEnabled, Text);
    InternalListBox = new RedirectedListBox(InternalFrame, ID);
    InternalListBox->Resize(Width, Height);
    InternalFrame->AddFrame(InternalListBox, new OLayoutHints(LHINTS_EXPAND_ALL));
    InternalFrame->MapSubwindows();
    InternalFrame->Layout();
    InternalFrame->Move(X, Y);
    InternalListBox->Associate(InternalFrame);
    InternalFrame->ConnectBox(InternalListBox);
    InternalFrame->ConnectText(&TextList);

    ListBoxExists++;
  }

  return ListBoxExists;
}

void OnyxListBox::AddEntry(char *text) {
  if (EndOfList == NULL) {
    TextList = new OnyxListText;
    EndOfList = TextList;
  } else {
    EndOfList->Next = new OnyxListText;
    EndOfList = EndOfList->Next;
  }
  EndOfList->Next = NULL;
  EndOfList->Text = new char[strlen(text) + 1];
  strcpy(EndOfList->Text, text);
  EndOfList->Number = TotalEntries;

  InternalListBox->AddEntry(new OString(text), TotalEntries++);

  // Force the remapping of the internal entries so taht they will show.
  InternalListBox->MapSubwindows();
  InternalListBox->Layout();
}

void OnyxListBox::SelectEntry(int Entry) {
  struct OnyxListText *temp;

  temp = TextList;
  while (temp) {
    if (temp->Number == Entry) {
      InternalListBox->Select(temp->Number);
      strcpy(Text, temp->Text);
      temp = NULL;
    } else {
      temp = temp->Next;
    }
  }
}

void OnyxListBox::SelectEntry(char *EntryText) {
  struct OnyxListText *temp;

  temp = TextList;
  while (temp) {
    if (!strcmp(EntryText, temp->Text)) {
      InternalListBox->Select(temp->Number);
      strcpy(Text, temp->Text);
      temp = NULL;
    } else {
      temp = temp->Next;
    }
  }

}

void OnyxListBox::RemoveEntry(int Entry) {
  struct OnyxListText *temp, *prev;

  prev = NULL;
  temp = TextList;

  while (temp) {
    if (temp->Number == Entry) {  // If this is the requested entry
      if (prev) {
        prev->Next = temp->Next;  // Remove node frop middle of list.
      } else {
        TextList = temp->Next;    // prev is null. Then move head of list.
      }

      delete[] temp->Text;        // Free space allocated for text.
      delete temp;                // Free space allocated for structure.
      break;
    } else {
      // Otherwise advance temp to the next node with prev trailing right
      // behind.
      prev = temp;
      temp = temp->Next;
    }
  }
  InternalListBox->RemoveEntry(Entry);

  // I don't know why these are needed here and not in the drop box.
  // But with them entries don't show.
  InternalListBox->MapSubwindows();
  InternalListBox->Layout();
}

void OnyxListBox::RemoveEntry(char *EntryText) {
  struct OnyxListText *temp, *prev;
  int Entry, Found;

  prev = NULL;
  temp = TextList;

  Found = False;
  while (temp) {
    if (!strcmp(EntryText, temp->Text)) {  // If this is the requested entry
      Found = True;          // Entry was located.
      Entry = temp->Number;  // Save number of entry.
      if (prev) {
        prev->Next = temp->Next;  // Remove node frop middle of list.
      } else {
        TextList = temp->Next;    // prev is null. Then move head of list.
      }

      delete[] temp->Text;  // Free space allocated for text.
      delete temp;          // Free space allocated for structure.
      break;
    } else {
      // Otherwise advance temp to the next node with prev trailing right
      // behind.
      prev = temp;
      temp = temp->Next;
    }
  }

  if (Found) {
    InternalListBox->RemoveEntry(Entry);
  }

  // I don't know why these are needed here and not in the drop box.
  // But with them entries don't show.
  InternalListBox->MapSubwindows();
  InternalListBox->Layout();
}

void OnyxListBox::ClearAllEntries() {
  struct OnyxListText *temp, *prev;

  prev = TextList;
  temp = TextList;

  while (temp) {
    temp = temp->Next;  // Advance temp tp the next entry.
    InternalListBox->RemoveEntry(prev->Number);  // Remove from visible list.
    delete[] prev->Text;  // Free space allocated for text.
    delete prev;          // Free space allocated for structure.
    prev = temp;
  }
  TextList = NULL;
  EndOfList = NULL;
  TotalEntries = 0;
}

void OnyxListBox::Move(int newX, int newY) {
  OnyxObject::Move(newX, newY);
  InternalFrame->Move(newX, newY);
}

void OnyxListBox::Resize(int newWidth, int newHeight) {
  OnyxObject::Resize(newWidth, newHeight);
  InternalFrame->Resize(newWidth, newHeight);
}

void OnyxListBox::MoveResize(int newX, int newY, int newWidth, int newHeight) {
  OnyxObject::MoveResize(newX, newY, newWidth, newHeight);
  InternalFrame->MoveResize(newX, newY, newWidth, newHeight);
}

OnyxListBox::~OnyxListBox() {
  struct OnyxListText *temp, *prev;

  prev = TextList;
  temp = TextList;

  while (temp) {
    temp = temp->Next;    // Advance temp tp the next entry.
    delete[] prev->Text;  // Free space allocated for text.
    delete prev;          // Free space allocated for structure.
    prev = temp;
  }
  TextList = NULL;
  EndOfList = NULL;
  TotalEntries = 0;

  InternalFrame->DestroyWindow();
  delete InternalFrame;
}
