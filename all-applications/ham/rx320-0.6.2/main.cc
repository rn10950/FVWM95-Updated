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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <vector>

#include <X11/keysym.h>

#include <xclass/utils.h>
#include <xclass/version.h>
#include <xclass/OXClient.h>
#include <xclass/OXTransientFrame.h>
#include <xclass/OXTextButton.h>
#include <xclass/OXPictureButton.h>
#include <xclass/OXAboutDialog.h>
#include <xclass/OXMenu.h>
#include <xclass/OXIcon.h>
#include <xclass/OXMsgBox.h>
#include <xclass/OXSlider.h>
#include <xclass/OString.h>
#include <xclass/OIniFile.h>
#include <xclass/OResourcePool.h>

#include "ORX320.h"
#include "OFreqRecord.h"
#include "OXFreqDB.h"
#include "OXDisplayPanel.h"
#include "OXSlideRuler.h"
#include "OXTuningKnob.h"
#include "OXScan.h"
#include "OXDialogs.h"

#include "main.h"
#include "versiondef.h"

#include "up.xpm"
#include "up2.xpm"
#include "down.xpm"
#include "down2.xpm"

#include "ttrx320.xpm"

#define SPOLL  100

#if 0
int filtno[] = {
  32, 31, 30, 29, 28, 27, 26, 25,
  24, 23, 22, 21, 20, 19, 18, 17,
  16, 15, 14, 13, 12, 11, 10,  9,
   8,  7,  6,  5,  4,  3,  2,  1,
   0, 33
};
#else
int filtno[] = {
  33,  0,  1,  2,  3,  4,  5,  6,
   7,  8,  9, 10, 11, 12, 13, 14,
  15, 16, 17, 18, 19, 20, 21, 22,
  23, 24, 25, 26, 27, 28, 29, 30,
  31, 32
};
#endif

SBand OXMain::default_bands[] = {
  { "AMBC",   530000,  1700000 },
  { "160m",  1800000,  2000000 },
  { "120m",  2300000,  2495000 },
  {  "90m",  3200000,  3400000 },
  {  "80m",  3500000,  4000000 },
  {  "75m",  3900000,  4000000 },
  {  "60m",  4750000,  5600000 },
  {  "49m",  5900000,  6200000 },
  {  "41m",  7100000,  7300000 },
  {  "40m",  7000000,  7350000 },
  {  "31m",  9400000,  9900000 },
  {  "30m", 10100000, 10150000 },
  {  "25m", 11600000, 12100000 },
  {  "21m", 13570000, 13870000 },
  {  "20m", 14000000, 14350000 },
  {  "19m", 15100000, 15800000 },
  {  "17m", 18068000, 18168000 },
  {  "16m", 17480000, 17900000 },
  {  "15m", 18900000, 19020000 },
  {  "15m", 21000000, 21450000 },
  {  "13m", 21450000, 21850000 },
  {  "12m", 24890000, 24990000 },
  {  "11m", 25670000, 26100000 },
  {  "10m", 28000000, 29700000 },
  {  0,     -1,       -1       } 
};


#define M_FREQDB   100
#define M_SCAN     101
#define M_CONFIG   102
#define M_ABOUT    103
#define M_HELP     104
#define M_EXIT     105


//---------------------------------------------------------------------

OXMain::OXMain(const OXWindow *p, int w, int h) :
  OXMainFrame(p, w, h) {
    int i, width;

    OXHorizontalFrame *hf = new OXHorizontalFrame(this, 1, 1);
    AddFrame(hf, new OLayoutHints(LHINTS_EXPAND_X | LHINTS_TOP, 0, 0, 1, 0));

    OXIcon *tt = new OXIcon(hf, _client->GetPicture("ttrx320.xpm", ttrx320_xpm));
    hf->AddFrame(tt, new OLayoutHints(LHINTS_LEFT | LHINTS_CENTER_Y, 20, 20, 0, 0));

    //----------------- Popup menu

    _menu = new OXPopupMenu(_client->GetRoot());
    _menu->AddEntry(new OHotString("&Frequency database...\tAlt+D"), M_FREQDB);
    _menu->AddEntry(new OHotString("&Scan..."), M_SCAN);
    _menu->AddEntry(new OHotString("&Configure..."), M_CONFIG);
    _menu->AddEntry(new OHotString("&About..."), M_ABOUT);
    _menu->AddEntry(new OHotString("&Help...\tF1"), M_HELP);
    _menu->AddSeparator();
    _menu->AddEntry(new OHotString("E&xit\tAlt+X"), M_EXIT);
    _menu->Associate(this);

    //----------------- Display panel and slide ruler

    OXVerticalFrame *vef = new OXVerticalFrame(hf);
    hf->AddFrame(vef, new OLayoutHints(LHINTS_EXPAND_X | LHINTS_TOP));

    _dpanel = new OXDisplayPanel(vef);
    vef->AddFrame(_dpanel, new OLayoutHints(LHINTS_EXPAND_X | LHINTS_TOP));

    _ruler = new OXSlideRuler(vef, this, 10, 10);
    vef->AddFrame(_ruler, new OLayoutHints(LHINTS_EXPAND_X | LHINTS_TOP, 0, 0, 1, 0));

    hf = new OXHorizontalFrame(this, 1, 1);
    AddFrame(hf, new OLayoutHints(LHINTS_EXPAND_X | LHINTS_TOP));

    //----------------- AGC

    OLayoutHints *lv = new OLayoutHints(LHINTS_LEFT | LHINTS_EXPAND_Y, 5, 5, 5, 5);

    vef = new OXVerticalFrame(hf, 10, 10, FIXED_WIDTH);
    hf->AddFrame(vef, lv);

    OXLabel *_al = new OXLabel(vef, new OString("AGC"));
    _al->SetFont(_client->GetFont("Helvetica -12 bold"));
    _al->Set3DStyle(LABEL_SUNKEN);
    _agc[0] = new OXTextButton(vef, new OHotString("&Slow"), 801);
    _agc[1] = new OXTextButton(vef, new OHotString("&Medium"), 802);
    _agc[2] = new OXTextButton(vef, new OHotString("&Fast"), 803);

    OLayoutHints *lh1 = new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X);
    OLayoutHints *lh2 = new OLayoutHints(LHINTS_BOTTOM | LHINTS_EXPAND_X);

    vef->AddFrame(_al, lh1);
    for (i = 0; i < 3; ++i) {
      vef->AddFrame(_agc[i], lh1);
      _agc[i]->SetFont(_client->GetFont("Lucida -10"));
      _agc[i]->TakeFocus(False);
//      _agc[i]->SetType(BUTTON_STAYDOWN);
      _agc[i]->Associate(this);
    }

    _al = new OXLabel(vef, new OString("VFO"));
    _al->SetFont(_client->GetFont("Helvetica -12 bold"));
    _al->Set3DStyle(LABEL_SUNKEN);

    _vfo[0] = new OXTextButton(vef, new OHotString("A / B"), 101);
    _vfo[1] = new OXTextButton(vef, new OHotString("A -> B"), 102);

    for (i = 0; i < 2; ++i) {
      vef->AddFrame(_vfo[1-i], lh2);
      _vfo[i]->SetFont(_client->GetFont("Lucida -10"));
      _vfo[i]->TakeFocus(False);
      _vfo[i]->Associate(this);
    }
    vef->AddFrame(_al, lh2);

    width = _agc[1]->GetDefaultWidth();
    vef->Resize(width + 10, vef->GetDefaultHeight());

    //----------------- Mode

    vef = new OXVerticalFrame(hf, 10, 10, FIXED_WIDTH);
    hf->AddFrame(vef, lv);

    _al = new OXLabel(vef, new OString("Mode"));
    _al->SetFont(_client->GetFont("Helvetica -12 bold"));
    _al->Set3DStyle(LABEL_SUNKEN);

    _mode[0] = new OXTextButton(vef, new OHotString("&AM"), 811);
    _mode[1] = new OXTextButton(vef, new OHotString("&LSB"), 812);
    _mode[2] = new OXTextButton(vef, new OHotString("&USB"), 813);
    _mode[3] = new OXTextButton(vef, new OHotString("&CW"), 814);

    vef->AddFrame(_al, lh1);
    for (i = 0; i < 4; ++i) {
      vef->AddFrame(_mode[i], lh1);
      _mode[i]->SetFont(_client->GetFont("Lucida -10"));
//      _mode[i]->SetType(BUTTON_STAYDOWN);
      _mode[i]->TakeFocus(False);
      _mode[i]->Associate(this);
    }

    _mute = new OXTextButton(vef, new OHotString("Mu&te"), 901);
    vef->AddFrame(_mute, new OLayoutHints(LHINTS_BOTTOM | LHINTS_EXPAND_X));
    _mute->SetFont(_client->GetFont("Lucida -10"));
