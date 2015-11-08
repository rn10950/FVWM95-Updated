/**************************************************************************

    This file is part of xclass, a Win95-looking GUI toolkit.
    Copyright (C) 1996, 1997 David Barth, Hector Peraza.

    This code is partially based on Robert W. McMullen's ListTree-3.0
    widget. Copyright (C) 1995.

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

#include <stdio.h>

#include <X11/keysym.h>

#include <xclass/OXClient.h>
#include <xclass/OXFont.h>
#include <xclass/OXListTree.h>
#include <xclass/OString.h>
#include <xclass/OPicture.h>
#include <xclass/OResourcePool.h>
#include <xclass/OTimer.h>
#include <xclass/OGC.h>

#include "icons/folder.t.xpm"
#include "icons/ofolder.t.xpm"


#define KBD_DELAY   500


// TODO:
// - add support for drag and drop
// - send more descriptive messages to the application
// - implement a new OXFileSystemTree class


//--------------------------------------------------------------------

OListTreeItem::OListTreeItem(OXClient *client, const char *name, 
                             const OPicture *opened,
                             const OPicture *closed) {
  length = strlen(name);
  text = new char[length+1];
  strcpy(text, name);

  open_pic = opened;
  closed_pic = closed;
  picWidth = max(open_pic->GetWidth(), closed_pic->GetWidth());

  open =
  active = False;

  parent =
  firstchild =
  prevsibling =
  nextsibling = NULL;

  _client = client;
}

OListTreeItem::~OListTreeItem() {
  delete[] text;
  _client->FreePicture(open_pic);
  _client->FreePicture(closed_pic);
}

void OListTreeItem::Rename(const char *new_name) {
  delete[] text;
  length = strlen(new_name);
  text = new char[length + 1];
  strcpy(text, new_name);
}

//--------------------------------------------------------------------

OXListTree::OXListTree(const OXWindow *p, int w, int h, int id,
                       unsigned int options) :
  OXView(p, w, h, id, options) {
    XGCValues gcv;
    unsigned int gcmask;
    OFontMetrics fm;

    _widgetType = "OXListTree";
    _msgType = MSG_LISTTREE;
    _msgObject = p;

    XSetWindowAttributes wattr;
    unsigned long mask;

    mask = CWBitGravity | CWWinGravity;
    wattr.bit_gravity = NorthWestGravity;
    wattr.win_gravity = NorthWestGravity;

    XChangeWindowAttributes(GetDisplay(), _canvas->GetId(), mask, &wattr);

    _font = GetResourcePool()->GetIconFont();
    _font->GetFontMetrics(&fm);

    _th = fm.linespace;
    _ascent = fm.ascent;

    _grayPixel = _client->GetColorByName("#808080");

    gcv.line_style = LineSolid;
    gcv.line_width = 0;
    gcv.fill_style = FillSolid;
    gcv.font = _font->GetId();
    gcv.background = _whitePixel;
    gcv.foreground = _blackPixel;

    gcmask = GCLineStyle | GCLineWidth | GCFillStyle |
             GCForeground | GCBackground | GCFont;
    _drawGC = XCreateGC(GetDisplay(), _id, gcmask, &gcv);

    gcv.fill_style = FillStippled;
    gcmask |= GCStipple;
    gcv.stipple = _client->GetResourcePool()->GetCheckeredBitmap();
    gcv.foreground = _grayPixel;
    _lineGC = XCreateGC(GetDisplay(), _id, gcmask, &gcv);

    gcv.background = _defaultSelectedBackground;
    gcv.foreground = _whitePixel;
    gcv.line_style = LineSolid;
    gcmask = GCLineStyle | GCLineWidth | GCFillStyle |
             GCForeground | GCBackground | GCFont;
    _highlightGC = XCreateGC(GetDisplay(), _id, gcmask, &gcv);

    _first = _selected = NULL;

    _virtualSize = ODimension(1, 1);

    _hspacing = 2;
    _vspacing = 2;  // 0;
    _indent = 3;    // 0;
    _margin = 2;

    _timer = NULL;
    _focused = False;
    AddInput(FocusChangeMask);

    XGrabButton(GetDisplay(), Button1, AnyModifier, _canvas->GetId(), True,
                ButtonPressMask | ButtonReleaseMask,
                GrabModeAsync, GrabModeAsync, None, None);

    _vsb->SetDelay(10, 10);
    _vsb->SetMode(SB_ACCELERATED);
    _hsb->SetDelay(10, 10);
    _hsb->SetMode(SB_ACCELERATED);

    _clearExposedArea = True;
}

OXListTree::~OXListTree() {
  OListTreeItem *item, *sibling;

  XFreeGC(GetDisplay(), _drawGC);
  XFreeGC(GetDisplay(), _highlightGC);
  item = _first;
  while (item) {
    if (item->firstchild) _DeleteChildren(item->firstchild);
    sibling = item->nextsibling;
    delete item;
    item = sibling;
  }

  if (_timer) delete _timer;
}


//-------------------------------------------- highlighting utilities

void OXListTree::_HighlightItem(OListTreeItem *item, int state, int draw) {
  if (item) {
    if ((item == _selected) && !state) _selected = NULL;
    if (state != item->active) {
      item->active = state;
      if (draw) {
        _DrawItemPic(item);
        _DrawItemName(item);
      }
    }
  }
}

void OXListTree::_HighlightChildren(OListTreeItem *item, int state, int draw) {
  while (item) {
    _HighlightItem(item, state, draw);
    if (item->firstchild)
      _HighlightChildren(item->firstchild, state, (item->open) ? draw : False);
    item = item->nextsibling;
  }
}

void OXListTree::_UnselectAll(int draw) {
  _HighlightChildren(_first, False, draw);
}

int OXListTree::HandleButton(XButtonEvent *event) {
  OListTreeItem *item;

  if (OXView::HandleButton(event)) return True;  // mouse-wheel scroll

  OPosition ep = ToVirtual(OPosition(event->x, event->y));

  if (event->type == ButtonPress) {
    if (TakesFocus()) RequestFocus();

    if ((item = _FindItem(ep.y)) != NULL) {
      if (item->xnode > 0) {
        if ((ep.x > item->xnode-5) && (ep.x < item->xnode+5)) {
          // toggle open 
          item->open = !item->open;
          NeedRedraw(ORectangle(0, item->y,
                     _virtualSize.w, _canvas->GetHeight()));
        }
      }
      _last_y = ep.y;
      _UnselectAll(True);
      _HighlightItem(_selected = item, True, True);
      OListTreeMessage msg(_msgType, MSG_CLICK, _widgetID,
                           event->button, event->x_root, event->y_root);
      SendMessage(_msgObject, &msg);
    }
  }
  return True;
}

int OXListTree::HandleDoubleClick(XButtonEvent *event) {
  OListTreeItem *item;

  if (event->button != Button1) return False;

  OPosition ep = ToVirtual(OPosition(event->x, event->y));

  if ((item = _FindItem(ep.y)) != NULL) {
    if (item->xnode > 0) {
      if ((ep.x > item->xnode-5) && (ep.x < item->xnode+5)) {
        return False;
      }
    }
    item->open = !item->open;
    NeedRedraw(ORectangle(0, item->y,
               _virtualSize.w, _canvas->GetHeight()));
    _UnselectAll(True);
    _selected = item;
    _HighlightItem(_selected = item, True, True);
    OListTreeMessage msg(_msgType, MSG_DBLCLICK, _widgetID,
                         event->button, event->x_root, event->y_root);
    SendMessage(_msgObject, &msg);
  }

  return True;
}

int OXListTree::DrawRegion(OPosition coord, ODimension size, int clear) {
  OXView::DrawRegion(coord, size, clear);
  _Draw(coord.y, size.h);
  return True;
}

int OXListTree::HandleKey(XKeyEvent *event) {
  OListTreeItem *next = _selected;

  if (!_selected) {
    _selected = _first;
    _HighlightItem(_selected, True, True);
    return True;
  }

  if (event->type == KeyPress) {
    KeySym keysym = XKeycodeToKeysym(GetDisplay(), event->keycode, 0);

    OListTreeMessage msg(_msgType, 0, _widgetID, 0, 0, 0);

    switch (keysym) {
      case XK_Left:
        // if the current node is open, close it
        // otherwise go to parent node
        if (_selected->open) {
          _selected->open = False;
          NeedRedraw(ORectangle(0, _selected->y,
                     _virtualSize.w, _canvas->GetHeight()));
        } else if (_selected != _first) {
          next = _selected->parent;
          if (_timer) delete _timer;
          _timer = new OTimer(this, KBD_DELAY);
        }
        break;

      case XK_Right:
        // if the current node is closed, open it
        // otherwise go to the first child
        if (!_selected->open) {
          if (_selected->firstchild) {
            _selected->open = True;
            NeedRedraw(ORectangle(0, _selected->y,
                       _virtualSize.w, _canvas->GetHeight()));
          }
        } else if (_selected->firstchild) {
          next = _selected->firstchild;
          if (_timer) delete _timer;
          _timer = new OTimer(this, KBD_DELAY);
        }  
        break;

      case XK_Up:
        // traverse tree up
        if (_selected->prevsibling) {
          next = _selected->prevsibling;
          while (next->open && next->firstchild) {
            next = next->firstchild;
            while (next->nextsibling) next = next->nextsibling;
          }
        } else if (_selected != _first) {
          next = _selected->parent;
        }
        if (_timer) delete _timer;
        _timer = new OTimer(this, KBD_DELAY);
        break;

      case XK_Down:
        // traverse tree down
        if (_selected->open && _selected->firstchild) {
          next = _selected->firstchild;
        } else if (_selected->nextsibling) {
          next = _selected->nextsibling;
        } else {
          next = _selected;
          while (next != _first) {
            next = next->parent;
            if (next->nextsibling) {
              next = next->nextsibling;
              break;
            }
          }
          if (next == _first) next = _selected;
        }
        if (_timer) delete _timer;
        _timer = new OTimer(this, KBD_DELAY);
        break;

      case XK_Page_Up:
        break;

      case XK_Page_Down:
        break;

      case XK_Home:
        // go to the very first node
        next = _first;
        if (_timer) delete _timer;
        _timer = new OTimer(this, KBD_DELAY);
        break;

      case XK_End:
        // go to the end of the tree
        next = _first;
        while (next->open && next->firstchild) {
          while (next->nextsibling) next = next->nextsibling;
        }
        if (_timer) delete _timer;
        _timer = new OTimer(this, KBD_DELAY);
        break;

      case XK_Return:
      case XK_KP_Enter:
        break;

      case XK_space:
        break;

      case XK_plus:
      case XK_KP_Add:
        // expand one level
        _selected->open = True;
        NeedRedraw(ORectangle(0, _selected->y,
                   _virtualSize.w, _canvas->GetHeight()));
        msg.action = MSG_DBLCLICK;
        SendMessage(_msgObject, &msg);
        break;

      case XK_minus:
      case XK_KP_Subtract:
        // collapse one level
        _selected->open = False;
        NeedRedraw(ORectangle(0, _selected->y,
                   _virtualSize.w, _canvas->GetHeight()));
        msg.action = MSG_DBLCLICK;
        SendMessage(_msgObject, &msg);
        break;

      case XK_asterisk:
      case XK_KP_Multiply:
        // fully expand this node
        break;

      default: {
          char input[2] = { 0, 0 };
          int  charcount;
          KeySym keysym;
          XComposeStatus compose = { NULL, 0 };
          charcount = XLookupString(event, input, sizeof(input)-1,
                                    &keysym, &compose);
          if (charcount > 0) {
            int passes = 0;
            next = _selected;
            while (passes < 2) {
              if (next->open && next->firstchild) {
                next = next->firstchild;
              } else if (next->nextsibling) {
                next = next->nextsibling;
              } else {
                while (next != _first) {
                  next = next->parent;
                  if (next->nextsibling) {
                    next = next->nextsibling;
                    break;
                  }
                }
                if (next == _first) ++passes;
              }
              if (next->text[0] == input[0]) break;
            }
            if (passes < 2) {
              if (_timer) delete _timer;
              _timer = new OTimer(this, KBD_DELAY);
            } else {
              next = _selected;
            }
          }
        }
        break;
    }

    if (next != _selected) {
      _HighlightItem(_selected, False, True);
      _selected = next;
      _EnsureVisible(_selected);
    }
    _HighlightItem(_selected, True, True);
  }

  return True;
}

int OXListTree::HandleTimer(OTimer *t) {
  if (t != _timer) return False;

  OListTreeMessage msg(_msgType, MSG_CLICK, _widgetID, 0, 0, 0);
  SendMessage(_msgObject, &msg);

  delete _timer;
  _timer = NULL;

  return True;
}

void OXListTree::_GotFocus() {
  OXFrame::_GotFocus();
  AddInput(KeyPressMask | KeyReleaseMask);
  _focused = True;
  ShowFocusHilite(True);
}

void OXListTree::_LostFocus() {
  OXFrame::_LostFocus();
  RemoveInput(KeyPressMask | KeyReleaseMask);
  _focused = False;
  ShowFocusHilite(False);

  OListTreeMessage message(_msgType, MSG_FOCUSLOST, _widgetID);
  SendMessage(_msgObject, &message);
}

void OXListTree::ShowFocusHilite(int onoff) {
  if (_selected) {
    NeedRedraw(ORectangle(0, _selected->y,
               _virtualSize.w, _selected->height));
  }
}

void OXListTree::_EnsureVisible(OListTreeItem *item) {
  if (item) {
    OPosition p = OPosition(item->xpic, item->y);

    if (p.y < _visibleStart.y + _margin) {
      ScrollToPosition(OPosition(_visibleStart.x, p.y - _margin));
    } else if (p.y > _visibleStart.y + _canvas->GetHeight()
                     - item->height - _margin) {
      ScrollToPosition(OPosition(_visibleStart.x, p.y - _canvas->GetHeight()
                     + item->height + _margin));
    }

    if (p.x < _visibleStart.x + _margin) {
      ScrollToPosition(OPosition(p.x - _margin, _visibleStart.y));
    } else if (p.x > _visibleStart.x + _canvas->GetWidth()
                     - (item->picWidth * 2) - _margin) {
      ScrollToPosition(OPosition(p.x - _canvas->GetWidth() +
                       (item->picWidth * 2) + _margin, _visibleStart.y));
    }
  }
}

void OXListTree::UpdateBackgroundStart() {
  OXView::UpdateBackgroundStart();

  XGCValues gcv;
  unsigned int mask = GCTileStipXOrigin | GCTileStipYOrigin;

  gcv.ts_x_origin = -_visibleStart.x;
  gcv.ts_y_origin = -_visibleStart.y;
  XChangeGC(GetDisplay(), _lineGC, mask, &gcv);
}


//--------------------------------------------------- drawing functions

// This also recalculates the size and calls Layout(), hmmm...

void OXListTree::_Draw(int yevent, int hevent) {
  OListTreeItem *item;
  int x, y, width, height, old_width, old_height;
  int xbranch;

  // Overestimate the expose region to be sure to draw an item
  // that gets cut by the region

  _exposeTop = yevent - _th;
  _exposeBottom = yevent + hevent + _th;
  old_width  = _virtualSize.w;
  old_height = _virtualSize.h;
  _virtualSize = ODimension(1, 1);

  x = _margin;
  y = _margin;
  item = _first;
  while (item) {
    xbranch = -1;
    _DrawItem(item, x, y, &xbranch, &width, &height);

    width += x + _hspacing + _margin;

    if (width > _virtualSize.w) _virtualSize.w = width;

    y += height + _vspacing;
    if (item->firstchild && item->open)
      y = _DrawChildren(item->firstchild, x, y, xbranch);

    item = item->nextsibling;
  }

  _virtualSize.h = y + _margin;

  if ((old_width != _virtualSize.w) || (old_height != _virtualSize.h)) {
    Layout();
  }
}

int OXListTree::_DrawChildren(OListTreeItem *item, int x, int y, int xroot) {
  int width, height;
  int xbranch;

  x += _indent + item->picWidth;
  while (item) {
    xbranch = xroot;
    _DrawItem(item, x, y, &xbranch, &width, &height);

    width += x + _hspacing + _margin;

    if (width > _virtualSize.w) _virtualSize.w = width;

    y += height + _vspacing;
    if ((item->firstchild) && (item->open))
      y = _DrawChildren(item->firstchild, x, y, xbranch);

    item = item->nextsibling;
  }
  return y;
}

// The next 3 routines are the only ones that actually draw to the screen 

void OXListTree::_DrawItem(OListTreeItem *item, int x, int y, int *xroot,
                           int *retwidth, int *retheight) {
  int height, xpic, ypic, xbranch, ybranch, xtext, ytext, yline, xc;
  const OPicture *pic;

  // Select the pixmap to use, if any
  if (item->open)
    pic = item->open_pic;
  else
    pic = item->closed_pic;

  // Compute the height of this line
  height = _th;
  xpic = x;
  xtext = x + _hspacing + item->picWidth;
  if (pic) {
    if (pic->GetHeight() > height) {
      ytext = y + ((pic->GetHeight() - height) >> 1);
      height = pic->GetHeight();
      ypic = y;
    } else {
      ytext = y;
      ypic = y + ((height - pic->GetHeight()) / 2);
    }
    xbranch = xpic + (item->picWidth >> 1);
    ybranch = ypic + pic->GetHeight();
    yline = ypic + (pic->GetHeight() >> 1);
  } else {
    ypic = ytext = y;
    xbranch = xpic + (item->picWidth >> 1);
    yline = ybranch = ypic + (height >> 1);
    yline = ypic + (height >> 1);
  }

  // height must be even, otherwise our dashed line will not appear properly
  ++height; height &= ~1;

  // Save the basic graphics info for use by other functions
  item->y = y;
  item->xnode = *xroot;
  item->xpic = xpic;
  item->ypic = ypic;
  item->xtext = xtext;
  item->ytext = ytext;
  item->height = height;

  if ((y+height >= _exposeTop) && (y <= _exposeBottom)) {
    OPosition p1, p2;

    if (*xroot >= 0) {
      xc = *xroot;

      p1 = ToPhysical(OPosition(xc, y));
      p2 = ToPhysical(OPosition(xc, yline));

      if (item->nextsibling)
        _canvas->DrawLine(_lineGC, p1.x, p1.y, p1.x, p1.y + height +1);
      else
        _canvas->DrawLine(_lineGC, p1.x, p1.y, p2.x, p2.y);

      OListTreeItem *p = item->parent;
      while (p) {
        xc -= (_indent + item->picWidth);
        p1 = ToPhysical(OPosition(xc, y));
        if (p->nextsibling)
          _canvas->DrawLine(_lineGC, p1.x, p1.y, p1.x, p1.y + height +1);
        p = p->parent;
      }

      p1 = ToPhysical(OPosition(*xroot, yline));
      p2 = ToPhysical(OPosition(xpic/*xbranch*/, yline));

      _canvas->DrawLine(_lineGC, p1.x, p1.y, p2.x, p2.y);
      _DrawNode(item, *xroot, yline);
    }

    if (item->open && item->firstchild) {
      p1 = ToPhysical(OPosition(xbranch, ybranch/*yline*/));
      p2 = ToPhysical(OPosition(xbranch, y));
      _canvas->DrawLine(_lineGC, p1.x, p1.y, p2.x, p2.y + height +1);
    }

    _DrawItemPic(item);
    _DrawItemName(item);
  }

  *xroot = xbranch;
  *retwidth = _font->TextWidth(item->text) + item->picWidth;
  *retheight = height;
}

