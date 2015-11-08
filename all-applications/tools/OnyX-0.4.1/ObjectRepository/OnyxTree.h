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

#ifndef __ONYXTREE_H
#define __ONYXTREE_H

#include <xclass/OXClient.h>
#include <xclass/OXWindow.h>
#include <xclass/OXFrame.h>
#include <xclass/OPicture.h>
#include <xclass/OXListTree.h>

#include "OnyxBase.h"


#define BranchClosed 0
#define BranchOpen 1


//----------------------------------------------------------------------

class RedirectedListTree : public OXListTree {
public:
  RedirectedListTree(const OXWindow *p, int w, int h, int ID,
                     OListTreeItem **pointer, char **selecteditem,
                     char **fullpathname);
  virtual ~RedirectedListTree();

  virtual int HandleButton(XButtonEvent *event);
  virtual int HandleDoubleClick(XButtonEvent *event);
  virtual int HandleMotion(XMotionEvent *event);
  virtual int HandleKey(XKeyEvent *event);

  virtual int ProcessMessage(OMessage *msg);

private:
  int InternalID;
  OListTreeItem **Pointer;
  char **SelectedItem;
  char **FullPathName;
  char FullPathString[255];
};


//----------------------------------------------------------------------

class OnyxTree : public OnyxObject {
public:
  OnyxTree();
  virtual ~OnyxTree();

  virtual int  Create();
  virtual void Move(int newX, int newY);
  virtual void Resize(int newWidth, int newHeight);
  virtual void MoveResize(int newX, int newY, int newWidth, int newHeight);
  virtual void Update();
  virtual void AddItem(char *Name);
  virtual void MoveUp();
  virtual void DeleteItem();
  virtual void SelectRoot();
  virtual void OpenItem();
  virtual void CloseItem();
  virtual void HighlightItem();
  virtual void UnhighlightItem();
  virtual void FindItem(char *ItemName);
  virtual void SelectItem(char *ItemName);
  virtual void FindChild(char *ChildName);
  virtual void FindSibling(char *SiblingName);
//  virtual void AddItem(char *string, char *OpenPicture, char *ClosedPicture);

  char *SelectedItem;
  char *FullPathName;

protected:
  RedirectedListTree *InternalTree;
  int TreeExists;
  OListTreeItem *Pointer;
  const OPicture *InternalPicture;
};


#endif  // __ONYXTREE_H
