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

#ifndef __OXTAB_H
#define __OXTAB_H

#include <xclass/OXCompositeFrame.h>
#include <xclass/OXWidget.h>
#include <xclass/ODimension.h>
#include <xclass/OMessage.h>


//----------------------------------------------------------------------

class OTabMessage : public OWidgetMessage {
public:
  OTabMessage(int typ, int act, int wid, int t) :
    OWidgetMessage(typ, act, wid) { tab = t; }

  int tab;
};


//----------------------------------------------------------------------
// Tab widget

class OXTabElt : public OXFrame {
public:
  OXTabElt(const OXWindow *p, OString *text, int w, int h,
              GC norm, const OXFont *font,
              unsigned int options = RAISED_FRAME,
              unsigned long back = _defaultFrameBackground);
  virtual ~OXTabElt();

  virtual void DrawBorder();
  virtual ODimension GetDefaultSize() const;
  const   OString *GetText() const { return _text; }
  void    ShowFocusHilite(int onoff);

protected:
  OString *_text;
  GC _normGC;
  const OXFont *_font;
  int _tw, _th, _yt, _focused;
};

class OTabLayout : public OLayoutManager {
public:
  OTabLayout(class OXTab *main);
  
  virtual void Layout();
  virtual ODimension GetDefaultSize() const;

protected:
  class OXTab *_main;
  SListFrameElt **_list;
};

class OXTab : public OXCompositeFrame, public OXWidget {
protected:
  static const OXFont *_defaultFont;
  static const OXGC *_defaultGC;
  static int _init;

public:
  OXTab(const OXWindow *p, int w, int h, int ID = -1,
        unsigned int options = CHILD_FRAME,
        unsigned long back = _defaultFrameBackground);
  virtual ~OXTab();

  virtual OXCompositeFrame *AddTab(OString *text);
  virtual void RemoveTab(int tabnum);
  virtual void RemoveTab(char *tabname);
  virtual void SelectTab(int tabnum, int sendmsg = False);
  virtual void DrawBorder();
  virtual int  HandleButton(XButtonEvent *event);
  virtual int  HandleKey(XKeyEvent *event);

  OXCompositeFrame *GetContainer() const { return _container; }
  int GetCurrent() const { return _current; }
  int GetTabNumber(char *name) const;
  int GetTabHeight() const { return _tabh; }
  int GetNumTabs() const { return _numtabs; }
  
protected:
  virtual void _GotFocus();
  virtual void _LostFocus();
  void _ShowFocusHilite();

  int _numtabs, _current, _tabh;
  OXCompositeFrame *_container;
  const OXFont *_font;
  OXGC *_normGC;
};


#endif  // __OXTAB_H
