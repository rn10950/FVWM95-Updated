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

#include <xclass/OXClient.h>
#include <xclass/OString.h>
#include <xclass/OPicture.h>

#include "OnyxTree.h"

extern MessageCenter *CentralMessageCenter;


//----------------------------------------------------------------------

RedirectedListTree::RedirectedListTree(const OXWindow *p, int w, int h,
                         int ID, OListTreeItem **pointer, char **selecteditem,
                         char **fullpathname) : 
  OXListTree(p, w, h, ID) {

  InternalID = ID;
  Pointer = pointer;
  SelectedItem = selecteditem;
  FullPathName = fullpathname;
  strcpy(FullPathString, "");
  *FullPathName = FullPathString;

  AddInput(StructureNotifyMask | ButtonPressMask | ButtonReleaseMask |
           PointerMotionMask | KeyPressMask | KeyReleaseMask);
}

int RedirectedListTree::HandleButton(XButtonEvent *event) {
  OnyxMessage omsg;

  omsg.onyx_type = ContainerType;
  omsg.id = InternalID;

  omsg.button = event->button;
  omsg.x = event->x;
  omsg.y = event->y;
  omsg.x_root = event->x_root;
  omsg.y_root = event->y_root;

  OXListTree::HandleButton(event);

  if (event->type == ButtonPress) {
    OListTreeItem *ptr = GetSelected();
    if (ptr) {
      *Pointer = ptr;
      *SelectedItem = ptr->text;
      GetPathnameFromItem(*Pointer, FullPathString);
    } else {
      *Pointer = NULL;
      *SelectedItem = NULL;
      strcpy(FullPathString, "");
    }
    omsg.action = ButtonPressed;
    CentralMessageCenter->ProcessMessage(&omsg);
  } else {
    omsg.action = ButtonReleased;
    CentralMessageCenter->ProcessMessage(&omsg);
  }
  return True;
}

int RedirectedListTree::HandleDoubleClick(XButtonEvent *event) {
  OnyxMessage omsg;

  omsg.onyx_type = ContainerType;
  omsg.id = InternalID;

  omsg.button = event->button;
  omsg.x = event->x;
  omsg.y = event->y;
  omsg.x_root = event->x_root;
  omsg.y_root = event->y_root;

  OXListTree::HandleDoubleClick(event);

  OListTreeItem *ptr = GetSelected();
  if (ptr) {
    *Pointer = ptr;
    *SelectedItem = ptr->text;
    GetPathnameFromItem(*Pointer, FullPathString);
  } else {
    *Pointer = NULL;
    *SelectedItem = NULL;
    strcpy(FullPathString, "");
  }
  omsg.action = ButtonDoubleClicked;
  CentralMessageCenter->ProcessMessage(&omsg);

  return True;
}

int RedirectedListTree::HandleMotion(XMotionEvent *event) {
  OnyxMessage omsg;

  OXListTree::HandleMotion(event);

  omsg.onyx_type = ContainerType;
  omsg.id = InternalID;
  omsg.action = PointerMoved;
  omsg.x = event->x;
  omsg.y = event->y;
  omsg.x_root = event->x_root;
  omsg.y_root = event->y_root;

  return CentralMessageCenter->ProcessMessage(&omsg);
}

