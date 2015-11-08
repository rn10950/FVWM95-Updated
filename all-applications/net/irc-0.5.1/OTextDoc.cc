#include <unistd.h>

#include <xclass/OXFont.h>

#include "OTextDoc.h"
#include "OXViewDoc.h"
#include "OXPreferences.h"

extern OSettings *foxircSettings;


//----------------------------------------------------------------------

OTextDoc::OTextDoc() {
  _w = _h = 0;
  _lines = NULL;
}

OTextDoc::~OTextDoc() {
  Clear();
}

void OTextDoc::CreateCanvas(OXViewDoc *p) {
  _textFrame = new OXViewDocFrame(p, 1, 1, CHILD_FRAME);
  _textFrame->SetDocument(this);
  _canvas = _textFrame;
  _mainWidget = p;
}

void OTextDoc::SetCanvas(OXViewDocFrame *c) {
  _textFrame = c;
  _canvas = c;
}

void OTextDoc::Clear() {
  _lines = NULL;
  _nlines = 0;
  _w = _h = 0;
}

int OTextDoc::AddLine(OLineDoc *line) {
  if (_lines)
    _lines->InsertBefore(line);
  else
    _lines = line;

  ++_nlines;

  line->Layout();
  _h += line->GetHeight();
  _mainWidget->AdjustScrollbars();

  return True;
}

int OTextDoc::LoadFile(FILE *fp) {
  OLineDoc *line;

  if (fp == NULL) return False;

  Clear();

  // Read each line of the file into the buffer.

  line = new OLineDoc();
  line->SetCanvas(_textFrame);
  while (line->LoadFile(fp)) {
    AddLine(line);

    line = new OLineDoc();
    line->SetCanvas(_textFrame);
  }

  return True;
}

void OTextDoc::Layout() {
  OLineDoc *i;

  _h = 0;
  i = _lines;
  if (i) {
    do {
      i->Layout();
      _h += i->GetHeight();
      i = i->next;
    } while (i != _lines);
  }
}

void OTextDoc::DrawRegion(Display *dpy, Drawable d,
                          int x, int y, XRectangle *rect) {
  OLineDoc *i;
  OFontMetrics fm;

  foxircSettings->GetFont()->GetFontMetrics(&fm);

  int lh = fm.ascent + fm.descent;
  int yloc = y;

  i = _lines;
  if (i) {
    do {
      if ((yloc - lh <= rect->y + rect->height) &&
          (yloc + i->GetHeight() >= rect->y))
        i->DrawRegion(dpy, d, x, yloc, rect);
      yloc += i->GetHeight();
      i = i->next;
    } while (i != _lines);
  }
}
