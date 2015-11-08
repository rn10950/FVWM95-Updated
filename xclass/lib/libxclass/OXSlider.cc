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
#include <xclass/OPicture.h>
#include <xclass/OXSlider.h>

#include "icons/slider1h.xpm"
#include "icons/slider1v.xpm"
#include "icons/slider2h.xpm"
#include "icons/slider2v.xpm"


//---------------------------------------------------------------------

OXSlider::OXSlider(const OXWindow *p, int w, int h, int type, int ID,
              unsigned int options, unsigned long back) :
  OXFrame(p, w, h, options, back) {

    _widgetID = ID;
    _widgetType = "OXSlider";
    _msgObject = p;

    _type = type;
    _scale = 10;
}
 


//---------------------------------------------------------------------

OXVSlider::OXVSlider(const OXWindow *p, int h, int type, int ID,
                           unsigned int options, unsigned long back) :
  OXSlider(p, SLIDER_WIDTH, h, type, ID, options, back) {

    _widgetType = "OXVSlider";

    if (_type & SLIDER_1)
      _sliderpic = _client->GetPicture("slider1h.xpm", XCP_slider1h);
    else
      _sliderpic = _client->GetPicture("slider2h.xpm", XCP_slider2h);

    if (!_sliderpic)
      FatalError("OXVSlider: missing required pixmaps");

    XGrabButton(GetDisplay(), AnyButton, AnyModifier, _id, False,
                ButtonPressMask | ButtonReleaseMask |
                PointerMotionMask,
                GrabModeAsync, GrabModeAsync, None, None);

    // set initial values
    _pos = h/2; _vmin = 0; _vmax = h;

}

OXVSlider::~OXVSlider() {
  _client->FreePicture(_sliderpic);
}

void OXVSlider::_DoRedraw() {

  OXFrame::_DoRedraw();

  DrawLine(_shadowGC,  _w/2,   8,    _w/2-1, 8);
  DrawLine(_shadowGC,  _w/2-1, 8,    _w/2-1, _h-9);
  DrawLine(_hilightGC, _w/2+1, 8,    _w/2+1, _h-8);
  DrawLine(_hilightGC, _w/2+1, _h-8, _w/2,   _h-8);
  DrawLine(_blackGC,   _w/2,   9,    _w/2,   _h-9);

  // check scale
  if (_scale == 1) _scale++;
  if (_scale * 2 > _h) _scale = 0;
  if (_scale > 0 && !(_type & SCALE_NO)) {
    int lines = (_h-16) / _scale;
    int remain = (_h-16) % _scale;
    for (int i = 0; i <= lines; i++) {
      int y = i * _scale + (i * remain) / lines;
      DrawLine(_blackGC, _w/2+8, y+7, _w/2+10, y+7);
      if ((_type & SLIDER_2) && (_type && SCALE_BOTH))
	DrawLine(_blackGC, _w/2-9, y+7, _w/2-11, y+7);
    }
  }
  if (_pos < _vmin) _pos = _vmin;
  if (_pos > _vmax) _pos = _vmax;

  // calc slider-picture position
  _relpos = ((_h-16) * (_pos - _vmin)) / (_vmax - _vmin) + 8;
  _sliderpic->Draw(_client->GetDisplay(), _id, _bckgndGC, _w/2-7, _relpos-6);
}

int OXVSlider::HandleButton(XButtonEvent *event) {
  if (event->type == ButtonPress) {
    if (event->y >= _relpos - 7 && event->y <= _relpos + 7) {
      // slider selected
      _dragging = True;
      _yp = event->y - (_relpos-7);
    } else {
      if (event->button == Button1) {
	// scroll up or down
	int m = (_vmax - _vmin) / (_h-16);
	if (event->y < _relpos) {
	  _pos -= ((m) ? m : 1);
	}
	if (event->y > _relpos) {
	  _pos += ((m) ? m : 1);
	}
      } else if (event->button == Button2) {
	// set absolute position
	_pos = ((_vmax - _vmin) * event->y) / (_h-16) + _vmin;
      }
      OSliderMessage message(MSG_VSLIDER, MSG_SLIDERPOS, _widgetID, _pos);
      SendMessage(_msgObject, &message);
    }
    NeedRedraw();
  } else {
    // ButtonRelease
    _dragging = False;
  }
  return True;
}

