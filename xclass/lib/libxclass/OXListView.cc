/**************************************************************************

    This file is part of xclass.
    Copyright (C) 1996-2000 Harald Radke, Hector Peraza.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

**************************************************************************/

#include <vector>
#include <algorithm>
#include <functional>

#include <X11/keysym.h>

#include <xclass/OXListView.h>

const OXFont *OListViewItem::_defaultFont = NULL;
const OXGC *OListViewItem::_dfselGC;
const OXGC *OListViewItem::_dfhilightGC;
const OXGC *OListViewItem::_dfnormGC;


//----------------------------------------------------------------------

// TODO:
// - add SetName, SetPics methods...

OListViewItem::OListViewItem(const OXListView *p, int id,
         const OPicture *bigpic, const OPicture *smallpic,
         std::vector<OString *> names, int viewMode, int fullLine) :
  OItem(p, id) {

  if (!_defaultFont) {
    _defaultFont = _client->GetResourcePool()->GetIconFont();

    XGCValues gcv;
    unsigned long gcm;

    gcm = GCFont | GCForeground | GCBackground | GCGraphicsExposures |
          GCFillStyle;
    gcv.font = _defaultFont->GetId();
    gcv.foreground = _client->GetResourcePool()->GetSelectedFgndColor();
    gcv.background = _client->GetResourcePool()->GetSelectedBgndColor();
    gcv.graphics_exposures = False;
    gcv.fill_style = FillSolid;

    // GC for selected text (white on...blue, I think)
    _dfselGC = _client->GetGC(p->GetId(), gcm, &gcv);

    gcv.foreground = _client->GetResourcePool()->GetSelectedBgndColor();
    gcv.background = _client->GetResourcePool()->GetSelectedFgndColor();

    // this one is for the blue rectangle behind selected text and for
    // creating the selected picture of an OListView item...
    _dfhilightGC = _client->GetGC(p->GetId(), gcm, &gcv);

    gcv.background = _client->GetResourcePool()->GetDocumentBgndColor();
    gcv.foreground = _client->GetResourcePool()->GetDocumentFgndColor();

    // standard GC black on white...
    _dfnormGC = _client->GetGC(p->GetId(), gcm, &gcv);
  }

  _position.x = _position.y = 0;
  _names = names;
  _bigPic = bigpic;
  _smallPic = smallpic;
  _viewMode = -1;
  _selectedPic = NULL;
  _currentPic = NULL;

  _font = _defaultFont;

  _selGC = (OXGC *) _dfselGC;
  _hilightGC = (OXGC *) _dfhilightGC;
  _normGC = (OXGC *) _dfnormGC;

  SetViewMode(viewMode, fullLine);
}

OListViewItem::~OListViewItem() {
  for (int i = 0; i < _names.size(); ++i) delete _names[i];
  if (_normGC != _dfnormGC) delete _normGC;
  if (_selGC != _dfselGC) delete _selGC;
  if (_hilightGC != _dfhilightGC) delete _hilightGC;
  if (_font != _defaultFont) _client->FreeFont((OXFont *) _font);
}

void OListViewItem::SetFont(const OXFont *f) {
  if (f) {
    const OXFont *oldfont = _font;
    _font = f;
    if (_normGC == _dfnormGC) _normGC = new OXGC(_dfnormGC);
    if (_selGC == _dfselGC) _selGC = new OXGC(_dfselGC);
    _normGC->SetFont(_font->GetId());
    _selGC->SetFont(_font->GetId());
    if (oldfont != _defaultFont) _client->FreeFont((OXFont *) oldfont);

    OFontMetrics fm;
    _font->GetFontMetrics(&fm);

    _ta = fm.ascent;
    _th = _font->TextHeight();
    _tw = _font->XTextWidth(_names[0]->GetString(), _names[0]->GetLength());

    Layout();
  }
}

void OListViewItem::SetViewMode(int viewMode, int fullLine) {

  _fullRowSelect = fullLine;

  if (_viewMode == viewMode) return;
  _viewMode = viewMode;

  if (_viewMode == LV_LARGE_ICONS)
    _currentPic = _bigPic;
  else
    _currentPic = _smallPic;

  OFontMetrics fm;
  _font->GetFontMetrics(&fm);

  _ta = fm.ascent;
  _th = _font->TextHeight();
  _tw = _font->XTextWidth(_names[0]->GetString(), _names[0]->GetLength());

  Resize(GetDefaultSize());

  if (_currentPic) {
    if (_selectedPic) delete _selectedPic;
    _selectedPic = new OSelectedPicture(_client, _currentPic);
  }
}

ODimension OListViewItem::GetDefaultSize() const {
  ODimension size;
  ODimension isize(0, 0);
  ODimension lsize(_tw, _th+1);

  if (_currentPic)
    isize = ODimension(_currentPic->GetWidth(), _currentPic->GetHeight());

  switch (_viewMode) {
    default:
    case LV_LARGE_ICONS:
      size.w = max(isize.w, lsize.w);
      size.h = isize.h + lsize.h + 6;
      break;

    case LV_SMALL_ICONS:
    case LV_LIST:
    case LV_DETAILS:
      size.w = isize.w + lsize.w + 4;
      size.h = max(isize.h, lsize.h);
      if (_fullRowSelect && (_viewMode == LV_DETAILS)) size.w = _parent->GetVirtualSize().w;
      break;
  }

  return size;
}

