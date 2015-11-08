/**************************************************************************

    This file is part of xclass.
    Copyright (C) 1997, Harald Radke.

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

#include <X11/X.h>
#include <X11/cursorfont.h>

#include <xclass/OXCompositeFrame.h>
#include <xclass/OXResizer.h>

#include "icons/VmaskResizer.xbm"
#include "icons/VResizer.xbm"
#include "icons/HmaskResizer.xbm"
#include "icons/HResizer.xbm"


OXHorizontalResizer::OXHorizontalResizer(const OXWindow *p, int wid,
		                         unsigned long resMode) :
  OXFrame(p, 4, 2, FIXED_HEIGHT) {
    Pixmap curPix, maskPix;

    _widgetID = wid;
    _widgetType = "OXResizer";
    _msgObject = p;

    _active  = RESIZER_ACTIVE;
    _resMode = resMode;
    _remoteResizer = NULL;
    _prev = _next = NULL;

    XSetWindowAttributes wattr;
    unsigned long mask;

    mask = CWSaveUnder;
    wattr.save_under = True;
    
    XChangeWindowAttributes(GetDisplay(), _id, mask, &wattr);

    int hspotx = 16, hspoty = 16;

    curPix  = XCreateBitmapFromData(GetDisplay(), GetId(), HResizer_bits,
                                    HResizer_width, HResizer_height);
    maskPix = XCreateBitmapFromData(GetDisplay(), GetId(), HmaskResizer_bits,
                                    HmaskResizer_width, HmaskResizer_height);

    XColor black, white;
    Colormap defColormap = _client->GetDefaultColormap();

    XParseColor(GetDisplay(), defColormap, "black", &black);
    XParseColor(GetDisplay(), defColormap, "white", &white);

    Cursor defaultCursor = XCreatePixmapCursor(GetDisplay(), curPix, maskPix,
					 &black, &white, hspotx, hspoty);

    DefineCursor(defaultCursor);

    XGrabButton(GetDisplay(), AnyButton, AnyModifier, _id, False,
                ButtonPressMask | ButtonReleaseMask | Button1MotionMask,
                GrabModeAsync, GrabModeAsync, None, None);

    if (resMode & RESIZER_USE_HANDLE) {
      _handle = new OXResizerHandle(_client, p, this);
      _handle->DefineCursor(defaultCursor);
      _handle->Resize(GetWidth(), 4);
      hx = hy = 0;
    } else {
      _handle = NULL;
    }

    XFreePixmap(GetDisplay(), curPix);
    XFreePixmap(GetDisplay(), maskPix);

    if (_h > 4) Resize(GetWidth(), 4);
}

OXHorizontalResizer::~OXHorizontalResizer() {
  if (_handle) delete _handle;
}

void OXHorizontalResizer::MoveResizeHandle(int x, int y, int w, int h) {
  if (_handle) _handle->MoveResize(_x + (hx = x), _y + (hy = y), w, h);
}

int OXHorizontalResizer::HandleButton(XButtonEvent *event) {

  if (_handle && (event->window == _handle->GetId())) {
    event->x += hx;
    event->y += hy;
  }

  if (_active != RESIZER_ACTIVE) return True;
  if (_remoteResizer) _remoteResizer->HandleButton(event);

  if (event->button == Button1) {
    if (event->type == ButtonPress) {
      if (_resMode & RESIZER_HIDDEN) {
        if (((OXFrame *)_parent)->IsComposite())
          ((OXCompositeFrame *) _parent)->ShowFrame(this);
        else
          OXFrame::MapWindow();
	RaiseWindow();
      }
      px = event->x;
      py = _y;
      _state = RESIZER_PRESSED;
    } else {
      if (!(_resMode & RESIZER_OPAQUE)) {
	if (_prev)
	  _prev->Resize(_prev->GetWidth(), _prev->GetHeight() + (_y - py));
	if (_next) {
	  _next->Resize(_next->GetWidth(), _next->GetHeight() - (_y - py));
	  _next->Move(_next->GetX(), _next->GetY() + (_y - py));
	}
//	((OXCompositeFrame *) _parent)->Layout(); // hmmm...because of OXListBox
      }
      if (_resMode & RESIZER_HIDDEN) {
	if (((OXFrame *)_parent)->IsComposite())
	  ((OXCompositeFrame *) _parent)->HideFrame(this);
        else
          OXFrame::UnmapWindow();
	LowerWindow();
        if (_handle) _handle->MapRaised();
      }

      OResizerMessage message(MSG_HRESIZER, MSG_SLIDERPOS, _widgetID,
                              _prev->GetHeight());
      SendMessage(_msgObject, &message);

      _state = RESIZER_RELEASED;
    }
    NeedRedraw();
  }
  return True;
}

int OXHorizontalResizer::HandleMotion(XMotionEvent *event) {

  if (_handle && (event->window == _handle->GetId())) {
    event->x += hx;
    event->y += hy;
  }

  if (_active != RESIZER_ACTIVE) return True;

  if (((event->y < 0) && (_min1 < _y + event->y)) ||
      ((event->y > 0) && _next && (_y + event->y < _next->GetY() + _next->GetHeight() - _min2))) {
    Motion(event->x, event->y);

    OResizerMessage message(MSG_HRESIZER, MSG_SLIDERTRACK, _widgetID,
                            _prev->GetHeight());
    SendMessage(_msgObject, &message);
  }

  return True;
}

void OXHorizontalResizer::Motion(int xmot, int ymot) {

  if (_active != RESIZER_ACTIVE) return;
  if (_remoteResizer) _remoteResizer->Motion(xmot, ymot);
  if (_state == RESIZER_RELEASED) return;

  int y = _y + ymot;

  if (_resMode & RESIZER_OPAQUE) {
    if ((_prev) && (y < _prev->GetY())) {
      _prev->Resize(_prev->GetWidth(), 0);
      Move(_x, _prev->GetY());
      if (_next) {
	_next->Resize(_next->GetWidth(),
	  _next->GetHeight() + (_next->GetY() - (GetY() + GetHeight())));
	_next->Move(_next->GetX(), GetHeight() + GetY());
      }
      return;
    }
    if ((_next) && (y + GetHeight() > _next->GetY() + _next->GetHeight())) {
      _next->Move(_next->GetX(), _next->GetY() + _next->GetHeight());
      _next->Resize(_next->GetWidth(), 0);
      Move(_x, _next->GetY() - GetHeight());
      if (_prev)
	_prev->Resize(_prev->GetWidth(), _y - _prev->GetY());
      return;
    }
    if (_prev)
      _prev->Resize(_prev->GetWidth(), _prev->GetHeight() + ymot);
    if (_next) {
      _next->Resize(_next->GetWidth(), _next->GetHeight() - ymot);
      _next->Move(_next->GetX(), _next->GetY() + ymot);
    }
  } else {
    if ((_prev) && (y < _prev->GetY()))
      y = _prev->GetY();
    if ((_next) && (y + GetHeight() > _next->GetY() + _next->GetHeight()))
      y = _next->GetY() + _next->GetHeight() - GetHeight();
  }
  Move(_x, y);
//  DrawBorder();
  return;
}

void OXHorizontalResizer::SetNext(OXFrame *next, int min) {
  _min2 = min;
  _next = next;
  if (_resMode & RESIZER_HIDDEN) {
    if (((OXFrame *)_parent)->IsComposite())
      ((OXCompositeFrame *) _parent)->HideFrame(this);
    LowerWindow();
  } else {
    RaiseWindow();
  }
}

void OXHorizontalResizer::DrawBorder() {

  // Draw the horizontal split

  if (_state == RESIZER_PRESSED) {
    FillRectangle(_shadowGC, 1, 0, _w-2, 2);
  } else if (_state == RESIZER_RELEASED) {
    FillRectangle(_bckgndGC, 1, 0, _w-2, 2);
  }
}

void OXHorizontalResizer::Move(int x, int y) {
  if (_handle) _handle->Move(x + hx, y + hy);
  OXFrame::Move(x, y);
}

void OXHorizontalResizer::Resize(int w, int h) {
  if (_handle) _handle->Resize(max(w, 1), max(h, 1));
  OXFrame::Resize(w, h);
}

void OXHorizontalResizer::MoveResize(int x, int y, int w, int h) {
  if (_handle) _handle->MoveResize(x + hx, y + hy, max(w, 1), max(h, 1));;
  OXFrame::MoveResize(x, y, w, h);
}

void OXHorizontalResizer::MapWindow() {
  if (!(_resMode & RESIZER_HIDDEN)) OXFrame::MapWindow();
  if (_handle) _handle->MapRaised();
}

void OXHorizontalResizer::UnmapWindow() {
  OXFrame::UnmapWindow();
  if (_handle) _handle->UnmapWindow();
}

void OXHorizontalResizer::RaiseWindow() {
  OXFrame::RaiseWindow();
  if (_handle) _handle->RaiseWindow();
}

void OXHorizontalResizer::LowerWindow() {
  if (_handle) _handle->LowerWindow();
  OXFrame::LowerWindow();
}

void OXHorizontalResizer::MapRaised() {
  if (!(_resMode & RESIZER_HIDDEN)) OXFrame::MapRaised();
  if (_handle) _handle->MapRaised();
}


//----------------------------------------------------------------------

OXVerticalResizer::OXVerticalResizer(const OXWindow *p, int wid,
		                     unsigned long resMode) :
  OXFrame(p, 2, 4, FIXED_WIDTH) {
    Pixmap curPix, maskPix;

    _widgetID = wid;
    _widgetType = "OXResizer";  
    _msgObject = p;
                                
    _active = RESIZER_ACTIVE;
    _resMode = resMode;
    _remoteResizer = NULL;
    _prev = _next = NULL;

    XSetWindowAttributes wattr;
    unsigned long mask;

    mask = CWSaveUnder;
    wattr.save_under = True;
    
    XChangeWindowAttributes(GetDisplay(), _id, mask, &wattr);

    int hspotx = 16, hspoty = 16;

    curPix = XCreateBitmapFromData(GetDisplay(), GetId(), VResizer_bits,
                                   VResizer_width, VResizer_height);
    maskPix = XCreateBitmapFromData(GetDisplay(), GetId(), VmaskResizer_bits,
                                    VmaskResizer_width, VmaskResizer_height);

    XColor black, white;
    Colormap defColormap = _client->GetDefaultColormap();
    XParseColor(GetDisplay(), defColormap, "black", &black);
    XParseColor(GetDisplay(), defColormap, "white", &white);

    Cursor defaultCursor = XCreatePixmapCursor(GetDisplay(), curPix, maskPix,
                                               &black, &white, hspotx, hspoty);

    DefineCursor(defaultCursor);

    XGrabButton(GetDisplay(), AnyButton, AnyModifier, _id, False,
                ButtonPressMask | ButtonReleaseMask | Button1MotionMask,
                GrabModeAsync, GrabModeAsync, None, None);

    if (resMode & RESIZER_USE_HANDLE) {
      _handle = new OXResizerHandle(_client, p, this);
      _handle->DefineCursor(defaultCursor);
      _handle->Resize(4, GetHeight());
      hx = hy = 0;
    } else {
      _handle = NULL;
    }

    XFreePixmap(GetDisplay(), curPix);
    XFreePixmap(GetDisplay(), maskPix);

    if (_w > 4) Resize(4, GetHeight());
}

OXVerticalResizer::~OXVerticalResizer() {
  if (_handle) delete _handle;
}

void OXVerticalResizer::MoveResizeHandle(int x, int y, int w, int h) {
  if (_handle) _handle->MoveResize(_x + (hx = x), _y + (hy = y), w, h);
}

int OXVerticalResizer::HandleButton(XButtonEvent *event) {

  if (_handle && event->window == _handle->GetId())
    event->x += hx; event->y += hy;

  if (_active != RESIZER_ACTIVE) return True;
  if (_remoteResizer) _remoteResizer->HandleButton(event);

  if (event->button == Button1) {
    if (event->type == ButtonPress) {
      if (_resMode & RESIZER_HIDDEN) {
	if (((OXFrame *)_parent)->IsComposite())
	  ((OXCompositeFrame *) _parent)->ShowFrame(this);
        else
          OXFrame::MapWindow();
	RaiseWindow();
      }
      px = _x;
      py = event->y;
      _state = RESIZER_PRESSED;
    } else {
      if (!(_resMode & RESIZER_OPAQUE)) {
	if (_prev)
	  _prev->Resize(_prev->GetWidth() + (_x - px), _prev->GetHeight());
	if (_next) {
	  _next->Resize(_next->GetWidth() - (_x - px), _next->GetHeight());
	  _next->Move(_next->GetX() + (_x - px), _next->GetY());
	}
//	((OXCompositeFrame *) _parent)->Layout();	// hmmm...because of OXListBox
      }
      if (_resMode & RESIZER_HIDDEN) {
	if (((OXFrame *)_parent)->IsComposite())
	  ((OXCompositeFrame *) _parent)->HideFrame(this);
        else
          OXFrame::UnmapWindow();
	LowerWindow();
        if (_handle) _handle->MapRaised();
      }

      OResizerMessage message(MSG_VRESIZER, MSG_SLIDERPOS, _widgetID,
                              _prev->GetWidth());
      SendMessage(_msgObject, &message);

      _state = RESIZER_RELEASED;
    }
    NeedRedraw();
  }
  return True;
}

int OXVerticalResizer::HandleMotion(XMotionEvent *event) {
  if (_active != RESIZER_ACTIVE) return True;

  if (_handle && event->window == _handle->GetId())
    event->x += hx; event->y += hy;

  if (((event->x < 0) && (_min1 < _x + event->x)) ||
      ((event->x > 0) && _next && (_x + event->x < _next->GetX() + _next->GetWidth() - _min2))) {
    Motion(event->x, event->y);

    OResizerMessage message(MSG_VRESIZER, MSG_SLIDERTRACK, _widgetID,
                            _prev->GetWidth());
    SendMessage(_msgObject, &message);
  }

  return True;
}

void OXVerticalResizer::Motion(int xmot, int ymot) {

  if (_active != RESIZER_ACTIVE) return;
  if (_remoteResizer) _remoteResizer->Motion(xmot, ymot);
  if (_state == RESIZER_RELEASED) return;

  int x = _x + xmot;

  if (_resMode & RESIZER_OPAQUE) {
    if ((_prev) && (x < _prev->GetX())) {
      _prev->Resize(0, _prev->GetHeight());
      Move(_prev->GetX(), _y);
      if (_next) {
	_next->Resize(_next->GetWidth() + (_next->GetX() - (GetX() + GetWidth())),
		      _next->GetHeight());
	_next->Move(GetWidth() + GetX(), _next->GetY());
      }
      return;
    }
    if ((_next) && (x + GetWidth() > _next->GetX() + _next->GetWidth())) {
      _next->Move(_next->GetX() + _next->GetWidth(), _next->GetY());
      _next->Resize(0, _next->GetHeight());
      Move(_next->GetX() - GetWidth(), _y);
      if (_prev)
	_prev->Resize(_x - _prev->GetX(), _prev->GetHeight());
      return;
    }
    if (_prev)
      _prev->Resize(_prev->GetWidth() + xmot, _prev->GetHeight());
    if (_next) {
      _next->Resize(_next->GetWidth() - xmot, _next->GetHeight());
      _next->Move(_next->GetX() + xmot, _next->GetY());
    }
  } else {
    if ((_prev) && (x < _prev->GetX())) {
      x = _prev->GetX();
    }
    if ((_next) && (x + GetWidth() > _next->GetX() + _next->GetWidth())) {
      x = _next->GetX() + _next->GetWidth() - GetWidth();
    }
  }

  Move(x, _y);
//  DrawBorder();

  return;
}

void OXVerticalResizer::SetNext(OXFrame *next, int min) {
  _min2 = min;
  _next = next;
  if (_resMode & RESIZER_HIDDEN) {
    if (((OXFrame *)_parent)->IsComposite())
      ((OXCompositeFrame *) _parent)->HideFrame(this);
    LowerWindow();
  } else {
    RaiseWindow();
  }
}

void OXVerticalResizer::DrawBorder() {

  // Draw the vertical split

  if (_state == RESIZER_PRESSED) {
    FillRectangle(_shadowGC, 0, 1, 2, _h-2);
  } else if (_state == RESIZER_RELEASED) {
    FillRectangle(_bckgndGC, 0, 1, 2, _h-2);
  }
}

void OXVerticalResizer::Move(int x, int y) {
  if (_handle) _handle->Move(x + hx, y + hy);
  OXFrame::Move(x, y);
}

void OXVerticalResizer::Resize(int w, int h) {
  if (_handle) _handle->Resize(max(w, 1), max(h, 1));
  OXFrame::Resize(w, h);
}

void OXVerticalResizer::MoveResize(int x, int y, int w, int h) {
  if (_handle) _handle->MoveResize(x + hx, y + hy, max(w, 1), max(h, 1));;
  OXFrame::MoveResize(x, y, w, h);
}

void OXVerticalResizer::MapWindow() {
  if (!(_resMode & RESIZER_HIDDEN)) OXFrame::MapWindow();
  if (_handle) _handle->MapRaised();
}

void OXVerticalResizer::UnmapWindow() {
  OXFrame::UnmapWindow();
  if (_handle) _handle->UnmapWindow();
}

void OXVerticalResizer::RaiseWindow() {
  OXFrame::RaiseWindow();
  if (_handle) _handle->RaiseWindow();
}

void OXVerticalResizer::LowerWindow() {
  if (_handle) _handle->LowerWindow();
  OXFrame::LowerWindow();
}

void OXVerticalResizer::MapRaised() {
  if (!(_resMode & RESIZER_HIDDEN)) OXFrame::MapRaised();
  if (_handle) _handle->MapRaised();
}


//----------------------------------------------------------------------

OXResizerHandle::OXResizerHandle(OXClient *c, const OXWindow *p, 
                                 OXFrame *res) :
  OXFrame(c, None, (OXWindow *) p) {

    _id = XCreateWindow(GetDisplay(), p->GetId(), _x, _y, 10, 10, 0, 
                        CopyFromParent, InputOnly, CopyFromParent,
                        0, NULL);

    _client->RegisterWindow(this);

    XGrabButton(GetDisplay(), AnyButton, AnyModifier, _id, False,
                ButtonPressMask | ButtonReleaseMask | Button1MotionMask,
                GrabModeAsync, GrabModeAsync, None, None);

    _resizer = res;
}
