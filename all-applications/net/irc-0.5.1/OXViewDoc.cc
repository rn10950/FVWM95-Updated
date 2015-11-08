#include <stdio.h>

#include <xclass/utils.h>
#include <xclass/OResourcePool.h>
#include <xclass/OXFont.h>
#include <xclass/OGC.h>

#include "OXViewDoc.h"
#include "OXPreferences.h"

#define ALWAYS_SCROLLED 1

extern OSettings *foxircSettings;


//----------------------------------------------------------------------

OXViewDocFrame::OXViewDocFrame(OXWindow *parent, int x, int y,
                         unsigned int options, unsigned long back) :
  OXFrame(parent, x, y, options, back) {

  const OPicture *bckgnd_pix = NULL;

  XGCValues gcv;
  int mask;

//  bckgnd_pix = GetResourcePool()->GetDocumentBckgndPicture();
  if (bckgnd_pix) {
    mask = GCTile | GCFillStyle | GCGraphicsExposures;
    SetBackgroundPixmap(bckgnd_pix->GetPicture());
    gcv.tile = bckgnd_pix->GetPicture();
    gcv.fill_style = FillTiled;
    gcv.graphics_exposures = True;
  } else {
    mask = GCForeground | GCBackground | GCGraphicsExposures;
    gcv.foreground = foxircSettings->GetPixelID(P_COLOR_BACKGROUND);
    gcv.background = gcv.foreground;
    gcv.graphics_exposures = True;
    SetBackgroundColor(gcv.foreground);
  }
  _backgc = new OXGC(GetDisplay(), _id, mask, &gcv);

  _font = _client->GetFont((OXFont *) foxircSettings->GetFont());

  _gc = new OXGC(GetDisplay(), _id);
  _gc->SetFont(_font->GetId());
  _gc->SetBackground(foxircSettings->GetPixelID(P_COLOR_BACKGROUND));

  _tx = 0;
  _ty = 0;
}

OXViewDocFrame::~OXViewDocFrame() {
  Clear();
  delete _backgc;
  delete _gc;
  _client->FreeFont(_font);
}

void OXViewDocFrame::SetupBackgroundPic(const char *name) {
  const OPicture *bckgnd_pix = _client->GetPicture(name);
  if (!bckgnd_pix) return;  // should set tile to None...

  SetBackgroundPixmap(bckgnd_pix->GetPicture());
  _backgc->SetTile(bckgnd_pix->GetPicture());
  _backgc->SetFillStyle(FillTiled);
}

void OXViewDocFrame::SetupBackground(unsigned long back) {
  SetBackgroundColor(back);
  _backgc->SetBackground(back);
  _backgc->SetForeground(back);
  _gc->SetBackground(back);
}

void OXViewDocFrame::SetFont(const char *fontname) {
  SetFont(_client->GetFont(fontname));
}

void OXViewDocFrame::SetFont(OXFont *f) {
  _client->FreeFont(_font);
  if (f) {
    _font = f;
  } else {
    _font = _client->GetFont((OXFont *) foxircSettings->GetFont());
  }
  _gc->SetFont(_font->GetId());
}

void OXViewDocFrame::Clear() {
  _document->Clear();
}

void OXViewDocFrame::Layout() {
  _document->SetWidth(_w);
  _document->Layout();
}

void OXViewDocFrame::DrawRegion(int x, int y, int w, int h, int clear) {
  XRectangle rect;

  rect.x = x;
  rect.y = y;
  rect.width = w;
  rect.height = h;

  if (clear) {
    int mask = GCTileStipXOrigin | GCTileStipYOrigin;
    XGCValues gcv;

    gcv.ts_x_origin = -_tx;
    gcv.ts_y_origin = -_ty;
    XChangeGC(GetDisplay(), _backgc->GetGC(), mask, &gcv);

    XFillRectangle(GetDisplay(), _id, _backgc->GetGC(), x, y, w, h);

//    XClearArea(GetDisplay(), _id, x, y, w, h, false);
  }

//    XClearArea(GetDisplay(), _id, x, y, w, h,false);
  _document->DrawRegion(GetDisplay(), _id, -_tx, -_ty, &rect);
//  _document->DrawRegion(GetDisplay(), _id, 0, 0, &rect);
}

