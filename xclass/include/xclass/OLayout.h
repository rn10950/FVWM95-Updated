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

#ifndef __OLAYOUT_H
#define __OLAYOUT_H

#include <xclass/OBaseObject.h>
#include <xclass/ODimension.h>


//---- layout hints

#define LHINTS_NOHINTS     0
#define LHINTS_LEFT        (1<<0)
#define LHINTS_CENTER_X    (1<<1)
#define LHINTS_RIGHT       (1<<2)
#define LHINTS_TOP         (1<<3)
#define LHINTS_CENTER_Y    (1<<4)
#define LHINTS_BOTTOM      (1<<5)
#define LHINTS_EXPAND_X    (1<<6)
#define LHINTS_EXPAND_Y    (1<<7)
#define LHINTS_NORMAL      (LHINTS_LEFT | LHINTS_TOP)

#define LHINTS_TOPLEFT     (LHINTS_LEFT  | LHINTS_TOP)
#define LHINTS_TOPRIGHT    (LHINTS_RIGHT | LHINTS_TOP)
#define LHINTS_BOTTOMLEFT  (LHINTS_LEFT  | LHINTS_BOTTOM)
#define LHINTS_BOTTOMRIGHT (LHINTS_RIGHT | LHINTS_BOTTOM)

#define LHINTS_CENTER      (LHINTS_CENTER_X | LHINTS_CENTER_Y)
#define LHINTS_EXPAND_ALL  (LHINTS_EXPAND_X | LHINTS_EXPAND_Y)


class OXFrame;
class OXCompositeFrame;
class OLayoutHints;


//----------------------------------------------------------------------
// Temporarily public as we need to share this structure definition
// with the layout manager class

struct SListFrameElt {
  class OXFrame *frame;
  class OLayoutHints *layout;
  SListFrameElt *prev, *next;
};


//----------------------------------------------------------------------

class OLayoutHints : public OBaseObject {
public:
  OLayoutHints(unsigned long hints = LHINTS_NORMAL,
	       unsigned int padleft = 0, unsigned int padright  = 0, 
	       unsigned int padtop  = 0, unsigned int padbottom = 0) {
    _padleft = padleft; _padright  = padright;
    _padtop  = padtop;  _padbottom = padbottom;
    _layoutHints = hints;
  }
  OLayoutHints(OLayoutHints *h) {
    _padleft = h->GetPadLeft(); _padright  = h->GetPadRight();
    _padtop  = h->GetPadTop();  _padbottom = h->GetPadBottom();
    _layoutHints = h->GetLayoutHints();
  }
  OLayoutHints(OLayoutHints &h) {
    _padleft = h.GetPadLeft(); _padright  = h.GetPadRight();
    _padtop  = h.GetPadTop();  _padbottom = h.GetPadBottom();
    _layoutHints = h.GetLayoutHints();
  }

  unsigned long GetLayoutHints() const { return _layoutHints; }
  unsigned int  GetPadTop() const { return _padtop; }
  unsigned int  GetPadBottom() const { return _padbottom; }
  unsigned int  GetPadLeft() const { return _padleft; }
  unsigned int  GetPadRight() const { return _padright; }

protected:
  unsigned long _layoutHints;
  unsigned int  _padtop, _padbottom, _padleft, _padright;
};


//----------------------------------------------------------------------
// Frame layout manager
// Abstract class

class OLayoutManager : public OBaseObject {
public:
  virtual void Layout() = 0;
  virtual ODimension GetDefaultSize() const = 0;
};


// subclassing is changed so that:
// - only a instanciable class has attributes
// - we share the common attributes of Vertical & Horizontal layouts
// - in OTabLayout (OXTab.h) we modify the constructor
//   signature to enforce the use of a OXTab class


class OVerticalLayout : public OLayoutManager {
public:
  OVerticalLayout(class OXCompositeFrame *main);
  
  virtual void Layout();
  virtual ODimension GetDefaultSize() const;

protected:
  class OXCompositeFrame *_main; // container frame
  SListFrameElt **_list;         // list of frames to arrange
};

class OHorizontalLayout : public OVerticalLayout {
public:
  OHorizontalLayout(class OXCompositeFrame *main) :
    OVerticalLayout(main) {}
  
  virtual void Layout();
  virtual ODimension GetDefaultSize() const;
};

//--- The follwing two layout managers do not make use of OLayoutHints

class ORowLayout : public OVerticalLayout {
public:
  ORowLayout(class OXCompositeFrame *main, int s = 0) :
    OVerticalLayout(main) { sep = s; }

  virtual void Layout();
  virtual ODimension GetDefaultSize() const;

  int sep; // interval between frames
};

class OColumnLayout : public ORowLayout {
public:
  OColumnLayout(class OXCompositeFrame *main, int s = 0) :
    ORowLayout(main, s) {}
  
  virtual void Layout();
  virtual ODimension GetDefaultSize() const;
};


class OMatrixLayout : public OLayoutManager {
public:
  OMatrixLayout(class OXCompositeFrame *main, int r, int c, int s=0, int h=0);

  virtual void Layout();
  virtual ODimension GetDefaultSize() const;

  int sep; // interval between frames
  int hints;
  int rows, columns;

protected:
  class OXCompositeFrame *_main; // container frame
  SListFrameElt **_list;         // list of frames to arrange
};


#endif  // __OLAYOUT_H
