/**************************************************************************

    This file is part of xclass.
    Copyright (C) 1996-2000 Harald Radke, Hector Peraza.

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

#include <xclass/utils.h>
#include <xclass/OPicture.h>
#include <xclass/OXView.h>


//#define FLUSH_EXPOSE

//----------------------------------------------------------------------

OXViewCanvas::OXViewCanvas(const OXView *p, int w, int h,
			   unsigned int options) : 
  OXFrame(p, w, h, options) {

    SetBackgroundColor(_defaultDocumentBackground);
    _clearBgnd = False;
    AddInput(KeyPressMask | EnterWindowMask | LeaveWindowMask |
             FocusChangeMask);
};


//----------------------------------------------------------------------

OXView::OXView(const OXWindow *p, int w, int h, int id,
	       unsigned int options, unsigned int sboptions) :
  OXCompositeFrame(p, w, h, options), OXWidget(id, "OXView") {

    SetLayoutManager(NULL);

    _docBgndColor = _defaultDocumentBackground;

    _canvas = new OXViewCanvas(this, 10, 10, CHILD_FRAME | OWN_BKGND);
    _sboptions = sboptions;

    _hsb = new OXHScrollBar(this, 10, 10, CHILD_FRAME);
    _hsb->Associate(this);

    _vsb = new OXVScrollBar(this, 10, 10, CHILD_FRAME);
    _vsb->Associate(this);

    AddFrame(_canvas, NULL);
    AddFrame(_hsb, NULL);
    AddFrame(_vsb, NULL);

    XGCValues gcv;
    int mask;

    const OPicture *bckgnd_pix = GetResourcePool()->GetDocumentBckgndPicture();
    if (bckgnd_pix) {
      mask = GCTile | GCFillStyle | GCGraphicsExposures;
      _canvas->SetBackgroundPixmap(bckgnd_pix->GetPicture());
      gcv.tile = bckgnd_pix->GetPicture();
      gcv.fill_style = FillTiled;
      gcv.graphics_exposures = True;
    } else{
      mask = GCForeground | GCBackground;
      gcv.foreground = _defaultDocumentBackground;
      _canvas->SetBackgroundColor(gcv.foreground);
    }
    _backGC = XCreateGC(GetDisplay(), _id, mask, &gcv);

    _offset.x = _offset.y = 0;

    _scrollValue.x = 1;
    _scrollValue.y = 1;
    _clearExposedArea = False;
    _clearBgnd = False;
    _exposedRegion.empty();

    Clear();

    AddInput(ButtonPressMask | ButtonReleaseMask);
}

OXView::~OXView() {
}

void OXView::Clear() {
  _visibleStart = OPosition(0, 0);
  UpdateBackgroundStart();
  _virtualSize = ODimension(0, 0);
  _canvas->ClearArea(0, 0, _canvas->GetWidth(), _canvas->GetHeight(), False);
  Layout();
}

void OXView::SetupBackgroundPic(const OPicture *pic) {
  XGCValues gcv;
  unsigned int mask;

  if (!pic) {
    mask = GCForeground | GCFillStyle | GCGraphicsExposures;
    gcv.fill_style = FillSolid;
    gcv.foreground = _docBgndColor;
    gcv.graphics_exposures = True;
    _canvas->SetBackgroundPixmap(None);
    _canvas->SetBackgroundColor(gcv.foreground);
  } else {
    mask = GCTile | GCFillStyle | GCGraphicsExposures;
    gcv.tile = pic->GetPicture();
    gcv.fill_style = FillTiled;
    gcv.graphics_exposures = True;
    _canvas->SetBackgroundPixmap(pic->GetPicture());
  }

  XChangeGC(GetDisplay(), _backGC, mask, &gcv);

  NeedRedraw(ORectangle(_visibleStart, _canvas->GetSize()));
}

void OXView::SetupBackgroundColor(unsigned long color) {
  XGCValues gcv;
  unsigned int mask;

  mask = GCForeground;
  gcv.foreground = _docBgndColor = color;
  _canvas->SetBackgroundColor(gcv.foreground);

  XChangeGC(GetDisplay(), _backGC, mask, &gcv);

  NeedRedraw(ORectangle(_visibleStart, _canvas->GetSize()));
}

void OXView::UpdateBackgroundStart() {
  XGCValues gcv;
  unsigned int mask = GCTileStipXOrigin | GCTileStipYOrigin;

  gcv.ts_x_origin = -_visibleStart.x;
  gcv.ts_y_origin = -_visibleStart.y;
  XChangeGC(GetDisplay(), _backGC, mask, &gcv);
}

int OXView::HandleButton(XButtonEvent *event) {
  if (event->type == ButtonPress) {
    int amount, ch;

    ch = _canvas->GetHeight();
    if (_scrollValue.y == 1)
      amount = _scrollValue.y * max(ch / 6, 1);
    else
      amount = _scrollValue.y * 5;

    if (event->state & ShiftMask)
      amount = _scrollValue.y;
    else if (event->state & ControlMask)
      amount = ch - max(ch / 20, 1);

    if (event->button == Button4) {
      ScrollDown(amount);
      return True;
    } else if (event->button == Button5) {
      ScrollUp(amount);
      return True;
    }

  }
  return False;
}

int OXView::HandleExpose(XExposeEvent *event) {
  if (event->window == _canvas->GetId())
#ifndef FLUSH_EXPOSE
    NeedRedraw(ORectangle(ToVirtual(OPosition(event->x, event->y)),
               ODimension(event->width, event->height)));
#else
    DrawRegion(ToVirtual(OPosition(event->x, event->y)),
               ODimension(event->width, event->height),
               _clearExposedArea);
#endif
  else
    OXFrame::HandleExpose(event);

  return True;
}

int OXView::HandleGraphicsExpose(XGraphicsExposeEvent *event) {
  if (event->drawable == _canvas->GetId())
#ifndef FLUSH_EXPOSE
    NeedRedraw(ORectangle(ToVirtual(OPosition(event->x, event->y)),
               ODimension(event->width, event->height)));
#else
    DrawRegion(ToVirtual(OPosition(event->x, event->y)),
               ODimension(event->width, event->height),
               _clearExposedArea);
#endif
  else
    OXFrame::HandleGraphicsExpose(event);

  return True;
}

void OXView::_DoRedraw() {
  OXFrame::_DoRedraw();
  if (!_exposedRegion.is_empty()) {
    DrawRegion(_exposedRegion.left_top(), _exposedRegion.size(),
               _clearExposedArea);
    _exposedRegion.empty();
  }
}

void OXView::NeedRedraw(ORectangle area) {
  if (_exposedRegion.is_empty())
    _exposedRegion = area;
  else
    _exposedRegion.merge(area);
  OXWindow::NeedRedraw(False);
}

int OXView::DrawRegion(OPosition coord, ODimension size, int clear) {

  OPosition endpt = ToPhysical(OPosition(_virtualSize.w, _virtualSize.h));

  // must clear anyway any space remaining after the visible end
  // of the virtual window

  if (endpt.x < _canvas->GetWidth())
    _canvas->FillRectangle(_backGC, endpt.x, 0,
                           _canvas->GetWidth() - endpt.x,
                           endpt.y);

  if (endpt.y < _canvas->GetHeight())
    _canvas->FillRectangle(_backGC, 0, endpt.y,
                           _canvas->GetWidth(),
                           _canvas->GetHeight() - endpt.y);

  if (clear) {
    _canvas->FillRectangle(_backGC,
                           ToPhysical(coord).x, ToPhysical(coord).y,
                           size.w, size.h);
  }

  return True;
}

int OXView::ProcessMessage(OMessage *msg) {
  OScrollBarMessage *sbmsg = (OScrollBarMessage *) msg;

  switch (msg->type) {
  case MSG_HSCROLL:
    switch (msg->action) {
    case MSG_SLIDERTRACK:
    case MSG_SLIDERPOS:
      if (sbmsg->pos >= 0)
	Scroll(OPosition(sbmsg->pos * _scrollValue.x, _visibleStart.y));
      break;
    }
    break;

  case MSG_VSCROLL:
    switch (msg->action) {
    case MSG_SLIDERTRACK:
    case MSG_SLIDERPOS:
      if (sbmsg->pos >= 0)
	Scroll(OPosition(_visibleStart.x, sbmsg->pos * _scrollValue.y));
      break;
    }
    break;
  }

  return True;
}

//#define INCL_SB

void OXView::Layout() {
  int need_vsb, need_hsb, cw, ch;

  need_vsb = need_hsb = False;

  // test whether we need scrollbars
  cw = _w - _insets.l - _insets.r - _offset.x;
  ch = _h - _insets.t - _insets.b - _offset.y;

  _canvas->SetWidth(cw);
  _canvas->SetHeight(ch);
  ItemLayout();

  if (_virtualSize.w > cw) {
    if (_sboptions & CANVAS_SCROLL_HORIZONTAL) {
      need_hsb = True;
      ch -= _vsb->GetDefaultWidth();
      _canvas->SetHeight(ch);
      ItemLayout();
    }
  }

  if (_virtualSize.h > ch) {
    if (_sboptions & CANVAS_SCROLL_VERTICAL) {
      need_vsb = True;
      cw -= _hsb->GetDefaultHeight();
      _canvas->SetWidth(cw);
      ItemLayout();
    }
  }

  // re-check again (putting the scrollbar could have changed things)

  if (_virtualSize.w > cw) {
    if (!need_hsb) {
      if (_sboptions & CANVAS_SCROLL_HORIZONTAL) {
	need_hsb = True;
	ch -= _vsb->GetDefaultWidth();
	_canvas->SetHeight(ch);
        ItemLayout();
      }
    }
  }

  if (need_hsb) {
#ifdef INCL_SB
    _hsb->MoveResize(_insets.l, ch + _insets.t + _offset.y,
                     cw + _offset.x, _hsb->GetDefaultHeight());
    _hsb->MapRaised();
#else
    _hsb->MoveResize(_insets.l + _offset.x, ch + _insets.t + _offset.y,
                     cw, _hsb->GetDefaultHeight());
    _hsb->MapWindow();
#endif
  } else {
    _hsb->UnmapWindow();
    _hsb->SetPosition(0 /*, False*/);
  }

  if (need_vsb) {
#ifdef INCL_SB
    _vsb->MoveResize(cw + _insets.l + _offset.x, _insets.t,
                     _vsb->GetDefaultWidth(), ch + _offset.y);
    _vsb->MapRaised();
#else
    _vsb->MoveResize(cw + _insets.l + _offset.x, _insets.t + _offset.y,
                     _vsb->GetDefaultWidth(), ch);
    _vsb->MapWindow();
#endif
  } else {
    _vsb->UnmapWindow();
    _vsb->SetPosition(0 /*, False*/);
  }

  _canvas->MoveResize(_insets.l + _offset.x, _insets.t + _offset.y, cw, ch);

  _hsb->SetRange(_virtualSize.w / _scrollValue.x,
		 _canvas->GetWidth() / _scrollValue.x/*, False*/);
  _vsb->SetRange(_virtualSize.h / _scrollValue.y,
		 _canvas->GetHeight() / _scrollValue.y/*, False*/);
}

