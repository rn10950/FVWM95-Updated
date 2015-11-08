/**************************************************************************

    This file is part of xclass.
    Copyright (C) 1998-2002 Mike McDonald, Hector Peraza.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

**************************************************************************/

#include <X11/Xlib.h>
#include <xclass/OXSpinner.h>

#include "icons/arrow-down.xpm"
#include "icons/arrow-up.xpm"


//----------------------------------------------------------------------

OXDoubleScroller :: OXDoubleScroller(const OXWindow *p, int w, int h) :
  OXCompositeFrame(p, w, h, CHILD_FRAME | VERTICAL_FRAME),
  OXWidget(-1, "OXDoubleScroller") {

    const OPicture *_picd = _client->GetPicture("arrow-down.xpm",
                                                XCP_arrow_down);
    const OPicture *_picu = _client->GetPicture("arrow-up.xpm",
                                                XCP_arrow_up);

    if (!(_picd || _picu))
      FatalError("OXDoubleScroller: arrow images failed to load");

    _up = new OXScrollBarElt(this, _picu, SB_WIDTH, _h/2, RAISED_FRAME);
    _down = new OXScrollBarElt(this, _picd, SB_WIDTH, _h/2, RAISED_FRAME);

#if 0
    AddFrame(_up, _l1 = new OLayoutHints(LHINTS_TOP));
    AddFrame(_down, _l2 = new OLayoutHints(LHINTS_BOTTOM));
#else
    AddFrame(_up, _l1 = new OLayoutHints(LHINTS_EXPAND_Y));
    AddFrame(_down, _l2 = new OLayoutHints(LHINTS_EXPAND_Y));
#endif

    XGrabButton(GetDisplay(), AnyButton, AnyModifier, _id, False,
                ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                GrabModeAsync, GrabModeAsync, None, None);
}

OXDoubleScroller::~OXDoubleScroller() {
  delete _l1;
  delete _l2;
}

int OXDoubleScroller::HandleButton(XButtonEvent *event) {

  if (event->type == ButtonPress) {
    if (event->subwindow == _up->GetId()) {
      _up->SetState(BUTTON_DOWN);
      OWidgetMessage msg(MSG_BUTTON, MSG_CLICK, 1);
      SendMessage(_msgObject, &msg);
    } else if (event->subwindow == _down->GetId()) {
      _down->SetState(BUTTON_DOWN);
      OWidgetMessage msg(MSG_BUTTON, MSG_CLICK, 0);
      SendMessage(_msgObject, &msg);
    }

  } else if (event->type == ButtonRelease) {
    _up->SetState(BUTTON_UP);
    _down->SetState(BUTTON_UP);
    OWidgetMessage msg(MSG_BUTTON, MSG_CLICK, 2);
    SendMessage(_msgObject, &msg);
  }

  return True;
}

//----------------------------------------------------------------------

OXSpinner::OXSpinner(const OXWindow *p, char *name, int id) :
  OXCompositeFrame(p, 150, 100,
                   HORIZONTAL_FRAME | SUNKEN_FRAME | DOUBLE_BORDER | OWN_BKGND,
		   _defaultDocumentBackground), OXWidget(id, "OXSpinner") {

  _te = new OXTextEntry(this, new OTextBuffer(255), 100);
  _te->Resize(_te->GetDefaultWidth() * 2, _te->GetDefaultHeight());
  _te->ChangeOptions(CHILD_FRAME);
  _te->Associate(this);

  int _buth = _te->GetDefaultHeight();

  _vf = new OXDoubleScroller(this, _buth, _buth);
  _vf->Associate(this);

  AddFrame(_te, _l1 = new OLayoutHints(LHINTS_LEFT | LHINTS_EXPAND_X));
  AddFrame(_vf, _l2 = new OLayoutHints(LHINTS_LEFT | LHINTS_EXPAND_Y));

//  Resize((_te->GetWidth() * 3) + _buth + 5, _buth);  // hmmm...

  _min = _value = 0.0;
  _max = 100.0;
  _step = 1.0;
  _prec = 0;
  _editable = _rollOver = true;
  _SetValue();
  _percent = true;
  _dir = 0;
  _tmr = NULL;
}

OXSpinner::~OXSpinner() {
  delete _l1;
  delete _l2;
}

void OXSpinner::SetEditable(bool ed) {
  ed ? _te->Enable() : _te->Disable();
  _editable = ed;
}

void OXSpinner::SetPercent(bool perc) {
  _percent = perc;
  _SetValue();
}

void OXSpinner::SetRange(float min, float max, float step) {
  _value = _min = min;
  _max = max;
  _step = step;
  _SetValue();
}

float OXSpinner::GetValue() {
  _GetValue();
  return _value;
}

void OXSpinner::_SetValue(bool sendmsg) {
  // Cute hack but it works :)
  static char temp1[256];
  static char temp2[256];

  if (_percent)
    sprintf(temp1, "%%.%df %%%%", _prec);
  else
    sprintf(temp1, "%%.%df", _prec);
  sprintf(temp2, temp1, _value);

  _te->Clear();
  _te->AddText(0, temp2);

  if (sendmsg) {
    OWidgetMessage msg(MSG_SPINNER, MSG_VALUECHANGED, _widgetID);
    SendMessage(_msgObject, &msg);
  }
}

int OXSpinner::HandleTimer(OTimer *timer) {

  if (timer == _tmr) {
    //int dir = tmr->GetDir();
    delete _tmr; _tmr = NULL;
    switch (_dir) {
    case 1:
      _Up();
      _tmr = new OTimer(this, 50);
      break;
    case 0:
      _Down();
      _tmr = new OTimer(this, 50);
      break;
    }
  }

  return True;
}

void OXSpinner::_Up() {

  if (_editable) _GetValue();

  if ((_value + _step) > _max) {
    if (_rollOver)
      _value = _min;
    else {
      _value = _max;
      if (_tmr) delete _tmr;
      _tmr = NULL;
      return;
    }
  } else {
    _value += _step;
  }
  _SetValue(True);
  if (_editable) _te->SelectAll();
}

void OXSpinner::_Down() {
  if (_editable)
    _GetValue();
  if ((_value - _step) < _min) {
    if (_rollOver)
      _value = _max;
    else {
      _value = _min;
      if (_tmr) delete _tmr;
      _tmr = NULL;
      return;
    }
  } else {
    _value -= _step;
  }
  _SetValue(True);
  if (_editable) _te->SelectAll();
}

void OXSpinner::_GetValue() {
  float vl;

  if (sscanf(_te->GetString(), "%f", &vl) == 1) {
    _value = vl;
    _SetValue();
  }
}

int OXSpinner::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;

  switch (msg->type) {
  case MSG_BUTTON:
    switch (msg->action) {
    case MSG_CLICK:
      switch (wmsg->id) {
      case 2:			// Button Released
	if (_tmr) delete _tmr;
	_tmr = NULL;
	break;

      case 1:			// Up Arrow Button
	_Up();
        if (_tmr) delete _tmr;
	_tmr = new OTimer(this, 400);
        _dir = 1;
	break;

      case 0:			// Down Arrow Button
	_Down();
        if (_tmr) delete _tmr;
	_tmr = new OTimer(this, 400);
        _dir = 0;
	break;

      }
    }
    break;

  case MSG_TEXTENTRY:
    switch (msg->action) {
    case MSG_TEXTCHANGED: {
#if 0
      _SetValue(True);
#else
      OWidgetMessage msg(MSG_SPINNER, MSG_VALUECHANGED, _widgetID);
      SendMessage(_msgObject, &msg);
#endif
    } break;

    }
    break;

  }

  return True;
}