//    _mute->SetType(BUTTON_ONOFF);
    _mute->TakeFocus(False);
    _mute->Associate(this);
    _muted = False;

    _mute_on_exit = False;

    vef->Resize(width + 10, vef->GetDefaultHeight());

    //----------------- Tuning steps

    vef = new OXVerticalFrame(hf, 10, 10, FIXED_WIDTH);
    hf->AddFrame(vef, lv);

    _al = new OXLabel(vef, new OString("Step"));
    _al->SetFont(_client->GetFont("Helvetica -12 bold"));
    _al->Set3DStyle(LABEL_SUNKEN);

    _step[0] = new OXTextButton(vef, new OHotString("1 Hz"),   851);
    _step[1] = new OXTextButton(vef, new OHotString("10 Hz"),  852);
    _step[2] = new OXTextButton(vef, new OHotString("100 Hz"), 853);
    _step[3] = new OXTextButton(vef, new OHotString("1 kHz"),  854);
    _step[4] = new OXTextButton(vef, new OHotString("5 kHz"),  855);
    _step[5] = new OXTextButton(vef, new OHotString("10 kHz"), 856);

    vef->AddFrame(_al, lh1);
    for (i = 0; i < 6; ++i) {
      vef->AddFrame(_step[i], lh1);
      _step[i]->SetFont(_client->GetFont("Lucida -10"));
//      _step[i]->SetType(BUTTON_STAYDOWN);
      _step[i]->TakeFocus(False);
      _step[i]->Associate(this);
    }

    vef->Resize(width + 10, vef->GetDefaultHeight());

    //----------------- tuning knob

    _knob = new OXTuningKnob(hf);
    _knob->Associate(this);
    hf->AddFrame(_knob, new OLayoutHints(LHINTS_CENTER, 15, 15, 5, 5));

    vef = new OXVerticalFrame(hf, 10, 10);
    _up2 = new OXPictureButton(vef, _client->GetPicture("up2.xpm", up2_xpm), 401);
    _up2->Associate(this);
    _up = new OXPictureButton(vef, _client->GetPicture("up.xpm", up_xpm), 402);
    _up->Associate(this);
    _down = new OXPictureButton(vef, _client->GetPicture("down.xpm", down_xpm), 403);
    _down->Associate(this);
    _down2 = new OXPictureButton(vef, _client->GetPicture("down2.xpm", down2_xpm), 404);
    _down2->Associate(this);

    vef->AddFrame(_up2, lh1);
    vef->AddFrame(_up, lh1);
    vef->AddFrame(_down, new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 0, 0, 5, 0));
    vef->AddFrame(_down2, lh1);

    hf->AddFrame(vef, new OLayoutHints(LHINTS_LEFT | LHINTS_CENTER_Y, 5, 5, 10, 5));

    //----------------- sliders

    OLayoutHints *ls1 = new OLayoutHints(LHINTS_LEFT | LHINTS_BOTTOM, 0, 1, 1, 1);
    OLayoutHints *ls2 = new OLayoutHints(LHINTS_LEFT | LHINTS_BOTTOM, 0, 5, 1, 1);

    OXVerticalFrame *vef1 = new OXVerticalFrame(hf, 1, 1, FIXED_WIDTH);
    _al = new OXLabel(vef1, new OString("Vol"));
    _al->SetFont(_client->GetFont("Helvetica -12 bold"));
    _al->Set3DStyle(LABEL_SUNKEN);
    _spkvol = new OXVSlider(vef1, 100, SLIDER_2 | SCALE_NONE, 301);
    _spkvol->Associate(this);
    _spkvol->SetRange(0, 63);
    vef1->AddFrame(_spkvol, lh1);
    vef1->AddFrame(_al, lh1);
    hf->AddFrame(vef1, ls1);
    width = _al->GetDefaultWidth();

    OXVerticalFrame *vef2 = new OXVerticalFrame(hf, 1, 1, FIXED_WIDTH);
    _al = new OXLabel(vef2, new OString("Line"));
    _al->SetFont(_client->GetFont("Helvetica -12 bold"));
    _al->Set3DStyle(LABEL_SUNKEN);
    _linevol = new OXVSlider(vef2, 100, SLIDER_2 | SCALE_NONE, 302);
    _linevol->Associate(this);
    _linevol->SetRange(0, 63);
    vef2->AddFrame(_linevol, lh1);
    vef2->AddFrame(_al, lh1);
    hf->AddFrame(vef2, ls1);
    width = max(width, _al->GetDefaultWidth());

    OXVerticalFrame *vef3 = new OXVerticalFrame(hf, 1, 1, FIXED_WIDTH);
    _al = new OXLabel(vef3, new OString("BW"));
    _al->SetFont(_client->GetFont("Helvetica -12 bold"));
    _al->Set3DStyle(LABEL_SUNKEN);
    _bw = new OXVSlider(vef3, 100, SLIDER_2 | SCALE_NONE, 303);
    _bw->Associate(this);
    _bw->SetRange(0, 33);
    vef3->AddFrame(_bw, lh1);
    vef3->AddFrame(_al, lh1);
    hf->AddFrame(vef3, ls1);
    width = max(width, _al->GetDefaultWidth());

    OXVerticalFrame *vef4 = new OXVerticalFrame(hf, 1, 1, FIXED_WIDTH);
    _al = new OXLabel(vef4, new OString("PBT"));
    _al->SetFont(_client->GetFont("Helvetica -12 bold"));
    _al->Set3DStyle(LABEL_SUNKEN);
    _pbt = new OXVSlider(vef4, 100, SLIDER_2 | SCALE_NONE, 304);
    _pbt->Associate(this);
    _pbt->SetRange(-20, +20);
    _pbt->SetPosition(0);
    vef4->AddFrame(_pbt, lh1);
    vef4->AddFrame(_al, lh1);
    hf->AddFrame(vef4, ls1);
    width = max(width, _al->GetDefaultWidth());

    OXVerticalFrame *vef5 = new OXVerticalFrame(hf, 1, 1, FIXED_WIDTH);
    _al = new OXLabel(vef5, new OString("CWO"));
    _al->SetFont(_client->GetFont("Helvetica -12 bold"));
    _al->Set3DStyle(LABEL_SUNKEN);
    _cwo = new OXVSlider(vef5, 100, SLIDER_2 | SCALE_NONE, 305);
    _cwo->Associate(this);
    _cwo->SetRange(-20, +20);
    _cwo->SetPosition(0);
    vef5->AddFrame(_cwo, lh1);
    vef5->AddFrame(_al, lh1);
    hf->AddFrame(vef5, ls2);
    width = max(width, _al->GetDefaultWidth());

    // ensure same widths

    vef1->Resize(width, vef1->GetDefaultHeight());
    vef2->Resize(width, vef2->GetDefaultHeight());
    vef3->Resize(width, vef3->GetDefaultHeight());
    vef4->Resize(width, vef4->GetDefaultHeight());
    vef5->Resize(width, vef5->GetDefaultHeight());

    //----------------- other controls...

