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

#include <xclass/utils.h>
#include <xclass/OXItemView.h>

#define SCROLL_DELAY 50

#define LV_CLEAR   0
#define LV_SET     1
#define LV_INVERT  2


//----------------------------------------------------------------------

OItem::OItem(const OXItemView *p, int id) : OComponent() {
  _parent = p;
  _id = id;
  _client = _parent->GetClient();
  _size = ODimension(0, 0);
  _position = OPosition(0, 0);
  _isSelected = _focused = False;
}


//---------------------------------------------------------------------------

OXItemView::OXItemView(const OXWindow *p, int w, int h, int id,
		       unsigned int options, unsigned int sboptions) :
  OXView(p, w, h, id, options, sboptions) {

  _needItemLayout = False;
  _hasSelection = False;
  _itemSep = ODimension(0, 0);

  // GC for the selection box outlining...
  XGCValues gcv;
  unsigned long gcm;

  unsigned int _fore = GetResourcePool()->GetDocumentFgndColor();
  unsigned int _back = GetResourcePool()->GetDocumentBgndColor();

  gcm = GCForeground | GCBackground | GCFunction | GCFillStyle |
      GCLineWidth | GCLineStyle | GCSubwindowMode | GCGraphicsExposures;
  gcv.foreground = _fore ^ _back;
  gcv.background = _back;
  gcv.function = GXxor;
  gcv.line_width = 0;
  gcv.line_style = LineOnOffDash;
  gcv.fill_style = FillSolid;
  gcv.subwindow_mode = IncludeInferiors;
  gcv.graphics_exposures = False;
  _lineGC = new OXGC(GetDisplay(), _id, gcm, &gcv);

  XSetDashes(GetDisplay(), _lineGC->GetGC(), 0, "\x1\x1", 2);

  _anchorItem = NULL;
  _isScrolling = False;
  _dragSelecting = False;
  _scrollTimer = NULL;

  _msgType = MSG_ITEMVIEW;
  _widgetType = "OXItemView";

  AddInput(FocusChangeMask);

  XGrabButton(GetDisplay(), AnyButton, AnyModifier, _canvas->GetId(), False,
              EnterWindowMask | LeaveWindowMask |
              ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
              GrabModeAsync, GrabModeAsync, None, None);
}

OXItemView::~OXItemView() {
  // do not call Clear() here!
  for (int i = 0; i < _items.size(); ++i) delete _items[i];
  _items.clear();
  _selectedItems.clear();
  _anchorItem = NULL;
  if (_scrollTimer) delete _scrollTimer;
}

void OXItemView::_GotFocus() {
  OXFrame::_GotFocus();
  AddInput(KeyPressMask | KeyReleaseMask);
  ShowFocusHilite(True);
}

void OXItemView::_LostFocus() {
  OXFrame::_LostFocus();
  RemoveInput(KeyPressMask | KeyReleaseMask);
  ShowFocusHilite(False);

  OItemViewMessage message(_msgType, MSG_FOCUSLOST, _widgetID);
  SendMessage(_msgObject, &message);
}

void OXItemView::ShowFocusHilite(int onoff) {
  if (_anchorItem) {
    _anchorItem->ShowFocusHilite(onoff);
    //NeedRedraw(ORectangle(_anchorItem->GetPosition(), _anchorItem->GetSize()));
    //DrawRegion(_anchorItem->GetPosition(), _maxItemSize, False);
    DrawRegion(_anchorItem->GetPosition(), _anchorItem->GetSize(), False);
  }
#if 1
  else if (onoff && (_items.size() > 0)) {
    _anchorItem = _items[0];
    _anchorItem->ShowFocusHilite(onoff);
    //NeedRedraw(ORectangle(_anchorItem->GetPosition(), _anchorItem->GetSize()));
    DrawRegion(_anchorItem->GetPosition(), _anchorItem->GetSize(), False);
  }
#endif
}

void OXItemView::Layout() {
  bool needRedraw;

  if (_needItemLayout) {
    needRedraw = ItemLayout();
    if (needRedraw)
      NeedRedraw(ORectangle(_visibleStart, _canvas->GetSize()));
  }

  OXView::Layout();
}