int OListViewItem::GetDefaultColumnWidth(int column) const {
  if (column >= _names.size()) return 0;

  if (column == 0)
    return (_smallPic ? _smallPic->GetWidth() + 2 : 0) + _tw;
  else
    return _font->XTextWidth(_names[column]->GetString(),
                             _names[column]->GetLength());
}

// Draw the item at the specified physical position

void OListViewItem::Draw(OXWindow *w, OPosition pos) {
  GC drawGC;
  GC rectGC;
  const OPicture *pic;
  int i, j;

  if (_isSelected) {
    drawGC = _selGC->GetGC();
    rectGC = _hilightGC->GetGC();
    pic = (const OPicture *) _selectedPic;
  } else {
    drawGC = _normGC->GetGC();
    rectGC = _backGC;
    pic = _currentPic;
  }

  std::vector<SColumnData *> columnData = ((OXListView *)_parent)->GetColumns();

  // display icon and name

  if ((_viewMode == LV_DETAILS) && (columnData.size() > 0)) {
    ODimension size = GetSize();

    if (_fullRowSelect) {
      w->FillRectangle(rectGC, -_parent->GetScrollPosition().x, pos.y, size.w, size.h);
    } else {
      w->FillRectangle(rectGC, pos.x + _textPos.x, pos.y + _textPos.y,
                       min(_tw, columnData[0]->width - _textPos.x - 2), _th + 1);
    }

    if (_focused && (_tw > 0)) {
      if (_fullRowSelect) {
        w->DrawRectangle(_client->GetResourcePool()->GetDocumentBckgndGC()->GetGC(),
                         -_parent->GetScrollPosition().x, pos.y, size.w-1, size.h-1);
        w->DrawRectangle(_client->GetResourcePool()->GetFocusHiliteGC()->GetGC(),
                         -_parent->GetScrollPosition().x, pos.y, size.w-1, size.h-1);
      } else {
        w->DrawRectangle(_client->GetResourcePool()->GetDocumentBckgndGC()->GetGC(),
                         pos.x + _textPos.x, pos.y + _textPos.y,
                         min(_tw, columnData[0]->width - _textPos.x - 2)-1, _th);
        w->DrawRectangle(_client->GetResourcePool()->GetFocusHiliteGC()->GetGC(),
                         pos.x + _textPos.x, pos.y + _textPos.y,
                         min(_tw, columnData[0]->width - _textPos.x - 2)-1, _th);
      }
    }

    DrawTruncated(w, drawGC, OPosition(pos.x + _textPos.x,
                                       pos.y + _textPos.y + _ta),
                  _names[0], columnData[0]->width - _textPos.x - 2);
  } else {

    w->FillRectangle(rectGC, pos.x + _textPos.x, pos.y + _textPos.y,
                     _tw, _th + 1);

    if (_focused && (_tw > 0)) {
      w->DrawRectangle(_client->GetResourcePool()->GetDocumentBckgndGC()->GetGC(),
                       pos.x + _textPos.x, pos.y + _textPos.y,
                       _tw-1, _th);
      w->DrawRectangle(_client->GetResourcePool()->GetFocusHiliteGC()->GetGC(),
                       pos.x + _textPos.x, pos.y + _textPos.y,
                       _tw-1, _th);
    }

    _names[0]->Draw(_client->GetDisplay(), w->GetId(), drawGC,
                    pos.x + _textPos.x, pos.y + _textPos.y + _ta);
  }

  if (pic)
    pic->Draw(_client->GetDisplay(), w->GetId(), drawGC,
              pos.x + _iconPos.x, pos.y + _iconPos.y);

  // as usual, DETAILS requires some more effort...

  if ((_viewMode == LV_DETAILS) && (columnData.size() > 0)) {
    pos.x += columnData[0]->width;
    for (i = 1; i < columnData.size(); i++) {

      j = columnData[i]->index;

      if (j < _names.size()) {
        int tx, tw = _font->XTextWidth(_names[j]->GetString(),
                                       _names[j]->GetLength());
        switch (columnData[i]->alignment) {
          default:
          case TEXT_LEFT:     tx = 2; break;
          case TEXT_RIGHT:    tx = columnData[i]->width - tw -2; break;
          case TEXT_CENTER_X: tx = (columnData[i]->width - tw) / 2; break;
        }
        if (tx < 2) tx = 2;

        DrawTruncated(w, _fullRowSelect ? drawGC : _normGC->GetGC(),
                      OPosition(pos.x + tx, pos.y + _textPos.y + _ta),
                     _names[j], columnData[i]->width - tx - 2);
      }

      pos.x += columnData[i]->width;
    }
  }
}