void OXView::DrawBorder() {

  switch (_options & (SUNKEN_FRAME | RAISED_FRAME | DOUBLE_BORDER)) {
  case SUNKEN_FRAME | DOUBLE_BORDER:
    DrawLine(_shadowGC, 0, 0, _w - 2, 0);
    DrawLine(_shadowGC, 0, 0, 0, _h - 2);
    DrawLine(_blackGC, 1, 1, _w - 3, 1);
    DrawLine(_blackGC, 1, 1, 1, _h - 3);

    DrawLine(_hilightGC, 0, _h - 1, _w - 1, _h - 1);
    DrawLine(_hilightGC, _w - 1, _h - 1, _w - 1, 0);
    DrawLine(_bckgndGC, 1, _h - 2, _w - 2, _h - 2);
    DrawLine(_bckgndGC, _w - 2, 1, _w - 2, _h - 2);
    break;

  default:
    OXFrame::DrawBorder();
    break;
  }
}

void OXView::ScrollToPosition(OPosition pos) {
  if (pos.x != _hsb->GetPosition()) _hsb->SetPosition(pos.x / _scrollValue.x);
  if (pos.y != _vsb->GetPosition()) _vsb->SetPosition(pos.y / _scrollValue.y);
}

void OXView::Scroll(OPosition newTop) {

  // well, if no change, no senseless calculations
  if ((newTop.x == _visibleStart.x) && (newTop.y == _visibleStart.y))
    return;

  // all objects start at zero, no negative coords!
  if ((newTop.x < 0) || (newTop.y < 0))
    return;

  // areas that have to be redrawn (in object coords)
  OPosition vRedrawStart(0, 0);
  OPosition hRedrawStart(0, 0);
  ODimension vRedrawSize(0, 0);
  ODimension hRedrawSize(0, 0);

  bool hNeedsRedraw = False;
  bool vNeedsRedraw = False;

  // old and new coordinates of the displayed object portion,
  // in screen coords for all calculations of screen areas.
  // Though the standard relation objectCoord <-> screenCoord
  // only differs by an offset but are both in pixels, subclassed
  // view instances might use other object measurements

  OPosition scrVisibleStart = ToPhysical(_visibleStart);
  OPosition scrNewTop = ToPhysical(newTop);

  // Screen geometry for the copy task (copy as much displayed data
  // as possible to reduce slower(??) DrawRegion() ohverhead
  // scrolling calculation is done in two steps, on the same data,
  // so copying and redrawing is done in the end and not twice

  OPosition copySource = scrVisibleStart;
  OPosition copyDest = scrVisibleStart;
  ODimension copySize = _canvas->GetSize();

  vRedrawStart = newTop;
  vRedrawSize = _canvas->GetSize();
  hRedrawStart = newTop;
  hRedrawSize = _canvas->GetSize();

  // check if scrolling makes it necessary to redraw the whole visible area
  // or if it is possible to save time by copying parts of the window contents

  if ((newTop.y + _canvas->GetHeight() < _visibleStart.y)
      || (newTop.y > _visibleStart.y + _canvas->GetHeight())
      || (newTop.x + _canvas->GetWidth() < _visibleStart.x)
      || (newTop.x > _visibleStart.x + _canvas->GetWidth())) {
    vNeedsRedraw = True;	// complete redraw, one redraw is enough

  } else {

    // This is the part for any change along the vertical axis

    if (newTop.y != _visibleStart.y) {
      if (newTop.y < _visibleStart.y) {	// scrolling up...
	copyDest.y += (scrVisibleStart.y - scrNewTop.y);
	copySize.h -= (scrVisibleStart.y - scrNewTop.y);
      } else {			//...and scrolling down
	copySource.y += (scrNewTop.y - scrVisibleStart.y);
	copySize.h -= (scrNewTop.y - scrVisibleStart.y);
	vRedrawStart = OPosition(newTop.x, newTop.y + copySize.h);
      }
      vRedrawSize.h -= copySize.h;

      // if redrawing is also necessary in horizontal direction,
      // avoid redrawing the intersecting area twice...

      hRedrawSize.h -= vRedrawSize.h;
      vNeedsRedraw = True;
    }

    // Scrolling along the horizontal axis

    if (newTop.x != _visibleStart.x) {
      if (newTop.x < _visibleStart.x) {	// left scrolling...
	copyDest.x += (scrVisibleStart.x - scrNewTop.x);
	copySize.w -= (scrVisibleStart.x - scrNewTop.x);
      } else {			// ...and right scrolling
	copySource.x += (scrNewTop.x - scrVisibleStart.x);
	copySize.w -= (scrNewTop.x - scrVisibleStart.x);
	hRedrawStart = OPosition(newTop.x + copySize.w, newTop.y);
      }
      hRedrawSize.w -= copySize.w;
      hNeedsRedraw = True;
    }
  }

  // now the new coordinates, the upper left corner of the visible
  // object portion is applied 

  _visibleStart = newTop;
  UpdateBackgroundStart();

  // copy as much of the window contents as possible to the appropriate 
  // position. This is faster, but only works if the new visibleStart
  // was inside/near the visible area (smaller scrolling)

  _canvas->CopyWindowArea(_backGC, copySource.x, copySource.y,
                          copySize.w, copySize.h, copyDest.x, copyDest.y);

  // We have to redraw the areas of the screen where formerly invisible
  // parts of the object are now shown. If scrolling was caused by user input
  // (via scrollbars, etc.), mostly there is only one area that has to be
  // redrawn, depending on the particualr axis along which was scrolled.
  // Nevertheless it is possible to call ScrollToPos() directly from the
  // code, therefore scrolling can take place in both directions...

  if (vNeedsRedraw) DrawRegion(vRedrawStart, vRedrawSize, _clearExposedArea);
  if (hNeedsRedraw) DrawRegion(hRedrawStart, hRedrawSize, _clearExposedArea);

#ifdef FLUSH_EXPOSE
  XEvent event;

  XSync(GetDisplay(), False);
  while (XCheckTypedWindowEvent(GetDisplay(), _id,
                                GraphicsExpose, &event)) {
    HandleGraphicsExpose((XGraphicsExposeEvent *) &event);
    if (event.xgraphicsexpose.count == 0) break;
  }
#endif
}
