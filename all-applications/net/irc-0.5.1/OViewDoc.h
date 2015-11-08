#ifndef __OVIEWDOC_H
#define __OVIEWDOC_H

#include <X11/Xlib.h>

#include <xclass/OBaseObject.h>

class OXViewDocFrame;
class OXViewDoc;


//----------------------------------------------------------------------

class OViewDoc : public OBaseObject {
public:
  OViewDoc() { _nlines = 0; }
  virtual ~OViewDoc() {}

  virtual void CreateCanvas(OXViewDoc *p);
  virtual void SetCanvas(OXViewDocFrame *c);
  OXViewDocFrame *GetCanvas() { return _canvas; }

  virtual int  LoadFile(FILE *file) { return false; }
  virtual void Clear() {}
  int GetHeight() const { return _h; }
  int GetWidth() const { return _w; }
  virtual int SetHeight(int h) { return False; }
  virtual int SetWidth(int w) { return False; }

  virtual void Layout() {}

  virtual void DrawRegion(Display *dpy, Drawable d,
                          int x, int y, XRectangle *rect) {}

  unsigned long NumLines() { return _nlines; }

protected:
  OXViewDocFrame *_canvas;
  int _h, _w;
  unsigned long _nlines;
};


#endif  // __OVIEWDOC_H