void OXItemView::Clear() {
  _items.clear();
  _selectedItems.clear();
  _hasSelection = False;
  _anchorItem = NULL;
  OXView::Clear();
}

void OXItemView::SelectAll() {

  if (_items.size() == 0) return;

  for (unsigned int i = 0; i < _items.size(); i++)
    SelectItem(_items[i], True);
}

void OXItemView::UnSelectAll() {

  if (_items.size() == 0) return;

  for (unsigned int i = 0; i < _items.size(); i++)
    if (_items[i]->IsSelected())
      SelectItem(_items[i], False);

  _selectedItems.clear();
  _hasSelection = False;
}

void OXItemView::InvertSelection() {

  if (_items.size() == 0) return;

  _hasSelection = False;
  _selectedItems.clear();

  for (unsigned int i = 0; i < _items.size(); i++)
    SelectItem(_items[i], !_items[i]->IsSelected());
}

void OXItemView::DeleteSelection() {
  int i;
  OItem *item;

  if (!_hasSelection) return;

  for (i = 0; i < _items.size(); ++i) {
    item = _items[i];
    if (item->IsSelected()) {
      _items.erase(_items.begin() + i);  //_items.erase(&_items[i]);
      delete item;
      --i;
    }
  }

  _selectedItems.clear();
  _hasSelection = False;
  _anchorItem = NULL;

  CalcMaxItemSize();
  _needItemLayout = True;
  Layout();
  NeedRedraw(ORectangle(_visibleStart, _canvas->GetSize()));

  OItemViewMessage message(_msgType, MSG_SELCHANGED, _id);
  SendMessage(_msgObject, &message);
}

void OXItemView::AddItem(OItem *newItem) {

  if (newItem == NULL) return;

  newItem->_backGC = _backGC;
  newItem->Move(OPosition(0, _virtualSize.h));
  _items.push_back(newItem);

  _virtualSize.h += newItem->GetDefaultSize().h + _itemSep.h;
#if 0
  CalcMaxItemSize();
#else
  ODimension itemSize = newItem->GetDefaultSize();
  newItem->Resize(itemSize);
  if (_maxItemSize.w < itemSize.w) _maxItemSize.w = itemSize.w;  
  if (_maxItemSize.h < itemSize.h) _maxItemSize.h = itemSize.h;
#endif
  _virtualSize.w = _maxItemSize.w;
#if 0
  Layout();
#endif

  if (!_needItemLayout)		// "manual" item display
    NeedRedraw(ORectangle(newItem->GetPosition(), newItem->GetDefaultSize()));
}

void OXItemView::DeleteItem(unsigned int num) {
  unsigned int i;
  OPosition pos;
  ODimension size;

  if (_items.size() <= num) return;

  pos = _items[num]->GetPosition();
  size = _items[num]->GetSize();
  if (_hasSelection) {
    for (i = 0; i < _selectedItems.size(); i++)
      if (_selectedItems[i] == _items[num]) {
	_selectedItems.erase(_selectedItems.begin() + i);
	break;
      }
  }

  for (i = num + 1; i < _items.size(); i++)
    _items[i]->Move(_items[i]->GetPosition() - OPosition(0, pos.y + size.h));

  if (_anchorItem == _items[num]) _anchorItem = NULL;

  delete _items[num];
  _items.erase(_items.begin() + num);
  _virtualSize.h -= size.h - _itemSep.h;
  CalcMaxItemSize();
  Layout();

  if (!_needItemLayout)		// screen area from deleted item might need refresh
    NeedRedraw(ORectangle(_visibleStart.x, _visibleStart.y,
                          _canvas->GetWidth() - ToPhysical(pos).x,
                          _canvas->GetHeight() - ToPhysical(pos).y));

}

OItem *OXItemView::GetItem(unsigned int num) {

  if (_items.size() <= num) return NULL;

  return _items[num];
}

