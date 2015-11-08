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
#include <string.h>

#include <vector>

#include <xclass/OXCheckButton.h>
#include <xclass/OXTextEntry.h>
#include <xclass/OX3dLines.h>
#include <xclass/OXMsgBox.h>
#include <xclass/OXLabel.h>
#include <xclass/OXDDListBox.h>
#include <xclass/O2ColumnsLayout.h>
#include <xclass/utils.h>

#include "ORX320.h"
#include "OXDialogs.h"

extern SFilter Filters[];

//-------------------------------------------------------------------

OXSetupDialog::OXSetupDialog(const OXWindow *p, const OXWindow *main,
                             OString *sdev, int *mute, int *retc) :
  OXTransientFrame(p, main, 400, 200) {
    int  width;
    char name[1024];

    _sdev = sdev;
    _mute = mute;
    _retc = retc;
    if (_retc) *_retc = ID_CANCEL;

    //--- OK and Cancel buttons
    OXHorizontalFrame *bf = new OXHorizontalFrame(this, 60, 20, FIXED_WIDTH);

    OXTextButton *Ok, *Cancel;
    Ok = new OXTextButton(bf, new OHotString("OK"), ID_OK);
    Cancel = new OXTextButton(bf, new OHotString("Cancel"), ID_CANCEL);

    Ok->Associate(this);
    Cancel->Associate(this);

    SetDefaultAcceptButton(Ok);
    SetDefaultCancelButton(Cancel);

    //--- layout for buttons: top align, equally expand horizontally
    bly = new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 5, 0, 0, 0);

    //--- layout for the frame: place at bottom, right-aligned
    bfly = new OLayoutHints(LHINTS_BOTTOM | LHINTS_RIGHT, 0, 5, 5, 4);

    bf->AddFrame(Ok, bly);
    bf->AddFrame(Cancel, bly);

    width = Ok->GetDefaultWidth();
    width = max(width, Cancel->GetDefaultWidth());
    bf->Resize((width + 20) * 2, bf->GetDefaultHeight());

    AddFrame(bf, bfly);

    //--- text entry for serial device name

    OXHorizontalFrame *hf = new OXHorizontalFrame(this, 60, 20, FIXED_WIDTH);

    _te = new OXTextEntry(hf, NULL, -1);
    if (_sdev) _te->AddText(0, _sdev->GetString());
    _te->Resize(150, _te->GetDefaultHeight());

    OXLabel *lb = new OXLabel(hf, new OString("Serial device:"));
    hf->AddFrame(lb, new OLayoutHints(LHINTS_LEFT | LHINTS_CENTER_Y, 0, 5, 0, 0));
    hf->AddFrame(_te, new OLayoutHints(LHINTS_EXPAND_X | LHINTS_CENTER_Y));

    hf->Resize(lb->GetDefaultWidth() + 155, hf->GetDefaultHeight());

    AddFrame(hf, new OLayoutHints(LHINTS_NORMAL, 5, 5, 5, 5));

    //--- "mute on exit" check button

    _mb = new OXCheckButton(this, new OHotString("&Mute receiver on exit"), -1);
    AddFrame(_mb, new OLayoutHints(LHINTS_NORMAL, 5, 5, 5, 5));

    if (_mute && *_mute) _mb->SetState(BUTTON_DOWN);

    //--- setup toplevel: non-resizable, modal, etc.

    MapSubwindows();
    ODimension size = GetDefaultSize();
    Resize(size);

    CenterOnParent();

    SetWMSize(size.w, size.h);
    SetWMSizeHints(size.w, size.h, size.w, size.h, 0, 0);

    sprintf(name, "Configure rx320");
    SetWindowName(name);
    SetIconName(name);
    SetClassHints("XCLASS", "RX320");

    SetMWMHints(MWM_DECOR_ALL | MWM_DECOR_RESIZEH | MWM_DECOR_MAXIMIZE |
                                MWM_DECOR_MINIMIZE | MWM_DECOR_MENU,
                MWM_FUNC_ALL | MWM_FUNC_RESIZE | MWM_FUNC_MAXIMIZE | 
                               MWM_FUNC_MINIMIZE,
                MWM_INPUT_MODELESS);

    SetFocusOwner(_te);

    MapWindow();
    _client->WaitFor(this);
}

OXSetupDialog::~OXSetupDialog() {
  delete bly;
  delete bfly;
}

int OXSetupDialog::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;

  switch (msg->action) {
    case MSG_CLICK:

      switch (msg->type) {
        case MSG_BUTTON:
          switch (wmsg->id) {
            case ID_OK:
              if (_sdev) {
                _sdev->Clear();
                _sdev->Append(_te->GetString());
              }
              if (_mute) {
                *_mute = (_mb->GetState() != BUTTON_UP);
              }
              if (_retc) *_retc = ID_OK;
            case ID_CANCEL:
              CloseWindow();
              break;

          }
          break;

        default:
          break;
      }
      break;

    default:
      break;
  }

  return True;
}

//-------------------------------------------------------------------

