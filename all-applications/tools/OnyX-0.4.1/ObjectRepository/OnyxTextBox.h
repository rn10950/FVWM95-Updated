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

#ifndef __ONYXTEXTBOX_H
#define __ONYXTEXTBOX_H

#include <xclass/OXCompositeFrame.h>
#include <xclass/OXTextEntry.h>

#include "OnyxBase.h"


#if 0
#define TextChanged  MSG_TEXTCHANGED  /* Define Text Changed Event ID */
#define EnterPressed 2                /* Define Enter Pressed Event ID */
#endif


//----------------------------------------------------------------------

class RedirectedTextEntry : public OXTextEntry {
public:
  RedirectedTextEntry(const OXWindow *p, char *str, int ID,
                      int *FocusPreference, int *Pseudo);

  virtual int ProcessMessage(OMessage *msg);

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
  char *Text;
};


//----------------------------------------------------------------------

class OnyxTextBox : public OnyxObject {
public:
  OnyxTextBox();
  virtual ~OnyxTextBox();

  virtual int  Create();
  virtual void Move(int newX, int newY);
  virtual void Resize(int newWidth, int newHeight);
  virtual void MoveResize(int newX, int newY, int newWidth, int newHeight);
  virtual void Update();
  virtual void AddText(char *TextToAdd);
  virtual void AppendText(char *TextToAppend);
  virtual void ClearText();

  int QuickFocusRelease;
  int PseudoFocus;

protected:
  OXCompositeFrame *InternalFrame;
  RedirectedTextEntry *InternalTextEntry;
  int TextBoxExists;
};


#endif  // __ONYXTEXTBOX_H
