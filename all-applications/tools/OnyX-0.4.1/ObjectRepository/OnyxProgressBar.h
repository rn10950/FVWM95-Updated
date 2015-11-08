/**************************************************************************

    This file is part of OnyX, a visual development environment
    using the xclass toolkit. Copyright (C) 2004 Hector Peraza.

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

#ifndef __ONYXPROGRESSBAR_H
#define __ONYXPROGRESSBAR_H

#include <xclass/OXCompositeFrame.h>
#include <xclass/OXProgressBar.h>

#include "OnyxBase.h"


//----------------------------------------------------------------------

class RedirectedProgressBar : public OXProgressBar {
public:
  RedirectedProgressBar(const OXWindow *p, int w, int h, int ID);

  virtual int HandleButton(XButtonEvent *event);
  virtual int HandleMotion(XMotionEvent *event);
  virtual int HandleKey(XKeyEvent *event);

private:
  int InternalID;
};


//----------------------------------------------------------------------

class OnyxProgressBar : public OnyxObject {
public:
  OnyxProgressBar();
  virtual ~OnyxProgressBar();

  virtual int  Create();
  virtual void Move(int newX, int newY);
  virtual void Resize(int newWidth, int newHeight);
  virtual void MoveResize(int newX, int newY, int newWidth, int newHeight);
  virtual void Update();

  virtual void SetColor(int r, int g, int b);
  virtual void SetColor(const char *color);
  
  int Min;
  int Max;
  int Level;
  int ShowPercent;

protected:
  RedirectedProgressBar *InternalProgressBar;
  int ProgressBarExists;
  unsigned long CurrentColor;
};


#endif  // __ONYXPROGRESSBAR_H