void OXListTree::_DrawItemPic(OListTreeItem *item) {

  OPosition p = ToPhysical(OPosition(item->xpic, item->ypic));

  _canvas->ClearArea(p.x, p.y, item->picWidth, item->height);
  if (item->active || (item == _selected))
    item->open_pic->Draw(GetDisplay(), _canvas->GetId(), _drawGC, p.x, p.y);
  else
    item->closed_pic->Draw(GetDisplay(), _canvas->GetId(), _drawGC, p.x, p.y);
}

void OXListTree::_DrawItemName(OListTreeItem *item) {

  int width = _font->TextWidth(item->text);
  OPosition p = ToPhysical(OPosition(item->xtext, item->ytext));

  if (item->active || item == _selected) {
    XSetForeground(GetDisplay(), _drawGC, _defaultSelectedBackground);
    _canvas->FillRectangle(_drawGC, p.x, p.y, width + 1, _th + 1);
    XSetForeground(GetDisplay(), _drawGC, _blackPixel);
    _canvas->DrawString(_highlightGC, p.x, p.y + _ascent,
                        item->text, item->length);
#if 1
    if (_focused) {
      _canvas->DrawRectangle(_client->GetResourcePool()->GetDocumentBckgndGC()->GetGC(),
                             p.x, p.y, width, _th);
      _canvas->DrawRectangle(_client->GetResourcePool()->GetFocusHiliteGC()->GetGC(),
                             p.x, p.y, width, _th);
    }
#endif
  } else {
    _canvas->FillRectangle(_highlightGC, p.x, p.y, width + 1, _th + 1);
    _canvas->DrawString(_drawGC, p.x, p.y + _ascent,
                        item->text, item->length);
  }
}

