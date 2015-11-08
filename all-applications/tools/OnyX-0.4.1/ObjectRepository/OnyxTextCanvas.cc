/**************************************************************************

    This file is part of OnyX, a visual development environment
    using the xclass toolkit. Copyright (C) 1997, 1998 Frank Hall.

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

#include "OnyxTextCanvas.h"

#include <stdio.h>

extern MessageCenter *CentralMessageCenter;


//----------------------------------------------------------------------

OXTextFrame::OXTextFrame(OXWindow *parent, int x, int y,
                         unsigned int options, unsigned long back) :
  OXFrame(parent, x, y, options, back) {
    XGCValues gcv;
    int mask = GCFont | GCForeground | GCBackground;

    _font = _client->GetFont("Courier -12");

    _th = _font->TextHeight();

    SetBackgroundColor(back);

    gcv.foreground = _client->GetColorByName("black");
    gcv.background = back;
    gcv.font = _font->GetId();
    _gc = new OXGC(GetDisplay(), _id, mask, &gcv);

    nlines = top = 0;
}

OXTextFrame::~OXTextFrame() {
  Clear();
  _client->FreeFont((OXFont *) _font);
  delete _gc;
}

void OXTextFrame::Clear() {
  int i;

  for (i=0; i<nlines; ++i) delete chars[i];
  nlines = top = 0;
  XClearWindow(GetDisplay(), _id);
}

int OXTextFrame::AddText(char *Text) {
  int cnt;
  char buf[MAXLINESIZE], c, *src;
  char line[MAXLINESIZE], *dst;

  if (nlines < MAXLINES) {
    strncpy(buf, Text, MAXLINESIZE);

    // Expand tabs
    src = buf;
    dst = line;
    cnt = 0;
    while ((c = *src++)) {
      if (c == 0x0D || c == 0x0A) {
        break;
      } else if (c == 0x09) {
        do {
          *dst++ = ' ';
        } while (((dst-line) & 0x7) && (cnt++ < MAXLINESIZE-1));
      } else {
        *dst++ = c;
      }

      if (cnt++ >= MAXLINESIZE-1) {
        break;
      }
    }

    *dst = '\0';
    chars[nlines] = new char[strlen(line) + 1];
    strcpy(chars[nlines], line);
    lnlen[nlines] = strlen(chars[nlines]);

    // Remember the number of lines, and initialize the current line
    // number to be 0.
  
    nlines++;

    DrawRegion(0, 0, _w, _h);

    return True;
  } else {
    return False;
  }
}

void OXTextFrame::DrawRegion(int x, int y, int w, int h) {
  int yloc = 0, index = top;
  XRectangle rect;

  OFontMetrics fm;
  _font->GetFontMetrics(&fm);

  rect.x = x;
  rect.y = y;
  rect.width = w;
  rect.height = h;

  // Set the clip mask of the GC

  XSetClipRectangles(GetDisplay(), _gc->GetGC(), 0, 0, &rect, 1, Unsorted);

  // Loop through each line until the bottom of the window is reached,
  // or we run out of lines. Redraw any lines that intersect the exposed
  // region.

  while (index < nlines && yloc < _h) {
    yloc += _th;
    if ((yloc - _th <= rect.y + rect.height) && (yloc >= rect.y))
      XDrawImageString(GetDisplay(), _id, _gc->GetGC(),
                       MARGIN, yloc - fm.descent,
                       chars[index], lnlen[index]);
    ++index;
  }

  // Set the GC clip mask back to None

  XSetClipMask(GetDisplay(), _gc->GetGC(), None);
}

void OXTextFrame::SetTopLine(int new_top) {
  if (top == new_top) return;
  ScrollWindow(new_top);
}

void OXTextFrame::ScrollWindow(int new_top) {
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
  XCopyArea(GetDisplay(), _id, _id, _gc->GetGC(),
            xsrc, ysrc, _w, _h, xdest, ydest);
  // Clear the remaining area of any old text, set the exposure
  // parameter to True to generate expose events for redraw.
  XClearArea(GetDisplay(), _id, points[0].x, points[0].y,
             0, points[2].y - points[0].y, False/*True*/);

  DrawRegion(points[0].x, points[0].y,
             points[2].x - points[0].x, points[2].y - points[0].y);
}

//----------------------------------------------------------------------