//    _reset = new OXTextButton(this, new OHotString("&Reset"), 100);
//    _reset->Associate(this);
//    AddFrame(_reset, new OLayoutHints(LHINTS_BOTTOM | LHINTS_EXPAND_X));

    //----------------- receiver

    _device = StrDup("/dev/ttyS1");

    _mute_on_exit = False;

    _rx = new ORX320(_client);
    _rx->Mute(True);
    _rx->CompressEvents(True);
    _rx->Associate(this);

    _vfoA.freq = _vfoB.freq = _rx->GetFrequency();
    _vfoA.step = _vfoB.step = 1000;
    _vfoA.mode = _vfoB.mode = _rx->GetMode();
    _vfoA.agc = _vfoB.agc = _rx->GetAGC();
    _vfoA.filter = _vfoB.filter = _rx->GetFilter()->filter;
    _vfoA.cwbfo = _vfoB.cwbfo = 1000;
    _vfoA.pbt = _vfoB.pbt = 0;

    _spoll = SPOLL;

    _freqWindow = 0;
    _helpWindow = 0;
    _scanWindow = 0;

    _scanning = False;

    _scanSettings.range = 3000;
    _scanSettings.ymode = SCOPE_LOG;
    _scanSettings.style = SCOPE_BARS;

    _bands = default_bands;

    ReadIniFile();

    i = _rx->SetSerial(_device);
    if (i < 0) {
      char tmp[1024];

      OString stitle("Error");
      sprintf(tmp, "Serial port error:\n%s", _rx->GetLastError());
      new OXMsgBox(_client->GetRoot(), this, &stitle, new OString(tmp),
                   MB_ICONSTOP, ID_OK);
    }

    _rx->SetFrequency(_vfoA.freq);
    _rx->SetMode(_vfoA.mode);
    _rx->SetAGC(_vfoA.agc);
    _rx->SetFilter(_vfoA.filter);
    _rx->SetPBT(_vfoA.pbt);
    _rx->SetCWO(_vfoA.cwbfo);

#if 0
    switch (_rx->GetMode()) {
      case RX320_AM:  _mode[0]->SetState(BUTTON_DOWN); break;  // BUTTON_ENGAGED
      case RX320_LSB: _mode[1]->SetState(BUTTON_DOWN); break;
      case RX320_USB: _mode[2]->SetState(BUTTON_DOWN); break;
      case RX320_CW:  _mode[3]->SetState(BUTTON_DOWN); break;
    }

    switch (_rx->GetAGC()) {
      case RX320_AGC_SLOW:   _agc[0]->SetState(BUTTON_DOWN); break;
      case RX320_AGC_MEDIUM: _agc[1]->SetState(BUTTON_DOWN); break;
      case RX320_AGC_FAST:   _agc[2]->SetState(BUTTON_DOWN); break;
    }