void OListViewItem::DrawTruncated(OXWindow *w, GC gc, OPosition pos,
                                  OString *s, int width) {
  static const char *t3p = "...";

  int x3p, w3p = _font->XTextWidth(t3p, 3);
  int len = s->GetLength();
  const char *text = s->GetString();

  if (_font->XTextWidth(text, len) > width) {
    while ((x3p = _font->XTextWidth(text, len)) > width - w3p)
      if (len-- <= 0) break;
    w->DrawString(gc, pos.x, pos.y, text, len);
    w->DrawString(gc, pos.x + x3p, pos.y, t3p, 3);
  } else {
    w->DrawString(gc, pos.x, pos.y, text, len);
  }
}

void OListViewItem::Layout() {
  int pic_w = _currentPic ? _currentPic->GetWidth() : 0;
  int pic_h = _currentPic ? _currentPic->GetHeight() : 0;

  switch (_viewMode) {
    case LV_LARGE_ICONS:
      _iconPos.x = (_size.w - pic_w) / 2;
      _iconPos.y = 0;
      _textPos.x = (_size.w - _tw) / 2;
      _textPos.y = _size.h - (_th+1) - 2;
      break;

    case LV_LIST:
    case LV_SMALL_ICONS:
    case LV_DETAILS:
      _iconPos.x = 0;
      _iconPos.y = (_size.h - pic_h) / 2;
      _textPos.x = _currentPic ? pic_w + 2 : 0;
      _textPos.y = (_size.h - (_th+1)) / 2;
      break;

    default:
      break;
  }

}

int OListViewItem::Compare(const OListViewItem *item, int column) const {

  OString *name1 = GetName(column);
  OString *name2 = item->GetName(column);

  if (!name1 || !name2) return 0;

  return strcmp(name1->GetString(), name2->GetString());
}

//----------------------------------------------------------------------

OXListView::OXListView(const OXWindow *p, int w, int h, int id,
                       unsigned int options, unsigned int sboptions) :
  OXItemView(p, w, h, id, options, sboptions) {

  _msgType = MSG_LISTVIEW;
  _widgetType = "OXListView";

  _header = new OXFrame(this, 10, 10, CHILD_FRAME);

  _needItemLayout = True;
  _columnMargin = 8;

  _columnData.clear();

  // kind of initial setting
  _viewMode = -1;
  _clearExposedArea = True;

  const OResourcePool *res = _client->GetResourcePool();

  _headerFont = (OXFont *) res->GetStatusFont();
  _itemFont = NULL;

  _sortColumn = 0;
  _autoSort = True;

  _rows = _cols = 0;

  SetViewMode(LV_LARGE_ICONS);
}

OXListView::~OXListView() {
  for (int i = 0; i < _columnData.size(); ++i) {
    delete _columnData[i]->button;
    delete _columnData[i]->resizer;
  }
  delete _header;
}

int OXListView::HandleButton(XButtonEvent *event) {

  if ((_viewMode == LV_LIST) && (event->type == ButtonPress)) {
    if (event->button == Button4) {
      ScrollRight(_scrollValue.x);
      return True;
    } else if (event->button == Button5) {
      ScrollLeft(_scrollValue.x);
      return True;
    }
  }

  return OXItemView::HandleButton(event);
}

//----------------------------------------------------------------------

// Keyboard handling...

