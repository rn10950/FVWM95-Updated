/**************************************************************************

    This file is part of xclass, a Win95-looking GUI toolkit.
    Copyright (C) 1996, 1997 David Barth, Hector Peraza.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

**************************************************************************/

#ifndef __OXBUTTON_H
#define __OXBUTTON_H

#include <X11/Xlib.h>

#include <xclass/utils.h>
#include <xclass/OXFrame.h>
#include <xclass/OXWidget.h>
#include <xclass/ODimension.h>
#include <xclass/OMessage.h>


//--- Button states

#define BUTTON_UP         0
#define BUTTON_DOWN       1
#define BUTTON_ENGAGED    2

//--- Button types

#define BUTTON_NORMAL     0
#define BUTTON_STAYDOWN   1
#define BUTTON_ONOFF      2


class OXGC;

#define OButtonMessage OWidgetMessage


//----------------------------------------------------------------------
// Abstract base class for buttons

class OXButton : public OXFrame, public OXWidget {
protected:
  static const OXGC *_hibckgndGC;
  static int _init;

public:
  OXButton(const OXWindow *p, int ID,
           unsigned int option = RAISED_FRAME | DOUBLE_BORDER);

  virtual int  HandleButton(XButtonEvent *event);
  virtual int  HandleCrossing(XCrossingEvent *event);
  virtual void SetState(int state);
  virtual int  GetState() const { return _state; }
  virtual void SetType(int a) { _type = a; }
  virtual void SetDefault(int onoff = True);
  virtual void DrawBorder();
  virtual void Reconfig();

protected:
  virtual void _GotFocus();
  virtual void _LostFocus();
  virtual void _Enable(int onoff);

  int _tw, _th, _type;
  int _state, _prevstate, _down, _click, _default;
  OXGC *_normGC;
};


#endif  // __OXBUTTON_H
