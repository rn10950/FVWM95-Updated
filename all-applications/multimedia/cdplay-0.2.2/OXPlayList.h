/**************************************************************************

    This file is part of explorer95, a file manager for fvwm95.
    Copyright (C) 1996, 1997 David Barth, Hector Peraza.

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

#ifndef __OXPLAYLIST_H
#define __OXPLAYLIST_H

#include <X11/Xlib.h>

#include <xclass/utils.h>
#include <xclass/OXCompositeFrame.h>
#include <xclass/OXWindow.h>
#include <xclass/OXWidget.h>
#include <xclass/OXScrollBar.h>
#include <xclass/OXButton.h>
#include <xclass/OString.h>

#include "OMedium.h"

#define MAXLINES    5000
#define MARGIN      20

class OXScrolledPlayList;


//----------------------------------------------------------------------

class OXPlayList : public OXFrame, public OXWidget {

friend class OXScrolledPlayList;

protected:
  XFontStruct *font;
  GC _gc, _selgc, _backselgc;

public:
  OXPlayList(OXWindow *parent, int w, int h, OMedium *medium,
             unsigned int options, unsigned long back = _defaultFrameBackground);
  virtual ~OXPlayList();

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

  virtual int HandleButton(XButtonEvent *event);
  virtual int HandleMotion(XMotionEvent *event);

  int tracks[MAXLINES];   // tracks
  int firstselline, lastselline, selline;
  int status;

protected:
  void DrawRegion(int x, int y, int w, int h);
  void _DoRedraw() { DrawRegion(0, 0, _w, _h); }
  void ScrollWindow(int new_top);

  OMedium *omedium;
  int _th, nlines, top;
};

class OXScrolledPlayList : public OXCompositeFrame {
public:
  OXScrolledPlayList(const OXWindow *p, int w, int h,
                     OMedium *medium,
                     unsigned int options = HORIZONTAL_FRAME,
                     unsigned long back = _defaultFrameBackground);
  virtual ~OXScrolledPlayList();

  void Clear();
  void AddTrack(int track);
  void RemoveSelection();
  int GetFirstSelectedTrack();
  void GetPlayListFromMedium();
  void SavePlayListToMedium();

  virtual int ProcessMessage(OMessage *msg);
  virtual void Layout();
  virtual ODimension GetDefaultSize() const { return ODimension(_w, _h); }

protected:
  OXPlayList *_playlist;
  OXVScrollBar *_scrollbar;
};

#endif  // __OXPLAYLIST_H