void OXViewDocFrame::HScrollWindow(int x) {
  XPoint points[4];
  int xsrc, ysrc, xdest, ydest;

  // These points are the same for both cases, so set them here.

  if (x == _tx) return;

  points[0].y = points[3].y = 0;
  points[1].y = points[2].y = _h;
  ysrc = ydest = 0;

  if (x < _tx) {
    // scroll left...
    xsrc = 0;
    xdest = _tx - x;
    // limit the destination to the window width
    if (xdest > _w) xdest = _w;
    // Fill in the points array with the bounding box of the area that
    // needs to be redrawn - that is, the area that is not copied.
    points[1].x = points[0].x = 0;
    points[3].x = points[2].x = xdest; // -1;
  } else {
    // scroll right...
    xdest = 0;
    xsrc = x - _tx;
    // limit the source to the window height
    if (xsrc > _w) xsrc = _w;
    // Fill in the points array with the bounding box of the area that
    // needs to be redrawn - that is, the area that is not copied.
    points[1].x = points[0].x = _w - xsrc; // +1;
    points[3].x = points[2].x = _w;
  }

  // Copy the scrolled region to its new position
  XCopyArea(GetDisplay(), _id, _id, _backgc->GetGC(),
            xsrc, ysrc, _w, _h, xdest, ydest);

  // Set the new origin
  _tx = x;
  DrawRegion(points[0].x, points[0].y,
             points[2].x - points[0].x, points[2].y - points[0].y, True);
}

void OXViewDocFrame::VScrollWindow(int y) {
  XPoint points[4];
  int xsrc, ysrc, xdest, ydest;

  // These points are the same for both cases, so set them here.

  if (y == _ty) return;

  points[0].x = points[3].x = 0;
  points[1].x = points[2].x = _w;
  xsrc = xdest = 0;

  if (y < _ty) {
    // scroll down...
    ysrc = 0;
    ydest = _ty - y;
    // limit the destination to the window height
    if (ydest > _h) ydest = _h;
    // Fill in the points array with the bounding box of the area that
    // needs to be redrawn - that is, the area that is not copied.
    points[1].y = points[0].y = 0;
    points[3].y = points[2].y = ydest; // -1;
  } else {
    // scroll up...
    ydest = 0;
    ysrc = y - _ty;
    // limit the source to the window height
    if (ysrc > _h) ysrc = _h;
    // Fill in the points array with the bounding box of the area that
    // needs to be redrawn - that is, the area that is not copied.
    points[1].y = points[0].y = _h - ysrc; // +1;
    points[3].y = points[2].y = _h;
  }

  // Copy the scrolled region to its new position
  XCopyArea(GetDisplay(), _id, _id, _backgc->GetGC(),
            xsrc, ysrc, _w, _h, xdest, ydest);

  // Set the new origin
  _ty = y;
  DrawRegion(points[0].x, points[0].y,
             points[2].x - points[0].x, points[2].y - points[0].y, True);

#if 1
  XEvent event;

  XSync(GetDisplay(), False);
  while (XCheckTypedWindowEvent(GetDisplay(), _id,
                                GraphicsExpose, &event)) {
    HandleGraphicsExpose((XGraphicsExposeEvent *) &event);
    if (event.xgraphicsexpose.count == 0) break;
  }
#endif
}


//----------------------------------------------------------------------

OXViewDoc::OXViewDoc(const OXWindow *parent, OViewDoc *d, int x, int y,
                       unsigned int options, unsigned long back) :
  OXCompositeFrame(parent, x, y, options,back) {

  SetLayoutManager(new OHorizontalLayout(this));

  _background = back;

  _document = d;
  _document->CreateCanvas(this);
  _canvas = _document->GetCanvas();

  _vscrollbar = new OXVScrollBar(this, 16, 16, CHILD_FRAME);
  _hscrollbar = new OXHScrollBar(this, 16, 16, CHILD_FRAME);

  _vscrollbar->SetDelay(10, 10);
  _vscrollbar->SetMode(SB_ACCELERATED);
  _hscrollbar->SetDelay(10, 10);
  _hscrollbar->SetMode(SB_ACCELERATED);

  AddFrame(_canvas, NULL);
  AddFrame(_hscrollbar, NULL);
  AddFrame(_vscrollbar, NULL);

  MapSubwindows();

  AddInput(ButtonPressMask | ButtonReleaseMask);
}

OXViewDoc::~OXViewDoc() {
}

int OXViewDoc::HandleButton(XButtonEvent *event) {
  if (event->type == ButtonPress) {
    int amount, ch;

    ch = _canvas->GetHeight();
    amount = max(ch / 6, 1);

    if (event->state & ShiftMask)
      amount = 1;
    else if (event->state & ControlMask)
      amount = ch - max(ch / 20, 1);

    if (event->button == Button4) {
      _vscrollbar->SetPosition(_vscrollbar->GetPosition() - amount);
      return True;
    } else if (event->button == Button5) {
      _vscrollbar->SetPosition(_vscrollbar->GetPosition() + amount);
      return True;
    }

  }
  return False;
}

