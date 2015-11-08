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

#ifndef __ONYXHSCROLLBAR_H
#define __ONYXHSCROLLBAR_H

#include <xclass/OXCompositeFrame.h>
#include <xclass/OXScrollBar.h>

#include "OnyxBase.h"


#define LineScrolled 1
#define PageScrolled 2
#define ScrollBarMoved 3
#define ScrollBarPositioned 4


//----------------------------------------------------------------------

class RedirectedHScrollBar : public OXHScrollBar {
public:
  RedirectedHScrollBar(const OXWindow *p, int w, int h, int ID, int *Pos);
  ~RedirectedHScrollBar();

  virtual int HandleKey(XKeyEvent *event);
  virtual int HandleButton(XButtonEvent *event);
  virtual int HandleMotion(XMotionEvent *event);
  virtual int ProcessMessage(OMessage *msg);

private:
  int InternalID;
  int *Position;
};

class RedirectedHScrollFrame : public OXCompositeFrame {
public:
  RedirectedHScrollFrame(const OXWindow *p, int w, int h, int ID, RedirectedHScrollBar **SB);
  virtual ~RedirectedHScrollFrame();

  virtual int HandleKey(XKeyEvent *event);
  virtual int HandleButton(XButtonEvent *event);
  virtual int HandleMotion(XMotionEvent *event);

private:
  RedirectedHScrollBar **ScrollBar;
};


//----------------------------------------------------------------------

class OnyxHScrollBar : public OnyxObject {
public:
  OnyxHScrollBar();
  virtual ~OnyxHScrollBar();

  virtual int  Create();
  virtual void Move(int newX, int newY);
  virtual void Resize(int newWidth, int newHeight);
  virtual void MoveResize(int newX, int newY, int newWidth, int newHeight);
  virtual void Update();
  virtual void ChangeRange(int NewRange);
  virtual void ChangePageSize(int NewPageSize);
  virtual void ChangeRangeAndPageSize(int NewRange, int NewPageSize);
  virtual void SetPosition(int NewPosition);

  int Range;
  int PageSize;
  int Position;

protected:
  RedirectedHScrollFrame *InternalFrame;
  RedirectedHScrollBar *InternalScrollBar;
  int ScrollBarExists;
};


#endif  // __ONYXHSCROLLBAR_H
