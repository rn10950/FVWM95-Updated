#ifndef __OTEXTDOC_H
#define __OTEXTDOC_H

#include <stdio.h>

#include <X11/Xlib.h>

#include "OLineDoc.h"

#define MAXLINES    50000

class OXViewDocFrame;
class OXViewDoc;


//----------------------------------------------------------------------

class OTextDoc : public OViewDoc {
public:
  OTextDoc();
  virtual ~OTextDoc();

  void CreateCanvas(OXViewDoc *p);
  void SetCanvas(OXViewDocFrame *c);

  int SetWidth(int w) { _w = w; return True; }

  void Clear();
  int AddLine(OLineDoc *line);
  int LoadFile(FILE *file);

  void Layout();
  void DrawRegion(Display *dpy, Drawable d, int x, int y, XRectangle *rect);

  OXViewDocFrame *GetTextFrame() const { return _textFrame; }

protected:
  OLineDoc *_lines;          // lines of text
  OXViewDoc *_mainWidget;
  OXViewDocFrame *_textFrame;
};


#endif  // __OTEXTDOC_H
