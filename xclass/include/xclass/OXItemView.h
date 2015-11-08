/**************************************************************************

    This file is part of xclass.
    Copyright (C) 1996-2000, Harald Radke, Hector Peraza.

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

#ifndef __OXITEMVIEW_H
#define __OXITEMVIEW_H

#include <vector>

#include <xclass/utils.h>
#include <xclass/OGC.h>
#include <xclass/OXView.h>


#define SCROLL_TOP        1
#define SCROLL_BOTTOM     2
#define SCROLL_LEFT       3
#define SCROLL_RIGHT      4


//----------------------------------------------------------------------

class OItemViewMessage : public OWidgetMessage {
public:
  OItemViewMessage(int type, int action, int wid, int buttonID = 0,
                   OPosition position = OPosition(0, 0)) :
    OWidgetMessage(type, action, wid) { button = buttonID, pos = position; }

  int button;
  OPosition pos;
};


//----------------------------------------------------------------------

class OXItemView;

class OItem : public OComponent {
public:
  OItem(const OXItemView *p, int id = -1);
  virtual ~OItem() {}

  const OXItemView *GetParent() const { return _parent; }
  
  int GetId() const { return _id; }
  void SetId(int id) { _id = id; }

  ODimension GetSize() const { return _size; }
  virtual ODimension GetDefaultSize() const { return _size; }
  OPosition GetPosition() const { return _position; }

  void Move(OPosition pos) { _position = pos; }
  void Resize(ODimension size) { _size = size; Layout(); }

  virtual void Select(bool state) { _isSelected = state; }
  bool IsSelected() const { return _isSelected; }

  virtual void Draw(OXWindow *w, OPosition pos) {}
  virtual void ShowFocusHilite(int onoff) { _focused = onoff; }
  virtual void Layout() {}

  friend class OXItemView;

protected:
  int _id;
  const OXItemView *_parent;
  ODimension _size;
  OPosition _position;
  bool _isSelected, _focused;
  GC _backGC;
};


//----------------------------------------------------------------------

// class derivated from OXView view that deals with a list of items... 

class OXItemView : public OXView {
public:
  OXItemView(const OXWindow *p, int w, int h, int id,
             unsigned int options = SUNKEN_FRAME | DOUBLE_BORDER | OWN_BKGND,
             unsigned int sboptions = CANVAS_SCROLL_BOTH);
  virtual ~OXItemView();

  virtual void Clear();
  virtual void Layout();

  virtual int DrawRegion(OPosition coord, ODimension size, int clear = True);

  // override the following if you don't like the default behaviour or need
  // a special mouse behaviour

  virtual int HandleButton(XButtonEvent *event);
  virtual int HandleDoubleClick(XButtonEvent *event);
  virtual int HandleMotion(XMotionEvent *event);
  virtual int HandleTimer(OTimer *t);

  virtual int AutoScroll(XMotionEvent *event);

  // Item handling

  virtual void AddItem(OItem *item);
  virtual void DeleteItem(unsigned int num);
  OItem *GetItem(unsigned int num);
  virtual void SelectItem(unsigned int num, bool state);
  std::vector <OItem *>GetSelectedItems() const { return _selectedItems; }
  unsigned int GetNumberOfItems() const { return _items.size(); }
  unsigned int NumSelected() const { return _selectedItems.size(); }
  OItem *GetItemByPos(OPosition pos, int *iret = NULL);

  // Selection handling

  bool HasSelection() const { return _hasSelection; }
  virtual void SelectAll();
  virtual void UnSelectAll();
  virtual void InvertSelection();
  virtual void SelectItem(OItem *item, bool state);
  virtual void SelectToPos(OPosition pos);
  virtual void DeleteSelection();

  virtual ODimension GetMaxItemSize() const { return _maxItemSize; }
  OXClient *GetClient() const { return _client; }  // this is not good...

protected:
  virtual void _GotFocus();
  virtual void _LostFocus();

  virtual bool ItemLayout() { return False; }
  virtual void CalcMaxItemSize();

  virtual void DrawItem(OItem *item, OPosition coord, ODimension size) 
    { item->Draw(_canvas, ToPhysical(item->GetPosition())); }

  virtual void DrawDragOutline();
  virtual void ShowFocusHilite(int onoff);

  // set if view is scrolled during selection dragging
  bool _isScrolling;
  OTimer *_scrollTimer;

  // can be SCROLL_TOP, SCROLL_BOTTOM, SCROLL_LEFT, SCROLL_RIGHT
  int _scrollDirection;

  // selection dragging is done
  bool _dragSelecting;
  OXGC *_lineGC;

  // the rectangular selection area
  OPosition _selDragStart;
  OPosition _selDragEnd;
  bool _hasSelection;

  // mouse position when selection dragging starts
  OPosition _selAnchor;
  OPosition _lastDragPos;
  int _anchorItemNo;
  OItem *_anchorItem;

  ODimension _maxItemSize;

  // default False, must be set to True if resizing affects item positioning
  bool _needItemLayout;

  // separation between items
  ODimension _itemSep;

  std::vector<OItem *> _items;
  std::vector<OItem *> _selectedItems;
};


#endif  // __OXITEMVIEW_H