void OXListTree::_DrawNode(OListTreeItem *item, int x, int y) {
  if (item->firstchild) {
    OPosition p = ToPhysical(OPosition(x, y));

    _canvas->FillRectangle(_highlightGC, p.x - 3, p.y - 3, 7, 7);
    XSetForeground(GetDisplay(), _highlightGC, _blackPixel);
    _canvas->DrawLine(_highlightGC, p.x - 2, p.y, p.x + 2, p.y);
    if (!item->open)
      _canvas->DrawLine(_highlightGC, p.x, p.y - 2, p.x, p.y + 2);
    XSetForeground(GetDisplay(), _highlightGC, _grayPixel);
    _canvas->DrawRectangle(_highlightGC, p.x - 4, p.y - 4, 8, 8);
    XSetForeground(GetDisplay(), _highlightGC, _whitePixel);
  }
}

//-----------------------------------------------------------------

// This function removes the specified item from the linked list.
// It does not do anything with the data contained in the item, though.

void OXListTree::_RemoveReference(OListTreeItem *item) {

  // If there exists a previous sibling, just skip over item to be
  // dereferenced
  if (item->prevsibling) {
    item->prevsibling->nextsibling = item->nextsibling;
    if (item->nextsibling)
      item->nextsibling->prevsibling = item->prevsibling;
  }
  // If not, then the deleted item is the first item in some branch.
  else {
    if (item->parent)
      item->parent->firstchild = item->nextsibling;
    else
      _first = item->nextsibling;
    if (item->nextsibling)
      item->nextsibling->prevsibling = NULL;
  }
}

