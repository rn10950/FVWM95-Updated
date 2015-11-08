/**************************************************************************

    This file is part of xclass.
    Copyright (C) 2000, 2001, Hector Peraza.

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

#include <stdio.h>

#include <xclass/OXClient.h>
#include <xclass/OX3dLines.h>
#include <xclass/OXMsgBox.h>
#include <xclass/OGC.h>
#include <xclass/OColor.h>
#include <xclass/OResourcePool.h>
#include <xclass/OXColorSelect.h>
#include <xclass/OXColorDialog.h>


//----------------------------------------------------------------------

OXColorFrame::OXColorFrame(const OXWindow *p, OColor c, int n) :
  OXFrame(p, 20, 20, OWN_BKGND) {

    SetBackgroundColor(_client->GetColor(c));

    _color = c;
    id = n;

    AddInput(ButtonPressMask | ButtonReleaseMask);

    _msgObject = p;
    _active = false;

    grayGC = _client->GetResourcePool()->GetFrameBckgndGC()->GetGC();
}

OXColorFrame::~OXColorFrame() {
  _client->FreeColor(_pixel);
}

int OXColorFrame::HandleButton(XButtonEvent *event) {

  if (event->type == ButtonPress) {
    OColorSelMessage msg(MSG_COLORSEL, MSG_CLICK, id, event->button, _color);
    SendMessage(_msgObject, &msg);
  } else {    // ButtonRelease
    OColorSelMessage msg(MSG_COLORSEL, MSG_SELCHANGED, id, event->button, _color);
    SendMessage(_msgObject, &msg);
  }

  return True;
}

void OXColorFrame::DrawBorder() {

  if (_active) {

    DrawRectangle(_blackGC, 0, 0, _w - 1, _h - 1);
    DrawRectangle(_whiteGC, 1, 1, _w - 3, _h - 3);
    DrawRectangle(_blackGC, 2, 2, _w - 5, _h - 5);

  } else {

    DrawRectangle(grayGC, 0, 0, _w - 1, _h - 1);

    _Draw3dRectangle(DOUBLE_BORDER | SUNKEN_FRAME, 1, 1, _w - 2, _h - 2);

  }
}

//----------------------------------------------------------------------

OX16ColorSelector::OX16ColorSelector(const OXWindow *p) :
  OXCompositeFrame(p, 10, 10) {

    SetLayoutManager(new OMatrixLayout(this, 4, 4, 0, 0));

    ce[0]  = new OXColorFrame(this, OColor(0xFF, 0xFF, 0xFF), 0);
    ce[1]  = new OXColorFrame(this, OColor(0x00, 0x00, 0x00), 1);
    ce[2]  = new OXColorFrame(this, OColor(0x00, 0x00, 0x80), 2);
    ce[3]  = new OXColorFrame(this, OColor(0x00, 0x80, 0x00), 3);
    ce[4]  = new OXColorFrame(this, OColor(0xFF, 0x00, 0x00), 4);
    ce[5]  = new OXColorFrame(this, OColor(0x80, 0x00, 0x00), 5);
    ce[6]  = new OXColorFrame(this, OColor(0x80, 0x00, 0x80), 6);
    ce[7]  = new OXColorFrame(this, OColor(0xFF, 0x80, 0x00), 7);
    ce[8]  = new OXColorFrame(this, OColor(0xFF, 0xFF, 0x00), 8);
    ce[9]  = new OXColorFrame(this, OColor(0x00, 0xFF, 0x00), 9);
    ce[10] = new OXColorFrame(this, OColor(0x00, 0x80, 0x80), 10);
    ce[11] = new OXColorFrame(this, OColor(0x00, 0xFF, 0xFF), 11);
    ce[12] = new OXColorFrame(this, OColor(0x00, 0x00, 0xFF), 12);
    ce[13] = new OXColorFrame(this, OColor(0xFF, 0x00, 0xFF), 13);
    ce[14] = new OXColorFrame(this, OColor(0x80, 0x80, 0x80), 14);
    ce[15] = new OXColorFrame(this, OColor(0xC0, 0xC0, 0xC0), 15);

    for (int i = 0; i < 16; i++)
      AddFrame(ce[i], new OLayoutHints(LHINTS_CENTER_X | LHINTS_CENTER_Y));

    _msgObject = p;
    active = -1;
}

void OX16ColorSelector::SetActive(int newat) {
  if (active != newat) {
    if ((active >= 0) && (active <= 15)) {
      ce[active]->SetActive(false);
    }
    active = newat;
    if ((active >= 0) && (active <= 15)) {
      ce[active]->SetActive(true);
    }
  }
}

int OX16ColorSelector::ProcessMessage(OMessage *msg) {
  OColorSelMessage *cmsg = (OColorSelMessage *) msg;

  switch (msg->type) {
  case MSG_COLORSEL:
    switch (msg->action) {
    case MSG_SELCHANGED:
      switch (cmsg->button) {
      case Button1:
	OColorSelMessage msg1(MSG_COLORSEL, MSG_SELCHANGED, active,
                              cmsg->button, cmsg->color);
	SendMessage(_msgObject, &msg1);
	break;
      }
      break;

    case MSG_CLICK:
      switch (cmsg->button) {
      case Button1:
	SetActive(cmsg->id);
	break;
      }
      break;

    }
  }

  return True;
}

//----------------------------------------------------------------------

OXColorPopup::OXColorPopup(const OXWindow *p, OColor color, int dlg) :
  OXCompositeFrame(p, 10, 10, DOUBLE_BORDER | RAISED_FRAME | OWN_BKGND,
		   _defaultFrameBackground) {

  _msgObject = NULL;
  _currentColor = color;

  XSetWindowAttributes wattr;
  unsigned long mask;

  mask = CWOverrideRedirect;  // | CWSaveUnder ;
  wattr.override_redirect = True;
  //wattr.save_under = True;
  XChangeWindowAttributes(GetDisplay(), _id, mask, &wattr);

  AddInput(StructureNotifyMask);

  active = -1;
  _launchDialog = False;

  OX16ColorSelector *cs = new OX16ColorSelector(this);
  AddFrame(cs, new OLayoutHints(LHINTS_CENTER_X, 1, 1, 1, 1));

  if (dlg) {
    AddFrame(new OXHorizontal3dLine(this),
             new OLayoutHints(LHINTS_EXPAND_X | LHINTS_CENTER_Y, 2, 2, 2, 2));
    OXButton *other = new OXTextButton(this, new OHotString("&Other..."), 1002);
    other->Associate(this);
    AddFrame(other, new OLayoutHints(LHINTS_EXPAND_X, 2, 10, 2, 2));
  }

  MapSubwindows();

//  Resize(cs->GetDefaultWidth() + 6,
//	 cs->GetDefaultHeight() + other->GetDefaultHeight());
  Resize(GetDefaultSize());
}

void OXColorPopup::EndPopup() {
  XUngrabPointer(GetDisplay(), CurrentTime);
  UnmapWindow();
}

void OXColorPopup::PlacePopup(int x, int y, int w, int h) {
  int rx, ry;
  unsigned int rw, rh;
  unsigned int dummy;
  Window wdummy;

  // Parent is root window for the popup:
  XGetGeometry(GetDisplay(), _parent->GetId(), &wdummy,
	       &rx, &ry, &rw, &rh, &dummy, &dummy);

  if (x < 0) x = 0;
  if (x + _w > rw) x = rw - _w;
  if (y < 0) y = 0;
  if (y + _h > rh) y = rh - _h;

  MoveResize(x, y, w, h);
  MapSubwindows();
  Layout();
  MapRaised();

  XGrabPointer(GetDisplay(), _id, False /*True*/,
               ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
               GrabModeAsync, GrabModeAsync, None,
               GetResourcePool()->GetGrabCursor(), CurrentTime);

  _launchDialog = False;

  _client->WaitForUnmap(this);
  EndPopup();

  if (_launchDialog) {
    int retc;
    OColor color = _currentColor;

    new OXColorDialog(_client->GetRoot(),
                      ((OXWindow *)_msgObject)->GetTopLevel(),
                      &retc, &color);

    if (retc == ID_OK) {
      _currentColor = color;
      OColorSelMessage msg1(MSG_COLORSEL, MSG_SELCHANGED, -1, 0, color);
      SendMessage(_msgObject, &msg1);
    }
  }

}

