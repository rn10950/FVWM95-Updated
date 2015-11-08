/**************************************************************************
 
    This file is part of OXMdi, an extension to the xclass toolkit.
    Copyright (C) 1998-2002 by Harald Radke, Hector Peraza.
 
    This application is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
 
    This application is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.
 
    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 
**************************************************************************/

#ifndef __OXMDIMAINFRAME_H
#define __OXMDIMAINFRAME_H

#include <xclass/OXCanvas.h>
#include <xclass/OXMenu.h>
#include <xclass/OXFont.h>
#include <xclass/ORectangle.h>


// resizing modes

#define MDI_OPAQUE		 1
#define MDI_NON_OPAQUE		 2
#define MDI_DEFAULT_RESIZE_MODE  MDI_OPAQUE

// MDI messages actions

#define MDI_CURRENT		 1
#define MDI_CREATE		 2

// MDI hints, also used to identify titlebar buttons

#define MDI_CLOSE		 4
#define MDI_RESTORE		 8
#define MDI_MOVE		 16
#define MDI_SIZE                 32
#define MDI_MINIMIZE		 64
#define MDI_MAXIMIZE		 128
#define MDI_HELP		 256
#define MDI_MENU                 512

#define MDI_WINLISTSTART	 1000

#define MDI_DEFAULT_HINTS        (MDI_MENU | MDI_MINIMIZE | MDI_RESTORE | \
                                  MDI_MAXIMIZE | MDI_SIZE | MDI_CLOSE)

// window arrangement modes

#define MDI_TILE_HORIZONTAL	 1
#define MDI_TILE_VERTICAL	 2
#define MDI_CASCADE		 3

// geometry value masks for ConfigureWindow() call

#define MDI_CLIENT_GEOMETRY      (1<<0)
#define MDI_DECOR_GEOMETRY       (1<<2)
#define MDI_ICON_GEOMETRY        (1<<3)


class OXGC;
class OXMdiMenuBar;
class OXMdiContainer;
class OXMdiDecorFrame;
class OXMdiFrame;


//----------------------------------------------------------------------

class OMdiFrameList {
public:
  OXMdiDecorFrame *GetDecorFrame() const { return decor; }
  OMdiFrameList *GetPrev() const { return prev; }
  OMdiFrameList *GetNext() const { return next; }
  OMdiFrameList *GetCyclePrev() const { return cyclePrev; }
  OMdiFrameList *GetCycleNext() const { return cycleNext; }

  friend class OXMdiMainFrame;

protected:
  OXMdiDecorFrame *decor;
  XID frameid;
  OMdiFrameList *prev, *next;
  OMdiFrameList *cyclePrev, *cycleNext;
};

class OMdiGeometry {
public:
  ORectangle client, decoration, icon;
  int value_mask;
};


//----------------------------------------------------------------------

class OXMdiMainFrame : public OXCanvas {
public:
  OXMdiMainFrame(const OXWindow *p, OXMdiMenuBar *menu, int w, int h,
                 unsigned int options = 0,
                 unsigned long back = _defaultFrameBackground);
  virtual ~OXMdiMainFrame();

  virtual int HandleClientMessage(XClientMessageEvent *event);
  virtual int HandleKey(XKeyEvent *event);
  virtual int ProcessMessage(OMessage *msg);

  virtual void Layout();

  void FreeMove(OXMdiFrame *frame);
  void FreeSize(OXMdiFrame *frame);
  void Restore(OXMdiFrame *frame);
  void Maximize(OXMdiFrame *frame);
  void Minimize(OXMdiFrame *frame);
  int  Close(OXMdiFrame *frame);
  int  ContextHelp(OXMdiFrame *frame);

  void Cascade() { ArrangeFrames(MDI_CASCADE); }
  void TileHorizontal() { ArrangeFrames(MDI_TILE_HORIZONTAL); }
  void TileVertical() { ArrangeFrames(MDI_TILE_VERTICAL); }

  void ArrangeFrames(int mode);
  void ArrangeMinimized();
  
  void CirculateUp();
  void CirculateDown();
  
  OXMdiFrame *GetCurrent() const;
  OXMdiFrame *GetMdiFrame(XID id) const;
  bool SetCurrent(XID newcurrent);
  bool SetCurrent(OXMdiFrame *f);

  OXPopupMenu *GetWinListMenu() const { return _winListMenu; }
  OXMdiMenuBar *GetMenu() const { return _menuBar; }

  OMdiFrameList *GetWindowList(int current = False) const
      { return current ? _current : _children; }
  long GetNumberOfFrames() const { return _numberOfFrames; }
  
  void SetResizeMode(int mode = MDI_DEFAULT_RESIZE_MODE);
  
  ORectangle GetBBox() const;
  ORectangle GetMinimizedBBox() const;

  OMdiGeometry GetWindowGeometry(OXMdiFrame *f) const;
  void ConfigureWindow(OXMdiFrame *f, OMdiGeometry &geom);

  bool IsMaximized(OXMdiFrame *f);
  bool IsMinimized(OXMdiFrame *f);

  friend class OXMdiFrame;

protected: 
  void AddMdiFrame(OXMdiFrame *f);
  bool RemoveMdiFrame(OXMdiFrame *f);

  bool SetCurrent(OMdiFrameList *newcurrent);
  OXMdiDecorFrame *GetDecorFrame(XID id) const;
  OXMdiDecorFrame *GetDecorFrame(OXMdiFrame *frame) const;

  void UpdateWinListMenu();
  
  long _numberOfFrames;
  OXMdiMenuBar *_menuBar;
  OXFrame *_container;
  OXPopupMenu *_winListMenu;
  OMdiFrameList *_children;
  OMdiFrameList *_current;

  int _current_x, _current_y, _resizeMode;
  const OXFont *_fontCurrent, *_fontNotCurrent;
  Pixel bgCurrent, fgCurrent;
  Pixel bgNotCurrent, fgNotCurrent;
  
  OXGC *_boxGC;
};
  
  
//----------------------------------------------------------------------

class OXMdiContainer : public OXFrame {
public:
  OXMdiContainer(const OXMdiMainFrame *p, int w, int h,
                 unsigned int options = 0,
                 unsigned long back = _defaultFrameBackground);
  
  virtual int HandleConfigureNotify(XConfigureEvent *event);

  virtual ODimension GetDefaultSize() const;

protected:
  const OXMdiMainFrame *_main;
};


#endif  // __OXMDIMAINFRAME_H
