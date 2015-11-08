/**************************************************************************

    This file is part of xclass.
    Copyright (C) 1997-2002 Harald Radke, Hector Peraza.

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

#ifndef __OXRESIZER_H
#define __OXRESIZER_H

#include <xclass/utils.h>
#include <xclass/OXFrame.h>
#include <xclass/OXWidget.h>
#include <xclass/OMessage.h>


/**************************************************************************

  ok, here are two classes for resizing OX(Composite)Frames ...

  Usage
  -----

  - create the first frame to be resized and add it to it's parent
  - create the resizer (horizontal/vertical) add it to the parent
    (don't forget to LINTS_EXPAND_X/Y)
  - create the second frame and add it to the parent frame
  - call the SetPrev() and SetNext() methods of the resizer with the
    particular frame as argument.

  Notes
  -----

  - If you set the layout hints of both resizeable frames to EXPAND into
    the resizing direction (horizontal/vertical) you'll get funny results
    as on resizing the window will cause the layout manager to ajust the
    frame sizes
    But IMHO I think it is senseless to do that as YOU want to customize
    the frame sizes and not the manager ((:
  - The resizer itself now has no border anymore, so it's up to you to set
    the frames as SUNKEN_FRAMES in order to actually see the resizer
  - When using frames that are resized not by points but by multiple of a
    certain line height/column width (e.g. OXListBox), you may run into
    problems. Watch out if those widgets have modifiers for setting
    resizing modes (OXListBox: IntegralHeight(False) for resizing as
    usual).
	
  Resizer Modes
  -------------

  OPAQUE/NON_OPAQUE

    The way the frames are resized, whether in "real time" or after
    releasing the mouse button

  HIDDEN
 
    doesn't show the resizer...
    Why that ? well u can chain resizers in order to resize frames via
    resizer that doesn't have the same parent as the frames do...
    so u add the visible resizer as normal and add HIDDEN resizers between
    your target frames...(don't forget to chain them)
    Example is a modified version of OXListView (explorer) where u
    can resize the columns via the resizer in the header frame

  ACTIVATE/INACTIVE

    well, there might be situations where u don't want the resizer to be
    usable (.e.g. one of the frames is invisble) 


  Well, hope u can use these classes.

  Bug reports should go to: harryrat@kawo1.rwth-aachen.de

  Have fun,
    Harry


  ChangeLog
  ---------

  98/09/10   Fixed some older bugs

  98/09/08   Thanx to David Essex, he suggested to add a possible resizing
             restriction. From now on you can limit resizing of frames
             by passing the minimum size of a frame as second parameter
             to SetPrev()/SetNext().
             David also added new cursors for the resizers, more Win95-like.
             He also suggested to get rid of the 3d Line look of the resizers
             and changed the redraw methods.
             Dumb bug fixed that sometimes prevented frames from being
             resized in NON_OPAQUE mode

  99/05/21   Cleaned-up the code a bit, removed unnecessary XGrabButton
  (Hector)   calls.
             Modified the drawing routines so the resizer gets highlited
             immediately after a button is pressed.
             Allocated pixmaps are freed when they are no longer neccessary.

  00/09/00   Added support for an optional transparent handle (done
             through the use of an InputOnly window) that can be placed
             in places where the normal resizer would affect the
             visibility of other existing frames (for example, between
             the two header buttons in the OXListView widget).

**************************************************************************/


#define RESIZER_PRESSED		1
#define RESIZER_RELEASED	0

#define RESIZER_NON_OPAQUE	(1 << 1)
#define RESIZER_OPAQUE		(1 << 2)
#define RESIZER_HIDDEN		(1 << 3)
#define RESIZER_USE_HANDLE	(1 << 4)
#define RESIZER_AUTOMATIC	(1 << 5)

#define RESIZER_ACTIVE		1
#define RESIZER_INACTIVE	2


class OXResizerHandle;

//----------------------------------------------------------------------

class OResizerMessage : public OWidgetMessage {
public:
  OResizerMessage(int typ, int act, int wid, int p) :
    OWidgetMessage(typ, act, wid) { pos = p; }

  int pos;
};


//----------------------------------------------------------------------

class OXHorizontalResizer : public OXFrame, public OXWidget {
public:
  OXHorizontalResizer(const OXWindow *p, int wid = -1,
		      unsigned long resMode = RESIZER_NON_OPAQUE);
  ~OXHorizontalResizer();

  virtual int HandleButton(XButtonEvent *event);
  virtual int HandleMotion(XMotionEvent *event);

  void MoveResizeHandle(int x, int y, int w, int h);

  void SetState(int state) { _active = state; }
  void Motion(int x, int y);
  void SetPrev(OXFrame *prev, int min = 0) { _prev = prev; _min1 = min; }
  void SetNext(OXFrame *next, int min = 0);
  void SetRemoteResizer(OXHorizontalResizer *resizer) 
       { _remoteResizer = resizer; }

  virtual void DrawBorder();

  virtual void MapWindow();
  virtual void UnmapWindow();
  virtual void RaiseWindow();
  virtual void LowerWindow();
  virtual void MapRaised();

  virtual void Move(int x, int y);
  virtual void Resize(int w, int h);
  virtual void Resize(ODimension size) { OXFrame::Resize(size); }
  virtual void MoveResize(int x, int y, int w, int h);

protected:
  int _min1, _min2;
  int _state, _active;
  unsigned long _resMode;
  OXFrame *_prev, *_next;
  OXResizerHandle *_handle;
  OXHorizontalResizer *_remoteResizer;
  int px, py, hx, hy;
};

class OXVerticalResizer : public OXFrame, public OXWidget {
public:
  OXVerticalResizer(const OXWindow *p, int wid = -1,
                    unsigned long resMode = RESIZER_NON_OPAQUE);
  ~OXVerticalResizer();

  virtual int HandleButton(XButtonEvent *event);
  virtual int HandleMotion(XMotionEvent *event);

  void MoveResizeHandle(int x, int y, int w, int h);

  void SetState(int state) { _active = state; }
  void Motion(int x, int y);
  void SetPrev(OXFrame *prev, int min = 0) { _prev = prev; _min1 = min; }
  void SetNext(OXFrame *next, int min = 0);
  void SetRemoteResizer(OXVerticalResizer *resizer)
       { _remoteResizer = resizer; }

  virtual void DrawBorder();

  virtual void MapWindow();
  virtual void UnmapWindow();
  virtual void RaiseWindow();
  virtual void LowerWindow();
  virtual void MapRaised();

  virtual void Move(int x, int y);
  virtual void Resize(int w, int h);
  virtual void Resize(ODimension size) { OXFrame::Resize(size); }
  virtual void MoveResize(int x, int y, int w, int h);

protected:
  int _min1, _min2;
  int _state, _active;
  unsigned long _resMode;
  OXFrame *_prev, *_next;
  OXResizerHandle *_handle;
  OXVerticalResizer *_remoteResizer;
  int px, py, hx, hy;
};

//----------------------------------------------------------------------

class OXResizerHandle : public OXFrame {
public:
  OXResizerHandle(OXClient *c, const OXWindow *p, OXFrame *res);

  virtual int HandleButton(XButtonEvent *event)
    { return _resizer->HandleButton(event); }
  virtual int HandleMotion(XMotionEvent *event)
    { return _resizer->HandleMotion(event); }

  virtual void DrawBorder() {}

protected:
  virtual void _DoRedraw() {}
  
  OXFrame *_resizer;
};


#endif  // __OXRESIZER_H