#endif

    UpdateDisplay();
    UpdateSliders();

    _rx->Mute(False);
    _rx->RequestSignal(_spoll);

    _typing = False;

    GrabKeys();
    MapSubwindows();
    Resize(GetDefaultSize());

    SetWindowName("Ten-Tec RX-320");
    SetClassHints("XCLASS", "RX320");

    SetWMSizeHints(_w, _h, _w * 4 / 3, _h, 1, 1);

    _dpanel->SetMuted(False);
    _dpanel->PowerOn();

#if 0
    XGrabButton(GetDisplay(), Button4, AnyModifier, _id, False,
                ButtonPressMask | ButtonReleaseMask,
                GrabModeAsync, GrabModeAsync, None, None);

    XGrabButton(GetDisplay(), Button5, AnyModifier, _id, False,
                ButtonPressMask | ButtonReleaseMask,
                GrabModeAsync, GrabModeAsync, None, None);
#else
    AddInput(ButtonPressMask | ButtonReleaseMask);
#endif
}

OXMain::~OXMain() {
  delete _rx;
  delete[] _device;
  delete _menu;
}

int OXMain::CloseWindow() {
  SaveIniFile();
  while (_freqWindow) {
    if (!_freqWindow->CloseWindow()) return False;
  }
  if (_helpWindow) _helpWindow->CloseWindow();
  if (_scanWindow) _scanWindow->CloseWindow();
  if (_mute_on_exit) _rx->Mute(True);
  _rx->CompressEvents(False);  // flushes output (ensure the receiver is muted)
  return OXMainFrame::CloseWindow();
}

void OXMain::ReadIniFile() {
  char *inipath, line[1024], arg[256];

  inipath = GetResourcePool()->FindIniFile("rx320rc", INI_READ);
  if (!inipath) return;

  OIniFile ini(inipath, INI_READ);

  while (ini.GetNext(line)) {

    if (strcasecmp(line, "defaults") == 0) {
      if (ini.GetItem("device", arg)) {
        delete[] _device;
        _device = StrDup(arg);
      }
      if (!ini.GetBool("show S meter", true)) {
      }
      if (ini.GetItem("speaker volume", arg)) {
        _rx->SetVolume(RX320_SPEAKER, 63 - atoi(arg));
      }
      if (ini.GetItem("line out volume", arg)) {
        _rx->SetVolume(RX320_LINE, 63 - atoi(arg));
      }
      _mute_on_exit = ini.GetBool("mute on exit", false);

    } else if (strcasecmp(line, "vfo A") == 0) {
      if (ini.GetItem("frequency", arg)) {
        _vfoA.freq = atol(arg);
      }
      if (ini.GetItem("step", arg)) {
        _vfoA.step = atol(arg);
      }
      if (ini.GetItem("mode", arg)) {
        if (strcmp(arg, "USB") == 0) {
          _vfoA.mode = RX320_USB;
        } else if (strcmp(arg, "LSB") == 0) {
          _vfoA.mode = RX320_LSB;
        } else if (strcmp(arg, "CW") == 0) {
          _vfoA.mode = RX320_CW;
        } else {
          _vfoA.mode = RX320_AM;
        }
      }
      if (ini.GetItem("agc", arg)) {
        if (strcmp(arg, "slow") == 0) {
          _vfoA.agc = RX320_AGC_SLOW;
        } else if (strcmp(arg, "fast") == 0) {
          _vfoA.agc = RX320_AGC_FAST;
        } else {
          _vfoA.agc = RX320_AGC_MEDIUM;
        }
      }
      if (ini.GetItem("filter", arg)) {
        _vfoA.filter = atoi(arg);
      }
      if (ini.GetItem("cw offset", arg)) {
        _vfoA.cwbfo = atoi(arg);
      }
      if (ini.GetItem("pbt offset", arg)) {
        _vfoA.pbt = atoi(arg);
      }

    } else if (strcasecmp(line, "vfo B") == 0) {
      if (ini.GetItem("frequency", arg)) {
        _vfoB.freq = atol(arg);
      }
      if (ini.GetItem("step", arg)) {
        _vfoB.step = atol(arg);
      }
      if (ini.GetItem("mode", arg)) {
        if (strcmp(arg, "USB") == 0) {
          _vfoB.mode = RX320_USB;
        } else if (strcmp(arg, "LSB") == 0) {
          _vfoB.mode = RX320_LSB;
        } else if (strcmp(arg, "CW") == 0) {
          _vfoB.mode = RX320_CW;
        } else {
          _vfoB.mode = RX320_AM;
        }
      }
      if (ini.GetItem("agc", arg)) {
        if (strcmp(arg, "slow") == 0) {
          _vfoB.agc = RX320_AGC_SLOW;
        } else if (strcmp(arg, "fast") == 0) {
          _vfoB.agc = RX320_AGC_FAST;
        } else {
          _vfoB.agc = RX320_AGC_MEDIUM;
        }
      }
      if (ini.GetItem("filter", arg)) {
        _vfoB.filter = atoi(arg);
      }
      if (ini.GetItem("cw offset", arg)) {
        _vfoB.cwbfo = atoi(arg);
      }
      if (ini.GetItem("pbt offset", arg)) {
        _vfoB.pbt = atoi(arg);
      }

    } else if (strcasecmp(line, "S meter") == 0) {
      if (ini.GetItem("peak hold", arg)) {
        _dpanel->Smeter()->SetPeakHold(atoi(arg));
      }
      if (ini.GetItem("peak average", arg)) {
        _dpanel->Smeter()->SetPeakAverage(atoi(arg));
      }
      if (ini.GetItem("sample interval", arg)) {
        _spoll = atoi(arg);
        if (_spoll < 20) _spoll = 20;
        if (_spoll > 10000) _spoll = 10000;
      }

    } else if (strcasecmp(line, "scan") == 0) {
      if (ini.GetItem("range", arg)) {
        _scanSettings.range = atoi(arg);
        if (_scanSettings.range < 3000)
          _scanSettings.range = 3000;
        else if (_scanSettings.range > 1500000)
          _scanSettings.range = 1500000;
      }
      if (ini.GetItem("mode", arg)) {
        if (strcasecmp(arg, "lin") == 0)
          _scanSettings.ymode = SCOPE_LIN;
        else
          _scanSettings.ymode = SCOPE_LOG;
      }
      if (ini.GetItem("style", arg)) {
        if (strcasecmp(arg, "line") == 0)
          _scanSettings.style = SCOPE_LINE;
        else
          _scanSettings.style = SCOPE_BARS;
      }

    }
  }

  delete[] inipath;
}