int OXViewDoc::LoadFile(const char *fname) {
  FILE *file = fopen(fname, "r");
  if (file) {
    _document->LoadFile(file);
    fclose(file);
    //Layout();
    Redisplay();
    return True;
  }
  return False;
}

void OXViewDoc::Clear() {
  _canvas->Clear();
  Layout();
}

int OXViewDoc::ProcessMessage(OMessage *msg) {
  OScrollBarMessage *sbmsg = (OScrollBarMessage *) msg;

  switch (msg->type) {
    case MSG_HSCROLL:
      switch (msg->action) {
        case MSG_SLIDERTRACK:
        case MSG_SLIDERPOS:
          _canvas->HScrollWindow(sbmsg->pos);
          break;
      }
      break;

    case MSG_VSCROLL:
      switch (msg->action) {
        case MSG_SLIDERTRACK:
        case MSG_SLIDERPOS:
          _canvas->VScrollWindow(sbmsg->pos);
          break;
      }
      break;
  }

  return True;
}

void OXViewDoc::AdjustScrollbars() {
  int tdw, tdh, tcw, tch;

  tch = _h - _insets.t - _insets.b;
  tcw = _w - _insets.l - _insets.r;
  tdw = _canvas->_document->GetWidth();
  tdh = _canvas->_document->GetHeight();

  if (_vscrollbar->IsMapped()) {
    tcw -= _vscrollbar->GetDefaultWidth();
    if (_hscrollbar->IsMapped()) {
      tch -= _hscrollbar->GetDefaultHeight();
      if (tcw > tdw) tdw = tcw;
      _hscrollbar->SetRange(tdw, tcw);
    }
    if (tch > tdh) tdh = tch;
    _vscrollbar->SetRange(tdh, tch);
  } else {
    if (_hscrollbar->IsMapped()) {
      if (tcw > tdw) tdw = tcw;
      _hscrollbar->SetRange(tdw, tcw);
    }
  }
}

void OXViewDoc::Layout() {
  int tcw, tch;

  tch = _h - _insets.t - _insets.b;
  tcw = _w - _insets.l - _insets.r;

  _canvas->_h = tch - _hscrollbar->GetDefaultHeight();
  _canvas->_w = tcw - _vscrollbar->GetDefaultWidth();
  _canvas->Layout();
  
  if (((tch >= _canvas->_document->GetHeight()) ||
       (_scroll_options & NO_VSCROLL)) &&
      (!(_scroll_options & FORCE_VSCROLL))) {
    _vscrollbar->UnmapWindow();
    _canvas->_ty = 0;
    if (((tcw >= _canvas->_document->GetWidth()) ||
         (_scroll_options & NO_HSCROLL)) &&
        (!(_scroll_options & FORCE_HSCROLL))) {
      _hscrollbar->UnmapWindow();
      _canvas->_tx = 0;
    } else {
      tch -= _hscrollbar->GetDefaultHeight();
      _hscrollbar->MoveResize(_insets.l, _insets.t + tch,
                              tcw, _hscrollbar->GetDefaultHeight());
      _hscrollbar->MapWindow();
    }
  } else {
    tcw -= _vscrollbar->GetDefaultWidth();
    if (((tcw >= _canvas->_document->GetWidth()) ||
         (_scroll_options & NO_HSCROLL)) &&
        (!(_scroll_options & FORCE_HSCROLL))) {
      _hscrollbar->UnmapWindow();
      _canvas->_tx = 0;
    } else {
      tch -= _hscrollbar->GetDefaultHeight();
      _hscrollbar->MoveResize(_insets.l, _insets.t + tch,
                              tcw, _hscrollbar->GetDefaultHeight());
      _hscrollbar->MapWindow();
    }
    _vscrollbar->MoveResize(_insets.l + tcw, _insets.t,
                            _vscrollbar->GetDefaultWidth(), tch);
    _vscrollbar->MapWindow();
  }
  AdjustScrollbars();
  _canvas->MoveResize(_insets.l, _insets.t, tcw, tch);
}

void OXViewDoc::ScrollUp() {
  int pos = _canvas->_document->GetHeight() - _canvas->GetHeight();

  if (pos < 0) pos = 0;
  if (pos == _canvas->_ty)
    _canvas->DrawRegion(0, 0, _canvas->_w, _canvas->_h, False);
  _vscrollbar->SetPosition(pos);
}

void OXViewDoc::Redisplay() {
  Layout();
#if 0
  _canvas->ClearWindow();
#else
  _canvas->DrawRegion(0, 0, _canvas->_w, _canvas->_h, True);
#endif
}
