/**************************************************************************

    This file is part of xclass.
    Copyright (C) 1998-2002 Hector Peraza.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

**************************************************************************/

#include <xclass/OXTextButton.h>
#include <xclass/OXShutter.h>

  
//--------------------------------------------------------------------

OXShutter::OXShutter(const OXWindow* p, int id, unsigned int options) :
  OXCompositeFrame(p, 10, 10, options), OXWidget(id, "OXShutter") {

    _msgObject = p;

    _selectedItem = NULL;
    _closingItem  = NULL;
    _heightIncrement = 1;
    _closingHeight = 0;
    _closingHadScrollbar = False;
    _timer = NULL;
    _lh = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y);
}

OXShutter::~OXShutter() {
  if (_timer) delete _timer;
  delete _lh;
}

void OXShutter::AddItem(OXShutterItem *item) {
//  AddFrame(item, NULL);
  AddFrame(item, _lh);
}

void OXShutter::InsertItem(OXShutterItem *item, int afterID) {
  OXShutterItem *i;
  SListFrameElt *nw, *ptr;

  for (ptr = _flist; ptr != NULL; ptr = ptr->next) {
    i = (OXShutterItem *) ptr->frame;
    if (i->WidgetID() == afterID) break;
  }

  if (ptr == NULL) {
    AddFrame(item, _lh);
  } else {
    nw = new SListFrameElt;
    nw->frame = item;
    nw->layout = _lh;
    if (afterID == -1) {
      _flist->prev = nw;
      nw->next = _flist;
      nw->prev = NULL;
      _flist = nw;
    } else {
      nw->next = ptr->next;
      ptr->next = nw;
      nw->prev = ptr;
      if (nw->next == NULL) _ftail = nw;
    }
  }
  //item->MapWindow();
  //Layout();
}

void OXShutter::RemoveItem(int id) {
#if 0
  int update;
  OXShutterItem *item;

  item = FindItem(id);
  if (!item) return;

  update = (item == _selectedItem);

  RemoveFrame(item);
  item->DestroyWindow();
  delete item;

  if (update) _selectedItem = NULL;

  Layout();
#else
  register SListFrameElt *ptr, *prev = NULL;
  OXShutterItem *child, *item = NULL;

  for (ptr = _flist; ptr != NULL; prev = ptr, ptr = ptr->next) {
    child = (OXShutterItem *) ptr->frame;
    if (child->WidgetID() == id) {
      item = child;
      break;
    }
  }

  if (!item) return;

  int update = (item == _selectedItem);

  RemoveFrame(item);
  item->DestroyWindow();
  delete item;

  if (update) {
    if (prev)
      _selectedItem = (OXShutterItem *) prev->frame;
    else
      _selectedItem = NULL;
  }

  Layout();
#endif
}

OXShutterItem *OXShutter::Select(int id, int animate) {
  OXShutterItem *item;

  item = FindItem(id);
  if (!item) return NULL;

  if (!_selectedItem) _selectedItem = (OXShutterItem *) _flist->frame;
  if (_selectedItem == item) return item;

  if (animate) {
    _heightIncrement = 1;
    _closingItem = _selectedItem;
    _closingHeight = _closingItem->GetHeight();
    _closingHeight -= _closingItem->_button->GetDefaultHeight();
    _selectedItem = item;
    _timer = new OTimer(this, 6); //10);
  } else {
    _closingItem = NULL;
    _closingHeight = 0;
    _selectedItem = item;
    Layout();
  }

  return item;
}

OXShutterItem *OXShutter::FindItem(int id) {
  register SListFrameElt *ptr;
  OXShutterItem *child;

  for (ptr = _flist; ptr != NULL; ptr = ptr->next) {
    child = (OXShutterItem *) ptr->frame;
    if (child->WidgetID() == id) return child;
  }

  return NULL;
}

int OXShutter::ProcessMessage(OMessage *msg) {
  OXShutterItem *item;

  if (msg->type != MSG_BUTTON) return False;
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;

  Select(wmsg->id);

  OShutterMessage smsg(MSG_SHUTTER, MSG_TABCHANGED, _widgetID, wmsg->id);
  SendMessage(_msgObject, &smsg);

  return True;
}


//--- Shutter Item Animation

int OXShutter::HandleTimer(OTimer *t) {
  if (!_closingItem) return 0;
  _closingHeight -= _heightIncrement;
  _heightIncrement += 5;
  if (_closingHeight > 0) {
    _timer = new OTimer(this, 6); //10);
  } else {
    _closingItem = NULL;
    _closingHeight = 0;
    _timer = NULL;
  }
  Layout();

  return 1;   
}

void OXShutter::Layout() {
  register SListFrameElt *ptr;
  OXShutterItem *child;
  int y, bh, exh;

  if (!_flist) return;

  if (_selectedItem == NULL) _selectedItem = (OXShutterItem*) _flist->frame;

  exh = _h - _insets.t - _insets.b;
  for (ptr = _flist; ptr != NULL; ptr = ptr->next) {
    child = (OXShutterItem *) ptr->frame;
    bh = child->_button->GetDefaultHeight();
    exh -= bh;
  }

  y = _insets.t;
  for (ptr = _flist; ptr != NULL; ptr = ptr->next) {
    child = (OXShutterItem *) ptr->frame;
    bh = child->_button->GetDefaultHeight();
    if (child == _selectedItem) {
      if (_closingItem)
        child->_canvas->SetScrolling(CANVAS_NO_SCROLL);
      else
        child->_canvas->SetScrolling(CANVAS_SCROLL_VERTICAL);
      child->ShowFrame(child->_canvas);
      child->MoveResize(_insets.l, y, _w - _insets.r - _insets.l,
                        exh - _closingHeight + bh);
      y += exh - _closingHeight + bh;
    } else if (child == _closingItem) {
      child->_canvas->SetScrolling(CANVAS_NO_SCROLL);
      child->MoveResize(_insets.l, y, _w - _insets.r - _insets.l,
                        _closingHeight + bh);
      y += _closingHeight + bh;
    } else {
      child->MoveResize(_insets.l, y, _w - _insets.r - _insets.l, bh);
      child->HideFrame(child->_canvas);
      y += bh;
    }
  }
}


//--------------------------------------------------------------------

OXShutterItem::OXShutterItem(const OXWindow *p, OString *s, int ID,
                             unsigned int options) :
  OXVerticalFrame (p, 10, 10, options),
  OXWidget (ID, "OXShutterItem") {

  _button = new OXTextButton(this, s, ID);
  _canvas = new OXCanvas(this, 10, 10, CHILD_FRAME);
  _container = new OXVerticalFrame(_canvas->GetViewPort(), 10, 10, OWN_BKGND);
  _canvas->SetContainer(_container);
  _container->SetBackgroundColor(_client->GetShadow(_defaultFrameBackground)); 

  AddFrame(_button, _l1 = new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X));
  AddFrame(_canvas, _l2 = new OLayoutHints(LHINTS_EXPAND_Y | LHINTS_EXPAND_X));

  _button->Associate((OXFrame *) p);
}

OXShutterItem::~OXShutterItem() {
  delete _l1;
  delete _l2;
}

void OXShutterItem::SetContainer(OXCompositeFrame *c) {
  _container = c;
  _canvas->SetContainer(_container);
  _container->ChangeOptions(_container->GetOptions() | OWN_BKGND);
  _container->SetBackgroundColor(_client->GetShadow(_defaultFrameBackground)); 
}
