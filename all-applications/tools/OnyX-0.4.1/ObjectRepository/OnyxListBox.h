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

#ifndef __ONYXLISTBOX_H
#define __ONYXLISTBOX_H

#include <xclass/OXCompositeFrame.h>
#include <xclass/OXListBox.h>

#include "OnyxBase.h"


struct OnyxListText {
  char *Text;
  int Number;
  struct OnyxListText *Next;
};


//----------------------------------------------------------------------

class RedirectedListBox : public OXListBox {
public:
  RedirectedListBox(const OXWindow *p, int ID);

  virtual int HandleButton(XButtonEvent *event);
  virtual int HandleKey(XKeyEvent *event);

private:
  int InternalID;
};

class RedirectedListBoxFrame : public OXCompositeFrame {
public:
  RedirectedListBoxFrame(const OXWindow *p, int w, int h, int ID,
                         int *Enabled, char *Txt);
  virtual ~RedirectedListBoxFrame();

  virtual int HandleButton(XButtonEvent *event);
  virtual int HandleKey(XKeyEvent *event);
  virtual int HandleMotion(XMotionEvent *event);

  virtual void ConnectBox(RedirectedListBox *Bx);
  virtual void ConnectText(struct OnyxListText **TL);

private:
  int InternalID;
  int *IsEnabled;
  RedirectedListBox *Box;
  struct OnyxListText **TextList;
  char *Text;
};


//----------------------------------------------------------------------

class OnyxListBox : public OnyxObject {
public:
  OnyxListBox();
  virtual ~OnyxListBox();

  virtual int  Create();
  virtual void AddEntry(char *Text);
  virtual void SelectEntry(int Entry);
  virtual void RemoveEntry(int Entry);
  virtual void SelectEntry(char *Text);
  virtual void RemoveEntry(char *Text);
  virtual void Move(int newX, int newY);
  virtual void Resize(int newWidth, int newHeight);
  virtual void MoveResize(int newX, int newY, int newWidth, int newHeight);
  virtual void ClearAllEntries();

protected:
  RedirectedListBoxFrame *InternalFrame;
  RedirectedListBox *InternalListBox;
  int ListBoxExists;
  int TotalEntries;
  struct OnyxListText *TextList;
  struct OnyxListText *EndOfList;
};


#endif  // __ONYXLISTBOX_H