void OXListTree::_DeleteChildren(OListTreeItem *item) {
  OListTreeItem *sibling;

  while (item) {
    if (item->firstchild) {
      _DeleteChildren(item->firstchild);
      item->firstchild = NULL;
    }
    sibling = item->nextsibling;
    delete item;
    item = sibling;
  }
}

void OXListTree::_InsertChild(OListTreeItem *parent, OListTreeItem *item) {
  OListTreeItem *i;

  item->parent = parent;
  item->nextsibling = item->prevsibling = NULL;
  if (parent) {

    if (parent->firstchild) {
      i = parent->firstchild;
      while (i->nextsibling) i = i->nextsibling;
      i->nextsibling = item;
      item->prevsibling = i;
    } else {
      parent->firstchild = item;
    }

  } else {  // if parent == NULL, this is a top level entry

    if (_first) {
      i = _first;
      while (i->nextsibling) i = i->nextsibling;
      i->nextsibling = item;
      item->prevsibling = i;
    } else {
      _first = item;
    }

  }
}

// Insert a list of ALREADY LINKED children into another list

void OXListTree::_InsertChildren(OListTreeItem *parent, OListTreeItem *item) {
  OListTreeItem *next, *newnext;

//  while (item) {
//    next = item->nextsibling;
//    _InsertChild(parent, item);
//    item = next;
//  }
//  return;

  // Save the reference for the next item in the new list
  next = item->nextsibling;

  // Insert the first item in the new list into the existing list
  _InsertChild(parent, item);

  // The first item is inserted, with its prev and next siblings updated
  // to fit into the existing list. So, save the existing list reference
  newnext = item->nextsibling;

  // Now, mark the first item's next sibling to point back to the new list
  item->nextsibling = next;

  // Mark the parents of the new list to the new parent. The order of the
  // rest of the new list should be OK, and the second item should still
  // point to the first, even though the first was reparented.
  while (item->nextsibling) {
    item->parent = parent;
    item = item->nextsibling;
  }

  // Fit the end of the new list back into the existing list
  item->nextsibling = newnext;
  if (newnext)
    newnext->prevsibling = item;
}