int OXColorPopup::HandleButton(XButtonEvent *event) {
  if (event->x < 0 || event->x >= _w ||
      event->y < 0 || event->y >= _h) {
    if (event->type == ButtonRelease) UnmapWindow();
  } else {
    OXFrame *f = GetFrameFromPoint(event->x, event->y);
    if (f && f != this) {
      TranslateCoordinates(f, event->x, event->y, &event->x, &event->y);
      f->HandleButton(event);
    }
  }
  return True;
}

int OXColorPopup::ProcessMessage(OMessage *msg) {
  OColorSelMessage *cmsg = (OColorSelMessage *) msg;

  switch (msg->type) {
  case MSG_COLORSEL:
    switch (msg->action) {
    case MSG_SELCHANGED:
      SendMessage(_msgObject, cmsg);
      UnmapWindow();
      break;

    default:
      break;
    }
    break;

  case MSG_BUTTON:
    switch (msg->action) {
    case MSG_CLICK:
      if (cmsg->id == 1002) {
        _launchDialog = True;
	UnmapWindow();
      }
      break;
    }
    break;

  }

  return True;
}

//----------------------------------------------------------------------

OXColorSelect::OXColorSelect(const OXWindow *p, OColor color,
                             int id, int dlg) : OXButton(p, id) {

  _dlg = dlg;
  _color = color;
  _pixel = _client->GetColor(_color); 

  _drawGC = new OXGC(GetDisplay(), _id);
  _drawGC->SetForeground(_pixel);
}

