/**************************************************************************
 
    This file is part of OXMdi an extension to the xclass toolkit.
    Copyright (C) 1998-2002 by Harald Radke, Hector Peraza.
 
    This application is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
 
    This application is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.
 
    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 
**************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include <xclass/OXMdi.h>
#include <xclass/OXMdiMenu.h>


//----------------------------------------------------------------------

// This is to be used as the application's menu bar (MDI menu bar).

OXMdiMenuBar::OXMdiMenuBar(const OXWindow *p, int w, int h) :
  OXCompositeFrame(p, w, h, HORIZONTAL_FRAME) {

  _lHint = new OLayoutHints(LHINTS_NORMAL);
  _leftHint = new OLayoutHints(LHINTS_LEFT | LHINTS_CENTER_Y, 1, 1, 1, 1);
  _barHint = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_CENTER_Y, 1, 1, 1, 1);
  _rightHint = new OLayoutHints(LHINTS_RIGHT | LHINTS_CENTER_Y, 1, 2, 1, 1);

  _left = new OXCompositeFrame(this, 10, 10, HORIZONTAL_FRAME);
  _bar = new OXMenuBar(this, 1, 20, HORIZONTAL_FRAME);   
  _right = new OXCompositeFrame(this, 10, 10, HORIZONTAL_FRAME);

  AddFrame(_left, _leftHint);
  AddFrame(_bar, _barHint);
  AddFrame(_right, _rightHint);
}

OXMdiMenuBar::~OXMdiMenuBar() {
  delete _lHint;
  delete _leftHint;
  delete _rightHint;
  delete _barHint;
}

void OXMdiMenuBar::AddPopup(OHotString *s, OXPopupMenu *menu,
                            OLayoutHints *l) {
  _bar->AddPopup(s, menu, l);
  //Layout();
}

// this is called from OXMdiMainFrame on Maximize()

void OXMdiMenuBar::AddFrames(OXMdiTitleIcon *icon, OXMdiButtons *buttons) {
  icon->ReparentWindow(_left);
  buttons->ReparentWindow(_right);
  _left->AddFrame(icon, _lHint);
  _right->AddFrame(buttons, _lHint);
}

// this is called from OXMdiMainFrame on Restore()

void OXMdiMenuBar::RemoveFrames(OXMdiTitleIcon *icon, OXMdiButtons *buttons) {
  _left->RemoveFrame(icon);
  _right->RemoveFrame(buttons);
}