int OXListView::HandleKey(XKeyEvent *event) {
  if (_items.size() == 0) return True;

  if (event->type == KeyPress) {

    if (!_anchorItem) {
      _anchorItem = _items[0];
      _anchorItem->ShowFocusHilite(HasFocus());
      NeedRedraw(ORectangle(_anchorItem->GetPosition(),
                            _anchorItem->GetSize()));
      return True;
    }

    KeySym keysym = XKeycodeToKeysym(GetDisplay(), event->keycode, 0);
    switch (keysym) {
      case XK_Left:
        _KeyLeft();
        break;

      case XK_Right:
        _KeyRight();
        break;

      case XK_Up:
        _KeyUp();
        break;

      case XK_Down:
        _KeyDown();
        break;

      case XK_Page_Up:
        _KeyPageUp();
        break;

      case XK_Page_Down:
        _KeyPageDown();
        break;

      case XK_Home:
        _KeyHome();
        break;

      case XK_End:
        _KeyEnd();
        break;

      case XK_KP_Enter:
      case XK_Return:
        {
        // change this into a keypress message!
        OItemViewMessage message(_msgType, MSG_DBLCLICK, _widgetID, Button1,
                                 OPosition(0, 0));
        SendMessage(_msgObject, &message);
        }
        break;

      case XK_space:
        {
        // change this into a keypress message!
        OItemViewMessage message(_msgType, MSG_CLICK, _widgetID, Button1,
                                 OPosition(0, 0));
        SendMessage(_msgObject, &message);
        }
        break;

      case XK_Delete:
        break;

      default:
        {
        char input[2] = { 0, 0 };
        int  charcount;
        KeySym keysym;
        XComposeStatus compose = { NULL, 0 };
        charcount = XLookupString(event, input, sizeof(input)-1,
                                  &keysym, &compose);
        if (charcount > 0) {
          int i;
          OListViewItem *item;
          for (i = 0; i < _items.size(); ++i) {
            if (_items[i] == _anchorItem) break;
          }
          if (_anchorItem && _anchorItem->IsSelected()) ++i;
          for (; i < _items.size(); ++i) {
            item = (OListViewItem *) _items[i];
            if (item->GetName())
              if (item->GetName()->GetString()[0] == input[0]) break;
          }
          if (i == _items.size()) {
            for (i = 0; i < _items.size(); ++i) {
              item = (OListViewItem *) _items[i];
              if (item->GetName())
                if (item->GetName()->GetString()[0] == input[0]) break;
            }
          }
          if (i < _items.size()) {
            if (_anchorItem) SelectItem(_anchorItem, False);
            if (!item->IsSelected()) SelectItem(item, True);
            ShowFocusHilite(False);
            _anchorItem = item;
            ShowFocusHilite(HasFocus());
          }
        } else {
          return True;  // probably a modifier key...
        }
        }
        break;
    }
  } else {
    return False;  // key release
  }

  if (_anchorItem) {
    OPosition p = _anchorItem->GetPosition();

    if (p.y < _visibleStart.y + _itemSep.h) {
      ScrollToPosition(OPosition(_visibleStart.x, p.y - _itemSep.h));
    } else if (p.y > _visibleStart.y + _canvas->GetHeight()
                     - _maxItemSize.h - _itemSep.h) {
      ScrollToPosition(OPosition(_visibleStart.x, p.y - _canvas->GetHeight()
                     + _maxItemSize.h + _itemSep.h));
    }

    if (p.x < _visibleStart.x + _itemSep.w) {
      ScrollToPosition(OPosition(p.x - _itemSep.w, _visibleStart.y));
    } else if (p.x > _visibleStart.x + _canvas->GetWidth()
                     - _maxItemSize.w - _itemSep.w) {
      ScrollToPosition(OPosition(p.x - _canvas->GetWidth() + _maxItemSize.w
                     + _itemSep.w, _visibleStart.y));
    }
  }

  return True;
}

void OXListView::_KeyLeft() {
  if (_anchorItem) {
    int i;
    for (i = 0; i < _items.size(); ++i) if (_items[i] == _anchorItem) break;
    if (_viewMode == LV_LIST || _viewMode == LV_DETAILS) {
      if (i >= _rows) i -= _rows;
    } else {
      if ((i % _cols) > 0) --i;
    }
    OItem *item = _items[i];
    SelectItem(_anchorItem, False);
    if (!item->IsSelected()) SelectItem(item, True);
    ShowFocusHilite(False);
    _anchorItem = item;
    ShowFocusHilite(HasFocus());
  }
}

void OXListView::_KeyRight() {
  if (_anchorItem) {
    int i;
    for (i = 0; i < _items.size(); ++i) if (_items[i] == _anchorItem) break;
    if (_viewMode == LV_LIST || _viewMode == LV_DETAILS) {
      if (i < _items.size() - _rows) i += _rows;
    } else {
      if (((i % _cols) < _cols - 1) && (i < _items.size() - 1)) ++i;
    }
    OItem *item = _items[i];
    SelectItem(_anchorItem, False);
    if (!item->IsSelected()) SelectItem(item, True);
    ShowFocusHilite(False);
    _anchorItem = item;
    ShowFocusHilite(HasFocus());
  }
}

void OXListView::_KeyUp() {
  if (_anchorItem) {
    int i;
    for (i = 0; i < _items.size(); ++i) if (_items[i] == _anchorItem) break;
    if (_viewMode == LV_LIST || _viewMode == LV_DETAILS) {
      if ((i % _rows) > 0) --i;
    } else {
      if (i >= _cols) i -= _cols;
    }
    OItem *item = _items[i];
    SelectItem(_anchorItem, False);
    if (!item->IsSelected()) SelectItem(item, True);
    ShowFocusHilite(False);
    _anchorItem = item;
    ShowFocusHilite(HasFocus());
  }
}

void OXListView::_KeyDown() {
  if (_anchorItem) {
    int i;
    for (i = 0; i < _items.size(); ++i) if (_items[i] == _anchorItem) break;
    if (_viewMode == LV_LIST || _viewMode == LV_DETAILS) {
      if (((i % _rows) < _rows - 1) && (i < _items.size() - 1)) ++i;
    } else {
      if (i < _items.size() - _cols) i += _cols;
    }
    OItem *item = _items[i];
    SelectItem(_anchorItem, False);
    if (!item->IsSelected()) SelectItem(item, True);
    ShowFocusHilite(False);
    _anchorItem = item;
    ShowFocusHilite(HasFocus());
  }
}

void OXListView::_KeyHome() {
  if (_items.size() > 0) {
    SelectItem(_anchorItem, False);
    OItem *item = _items[0];
    if (!item->IsSelected()) SelectItem(item, True);
    ShowFocusHilite(False);
    if (item) _anchorItem = item;
    ShowFocusHilite(HasFocus());
  }
}

