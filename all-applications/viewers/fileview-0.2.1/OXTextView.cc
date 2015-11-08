#include <stdio.h>
#include <string.h>

#include "OXTextView.h"

//----------------------------------------------------------------------

OXTextFrame::OXTextFrame(OXWindow *parent, int x, int y,
                         unsigned int options, unsigned long back) :
  OXFrame(parent, x, y, options, back) {
    XGCValues gcv;
    int mask = GCFont | GCForeground | GCBackground;

    font = XLoadQueryFont(GetDisplay(),
//           "-adobe-courier-medium-r-*-*-12-*-*-*-*-*-iso8859-1");
           "8x13");

    _th = font->ascent + font->descent;

    SetBackgroundColor(back);

    gcv.foreground = _client->GetColorByName("black");
    gcv.background = back;
    gcv.font = font->fid;

    _gc = XCreateGC(GetDisplay(), _id, mask, &gcv);

    nlines = top = 0;
}

OXTextFrame::~OXTextFrame() {
  // Clear();  -- this tries to clear the (maybe) already unexisting
  //              window as well.
  int i;

  for (i=0; i<nlines; ++i) delete chars[i];
  nlines = top = 0;
}

void OXTextFrame::Clear() {
  int i;

  for (i=0; i<nlines; ++i) delete chars[i];
  nlines = top = 0;
  XClearWindow(GetDisplay(), _id);
}

int OXTextFrame::LoadFile(char *filename) {
  FILE *fp;
  int  i, cnt;
  char buf[MAXLINESIZE], c, *src;
  char line[MAXLINESIZE], *dst;

  if ((fp = fopen(filename, "r")) == NULL) return False;

  if (nlines > 0) Clear();

  // Read each line of the file into the buffer.

  i = 0;
  while ((fgets(buf, MAXLINESIZE, fp) != NULL) && (i < MAXLINES)) {
    // Expand tabs
    src = buf;
    dst = line;
    cnt = 0;
    while (c = *src++) {
      if (c == 0x0D || c == 0x0A)
        break;
      else if (c == 0x09)
        do
          *dst++ = ' ';
        while (((dst-line) & 0x7) && (cnt++ < MAXLINESIZE-1));
      else
        *dst++ = c;
      if (cnt++ >= MAXLINESIZE-1) break;
    }
    *dst = '\0';
    chars[i] = new char[strlen(line) + 1];
    strcpy(chars[i], line);
    lnlen[i] = strlen(chars[i]);
    ++i;
  }

  fclose(fp);

  // Remember the number of lines, and initialize the current line
  // number to be 0.

  nlines = i;
  top = 0;

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

    int background = _client->GetColorByName("white");
    SetBackgroundColor(background);

    _textCanvas = new OXTextFrame(this, 10, 10, CHILD_FRAME, background);
    _vsb = new OXVScrollBar(this, 10, 10, CHILD_FRAME);

    AddFrame(_textCanvas, NULL);
    AddFrame(_vsb, NULL);
}

OXTextView::~OXTextView() {
}

int OXTextView::LoadFile(char *fname) {
  int retc = _textCanvas->LoadFile(fname);
//  _vsb->SetRange(_textCanvas->GetLines(), _textCanvas->GetVisibleLines());
  if (retc) {
    _vsb->SetPosition(0);
    Layout();
  }
  return retc;
}

void OXTextView::Clear() {
  _textCanvas->Clear();
  Layout();
}

int OXTextView::ProcessMessage(OMessage *msg) {
  switch(msg->type) {
    case MSG_VSCROLL:
      switch(msg->action) {
        case MSG_SLIDERTRACK:
        case MSG_SLIDERPOS:
          OScrollBarMessage *sbmsg = (OScrollBarMessage *) msg;
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
