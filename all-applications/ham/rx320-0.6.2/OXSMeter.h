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

#ifndef __OXSMETER_H
#define __OXSMETER_H

#include <xclass/OXFrame.h>
#include <xclass/OXLabel.h>
#include <xclass/OXFont.h>
#include <xclass/OGC.h>


#define S_HORIZONTAL  1
#define S_VERTICAL    2


class OXSMeter : public OXFrame {
public:
  OXSMeter(const OXWindow *p, int orientation = S_HORIZONTAL);
  virtual ~OXSMeter();
  
  virtual ODimension GetDefaultSize() const;
  
  void SetS(unsigned int sval);
  void SetPeakHold(unsigned int hold);
  void SetPeakAverage(unsigned int avg);
  
  unsigned int GetPeakHold() const { return _pkhold; }
  unsigned int GetPeakAverage() const { return _avg; }

protected:
  virtual void _DoRedraw();

  void _Draw();
  
  int _orien;
  unsigned int _sval, _smax, _avg, _hold, _pkhold;
  OXGC *_clrGC, *_barGC, *_maxGC;
  OXFont *_sfont, *_dBfont;
  
  Pixmap _pix;
  int _pixw, _pixh;
};


#endif  // __OXSMETER_H