void OXListView::_KeyEnd() {
  if (_items.size() > 0) {
    SelectItem(_anchorItem, False);
    OItem *item = _items[_items.size() - 1];
    if (!item->IsSelected()) SelectItem(item, True);
    ShowFocusHilite(False);
    if (item) _anchorItem = item;
    ShowFocusHilite(HasFocus());
  }
}

void OXListView::_KeyPageUp() {
}

void OXListView::_KeyPageDown() {
}


//----------------------------------------------------------------------

void OXListView::SetHeaderFont(const OXFont *f) {
  if (f) {
    _headerFont = (OXFont *) f;
    const OResourcePool *res = _client->GetResourcePool();
    for (int i = 0; i < _columnData.size(); ++i) {
      _columnData[i]->button->SetFont(res->GetFontPool()->GetFont(_headerFont));
    }
    Layout();
  }
}

void OXListView::SetItemFont(const OXFont *f) {
  if (f) {
    _itemFont = (OXFont *) f;
    const OResourcePool *res = _client->GetResourcePool();
    for (int i = 0; i < _items.size(); ++i) {
      ((OListViewItem *)_items[i])->SetFont(res->GetFontPool()->GetFont(_itemFont));
    }
    CalcMaxItemSize();
    Layout();
  }
}

void OXListView::SetViewMode(int viewMode, int fullLine, int grid) {

  _fullRowSelect = fullLine;
  _gridLines = grid;

  if (_viewMode == viewMode) return;

  _viewMode = viewMode;

  // depending on the view mode, the item spacing differs

  if (_viewMode == LV_LARGE_ICONS)
    _itemSep = ODimension(8, 12);
  else
    _itemSep = ODimension(2, _gridLines ? 1 : 3);

  for (unsigned int i = 0; i < _items.size(); i++)
    ((OListViewItem *) _items[i])->SetViewMode(_viewMode, _fullRowSelect);

  CalcMaxItemSize();

  switch (_viewMode) {
    case LV_LARGE_ICONS:
    case LV_SMALL_ICONS:
      _vsb->SetDelay(10, 10);
      _vsb->SetMode(SB_ACCELERATED);
      _hsb->SetDelay(10, 10);
      _hsb->SetMode(SB_ACCELERATED);
      break;

    case LV_LIST:
      _vsb->SetDelay();
      _vsb->SetMode();
      _hsb->SetDelay();
      _hsb->SetMode();
      break;

    case LV_DETAILS:
      _vsb->SetDelay();
      _vsb->SetMode();
      _hsb->SetDelay(10, 10);
      _hsb->SetMode(SB_ACCELERATED);
      break;

  }

  Layout();
  ScrollToPosition(OPosition(0, 0));

  XSync(GetDisplay(), False);

  NeedRedraw(ORectangle(_visibleStart, _canvas->GetSize()));
//  DrawRegion(_visibleStart, _canvas->GetSize());
}

void OXListView::Layout() {

  if ((_viewMode == LV_LIST) && (_maxItemSize.w > 0)) {
    _scrollValue.x = _maxItemSize.w + _itemSep.w;
    _scrollValue.y = 1;
  } else if ((_viewMode == LV_DETAILS) && (_maxItemSize.h > 0)) {
    _scrollValue.x = 1;
    _scrollValue.y = _maxItemSize.h + _itemSep.h;
  } else {
    _scrollValue.x = 1;
    _scrollValue.y = 1;
  }

  if ((_viewMode == LV_DETAILS) && (_columnData.size() > 0)) {
    _offset.y = _columnData[0]->button->GetDefaultHeight() - 4;
  } else {
    _offset.y = 0;
  }

  OXItemView::Layout();

  if (_viewMode == LV_DETAILS)
    ShowHeaderBar();
  else
    HideHeaderBar();
}

void OXListView::ShowHeaderBar() {
  int n = _columnData.size();

  if (n > 0) {
    int i, w, h, xl, bfw;

    w = h = 0;

    xl = -_visibleStart.x;
    bfw = _w - _insets.r - _insets.l;

    h = _columnData[0]->button->GetDefaultHeight() - 4;
    for (i = 0; i < n; ++i) {
      w = _columnData[i]->width;
      if (i == 0) w += _itemSep.w;
      if (i == n - 1) w = bfw - xl;
      _columnData[i]->resizer->MoveResize(xl + w - 1 + _insets.l, _insets.t,
                                          2, _h - _insets.t - _insets.b);
      _columnData[i]->resizer->MoveResizeHandle(-3, 0, 6, h);
      if ((xl + w > 0) && (xl < bfw)) {
        _columnData[i]->button->MoveResize(xl, 0, w, h);
        _columnData[i]->button->MapWindow();
        if (i < n - 1)
          _columnData[i]->resizer->MapRaised();
        else
          _columnData[i]->resizer->UnmapWindow();
      } else {
        _columnData[i]->button->UnmapWindow();
        _columnData[i]->resizer->UnmapWindow();
      }
      xl += w;
    }
    _header->MoveResize(_insets.l, _insets.t, bfw, h);
    _header->MapWindow();
  }
}

