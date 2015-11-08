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

#ifndef __OXDISPLAYPANEL_H
#define __OXDISPLAYPANEL_H

#include <xclass/OXCompositeFrame.h>
#include <xclass/OXTextEntry.h>
#include <xclass/OXLabel.h>
#include <xclass/OXIcon.h>
#include <xclass/OTimer.h>

#include "OXSMeter.h"


#define VFO_A   1
#define VFO_B   2


class OXDisplayPanel : public OXCompositeFrame {
public:
  OXDisplayPanel(const OXWindow *p);
  virtual ~OXDisplayPanel();

  virtual int HandleTimer(OTimer *t);
  
  void SetFreq(int vfo, long freq);
  void SetMode(int vfo, int mode);
  void SetS(int sval) { _Smeter->SetS(sval); }
  void SetAGC(int agc);
  void SetPBT(int pbt);
  void SetCWO(int cwo, int onoff);
  void SetBW(int bw);
  void PowerOn();
  void Scanning(int onoff);
  void SetMuted(int onoff);
  
  OXSMeter *Smeter() const { return _Smeter; }
  
protected:
  OXHorizontalFrame *hfA, *hfB;
  OXVerticalFrame *vfL;
  OXLabel *_freqA, *_freqB, *_modeA, *_modeB;
  OXLabel *_agc, *_tstep, *_pbt, *_cwo, *_bw, *_pwr;
  OXIcon *_muted;
  OXSMeter *_Smeter;
  OTimer *_pwrt, *_scant;
  int _scanstat;
};


#endif  // __OXDISPLAYPANEL_H