void OXMain::SaveIniFile() {
  char *inipath, tmp[256];

  inipath = GetResourcePool()->FindIniFile("rx320rc", INI_WRITE);
  if (!inipath) return;

  OIniFile ini(inipath, INI_WRITE);

  ini.PutNext("defaults");
  ini.PutItem("device", _device);
  ini.PutBool("show S meter", true);
  sprintf(tmp, "%d", 63 - _rx->GetVolume(RX320_SPEAKER));
  ini.PutItem("speaker volume", tmp);
  sprintf(tmp, "%d", 63 - _rx->GetVolume(RX320_LINE));
  ini.PutItem("line out volume", tmp);
  ini.PutBool("mute on exit", _mute_on_exit);
  ini.PutNewLine();

  ini.PutNext("vfo A");
  sprintf(tmp, "%ld", _vfoA.freq);
  ini.PutItem("frequency", tmp);
  sprintf(tmp, "%ld", _vfoA.step);
  ini.PutItem("step", tmp);
  switch (_vfoA.mode) {
    case RX320_AM:  strcpy(tmp, "AM"); break;
    case RX320_LSB: strcpy(tmp, "LSB"); break;
    case RX320_USB: strcpy(tmp, "USB"); break;
    case RX320_CW:  strcpy(tmp, "CW"); break;
  }
  ini.PutItem("mode", tmp);
  switch (_vfoA.agc) {
    case RX320_AGC_SLOW:   strcpy(tmp, "slow"); break;
    case RX320_AGC_MEDIUM: strcpy(tmp, "medium"); break;
    case RX320_AGC_FAST:   strcpy(tmp, "fast"); break;
  }
  ini.PutItem("agc", tmp);
  sprintf(tmp, "%d", _vfoA.filter);
  ini.PutItem("filter", tmp);
  sprintf(tmp, "%d", _vfoA.cwbfo);
  ini.PutItem("cw offset", tmp);
  sprintf(tmp, "%d", _vfoA.pbt);
  ini.PutItem("pbt offset", tmp);
  ini.PutNewLine();

  ini.PutNext("vfo B");
  sprintf(tmp, "%ld", _vfoB.freq);
  ini.PutItem("frequency", tmp);
  sprintf(tmp, "%ld", _vfoB.step);
  ini.PutItem("step", tmp);
  switch (_vfoB.mode) {
    case RX320_AM:  strcpy(tmp, "AM"); break;
    case RX320_LSB: strcpy(tmp, "LSB"); break;
    case RX320_USB: strcpy(tmp, "USB"); break;
    case RX320_CW:  strcpy(tmp, "CW"); break;
  }
  ini.PutItem("mode", tmp);
  switch (_vfoB.agc) {
    case RX320_AGC_SLOW:   strcpy(tmp, "slow"); break;
    case RX320_AGC_MEDIUM: strcpy(tmp, "medium"); break;
    case RX320_AGC_FAST:   strcpy(tmp, "fast"); break;
  }
  ini.PutItem("agc", tmp);
  sprintf(tmp, "%d", _vfoB.filter);
  ini.PutItem("filter", tmp);
  sprintf(tmp, "%d", _vfoB.cwbfo);
  ini.PutItem("cw offset", tmp);
  sprintf(tmp, "%d", _vfoB.pbt);
  ini.PutItem("pbt offset", tmp);
  ini.PutNewLine();

  ini.PutNext("S meter");
  sprintf(tmp, "%d", _dpanel->Smeter()->GetPeakHold());
  ini.PutItem("peak hold", tmp);
  sprintf(tmp, "%d", _dpanel->Smeter()->GetPeakAverage());
  ini.PutItem("peak average", tmp);
  sprintf(tmp, "%d", _spoll);
  ini.PutItem("sample interval", tmp);
  ini.PutNewLine();

  ini.PutNext("scan");
  sprintf(tmp, "%ld", _scanSettings.range);
  ini.PutItem("range", tmp);
  strcpy(tmp, (_scanSettings.ymode == SCOPE_LIN) ? "lin" : "log");
  ini.PutItem("mode", tmp);
  strcpy(tmp, (_scanSettings.style == SCOPE_LINE) ? "line" : "bars");
  ini.PutItem("style", tmp);
  ini.PutNewLine();

  if (_bands) {
    char name[20];

    ini.PutNext("bands");
    for (int i = 0; _bands[i].name != 0; ++i) {
      sprintf(name, "band%d", i+1);
      sprintf(tmp, "\"%s\",%ld,%ld", _bands[i].name,
                   _bands[i].lofreq, _bands[i].hifreq);
      ini.PutItem(name, tmp);
    }
    ini.PutNewLine();
  }

  delete[] inipath;
}

