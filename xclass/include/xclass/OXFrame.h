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

#ifndef __OXFRAME_H
#define __OXFRAME_H

#include <X11/Xproto.h>

#include <xclass/OXWindow.h>
#include <xclass/ODimension.h>
#include <xclass/OXSList.h>
#include <xclass/OTimer.h>
#include <xclass/OLayout.h>


//---- frame states and flags

#define IS_VISIBLE        (1<<0)
#define IS_MAPPED         IS_VISIBLE
#define IS_ARRANGED       (1<<1)
#define IS_ENABLED        (1<<2)
#define TAKES_FOCUS       (1<<3)
#define HAS_FOCUS         (1<<4)
#define MDI_HAS_FOCUS     (1<<5)
#define HANDLES_TAB       (1<<6)
#define IS_DND_SOURCE     (1<<7)
#define IS_DND_TARGET     (1<<8)

//---- types of frames (and borders)

#define CHILD_FRAME       0
#define MAIN_FRAME        (1<<0)
#define VERTICAL_FRAME    (1<<1)
#define HORIZONTAL_FRAME  (1<<2)
#define SUNKEN_FRAME      (1<<3)
#define RAISED_FRAME      (1<<4)
#define DOUBLE_BORDER     (1<<5)
#define FIT_WIDTH         (1<<6)
#define FIXED_WIDTH       (1<<7)
#define FIT_HEIGHT        (1<<8)
#define FIXED_HEIGHT      (1<<9)
#define FIXED_SIZE        (FIXED_WIDTH | FIXED_HEIGHT)
#define OWN_BKGND         (1<<10)
#define MDI_MAIN_FRAME    (1<<11)
#define MDI_FRAME         (1<<12)
#define MDI_CHILD_FRAME   (MDI_FRAME)


//----------------------------------------------------------------------
// This class subclasses OXWindow, used as base class for some simple
// widgets (buttons, labels, etc.).
// It provides:
//  - position & dimension fields
//  - an 'options' attribute (see #defines above)
//  - a generic event handler
//  - a generic layout mechanism
//  - a generic border

class OXTip;
class OString;
class OResourcePool;
class ODNDdata;

class OXFrame : public OXWindow {
protected:
  static const OPicture *_defaultBackgroundPicture;
  static unsigned long _defaultFrameBackground;
  static unsigned long _defaultFrameHilite;
  static unsigned long _defaultFrameShadow;
  static unsigned long _defaultSelectedBackground;
  static unsigned long _defaultSelectedForeground;
  static unsigned long _defaultDocumentBackground;
  static unsigned long _defaultDocumentForeground;
  static unsigned long _whitePixel;
  static unsigned long _blackPixel;
  static GC _blackGC, _whiteGC;
  static GC _hilightGC;
  static GC _shadowGC;
  static GC _bckgndGC;
  static Time _lastclick;
  static unsigned int _lastbutton, _dbx, _dby, _dbw, _clickcount;
  static int _init;

public:
  OXFrame(const OXWindow *p, int w, int h,
          unsigned int options = 0,
          unsigned long back = _defaultFrameBackground);
  OXFrame(OXClient *c, Window id, OXWindow *parent = NULL);
  virtual ~OXFrame();

  const unsigned long GetEventMask() const { return _eventMask; }
  void AddInput(unsigned long emask);
  void RemoveInput(unsigned long emask);

  virtual int IsComposite() const { return False; }

  virtual void SetBackgroundColor(unsigned long color)
       { OXWindow::SetBackgroundColor(_backPixel = color); }

  virtual int HandleEvent(XEvent *event);
  virtual int HandleCreateNotify(XCreateWindowEvent * /*event*/) { return False; }
  virtual int HandleDestroyNotify(XDestroyWindowEvent *event);
  virtual int HandleConfigureNotify(XConfigureEvent *event);
  virtual int HandleMapNotify(XMapEvent * /*event */) { return False; }
  virtual int HandleUnmapNotify(XUnmapEvent * /*event */) { return False; }
  virtual int HandleButton(XButtonEvent * /*event*/); 
  virtual int HandleDoubleClick(XButtonEvent * /*event*/) { return False; }
  virtual int HandleTripleClick(XButtonEvent * /*event*/) { return False; }
  virtual int HandleCrossing(XCrossingEvent * /*event*/);
  virtual int HandleMotion(XMotionEvent * /*event*/) { return False; }
  virtual int HandleKey(XKeyEvent * /*event*/) { return False; }
  virtual int HandleFocusChange(XFocusChangeEvent *event);
  virtual int HandleClientMessage(XClientMessageEvent *event);
  virtual int HandleSelection(XSelectionEvent * /*event*/) { return False; }
  virtual int HandleSelectionRequest(XSelectionRequestEvent * /*event*/) { return False; }
  virtual int HandleSelectionClear(XSelectionClearEvent * /*event*/) { return False; }
  virtual int HandlePropertyChange(XPropertyEvent * /*event*/) { return False; }

