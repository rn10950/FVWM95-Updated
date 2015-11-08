/**************************************************************************

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
#include <xclass/OResourcePool.h>
#include <xclass/OXCompositeFrame.h>
#include <xclass/OXWidget.h>
#include <xclass/OXScrollBar.h>
#include <xclass/OXButton.h>
#include <xclass/OPicture.h>
#include <xclass/OXClient.h>
#include <xclass/OString.h>

#include <X11/cursorfont.h>
#include <X11/keysym.h>

#include "icons/arrow-left.xpm"
#include "icons/arrow-right.xpm"
#include "icons/arrow-up.xpm"
#include "icons/arrow-down.xpm"


#define IN_RANGE(x, min, max)   \
        if (x < min) {          \
          x = min;              \
        } else if (x > max) {   \
          x = max;              \
        }


Pixmap OXScrollBar::_bckgndPixmap;
const OPicture *OXScrollBar::_arrow_left;
const OPicture *OXScrollBar::_arrow_right;
const OPicture *OXScrollBar::_arrow_up;   
const OPicture *OXScrollBar::_arrow_down; 
int OXScrollBar::_sb_width;
int OXScrollBar::_init = False;


//-----------------------------------------------------------------

void OXScrollBarElt::SetState(int state) {

  if (state != _state) {
    switch (state) {
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
    _client->NeedRedraw(this);
  }
}

void OXScrollBarElt::DrawBorder() {
  switch (_options & (SUNKEN_FRAME | RAISED_FRAME)) {
  case SUNKEN_FRAME: // pressed
    DrawLine(_blackGC, 0, 0, _w-2, 0);
    DrawLine(_blackGC, 0, 0, 0, _h-2);
    DrawLine(_shadowGC,  1, 1, _w-3, 1);
    DrawLine(_shadowGC,  1, 1, 1, _h-3);

    DrawLine(_whiteGC,  0, _h-1, _w-1, _h-1);
    DrawLine(_whiteGC,  _w-1, _h-1, _w-1, 1);
    DrawLine(_bckgndGC,  1, _h-2, _w-2, _h-2);
    DrawLine(_bckgndGC,  _w-2, _h-2, _w-2, 2);

    if (_pic != NULL) {
      int x = (_w - _pic->GetWidth()) >> 1;
      int y = (_h - _pic->GetHeight()) >> 1;
      _pic->Draw(_client->GetDisplay(), _id, _bckgndGC, x+1, y+1); // 3, 3
    }
    break;

  case RAISED_FRAME: // normal
    DrawLine(_bckgndGC, 0, 0, _w-2, 0);
    DrawLine(_bckgndGC, 0, 0, 0, _h-2);
    DrawLine(_hilightGC, 1, 1, _w-3, 1);
    DrawLine(_hilightGC, 1, 1, 1, _h-3);
    
    DrawLine(_shadowGC,  1, _h-2, _w-2, _h-2);
    DrawLine(_shadowGC,  _w-2, _h-2, _w-2, 1);
    DrawLine(_blackGC, 0, _h-1, _w-1, _h-1);
    DrawLine(_blackGC, _w-1, _h-1, _w-1, 0);

    if (_pic != NULL) {
      int x = (_w - _pic->GetWidth()) >> 1;
      int y = (_h - _pic->GetHeight()) >> 1;
      _pic->Draw(_client->GetDisplay(), _id, _bckgndGC, x, y); // 2, 2
    }
    break;

  default:
    break;
  }
}

void OXScrollBarElt::Reconfig() {
  _backPixel = _defaultFrameBackground;
  OXFrame::Reconfig();
  SetBackgroundColor(_backPixel);
}

//-----------------------------------------------------------------

OXScrollBar::OXScrollBar(const OXWindow *p, int w, int h,
                         unsigned int options, unsigned long back) :
  OXFrame(p, w, h, options | OWN_BKGND, back) {

    if (!_init) {
      _bckgndPixmap = _client->GetResourcePool()->GetCheckeredPixmap();

      _arrow_left  = _client->GetPicture("arrow-left.xpm", XCP_arrow_left);
      _arrow_right = _client->GetPicture("arrow-right.xpm", XCP_arrow_right);
      _arrow_up    = _client->GetPicture("arrow-up.xpm", XCP_arrow_up);
      _arrow_down  = _client->GetPicture("arrow-down.xpm", XCP_arrow_down);

      _sb_width = SB_WIDTH;

      _init = True;
    }

    _widgetID = -1;
    _widgetType = "OXScrollBar";
    _msgObject = p;

    _mode = SB_NORMAL;
    _incr = 1;
    _preDelay = SB_DF_PREDELAY;  // 400 (500)
    _rptDelay = SB_DF_RPTDELAY;  // 50  (100)
    _rpt = NULL;

    SetBackgroundPixmap(_bckgndPixmap);
}

OXScrollBar::~OXScrollBar() {
  if (_rpt) delete _rpt;
}

void OXScrollBar::SetDelay(int pre, int repeat) {
  _preDelay = pre;
  _rptDelay = repeat;
}

void OXScrollBar::Reconfig() {
  OXFrame::Reconfig();
  _bckgndPixmap = _client->GetResourcePool()->GetCheckeredPixmap();
  SetBackgroundPixmap(_bckgndPixmap);
  _head->Reconfig();
  _tail->Reconfig();
  _slider->Reconfig();
}

//-----------------------------------------------------------------

// some of the common code could be moved to the abstract
// base class functions...

OXHScrollBar::OXHScrollBar(const OXWindow *p, int w, int h,
                           unsigned int options, unsigned long back) :
  OXScrollBar(p, w, h, options, back) {

    _widgetType = "OXHScrollBar";

    _headpic = _arrow_left;
    _tailpic = _arrow_right;

    if (!_headpic || !_tailpic)
      FatalError("OXHScrollBar: missing required pixmaps");

    _head   = new OXScrollBarElt(this, _headpic, _sb_width, _sb_width,
                                 RAISED_FRAME);
    _tail   = new OXScrollBarElt(this, _tailpic, _sb_width, _sb_width,
                                 RAISED_FRAME);
    _slider = new OXScrollBarElt(this, NULL, _sb_width, 50,
                                 RAISED_FRAME);

    XGrabButton(GetDisplay(), Button1, AnyModifier, _id, False,
                ButtonPressMask | ButtonReleaseMask |
                PointerMotionMask,
                GrabModeAsync, GrabModeAsync, None, None);

    _dragging = False;
    _x0 = _y0 = (_sb_width = max(_sb_width, 5));
    _pos = 0;

    _range = max(h - (_sb_width << 1), 1);
    _psize = _range >> 1;
    
    _slsize = 50;
    _slrange = 1;
}
    
OXHScrollBar::~OXHScrollBar() {
  delete _head;
  delete _tail;
  delete _slider;
  if (_rpt) { delete _rpt; _rpt = NULL; }
}
    
void OXHScrollBar::Layout() {

  // should recalculate _slsize, _slrange, _x0, _y0, etc. too...

  _head->MoveResize(0, 0, _sb_width, _sb_width);
  _tail->MoveResize(_w-_sb_width, 0, _sb_width, _sb_width);
  _slider->MoveResize(_x0, 0, 50, _sb_width);
}

int OXHScrollBar::HandleButton(XButtonEvent *event) {
  if (event->type == ButtonPress) {
    
    if (event->subwindow == _slider->GetId()) {
      _xp = event->x - _x0;
      _yp = event->y - _y0;
      _dragging = True;
  
    } else {

      if (!_rpt) {
        _rpt = new OTimer(this, _preDelay);
        _subw = event->subwindow;
      }

      int action;

      if (event->subwindow == _head->GetId()) {
        _head->SetState(BUTTON_DOWN);
        _pos -= _incr;
        action = MSG_LINEUP;
      } else if (event->subwindow == _tail->GetId()) {
        _tail->SetState(BUTTON_DOWN);
        _pos += _incr;
        action = MSG_LINEDOWN;
      } else if (event->x > _sb_width && event->x < _x0) {
        _pos -= _psize;
        action = MSG_PAGEUP;
      } else if (event->x > _x0 && event->x < _w-_sb_width) {
        _pos += _psize;
        action = MSG_PAGEDOWN;
      }
   
      IN_RANGE(_pos, 0, _range - _psize);

      _x0 = _sb_width + _pos * _slrange / max(_range-_psize, 1);
    
      IN_RANGE(_x0, _sb_width, _sb_width + _slrange);

      _slider->Move(_x0, 0);
  
      OScrollBarMessage message1(MSG_HSCROLL, action, _widgetID, _pos);
      SendMessage(_msgObject, &message1);
      OScrollBarMessage message2(MSG_HSCROLL, MSG_SLIDERTRACK, _widgetID, _pos);
      SendMessage(_msgObject, &message2);

    }
  } else {
    _head->SetState(BUTTON_UP);
    _tail->SetState(BUTTON_UP);

    if (_rpt) { delete _rpt; _rpt = NULL; }

    _dragging = False;
    _incr = 1;

    IN_RANGE(_pos, 0, _range - _psize);
  
    OScrollBarMessage message(MSG_HSCROLL, MSG_SLIDERPOS, _widgetID, _pos);
    SendMessage(_msgObject, &message);
  }     

  return True;
}
        
int OXHScrollBar::HandleMotion(XMotionEvent *event) {
  if (_dragging) {
    _x0 = event->x - _xp;
    _y0 = event->y - _yp;

    IN_RANGE(_x0, _sb_width, _sb_width + _slrange);

    _slider->Move(_x0, 0);
    _pos = (_x0 - _sb_width) * (_range-_psize) / _slrange;

    IN_RANGE(_pos, 0, _range - _psize);

    OScrollBarMessage message(MSG_HSCROLL, MSG_SLIDERTRACK, _widgetID, _pos);
    SendMessage(_msgObject, &message);
  }

  return True;
}

int OXHScrollBar::HandleTimer(OTimer *t) {
  XButtonEvent ev;
  Window dummy;

  if (t != _rpt) return False;

  delete _rpt;
  _rpt = new OTimer(this, _rptDelay);

  if (_mode == SB_ACCELERATED) {
    ++_incr;
    if (_incr > 100) _incr = 100;
  }

  ev.type = ButtonPress;

  XQueryPointer(GetDisplay(), _id, &dummy, &ev.subwindow,
                &ev.x_root, &ev.y_root, &ev.x, &ev.y,
                &ev.state);

  HandleButton(&ev);

  return True;
}

void OXHScrollBar::SetRange(int range, int page_size, int sendMsg) {

  _range = max(range, 1);
  _psize = max(page_size, 0);
      
  _slsize = max(_psize * (_w - (_sb_width << 1)) / _range, 6);
  _slsize = min(_slsize, _w - (_sb_width << 1));

  _slrange = max(_w - (_sb_width << 1) - _slsize, 1);

  _x0 = _sb_width + _pos * _slrange / max(_range-_psize, 1);
  IN_RANGE(_x0, _sb_width, _sb_width + _slrange);

  _slider->MoveResize(_x0, 0, _slsize, _sb_width);

  //---//

//  _pos = (_x0 - _sb_width) * (_range-_psize) / _slrange;

  //---// 

  IN_RANGE(_pos, 0, _range - _psize);

  if (sendMsg) {
    OScrollBarMessage message(MSG_HSCROLL, MSG_SLIDERPOS, _widgetID, _pos);
    SendMessage(_msgObject, &message);
  }
}

void OXHScrollBar::SetPosition(int pos, int sendMsg) {

  _pos = pos;
  IN_RANGE(_pos, 0, _range - _psize);

  _x0 = _sb_width + _pos * _slrange / max(_range-_psize, 1);
  IN_RANGE(_x0, _sb_width, _sb_width + _slrange);

  _slider->MoveResize(_x0, 0, _slsize, _sb_width);

  //---//

  if (sendMsg) {
    OScrollBarMessage message(MSG_HSCROLL, MSG_SLIDERPOS, _widgetID, _pos);
    SendMessage(_msgObject, &message);
  }
}

//-----------------------------------------------------------------

OXVScrollBar::OXVScrollBar(const OXWindow *p, int w, int h,
                           unsigned int options, unsigned long back) :
  OXScrollBar(p, w, h, options, back) {

    _widgetType = "OXVScrollBar";

    _headpic = _arrow_up;
    _tailpic = _arrow_down;
  
    if (!_headpic || !_tailpic)
      FatalError("OXHScrollBar: missing required pixmaps");

    _head   = new OXScrollBarElt(this, _headpic, _sb_width, _sb_width,
                                 RAISED_FRAME);
    _tail   = new OXScrollBarElt(this, _tailpic, _sb_width, _sb_width,
                                 RAISED_FRAME);
    _slider = new OXScrollBarElt(this, NULL, _sb_width, 50,
                                 RAISED_FRAME);
    
    XGrabButton(GetDisplay(), Button1, AnyModifier, _id, False,
                ButtonPressMask | ButtonReleaseMask |
                PointerMotionMask,
                GrabModeAsync, GrabModeAsync, None, None);

    _dragging = False;
    _x0 = _y0 = (_sb_width = max(_sb_width, 5));
    _pos = 0;

    _range = max(h - (_sb_width << 1), 1);
    _psize = _range >> 1;
    
    _slsize = 50;
    _slrange = 1;
}

OXVScrollBar::~OXVScrollBar() {
  delete _head;
  delete _tail;
  delete _slider;
  if (_rpt) { delete _rpt; _rpt = NULL; }
}

void OXVScrollBar::Layout() {

  // should recalculate _slsize, _slrange, _x0, _y0, etc. too...

  _head->MoveResize(0, 0, _sb_width, _sb_width);
  _tail->MoveResize(0, _h-_sb_width, _sb_width, _sb_width);
  _slider->MoveResize(0, _y0, _sb_width, 50);
}
  
int OXVScrollBar::HandleButton(XButtonEvent *event) {
  if (event->type == ButtonPress) {
    
    if (event->subwindow == _slider->GetId()) {
      _xp = event->x - _x0;
      _yp = event->y - _y0;
      _dragging = True;
  
    } else {  
      
      if (!_rpt) {
        _rpt = new OTimer(this, _preDelay);
        _subw = event->subwindow;
      }

      int action;

      if (event->subwindow == _head->GetId()) {
        _head->SetState(BUTTON_DOWN);
        _pos -= _incr;
        action = MSG_LINEUP;
      } else if (event->subwindow == _tail->GetId()) {
        _tail->SetState(BUTTON_DOWN);
        _pos += _incr;
        action = MSG_LINEDOWN;
      } else if (event->y > _sb_width && event->y < _y0) {
        _pos -= _psize;
        action = MSG_PAGEUP;
      } else if (event->y > _y0 && event->y < _h-_sb_width) {
        _pos += _psize;
        action = MSG_PAGEDOWN;
      }
        
      IN_RANGE(_pos, 0, _range - _psize);
  
      _y0 = _sb_width + _pos * _slrange / max(_range-_psize, 1);
        
      IN_RANGE(_y0, _sb_width, _sb_width + _slrange);

      _slider->Move(0, _y0);
        
      OScrollBarMessage message1(MSG_VSCROLL, action, _widgetID, _pos);
      SendMessage(_msgObject, &message1);
      OScrollBarMessage message2(MSG_VSCROLL, MSG_SLIDERTRACK, _widgetID, _pos);
      SendMessage(_msgObject, &message2);
    }
  } else {
    _head->SetState(BUTTON_UP);
    _tail->SetState(BUTTON_UP);

    if (_rpt) { delete _rpt; _rpt = NULL; }

    _dragging = False;
    _incr = 1;

    IN_RANGE(_pos, 0, _range - _psize);
  
    OScrollBarMessage message(MSG_VSCROLL, MSG_SLIDERPOS, _widgetID, _pos);
    SendMessage(_msgObject, &message);
  }     

  return True;
}

int OXVScrollBar::HandleMotion(XMotionEvent *event) {
  if (_dragging) {
    _x0 = event->x - _xp;
    _y0 = event->y - _yp;
      
    IN_RANGE(_y0, _sb_width, _sb_width + _slrange);
    _slider->Move(0, _y0);
    _pos = (_y0 - _sb_width) * (_range-_psize) / _slrange;

    IN_RANGE(_pos, 0, _range - _psize);

    OScrollBarMessage message(MSG_VSCROLL, MSG_SLIDERTRACK, _widgetID, _pos);
    SendMessage(_msgObject, &message);
  }

  return True;
}

int OXVScrollBar::HandleTimer(OTimer *t) {
  XButtonEvent ev;
  Window dummy;

  if (t != _rpt) return False;

  delete _rpt;
  _rpt = new OTimer(this, _rptDelay);

  if (_mode == SB_ACCELERATED) {
    ++_incr;
    if (_incr > 100) _incr = 100;
  }

  ev.type = ButtonPress;

  XQueryPointer(GetDisplay(), _id, &dummy, &ev.subwindow,
                &ev.x_root, &ev.y_root, &ev.x, &ev.y,
                &ev.state);

  HandleButton(&ev);

  return True;
}

void OXVScrollBar::SetRange(int range, int page_size, int sendMsg) {

  _range = max(range, 1);
  _psize = max(page_size, 0);

  _slsize = max(_psize * (_h - (_sb_width << 1)) / _range, 6);
  _slsize = min(_slsize, _h - (_sb_width << 1));

  _slrange = max(_h - (_sb_width << 1) - _slsize, 1);

  _y0 = _sb_width + _pos * _slrange / max(_range-_psize, 1);
  IN_RANGE(_y0, _sb_width, _sb_width + _slrange);

  _slider->MoveResize(0, _y0, _sb_width, _slsize);
    
  //---//
 
//  _pos = (_y0 - _sb_width) * (_range-_psize) / _slrange;

  //---// 
      
  IN_RANGE(_pos, 0, _range - _psize);
    
  if (sendMsg) {
    OScrollBarMessage message(MSG_VSCROLL, MSG_SLIDERPOS, _widgetID, _pos);
    SendMessage(_msgObject, &message);
  }
}

void OXVScrollBar::SetPosition(int pos, int sendMsg) {

  _pos = pos;
  IN_RANGE(_pos, 0, _range - _psize);

  _y0 = _sb_width + _pos * _slrange / max(_range-_psize, 1);
  IN_RANGE(_y0, _sb_width, _sb_width + _slrange);

  _slider->MoveResize(0, _y0, _sb_width, _slsize);

  //---//

  if (sendMsg) {
    OScrollBarMessage message(MSG_VSCROLL, MSG_SLIDERPOS, _widgetID, _pos);
    SendMessage(_msgObject, &message);
  }
}
