/**************************************************************************

    This file is part of foxftp, a ftp client for fOX.
    Copyright (C) 1998, 1999, G. Matzka.

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

#ifndef __OXLOGVIEW_H
#define __OXLOGVIEW_H

#include <xclass/OXTransientFrame.h>

class OXFont;
class OXVScrollBar;
class OXPopupMenu;


//----------------------------------------------------------------------

#define NO_TYPE       1
#define SENT_TYPE     2
#define RECEIVE_TYPE  3

class OXLogFrame : public OXFrame {
public:
  OXLogFrame(OXWindow *p, int w, int h,
             unsigned int options = 0,
             unsigned int back = _defaultFrameBackground);
  virtual ~OXLogFrame();

  virtual ODimension GetDefaultSize() const { return ODimension(_w, _h); }

  void Clear();
  void SetTopLine(int new_top);
  int  GetTopLine() { return _top; }
  void AddLine(int type, char *line);
  int  GetLines() const { return _nlines; }
  int  GetVisibleLines() const { return (_h + _th - 1)/ _th; }

  virtual int HandleExpose(XExposeEvent *event) {
    DrawRegion(event->x, event->y, event->width, event->height);
    return true;
  }
  virtual int HandleGraphicsExpose(XGraphicsExposeEvent *event) {
    DrawRegion(event->x, event->y, event->width, event->height);
    return true;
  }

protected:
  void DrawRegion(int x, int y, int w, int h);
  void ScrollWindow(int new_top);

  OXFont *_font;
  GC _gc_received, _gc_sent;

  char **_lines;            // lines of text
  int *_types;
  int _th, _top, _nlines, _alines;
};

class OXLogView : public OXCompositeFrame {
public:
  OXLogView(OXWindow *parent, int w, int h, unsigned int options = 0,
            unsigned int back = _defaultFrameBackground);
  virtual ~OXLogView();

  void Clear() { _textCanvas->Clear(); Layout(); }
  int  AddLine(int type, char *line) {
    if (_do_log == false) return true;
    _textCanvas->AddLine(type, line);
    Layout();
    return true;
  }

  void AdjustScrollBar();

  virtual int HandleButton(XButtonEvent *event);
  virtual int ProcessMessage(OMessage *msg);
  virtual void Layout();
  virtual ODimension GetDefaultSize() const { return ODimension(_w, _h); }

protected:
  OXLogFrame *_textCanvas;
  OXVScrollBar *_vsb;

  OXPopupMenu *_popup;
  bool _do_log;
};


#endif  // __OXLOGVIEW_H
