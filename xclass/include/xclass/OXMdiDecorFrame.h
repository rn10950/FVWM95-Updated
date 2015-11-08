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

#ifndef __OXMDIDECORFRAME_H
#define __OXMDIDECORFRAME_H

#include <xclass/OXCompositeFrame.h>
#include <xclass/OXMenu.h>
#include <xclass/OXPictureButton.h>
#include <xclass/OXIcon.h>
#include <xclass/OXLabel.h>
#include <xclass/OXFont.h>
#include <xclass/OXMdiMainFrame.h>
#include <xclass/OXMdiFrame.h>


// placement of window resizers

#define MDI_RESIZER_TOP		 1
#define MDI_RESIZER_BOTTOM	 2
#define MDI_RESIZER_LEFT	 4
#define MDI_RESIZER_RIGHT	 8

// border width of decorated windows

#define MDI_BORDER_WIDTH   5

// the width of minimized windows, in "height" units, "aspect ratio" would
// have been a better name for this...

#define MINIMIZED_WIDTH    5


class OXMdiMainFrame;
class OXMdiDecorFrame;
class OXMdiFrame;
class OXMdiTitleBar;
class OXMdiTitleIcon;
class OXMdiButtons;


//----------------------------------------------------------------------

class OXMdiWinResizer : public OXFrame {
public:
  OXMdiWinResizer(const OXWindow *p, const OXWindow *mdiwin, int pos,
                  const OXGC *boxGC, int linew,
                  int mdioptions = MDI_DEFAULT_RESIZE_MODE,
                  int w = 1, int h = 1, unsigned int options = OWN_BKGND);

  virtual int  HandleButton(XButtonEvent *event);
  virtual void DrawBorder() {};
  
  void SetResizeMode(int mode) { _mdioptions = mode; }
  void SetMinSize(int w = 50, int h = 20) { min_w = w; min_h = h; }

  friend class OXMdiMainFrame;

protected:
  void MoveResizeIt();
  void DrawBox(int x, int y, unsigned int width, unsigned int height);

  const OXWindow *_mdiwin;
  int winx, winy, winh, winw;
  int old_x, old_y, old_w, old_h;
  int new_x, new_y, new_w, new_h;
  int min_w, min_h;
  int _mdioptions;
  int _pos;
  int x0, y0;
  bool buttonLeftPressed, buttonRightPressed, buttonMiddlePressed;

  const OXGC *_boxGC;
  int _lnw;
};


class OXMdiVerticalWinResizer : public OXMdiWinResizer {
public:
  OXMdiVerticalWinResizer(const OXWindow *p, const OXWindow *mdiwin,
                          int pos, const OXGC *boxGC, int linew,
                          int mdioptions = MDI_DEFAULT_RESIZE_MODE,
                          int w = 4, int h = 5);

  virtual int  HandleMotion(XMotionEvent *event);
  virtual void DrawBorder();

};


class OXMdiHorizontalWinResizer : public OXMdiWinResizer {
public:
  OXMdiHorizontalWinResizer(const OXWindow *p, const OXWindow *mdiwin,
                            int pos, const OXGC *boxGC, int linew,
                            int mdioptions = MDI_DEFAULT_RESIZE_MODE,
                            int w = 5, int h = 4);

  virtual int  HandleMotion(XMotionEvent *event);
  virtual void DrawBorder();

};


class OXMdiCornerWinResizer : public OXMdiWinResizer {
public:
  OXMdiCornerWinResizer(const OXWindow *p, const OXWindow *mdiwin,
                        int pos, const OXGC *boxGC, int linew,
                        int mdioptions = MDI_DEFAULT_RESIZE_MODE,
                        int w = 20, int h = 20);

  virtual int  HandleMotion(XMotionEvent *event);
  virtual void DrawBorder();

};


//----------------------------------------------------------------------

class OXMdiButtons : public OXCompositeFrame {
public:
  OXMdiButtons(const OXWindow *p, const OXWindow *titlebar);
  virtual ~OXMdiButtons();

  friend class OXMdiTitleBar;

protected:
  OXPictureButton *_button1, *_button2, *_button3, *_button4, *_button5;
  OLayoutHints *_defaultHint, *_closeHint;
};


//----------------------------------------------------------------------

class OXMdiTitleIcon : public OXIcon {
public:
  OXMdiTitleIcon(const OXWindow *p, const OXWindow *titlebar, 
                 const OPicture *pic, int w, int h);
  virtual ~OXMdiTitleIcon();

