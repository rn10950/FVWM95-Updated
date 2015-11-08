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

#ifndef __ONYXTEXTCANVAS_H
#define __ONYXTEXTCANVAS_H

/* Modified from OXTextView of the fileview application originaly written by Hector Peraza */

// ==!== TODO: use xclass' OXTextEdit?

#include <stdio.h>

#include <xclass/OXClient.h>
#include <xclass/OXCompositeFrame.h>
#include <xclass/OXScrollBar.h>
#include <xclass/OXFont.h>
#include <xclass/OGC.h>

#include "OnyxBase.h"


#define MAXLINESIZE 300
#define MAXLINES    5000
#define MARGIN      5


//----------------------------------------------------------------------

class OXTextFrame : public OXFrame {
public:
  OXTextFrame(OXWindow *parent, int w, int h, unsigned int options,
              unsigned long back = _defaultFrameBackground);
  virtual ~OXTextFrame();

  int  AddText(char *Text);
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
  OXFont *_font;
  OXGC *_gc;
};

class OXTextView : public OXCompositeFrame {
public:
  OXTextView(OXWindow *parent, int w, int h, unsigned int options,
             unsigned long back = _defaultFrameBackground);
  virtual ~OXTextView();

  int  AddText(char *Text);
  void Clear();

  virtual int ProcessMessage(OMessage *msg);
  virtual void Layout();
  virtual ODimension GetDefaultSize() const { return ODimension(_w, _h); }

  const OXTextFrame *GetTextFrame() const { return _textCanvas; }

protected:
  OXTextFrame  *_textCanvas;
  OXVScrollBar *_vsb;
};


//----------------------------------------------------------------------

class OnyxTextCanvas : public OnyxObject {
public:
  OnyxTextCanvas();
  virtual ~OnyxTextCanvas();

  virtual int  Create();
  virtual void Move(int, int);
  virtual void Resize(int, int);
  virtual void MoveResize(int, int, int, int);
  virtual int  AddText(char *Text);
  virtual void Clear();

private:
  OXCompositeFrame *InternalFrame;
  OXTextView *InternalTextView;
};


#endif  // __ONYXTEXTCANVAS_H
