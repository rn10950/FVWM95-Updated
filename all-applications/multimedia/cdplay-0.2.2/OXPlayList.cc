/**************************************************************************

    This file is part of explorer95, a file manager for fvwm95.
    Copyright (C) 1996, 1997 David Barth, Hector Peraza.

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
#include <xclass/ODimension.h>

#include "OXPlayList.h"


//----------------------------------------------------------------------

OXPlayList::OXPlayList(OXWindow *parent, int x, int y, OMedium *medium,
                       unsigned int options, unsigned long back) :
  OXFrame(parent, x, y, options, back) {
    XGCValues gcv;
    int mask = GCFont | GCForeground | GCBackground;

    font = XLoadQueryFont(GetDisplay(),
                 "-adobe-helvetica-medium-r-*-*-12-*-*-*-*-*-iso8859-1");

    _th = font->ascent + font->descent + 4;

    SetBackgroundColor(back);

    gcv.foreground = _client->GetColorByName("black");
    gcv.background = back;
    gcv.font = font->fid;

    _gc = XCreateGC(GetDisplay(), _id, mask, &gcv);

    gcv.foreground = _client->GetColorByName("#000080");

    _backselgc = XCreateGC(GetDisplay(), _id, mask, &gcv);

    gcv.foreground = back;
    gcv.background = _client->GetColorByName("black");

    _selgc = XCreateGC(GetDisplay(), _id, mask, &gcv);

    XGrabButton(GetDisplay(), Button1, AnyModifier, _id, True,
                ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                GrabModeAsync, GrabModeAsync, None, CurrentTime);

    nlines = top = 0;
    omedium = medium;

    status = 0;
    firstselline = 1;
    lastselline = 0;
} 

OXPlayList::~OXPlayList() {
}

int OXPlayList::HandleMotion(XMotionEvent *event) {
  int line;

  OXFrame::HandleMotion(event);

  if (status) {
    line = event->y / _th + top;
    if (line < 0) line = 0;
    if (line >= nlines) line = nlines-1;
    if (line < selline) {
      firstselline = line;
      lastselline = selline;
    } else {
      firstselline = selline;
      lastselline = line;
    }
    DrawRegion(0,0,_w,_h);
  }

  XGrabButton(GetDisplay(), Button1, AnyModifier, _id, True,
              ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
              GrabModeAsync, GrabModeAsync, None, CurrentTime);

  return True;
}

int OXPlayList::HandleButton(XButtonEvent *event) {
  int line;

  if ((!status) && (event->type == ButtonPress)) {
    line = event->y / _th + top;
    if (line < 0) line = 0;
    if (line >= nlines) line = nlines-1;
    firstselline = lastselline = selline = line;
    status = 1;
    DrawRegion(0,0,_w,_h);
  } else if ((status) && (event->type == ButtonRelease)) {
    status = 0;
  }

  XGrabButton(GetDisplay(), Button1, AnyModifier, _id, True,
              ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
              GrabModeAsync, GrabModeAsync, None, CurrentTime);

  return True;
}

void OXPlayList::DrawRegion(int x, int y, int w, int h) {
  int yloc = 0, index = top;
  XRectangle rect;
  char *str;

  rect.x = x;
  rect.y = y;
  rect.width = w;
  rect.height = h;

  XSetClipRectangles(GetDisplay(), _gc, 0, 0, &rect, 1, Unsorted);
  XSetClipRectangles(GetDisplay(), _selgc, 0, 0, &rect, 1, Unsorted);
  XSetClipRectangles(GetDisplay(), _backselgc, 0, 0, &rect, 1, Unsorted);

  while (index < nlines && yloc < _h) {
    if ((yloc <= rect.y + rect.height) && (yloc + _th >= rect.y)) {
      str = omedium->track_db[tracks[index]].name;
      if (( index >= firstselline ) && ( index <= lastselline )) {
        XFillRectangle(GetDisplay(),_id,_backselgc, 0, yloc, _w, _th);
        XDrawString(GetDisplay(), _id, _selgc, MARGIN,
                    yloc + _th - font->descent - 2, str, strlen(str));
      } else {
        XFillRectangle(GetDisplay(),_id,_selgc, 0, yloc, _w, _th);
        XDrawString(GetDisplay(), _id, _gc, MARGIN,
                    yloc + _th - font->descent - 2, str, strlen(str));
      }            
    }
    ++index;
    yloc += _th;
  }
  XFillRectangle(GetDisplay(),_id,_selgc, 0, yloc, _w, _th);

  XSetClipMask(GetDisplay(), _gc, None);
  XSetClipMask(GetDisplay(), _selgc, None);
  XSetClipMask(GetDisplay(), _backselgc, None);
}

void OXPlayList::SetTopLine(int new_top) {
  if (top == new_top) return;
  // top = new_top; DrawRegion(0,0,_w,_h);
  ScrollWindow(new_top);
}

void OXPlayList::ScrollWindow(int new_top) {
  XPoint points[4];
  int xsrc, ysrc, xdest, ydest;

  // These points are the same for both cases, so set them here.

  points[0].x = points[3].x = 0;
  points[1].x = points[2].x = _w;
  xsrc = xdest = 0;

  if (new_top < top) {
    // scroll down...
    ysrc = 0;
    // convert new_top row position to pixels
    ydest = (top - new_top) * _th;
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
    ysrc = (new_top - top) * _th;
    // limit the source to the window height
    if (ysrc > _h) ysrc = _h;
    // Fill in the points array with the bounding box of the area that
    // needs to be redrawn - that is, the area that is not copied.
    points[1].y = points[0].y = _h - ysrc; // +1;
    points[3].y = points[2].y = _h;
  }
  // Set the top line of the text buffer
  top = new_top;
  // Copy the scrolled region to its new position
  XCopyArea(GetDisplay(), _id, _id, _gc, xsrc, ysrc, _w, _h, xdest, ydest);
  // Clear the remaining area of any old text, set the exposure
  // parameter to True to generate expose events for redraw.
  //XClearArea(GetDisplay(), _id, points[0].x, points[0].y,
                                //0, points[2].y - points[0].y, False/*True*/);

  DrawRegion(points[0].x, points[0].y,
             points[2].x - points[0].x, points[2].y - points[0].y);
}

