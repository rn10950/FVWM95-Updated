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

#ifndef __OXRADIOBUTTON_H
#define __OXRADIOBUTTON_H

#include <X11/Xlib.h>

#include <xclass/utils.h>
#include <xclass/OXTextButton.h>


//----------------------------------------------------------------------

class OXRadioButton : public OXTextButton {
protected:
  static const OPicture *_on, *_off, *_ond, *_offd;
  static int _init;

public:
  OXRadioButton(const OXWindow *p, OString *s, int ID,
                unsigned int option = 0);

  virtual ODimension GetDefaultSize() const;

  virtual int  HandleKey(XKeyEvent *event);
  virtual int  HandleButton(XButtonEvent *event);
  virtual int  HandleCrossing(XCrossingEvent *event);
  virtual void SetState(int state) { _SetState(_prevstate = state); }

protected:
  virtual void _DoRedraw();
  void _SetState(int state);
};


#endif  // __OXRADIOBUTTON_H
