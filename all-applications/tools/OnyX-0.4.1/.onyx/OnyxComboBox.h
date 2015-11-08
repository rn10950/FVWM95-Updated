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

#ifndef __ONYXCOMBOBOX_H
#define __ONYXCOMBOBOX_H

#include <xclass/OXCompositeFrame.h>
#include <xclass/OXComboBox.h>

#include "OnyxBase.h"


//----------------------------------------------------------------------

class RedirectedComboBox : public OXComboBox {
public:
  RedirectedComboBox(const OXWindow *p, char *text, int ID,
                     int *FocusPreference, int *Pseudo, int *RdOnly);

  virtual int HandleButton(XButtonEvent *event);
  virtual int HandleCrossing(XCrossingEvent *event);
  virtual int HandleMotion(XMotionEvent *event);
  virtual int HandleKey(XKeyEvent *event);
  virtual int HandleFocusChange(XFocusChangeEvent *event);

private:
  int InternalID;
  int Clicked;
  int *ReleaseFocus;
  int *PseudoFocus;
  int *ReadOnly;
};

class RedirectedComboBoxFrame : public OXCompositeFrame {
public:
  RedirectedComboBoxFrame(const OXWindow *p, int w, int h, int ID,
                          char *txt, int *Enabled);
  virtual ~RedirectedComboBoxFrame();

  virtual int HandleButton(XButtonEvent *event);
  virtual int HandleMotion(XMotionEvent *event);

  virtual int ProcessMessage(OMessage *msg);

  void ConnectBox(RedirectedComboBox *Bx);

private:
  int InternalID;
  int *IsEnabled;
  RedirectedComboBox *Box;
  char *Text;
};


//----------------------------------------------------------------------

class OnyxComboBox : public OnyxObject {
public:
  OnyxComboBox();
  virtual ~OnyxComboBox();

  virtual int  Create();
  virtual void Move(int newX, int newY);
  virtual void Resize(int newWidth, int newHeight);
  virtual void MoveResize(int newX, int newY, int newWidth, int newHeight);
  virtual void Update();
  virtual void AddEntry(char *Text);
  virtual void SelectEntry(int Entry);
  virtual void RemoveEntry(int Entry);
//  virtual void SelectEntry(char *Text);
//  virtual void RemoveEntry(char *Text);
  virtual void AddText(char *TextToAdd);
  virtual void AppendText(char *TextToAppend);
  virtual void ClearText();
  virtual void ClearAllEntries();

  int ReadOnly;
  int QuickFocusRelease;
  int PseudoFocus;

protected:
  RedirectedComboBoxFrame *InternalFrame;
  RedirectedComboBox *InternalComboBox;
  int ComboBoxExists;
  int TotalEntries;
};


#endif  // __ONYXCOMBOBOX_H