int OXMain::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;
  OSliderMessage *slmsg = (OSliderMessage *) msg;
  int i;

  switch (msg->type) {

    case MSG_BUTTON:
    case MSG_RADIOBUTTON:
      switch (msg->action) {

        case MSG_CLICK:
          switch (wmsg->id) {
            case 100:
              _rx->Reset();
              break;

            case 101:
              SwapVFO();
              break;

            case 102:
              CopyVFO();
              break;

            case 401:
              TuneUp(10);
              break;

            case 402:
              TuneUp(1);
              break;

            case 403:
              TuneDown(1);
              break;

            case 404:
              TuneDown(10);
              break;

            case 801:
            case 802:
            case 803:
              for (i = 0; i < 3; ++i)
                if (_agc[i]->WidgetID() != wmsg->id)
                  _agc[i]->SetState(BUTTON_UP);
              switch (wmsg->id) {
                case 801: _rx->SetAGC(RX320_AGC_SLOW); break;
                case 802: _rx->SetAGC(RX320_AGC_MEDIUM); break;
                case 803: _rx->SetAGC(RX320_AGC_FAST); break;
              }
              _vfoA.agc = _rx->GetAGC();
              _dpanel->SetAGC(_vfoA.agc);
              break;

            case 811:
            case 812:
            case 813:
            case 814:
              for (i = 0; i < 4; ++i)
                if (_mode[i]->WidgetID() != wmsg->id)
                  _mode[i]->SetState(BUTTON_UP);
              switch (wmsg->id) {
                case 811: _rx->SetMode(i = RX320_AM); break;
                case 812: _rx->SetMode(i = RX320_LSB); break;
                case 813: _rx->SetMode(i = RX320_USB); break;
                case 814: _rx->SetMode(i = RX320_CW); break;
              }
              _dpanel->SetMode(VFO_A, _vfoA.mode = i);
              _dpanel->SetCWO(_vfoA.cwbfo, (i == RX320_CW));
              break;

            case 851: SetStep(1); break;
            case 852: SetStep(10); break;
            case 853: SetStep(100); break;
            case 854: SetStep(1000); break;
            case 855: SetStep(5000); break;
            case 856: SetStep(10000); break;

            case 901:
              ToggleMuted();
              break;

            default:
              break;

          }
          break;

      }
      break;

    case MSG_TUNING_KNOB:
      switch (msg->action) {
        case MSG_UP:
          TuneUp(1, False);
          break;

        case MSG_DOWN:
          TuneDown(1, False);
          break;
      }
      break;

    case MSG_VSLIDER:
      switch (msg->action) {
        case MSG_SLIDERPOS:
          switch (slmsg->id) {
            case 301:
              _rx->SetVolume(RX320_SPEAKER, slmsg->pos);
              break;

            case 302:
              _rx->SetVolume(RX320_LINE, slmsg->pos);
              break;

            case 303:
              _rx->SetFilter(filtno[slmsg->pos]);
              _vfoA.filter = _rx->GetFilter()->filter;
              _dpanel->SetBW(_rx->GetFilter()->bandwidth);
              if (_scanWindow) _scanWindow->UpdateDisplay();
              break;

            case 304:
              _rx->SetPBT(100 * -slmsg->pos);
              _vfoA.pbt = _rx->GetPBT();
              _dpanel->SetPBT(_rx->GetPBT());
              break;

            case 305:
              _rx->SetCWO(100 * -slmsg->pos);
              _vfoA.cwbfo = _rx->GetCWO();
              _dpanel->SetCWO(_rx->GetCWO(), (_rx->GetMode() == RX320_CW));
              break;
          }
          break;
      }
      break;

    case MSG_RX320:
      switch (msg->action) {
        case MSG_SIGNAL:
          _dpanel->SetS(wmsg->id);
          if (_scanning) ScanStep(wmsg->id);
          break;

        case MSG_POWERON:
          _dpanel->PowerOn();
          break;

        default:
          break;
      }
      break;

    case MSG_HELP:
      if (msg->action == MSG_CLOSE) _helpWindow = 0;
      break;

    default:
      break;

  }

  return True;
}

void OXMain::SwapVFO() {
  Svfo temp;

  temp = _vfoA;
  _vfoA = _vfoB;
  _vfoB = temp;

  _rx->SetFrequency(_vfoA.freq);
  _rx->SetMode(_vfoA.mode);
  _rx->SetAGC(_vfoA.agc);
  _rx->SetFilter(_vfoA.filter);
  _rx->SetPBT(_vfoA.pbt);
  _rx->SetCWO(_vfoA.cwbfo);

  UpdateDisplay();
  UpdateSliders();
}

void OXMain::CopyVFO() {
  _vfoB = _vfoA;
  UpdateDisplay();
}

void OXMain::SetStep(int step) {
  _vfoA.step = step;
  _ruler->SetStep(step);
}

// This function is called from OXScanWindow

void OXMain::TuneTo(long freq, int knob_step) {

  if (_scanning) return;

  if (freq < 0) freq = 0;
  if (freq > 30000000) freq = 30000000;

  _vfoA.freq = freq;
  _rx->SetFrequency(_vfoA.freq);

  if (knob_step) _knob->StepKnob(knob_step);

  UpdateDisplay();
}

// This function is called from the OXFreqDB window

void OXMain::TuneTo(OFreqRecord *frec) {

  _vfoA.freq = (long) ((frec->freq * 1000000.0) + 0.5);
  _vfoA.step = frec->tuning_step;
  _vfoA.mode = frec->mode;
  _vfoA.agc = frec->agc;
  // TODO: use closest filter
  _vfoA.filter = frec->filter_bw;
  _vfoA.cwbfo = 1000;
  _vfoA.pbt = frec->pbt_offset;

  _rx->SetFrequency(_vfoA.freq);
  _rx->SetMode(_vfoA.mode);
  _rx->SetAGC(_vfoA.agc);
  _rx->SetFilter(_vfoA.filter);
  _rx->SetPBT(_vfoA.pbt);
  _rx->SetCWO(_vfoA.cwbfo);

  UpdateDisplay();
  UpdateSliders();
}

void OXMain::TuneUp(int steps, int move_knob) {
  _vfoA.freq = _rx->GetFrequency() + steps * _vfoA.step;
  if (_vfoA.freq > 30000000) _vfoA.freq = 30000000;
  _rx->SetFrequency(_vfoA.freq);
  _dpanel->SetFreq(VFO_A, _vfoA.freq);
  _ruler->SetFreq(_vfoA.freq);
  if (move_knob) _knob->StepKnob(1);
  if (_scanWindow) _scanWindow->UpdateDisplay();
}

void OXMain::TuneDown(int steps, int move_knob) {
  _vfoA.freq = _rx->GetFrequency() - steps * _vfoA.step;
  if (_vfoA.freq < 0) _vfoA.freq = 0;
  _rx->SetFrequency(_vfoA.freq);
  _dpanel->SetFreq(VFO_A, _vfoA.freq);
  _ruler->SetFreq(_vfoA.freq);
  if (move_knob) _knob->StepKnob(-1);
  if (_scanWindow) _scanWindow->UpdateDisplay();
}