  virtual int HandleButton(XButtonEvent *event);

  friend class OXMdiFrame;
  friend class OXMdiTitleBar;

protected:
  virtual void _DoRedraw();

  OXPopupMenu *_popup;
};


//----------------------------------------------------------------------

class OXMdiTitleBar : public OXCompositeFrame {
public:
  virtual ~OXMdiTitleBar();

  virtual int HandleButton(XButtonEvent *event);
  virtual int HandleDoubleClick(XButtonEvent *event);
  virtual int HandleMotion(XMotionEvent *event);
  virtual int HandleCrossing(XCrossingEvent *event);
  virtual int ProcessMessage(OMessage *msg);

  void SetTitleBarColors(unsigned int fore, unsigned int back, const OXFont *f);

  friend class OXMdiDecorFrame;
  friend class OXMdiMainFrame;

protected:
  OXMdiTitleBar(const OXWindow *p, const OXWindow *mdiwin, 
                const char *name = "Untitled");
  void LayoutButtons(unsigned int buttonmask, bool isMinimized,
                     bool isMaximized);

  void AddFrames(OXMdiTitleIcon *icon, OXMdiButtons *buttons);
  void RemoveFrames(OXMdiTitleIcon *icon, OXMdiButtons *buttons);

  const OXWindow *_mdiwin;
  OXMdiButtons *_buttons;
  OXMdiTitleIcon *_winIcon;
  OXLabel *_winName;
  OXCompositeFrame *_lframe, *_mframe,*_rframe;
  OLayoutHints *_lHint, *_leftHint, *_middleHint, *_rightHint;

  int x0, y0;
  bool buttonLeftPressed, buttonRightPressed, buttonMiddlePressed;
};


//----------------------------------------------------------------------

class OXMdiDecorFrame : public OXCompositeFrame {
public:
  OXMdiDecorFrame(OXMdiMainFrame *main, OXMdiFrame *frame, int w, int h,
                  const OXGC *boxGC, unsigned int options = 0,
	          unsigned long back = _defaultFrameBackground);
  virtual ~OXMdiDecorFrame();

  virtual int HandleMaskEvent(XEvent *event);

  virtual int HandleButton(XButtonEvent *event);
  virtual int HandleConfigureNotify(XConfigureEvent *event);

  virtual int  CloseWindow() { return _frame->CloseWindow(); }
  virtual void Layout();
 
  virtual void Move(int x, int y);
  virtual void MoveResize(int x, int y, int w, int h);

  void SetMdiButtons(unsigned long buttons);
  unsigned long GetMdiButtons() const { return _buttonMask; }

  void SetResizeMode(int mode = MDI_DEFAULT_RESIZE_MODE);

  void SetWindowName(const char *name);
  void SetWindowIcon(const OPicture *pic);
  const char *GetWindowName() {
    return (const char *)_titlebar->_winName->GetText()->GetString();
  }
  const OPicture *GetWindowIcon() {
    return _titlebar->_winIcon->GetPicture();
  }
  bool IsMinimized() const { return _isMinimized; }
  bool IsMaximized() const { return _isMaximized; }
  
  void SetDecorBorderWidth(int bw);

  const OXMdiFrame *GetMdiFrame() const { return _frame; }

  friend class OXMdiMainFrame;

protected:
  void SetGrab();
  void UnsetGrab();

  OXMdiFrame *_frame;
  OXMdiMainFrame *_mdiMainFrame;

  OXMdiVerticalWinResizer *_upperHR, *_lowerHR;
  OXMdiCornerWinResizer *_upperLeftCR, *_lowerLeftCR;
  OXMdiCornerWinResizer *_upperRightCR, *_lowerRightCR;
  OXMdiHorizontalWinResizer *_leftVR, *_rightVR;
  OXCompositeFrame *_lMenu, *_rMenu;
  OLayoutHints *_lHint, *_expandHint;

  unsigned long _buttonMask;
  OXMdiTitleBar *_titlebar;

  int _preResizeX, _preResizeY, _preResizeWidth, _preResizeHeight;
  int _minimizedX, _minimizedY;
  bool _isMinimized, _isMaximized, _minimizedUserPlacement;

  int _iconX, _iconY;
  OXIcon *_largeIcon;
};


#endif  // __OXMDIDECORFRAME_H
