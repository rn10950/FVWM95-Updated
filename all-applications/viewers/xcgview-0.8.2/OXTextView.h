#ifndef __OXTEXTVIEW_H
#define __OXTEXTVIEW_H


#include <stdio.h>

#include <xclass/OXClient.h>
#include <xclass/OXCompositeFrame.h>
#include <xclass/OXScrollBar.h>


#define MAXLINESIZE 300
#define MAXLINES    5000
#define MARGIN      5


class OXTextFrame : public OXFrame {
protected:
  XFontStruct *font;
  GC _gc;

public:
  OXTextFrame(OXWindow *parent, int w, int h, unsigned int options,
              unsigned long back = _defaultFrameBackground);
  virtual ~OXTextFrame();

  int  AddText(char *text);
  void Clear();
  int  GetLines() const { return nlines; }
  int  GetVisibleLines() const { return (_h / _th); }
  void SetTopLine(int new_top);

  virtual int HandleExpose(XExposeEvent *event) {
    DrawRegion(event->x, event->y, event->width, event->height);
    return True;
  }
  virtual int HandleGraphicsExpose(XGraphicsExposeEvent *event) {
    DrawRegion(event->x, event->y, event->width, event->height);
    return True;
  }

protected:
  void DrawRegion(int x, int y, int w, int h);
  void ScrollWindow(int new_top);

  char *chars[MAXLINES];   // lines of text
  int lnlen[MAXLINES];     // length of each line
  int _th, nlines, top;
};

class OXTextView : public OXCompositeFrame {
public:
  OXTextView(OXWindow *parent, int w, int h, unsigned int options,
             unsigned long back = _defaultFrameBackground);
  virtual ~OXTextView();

  int  AddText(char *text);
  void Clear();
  virtual int ProcessMessage(OMessage *msg);
  virtual void Layout();
  virtual ODimension GetDefaultSize() const { return ODimension(_w, _h); }

  const OXTextFrame *GetTextFrame() const { return _textCanvas; }

protected:
  OXTextFrame  *_textCanvas;
  OXVScrollBar *_vsb;
};


#endif  // __OXTEXTVIEW_H