int RedirectedListTree::HandleKey(XKeyEvent *event) {
  OnyxMessage omsg;

  OXListTree::HandleKey(event);

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

int RedirectedListTree::ProcessMessage(OMessage *msg) {
  OnyxMessage omsg;

  omsg.onyx_type = ContainerType;
  omsg.id = InternalID;

  if (msg->type == MSG_LISTTREE &&
      (msg->action == MSG_CLICK || msg->action == MSG_DBLCLICK)) {

    OListTreeItem *ptr = GetSelected();
    if (ptr) {
      *Pointer = ptr;
      *SelectedItem = ptr->text;
      GetPathnameFromItem(*Pointer, FullPathString);
    } else {
      *Pointer = NULL;
      *SelectedItem = NULL;
      strcpy(FullPathString, "");
    }
    omsg.action = ItemSelected;
    CentralMessageCenter->ProcessMessage(&omsg);
  } else {
    OXListTree::ProcessMessage(msg);
  }
  return True;
}

RedirectedListTree::~RedirectedListTree() {
}


//----------------------------------------------------------------------

OnyxTree::OnyxTree() : OnyxObject("OnyxTree") {
  TreeExists = 0;
  Width = 100;
  Height = 100;
  X = 0;
  Y = 15;
  Pointer = NULL;
  SelectedItem = NULL;
  FullPathName = NULL;
}

int OnyxTree::Create() {
  OnyxObject::Create();
  if (!TreeExists) {
    InternalPicture = ApplicationPicturePool()->GetPicture(Picture);
    InternalTree = new RedirectedListTree(Parent, Width, Height, ID,
                                          &Pointer, &SelectedItem,
                                          &FullPathName);
    InternalTree->Associate(InternalTree);  // Send messages to itself
    InternalTree->MapSubwindows();
    InternalTree->Layout();
    InternalTree->Move(X, Y);
    TreeExists++;
  }
  return TreeExists;
}

void OnyxTree::AddItem(char *Name) {
  Pointer = InternalTree->AddItem(Pointer, Name, NULL, NULL);
  SelectedItem = Pointer->text;
  InternalTree->GetPathnameFromItem(Pointer, FullPathName);
  Update();
}

#if 0
void OnyxTree::AddItem(char *Name, char *OpenPicture, char *ClosedPicture) {

  if (InternalPicture) ApplicationPicturePool()->FreePicture(InternalPicture);
  InternalPicture = ApplicationPicturePool()->GetPicture(Picture);
 
  Pointer = InternalTree->AddItem(Pointer, Name, NULL, NULL);
  SelectedItem = Pointer->text;
  InternalTree->GetPathnameFromItem(Pointer, FullPathName);
  Update();
}
#endif

void OnyxTree::OpenItem() {
  if (Pointer != NULL) {
    InternalTree->OpenNode(Pointer);
  }
  Update();
}

void OnyxTree::CloseItem() {
  if (Pointer != NULL) {
    InternalTree->CloseNode(Pointer);
  }
  Update();
}

void OnyxTree::MoveUp() {
  if (Pointer != NULL) {
    Pointer = Pointer->parent;
    SelectedItem = Pointer->text;
    InternalTree->GetPathnameFromItem(Pointer, FullPathName);
  }
}

void OnyxTree::DeleteItem() {
  OListTreeItem *tmp;
  if (Pointer != NULL) {
    tmp = Pointer->parent;
    InternalTree->DeleteItem(Pointer);
    Pointer = tmp;
    if (Pointer != NULL) {
      SelectedItem = Pointer->text;
      InternalTree->GetPathnameFromItem(Pointer, FullPathName);
    } else {
      SelectedItem = NULL;
      strcpy(FullPathName, "");
    }
  }
  Update();
}

void OnyxTree::Update() {
  ApplicationClient()->NeedRedraw(InternalTree);
}

void OnyxTree::HighlightItem() {
  InternalTree->HighlightItem(Pointer);
  Update();
}

void OnyxTree::UnhighlightItem() {
  InternalTree->ClearHighlighted();
  Update();
}

void OnyxTree::FindItem(char *ItemName) {
  OListTreeItem *tmp;
  tmp = Pointer;

  while (Pointer != NULL) {
    InternalTree->OpenNode(tmp); //tmp->open = True;
    FindChild(ItemName);
    if (Pointer == NULL) {
      Pointer = tmp->firstchild;
      FindItem(ItemName);
    }

    if (Pointer != NULL) {
      break;
    } else {
      InternalTree->CloseNode(tmp); //tmp->open = False;
      tmp = tmp->nextsibling;
      Pointer = tmp;
    }
  }
}

void OnyxTree::SelectItem(char *ItemName) {
  UnhighlightItem();
  SelectRoot();
  FindItem(ItemName);
  OpenItem();
  HighlightItem();
}

void OnyxTree::FindChild(char *ChildName) {
  Pointer = InternalTree->FindChildByName(Pointer, ChildName);
  if (Pointer != NULL) {
    SelectedItem = Pointer->text;
    InternalTree->GetPathnameFromItem(Pointer, FullPathName);
  } else {
    SelectedItem = NULL;
    strcpy(FullPathName, "");
  }
}

void OnyxTree::FindSibling(char *SiblingName) {
  Pointer = InternalTree->FindSiblingByName(Pointer, SiblingName);
  if (Pointer != NULL) {
    SelectedItem = Pointer->text;
    InternalTree->GetPathnameFromItem(Pointer, FullPathName);
  } else {
    SelectedItem = NULL;
    strcpy(FullPathName, "");
  }
}

void OnyxTree::SelectRoot() {
  Pointer = InternalTree->GetFirstItem();
  if (Pointer != NULL) {
    SelectedItem = Pointer->text;
    InternalTree->GetPathnameFromItem(Pointer, FullPathName);
  } else {
    SelectedItem = NULL;
    strcpy(FullPathName, "");
  }
}

void OnyxTree::Move(int newX, int newY) {
  OnyxObject::Move(newX, newY);
  InternalTree->Move(newX, newY);
}

void OnyxTree::Resize(int newWidth, int newHeight) {
  OnyxObject::Resize(newWidth, newHeight);
  InternalTree->Resize(newWidth, newHeight);
}

void OnyxTree::MoveResize(int newX, int newY, int newWidth, int newHeight) {
  OnyxObject::MoveResize(newX, newY, newWidth, newHeight);
  InternalTree->MoveResize(newX, newY, newWidth, newHeight);
}

OnyxTree::~OnyxTree() {
  InternalTree->DestroyWindow();
  delete InternalTree;
}