void OXListView::HideHeaderBar() {
  _header->UnmapWindow();
  for (int i = 0; i < _columnData.size(); ++i)
    _columnData[i]->resizer->UnmapWindow();
}

void OXListView::AddColumn(OString *name, int index, int align) {

  int n = _columnData.size();

  OXTextButton *b = new OXTextButton(_header, name, index);

  OXVerticalResizer *resizer = new OXVerticalResizer(this, n,
                                   RESIZER_OPAQUE | RESIZER_HIDDEN |
                                   RESIZER_USE_HANDLE);

  const OResourcePool *res = _client->GetResourcePool();

  b->SetFont(res->GetFontPool()->GetFont(_headerFont));
  b->TakeFocus(False);
  b->Associate(this);
  b->SetTextAlignment(align | TEXT_CENTER_Y);

  resizer->SetPrev(b, 8);

  if (n > 0) _columnData[n-1]->resizer->SetNext(b, 8);

  SColumnData *col = new SColumnData;
  col->button = b;
  col->resizer = resizer;
  col->index = index;
  col->width = b->GetDefaultWidth() + _columnMargin;
  col->user_width = -1;
  col->alignment = align;
  col->sort_mode = LV_UNSORTED;

  _columnData.push_back(col);
}

void OXListView::SetColumnWidth(int column, int width) {
  if ((column >= 0) && (column < _columnData.size())) {
    _columnData[column]->user_width = width;
    if (width >= 0) _columnData[column]->width = width;
    Layout();
  }
}

int OXListView::GetColumnWidth(int column) {
  if ((column >= 0) && (column < _columnData.size())) {
    return _columnData[column]->width;
  } else {
    return -1;
  }
}

// hm... this should be reconsidered...

void OXListView::AddItem(OListViewItem *newItem) {

  if (newItem == NULL) return;

  newItem->_backGC = _backGC;
  newItem->Move(OPosition(0, _virtualSize.h));

  const OResourcePool *res = _client->GetResourcePool();
  if (_itemFont) newItem->SetFont(res->GetFontPool()->GetFont(_itemFont));

  ODimension itemSize = newItem->GetDefaultSize();
  newItem->Resize(itemSize);

  _items.push_back(newItem);

  // Adjust the column widths for LV_DETAILS, if necessary

  int firstCol = newItem->GetDefaultColumnWidth(0);

  if (_columnData.size() > 0) {
    if (firstCol + _columnMargin > _columnData[0]->width) {
      if (_columnData[0]->user_width < 0)
        _columnData[0]->width = firstCol + _columnMargin;
    }

    for (unsigned int j = 1; j < _columnData.size(); j++) {

      int colw = newItem->GetDefaultColumnWidth(_columnData[j]->index);

      if (_columnData[j]->width < colw + _columnMargin) {
        if (_columnData[j]->user_width < 0)
          _columnData[j]->width = colw + _columnMargin;
      }
    }
  }

  newItem->SetViewMode(_viewMode, _fullRowSelect);
  CalcMaxItemSize();

//  Layout();
}


void OXListView::DeleteItem(unsigned int num) {
  unsigned int i;

  if (_items.size() <= num) return;

  if (_hasSelection) {
    for (i = 0; i < _selectedItems.size(); i++)
      if (_selectedItems[i] == _items[num]) {
        _selectedItems.erase(_selectedItems.begin() + i);
        break;
      }
  }
  delete _items[num];
  _items.erase(_items.begin() + num);

  CalcColumnMaxWidths();
  CalcMaxItemSize();

  Layout();
}


void OXListView::CalcMaxItemSize() {

  OXItemView::CalcMaxItemSize();

  if (_viewMode == LV_DETAILS) {
    _maxItemSize.w = 0;
    for (unsigned int i = 0; i < _columnData.size(); i++)
      _maxItemSize.w += _columnData[i]->width;
  }
}


void OXListView::CalcColumnMaxWidths() {
  OListViewItem *item;
  unsigned int firstCol;

  if ((_items.size() == 0) || (_columnData.size() == 0)) return;

  for (unsigned int i = 0; i < _items.size(); i++) {
    item = (OListViewItem *) _items[i];

    firstCol = item->GetDefaultColumnWidth(0);

    if (firstCol + _columnMargin > _columnData[0]->width) {
      if (_columnData[0]->user_width < 0)
        _columnData[0]->width = firstCol + _columnMargin;
    }

    for (unsigned int j = 1; j < _columnData.size(); j++) {

      int colw = item->GetDefaultColumnWidth(_columnData[j]->index);

      if (_columnData[j]->width < colw + _columnMargin) {
        if (_columnData[j]->user_width < 0)
          _columnData[j]->width = colw + _columnMargin;
      }
    }

  }
}