void OXItemView::SelectItem(OItem *item, bool state) {
  unsigned int i;

  if (item == NULL) return;

  // there are things that should be done only when item changes
  // state...

  if (state != item->IsSelected()) {
    if (state) {
      _selectedItems.push_back(item);
    } else {
      for (i = 0; i < _selectedItems.size(); i++)
	if (_selectedItems[i] == item) {
	  _selectedItems.erase(_selectedItems.begin() + i);
	  break;
	}
    }
    item->Select(state);
#if 0
    _hasSelection = state;
    if (!state) {
      for (i = 0; i < _items.size(); i++)
        _hasSelection = _hasSelection | _items[i]->IsSelected();
    }
#else
    if (state) {
      _hasSelection = state;
    } else {
      _hasSelection = (_selectedItems.size() > 0);
    }
#endif
  }

  // can't use NeedRedraw here...
  DrawRegion(item->GetPosition(), _maxItemSize, False);
}

void OXItemView::SelectItem(unsigned int num, bool state) {

  if (_items.size() <= num) return;

  SelectItem(_items[num], state);
}


void OXItemView::CalcMaxItemSize() {
  _maxItemSize = ODimension(0, 0);

  if (_items.size() > 0) {
    for (unsigned int i = 0; i < _items.size(); i++) {
#if 0
      ODimension itemSize = _items[i]->GetDefaultSize();
#else
      ODimension itemSize = _items[i]->GetSize();
#endif
      if (_maxItemSize.w < itemSize.w) _maxItemSize.w = itemSize.w;
      if (_maxItemSize.h < itemSize.h) _maxItemSize.h = itemSize.h;
    }
  }
}

