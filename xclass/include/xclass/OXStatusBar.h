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

#ifndef __OXSTATUSBAR_H
#define __OXSTATUSBAR_H


#include <xclass/utils.h>
#include <xclass/OXClient.h>
#include <xclass/OXCompositeFrame.h>
#include <xclass/OXLabel.h>
#include <xclass/OString.h>


//---------------------------------------------------------------------

class OXStatusLabel : public OXLabel {
public:
  OXStatusLabel(const OXWindow *p, OString *text,
                int width, int hints = LHINTS_LEFT) :
    OXLabel(p, text) { _width = width; _hints = hints; }

  friend class OXStatusBar;

  virtual ODimension GetDefaultSize() const 
    { return ODimension(_width, OXLabel::GetDefaultSize().h); }

protected:
  int _width, _hints;
};

class OXStatusBar : public OXHorizontalFrame {
public:
  OXStatusBar(const OXWindow *p, int w = 10, int h = 10,
              unsigned int options = CHILD_FRAME,
              unsigned long back = _defaultFrameBackground);
  virtual ~OXStatusBar();

  virtual void AddLabel(int width, int hints = LHINTS_LEFT);
  virtual void SetWidth(int num, int width);
  virtual void SetText(int num, OString *text);
  virtual void SetAlignment(int num, int hints);

  //void SetDefaultFont(OXFont *font);
  //void SetFont(int num, OXFont *font);

protected:
  int _nlabels;
};


#endif   // __OXSTATUSBAR_H
