#include <stdio.h>
#include <ctype.h>

#include "OXTextView.h"

//----------------------------------------------------------------------

OXTextFrame::OXTextFrame(OXWindow *parent, int x, int y,
                         unsigned int options, unsigned long back) :
  OXFrame(parent, x, y, options, back) {
    XGCValues gcv;
    int mask = GCFont | GCForeground | GCBackground;

    font = XLoadQueryFont(GetDisplay(),
           "-adobe-courier-medium-r-*-*-12-*-*-*-*-*-iso8859-1");
//           "8x13");

    _th = font->ascent + font->descent;

    gcv.foreground = _client->GetColorByName("black");
    gcv.background = _backPixel;
    gcv.font = font->fid;

    _gc = XCreateGC(GetDisplay(), _id, mask, &gcv);

    top = 0;
    nlines = 1;

    chars[0] = new char[1];
    *chars[0] = '\0';
    lnlen[0] = 0;
}

OXTextFrame::~OXTextFrame() {
  int i;

  for (i=0; i<nlines; ++i) delete[] chars[i];
}

void OXTextFrame::Clear() {
  int i;

  for (i=0; i<nlines; ++i) delete[] chars[i];
  nlines = 1;
  top = 0;
  chars[0] = new char[1];
  *chars[0] = '\0';
  lnlen[0] = 0;
  XClearWindow(GetDisplay(), _id);
}

int OXTextFrame::AddText(char *text) {
  int  i, cnt;
  char c, *src;
  char line[MAXLINESIZE], *dst;

  // Copy each line of 'text' into the buffer.
  // Previous line might not have been completed!

  i = nlines-1;
  src = text;
  strcpy(line, chars[i]);
  delete[] chars[i];
  dst = line + (cnt = strlen(line));
  while (i < MAXLINES) {
    while (c = *src) {
      if (!c || c == '\n') break;
      ++src;
      if (c == '\t')
        // Expand tabs
        do
          *dst++ = ' ';
        while (((dst-line) & 0x7) && (cnt++ < MAXLINESIZE-1));
      else if (!isprint(c))
        continue;
      else
        *dst++ = c;
      if (cnt++ >= MAXLINESIZE-1) {
        while (*src && *src != '\n') ++src;
        break;
      }
    }
    *dst = '\0';
    chars[i] = new char[strlen(line) + 1];
    strcpy(chars[i], line);
    lnlen[i] = strlen(chars[i]);
    ++i;
    dst = line;
    cnt = 0;
    if (!*src++) break;
  }

  nlines = i;
  //top = 0;

/*
  if (nlines > _h/_th)
    top = nlines - _h/_th;
  else
    top = 0;
*/
  DrawRegion(0, 0, _w, _h);

  return True;
}

void OXTextFrame::DrawRegion(int x, int y, int w, int h) {
  int yloc = 0, index = top;
  XRectangle rect;

  rect.x = x;
  rect.y = y;
  rect.width = w;
  rect.height = h;

  // Set the clip mask of the GC

  XSetClipRectangles(GetDisplay(), _gc, 0, 0, &rect, 1, Unsorted);

  // Loop through each line until the bottom of the window is reached,
  // or we run out of lines. Redraw any lines that intersect the exposed
  // region.

  while (index < nlines && yloc < _h) {
    yloc += _th;
    if ((yloc - _th <= rect.y + rect.height) && (yloc >= rect.y))
      XDrawImageString(GetDisplay(), _id, _gc, MARGIN, yloc - font->descent,
                       chars[index], lnlen[index]);
    ++index;
  }

  // Set the GC clip mask back to None

  XSetClipMask(GetDisplay(), _gc, None);

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
  XCopyArea(GetDisplay(), _id, _id, _gc, xsrc, ysrc, _w, _h, xdest, ydest);
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

    _textCanvas = new OXTextFrame(this, 10, 10, CHILD_FRAME | OWN_BKGND,
                                  _defaultDocumentBackground);
    _vsb = new OXVScrollBar(this, 10, 10, CHILD_FRAME);

    AddFrame(_textCanvas, NULL);
    AddFrame(_vsb, NULL);
}

OXTextView::~OXTextView() {
}

int OXTextView::AddText(char *text) {
  int retc = _textCanvas->AddText(text);
  if (retc) {
    int nlines = _textCanvas->GetLines();
    int vlines = _textCanvas->GetVisibleLines();
    int top;

    if (nlines > vlines)
      top = nlines - vlines;
    else
      top = 0;

    Layout();

    _vsb->SetPosition(top);
  }
  return retc;
}

void OXTextView::Clear() {
  _textCanvas->Clear();
  Layout();
}

int OXTextView::ProcessMessage(OMessage *msg) {
  OScrollBarMessage *sbmsg;

  switch(msg->type) {
    case MSG_VSCROLL:
      sbmsg = (OScrollBarMessage *) msg;
      switch(msg->action) {
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
  }

  _textCanvas->MoveResize(_insets.l, _insets.t, tcw, tch);
}
