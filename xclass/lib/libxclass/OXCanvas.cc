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
#include <xclass/OXClient.h>
#include <xclass/OXWidget.h>
#include <xclass/OXCanvas.h>


//-----------------------------------------------------------------

OXCanvas::OXCanvas(const OXWindow *p, int w, int h,
                   unsigned int options, unsigned long back) :
  OXCompositeFrame(p, w, h, options, back) {

    SetLayoutManager(NULL);

    _vport = new OXViewPort(this, w-4, h-4, CHILD_FRAME | OWN_BKGND,
                            _defaultDocumentBackground);
    _hscrollbar = new OXHScrollBar(this, w-4, SB_WIDTH);
    _vscrollbar = new OXVScrollBar(this, SB_WIDTH, h-4);

    _scrolling = CANVAS_SCROLL_BOTH;

    _hscrollbar->Associate(this);
    _vscrollbar->Associate(this);

    OXCompositeFrame::AddFrame(_hscrollbar, NULL);
    OXCompositeFrame::AddFrame(_vscrollbar, NULL);
    OXCompositeFrame::AddFrame(_vport, NULL);

#if 0
    XGrabButton(GetDisplay(), Button4, AnyModifier, _id, True/*False*/,
                ButtonPressMask | ButtonReleaseMask,
                GrabModeAsync, GrabModeAsync, None, None);

    XGrabButton(GetDisplay(), Button5, AnyModifier, _id, True/*False*/,
                ButtonPressMask | ButtonReleaseMask,
                GrabModeAsync, GrabModeAsync, None, None);
#else
    AddInput(ButtonPressMask);
#endif
}
           
OXCanvas::~OXCanvas() {
}

int OXCanvas::HandleButton(XButtonEvent *event) {
  if (event->type == ButtonPress) {
    int amount, ch;

    ch = _vport->GetHeight();
    amount = max(ch / 6, 1);

    if (event->state & ShiftMask)
      amount = 1;
    else if (event->state & ControlMask)
      amount = ch - max(ch / 20, 1);

    if (event->button == Button4) {
      SetVPos(-_vport->GetVPos() - amount);
      return True;
    } else if (event->button == Button5) {
      SetVPos(-_vport->GetVPos() + amount);
      return True;
    }

  }
  return False;
}

void OXCanvas::AddFrame(OXFrame *f, OLayoutHints *l) {
  ((OXCompositeFrame *)(_vport->GetContainer()))->AddFrame(f, l);
}

void OXCanvas::Layout() {
  OXFrame *_container;
  int need_vsb, need_hsb, cw, ch, tcw, tch;

  need_vsb = need_hsb = False;

  _container = (OXFrame *) _vport->GetContainer();    

  // test whether we need scrollbars

  cw = _w - _insets.l - _insets.r;
  ch = _h - _insets.t - _insets.b;

  _container->SetWidth(cw);
  _container->SetHeight(ch);

  if (_container->GetDefaultWidth() > cw) {
    if (_scrolling & CANVAS_SCROLL_HORIZONTAL) {
      need_hsb = True;
      ch = _h - _insets.t - _insets.b - _vscrollbar->GetDefaultWidth(); 
      _container->SetHeight(ch);
    }
  }

  if (_container->GetDefaultHeight() > ch) {
    if (_scrolling & CANVAS_SCROLL_VERTICAL) {
      need_vsb = True;
      cw = _w - _insets.l - _insets.r - _hscrollbar->GetDefaultHeight();
      _container->SetWidth(cw);
    }
  }

  // re-check again (putting the scrollbar
  // could have changed things)

  if (_container->GetDefaultWidth() > cw) {
    if (!need_hsb) {
      if (_scrolling & CANVAS_SCROLL_HORIZONTAL) {
        need_hsb = True;
        ch = _h - _insets.t - _insets.b - _vscrollbar->GetDefaultWidth();
        _container->SetHeight(ch);
      }
    }
  }

  if (need_hsb) {
    _hscrollbar->MoveResize(_insets.l, ch + _insets.t,
                            cw, _hscrollbar->GetDefaultHeight());
    _hscrollbar->MapWindow();
  } else {
    _hscrollbar->UnmapWindow();
    _hscrollbar->SetPosition(0);
  }

  if (need_vsb) {
    _vscrollbar->MoveResize(cw + _insets.l, _insets.t,
                            _vscrollbar->GetDefaultWidth(), ch);
    _vscrollbar->MapWindow();
  } else {
    _vscrollbar->UnmapWindow();
    _vscrollbar->SetPosition(0);
  }

  _vport->MoveResize(_insets.l, _insets.t, cw, ch);

  tcw = max(_container->GetDefaultWidth(), cw);
  tch = max(_container->GetDefaultHeight(), ch);
  _container->SetHeight(0); // force a resize in OXFrame::Resize
  _container->Resize(tcw, tch);

  _hscrollbar->SetRange(_container->GetWidth(), _vport->GetWidth());
  _vscrollbar->SetRange(_container->GetHeight(), _vport->GetHeight());
}

int OXCanvas::ProcessMessage(OMessage *msg) {
  OScrollBarMessage *sbmsg;

  switch (msg->type) {
    case MSG_HSCROLL:
      switch (msg->action) {
        case MSG_SLIDERTRACK:
        case MSG_SLIDERPOS:
          sbmsg = (OScrollBarMessage *) msg;
          _vport->SetHPos(-sbmsg->pos);
          break;
      }
      break;

    case MSG_VSCROLL:
      switch (msg->action) {
        case MSG_SLIDERTRACK:
        case MSG_SLIDERPOS:
          sbmsg = (OScrollBarMessage *) msg;
          _vport->SetVPos(-sbmsg->pos);
          break;
      }
      break;

    default:
      break;
   
  }

  return True;
}

void OXCanvas::SetScrolling(int scrolling) {
  if (scrolling != _scrolling) {
    _scrolling = scrolling;
    Layout();
  }
}


//-----------------------------------------------------------------

OXViewPort::OXViewPort(const OXWindow *p, int w, int h,
                       unsigned int options, unsigned long back) :   
  OXCompositeFrame(p, w, h, options, back) {

    _container = NULL;
    _x0 = _y0 = 0;
    MapSubwindows();
}

void OXViewPort::SetContainer(OXFrame *f) { 
  if (_container) {
    RemoveFrame(_container);
    _container->DestroyWindow();
    delete _container;
  }
  _container = f;
  if (_container) AddFrame(_container, NULL);
}