int OXVSlider::HandleMotion(XMotionEvent *event) {
  if (_dragging) {
    int old = _pos;
    _pos = ((_vmax - _vmin) * (event->y - _yp)) / (_h-16) + _vmin;
    if (_pos > _vmax) _pos = _vmax;
    if (_pos < _vmin) _pos = _vmin;

    // check if position changed
    if (old != _pos) {
      NeedRedraw();
      OSliderMessage message(MSG_VSLIDER, MSG_SLIDERPOS, _widgetID, _pos);
      SendMessage(_msgObject, &message);
    }
  }
  return True;
}


//---------------------------------------------------------------------

OXHSlider::OXHSlider(const OXWindow *p, int w, int type, int ID,
                           unsigned int options, unsigned long back) :
  OXSlider(p, w, SLIDER_HEIGTH, type, ID, options, back) {

    _widgetType = "OXHSlider";

    if (_type & SLIDER_1)
      _sliderpic = _client->GetPicture("slider1v.xpm", XCP_slider1v);
    else
      _sliderpic = _client->GetPicture("slider2v.xpm", XCP_slider2v);
    
    if (!_sliderpic)
      FatalError("OXHSlider: missing required pixmaps");

    XGrabButton(GetDisplay(), AnyButton, AnyModifier, _id, False,
                ButtonPressMask | ButtonReleaseMask |
                PointerMotionMask,
                GrabModeAsync, GrabModeAsync, None, None);

    // set initial values
    _pos = w/2; _vmin = 0; _vmax = w;
}

OXHSlider::~OXHSlider() {
  _client->FreePicture(_sliderpic);
}

void OXHSlider::_DoRedraw() {

  OXFrame::_DoRedraw();

  DrawLine(_shadowGC,  8,    _h/2,   8,    _h/2-1);
  DrawLine(_shadowGC,  8,    _h/2-1, _w-9, _h/2-1);
  DrawLine(_hilightGC, 8,    _h/2+1, _w-8, _h/2+1);
  DrawLine(_hilightGC, _w-8, _h/2+1, _w-8, _h/2);
  DrawLine(_blackGC,   9,    _h/2,   _w-9, _h/2);

  if (_scale == 1) _scale++;
  if (_scale * 2 > _w) _scale = 0;
  if (_scale > 0 && !(_type & SCALE_NO)) {
    int lines = (_w-16) / _scale;
    int remain = (_w-16) % _scale;
    for (int i = 0; i <= lines; i++) {
      int x = i * _scale + (i * remain) / lines;
      DrawLine(_blackGC, x+7, _h/2+8, x+7, _h/2+10);
      if ((_type & SLIDER_2) && (_type && SCALE_BOTH))
	DrawLine(_blackGC, x+7, _h/2-9, x+7, _h/2-11);
    }
  }
  if (_pos < _vmin) _pos = _vmin;
  if (_pos > _vmax) _pos = _vmax;

  // calc slider-picture position
  _relpos = ((_w-16) * (_pos - _vmin)) / (_vmax - _vmin) + 8;
  _sliderpic->Draw(_client->GetDisplay(), _id, _bckgndGC, _relpos-6, _h/2-7);
}

int OXHSlider::HandleButton(XButtonEvent *event) {
  if (event->type == ButtonPress) {
    if (event->x >= _relpos - 7 && event->x <= _relpos + 7) {
      // slider selected
      _dragging = True;
      _xp = event->x - (_relpos-7);
    } else {
      if (event->button == Button1) {
	int m = (_vmax - _vmin) / (_w-16);
	if (event->x < _relpos) {
	  _pos -= ((m) ? m : 1);
	}
	if (event->x > _relpos) {
	  _pos += ((m) ? m : 1);
	}
      } else if (event->button == Button2) {
	_pos = ((_vmax - _vmin) * event->x) / (_w-16) + _vmin;
      }
      OSliderMessage message(MSG_HSLIDER, MSG_SLIDERPOS, _widgetID, _pos);
      SendMessage(_msgObject, &message);
    }
    NeedRedraw();
  } else {
    // ButtonRelease
    _dragging = False;
  }
  return True;
}

int OXHSlider::HandleMotion(XMotionEvent *event) {
  if (_dragging) {
    int old = _pos;
    _pos = ((_vmax - _vmin) * (event->x - _xp)) / (_w-16) + _vmin;
    if (_pos > _vmax) _pos = _vmax;
    if (_pos < _vmin) _pos = _vmin;

    // check if position changed
    if (old != _pos) {
      NeedRedraw();
      OSliderMessage message(MSG_HSLIDER, MSG_SLIDERPOS, _widgetID, _pos);
      SendMessage(_msgObject, &message);
    }
  }
  return True;
}