int OXListTree::_SearchChildren(OListTreeItem *item, int y, int findy,
                                OListTreeItem **finditem) {
  int height;
  const OPicture *pic;

  while (item) {
    // Select the pixmap to use, if any
    if (item->open)
      pic = item->open_pic;
    else
      pic = item->closed_pic;

    // Compute the height of this line
    height = _th;
    if (pic && (pic->GetHeight() > height))
      height = pic->GetHeight();

    if ((findy >= y) && (findy <= y + height)) {
      *finditem = item;
      return -1;
    }

    y += height + _vspacing;
    if ((item->firstchild) && (item->open)) {
      y = _SearchChildren(item->firstchild, y, findy, finditem);
      if (*finditem) return -1;
    }

    item = item->nextsibling;
  }

  return y;
}

OListTreeItem *OXListTree::_FindItem(int findy) {
  int y, height;
  OListTreeItem *item, *finditem;
  const OPicture *pic;

  y = _margin;
  item = _first;
  finditem = NULL;
  while (item && !finditem) {
    // Select the pixmap to use, if any
    if (item->open)
      pic = item->open_pic;
    else
      pic = item->closed_pic;

    // Compute the height of this line
    height = _th;
    if (pic && (pic->GetHeight() > height))
      height = pic->GetHeight();

    if ((findy >= y) && (findy <= y + height))
      return item;

    y += height + _vspacing;
    if ((item->firstchild) && (item->open)) {
      y = _SearchChildren(item->firstchild,
	 		  y, findy, &finditem);
      // if (finditem) return finditem;
    }
    item = item->nextsibling;
  }

  return finditem;
}