OXColorSelect::~OXColorSelect() {
  delete _drawGC;
  _client->FreeColor(_pixel);
}

int OXColorSelect::ProcessMessage(OMessage *msg) {
  OColorSelMessage *cmsg = (OColorSelMessage *) msg;

  switch (msg->type) {
  case MSG_COLORSEL:
    switch (msg->action) {
    case MSG_SELCHANGED:
      {
	SetColor(cmsg->color);
	OColorSelMessage msg1(MSG_COLORSEL, MSG_CLICK, _widgetID, 0, cmsg->color);
	SendMessage(_msgObject, &msg1);
      }
      break;

    default:
      break;
    }
    break;

  }

  return True;
}

int OXColorSelect::HandleButton(XButtonEvent *event) {

  OXFrame::HandleButton(event);
  if (!IsEnabled()) return True;

  if (event->button != Button1) return False;

  if ((event->type == ButtonPress) && TakesFocus())
    RequestFocus();
 
  if (event->type == ButtonPress) {
    _click = True;
    if (_state != BUTTON_DOWN) {
      _prevstate = _state;
      SetState(BUTTON_DOWN);
    }
  } else {
    _click = False;
    if (_state != _prevstate) {
      if (_state == BUTTON_DOWN) {
        SetState(BUTTON_UP);

        int ax, ay;
        Window wdummy;

        OXColorPopup *cd = new OXColorPopup(_client->GetRoot(), _color, _dlg);
        XTranslateCoordinates(GetDisplay(), _id, _client->GetRoot()->GetId(),
                              0, _h, &ax, &ay, &wdummy);
        cd->Associate(this);
        cd->PlacePopup(ax, ay, cd->GetDefaultWidth(), cd->GetDefaultHeight());
      }
    }
  }

  return True;
}

void OXColorSelect::_DoRedraw() {
  int x, y, w, h;

  OXButton::_DoRedraw();

  if (IsEnabled()) {

    // color rectangle

    x = _insets.l + 2;
    y = _insets.t + 2;  // 1;
    w = 22;
    h = _h - _insets.t - _insets.b - 4;  // -3;  // 14

    if (_state == BUTTON_DOWN) { ++x; ++y; }

    DrawRectangle(_blackGC, x, y, w - 1, h - 1);
    FillRectangle(_drawGC->GetGC(), x + 1, y + 1, w - 2, h - 2);

    // separator

    x = _w - 6 - _insets.r - 6;
    y = _insets.t + 1;
    h = _h - _insets.b - 1;  // actually y1

    if (_state == BUTTON_DOWN) { ++x; ++y; }

    DrawLine(_shadowGC,  x, y, x, h - 2);
    DrawLine(_hilightGC, x + 1, y, x + 1, h - 1);
    DrawLine(_hilightGC, x, h - 1, x + 1, h - 1);

    // arrow

    x = _w - 6 - _insets.r - 2;
    y = (_h - 4) / 2 + 1;

    if (_state == BUTTON_DOWN) { ++x; ++y; }

    _DrawTriangle(_blackGC, x, y);

  } else {

    // sunken rectangle

    x = _insets.l + 2;
    y = _insets.t + 2;  // 1;
    w = 22;
    h = _h - _insets.t - _insets.b - 4;  // 3;

    _Draw3dRectangle(SUNKEN_FRAME, x, y, w, h);

    // separator

    x = _w - 6 - _insets.r - 6;
    y = _insets.t + 1;
    h = _h - _insets.b - 1;  // actually y1

    DrawLine(_shadowGC,  x, y, x, h - 2);
    DrawLine(_hilightGC, x + 1, y, x + 1, h - 1);
    DrawLine(_hilightGC, x, h - 1, x + 1, h - 1);

    // sunken arrow

    x = _w - 6 - _insets.r - 2;
    y = (_h - 4) / 2 + 1;

    _DrawTriangle(_hilightGC, x + 1, y + 1);
    _DrawTriangle(_shadowGC, x, y);

  }
}

void OXColorSelect::_DrawTriangle(GC gc, int x, int y) {
  XPoint points[3];

  points[0].x = x;
  points[0].y = y;
  points[1].x = x + 5;
  points[1].y = y;
  points[2].x = x + 2;
  points[2].y = y + 3;

  XFillPolygon(GetDisplay(), _id, gc, points, 3, Convex, CoordModeOrigin);
}

void OXColorSelect::SetColor(unsigned long color) {
  _color.SetColor(_client, color);
  _client->FreeColor(_pixel);
  _pixel = _client->GetColor(_color);
  _drawGC->SetForeground(_pixel);
  NeedRedraw();
}

void OXColorSelect::SetColor(OColor color) {
  _color = color;
  _client->FreeColor(_pixel);
  _pixel = _client->GetColor(_color);
  _drawGC->SetForeground(_pixel);
  NeedRedraw();
}