  virtual void Move(int x, int y);
  virtual void Resize(int w, int h);
  virtual void Resize(ODimension size);
  virtual void MoveResize(int x, int y, int w, int h);
  virtual int  GetDefaultWidth() const { return GetDefaultSize().w; }
  virtual int  GetDefaultHeight() const { return GetDefaultSize().h; }
  virtual ODimension GetDefaultSize() const 
          { return ODimension(_w, _h); }

  virtual unsigned int GetOptions() const { return _options; }
  virtual void ChangeOptions(unsigned int options);
  virtual void SetOptions(unsigned int options);
  virtual void ClearOptions(unsigned int options);
      
  virtual void Layout() {}
  virtual void MapSubwindows() {}  // Simple frames do not have subwindows 
                                   // Redefine this in OXCompositeFrame!

  virtual void SetBorderStyle(unsigned int border_bits);

  virtual void DrawBorder();
  virtual void Reconfig();

  int GetWidth() const { return _w; }
  int GetHeight() const { return _h; }
  ODimension GetSize() const { return ODimension(_w, _h); }
  int GetX() const { return _x; }
  int GetY() const { return _y; }
  OPosition GetPosition() const { return OPosition(_x, _y); }
  int GetBorderWidth() const { return _bw; }

  OInsets GetInsets() const { return _insets; }
  void SetInsets(const OInsets &in) { _insets = in; }

  int Contains(int x, int y) const { return ((x >= 0) && (x < _w) &&
                                             (y >= 0) && (y < _h)); }

  virtual OXFrame *GetFrameFromPoint(int x, int y)
                   { return (Contains(x, y) ? this : NULL); }

  // Modifiers (without graphic update)
  void SetWidth(int w) { _w = w; }
  void SetHeight(int h) { _h = h; }
  void SetSize(const ODimension &s) { _w = s.w; _h = s.h; }

  void SetTip(char *text);
  void RemoveTip();

  void ShowTip();
  void HideTip();

  int TakesFocus() const { return _flags & TAKES_FOCUS; }
  int HasFocus() const { return _flags & HAS_FOCUS; }
  int HandlesTab() const { return _flags & HANDLES_TAB; }
  int IsEnabled() const { return _flags & IS_ENABLED; }
  int IsVisible() const;

  void SetVisible(int onoff)
       { if (onoff) _flags |= IS_VISIBLE; else _flags &= ~IS_VISIBLE; }
  void TakeFocus(int onoff) 
       { if (onoff) _flags |= TAKES_FOCUS; else _flags &= ~TAKES_FOCUS; }
  void Enable() { _flags |= IS_ENABLED; _Enable(True); }
  void Disable() { _flags &= ~IS_ENABLED; _Enable(False); }

  unsigned int GetFlags() const { return _flags; }

  // drag and drop...
  void SetDNDSource(int onoff)
       { if (onoff) _flags |= IS_DND_SOURCE; else _flags &= ~IS_DND_SOURCE; }
  void SetDNDTarget(int onoff)
       { if (onoff) _flags |= IS_DND_TARGET; else _flags &= ~IS_DND_TARGET; }
  int IsDNDSource() const { return _flags & IS_DND_SOURCE; }
  int IsDNDTarget() const { return _flags & IS_DND_TARGET; }

  virtual ODNDdata *GetDNDdata(Atom /*dataType*/) { return NULL; }
  virtual int  HandleDNDdrop(ODNDdata * /*DNDdata*/) { return False; }
  virtual Atom HandleDNDposition(int /*x*/, int /*y*/, Atom /*action*/,
                        int /*xroot*/, int /*yroot*/) { return None; }
  virtual Atom HandleDNDenter(Atom */*typelist*/) { return None; }
  virtual int  HandleDNDleave() { return False; }
  virtual int  HandleDNDfinished() { return False; }

  friend class OXClient;

protected:
  virtual void _DoRedraw();
  virtual void _Enable(int onoff) {}
  virtual void _GotFocus();
  virtual void _LostFocus() {}
  virtual void _Moved() {}
  virtual void _Resized() {}
  virtual void _OptionsChanged();
  
  virtual void _ComputeInsets();

  void _Draw3dRectangle(int type, int x, int y, int w, int h);

  const OResourcePool *GetResourcePool() const
    { return _client->GetResourcePool(); }

  int _x, _y, _w, _h, _bw;
  unsigned int _options, _flags;
  unsigned long _eventMask, _backPixel, _hilitePixel, _shadowPixel;
  int _compressConfigureEvents, _compressMotionEvents;

  OXTip *_tip;
  OInsets _insets;
};


#endif  // __OXFRAME_H
