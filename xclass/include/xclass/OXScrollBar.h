/**************************************************************************

    This file is part of xclass, a Win95-looking GUI toolkit.
    Copyright (C) 1996, 1997 David Barth, Hector Peraza.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

**************************************************************************/

#ifndef __OXSCROLLBAR_H
#define __OXSCROLLBAR_H

#include <xclass/OXWidget.h>
#include <xclass/OXFrame.h>
#include <xclass/ODimension.h>


//--- Scroll bar types, modes, etc.

#define SB_HORIZONTAL     0
#define SB_VERTICAL       1

#define SB_TRACK          0
#define SB_JUMP           1

#define SB_NORMAL         0
#define SB_ACCELERATED    1

#define SB_DF_PREDELAY    400
#define SB_DF_RPTDELAY    50

#define SB_WIDTH          16


//----------------------------------------------------------------------

class OScrollBarMessage : public OWidgetMessage {
public:
  OScrollBarMessage(int typ, int act, int wid, int p) :
    OWidgetMessage(typ, act, wid) { pos = p; }

  int pos;
};


//----------------------------------------------------------------------
// Scrollbar element for heads and thumb

class OXScrollBarElt : public OXFrame {
public:
  OXScrollBarElt(const OXWindow *p, const OPicture *pic, int w, int h,
              unsigned int options = RAISED_FRAME | DOUBLE_BORDER,
              unsigned long back = _defaultFrameBackground) :
    OXFrame(p, w, h, options | OWN_BKGND, back) {_pic = pic; _state = False;}

  virtual void SetState(int state);
  virtual void DrawBorder();
  virtual void Reconfig();

protected:
  int _state;
  const OPicture *_pic;
};


//----------------------------------------------------------------------
// Abstract scrollbar class

class OXScrollBar : public OXFrame, public OXWidget {
protected:
  static Pixmap _bckgndPixmap;
  static const OPicture *_arrow_left, *_arrow_right, *_arrow_up, *_arrow_down;
  static int _init, _sb_width;

public:
  OXScrollBar(const OXWindow *p, int w, int h,
              unsigned int options = CHILD_FRAME,
              unsigned long back = _defaultFrameBackground);
  virtual ~OXScrollBar();

  virtual void DrawBorder() {}
  virtual int  HandleButton(XButtonEvent *event) = 0;
  virtual int  HandleMotion(XMotionEvent *event) = 0;
  virtual int  HandleTimer(OTimer *t) = 0;
  virtual void Layout() = 0;
  virtual void Reconfig();

  virtual void SetRange(int range, int page_size, int sendMsg = True) = 0;
  virtual void SetDelay(int pre = SB_DF_PREDELAY, int repeat = SB_DF_RPTDELAY);
  virtual void SetMode(int mode = SB_NORMAL) { _mode = mode; }
  virtual void SetPosition(int pos, int sendMsg = True) = 0;
  virtual int  GetPosition() { return _pos; } // ?

  virtual void MapSubwindows() { OXWindow::MapSubwindows(); }

protected:
  int _x0, _y0, _xp, _yp, _dragging;
  int _range, _psize, _pos, _slsize, _slrange;
  int _mode, _incr, _preDelay, _rptDelay;
  OXScrollBarElt *_head, *_tail, *_slider;
  const OPicture *_headpic, *_tailpic;
  OTimer *_rpt;
  Window _subw;
};

class OXHScrollBar : public OXScrollBar {
public:
  OXHScrollBar(const OXWindow *p, int w, int h,
                unsigned int options = HORIZONTAL_FRAME,  
                unsigned long back = _defaultFrameBackground);
  virtual ~OXHScrollBar();

  virtual int HandleButton(XButtonEvent *event);
  virtual int HandleMotion(XMotionEvent *event);
  virtual int HandleTimer(OTimer *t);
  virtual ODimension GetDefaultSize() const
                     { return ODimension(_w, _sb_width); }
  virtual void Layout();

  virtual void SetRange(int range, int page_size, int sendMsg = True);
  virtual void SetPosition(int pos, int sendMsg = True);
};

class OXVScrollBar : public OXScrollBar {
public:
  OXVScrollBar(const OXWindow *p, int w, int h,
               unsigned int options = VERTICAL_FRAME,
               unsigned long back = _defaultFrameBackground);
  virtual ~OXVScrollBar();
      
  virtual int HandleButton(XButtonEvent *event);
  virtual int HandleMotion(XMotionEvent *event);
  virtual int HandleTimer(OTimer *t);
  virtual ODimension GetDefaultSize() const 
                     { return ODimension(_sb_width, _h); }
  virtual void Layout();

  virtual void SetRange(int range, int page_size, int sendMsg = True);
  virtual void SetPosition(int pos, int sendMsg = True);
};


#endif  // __OXSCROLLBAR_H
