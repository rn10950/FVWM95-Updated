/**************************************************************************

    This file is part of xclass.
    Copyright (C) 1996-2000 Harald Radke, Hector Peraza.

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

#ifndef __OXVIEW_H
#define __OXVIEW_H

#include <stdio.h>

#include <X11/Xatom.h>

#include <xclass/OXCompositeFrame.h>
#include <xclass/OXScrollBar.h>
#include <xclass/ODimension.h>
#include <xclass/ORectangle.h>
#include <xclass/OResourcePool.h>


#define CANVAS_NO_SCROLL          0
#define CANVAS_SCROLL_HORIZONTAL  (1<<0)
#define CANVAS_SCROLL_VERTICAL    (1<<1)
#define CANVAS_SCROLL_BOTH        (CANVAS_SCROLL_HORIZONTAL | CANVAS_SCROLL_VERTICAL)

class OXViewCanvas;


//----------------------------------------------------------------------

// Base class for widgets that need to display portions of large Objects or
// data that would otherwise either require a lot of X windows or overflow
// the X coordinate space.

// The drawing is done onto a canvas (subclassed OXFrame with user input and
// X events redirected to the parent OXView frame).

// The OXView class handles a "virtual" coordinate space, the object that is
// to be (partially) displayed is measured in these coordinates. Some
// methods are provided to translate between the "virtual" and "physical"
// coordinate spaces.

// The scrollbars are displayed when necessary and the position and size of
// their sliders are automatically kept up to date.

// The drawing method uses virtual coordinates and has to check whether the
// specified rectangle is within the visible object area.

// All in all this class provides only some basic stuff, all exciting code
// has to be implemented in the particular subclasses


//----------------------------------------------------------------------

class OXView : public OXCompositeFrame, public OXWidget {
public:
  OXView(const OXWindow *p, int w, int h, int id,
         unsigned int options = SUNKEN_FRAME | DOUBLE_BORDER | OWN_BKGND,
         unsigned int sboptions = CANVAS_SCROLL_BOTH);
  virtual ~OXView();

  virtual void Clear();

  virtual void Layout();
  virtual int  ProcessMessage(OMessage *msg);
  virtual int  HandleButton(XButtonEvent *event);
  virtual int  HandleExpose(XExposeEvent *event);
  virtual int  HandleGraphicsExpose(XGraphicsExposeEvent *event);
  virtual void DrawBorder();

  void NeedRedraw(ORectangle area);

  virtual int DrawRegion(OPosition coord, ODimension size, int clear = True);

  void SetScrollValues(OPosition value) { _scrollValue = value; }
  void SetScrollOptions(int sboptions) { _sboptions = sboptions; Layout(); }

  // outside interface, just moves the scrollbars 
  // (the last generate a message that eventually causes a true scroll)
  // the scroll is done to the nearest _scrollValue units

  virtual void ScrollToPosition(OPosition newPos);
  
  // some shortcuts for scrolling
  
  void ScrollUp(int pixels)
    { ScrollToPosition(OPosition(_visibleStart.x, _visibleStart.y + pixels)); }
  void ScrollDown(int pixels)
    { ScrollToPosition(OPosition(_visibleStart.x, _visibleStart.y - pixels)); }
  void ScrollLeft(int pixels)
    { ScrollToPosition(OPosition(_visibleStart.x + pixels, _visibleStart.y)); }
  void ScrollRight(int pixels)
    { ScrollToPosition(OPosition(_visibleStart.x - pixels, _visibleStart.y)); }

  // methods to translate coordinates

  OPosition ToVirtual(OPosition coord)  const { return coord + _visibleStart; }
  OPosition ToPhysical(OPosition coord) const { return coord - _visibleStart; }

  virtual ODimension GetDefaultSize() const { return ODimension(_w, _h); }

  ODimension GetVirtualSize() const { return _virtualSize; }
  OPosition  GetScrollValues() const { return _scrollValue; }
  OPosition  GetScrollPosition() const { return _visibleStart; }
  int GetScrollOptions() const { return _sboptions; }
  const OXViewCanvas *GetCanvas() const { return _canvas; }

  void SetupBackgroundPic(const OPicture *pic);
  void SetupBackgroundColor(unsigned long color);

protected:
  virtual void _DoRedraw();
  virtual bool ItemLayout() { return False; }
  
  virtual void Scroll(OPosition pos);

  virtual void UpdateBackgroundStart();

  bool _clearExposedArea;        // whether any exposed area of the object
                                 // should be cleared before readrawing.
                                 // The default is False
  ORectangle _exposedRegion;     // the region to draw

  OPosition _visibleStart;       // the start of the visible area, in
                                 // virtual coordinates
  ODimension _virtualSize;       // the current virtual window size
  OPosition _scrollValue;

  OPosition _offset;             // space for headers and/or other additional
                                 // artifacts... in fact, this should be a
                                 // rectangle!
  unsigned long _docBgndColor;

  OXViewCanvas *_canvas;
  OXHScrollBar *_hsb;
  OXVScrollBar *_vsb;
  int _sboptions;
  
  GC _backGC;
};


//----------------------------------------------------------------------

class OXViewCanvas : public OXFrame {
public:
  OXViewCanvas(const OXView *p, int w, int h, unsigned int options = 0);

  virtual int HandleSelectionRequest(XSelectionRequestEvent *event)
    { return ((OXView *) _parent)->HandleSelectionRequest(event); }
  virtual int HandleSelectionClear(XSelectionClearEvent *event)
    { return ((OXView *) _parent)->HandleSelectionClear(event); }
  virtual int HandleSelection(XSelectionEvent *event)
    { return ((OXView *) _parent)->HandleSelection(event); }
  virtual int HandleButton(XButtonEvent *event)
    { return ((OXView *) _parent)->HandleButton(event); }
  virtual int HandleDoubleClick(XButtonEvent *event)
    { return ((OXView *) _parent)->HandleDoubleClick(event); }
  virtual int HandleTripleClick(XButtonEvent *event)
    { return ((OXView *) _parent)->HandleTripleClick(event); }
  virtual int HandleExpose(XExposeEvent *event)
    { _DoRedraw(); ((OXView *) _parent)->HandleExpose(event); return True; }
  virtual int HandleGraphicsExpose(XGraphicsExposeEvent *event)
    { _DoRedraw(); ((OXView *) _parent)->HandleGraphicsExpose(event); return True; }
  virtual int HandleCrossing(XCrossingEvent *event)
    { return ((OXView *) _parent)->HandleCrossing(event); }
  virtual int HandleMotion(XMotionEvent *event)
    { return ((OXView *) _parent)->HandleMotion(event); }
  virtual int HandleKey(XKeyEvent *event)
    { return ((OXView *) _parent)->HandleKey(event); }

  virtual void Reconfig() {}

  virtual Atom HandleDNDenter(Atom *typelist)
    { return ((OXView *) _parent)->HandleDNDenter(typelist); }
  virtual int  HandleDNDleave()
    { return ((OXView *) _parent)->HandleDNDleave(); }
  virtual Atom HandleDNDposition(int x, int y, Atom action, int xr, int yr)
    { return ((OXView *) _parent)->HandleDNDposition(x, y, action, xr, yr); }
  virtual int  HandleDNDdrop(ODNDdata *DNDdata)
    { return ((OXView *) _parent)->HandleDNDdrop(DNDdata); }
  virtual int  HandleDNDfinished()
    { return ((OXView *) _parent)->HandleDNDfinished(); }
  virtual ODNDdata *GetDNDdata(Atom dataType)
    { return ((OXView *) _parent)->GetDNDdata(dataType); }

};


#endif  // __OXVIEW_H