//----------------------------------------------- Public Functions

void OXListTree::AddItem(OListTreeItem *parent, OListTreeItem *item) {

  if (!item->open_pic)
    item->open_pic = _client->GetPicture("ofolder.t.xpm");
  if (!item->open_pic)
    item->open_pic = _client->GetPicture("ofolder.t.xpm", XCP_ofolder_t_xpm);

  if (!item->closed_pic)
    item->closed_pic = _client->GetPicture("folder.t.xpm");
  if (!item->closed_pic)
    item->closed_pic = _client->GetPicture("folder.t.xpm", XCP_folder_t_xpm);

  _InsertChild(parent, item);

  int affects_display = True;

  while (parent) {
    if (!parent->open) {
      affects_display = False;
      break;
    }
    parent = parent->parent;
  }

  if (affects_display)
    NeedRedraw(ORectangle(_visibleStart, _canvas->GetSize()));
}

OListTreeItem *OXListTree::AddItem(OListTreeItem *parent, const char *string,
                                   const OPicture *open,
                                   const OPicture *closed) {
  OListTreeItem *item;

  if (!open)
    open = _client->GetPicture("ofolder.t.xpm");
  if (!open)
    open = _client->GetPicture("ofolder.t.xpm", XCP_ofolder_t_xpm);

  if (!closed)
    closed = _client->GetPicture("folder.t.xpm");
  if (!closed)
    closed = _client->GetPicture("folder.t.xpm", XCP_folder_t_xpm);

  item = new OListTreeItem(_client, string, open, closed);
  _InsertChild(parent, item);

  int affects_display = True;

  while (parent) {
    if (!parent->open) {
      affects_display = False;
      break;
    }
    parent = parent->parent;
  }

  if (affects_display)
    NeedRedraw(ORectangle(_visibleStart, _canvas->GetSize()));

  return item;
}

