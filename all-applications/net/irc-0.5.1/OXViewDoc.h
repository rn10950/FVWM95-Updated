#ifndef __OXVIEWDOC_H
#define __OXVIEWDOC_H

#include <stdio.h>

#include <xclass/OXClient.h>
#include <xclass/OXCompositeFrame.h>
#include <xclass/OXWidget.h>
#include <xclass/OXScrollBar.h>

#include "OViewDoc.h"

class OXViewDoc;
class OXGC;
class OXFont;


//----------------------------------------------------------------------

class OXViewDocFrame : public OXFrame, public OXWidget {
public:
  OXViewDocFrame(OXWindow *parent, int w, int h, unsigned int options,
              unsigned long back = _defaultDocumentBackground);
  virtual ~OXViewDocFrame();

  void SetDocument(OViewDoc *d) { _document = d; }
  void SetupBackground(unsigned long back);
  void SetupBackgroundPic(const char *name);
  void SetFont(OXFont *f);
  void SetFont(const char *fontname);
  const OXFont *GetFont() const { return _font; }
  void Clear();
  int  GetDocHeight() { return _document->GetHeight(); }
  int  GetDocWidth() { return _document->GetWidth(); }
  void HScrollWindow(int x);
  void VScrollWindow(int y);

  virtual int HandleExpose(XExposeEvent *event) {
    DrawRegion(event->x, event->y, event->width, event->height, True);
    return True;
  }
  virtual int HandleGraphicsExpose(XGraphicsExposeEvent *event) {
    DrawRegion(event->x, event->y, event->width, event->height, True);
    return True;
  }
  OXGC *GetGC() { return _gc; }

  virtual void Layout();

  friend class OXViewDoc;

protected:
  virtual void DrawRegion(int x, int y, int w, int h, int clear);

  OViewDoc *_document;

  OXGC *_backgc, *_gc;
  OXFont *_font;
  int _tx, _ty;
};

#define NO_HSCROLL    1
#define NO_VSCROLL    2
#define FORCE_HSCROLL 4
#define FORCE_VSCROLL 8

class OXViewDoc : public OXCompositeFrame {
public:
  OXViewDoc(const OXWindow *parent, OViewDoc *d,
            int w, int h, unsigned int options,
            unsigned long back = _defaultDocumentBackground);
  virtual ~OXViewDoc();

  virtual int HandleButton(XButtonEvent *event);
  virtual int ProcessMessage(OMessage *msg);

  virtual ODimension GetDefaultSize() const { return ODimension(_w, _h); }

  virtual void Layout();

  int  LoadFile(const char *fname);
  void Clear();
  void AdjustScrollbars();
  void SetScrollOptions(int o) { _scroll_options = o; }
  void SetupBackground(unsigned long back) { _canvas->SetupBackground(back); }
  void SetupBackgroundPic(const char *name) { _canvas->SetupBackgroundPic(name); }
  int  GetBackground() { return _background; }
  void SetFont(OXFont *f) { _canvas->SetFont(f); Redisplay(); }
  void SetFont(const char *fontname) { _canvas->SetFont(fontname); Redisplay(); }
  const OXFont *GetFont() const { return _canvas->GetFont(); }
  void ScrollUp();
  void Redisplay();

protected:
  unsigned long _background;

  OViewDoc *_document;
  OXViewDocFrame  *_canvas;
  OXHScrollBar *_hscrollbar;
  OXVScrollBar *_vscrollbar;

  int _scroll_options;
};


#endif  // __OXVIEWDOC_H
