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

#ifndef __MAIN_H
#define __MAIN_H

#include <xclass/OXClient.h>
#include <xclass/OXMainFrame.h>
#include <xclass/OXHelpWindow.h>
#include <xclass/OXTextButton.h>
#include <xclass/OXPictureButton.h>
#include <xclass/OXMenu.h>
#include <xclass/OXSlider.h>
#include <xclass/utils.h>

struct SBand {
  char *name;
  long lofreq, hifreq;
};
    
struct Svfo {
  long freq, step;
  int mode, agc, filter, cwbfo, pbt;
};

struct SScanSettings {
  long range;
  int  ymode, style;
};
    
class ORX320;
class OFreqRecord;
class OXFreqDB;
class OXDisplayPanel;
class OXScanWindow;
class OXTuningKnob;
class OXSlideRuler;


//---------------------------------------------------------------------

class OXMain : public OXMainFrame {
protected:
  static SBand default_bands[];

public:
  OXMain(const OXWindow *p, int w, int h);
  virtual ~OXMain();

  virtual int HandleButton(XButtonEvent *event);
  virtual int HandleKey(XKeyEvent *event);
  virtual int ProcessMessage(OMessage *msg);
  virtual int CloseWindow();

  void GrabKeys();
  void ReadIniFile();
  void SaveIniFile();
  void OpenFreqDB();
  void SwapVFO();
  void CopyVFO();
  void SetStep(int step);
  void TuneTo(long freq, int knob_step = 0);
  void TuneTo(OFreqRecord *frec);
  void TuneUp(int steps, int move_knob = True);
  void TuneDown(int steps, int move_knob = True);
  void VolumeUp(int steps, int channel);
  void VolumeDown(int steps, int channel);
  void ToggleMuted();
  void ShowHelp();
  void ShowAbout(const OXWindow *t = NULL);

  void UpdateDisplay();
  void UpdateSliders();

  ORX320 *GetRX() const { return _rx; }
  
  Svfo GetVFOA() const { return _vfoA; }
  Svfo GetVFOB() const { return _vfoB; }

  void AddFreqDBwindow(OXFreqDB *fdb);
  void RemoveFreqDBwindow(OXFreqDB *fdb);
  
  void OpenScan();
  void CloseScan();
  void StartScan();
  void StopScan();
  void ScanStep(int sval);
  
  SBand *GetBandInfo() const { return _bands; }

protected:
  OXPopupMenu *_menu;
  OXDisplayPanel *_dpanel;
  OXSlideRuler *_ruler;
  OXTextButton *_reset, *_mute;
  OXPictureButton *_up, *_up2, *_down, *_down2;
  OXTextButton *_agc[3], *_mode[4], *_vfo[2], *_step[6];
  OXVSlider *_spkvol, *_linevol, *_bw, *_pbt, *_cwo;
  OXTuningKnob *_knob;

  ORX320 *_rx;
  int _muted, _typing, _scanning, _spoll, _mute_on_exit;
  long _tfreq;
  Svfo _vfoA, _vfoB;
  SScanSettings _scanSettings;
  SBand *_bands;
  char *_device;

  OXFreqDB *_freqWindow;
  OXScanWindow *_scanWindow;
  OXHelpWindow *_helpWindow;
};


#endif  // __MAIN_H
