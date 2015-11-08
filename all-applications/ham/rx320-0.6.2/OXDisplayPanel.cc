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

#include <vector>

#include <xclass/utils.h>

#include "OXDisplayPanel.h"
#include "ORX320.h"

#include "muted.xpm"


//----------------------------------------------------------------------

OXDisplayPanel::OXDisplayPanel(const OXWindow *p) :
  OXCompositeFrame(p, 1, 1, SUNKEN_FRAME | DOUBLE_BORDER | OWN_BKGND) {

  SetBackgroundColor(_blackPixel);
  SetLayoutManager(new OHorizontalLayout(this));

//  _Smeter = new OXSMeter(this, S_VERTICAL);
//  AddFrame(_Smeter, new OLayoutHints(LHINTS_LEFT | LHINTS_EXPAND_Y));

  OXVerticalFrame *vf = new OXVerticalFrame(this, 10, 10);
  AddFrame(vf, new OLayoutHints(LHINTS_EXPAND_ALL));

  hfA = new OXHorizontalFrame(vf, 10, 10);
  vf->AddFrame(hfA, new OLayoutHints(LHINTS_BOTTOM | LHINTS_EXPAND_X, 5, 5, 0, 4));

  hfB = new OXHorizontalFrame(vf, 10, 10);
  vf->AddFrame(hfB, new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 5, 5, 6, 3));


  _modeA = new OXLabel(hfA, new OString("USB"));
  hfA->AddFrame(_modeA, new OLayoutHints(LHINTS_BOTTOMRIGHT, 5, 5, 0, 4));

  _modeA->SetTextColor(_client->GetColorByName("yellow"));
  _modeA->SetFont(_client->GetFont("Helvetica -12 bold"));
  _modeA->SetTextAlignment(TEXT_LEFT);

  _freqA = new OXLabel(hfA, new OString("00.000.000"));
  hfA->AddFrame(_freqA, new OLayoutHints(LHINTS_BOTTOMRIGHT));

  _freqA->SetTextColor(_client->GetColorByName("yellow"));
  _freqA->SetFont(_client->GetFont("Courier -34 bold"));
  _freqA->SetTextAlignment(TEXT_RIGHT);

  _muted = new OXIcon(hfA, _client->GetPicture("muted.xpm", muted_xpm));
  hfA->AddFrame(_muted, new OLayoutHints(LHINTS_BOTTOMLEFT, 5, 5, 5, 5));


  _modeB = new OXLabel(hfB, new OString("USB"));
  hfB->AddFrame(_modeB, new OLayoutHints(LHINTS_BOTTOMRIGHT, 5, 5, 0, 2));

  _modeB->SetTextColor(_client->GetColorByName("yellow"));
  _modeB->SetFont(_client->GetFont("Helvetica -8"));
  _modeB->SetTextAlignment(TEXT_LEFT);

  _freqB = new OXLabel(hfB, new OString("00.000.000"));
  hfB->AddFrame(_freqB, new OLayoutHints(LHINTS_BOTTOMRIGHT));

  _freqB->SetTextColor(_client->GetColorByName("yellow"));
  _freqB->SetFont(_client->GetFont("Courier -14 bold"));
  _freqB->SetTextAlignment(TEXT_RIGHT);

  _Smeter = new OXSMeter(hfB);
  _Smeter->Resize(150, _Smeter->GetDefaultHeight());
  hfB->AddFrame(_Smeter, new OLayoutHints(LHINTS_TOPLEFT, 0, 20, 0, 0));

  vfL = new OXVerticalFrame(this, 10, 10, FIXED_WIDTH);
  AddFrame(vfL, new OLayoutHints(LHINTS_RIGHT | LHINTS_EXPAND_Y, 8, 8, 2, 0));

  OLayoutHints *ll = new OLayoutHints(LHINTS_TOPLEFT, 2, 2, 1, 1);

  _agc = new OXLabel(vfL, new OString("AGC Slow"));
  _agc->SetTextColor(_client->GetColorByName("#00ffff"));
  _agc->SetFont(_client->GetFont("Helvetica -12"));
  vfL->AddFrame(_agc, ll);
  int w = _agc->GetDefaultWidth();

  _bw = new OXLabel(vfL, new OString("BW: 0000 Hz"));
  _bw->SetTextColor(_client->GetColorByName("#00ffff"));
  _bw->SetFont(_client->GetFont("Helvetica -12"));
  vfL->AddFrame(_bw, ll);
  w = max(w, _bw->GetDefaultWidth());

  _pwr = new OXLabel(vfL, new OString("Power ON"));
  _pwr->SetTextColor(_client->GetColorByName("red"));
  _pwr->SetFont(_client->GetFont("Helvetica -12 bold"));
  vfL->AddFrame(_pwr, ll);
  w = max(w, _pwr->GetDefaultWidth());

  OXHorizontalFrame *hfc = new OXHorizontalFrame(vfL);
  vfL->AddFrame(hfc, new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 2, 2, 1, 1));

  _pbt = new OXLabel(hfc, new OString("0000>"));
  _pbt->SetTextColor(_client->GetColorByName("#00ff00"));
  _pbt->SetFont(_client->GetFont("Courier -12"));
  hfc->AddFrame(_pbt, new OLayoutHints(LHINTS_TOPLEFT));
  _cwo = new OXLabel(hfc, new OString("+0000"));
  _cwo->SetTextColor(_client->GetColorByName("#00ff00"));
  _cwo->SetFont(_client->GetFont("Courier -12"));
  hfc->AddFrame(_cwo, new OLayoutHints(LHINTS_TOPRIGHT, 2, 0, 0, 0));
  w = max(w, hfc->GetDefaultWidth());

  vfL->Resize(w + 5, vfL->GetDefaultHeight());

  _pwrt = NULL;
  _scant = NULL;
  _scanstat = 0;
}

