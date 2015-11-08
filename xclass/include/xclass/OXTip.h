/**************************************************************************

    This file is part of xclass.
    Copyright (C) 1996, 1997 David Barth, Hector Peraza.

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

#ifndef __OXTIP_H
#define __OXTIP_H

#include <X11/Xlib.h>

#include <xclass/utils.h>
#include <xclass/OXCompositeFrame.h>
#include <xclass/OXLabel.h>
#include <xclass/OString.h>


//----------------------------------------------------------------------

class OXTip : public OXCompositeFrame {
public:
  OXTip(const OXWindow *p, OString *text);
  virtual ~OXTip();

  virtual void DrawBorder();
  virtual void Reconfig();

  void SetText(OString *text);
  void Show(int x, int y);
  void Hide();
  
  void UseShadow(int onoff);

protected:
  void SetupShadow();

private:
  OXLabel *_label;
  OLayoutHints *_ll;
  
  unsigned long _bg, _fg;
  int _useShadow;
  Pixmap _pclip, _pmask;
};


#endif  // __OXTIP_H
