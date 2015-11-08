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

#ifndef __OXGROUPFRAME_H
#define __OXGROUPFRAME_H

#include <X11/Xlib.h>

#include <xclass/utils.h>
#include <xclass/OXCompositeFrame.h>
#include <xclass/ODimension.h>
#include <xclass/OString.h>


class OXFont;
class OXGC;

//----------------------------------------------------------------------

class OXGroupFrame : public OXCompositeFrame {
protected:
  static const OXGC *_defaultGC;
  static const OXFont *_defaultFont;
  static int _init;

public:
  OXGroupFrame(const OXWindow *p, OString *title,
               unsigned int options = VERTICAL_FRAME,
               unsigned long back = _defaultFrameBackground);
  virtual ~OXGroupFrame();

  void OldInsets(int onoff = True);

  virtual void DrawBorder();

protected:
  virtual void _Enable(int onoff);
  virtual void _ComputeInsets();

  OString *_text;
  const OXFont *_font;
  OXGC *_normGC;
  int _old_insets;
};


#endif  // __OXGROUPFRAME_H