void OXListTree::RenameItem(OListTreeItem *item, const char *string) {
  item->Rename(string);
  NeedRedraw(ORectangle(0, item->y, _virtualSize.w, item->height));
}

int OXListTree::DeleteItem(OListTreeItem *item) {
  int y = item->y;
  OListTreeItem *parent = item->parent;

  if (item->firstchild)
    _DeleteChildren(item->firstchild);

  item->firstchild = NULL;
  _RemoveReference(item);
  delete item;

  int affects_display = True;

  while (parent) {
    if (!parent->open) {
      affects_display = False;
      break;
    }
    parent = parent->parent;
  }

  if (affects_display)
    NeedRedraw(ORectangle(_visibleStart, _canvas->GetSize()));

  return 1;
}

int OXListTree::DeleteChildren(OListTreeItem *item) {
  int y = item->y;

  if (item->firstchild)
    _DeleteChildren(item->firstchild);

  item->firstchild = NULL;

  int affects_display = True;

  for (item = item->parent; item; item = item->parent) {
    if (!item->open) {
      affects_display = False;
      break;
    }
  }

  if (affects_display)
    NeedRedraw(ORectangle(_visibleStart, _canvas->GetSize()));

  return 1;
}

int OXListTree::Reparent(OListTreeItem *item, OListTreeItem *newparent) {

  // Remove the item from its old location.
  _RemoveReference(item);

  // The item is now unattached. Reparent it.
  _InsertChild(newparent, item);

  NeedRedraw(ORectangle(_visibleStart, _canvas->GetSize()));

  return 1;
}

int OXListTree::ReparentChildren(OListTreeItem *item,
                                 OListTreeItem *newparent) {
  OListTreeItem *first;

  if (item->firstchild) {
    first = item->firstchild;
    item->firstchild = NULL;

    _InsertChildren(newparent, first);

    NeedRedraw(ORectangle(_visibleStart, _canvas->GetSize()));
    return 1;
  }
  return 0;
}

