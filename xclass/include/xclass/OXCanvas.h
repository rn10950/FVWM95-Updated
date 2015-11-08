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

#ifndef __OXCANVAS_H
#define __OXCANVAS_H

#include <X11/Xlib.h>

#include <xclass/utils.h>
#include <xclass/OXCompositeFrame.h>
#include <xclass/ODimension.h>
#include <xclass/OMessage.h>
#include <xclass/OXScrollBar.h>


#define CANVAS_NO_SCROLL          0
#define CANVAS_SCROLL_HORIZONTAL  (1<<0)
#define CANVAS_SCROLL_VERTICAL    (1<<1)
#define CANVAS_SCROLL_BOTH        (CANVAS_SCROLL_HORIZONTAL | CANVAS_SCROLL_VERTICAL)


//----------------------------------------------------------------------
// Not used directly by OXCanvas, but by derived classes...

class OContainerMessage : public OWidgetMessage {
public:
  OContainerMessage(int typ, int act, int wid,
                    int b = 0, int tot = 0, int sel = 0,
                    int xr = 0, int yr = 0) :
    OWidgetMessage(typ, act, wid) {
      button   = b;
      total    = tot;
      selected = sel;
      xroot    = xr;
      yroot    = yr;
  }

  int button, total, selected, xroot, yroot;
};


//----------------------------------------------------------------------
// This frame acts as the window through which we look at the
// contents of the container frame

class OXViewPort : public OXCompositeFrame {
public:
  OXViewPort(const OXWindow *p, int w, int h,
           unsigned int options = CHILD_FRAME,
           unsigned long back = _defaultFrameBackground);

  OXFrame *GetContainer() const { return _container; }
  void SetContainer(OXFrame *f);

  virtual void DrawBorder() {};
  virtual void Reconfig() { OXCompositeFrame::Reconfig(); _container->Reconfig(); }
  virtual void Layout() {}
  virtual ODimension GetDefaultSize() const { return ODimension(_w, _h); }
  
  void SetHPos(int xpos) { if (_container) _container->Move(_x0 = xpos, _y0); }
  void SetVPos(int ypos) { if (_container) _container->Move(_x0, _y0 = ypos); }
  void SetPos(int xpos, int ypos) { if (_container) _container->Move(_x0 = xpos, _y0 = ypos); }
  int  GetHPos() const { return _x0; }
  int  GetVPos() const { return _y0; }

protected:
  int _x0, _y0;
  OXFrame *_container;
};


//----------------------------------------------------------------------
// This is the scrollable canvas frame arrangement

class OXCanvas : public OXCompositeFrame {
public:
  OXCanvas(const OXWindow *p, int w, int h,
           unsigned int options = SUNKEN_FRAME | DOUBLE_BORDER,
           unsigned long back = _defaultFrameBackground);
  virtual ~OXCanvas();
  
  virtual int HandleButton(XButtonEvent *event);

  virtual void AddFrame(OXFrame *f, OLayoutHints *l);
  OXFrame *GetContainer() const { return _vport->GetContainer(); }
  OXViewPort *GetViewPort()  const { return _vport; }
  virtual void SetContainer(OXFrame *f) { _vport->SetContainer(f); }
  virtual void Layout();
  virtual ODimension GetDefaultSize() const { return ODimension(_w, _h); }

  virtual int ProcessMessage(OMessage *msg);

  void SetScrolling(int scrolling);
  int  GetScrolling() const { return _scrolling; }
  void SetVPos(int pos) { _vscrollbar->SetPosition(pos); }
  void SetHPos(int pos) { _hscrollbar->SetPosition(pos); }

  void SetScrollDelay(int pre = SB_DF_PREDELAY, int rpt = SB_DF_RPTDELAY) {
    _vscrollbar->SetDelay(pre, rpt);
    _hscrollbar->SetDelay(pre, rpt);
  }
  void SetScrollMode(int mode = SB_NORMAL) {
    _vscrollbar->SetMode(mode);
    _hscrollbar->SetMode(mode);
  }

protected:
  OXViewPort   *_vport;
  OXHScrollBar *_hscrollbar;
  OXVScrollBar *_vscrollbar;

  int _scrolling;
};


#endif  // __OXCANVAS_H