OItem *OXItemView::GetItemByPos(OPosition coord, int *iret) {
  OPosition itemPos;
  ODimension itemSize;

  if (_items.size() == 0) return NULL;

  for (unsigned int i = 0; i < _items.size(); i++) {
    itemPos = _items[i]->GetPosition();
    itemSize = _items[i]->GetSize();
#if 1
    if (((itemPos.y <= coord.y) && (itemPos.y + itemSize.h >= coord.y))
	&& ((itemPos.x <= coord.x) && (itemPos.x + itemSize.w >= coord.x))) {
#else
    if (((itemPos.y <= coord.y) && (itemPos.y + itemSize.h > coord.y))
	&& ((itemPos.x <= coord.x) && (itemPos.x + itemSize.w > coord.x))) {
#endif
      if (iret) *iret = i;
      return _items[i];
    }
  }

  return NULL;
}


int OXItemView::HandleButton(XButtonEvent *event) {
  OItem *selItem;
  bool selChange = False;
  int inv, add;

  if (OXView::HandleButton(event)) return True;  // wheel-mouse scroll

  if (event->type == ButtonPress) {

    if (TakesFocus()) RequestFocus();

    inv = event->state & ControlMask;
    add = event->state & ShiftMask;

    _selAnchor = ToVirtual(OPosition(event->x, event->y));
    _lastDragPos = _selAnchor;
    _selDragStart = _selAnchor;
    _selDragEnd = _selDragStart;

    selItem = GetItemByPos(_selAnchor);

    if ((event->button == Button1) || !selItem || !selItem->IsSelected()) {
      if (!inv && !add && _hasSelection) {
        selChange = True;
        UnSelectAll();
      }
    }

    if (selItem) {
      if (inv) {
        if (selItem->IsSelected())
          SelectItem(selItem, False);
        else
          SelectItem(selItem, True);
        selChange = True;
      } else {
        if (!selItem->IsSelected()) {
          SelectItem(selItem, True);
          selChange = True;
        }
      }
    }

    ShowFocusHilite(False);
    if (selItem) _anchorItem = selItem;
    ShowFocusHilite(HasFocus());

    if (selChange) {
      OItemViewMessage message(_msgType, MSG_SELCHANGED, _widgetID);
      SendMessage(_msgObject, &message);
    }

    if (_selectedItems.size() == 0) {
      _dragSelecting = True;
      DrawDragOutline();
    }

#if 0
    if (_selectedItems.size() > 0) {
      OItemViewMessage message(_msgType, MSG_CLICK, _widgetID, event->button,
                               OPosition(event->x_root, event->y_root));
      SendMessage(_msgObject, &message);
    }
#endif

  } else {  // ButtonRelease

    if (_isScrolling) _isScrolling = False;
    if (_scrollTimer) delete _scrollTimer;
    _scrollTimer = NULL;
    if (_dragSelecting) {
      _dragSelecting = False;
      DrawDragOutline();
    }

    OItemViewMessage message(_msgType, MSG_CLICK, _widgetID, event->button,
                             OPosition(event->x_root, event->y_root));
    SendMessage(_msgObject, &message);

  }

  return True;
}

int OXItemView::HandleDoubleClick(XButtonEvent *event) {
  if (event->button != Button1) return False;

  if (_selectedItems.size() > 0) {
    OItemViewMessage message(_msgType, MSG_DBLCLICK, _widgetID, event->button,
                             OPosition(event->x_root, event->y_root));
    SendMessage(_msgObject, &message);
  }
  return True;
}

int OXItemView::HandleMotion(XMotionEvent *event) {
//  if (AutoScroll(event)) return True;
  AutoScroll(event);

  if (_dragSelecting) DrawDragOutline();
  SelectToPos(ToVirtual(OPosition(event->x, event->y)));
  if (_dragSelecting) DrawDragOutline();

  return True;
}

void OXItemView::DrawDragOutline() {
  OPosition start = ToPhysical(_selDragStart);
  OPosition end = ToPhysical(_selDragEnd);

  start.y = max(start.y, 0);
  start.x = max(start.x, 0);
  start.y = min(start.y, _canvas->GetHeight());
  start.x = min(start.x, _canvas->GetWidth());

  end.y = max(end.y, 0);
  end.x = max(end.x, 0);
  end.y = min(end.y, _canvas->GetHeight());
  end.x = min(end.x, _canvas->GetWidth());

  _canvas->DrawRectangle(_lineGC->GetGC(),
                         start.x, start.y, end.x - start.x, end.y - start.y);
}

void OXItemView::SelectToPos(OPosition mousePos) {
  int selected, selChange = False;
  OItem *item;
  OPosition itemPos;
  ODimension itemSize;

  if (_items.size() == 0) return;

  if (_dragSelecting) {
    _selDragStart.x = min(_selAnchor.x, mousePos.x);
    _selDragEnd.x = max(_selAnchor.x, mousePos.x);
    _selDragStart.y = min(_selAnchor.y, mousePos.y);
    _selDragEnd.y = max(_selAnchor.y, mousePos.y);

    selected = 0;
    for (unsigned int i = 0; i < _items.size(); i++) {
      item = _items[i];
      itemPos = item->GetPosition();
      itemSize = item->GetSize();
      if ((itemPos.x < _selDragEnd.x) &&
          (itemPos.x + itemSize.w > _selDragStart.x) &&
          (itemPos.y < _selDragEnd.y) &&
          (itemPos.y + itemSize.h > _selDragStart.y)) {
	++selected;
	if (!item->IsSelected()) {
	  SelectItem(item, True);
          selChange = True;
        }
      } else if (item->IsSelected()) {
	SelectItem(item, False);
        selChange = True;
      }
    }
    if (selChange) {
      OItemViewMessage message(_msgType, MSG_SELCHANGED, _widgetID);
      SendMessage(_msgObject, &message);
    }
  }
  _lastDragPos = mousePos;
}

// We won't use HandleCrossing for auto scrolling, since we could
// be leaving the canvas window and entering another some where ontop
// of the canvas, in which case the algorithm gets confused. Besides,
// using 

int OXItemView::AutoScroll(XMotionEvent *event) {
  if (event->window != _canvas->GetId()) return False;

  _isScrolling = False;

  if (((event->state & Button1Mask) != 0) && (_dragSelecting)) {

    if (event->x < 0) {
      _scrollDirection = SCROLL_LEFT;
      _isScrolling = True;
    } else if (event->x >= _canvas->GetWidth()) {
      _scrollDirection = SCROLL_RIGHT;
      _isScrolling = True;
    }

    if (event->y < 0) {
      _scrollDirection = SCROLL_TOP;
      _isScrolling = True;
    } else if (event->y >= _canvas->GetHeight()) {
      _scrollDirection = SCROLL_BOTTOM;
      _isScrolling = True;
    }

  }

  if (_isScrolling) {
    if (!_scrollTimer) _scrollTimer = new OTimer(this, SCROLL_DELAY);
  } else {
    if (_scrollTimer) delete _scrollTimer;
    _scrollTimer = NULL;
  }

  return _isScrolling;
}

int OXItemView::HandleTimer(OTimer *t) {
  OPosition newPos = _visibleStart;

  if (t != _scrollTimer) return True;
  delete _scrollTimer;

  switch (_scrollDirection) {
  case SCROLL_TOP:
    newPos.y -= _scrollValue.y *10;
    if (newPos.y <= 0) {
      newPos.y = 0;
      _isScrolling = False;
    }
    _lastDragPos = ToVirtual(OPosition(ToPhysical(_lastDragPos).x, 0));
    break;

  case SCROLL_BOTTOM:
    newPos.y += _scrollValue.y *10;
    if (newPos.y >= _virtualSize.h - _canvas->GetHeight()) {
      newPos.y = _virtualSize.h - _canvas->GetHeight();
      _isScrolling = False;
    }
    _lastDragPos = ToVirtual(OPosition(ToPhysical(_lastDragPos).x,
                                                  _canvas->GetHeight()));
    break;

  case SCROLL_LEFT:
    newPos.x -= _scrollValue.x *10;
    if (newPos.x <= 0) {
      newPos.x = 0;
      _isScrolling = False;
    }
    _lastDragPos = ToVirtual(OPosition(0, ToPhysical(_lastDragPos).y));
    break;

  case SCROLL_RIGHT:
    newPos.x += _scrollValue.x *10;
    if (newPos.x >= _virtualSize.w - _canvas->GetWidth()) {
      newPos.x = _virtualSize.w - _canvas->GetWidth();
      _isScrolling = False;
    }
    _lastDragPos = ToVirtual(OPosition(_canvas->GetWidth(),
                                       ToPhysical(_lastDragPos).y));
    break;
  }

  if (_dragSelecting) DrawDragOutline();
  ScrollToPosition(newPos);
  SelectToPos(_lastDragPos);
  if (_dragSelecting) DrawDragOutline();

  if (_isScrolling) {
    _scrollTimer = new OTimer(this, SCROLL_DELAY);
  } else {
    _scrollTimer = NULL;
  }

  return True;
}

int OXItemView::DrawRegion(OPosition coord, ODimension size, int clear) {

  OPosition pcoord = ToPhysical(coord);

  // check whether the rectangle is within the visible area

  if ((pcoord.y + (int) size.h < 0) || (pcoord.y > _canvas->GetHeight()))
    return False;

  if ((pcoord.x + (int) size.w < 0) || (pcoord.x > _canvas->GetWidth()))
    return False;

  // clip the size argument to the canvas size

  if (pcoord.x < 0) {
    if (-pcoord.x > (int) size.w) return False;
    coord.x += -pcoord.x;
    size.w -= -pcoord.x;
    pcoord.x = 0;
  }

  if (pcoord.y < 0) {
    if (-pcoord.y > (int) size.h) return False;
    coord.y += -pcoord.y;
    size.h -= -pcoord.y;
    pcoord.y = 0;
  }

  if (pcoord.x + (int) size.w > _canvas->GetWidth())
    size.w = _canvas->GetWidth() - pcoord.x;

  if (pcoord.y + (int) size.h > _canvas->GetHeight())
    size.h = _canvas->GetHeight() - pcoord.y;

  // clear parts that are outside the object

  OXView::DrawRegion(coord, size, clear);

  // well, no items, no jobs

  if (_items.size() == 0) return False;

  // check whether the area is inside the virtual window

  if ((coord.y > _virtualSize.h) || (coord.x > _virtualSize.w)) return False;

  // ok, at this point it is almost for sure that there has to be
  // redisplayed something

  for (unsigned int i = 0; i < _items.size(); ++i) {
    if ((_items[i]->GetPosition().y < coord.y + size.h) &&
        (_items[i]->GetPosition().y + _items[i]->GetSize().h > coord.y) &&
        (_items[i]->GetPosition().x < coord.x + size.w) &&
        (_items[i]->GetPosition().x + _items[i]->GetSize().w > coord.x)) {
      DrawItem(_items[i], coord, size);
    }
  }

  return True;
}