void OXMain::VolumeUp(int steps, int channel) {
  int vol;

  vol = _rx->GetVolume(channel) - steps;
  if (vol < 0) vol = 0;
  _rx->SetVolume(channel, vol);

  _spkvol->SetPosition(_rx->GetVolume(RX320_SPEAKER));
  _linevol->SetPosition(_rx->GetVolume(RX320_LINE));
}

void OXMain::VolumeDown(int steps, int channel) {
  int vol;

  vol = _rx->GetVolume(channel) + steps;
  if (vol > 63) vol = 63;
  _rx->SetVolume(channel, vol);

  _spkvol->SetPosition(_rx->GetVolume(RX320_SPEAKER));
  _linevol->SetPosition(_rx->GetVolume(RX320_LINE));
}

void OXMain::ToggleMuted() {
  _muted = !_muted;
  _rx->Mute(_muted);
  _dpanel->SetMuted(_muted);
}

void OXMain::UpdateDisplay() {

  _dpanel->SetFreq(VFO_A, _vfoA.freq);
  _dpanel->SetMode(VFO_A, _vfoA.mode);

  _dpanel->SetFreq(VFO_B, _vfoB.freq);
  _dpanel->SetMode(VFO_B, _vfoB.mode);

  _dpanel->SetAGC(_vfoA.agc);
  _dpanel->SetPBT(_rx->GetPBT());
  _dpanel->SetBW(_rx->GetFilter()->bandwidth);
  _dpanel->SetCWO(_rx->GetCWO(), (_rx->GetMode() == RX320_CW));

  _ruler->SetFreq(_vfoA.freq);
  _ruler->SetStep(_vfoA.step);

  // TODO: show tuning step

  if (_scanWindow) _scanWindow->UpdateDisplay();
}

void OXMain::UpdateSliders() {

  _spkvol->SetPosition(_rx->GetVolume(RX320_SPEAKER));
  _linevol->SetPosition(_rx->GetVolume(RX320_LINE));

  int f = _rx->GetFilter()->filter;
  for (int i = 0; i < 34; ++i) {
    if (filtno[i] == f) {
      _bw->SetPosition(i);
      break;
    }
  }

  _pbt->SetPosition(-_rx->GetPBT() / 100);
  _cwo->SetPosition(-_rx->GetCWO() / 100);
}

void OXMain::GrabKeys() {
  static int keys[] = { XK_KP_0, XK_KP_1, XK_KP_2, XK_KP_3, XK_KP_4,
                        XK_KP_5, XK_KP_6, XK_KP_7, XK_KP_8, XK_KP_9,
                        XK_KP_Enter, XK_Escape, XK_Return, XK_space,
                        XK_0, XK_1, XK_2, XK_3, XK_4, XK_5, XK_6, XK_7,
                        XK_8, XK_9, XK_comma, XK_period, XK_KP_Decimal,
                        XK_Up, XK_Down, XK_Page_Up, XK_Page_Down,
                        XK_Left, XK_Right, XK_KP_Decimal, XK_F1,
                        XK_a, XK_c, XK_d, XK_l, XK_m, XK_u, XK_x,
                        XK_plus, XK_KP_Add, XK_minus, XK_KP_Subtract };

  for (int i = 0; i < sizeof(keys) / sizeof(keys[0]); ++i) {
    int keycode = XKeysymToKeycode(GetDisplay(), keys[i]);
    XGrabKey(GetDisplay(), keycode, AnyModifier, _id, True,
             GrabModeAsync, GrabModeAsync);
  }
}

int OXMain::HandleKey(XKeyEvent *event) {
  int  n, step;
  char tmp[10];
  KeySym keysym;
  XComposeStatus compose = { NULL, 0 };

  if (event->type == KeyPress) {
    n = XLookupString(event, tmp, sizeof(tmp)-1, &keysym, &compose);
    tmp[n] = 0;

    switch (keysym) {
      case XK_Delete:
      case XK_BackSpace:
        break;

      case XK_Page_Up:
        TuneUp(10);
        break;

      case XK_Page_Down:
        TuneDown(10);
        break;

      case XK_Up:
        VolumeUp(1, (event->state & ShiftMask) ? RX320_LINE : RX320_SPEAKER);
        break;

      case XK_Down:
        VolumeDown(1, (event->state & ShiftMask) ? RX320_LINE : RX320_SPEAKER);
        break;

      case XK_Left:
        TuneDown(1);
        break;

      case XK_Right:
        TuneUp(1);
        break;

      case XK_Escape:
        _typing = False;
        UpdateDisplay();
        break;

      case XK_plus:
      case XK_KP_Add:
        step = _vfoA.step;
        if (step < 1000)
          step *= 10;
        else if (step == 1000)
          step = 5000;
        else if (step == 5000)
          step = 10000;
        SetStep(step);
        break;

      case XK_minus:
      case XK_KP_Subtract:
        step = _vfoA.step;
        if (step == 10000)
          step = 5000;
        else if (step == 5000)
          step = 1000;
        else if (step > 1)
          step /= 10;
        SetStep(step);
        break;

      case XK_KP_Enter:
      case XK_Return:
        if (_tfreq < 0)
          _tfreq = 0;
        else if (_tfreq > 30000000)
          _tfreq = 30000000;
        _vfoA.freq = _tfreq;
        _rx->SetFrequency(_vfoA.freq);
        _typing = False;
        UpdateDisplay();
        break;

      case XK_a:  // Execute the action through the button grab-key
      case XK_u:  // mechanism without having to press the Alt key
      case XK_l:  // (note that we do not need to fake the Mod1Mask bit
      case XK_c:  // in event->state)
        return OXMainFrame::HandleKey(event);

      case XK_m:
        if (event->state == 0)
          ToggleMuted();
        else
          return OXMainFrame::HandleKey(event);
        break;

      case XK_d:
        if (event->state & Mod1Mask) OpenFreqDB();
        break;

      case XK_x:
        if (event->state & Mod1Mask) CloseWindow();
        break;

      case XK_F1:
        ShowHelp();
        break;

      case XK_space:
        SwapVFO();
        break;

      default:
        if (tmp[0] >= '0' && tmp[0] <= '9') {
          if (_typing) {
            _tfreq *= 10;
            _tfreq += tmp[0] - '0';
            _tfreq %= 100000000;
          } else {
            _tfreq = tmp[0] - '0';
            _typing = True;
          }
          _dpanel->SetFreq(VFO_A, _tfreq);
        } else if (tmp[0] == '.' || tmp[0] == ',') {
          _tfreq *= 1000;
          _tfreq %= 100000000;
          _typing = True;
          _dpanel->SetFreq(VFO_A, _tfreq);
        } else {
          return OXMainFrame::HandleKey(event);
        }
        break;
    }

    return True;
  }

  return OXMainFrame::HandleKey(event);
}