OXEditStation::OXEditStation(const OXWindow *p, const OXWindow *main,
                             OFreqRecord *frec, int *retc, int new_station,
                             unsigned long options) :
  OXTransientFrame(p, main, 400, 200, options) {
    int  width;
    char name[1024];

    _freqRec = frec;

    _retc = retc;
    if (_retc) *_retc = ID_CANCEL;

    //--- OK and Cancel buttons
    OXHorizontalFrame *bf = new OXHorizontalFrame(this, 60, 20, FIXED_WIDTH);

    OXTextButton *Ok, *Cancel;
    Ok = new OXTextButton(bf, new OHotString("OK"), ID_OK);
    Cancel = new OXTextButton(bf, new OHotString("Cancel"), ID_CANCEL);

    Ok->Associate(this);
    Cancel->Associate(this);

    SetDefaultAcceptButton(Ok);
    SetDefaultCancelButton(Cancel);

    //--- layout for buttons: top align, equally expand horizontally
    bly = new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 5, 0, 0, 0);

    //--- layout for the frame: place at bottom, right-aligned
    bfly = new OLayoutHints(LHINTS_BOTTOM | LHINTS_RIGHT, 0, 5, 5, 4);

    bf->AddFrame(Ok, bly);
    bf->AddFrame(Cancel, bly);

    width = Ok->GetDefaultWidth();
    width = max(width, Cancel->GetDefaultWidth());
    bf->Resize((width + 20) * 2, bf->GetDefaultHeight());

    AddFrame(bf, bfly);

    //--- layout for separator lines...

    lnly = new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 5, 5, 0, 0);

    //--- text entry frames...

    lyl = new OLayoutHints(LHINTS_LEFT | LHINTS_CENTER_Y);
    lyr = new OLayoutHints(LHINTS_RIGHT | LHINTS_CENTER_Y);
    OLayoutHints *lv = new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X,
                                        5, 5, 5, 5);

    OXCompositeFrame *hf = new OXCompositeFrame(this, 10, 10);
    hf->SetLayoutManager(new O2ColumnsLayout(hf, 10, 0));

    hf->AddFrame(new OXLabel(hf, new OHotString("Station name:")), NULL);
    _name = new OXTextEntry(hf, NULL, 101);
    _name->Resize(180, _name->GetDefaultHeight());
    hf->AddFrame(_name, NULL);

    hf->AddFrame(new OXLabel(hf, new OHotString("Frequency, MHz:")), NULL);
    _freq = new OXTextEntry(hf, NULL, 103);
    _freq->Resize(180, _freq->GetDefaultHeight());
    hf->AddFrame(_freq, NULL);

    hf->AddFrame(new OXLabel(hf, new OHotString("Mode:")), NULL);
    _mode = new OXDDListBox(hf, 103);
    _mode->Resize(180, _mode->GetDefaultHeight());
    hf->AddFrame(_mode, NULL);

    hf->AddFrame(new OXLabel(hf, new OHotString("Filter:")), NULL);
    _filter = new OXDDListBox(hf, 103);
    _filter->Resize(180, _filter->GetDefaultHeight());
    hf->AddFrame(_filter, NULL);

    hf->AddFrame(new OXLabel(hf, new OHotString("AGC:")), NULL);
    _agc = new OXDDListBox(hf, 103);
    _agc->Resize(180, _agc->GetDefaultHeight());
    hf->AddFrame(_agc, NULL);

    hf->AddFrame(new OXLabel(hf, new OHotString("Tuning step:")), NULL);
    _tstep = new OXTextEntry(hf, NULL, 102);
    _tstep->Resize(180, _tstep->GetDefaultHeight());
    hf->AddFrame(_tstep, NULL);

    hf->AddFrame(new OXLabel(hf, new OHotString("PBT offset:")), NULL);
    _pbt = new OXTextEntry(hf, NULL, 102);
    _pbt->Resize(180, _pbt->GetDefaultHeight());
    hf->AddFrame(_pbt, NULL);

    hf->AddFrame(new OXLabel(hf, new OHotString("Location:")), NULL);
    _loc = new OXTextEntry(hf, NULL, 102);
    _loc->Resize(180, _loc->GetDefaultHeight());
    hf->AddFrame(_loc, NULL);

    hf->AddFrame(new OXLabel(hf, new OHotString("Language:")), NULL);
    _lang = new OXTextEntry(hf, NULL, 104);
    _lang->Resize(180, _lang->GetDefaultHeight());
    hf->AddFrame(_lang, lyr);

    hf->AddFrame(new OXLabel(hf, new OHotString("Start time:")), NULL);
    _stime = new OXTextEntry(hf, NULL, 104);
    _stime->Resize(180, _stime->GetDefaultHeight());
    hf->AddFrame(_stime, lyr);

    hf->AddFrame(new OXLabel(hf, new OHotString("End time:")), NULL);
    _etime = new OXTextEntry(hf, NULL, 104);
    _etime->Resize(180, _etime->GetDefaultHeight());
    hf->AddFrame(_etime, lyr);

    hf->AddFrame(new OXLabel(hf, new OHotString("Notes:")), NULL);
    _notes = new OXTextEntry(hf, NULL, 104);
    _notes->Resize(180, _notes->GetDefaultHeight());
    hf->AddFrame(_notes, lyr);

    AddFrame(hf, lv);

    AddFrame(new OXHorizontal3dLine(this), lnly);

    //--- init controls

    InitControls();

    //--- setup toplevel: non-resizable, modal, etc.

    MapSubwindows();
    ODimension size = GetDefaultSize();
    Resize(size);

    CenterOnParent();

    SetWMSize(size.w, size.h);
    SetWMSizeHints(size.w, size.h, size.w, size.h, 0, 0);

    sprintf(name, "%s Station", new_station ? "Add New" : "Edit");
    SetWindowName(name);
    SetIconName(name);
    SetClassHints("XCLASS", "RX320");

    SetMWMHints(MWM_DECOR_ALL | MWM_DECOR_RESIZEH | MWM_DECOR_MAXIMIZE |
                                MWM_DECOR_MINIMIZE | MWM_DECOR_MENU,
                MWM_FUNC_ALL | MWM_FUNC_RESIZE | MWM_FUNC_MAXIMIZE | 
                               MWM_FUNC_MINIMIZE,
                MWM_INPUT_MODELESS);

    SetFocusOwner(_name);

    MapWindow();
    _client->WaitFor(this);
}