bool OXListView::ItemLayout() {

  bool needRedraw = False;

  ODimension canvasSize = _canvas->GetSize();
  unsigned int i;
  ODimension itemSize, maxItemSize = GetMaxItemSize();
  _virtualSize = ODimension(0, 0);
  OPosition itemPos, currentPos;

  if (_items.size() > 0) {
    switch (_viewMode) {
    case LV_LARGE_ICONS:
      currentPos.x = _itemSep.w;
      currentPos.y = _itemSep.h;
      _virtualSize.w = max(canvasSize.w, maxItemSize.w + _itemSep.w * 2);
      _rows = 0;
      for (i = 0; i < _items.size(); i++) {

        itemSize = _items[i]->GetDefaultSize();

        itemPos.x = currentPos.x + ((int) maxItemSize.w - (int) itemSize.w) / 2;
        itemPos.y = currentPos.y;

        if ((_items[i]->GetPosition().x != itemPos.x) ||
            (_items[i]->GetPosition().y != itemPos.y)) {
          needRedraw = True;
        }
        _items[i]->Move(itemPos);

        currentPos.x += maxItemSize.w + _itemSep.w;

        if (currentPos.x + maxItemSize.w > _virtualSize.w) {
          currentPos.x = _itemSep.w;
          currentPos.y += maxItemSize.h + _itemSep.h;
          ++_rows;
        }
      }
      if (currentPos.x != _itemSep.w) {
        currentPos.y += maxItemSize.h + _itemSep.h;
        ++_rows;
      }
      _virtualSize.h = currentPos.y;
      _cols = _virtualSize.w / (maxItemSize.w + _itemSep.w);
      break;

    case LV_SMALL_ICONS:
      currentPos.x = _itemSep.w;
      currentPos.y = _itemSep.h;
      _virtualSize.w = max(canvasSize.w, maxItemSize.w + _itemSep.w * 2);
      _rows = 0;
      for (i = 0; i < _items.size(); i++) {

        itemSize = _items[i]->GetDefaultSize();

        itemPos.x = currentPos.x;
        itemPos.y = currentPos.y + ((int) maxItemSize.h - (int) itemSize.h) / 2;

        if ((_items[i]->GetPosition().x != itemPos.x) ||
            (_items[i]->GetPosition().y != itemPos.y)) {
          needRedraw = True;
        }
        _items[i]->Move(itemPos);

        currentPos.x += maxItemSize.w + _itemSep.w;

        if (currentPos.x + maxItemSize.w > _virtualSize.w) {
          currentPos.x = _itemSep.w;
          currentPos.y += maxItemSize.h + _itemSep.h;
          ++_rows;
        }
      }
      if (currentPos.x != _itemSep.w) {
        currentPos.y += maxItemSize.h + _itemSep.h;
        ++_rows;
      }
      _virtualSize.h = currentPos.y;
      _cols = _virtualSize.w / (maxItemSize.w + _itemSep.w);
      break;

    case LV_LIST:
      currentPos.x = _itemSep.w;
      currentPos.y = _itemSep.h;
      _virtualSize.h = max(canvasSize.h, maxItemSize.h + _itemSep.h * 2);
      _cols = 0;
      for (i = 0; i < _items.size(); i++) {

        itemSize = _items[i]->GetDefaultSize();

        itemPos.x = currentPos.x;
        itemPos.y = currentPos.y + ((int) maxItemSize.h - (int) itemSize.h) / 2;

        if ((_items[i]->GetPosition().x != itemPos.x) ||
            (_items[i]->GetPosition().y != itemPos.y)) {
          needRedraw = True;
        }
        _items[i]->Move(itemPos);

        currentPos.y += maxItemSize.h + _itemSep.h;

        if (currentPos.y + maxItemSize.h > _virtualSize.h) {
          currentPos.y = _itemSep.h;
          currentPos.x += maxItemSize.w + _itemSep.w;
          ++_cols;
        }
      }
      if (currentPos.y != _itemSep.h) {
        currentPos.x += maxItemSize.w + _itemSep.w;
        ++_cols;
      }
      _virtualSize.w = currentPos.x;
      _rows = _virtualSize.h / (maxItemSize.h + _itemSep.h);
      break;

    case LV_DETAILS:
      _virtualSize.h = _itemSep.h / 2;
      _rows = _items.size();
      _cols = 1;
      for (i = 0; i < _items.size(); i++) {
        currentPos.x = _itemSep.w;
        currentPos.y = _virtualSize.h;
        _items[i]->Move(currentPos);
        _items[i]->Resize(_items[i]->GetDefaultSize());
        _virtualSize.h += _items[i]->GetDefaultSize().h + _itemSep.h;
      }
      _virtualSize.w = GetMaxItemSize().w;
      if (_fullRowSelect) {
        for (i = 0; i < _items.size(); i++) {
          _items[i]->Resize(_items[i]->GetDefaultSize());
        }
      }
      needRedraw = True;
      break;

    default:
      break;
    }
  }

  return needRedraw;
}