int OXMain::HandleButton(XButtonEvent *event) {

  if (event->type == ButtonPress) {
    if (event->button == Button4) {
      TuneUp((event->state & ShiftMask) ? 10 : 1);
    } else if (event->button == Button5) {
      TuneDown((event->state & ShiftMask) ? 10 : 1);
    }
  }

  if (event->type == ButtonRelease && event->button == Button3) {
    int retc, mute = _mute_on_exit;
    int menu_id = _menu->PopupMenu(event->x_root, event->y_root);
    OString sdev(_device);

    switch (menu_id) {
      case M_FREQDB:
        OpenFreqDB();
        break;

      case M_SCAN:
        OpenScan();
        break;

      case M_CONFIG:
        new OXSetupDialog(_client->GetRoot(), this, &sdev, &mute, &retc);
        if (retc == ID_OK) {
          _mute_on_exit = mute;
          delete[] _device;
          _device = StrDup(sdev.GetString());
          retc = _rx->SetSerial(_device);
          if (retc < 0) {
            char tmp[1024];

            OString stitle("Error");
            sprintf(tmp, "Serial port error:\n%s", _rx->GetLastError());
            new OXMsgBox(_client->GetRoot(), this, &stitle, new OString(tmp),
                   MB_ICONSTOP, ID_OK);
          }
        }
        break;

      case M_HELP:
        ShowHelp();
        break;

      case M_ABOUT:
        ShowAbout(this);
        break;

      case M_EXIT:
        CloseWindow();
        break;
    }
  }

  return True;
}

void OXMain::OpenFreqDB() {
  OXFreqDB *fdb;

  fdb = new OXFreqDB(_client->GetRoot(), this, 100, 100);
  fdb->Associate(this);
  AddFreqDBwindow(fdb);
}

void OXMain::AddFreqDBwindow(OXFreqDB *fdb) {
  // new windows are added to the beginning of the list
  fdb->next = _freqWindow;
  if (fdb->next) fdb->next->prev = fdb;
  fdb->prev = NULL;
  _freqWindow = fdb;
}

void OXMain::RemoveFreqDBwindow(OXFreqDB *fdb) {
  if (_freqWindow == fdb) _freqWindow = fdb->next;
  if (fdb->next) fdb->next->prev = fdb->prev;
  if (fdb->prev) fdb->prev->next = fdb->next;
}

void OXMain::OpenScan() {
  if (!_scanWindow) {
    _scanWindow = new OXScanWindow(_client->GetRoot(), this, &_scanSettings);
  }
  _scanWindow->MapRaised();
}

void OXMain::CloseScan() {
  _scanWindow = 0;
  StopScan();
}

void OXMain::StartScan() {
  if (_scanWindow) {  // this is superfluous, but just in case...
    _scanWindow->InitScan();
    _rx->CompressEvents(False);
    long freq = _scanWindow->GetStartFreq();
    _rx->SetFrequency(freq);
    //_rx->SetAGC(RX320_AGC_FAST);
    //_rx->RequestSignal(...)
    //mute?, etc...
    _dpanel->Scanning(True);
#if 1
    _dpanel->SetFreq(VFO_A, freq);
    _ruler->SetFreq(freq);
    _knob->StepKnob(1);
#endif
    _scanning = True;
  }
}

void OXMain::StopScan() {
  if (_scanning) {
    _dpanel->Scanning(False);
    _dpanel->SetFreq(VFO_A, _vfoA.freq);
    //_rx->RequestSignal(_spoll);
    //unmute, restore filters, mode, etc...
    _rx->SetAGC(_vfoA.agc);
    _rx->SetFrequency(_vfoA.freq);
    _rx->CompressEvents(True);
#if 1
    _dpanel->SetFreq(VFO_A, _vfoA.freq);
    _ruler->SetFreq(_vfoA.freq);
#endif
    _scanning = False;
  }
}

void OXMain::ScanStep(int sval) {
  if (_scanWindow) {
    _scanWindow->AddPoint(sval);
    long freq = _scanWindow->GetNextFreq();
    if (freq >= 0) {
      _rx->SetFrequency(freq);
#if 1
      _dpanel->SetFreq(VFO_A, freq);
      _ruler->SetFreq(freq);
      _knob->StepKnob(1);
#endif
    } else {
      StopScan();
    }
  } else {
    StopScan();
  }
}

void OXMain::ShowHelp() {
  if (!_helpWindow) {
    _helpWindow = new OXHelpWindow(_client->GetRoot(), NULL, 600, 650,
                                   "index.html", NULL, "rx320");
    _helpWindow->Associate(this);
  }
  _helpWindow->MapRaised();
}

void OXMain::ShowAbout(const OXWindow *t) {
  OAboutInfo info;
  
  info.wname = "About RX320";
  info.title = "RX320 version "RX320_VERSION"\n"
               "A control program for the Ten-Tec RX320\n"
               "Shortwave Receiver\n"
               "Compiled with xclass version "XCLASS_VERSION;
  info.copyright = "Copyright © 2000-2004, Hector Peraza.";
  info.text = "This program is free software; you can redistribute it "
              "and/or modify it under the terms of the GNU "
              "General Public License.\n\n"
              "http://xclass.sourceforge.net";

  new OXAboutDialog(_client->GetRoot(), t, &info);
}


//---------------------------------------------------------------------

int main(int argc, char *argv[]) {
  OXClient *clientX = new OXClient(argc, argv);

  OXMain *mainWindow = new OXMain(clientX->GetRoot(), 500, 200);
  mainWindow->MapWindow();

  clientX->Run();

  return 0;
}
