/*************************************************************************

    This file is part of xclass, a Win95-looking GUI toolkit.
    Copyright (C) 1996, 1997 David Barth, Hector Peraza.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

**************************************************************************/

#include <xclass/utils.h>
#include <xclass/OXButton.h>
#include <xclass/OXClient.h>
#include <xclass/OResourcePool.h>
#include <xclass/OGC.h>


const OXGC *OXButton::_hibckgndGC = 0;
int OXButton::_init = False;


//-----------------------------------------------------------------

OXButton::OXButton(const OXWindow *p, int ID, unsigned int options)
  : OXFrame(p, 1, 1, options) {
    
    if (!_init) {
      XGCValues gval;
      unsigned long gmask;

      gmask = GCForeground | GCBackground | GCTile |
              GCFillStyle  | GCGraphicsExposures;
      gval.foreground = GetResourcePool()->GetFrameHiliteColor();
      gval.background = _backPixel;
      gval.fill_style = FillTiled;
      gval.tile = GetResourcePool()->GetCheckeredPixmap();
      gval.graphics_exposures = False;
      _hibckgndGC = _client->GetGC(GetId(), gmask, &gval);

      _init = True;
    }

    _widgetID = ID;
    _widgetType = "OXButton";
    _msgObject = p;

    _prevstate = _state = BUTTON_UP;
    _default = _click = _down = False;
    _type = BUTTON_NORMAL;

    XGrabButton(GetDisplay(), Button1, AnyModifier, _id, False,
		ButtonPressMask | ButtonReleaseMask |
		EnterWindowMask | LeaveWindowMask,
		GrabModeAsync, GrabModeAsync, None, None);

//    AddInput(KeyPressMask | KeyReleaseMask | FocusChangeMask);
    AddInput(FocusChangeMask);
}

void OXButton::SetState(int state) {

  if (state != _state) {
    switch (state) {
    case BUTTON_ENGAGED:
    case BUTTON_DOWN:
       _options &= ~RAISED_FRAME;
       _options |= SUNKEN_FRAME;
       break;

    case BUTTON_UP:
       _options &= ~SUNKEN_FRAME;
       _options |= RAISED_FRAME;
       break;
    }
    _state = state;
    NeedRedraw(True);
  }
}

int OXButton::HandleButton(XButtonEvent *event) {
  
  OXFrame::HandleButton(event);
  if (!IsEnabled()) return True;

  if (event->button != Button1) return False;

  if ((event->type == ButtonPress) && TakesFocus())
    RequestFocus();
 
  switch (_type) {
  default:
  case BUTTON_NORMAL:
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
	  OButtonMessage message(MSG_BUTTON, MSG_CLICK, _widgetID);
          SendMessage(_msgObject, &message);
        }
      }
    }
    break;

  case BUTTON_STAYDOWN:
    if (event->type == ButtonPress) {
      _click = True;
      if (_state != BUTTON_DOWN) {
        _prevstate = _state;
        /*if (_state == BUTTON_UP)*/ SetState(BUTTON_DOWN);
      }
    } else {
      _click = False;
      if (_state != _prevstate) {
        if (_state == BUTTON_DOWN) {
          SetState(BUTTON_ENGAGED);
	  OButtonMessage message(MSG_BUTTON, MSG_CLICK, _widgetID);
          SendMessage(_msgObject, &message);
        }
      }
    }
    break;

  case BUTTON_ONOFF:
    if (event->type == ButtonPress) {
      _click = True;
      if (_state != BUTTON_DOWN) {
        _prevstate = _state;
        SetState(BUTTON_DOWN);
      }
    } else {
      _click = False;
      if (_state != _prevstate) {
        if (_prevstate == BUTTON_UP) {
          SetState(BUTTON_ENGAGED);
          OButtonMessage message(MSG_BUTTON, MSG_CLICK, _widgetID);
          SendMessage(_msgObject, &message);
        } else if (_prevstate == BUTTON_ENGAGED) {
          SetState(BUTTON_UP);
          OButtonMessage message(MSG_BUTTON, MSG_CLICK, _widgetID);
          SendMessage(_msgObject, &message);
        }
      }
    }
    break;
  }

  return True;
}

int OXButton::HandleCrossing(XCrossingEvent *event) {

  OXFrame::HandleCrossing(event);
  if (!IsEnabled()) return True;
  if (!_click) return True;

  switch (_type) {
  default:
  case BUTTON_NORMAL:
    if (event->type == LeaveNotify) {
      SetState(_prevstate);
    } else {
      SetState(BUTTON_DOWN);
    }
    break;

  case BUTTON_STAYDOWN:
  case BUTTON_ONOFF:
    if (event->type == LeaveNotify) {
      SetState(_prevstate);
    } else {
      if (_prevstate != BUTTON_ENGAGED) SetState(BUTTON_DOWN);
    }
    break;
  }

  return True;
}

void OXButton::DrawBorder() {
  int x = 0, y = 0, w = _w, h = _h;

  if (_default) {
    DrawRectangle(_blackGC, 0, 0, _w-1, _h-1);
    ++x; ++y; --w; --w; --h; --h;
  }

  switch (_options & (SUNKEN_FRAME | RAISED_FRAME | DOUBLE_BORDER)) {
  case SUNKEN_FRAME | DOUBLE_BORDER:
    DrawLine(_blackGC,   x,     y,     x+w-1, y);
    DrawLine(_blackGC,   x,     y,     x,     y+h-1);
    DrawLine(_shadowGC,  x+1,   y+1,   x+w-3, y+1);
    DrawLine(_shadowGC,  x+1,   y+1,   x+1,   y+h-3);

    DrawLine(_hilightGC, x+1,   y+h-1, x+w-1, y+h-1);
    DrawLine(_hilightGC, x+w-1, y+h-1, x+w-1, y+1);
    DrawLine(_bckgndGC,  x+2,   y+h-2, x+w-2, y+h-2);
    DrawLine(_bckgndGC,  x+w-2, y+2,   x+w-2, y+h-2);

    // 3-st button DrawLine(_hilightGC, 2, 2, _w-3, 2);
    break;

  default:
    //OXFrame::DrawBorder();
    _Draw3dRectangle(_options & (SUNKEN_FRAME | RAISED_FRAME | DOUBLE_BORDER),
                     x, y, w, h);
    break;
  }
}

void OXButton::Reconfig() {
  XGCValues gval;
  unsigned long gmask;

  OXFrame::Reconfig();

  gmask = GCForeground | GCBackground | GCTile |
          GCFillStyle | GCGraphicsExposures;
  gval.foreground = GetResourcePool()->GetFrameHiliteColor();
  gval.background = _backPixel;
  gval.fill_style = FillTiled;
  gval.tile = GetResourcePool()->GetCheckeredPixmap();
  gval.graphics_exposures = False;

  _client->FreeGC((OXGC *) _hibckgndGC);
  _hibckgndGC = _client->GetGC(GetId(), gmask, &gval);
}

void OXButton::_GotFocus() {
  OXFrame::_GotFocus();
  AddInput(KeyPressMask | KeyReleaseMask);
  NeedRedraw(False);
}

void OXButton::_LostFocus() {
  OXFrame::_LostFocus();
  RemoveInput(KeyPressMask | KeyReleaseMask);
  NeedRedraw(True);
}

void OXButton::_Enable(int onoff) {
  OXFrame::_Enable(onoff);
  NeedRedraw();
}

void OXButton::SetDefault(int onoff) {
  if (_default != onoff) {
    if (onoff) ++_bw; else --_bw;
    _default = onoff;
    NeedRedraw();
  }
}
