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

#ifndef __OXCOMPOSITEFRAME_H
#define __OXCOMPOSITEFRAME_H

#include <xclass/OXFrame.h>


//--------------------------------------------------------------------
// This class is the base class for composite widgets 
// (menu bars, list boxes, etc.).
//
// It provides:
//  - a layout manager
//  - a frame container (SListFrameElt *)

class OXCompositeFrame : public OXFrame {
public:
  OXCompositeFrame(const OXWindow *p, int w = 10, int h = 10,
		   unsigned int options = 0,
		   unsigned long back = _defaultFrameBackground);
  virtual ~OXCompositeFrame();

  virtual int IsComposite() const { return True; }

  virtual int GetDefaultWidth() const 
                 { return GetDefaultSize().w; }
  virtual int GetDefaultHeight() const
                 { return GetDefaultSize().h; }
  virtual ODimension GetDefaultSize() const 
                 { if (_layoutManager)
                     return _layoutManager->GetDefaultSize();
                   else
                     return ODimension(_w, _h); }

  virtual void MapSubwindows();
  virtual void Layout();
  
  virtual void Reconfig();

  virtual void ChangeOptions(unsigned int options);

  virtual OXFrame *GetFrameFromPoint(int x, int y);

  virtual int TranslateCoordinates(OXFrame *child, int x, int y,
                                   int *fx, int *fy);

  OLayoutManager *GetLayoutManager() const { return _layoutManager; }
  void SetLayoutManager(OLayoutManager *l);

  virtual void AddFrame(OXFrame *f, OLayoutHints *l = NULL);
  virtual void RemoveFrame(OXFrame *f);

  void ShowFrame(OXFrame *f);
  void HideFrame(OXFrame *f);

  SListFrameElt **GetListAddr() { return &_flist; }
  SListFrameElt **GetListTail() { return &_ftail; }

protected:
  SListFrameElt *_flist, *_ftail;

private:
  OLayoutManager *_layoutManager;
};


class OXVerticalFrame : public OXCompositeFrame {
public:
  OXVerticalFrame(const OXWindow *p, int w = 10, int h = 10,
		  unsigned int options = CHILD_FRAME,
                  unsigned long back = _defaultFrameBackground) :
    OXCompositeFrame(p, w, h, options | VERTICAL_FRAME, back) {}
};


class OXHorizontalFrame : public OXCompositeFrame {
public:
  OXHorizontalFrame(const OXWindow *p, int w = 10, int h = 10,
		  unsigned int options = CHILD_FRAME,
                  unsigned long back = _defaultFrameBackground) :
    OXCompositeFrame(p, w, h, options | HORIZONTAL_FRAME, back) {}
};


#endif  // __OXCOMPOSITEFRAME_H
