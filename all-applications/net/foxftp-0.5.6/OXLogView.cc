/**************************************************************************

    This file is part of foxftp, a ftp client for fOX.
    Copyright (C) 1998, 1999, G. Matzka.

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

#include <xclass/utils.h>
#include <xclass/OXMenu.h>
#include <xclass/OXScrollBar.h>
#include <xclass/OXFont.h>

#include "OXLogView.h"


//----------------------------------------------------------------------

OXLogFrame::OXLogFrame(OXWindow *parent, int w, int h,
                       unsigned int options, unsigned int back) :
  OXFrame(parent, w, h, options, back) {
    XGCValues gcv;
    int mask = GCFont | GCForeground | GCBackground;

    _font = _client->GetFont("courier -12");

    _th = _font->TextHeight();

    gcv.font = _font->GetId();

    SetBackgroundColor(back);

    unsigned long white = _client->GetColorByName("white");
    gcv.background = white;
    gcv.foreground = _client->GetColorByName("red");
    _gc_received = XCreateGC(GetDisplay(), _id, mask, &gcv);

    gcv.background = white;
    gcv.foreground = _client->GetColorByName("blue");
    _gc_sent = XCreateGC(GetDisplay(), _id, mask, &gcv);

    _alines = 10;
    _lines = new char*[_alines];
    _types = new int[_alines];
    _nlines = _top = 0;
}

OXLogFrame::~OXLogFrame() {
  Clear();
  delete[] _lines;
  delete[] _types;
  _client->FreeFont(_font);
  XFreeGC(GetDisplay(), _gc_received);
  XFreeGC(GetDisplay(), _gc_sent);
}

void OXLogFrame::Clear() {
  int i;

  for (i = 0; i < _nlines; i++)
    if (_lines[i]) delete[] _lines[i];

  _nlines = _top = 0;
  XClearWindow(GetDisplay(), _id);
}

void OXLogFrame::SetTopLine(int new_top) {
  if (_top == new_top) return;
  ScrollWindow(new_top);
}

void OXLogFrame::AddLine(int type, char *line) {
  if (_nlines == _alines) {
    char **tmp = _lines;
    int *tmp2 = _types;
    _alines += 10;
    _lines = new char*[_alines];
    _types = new int[_alines];
    for (int i = 0; i < _nlines; i++) {
      _lines[i] = tmp[i];
      _types[i] = tmp2[i];
    }
    delete[] tmp;
    delete[] tmp2;
  }
  _types[_nlines] = type;
  _lines[_nlines] = StrDup(line);
  char *ln = _lines[_nlines];
  int len = strlen(ln);
  if ((len > 0) && (ln[len-1] == '\r')) ln[len-1] = '\0';
  _nlines++;
  if (_nlines <= GetVisibleLines())
    DrawRegion(0, 0, _w, _h);
  else if (_top + GetVisibleLines() == _nlines - 1)
    SetTopLine(_top + 1);
}

void OXLogFrame::DrawRegion(int x, int y, int w, int h) {
  int yloc, index = _top;
  XRectangle rect;
  OFontMetrics fm;
  GC *gc;

  _font->GetFontMetrics(&fm);

  rect.x = x;
  rect.y = y;
  rect.width = w;
  rect.height = h;

  // Set the clip mask of the GC
  XSetClipRectangles(GetDisplay(), _gc_sent, 0, 0, &rect, 1, Unsorted);
  XSetClipRectangles(GetDisplay(), _gc_received, 0, 0, &rect, 1, Unsorted);

  yloc = 0;
  while (index < _nlines && yloc < _h) {
    yloc += _th;
    if ((yloc - _th <= rect.y + rect.height) && (yloc >= rect.y) &&
        (_types[index] != NO_TYPE)) {
      if (_types[index] == SENT_TYPE)
        gc = &_gc_sent;
      else
        gc = &_gc_received;

      XDrawImageString(GetDisplay(), _id, *gc, 5,
                       yloc - fm.descent,
                       _lines[index], strlen(_lines[index]));
    }
    index++;
  }
 
  // Set the GC clip mask back to None
  XSetClipMask(GetDisplay(), _gc_sent, None);
  XSetClipMask(GetDisplay(), _gc_received, None);
}

void OXLogFrame::ScrollWindow(int new_top) {
  XPoint points[4];
  int xsrc, ysrc, xdest, ydest;
 
  // These points are the same for both cases, so set them here.
 
  points[0].x = points[3].x = 0;
  points[1].x = points[2].x = _w;
  xsrc = xdest = 0;
 
  if (new_top < _top) {
    // scroll down...
    ysrc = 0;
    // convert new_top row position to pixels
    ydest = (_top - new_top) * _th;
    // limit the destination to the window height
    if (ydest > _h) ydest = _h;
    // Fill in the points array with the bounding box of the area that
    // needs to be redrawn - that is, the area that is not copied.
    points[1].y = points[0].y = 0;
    points[3].y = points[2].y = ydest + _th; // -1;
  } else {
    // scroll up...
    ydest = 0;
    // convert new_top row position to pixels
    ysrc = (new_top - _top) * _th;
    // limit the source to the window height
    if (ysrc > _h) ysrc = _h;
    // Fill in the points array with the bounding box of the area that
    // needs to be redrawn - that is, the area that is not copied.
    points[1].y = points[0].y = _h - ysrc; // +1;
    points[3].y = points[2].y = _h;
  }

  // Set the top line of the text buffer
  _top = new_top;

  // Copy the scrolled region to its new position
  XCopyArea(GetDisplay(), _id, _id, _gc_sent, xsrc, ysrc, _w, _h, xdest, ydest);

  XClearArea(GetDisplay(), _id, points[0].x, points[0].y,
             0, points[2].y - points[0].y, false);
 
  DrawRegion(points[0].x, points[0].y,
             points[2].x - points[0].x, points[2].y - points[0].y);
}


//----------------------------------------------------------------------

OXLogView::OXLogView(OXWindow *parent, int w, int h,
                     unsigned int options, unsigned int back) :
    OXCompositeFrame(parent, w, h, options, back) {
	
  SetLayoutManager(NULL);

  int background = _client->GetColorByName("white");
  SetBackgroundColor(background);

  _textCanvas = new OXLogFrame(this, 10, 10, CHILD_FRAME, background);
  _vsb = new OXVScrollBar(this, 10, 10, CHILD_FRAME);

  AddFrame(_textCanvas, NULL);
  AddFrame(_vsb, NULL);

  _do_log = true;
  _popup = new OXPopupMenu(_client->GetRoot());
  _popup->AddEntry(new OHotString("&Continue ..."), 1);
  _popup->AddEntry(new OHotString("&Stop ..."), 2);
  _popup->AddSeparator();
  _popup->AddEntry(new OHotString("Clear"), 3);
  _popup->Associate(this);
}

OXLogView::~OXLogView() {
}

void OXLogView::AdjustScrollBar() {
  _vsb->SetPosition(_textCanvas->GetTopLine());
}

int OXLogView::HandleButton(XButtonEvent *event) {
  if (event->button != Button3)
    return true;

  if (_do_log) {
    _popup->DisableEntry(1);
    _popup->EnableEntry(2);
  } else {
    _popup->DisableEntry(2);
    _popup->EnableEntry(1);
  }
  _popup->PlaceMenu(event->x_root, event->y_root, true, true);
  return true;
}

int OXLogView::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;
  OScrollBarMessage *sbmsg = (OScrollBarMessage *) msg;

  switch (msg->type) {
    case MSG_MENU:
      switch (msg->action) {
        case MSG_CLICK:
          switch (wmsg->id) {
            case 1: {
              _do_log = true;
              break;
            }
            case 2: {
              _do_log = false;
              break;
            }
            case 3: {
              Clear();
              break;
            }
          }
          break;
      }
      break;

    case MSG_VSCROLL:
      switch (msg->action) {
        case MSG_SLIDERTRACK:
 //       case MSG_SLIDERPOS:
            if (sbmsg->pos != _textCanvas->GetTopLine())
              _textCanvas->SetTopLine(sbmsg->pos);
          break;
      }
      break;
  }
  return true;
}

void OXLogView::Layout() {
  int lines, vlines, tcw, tch;

  tch = _h - _insets.t - _insets.b;
  tcw = _w - _insets.l - _insets.r;
  _textCanvas->SetHeight(tch);
  lines = _textCanvas->GetLines();
  vlines = _textCanvas->GetVisibleLines();

  if (lines <= vlines) {
    _vsb->UnmapWindow();
  } else {
    tcw -= _vsb->GetDefaultWidth();
    _vsb->MoveResize(_insets.l + tcw, _insets.t, _vsb->GetDefaultWidth(), tch);
    _vsb->MapWindow();
    _vsb->SetRange(lines, vlines);
    _vsb->SetPosition(_textCanvas->GetTopLine());
  }

  _textCanvas->MoveResize(_insets.l, _insets.t, tcw, tch);
}
