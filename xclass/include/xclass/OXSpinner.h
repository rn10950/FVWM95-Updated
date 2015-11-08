/**************************************************************************

    This file is part of xclass.
    Copyright (C) 1998-2002 Mike McDonald, Hector Peraza.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

**************************************************************************/

#ifndef __OXSPINNER_H
#define __OXSPINNER_H

#include <xclass/OXCompositeFrame.h>
#include <xclass/OXWidget.h>
#include <xclass/OXTextEntry.h>
#include <xclass/OXButton.h>
#include <xclass/OXScrollBar.h>
#include <xclass/OTimer.h>


//----------------------------------------------------------------------

class OXDoubleScroller : public OXCompositeFrame, public OXWidget {
public:
  OXDoubleScroller(const OXWindow *p, int w, int h);
  virtual ~OXDoubleScroller();

  virtual int HandleButton(XButtonEvent *event);

protected:
  OXScrollBarElt *_up, *_down;
  OLayoutHints *_l1, *_l2;
};

class OXSpinner : public OXCompositeFrame, public OXWidget {
public:
  OXSpinner(const OXWindow *p, char *name, int id);
  virtual ~OXSpinner();

  void SetValue(float value) { _value = value; _SetValue(); }
  float GetValue();
  void SetRange(float min, float max, float step);
  void SetPrec(unsigned short per) { _prec = per; _SetValue(); }
  void SetRollOver(bool roll) { _rollOver = roll; }
  virtual int ProcessMessage(OMessage *msg);
  virtual int HandleTimer(OTimer *timer);
  void SetEditable(bool ed);
  void SetPercent(bool perc);

protected:
  void _SetValue(bool sendmsg = False);
  void _GetValue();
  void _Up();
  void _Down();

  OXTextEntry *_te;
  OXDoubleScroller *_vf;
  OLayoutHints *_l1, *_l2;
  OTimer *_tmr;

  bool _rollOver, _editable, _percent;

  float _min, _max, _step, _value;
  unsigned short _prec;
  int _dir;
};


#endif  // __OXSPINNER_H
