/**************************************************************************
 
    This file is part of OXMdi, an extension to the xclass toolkit.
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

#ifndef __OXMDIMENU_H
#define __OXMDIMENU_H

#include <xclass/OXMenu.h>
#include <xclass/OXCompositeFrame.h>


class OXMdiMainFrame;
class OXMdiTitleIcon;
class OXMdiButtons;


//----------------------------------------------------------------------
// This is to be used as the MDI application's menu bar.

class OXMdiMenuBar : public OXCompositeFrame {
public:
  OXMdiMenuBar(const OXWindow *p, int w = 1, int h = 20);
  virtual ~OXMdiMenuBar();

  void AddPopup(OHotString *s, OXPopupMenu *menu, OLayoutHints *l);

  friend class OXMdiMainFrame;

protected:
  void AddFrames(OXMdiTitleIcon *icon, OXMdiButtons *buttons);
  void RemoveFrames(OXMdiTitleIcon *icon, OXMdiButtons *buttons);

  OXCompositeFrame *_left, *_right;
  OXMenuBar *_bar;
  OLayoutHints *_lHint, *_leftHint, *_rightHint, *_barHint;
};


#endif  // __OXMDIMENU_H