//----------------------------------------------------------------------

OXScrolledPlayList::OXScrolledPlayList(const OXWindow *parent, int x, int y,
                                       OMedium *medium,
                                       unsigned int options,
                                       unsigned long back) :
  OXCompositeFrame(parent, x, y, options, back) {

    SetLayoutManager(new OHorizontalLayout(this));

    int background = _client->GetColorByName("white");
    SetBackgroundColor(background);

    _playlist = new OXPlayList(this, 10, 10, medium, CHILD_FRAME,
                               _client->GetColorByName("white"));
    _scrollbar = new OXVScrollBar(this, 10, 10, CHILD_FRAME);

    AddFrame(_playlist, NULL);
    AddFrame(_scrollbar, NULL);
}

OXScrolledPlayList::~OXScrolledPlayList() {
}

int OXScrolledPlayList::ProcessMessage(OMessage *msg) {
  switch(msg->type) {
    case MSG_VSCROLL:
      switch(msg->action) {
        case MSG_SLIDERTRACK:
        case MSG_SLIDERPOS:
          OScrollBarMessage *sbmsg = (OScrollBarMessage *) msg;
          _playlist->SetTopLine(sbmsg->pos);
          break;
      }
      break;
  }
  return True;
}

void OXScrolledPlayList::Layout() {
  int lines, vlines, tcw, tch;

  tch = _h - (_bw << 1);
  tcw = _w - (_bw << 1);
  _playlist->SetHeight(tch);
  lines = _playlist->GetLines();
  vlines = _playlist->GetVisibleLines();

  if (lines <= vlines) {
    _scrollbar->UnmapWindow();
    _playlist->SetTopLine(0);
  } else {
    tcw -= _scrollbar->GetDefaultWidth();
    _scrollbar->MoveResize(_bw + tcw, _bw, _scrollbar->GetDefaultWidth(), tch);
    _scrollbar->MapWindow();
    _scrollbar->SetRange(lines, vlines);
  }

  _playlist->MoveResize(_bw, _bw, tcw, tch);
}

void OXScrolledPlayList::Clear() {
  _playlist->nlines = _playlist->top = 0;
  XClearWindow(GetDisplay(), _playlist->_id);
  Layout();
}

void OXScrolledPlayList::AddTrack(int track) {
  _playlist->tracks[_playlist->nlines++] = track;
  _playlist->DrawRegion(0,0,_playlist->_w,_playlist->_h);
  Layout();
}

void OXScrolledPlayList::RemoveSelection() {
  int dec = _playlist->lastselline - _playlist->firstselline + 1;

  if (dec > 0) {
    for(int i = _playlist->lastselline + 1; i <= _playlist->nlines; i++)
      _playlist->tracks[i - dec] = _playlist->tracks[i];
    _playlist->nlines -= dec;
    _playlist->firstselline = 1;
    _playlist->lastselline = 0;
    XClearWindow(GetDisplay(), _playlist->_id);
    _playlist->DrawRegion(0,0,_playlist->_w,_playlist->_h);
  }
}

int OXScrolledPlayList::GetFirstSelectedTrack() {
  int track = -1;

  if (_playlist->firstselline <= _playlist->lastselline) {
    track = _playlist->tracks[_playlist->firstselline++];
    _playlist->DrawRegion(0,0,_playlist->_w,_playlist->_h);
  }
  return(track);
}

void OXScrolledPlayList::SavePlayListToMedium() {
  _playlist->omedium->ClearPlayList();
  if (_playlist->nlines > 0) {
    for(int i = 0; i < _playlist->nlines; i++)
      _playlist->omedium->AddToPlayList(_playlist->tracks[i]);
  } else {
    _playlist->omedium->ResetPlayList();
  }
}

void OXScrolledPlayList::GetPlayListFromMedium() {
  Clear();
  for(int i = 0; i < _playlist->omedium->play_list_length; i++)
    AddTrack(_playlist->omedium->play_list[i]);
}