void OXListTree::OpenNode(OListTreeItem *item) {
  if (!item->open) {
    item->open = True;
    NeedRedraw(ORectangle(0, item->y,
               _virtualSize.w, _canvas->GetHeight()));
  }
}

void OXListTree::CloseNode(OListTreeItem *item) {
  if (item->open) {
    item->open = False;
    NeedRedraw(ORectangle(0, item->y,
               _virtualSize.w, _canvas->GetHeight()));
  }
}

//static int OXListTree::_compare(const void *item1, const void *item2) {
int _compare(const void *item1, const void *item2) {
  return strcmp((*((OListTreeItem **) item1))->text,
		(*((OListTreeItem **) item2))->text);
}

// Sort siblings

int OXListTree::Sort(OListTreeItem *item) {
  OListTreeItem *first, *parent, **list;
  size_t i, count;

  // Get first child in list;
  while (item->prevsibling) item = item->prevsibling;

  first = item;

  i = 0;
  while (item->nextsibling) {
    if (strcmp(item->text, item->nextsibling->text) > 0) { i = 1; break; }
    item = item->nextsibling;
  }

  if (i == 0) return 1;  // no need to sort!

  parent = first->parent;

  // Count the children
  count = 1;
  for (item = first ; item->nextsibling ; item = item->nextsibling) count++;
  if (count <= 1) return 1;

  list = new OListTreeItem* [count];
  list[0] = item = first;
  count = 1;
  while (item->nextsibling) {
    list[count] = item->nextsibling;
    count++;
    item = item->nextsibling;
  }

  qsort(list, count, sizeof(OListTreeItem*), _compare);

  list[0]->prevsibling = NULL;
  for (i = 0; i < count; i++) {
    if (i < count - 1)
      list[i]->nextsibling = list[i + 1];
    if (i > 0)
      list[i]->prevsibling = list[i - 1];
  }
  list[count - 1]->nextsibling = NULL;
  if (parent)
    parent->firstchild = list[0];
  else
    _first = list[0];

  delete[] list;

  int affects_display = True;

  for (item = first->parent; item; item = item->parent) {
    if (!item->open) {
      affects_display = False;
      break;
    }
  }

  if (affects_display)
    NeedRedraw(ORectangle(_visibleStart, _canvas->GetSize()));

  return 1;
}

int OXListTree::SortSiblings(OListTreeItem *item) {
  return Sort(item);
}

int OXListTree::SortChildren(OListTreeItem *item) {
  OListTreeItem *first = NULL;

  if (item) {
    first = item->firstchild;
  } else {
    if (_first) first = _first->firstchild;
  }
  if (first)
    SortSiblings(first);
  return 1;
}

OListTreeItem *OXListTree::FindSiblingByName(OListTreeItem *item,
                                             const char *name) {
  OListTreeItem *first;

  // Get first child in list;
  if (item) {
    while (item->prevsibling)
      item = item->prevsibling;
    first = item;

    while (item) {
      if (strcmp(item->text, name) == 0)
	return item;
      item = item->nextsibling;
    }
    return item;
  }
  return NULL;
}

OListTreeItem *OXListTree::FindChildByName(OListTreeItem *item,
                                           const char *name) {

  // Get first child in list
  if (item && item->firstchild) {
    item = item->firstchild;
  } else if (!item && _first) {
    item = _first;
  } else {
    item = NULL;
  }

  while (item) {
    if (strcmp(item->text, name) == 0)
      return item;
    item = item->nextsibling;
  }
  return NULL;
}

void OXListTree::HighlightItem(OListTreeItem *item, int ensure_visible) {
  _UnselectAll(True);
  _HighlightItem(_selected = item, True, True);
  if (ensure_visible) _EnsureVisible(item);
}

void OXListTree::ClearHighlighted() {
  _UnselectAll(True);
}

void OXListTree::EnsureVisible(OListTreeItem *item) {
  _EnsureVisible(item);
}

void OXListTree::GetPathnameFromItem(OListTreeItem *item, char *path) {
  char tmppath[1024];

  *path = '\0';
  while (item) {
    sprintf(tmppath, "/%s%s", item->text, path);
    strcpy(path, tmppath);
    item = item->parent;
  }
}
