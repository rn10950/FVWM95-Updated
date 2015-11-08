/**************************************************************************

    This file is part of xclass.
    Copyright (C) 2004, Hector Peraza.                 

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

#ifndef __OXHELPPOPUP_H
#define __OXHELPPOPUP_H

#include <X11/Xlib.h>

#include <xclass/utils.h>
#include <xclass/OXCompositeFrame.h>
#include <xclass/OXLabel.h>
#include <xclass/OString.h>


class OXHelpDoc;

//----------------------------------------------------------------------

class OXHelpPopup : public OXCompositeFrame {
public:
  OXHelpPopup(const OXWindow *p);
  virtual ~OXHelpPopup();

  virtual int  HandleButton(XButtonEvent *event);
  virtual void DrawBorder();

  void Show(int x, int y, int wa = False);
  void Hide();
  
  void UseShadow(int onoff);
  void Relayout();
  
  OXHelpDoc *GetDoc() const { return _doc; }

protected:
  void SetupShadow(ODimension size);

private:
  OXHelpDoc *_doc;
  OLayoutHints *_ll;
  
  int _useShadow, _waiting;
  Pixmap _pclip, _pmask;
};


#endif  // __OXHELPPOPUP_H