OXEditStation::~OXEditStation() {

  delete bly;
  delete bfly;

  delete lnly;

  delete lyl;
  delete lyr;

  //delete lv;
}

void OXEditStation::InitControls() {
  char string[256];

  _mode->AddEntry(new OString("AM"), RX320_AM);
  _mode->AddEntry(new OString("LSB"), RX320_LSB);
  _mode->AddEntry(new OString("USB"), RX320_USB);
  _mode->AddEntry(new OString("CW"), RX320_CW);

  _agc->AddEntry(new OString("Slow"), RX320_AGC_SLOW);
  _agc->AddEntry(new OString("Medium"), RX320_AGC_MEDIUM);
  _agc->AddEntry(new OString("Fast"), RX320_AGC_FAST);

  SFilter *fe;
  for (fe = Filters; fe->bandwidth > 0; fe++) {
    sprintf(string, "%d Hz", fe->bandwidth);
    _filter->AddEntry(new OString(string), fe->filter);
  }

  if (!_freqRec) return;

  _name->AddText(0, _freqRec->name);

  sprintf(string, "%.6f", _freqRec->freq);
  _freq->AddText(0, string);

  _mode->Select(_freqRec->mode);

  if (_freqRec->filter_bw > 34) {
    for (fe = Filters; fe->bandwidth > 0; fe++)
      if (_freqRec->filter_bw >= fe->bandwidth) break;
    _filter->Select(fe->filter);
  } else {
    _filter->Select(_freqRec->filter_bw);
  }

  _agc->Select(_freqRec->agc);

  sprintf(string, "%d", _freqRec->tuning_step);
  _tstep->AddText(0, string);

  sprintf(string, "%d", _freqRec->pbt_offset);
  _pbt->AddText(0, string);

  _loc->AddText(0, _freqRec->location);
  _lang->AddText(0, _freqRec->language);
  _stime->AddText(0, _freqRec->start_time);
  _etime->AddText(0, _freqRec->end_time);
  _notes->AddText(0, _freqRec->notes);
}

void OXEditStation::UpdateRecord() {

  if (!_freqRec) return;

  strncpy(_freqRec->name, _name->GetString(), 20);
  _freqRec->name[20] = '\0';

  _freqRec->freq = atof(_freq->GetString());
  _freqRec->mode = _mode->GetSelectedEntry()->ID();

  SFilter *fe;
  for (fe = Filters; fe->bandwidth > 0; fe++)
    if (_filter->GetSelectedEntry()->ID() == fe->filter) break;
  _freqRec->filter_bw = fe->bandwidth;

  _freqRec->agc = _agc->GetSelectedEntry()->ID();
  _freqRec->tuning_step = atoi(_tstep->GetString());
  _freqRec->pbt_offset = atoi(_pbt->GetString());

  strncpy(_freqRec->location, _loc->GetString(), 15);
  _freqRec->location[15] = '\0';

  strncpy(_freqRec->language, _lang->GetString(), 10);
  _freqRec->language[10] = '\0';

  strncpy(_freqRec->start_time, _stime->GetString(), 4);
  _freqRec->start_time[4] = '\0';

  strncpy(_freqRec->end_time, _etime->GetString(), 4);
  _freqRec->end_time[4] = '\0';

  strncpy(_freqRec->notes, _notes->GetString(), 64);
  _freqRec->notes[64] = '\0';
}

int OXEditStation::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;

  switch (msg->action) {
    case MSG_CLICK:

      switch (msg->type) {
        case MSG_BUTTON:
          switch (wmsg->id) {
            case ID_OK:
              UpdateRecord();
              if (_retc) *_retc = ID_OK;
            case ID_CANCEL:
              CloseWindow();
              break;

          }
          break;

        default:
          break;
      }
      break;

    default:
      break;
  }

  return True;
}
