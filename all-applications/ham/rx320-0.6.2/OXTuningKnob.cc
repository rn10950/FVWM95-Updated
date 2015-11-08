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

#include <math.h>

#include <xclass/OXClient.h>

#include "OXTuningKnob.h"

#include "knob.xpm"
#include "hole.xpm"

//----------------------------------------------------------------------

OXTuningKnob::OXTuningKnob(const OXWindow *p, int id) :
  OXFrame(p, 1, 1), OXWidget(id, "OXTuningKnob") {

    _knob = _client->GetPicture("knob.xpm", knob_xpm);
    _hole = _client->GetPicture("hole.xpm", hole_xpm);

    _a = 0;   // 0...360

    XGrabButton(GetDisplay(), Button1, AnyModifier, _id, False,
                ButtonPressMask | ButtonReleaseMask | ButtonMotionMask,
                GrabModeAsync, GrabModeAsync, None, None);
}

OXTuningKnob::~OXTuningKnob() {
  _client->FreePicture(_knob);
  _client->FreePicture(_hole);
}

int OXTuningKnob::HandleButton(XButtonEvent *event) {
  return True;
}

int OXTuningKnob::HandleMotion(XMotionEvent *event) {
  int xc = (_knob->GetWidth() + 1) / 2;
  int yc = (_knob->GetHeight() + 1) / 2;

  int dy = event->y - yc;
  int dx = event->x - xc;

  int a;

  if (dy == 0) {
    if (dx > 0) a = 90; else a = 270;
  } else {
    a = (int) (atan((double) dx / (double) -dy) * 180.0 / M_PI + 0.5);
    // make the threshold symmetrical around the 18 degree step
    // and round the angle value to the closest step value
    if (a >= 0) a += 9; else a -= 9;
    a /= 18;
    a *= 18;
    if (dy > 0) a += 180;
  }

  // normalize the result to 0...360-1
  if (a > 360) a -= 360; else if (a < 0) a += 360;

  if (a != _a) {
    int action;
    int da = a - _a;
    if (abs(da) > 180) {
      action = (da > 0) ? MSG_DOWN : MSG_UP;
    } else {
      action = (da > 0) ? MSG_UP : MSG_DOWN;
    }
    _a = a;
    NeedRedraw(False);
    OWidgetMessage msg(MSG_TUNING_KNOB, action, _widgetID);
    SendMessage(_msgObject, &msg);
  }

  return True;
}

void OXTuningKnob::StepKnob(int step) {
  _a += 18 * step;
  if (_a > 360)
    _a -= 360;
  else if (_a < 0)
    _a += 360;
  NeedRedraw(False);
}

void OXTuningKnob::_DoRedraw() {
  OXFrame::_DoRedraw();

  _knob->Draw(GetDisplay(), _id, _whiteGC, 0, 0);

  int xc = (_knob->GetWidth() + 1) / 2;
  int yc = (_knob->GetHeight() + 1) / 2;

  int r = (_knob->GetWidth() - _hole->GetWidth()) / 2 - 7;

  int xch = (int) ((double) xc + r * sin(_a * M_PI / 180) + 0.5);
  int ych = (int) ((double) yc - r * cos(_a * M_PI / 180) + 0.5);

  int hx = xch - (_hole->GetWidth() + 1) / 2;
  int hy = ych - (_hole->GetHeight() + 1) / 2;

  _hole->Draw(GetDisplay(), _id, _whiteGC, hx, hy);
}
