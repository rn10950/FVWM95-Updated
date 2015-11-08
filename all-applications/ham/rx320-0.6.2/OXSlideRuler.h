/**************************************************************************

    This file is part of rx320, a control program for the Ten-Tec RX320
    receiver. Copyright (C) 2000-2004, Hector Peraza.

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

#ifndef __OXSLIDERULER_H
#define __OXSLIDERULER_H

#include <xclass/utils.h>
#include <xclass/OXFrame.h>
#include <xclass/ODimension.h>
#include <xclass/OXFont.h>
#include <xclass/OGC.h>

#include "ORX320.h"
#include "main.h"


//----------------------------------------------------------------------

class OXSlideRuler : public OXFrame {
public:
  OXSlideRuler(const OXWindow *p, OXMain *m, int w, int h,
               unsigned int options = SUNKEN_FRAME | DOUBLE_BORDER);
  virtual ~OXSlideRuler();
  
  virtual int HandleButton(XButtonEvent *event);
  virtual int HandleMotion(XMotionEvent *event);

  void SetFreq(long freq);
  void SetStep(long fstep);
  
  virtual ODimension GetDefaultSize() const;
  
protected:
  virtual void _DoRedraw();

  void CreatePixmap();
  void ClearPixmap();
  void Draw();
  void DoDraw();
  void DrawTicks();
  void DrawBands();
  void DrawCursor();
  
  OXGC *_gc;
  Pixmap _pix;
  int _pixw, _pixh;
  
  int _bdown, _lastx, _draw_pending;
  OXFont *_sfont;
  long _freq, _tickval;

  OXMain *_rxmain;
};


#endif  // __OXSLIDERULER_H
