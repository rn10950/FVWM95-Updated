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

#ifndef __OXMENU_H
#define __OXMENU_H

#include <X11/Xlib.h>

#include <xclass/OXCompositeFrame.h>
#include <xclass/OString.h>
#include <xclass/OTextBuffer.h>
#include <xclass/OPicture.h>
#include <xclass/OMessage.h>


//--- Menu entry status mask

#define MENU_ACTIVE_MASK   (1<<0)
#define MENU_ENABLE_MASK   (1<<1)
#define MENU_DEFAULT_MASK  (1<<2)
#define MENU_CHECKED_MASK  (1<<3)
#define MENU_RCHECKED_MASK (1<<4)

//--- Menu entry types

#define MENU_SEPARATOR    0
#define MENU_ENTRY        1
#define MENU_POPUP        2


class OXPopupMenu;

#define OMenuMessage OWidgetMessage


//----------------------------------------------------------------------

class OMenuEntry : public OBaseObject {
public:
  OMenuEntry(int type, int id = -1,
             OHotString *s = NULL, const OPicture *p = NULL);
  virtual ~OMenuEntry();

//  virtual void DrawEntry(...);

  friend class OXPopupMenu;

protected:
  OHotString *_label;
  const OPicture *_pic;
  int _status;
  int _type;
  int _entryID;
  int _ex, _ey;
  int _hkeycode;
  OXPopupMenu *_popup;
  OMenuEntry *next, *prev;
};

class OXPopupMenu : public OXFrame {
protected:
  static const OXGC *_defaultGC, *_defaultSelGC, *_defaultSelBckgndGC;
  static const OXFont *_defaultFont;
  static const OXFont *_hilightFont;
  static int _init;

public:
  OXPopupMenu(const OXWindow *p, int w = 8, int h = 6,
	      unsigned int options = 0);
  virtual ~OXPopupMenu();

  virtual int HandleButton(XButtonEvent *event);
  virtual int HandleMotion(XMotionEvent *event);
  virtual int HandleKey(XKeyEvent *event);
  virtual int HandleTimer(OTimer *t);

  virtual ODimension GetDefaultSize() const;

  void AddEntry(OHotString *s, int ID, const OPicture *p = NULL);
  void AddSeparator(int ID = -1);
  void AddPopup(OHotString *s, OXPopupMenu *popup, int ID = -2);
  void RemoveEntry(int ID, int type = MENU_ENTRY);
  void RemoveAllEntries();
  void EnableEntry(int ID, int enabled = True);
  void DisableEntry(int ID);
  int  IsEntryEnabled(int ID);
  void SetDefaultEntry(int ID);
  void CheckEntry(int ID, int checked = True);
  void UnCheckEntry(int ID);
  int  IsEntryChecked(int ID);
  void RCheckEntry(int ID, int IDfirst, int IDlast);
  int  IsEntryRChecked(int ID);
  int  RenameEntry(int ID, OHotString *s);
  int  SetEntryPic(int ID, const OPicture *p = NULL);
  void PlaceMenu(int x, int y, int stick_mode, int grab_pointer);
  int  PopupMenu(int x, int y);
  int  EndMenu();
  void SetSidePic(const OPicture *pic, unsigned long bgnd_pixel);

  virtual void Activate(OMenuEntry *entry, int delayed = True);
  virtual void DrawBorder();
  virtual void Reconfig();

  friend class OXMenuTitle;

protected:
  virtual void _DoRedraw();
  virtual void DrawEntry(OMenuEntry *entry);

  void DrawTrianglePattern(GC gc, int l, int t, int r, int b);
  void DrawCheckMark(GC gc, int l, int t, int r, int b);
  void DrawRCheckMark(GC gc, int l, int t, int r, int b);
  void AdjustSize();
  void AdjustEntries();

  OMenuEntry *_first, *_last, *_current;
  OXGC *_normGC, *_selGC, *_selbackGC;
  const OXFont *_font, *_hifont;
  int _stick, _hasgrab, _popdown, _lastID;
  int _xl, _xr;
  int _mw, _mh;    // temp!
  OTimer *_delay;
  const OPicture *_sidePic;
  unsigned long _sideBgnd;
};

class OXMenuTitle : public OXFrame {
protected:
  static const OXGC *_defaultGC, *_defaultSelGC, *_defaultSelBckgndGC;
  static const OXFont *_defaultFont;
  static const OXFont *_hilightFont;
  static int _init;

public:
  OXMenuTitle(const OXWindow *p, OHotString *s, OXPopupMenu *menu, 
	      unsigned int options = 0);
  ~OXMenuTitle();

  virtual void SetState(int state);
  virtual void DoSendMessage();
  virtual int  GetState() const { return _state; }
  virtual int  GetHotKeyCode() const { return _hkeycode; }

  virtual int  HandleButton(XButtonEvent *event);
  virtual int  HandleMotion(XMotionEvent *event);
  virtual int  HandleKey(XKeyEvent *event);
  
  virtual void Reconfig();
  
protected:
  virtual void _DoRedraw();

  OXGC *_normGC, *_selGC, *_selbackGC;
  const OXFont *_font;
  OXPopupMenu *_menu;
  OHotString *_label;
  int _ID, _state, _hkeycode;
};

class OXMenuBar : public OXHorizontalFrame {
public:
  OXMenuBar(const OXWindow *p, int w, int h,
	    unsigned int options = HORIZONTAL_FRAME | RAISED_FRAME);
  virtual ~OXMenuBar();

  virtual void AddPopup(OHotString *s, OXPopupMenu *menu, OLayoutHints *l);

  virtual int HandleButton(XButtonEvent *event);
  virtual int HandleMotion(XMotionEvent *event);
  virtual int HandleKey(XKeyEvent *event);

protected:
  OXMenuTitle *_current;
  int _x0, _y0, _stick;
};


#endif  // __OXMENU_H
