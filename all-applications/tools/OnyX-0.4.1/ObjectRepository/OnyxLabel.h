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

#ifndef __ONYXLABEL_H
#define __ONYXLABEL_H

#include <xclass/OXCompositeFrame.h>
#include <xclass/OXLabel.h>

#include "OnyxBase.h"


//----------------------------------------------------------------------

class RedirectedLabel : public OXLabel {
public:
  RedirectedLabel(const OXWindow *p, OString *text, int ID);
  virtual ~RedirectedLabel();

  virtual int HandleKey(XKeyEvent *event);
  virtual int HandleButton(XButtonEvent *event);
  virtual int HandleMotion(XMotionEvent *event);

  void SetupAlignment(int horiz, int vert);

private:
  int InternalID;
};


//----------------------------------------------------------------------

class OnyxLabel : public OnyxObject {
public:
  OnyxLabel();
  virtual ~OnyxLabel();

  virtual int  Create();
  virtual void Move(int newX, int newY);
  virtual void Resize(int newWidth, int newHeight);
  virtual void MoveResize(int newX, int newY, int newWidth, int newHeight);
  virtual void Update();

protected:
  RedirectedLabel *InternalLabel;
  int LabelExists;
};


#endif  // __ONYXLABEL_H
