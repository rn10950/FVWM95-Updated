/**************************************************************************

    This file is part of xclauncher.
    Copyright (C) 1998-2002 Hector Peraza.

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

#ifndef __OXLAUNCHBUTTON_H
#define __OXLAUNCHBUTTON_H

#include <xclass/OXButton.h>
#include <xclass/OXFont.h>
#include <xclass/OGC.h>


//----------------------------------------------------------------------

class OXLaunchButton : public OXButton {
protected:
  static OXFont *_defaultFont;
  OXFont *_font;
  OXGC *_gc;
  unsigned long _hilitePixel, _shadowPixel;

public:
  OXLaunchButton(const OXWindow *p, const OPicture *pic, OHotString *text,
  		 int ID, unsigned int option = RAISED_FRAME);
  virtual ~OXLaunchButton();

  virtual ODimension GetDefaultSize() const;
  virtual int HandleButton(XButtonEvent *event);
  virtual int HandleCrossing(XCrossingEvent *event);

  void SetLabel(OHotString *label);
  void SetLabelColor(unsigned long color);
  void SetNormalPic(const OPicture *pic);
  void SetOnPic(const OPicture *pic);
  void SetDisabledPic(const OPicture *pic);
  void ShowLabel(int onoff);

protected:
  void SetUpSize();
  virtual void _DoRedraw();

  OHotString *_label;
  const OPicture *_normalPic;
  const OPicture *_onPic;
  const OPicture *_disabledPic;

  int _showLabel, _mouseOn;
  unsigned long _labelColor;
};


#endif  // __OXLAUNCHBUTTON_H