OXDisplayPanel::~OXDisplayPanel() {
  if (_pwrt) delete _pwrt;
  if (_scant) delete _scant;
}

//----------------------------------------------------------------------

int OXDisplayPanel::HandleTimer(OTimer *t) {
  if (t == _pwrt) {
    delete _pwrt;
    _pwrt = NULL;
    _pwr->SetText(new OString(""));
    vfL->Layout();
    return True;
  } else if (t == _scant) {
    delete _scant;
    _scant = new OTimer(this, 750);
    if (_scanstat) {
      _pwr->SetText(new OString(""));
      vfL->Layout();
      _scanstat = 0;
    } else {
      _pwr->SetText(new OString("Scanning"));
      vfL->Layout();
      _scanstat = 1;
    }
    return True;
  }
  return False;
}

void OXDisplayPanel::SetFreq(int vfo, long freq) {
  char tmp[30];
  int mhz, khz, hz;

  mhz = freq / 1000000L;
  khz = (freq / 1000L) % 1000L;
  hz = freq % 1000L;

  sprintf(tmp, "%2d.%03d.%03d", mhz, khz, hz);

  if (vfo == VFO_A)
    _freqA->SetText(new OString(tmp));
  else
    _freqB->SetText(new OString(tmp));

  Layout();
}

void OXDisplayPanel::SetMode(int vfo, int mode) {
  char *str;

  switch (mode) {
    default:
    case RX320_AM:  str = "AM"; break;
    case RX320_LSB: str = "LSB"; break;
    case RX320_USB: str = "USB"; break;
    case RX320_CW:  str = "CW"; break;
  }

  if (vfo == VFO_A)
    _modeA->SetText(new OString(str));
  else
    _modeB->SetText(new OString(str));

  Layout();
}

void OXDisplayPanel::SetAGC(int agc) {
  char *str;

  switch (agc) {
    default:
    case RX320_AGC_MEDIUM: str = "AGC Med"; break;
    case RX320_AGC_SLOW:   str = "AGC Slow"; break;
    case RX320_AGC_FAST:   str = "AGC Fast"; break;
  }

  _agc->SetText(new OString(str));
  vfL->Layout();
}

void OXDisplayPanel::SetBW(int bw) {
  char str[100];

  sprintf(str, "BW: %d Hz", bw);
  _bw->SetText(new OString(str));
  vfL->Layout();
}

void OXDisplayPanel::SetPBT(int pbt) {
  char str[100];

  sprintf(str, "%s%d%s",
          (pbt <= 0) ? "<" : "", abs(pbt), (pbt >= 0) ? ">" : "");
  _pbt->SetText(new OString(str));
  vfL->Layout();
}

void OXDisplayPanel::SetCWO(int cwo, int onoff) {
  char str[100];

  if (onoff) {
    sprintf(str, "%s%d",
            (cwo < 0) ? "-" : (cwo > 0) ? "+" : "", abs(cwo));
    _cwo->SetText(new OString(str));
  } else {
    _cwo->SetText(new OString(""));
  }
  vfL->Layout();
}

void OXDisplayPanel::PowerOn() {
  _pwr->SetText(new OString("Power ON"));
  vfL->Layout();
  if (!_pwrt) _pwrt = new OTimer(this, 1500);
}

void OXDisplayPanel::Scanning(int onoff) {
  if (onoff) {
    if (!_scant) {
      _scant = new OTimer(this, 750);
      _pwr->SetText(new OString("Scanning"));
      vfL->Layout();
      _scanstat = 1;
    }
  } else {
    if (_scant) delete _scant;
    _scant = NULL;
    _pwr->SetText(new OString(""));
    vfL->Layout();
    _scanstat = 0;
  }
}

void OXDisplayPanel::SetMuted(int onoff) {
  if (onoff)
    hfA->ShowFrame(_muted);
  else
    hfA->HideFrame(_muted);
  Layout();
}