int OXListView::DrawRegion(OPosition coord, ODimension size, int clear) {

  if (!OXItemView::DrawRegion(coord, size, clear)) return False;

#if 1
  OPosition pcoord = ToPhysical(coord);

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

  // check whether the area is inside the virtual window

  if ((coord.y > _virtualSize.h) || (coord.x > _virtualSize.w)) return False;

  if (_viewMode == LV_DETAILS) {
    if (_gridLines) {
      for (unsigned int i = 0; i < _items.size(); ++i) {
        int h = _items[i]->GetSize().h + _itemSep.h;
        if ((_items[i]->GetPosition().y < coord.y + size.h) &&
            (_items[i]->GetPosition().y + h > coord.y)) {
          DrawItem(_items[i], coord, size);
          int x = -_visibleStart.x;
          int y = _items[i]->GetPosition().y - _visibleStart.y - 1;
          int w = _virtualSize.w - 1;
          _canvas->DrawRectangle(_bckgndGC, x, y, w, h);
          for (unsigned int j = 0; j < _columnData.size() - 1; ++j) {
            x += _columnData[j]->width;
            _canvas->DrawLine(_bckgndGC, x+1, y, x+1, y + h);
          }
        }
      }
    } else {
      for (unsigned int i = 0; i < _items.size(); ++i) {
        if ((_items[i]->GetPosition().y < coord.y + size.h) &&
            (_items[i]->GetPosition().y + _items[i]->GetSize().h > coord.y)) {
          DrawItem(_items[i], coord, size);
        }
      }
    }
  }
#endif

  return True;
}

struct SLVSortAscending : public std::binary_function<OItem*, OItem*, bool> {
public:
  bool operator()(const OItem* item1, const OItem* item2) const {
    OListViewItem *i1 = (OListViewItem *) item1;
    OListViewItem *i2 = (OListViewItem *) item2;

    int column = ((OXListView *)i1->GetParent())->GetSortColumn();

    return (i1->Compare(i2, column) < 0);
  }
};

struct SLVSortDescending : public std::binary_function<OItem*, OItem*, bool> {
public:
  bool operator()(const OItem* item1, const OItem* item2) const {
    OListViewItem *i1 = (OListViewItem *) item1;
    OListViewItem *i2 = (OListViewItem *) item2;

    int column = ((OXListView *)i1->GetParent())->GetSortColumn();

    return (i2->Compare(i1, column) < 0);
  }
};

void OXListView::SortColumn(int column, int mode) {
  if (column >= _columnData.size()) return;
  if (mode == LV_UNSORTED) return;

  if (_items.size() == 0) return;

  _sortColumn = column;

  if (mode == LV_ASCENDING)
    std::stable_sort(_items.begin(), _items.end(), SLVSortAscending());
  else
    std::stable_sort(_items.begin(), _items.end(), SLVSortDescending());

  ItemLayout();

  NeedRedraw(ORectangle(_visibleStart, _canvas->GetSize()));
//  DrawRegion(_visibleStart, ODimension(_canvas->GetSize()));
}


int OXListView::ProcessMessage(OMessage *msg) {
  if ((msg->type == MSG_BUTTON) && (msg->action == MSG_CLICK)) {

    int col = ((OWidgetMessage *)msg)->id;

    if (_autoSort) {
      if (col < _columnData.size()) {
        int mode = _columnData[col]->sort_mode;
        if (mode == LV_UNSORTED)
          mode = LV_ASCENDING;
        else if (mode == LV_ASCENDING)
          mode = LV_DESCENDING;
        else
          mode = LV_ASCENDING;
        for (int i = 0; i < _columnData.size(); ++i)
          _columnData[i]->sort_mode = LV_UNSORTED;
        _columnData[col]->sort_mode = mode;
        SortColumn(col, mode);
      }
    } else {
    }

  } else if ((msg->type == MSG_HSCROLL) && (_viewMode == LV_DETAILS)) {

    int pos = ((OScrollBarMessage *)msg)->pos;
    Scroll(OPosition(pos * _scrollValue.x, _visibleStart.y));
    ShowHeaderBar();  // update headers...

  } else if (msg->type == MSG_VRESIZER) {

    if (msg->action == MSG_SLIDERTRACK) {

      int n = ((OResizerMessage *)msg)->id;
      int pos = ((OResizerMessage *)msg)->pos;

      if ((n >= 0) && (n < _columnData.size())) {
        _columnData[n]->width = pos;
        int x = 0;
        for (int i = 0; i < n; ++i) x += _columnData[i]->width;

//        CalcMaxItemSize();
//        Layout();
//#define DYN_RESIZE
#ifdef DYN_RESIZE
        NeedRedraw(ORectangle(_visibleStart.x + x, _visibleStart.y,
                   _canvas->GetWidth() - ToPhysical(_visibleStart).x,
                   _canvas->GetHeight() - ToPhysical(_visibleStart).y));
#endif
        ShowHeaderBar();
      }

    } else if (msg->action == MSG_SLIDERPOS) {

      int n = ((OResizerMessage *)msg)->id;
      int pos = ((OResizerMessage *)msg)->pos;

      if ((n >= 0) && (n < _columnData.size())) {
        _columnData[n]->width = pos;

        CalcMaxItemSize();
        Layout();
        DrawRegion(_visibleStart, _canvas->GetSize(), True);
        ShowHeaderBar();
      }
    }

  } else {

    return OXItemView::ProcessMessage(msg);
  }

  return True;
}