OXTextView::OXTextView(OXWindow *parent, int x, int y,
                       unsigned int options, unsigned long back) :
  OXCompositeFrame(parent, x, y, options, back) {

    SetLayoutManager(new OHorizontalLayout(this));

    int background = _client->GetColorByName("white");
    SetBackgroundColor(background);

    _textCanvas = new OXTextFrame(this, 10, 10, CHILD_FRAME, background);
    _vsb = new OXVScrollBar(this, 10, 10, CHILD_FRAME);

    AddFrame(_textCanvas, NULL);
    AddFrame(_vsb, NULL);
}

OXTextView::~OXTextView() {
  delete _textCanvas;
  delete _vsb;
}

int OXTextView::AddText(char *text) {
#if 0
  int retc = _textCanvas->AddText(text);
  if (retc) {
    _vsb->SetPosition(0);
    Layout();
  }
  return retc;
#else
  int nlines, vlines, top, newtop, pos, retc;

  nlines = _textCanvas->GetLines();
  vlines = _textCanvas->GetVisibleLines();
  pos = _vsb->GetPosition();
  if (nlines > vlines)
    top = nlines - vlines;
  else
    top = 0;

  retc = _textCanvas->AddText(text);

  if (retc) {
    nlines = _textCanvas->GetLines();
    vlines = _textCanvas->GetVisibleLines();

    if (nlines > vlines)
      newtop = nlines - vlines;
    else
      newtop = 0;

    if (top != newtop) {
      Layout();
      if (pos == top) _vsb->SetPosition(newtop);
    }
  }

  return retc;
#endif
}

void OXTextView::Clear() {
  _textCanvas->Clear();
  Layout();
}

int OXTextView::ProcessMessage(OMessage *msg) {
  OScrollBarMessage *sbmsg = (OScrollBarMessage *) msg;

  switch (msg->type) {
    case MSG_VSCROLL:
      switch (msg->action) {
        case MSG_SLIDERTRACK:
        case MSG_SLIDERPOS:
          _textCanvas->SetTopLine(sbmsg->pos);
          break;
      }
      break;
  }
  return True;
}

void OXTextView::Layout() {
  int lines, vlines, tcw, tch;

  tch = _h - (_bw << 1);
  tcw = _w - (_bw << 1);
  _textCanvas->SetHeight(tch);
  lines = _textCanvas->GetLines();
  vlines = _textCanvas->GetVisibleLines();

  if (lines <= vlines) {
    _vsb->UnmapWindow();
  } else {
    tcw -= _vsb->GetDefaultWidth();
    _vsb->MoveResize(_bw + tcw, _bw, _vsb->GetDefaultWidth(), tch);
    _vsb->MapWindow();
    _vsb->SetRange(lines, vlines);
  }

  _textCanvas->MoveResize(_bw, _bw, tcw, tch);
}


//----------------------------------------------------------------------

OnyxTextCanvas::OnyxTextCanvas() : OnyxObject("OnyxTextCanvas") {
  X = 0;
  Y = 0;
  Width = 10;
  Height = 10;
}

int OnyxTextCanvas::Create() {
  OnyxObject::Create();
  InternalFrame = new OXCompositeFrame(Parent,Width,Height);
  InternalTextView = new OXTextView(InternalFrame, Width, Height, SUNKEN_FRAME | DOUBLE_BORDER);
  InternalFrame->AddFrame(InternalTextView, new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y, 0, 0, 3, 0));

  InternalTextView->Clear();

  InternalFrame->MapSubwindows();
  InternalFrame->Layout();
  InternalFrame->Move(X, Y);
/*    TextBoxExists++;*/
/*    InternalTextEntry->Associate(CentralMessageCenter);*/
//    InternalTextView->Associate(InternalFrame);
  return True;
}


int OnyxTextCanvas::AddText(char *Text) {
  return InternalTextView->AddText(Text);
}

void OnyxTextCanvas::Clear() {
  InternalTextView->Clear();  
}

void OnyxTextCanvas::Move(int newX, int newY) {
  OnyxObject::Move(newX, newY);
  InternalFrame->Move(newX, newY);
}

void OnyxTextCanvas::Resize(int newWidth, int newHeight) {
  OnyxObject::Resize(newWidth, newHeight);
  InternalFrame->Resize(newWidth, newHeight);
  InternalFrame->Layout();
}

void OnyxTextCanvas::MoveResize(int newX, int newY, int newWidth, int newHeight) {
  OnyxObject::MoveResize(newX, newY, newWidth, newHeight);
  InternalFrame->MoveResize(newX, newY, newWidth, newHeight);
  InternalFrame->Layout();
}

OnyxTextCanvas::~OnyxTextCanvas() {
  InternalFrame->DestroyWindow();
  delete InternalFrame;
}
