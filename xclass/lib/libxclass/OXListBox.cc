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

#include <X11/keysym.h>

#include <xclass/utils.h>
#include <xclass/OXListBox.h>
#include <xclass/ODimension.h>
#include <xclass/OXClient.h>
#include <xclass/OXFont.h>
#include <xclass/OGC.h>
#include <xclass/OResourcePool.h>
#include <xclass/OString.h>


unsigned long OXTextLBEntry::_selPixel;
OXGC  *OXTextLBEntry::_defaultGC;            // this is not shared.
const OXFont *OXTextLBEntry::_defaultFont;
int   OXTextLBEntry::_init = False;

//-----------------------------------------------------------------

void OXLBEntry::Activate(int a) {
  if (_active == a) return;
  _active = a;
  NeedRedraw();
}

//-----------------------------------------------------------------

OXTextLBEntry::OXTextLBEntry(const OXWindow *p, OString *s, int ID,
                             unsigned int options, unsigned long back) :
  OXLBEntry(p, ID, options, back) {

    if (!_init) {
      _defaultFont = GetResourcePool()->GetDefaultFont();
      _defaultGC = new OXGC(GetResourcePool()->GetDocumentGC());
      _defaultGC->SetFont(_defaultFont->GetId());
      _selPixel = GetResourcePool()->GetSelectedFgndColor();
      _init = True;
    }

    _text = s;
    _font = _defaultFont;
    _normGC = _defaultGC;

    OFontMetrics fm;
    _font->GetFontMetrics(&fm);
    _tw = _font->TextWidth(_text->GetString(), _text->GetLength());
    _th = _font->TextHeight();
    _ta = fm.ascent;
    Resize(_tw, _th + 1);
}

OXTextLBEntry::~OXTextLBEntry() {
  if (_text) delete _text;
  if (_normGC != _defaultGC) delete _normGC;
  if (_font != _defaultFont) _client->FreeFont((OXFont *) _font);
}

void OXTextLBEntry::_DoRedraw() {
  int x, y;
  unsigned long old_bg;
  
  //OXFrame::_DoRedraw();

  x = 3;
  y = (_h - _th) >> 1;

  old_bg = _backPixel;
  if (_active) {
    SetBackgroundColor(_defaultSelectedBackground);
    ClearWindow();
    _normGC->SetForeground(_selPixel);
  } else {
    SetBackgroundColor(_backPixel);
    ClearWindow();
    _normGC->SetForeground(GetResourcePool()->GetDocumentFgndColor());
  }
  _backPixel = old_bg;

  _text->Draw(GetDisplay(), _id, _normGC->GetGC(), x, y + _ta);

  if (_focused) {
    _normGC->SetForeground(_backPixel);
    DrawRectangle(_normGC->GetGC(),
                  0, 0, _w-1, _h-1);
    DrawRectangle(GetResourcePool()->GetFocusHiliteGC()->GetGC(),
                  0, 0, _w-1, _h-1);
    _normGC->SetForeground(GetResourcePool()->GetDocumentFgndColor());
  }
}

void OXTextLBEntry::SetText(OString *new_text) {

  if (_text) delete _text;
  _text = new_text;

  _tw = _font->TextWidth(_text->GetString(), _text->GetLength());

  Resize(_tw, _th + 1);  //////

  NeedRedraw();
}

//-----------------------------------------------------------------

OXLBContainer::OXLBContainer(const OXWindow *p, int w, int h,
                             unsigned int options, unsigned long back) :
  OXCompositeFrame(p, w, h, options, back) {

    _last_active = NULL;
    _msgObject = p;

    _multipleSelect = False;
    _dragging = False;
    _focused = False;

//    SetLayoutManager(new OColumnLayout(this, 0));

    XGrabButton(GetDisplay(), AnyButton, AnyModifier, _id, False,
                ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                GrabModeAsync, GrabModeAsync, None, None);
}

OXLBContainer::~OXLBContainer() {
  SListFrameElt *ptr, *next;
  OXFrame *f;
  OLayoutHints *l;

  ptr = _flist;
  while (ptr) {
    next = ptr->next;
    f = ptr->frame;
    l = ptr->layout;
    delete f;
    delete l;
    delete ptr;
    ptr = next;
  }
  // =!= (can cause troubles here) XDestroySubwindows(GetDisplay(), _id);
  // =!= (in case that OXListBox is being deleted from a recursive
  // =!= OXMainFrame deletion, at this point the current window no
  // =!= longer exists)
  _flist = NULL; // =!= well, having this here will keep OXCompositeFrame happy.
  _ftail = NULL;
}

void OXLBContainer::SetBackgroundColor(unsigned long color) {
  SListFrameElt *ptr;

  OXCompositeFrame::SetBackgroundColor(color);

  for (ptr = _flist; ptr != NULL; ptr = ptr->next) {
    ptr->frame->SetBackgroundColor(color);
  }
}

void OXLBContainer::AddEntry(OXLBEntry *lbe, OLayoutHints *lhints) {
  lbe->SetBackgroundColor(_backPixel);
  AddFrame(lbe, lhints);
  //Layout(); // called from OXListBox
  //lbe->MapWindow();
}

void OXLBContainer::MoveSelectUp(OXLBEntry *lbe) {
  OXLBEntry *e, *first = NULL;
  SListFrameElt *ptr;

  if (!lbe) return;

  //printf("OXLBContainer::MoveSelectUp(ID:%d)\n", lbe->ID());
  //if (lbe) {
  //  lbe->Activate(False);
  //  lbe->ShowFocusHilite(False);  // ==!==
  //}

  if (_flist)
    first = (OXLBEntry *) _flist->frame;

  if (lbe == first) {
#if 0
    ptr = _flist; 
    while (ptr->next != NULL) ptr = ptr->next;
    e = (OXLBEntry *) ptr->frame;
#else
    return;
#endif
  } else {
    for (ptr = _flist; ptr->next != NULL; ptr = ptr->next) {
      if ((OXLBEntry *) ptr->next->frame == lbe) break;
    }
    e = (OXLBEntry *) ptr->frame;
  }

  //printf("Selecting %d\n", e->ID());
  Select(e->ID());
  e->Activate(True);
  e->ShowFocusHilite(_focused);
  //_last_active = e;
  //Select(e->ID());

  OContainerMessage message(MSG_CONTAINER, MSG_CLICK, e->ID(), 0);
  SendMessage(_msgObject, &message);
}

void OXLBContainer::MoveSelectDown(OXLBEntry *lbe){
  OXLBEntry *e, *first = NULL;
  SListFrameElt *ptr;

  if (!lbe) return;

  //if (lbe) {
  //  lbe->Activate(False);
  //  lbe->ShowFocusHilite(False);  // ==!==
  //  printf("OXLBContainer::MoveSelectDown(ID:%d)\n",lbe->ID());
  //}

  if (_flist)
    first = (OXLBEntry *) _flist->frame;

  for (ptr = _flist; ptr->next != NULL; ptr = ptr->next) {
    if ((OXLBEntry *) ptr->frame == lbe) {
      e = (OXLBEntry *) ptr->next->frame;
      break;
    }
  }

#if 0
  if (!ptr->next) e = first;
#else
  if (!ptr->next) return;
#endif

  //printf("Selecting %d\n",e->ID());
  Select(e->ID());
  e->Activate(True);
  e->ShowFocusHilite(_focused);
  //_last_active = e;
  //Select(e->ID());

  OContainerMessage message(MSG_CONTAINER, MSG_CLICK, e->ID(), 0);
  SendMessage(_msgObject, &message);
}

void OXLBContainer::InsertEntry(OXLBEntry *lbe, OLayoutHints *lhints, int afterID) {
  OXLBEntry *e;
  SListFrameElt *nw, *ptr;

  lbe->SetBackgroundColor(_backPixel);

  for (ptr = _flist; ptr != NULL; ptr = ptr->next) {
    e = (OXLBEntry *) ptr->frame;
    if (e->ID() == afterID) break;
  }

  if (ptr == NULL) {
    AddFrame(lbe, lhints);
  } else {
    nw = new SListFrameElt;
    nw->frame = lbe;
    nw->layout = lhints;
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
  //lbe->MapWindow();
  //Layout(); // called from OXListBox
}

void OXLBContainer::RemoveEntry(int ID) {
  OXLBEntry *e;
  SListFrameElt *ptr;
  OLayoutHints *l;

  for (ptr = _flist; ptr != NULL; ptr = ptr->next) {
    e = (OXLBEntry *) ptr->frame;
    l = ptr->layout;
    if (e->ID() == ID) {
      if (_last_active == e) _last_active = NULL;
      e->DestroyWindow();
      RemoveFrame(e);
      delete e;
      delete l;
      //Layout(); // called from OXListBox
      break;
    }
  }
}

void OXLBContainer::RemoveEntries(int from_ID, int to_ID) {
  OXLBEntry *e;
  SListFrameElt *ptr, *next;
  OLayoutHints *l;

  ptr = _flist;
  while (ptr) {
    next = ptr->next;
    e = (OXLBEntry *) ptr->frame;
    l = ptr->layout;
    if ((e->ID() >= from_ID) && (e->ID() <= to_ID)) {
      if (_last_active == e) _last_active = NULL;
      e->DestroyWindow();
      RemoveFrame(e);
      delete e;
      delete l;
    }
    ptr = next;
  }
  //Layout();  // called from OXListBox
}

void OXLBContainer::RemoveAllEntries() {
  OXLBEntry *e;
  SListFrameElt *ptr, *next;
  OLayoutHints *l;

  ptr = _flist;
  while (ptr) {
    next = ptr->next;
    e = (OXLBEntry *) ptr->frame;
    l = ptr->layout;
    //e->DestroyWindow();  // XDestroySubwindows() below is faster...
    RemoveFrame(e);
    delete e;
    delete l;
    ptr = next;
  }
  XDestroySubwindows(GetDisplay(), _id);
  _flist = NULL;
  _last_active = NULL;
  //Layout();  // called from OXListBox
}

void OXLBContainer::UnSelectAll() {
  SListFrameElt *ptr;
  OXLBEntry *f;

  for (ptr = _flist; ptr != NULL; ptr = ptr->next) {   
    f = (OXLBEntry *) ptr->frame;
    f->Activate(False);
    f->ShowFocusHilite(False);  // ==!==
  }

  _last_active = NULL;

  OContainerMessage message(MSG_CONTAINER, MSG_SELCHANGED, -1, 0, 0, 0);
  SendMessage(_msgObject, &message);
}

void OXLBContainer::SelectAll() {
  SListFrameElt *ptr;
  OXLBEntry *f;
  int total = 0;

  if (!_multipleSelect) return;

  for (ptr = _flist; ptr != NULL; ptr = ptr->next) {   
    f = (OXLBEntry *) ptr->frame;
    f->Activate(True);
    ++total;
  }

  OContainerMessage message(MSG_CONTAINER, MSG_SELCHANGED, -1, 0, 
                            total, total);
  SendMessage(_msgObject, &message);
}

OXLBEntry *OXLBContainer::Select(int ID) {
  SListFrameElt *ptr;
  OXLBEntry *f;

  if (_last_active) {
    f = (OXLBEntry *) _last_active;
    f->Activate(False);
    f->ShowFocusHilite(False);
    _last_active = NULL;
  }

  for (ptr = _flist; ptr != NULL; ptr = ptr->next) {   
    f = (OXLBEntry *) ptr->frame;
    if (f->ID() == ID) {
      f->Activate(True);
      f->ShowFocusHilite(_focused);
      _last_active = f;
    } else {
      f->Activate(False);
      f->ShowFocusHilite(False);
    }
  }

  return _last_active;
}

OXLBEntry *OXLBContainer::FindEntry(int ID) {
  SListFrameElt *ptr;
  OXLBEntry *e;

  for (ptr = _flist; ptr != NULL; ptr = ptr->next) {   
    e = (OXLBEntry *) ptr->frame;
    if (e->ID() == ID) return e;
  }

  return NULL;
}

OXLBEntry *OXLBContainer::GetFirstEntry() {
  if (_flist) 
    return (OXLBEntry *) _flist->frame;
  else
    return NULL;
}

int OXLBContainer::GetSelected() const {
  if (_last_active == NULL) return -1;
  return _last_active->ID();
}

int OXLBContainer::HandleKey(XKeyEvent *event) {
  SListFrameElt *ptr;
  OXLBEntry *f = NULL;
  int keysym = XLookupKeysym(event, 0);

  if (event->type == KeyPress) {

    if (keysym == XK_Down) {
      if (_last_active) {
        for (ptr = _flist; ptr != NULL; ptr = ptr->next) {
          f = (OXLBEntry *) ptr->frame;
          if (f == _last_active) {
            if (ptr->next) f = (OXLBEntry *) ptr->next->frame;
            break;
          }
        }
      } else {
        if (_flist) f = (OXLBEntry *) _flist->frame;
      }

    } else if (keysym == XK_Up) {
      if (_last_active) {
        for (ptr = _flist; ptr != NULL; ptr = ptr->next) {
          f = (OXLBEntry *) ptr->frame;
          if (f == _last_active) {
            if (ptr->prev) f = (OXLBEntry *) ptr->prev->frame;
            break;
          }
        }
      } else {
        if (_flist) f = (OXLBEntry *) _flist->frame;
      }

    } else {
      return False;
    }

    if (event->state & ShiftMask) {
      if (_multipleSelect) {
        if (f) f->Activate(True);

        // TODO: send the proper values for selected and total
        OContainerMessage message(MSG_CONTAINER, MSG_SELCHANGED, -1, 0,
                                  0, 0);
        SendMessage(_msgObject, &message);
      }

    } else if (event->state & ControlMask) {
      if (_multipleSelect) {
        // toggle this item
        if (f) f->Activate(!f->IsActive());

        // TODO: send the proper values for selected and total
        OContainerMessage message(MSG_CONTAINER, MSG_SELCHANGED, -1, 0, 
                                  0, 0);
        SendMessage(_msgObject, &message);
      }

    } else {
      if (_multipleSelect) UnSelectAll();
      if (_last_active) {
        _last_active->Activate(False);
        _last_active->ShowFocusHilite(False);
        _last_active = NULL;
      }
      if (f) {
        f->Activate(True);
        f->ShowFocusHilite(_focused);
        _last_active = f;
      }
    }

  } else {

    // key release

    if (_last_active && !(_multipleSelect && _dragging)) {
      f = (OXLBEntry *) _last_active;
      OContainerMessage message(MSG_CONTAINER, MSG_CLICK, f->ID(), 0);
      SendMessage(_msgObject, &message);
    } else if (_multipleSelect && _dragging) {
      // TODO: send the proper values for selected and total
      OContainerMessage message(MSG_CONTAINER, MSG_SELCHANGED, -1, 0,
                                0, 0);
      SendMessage(_msgObject, &message);
    }
  }

  _dragging = False;
  return True;
}

int OXLBContainer::HandleButton(XButtonEvent *event) {
  SListFrameElt *ptr;
  OXLBEntry *f;
  int selected = 0;

  if (event->type == ButtonPress) {
    if (event->button == Button4) {
      return True;
    } else if (event->button == Button5) {
      return True;
    }

    if (event->state & ShiftMask) {
      if (_multipleSelect) {
        if (_last_active) {
          // highlight everything between _last_active
          // and this.
          int highlighting = False;

          for (ptr = _flist; ptr != NULL; ptr = ptr->next) {
            f = (OXLBEntry *) ptr->frame;
            if ((f->GetId() == event->subwindow) ||
                 (f == _last_active)) {
              f->Activate(True);
              if (highlighting) break;
              highlighting = True;
            }
            if (highlighting) f->Activate(True);
          }
          // TODO: send the proper values for selected and total
          OContainerMessage message(MSG_CONTAINER, MSG_SELCHANGED, -1, 0,
                                    0, 0);
          SendMessage(_msgObject, &message);
        }
      }
    } else if (event->state & ControlMask) {
      if (_multipleSelect) {
        // toggle this item
        for (ptr = _flist; ptr != NULL; ptr = ptr->next) {
          f = (OXLBEntry *) ptr->frame;
          if (f->GetId() == event->subwindow) {
            f->Activate(!f->IsActive());
            break;
          }
        }
        // TODO: send the proper values for selected and total
        OContainerMessage message(MSG_CONTAINER, MSG_SELCHANGED, -1, 0,
                                  0, 0);
        SendMessage(_msgObject, &message);
      }
    } else {
      if (_multipleSelect) UnSelectAll();
      if (_last_active) {
        f = (OXLBEntry *) _last_active;
        f->Activate(False);
        f->ShowFocusHilite(False);
        _last_active = NULL;
      }
      for (ptr = _flist; ptr != NULL; ptr = ptr->next) {   
        f = (OXLBEntry *) ptr->frame;
        if (f->GetId() == event->subwindow) {
          f->Activate(True);
          f->ShowFocusHilite(_focused);
          _last_active = f;
  	  selected = 1;
          } else {
          f->Activate(False);
          f->ShowFocusHilite(False);
        }
      }
      if (selected == 0) {
   	OContainerMessage message(MSG_CONTAINER, MSG_SELCLEARED, -1, 0,
                                  0, 0);
        SendMessage(_msgObject, &message);
      }
    }
  } else {
    if (_last_active && !(_multipleSelect && _dragging)) {
      f = (OXLBEntry *) _last_active;
      OContainerMessage message(MSG_CONTAINER, MSG_CLICK, f->ID(), 0);
      SendMessage(_msgObject, &message);
    } else if (_multipleSelect && _dragging) {
      // TODO: send the proper values for selected and total
      OContainerMessage message(MSG_CONTAINER, MSG_SELCHANGED, -1, 0,
                                0, 0);
      SendMessage(_msgObject, &message);
    }
  }

  _dragging = False;
  return True;
}

int OXLBContainer::HandleMotion(XMotionEvent *event) {
  SListFrameElt *ptr;
  OXLBEntry *f;

//  if (_last_active) {
//    f = (OXLBEntry *) _last_active;
//    f->Activate(False);
//    _last_active = NULL;
//  }
  for (ptr = _flist; ptr != NULL; ptr = ptr->next) {   
    f = (OXLBEntry *) ptr->frame;
    if (f->GetId() == event->subwindow) {
      f->Activate(True);
      if (!_multipleSelect) f->ShowFocusHilite(_focused);
      if (_last_active != f) _dragging = True;
      _last_active = f;
    } else {
      if (!_multipleSelect) {
        f->Activate(False);
        f->ShowFocusHilite(False);
      }
    }
  }

  return True;
}

void OXLBContainer::ShowFocusHilite(int onoff) {
  _focused = onoff;
  if (_last_active) {
    _last_active->ShowFocusHilite(onoff);
  } else {
    if (_flist) ((OXLBEntry *)(_flist->frame))->ShowFocusHilite(onoff);
  }
}

//-----------------------------------------------------------------

OXListBox::OXListBox(const OXWindow *p, int ID,
                     unsigned int options, unsigned long back) :
  OXCompositeFrame(p, 200, 10, options, back) {

    _vport = new OXViewPort(this, 6, 6, CHILD_FRAME|OWN_BKGND, _backPixel);
    _vscrollbar = new OXVScrollBar(this, SB_WIDTH, 6);
//    _vscrollbar->SetScrollByLine(true);
    _lbc = new OXLBContainer(_vport, 10, 10, VERTICAL_FRAME, _backPixel);
    _lbc->Associate(this);
    _SetContainer(_lbc);

    _widgetID = ID;
    _widgetType = "OXListBox";
    _msgObject = p;

    AddFrame(_vport, NULL);
    AddFrame(_vscrollbar, NULL);

    _vscrollbar->Associate(this);
    _itemVsize = 1;

    _integralHeight = True;
    _autoUpdate = True;

    _vscrollbar->AddInput(ButtonPressMask | ButtonReleaseMask |
                          PointerMotionMask);
    _lbc->AddInput(ButtonPressMask | ButtonReleaseMask
                   /*|PointerMotionMask*/);

    AddInput(FocusChangeMask);
}

OXListBox::~OXListBox() {
  // =!= delete _vscrollbar;
  // =!= (deleted by OXViewPort) delete _lbc; // should delete contents...
  // =!= delete _vport;
}

void OXListBox::SetBackgroundColor(unsigned long color) {
  OXCompositeFrame::SetBackgroundColor(color);
  _lbc->SetBackgroundColor(color);
}

void OXListBox::AddEntry(OString *s, int ID) {
  OXTextLBEntry *lbe;
  OLayoutHints *lhints;

  lbe = new OXTextLBEntry(_lbc, s, ID);
  lhints = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_TOP);
  _itemVsize = max(_itemVsize, lbe->GetDefaultHeight());
  _lbc->AddEntry(lbe, lhints);
  if (_autoUpdate) Update();
}

void OXListBox::AddEntry(OXLBEntry *lbe, OLayoutHints *lhints) {
  _itemVsize = max(_itemVsize, lbe->GetDefaultHeight());
  _lbc->AddEntry(lbe, lhints);
  if (_autoUpdate) Update();
}

void OXListBox::InsertEntry(OString *s, int ID, int afterID) {
  OXTextLBEntry *lbe;
  OLayoutHints *lhints;

  lbe = new OXTextLBEntry(_lbc, s, ID);
  lhints = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_TOP);
  _itemVsize = max(_itemVsize, lbe->GetDefaultHeight());
  _lbc->InsertEntry(lbe, lhints, afterID);
  if (_autoUpdate) Update();
}

void OXListBox::InsertEntry(OXLBEntry *lbe, OLayoutHints *lhints, int afterID) {
  _itemVsize = max(_itemVsize, lbe->GetDefaultHeight());
  _lbc->InsertEntry(lbe, lhints, afterID);
  if (_autoUpdate) Update();
}

OXLBEntry *OXListBox::Select(int ID) {
  OXLBEntry *e = _lbc->Select(ID);
  if (_autoUpdate) _EnsureVisible(e);
  return e;
}

void OXListBox::Update() {
  _lbc->MapSubwindows();
  _EnsureVisible(GetSelectedEntry());
  Layout();
}

void OXListBox::Resize(int w, int h) {
  if (_integralHeight) {
    int ih = _insets.t + _insets.b;
    h = max(_itemVsize, ((h - ih) / _itemVsize) * _itemVsize) + ih;
  }
  OXCompositeFrame::Resize(w, h);
}

void OXListBox::MoveResize(int x, int y, int w, int h) {
  if (_integralHeight) {
    int ih = _insets.t + _insets.b;
    h = max(_itemVsize, ((h - ih) / _itemVsize) * _itemVsize) + ih;
  }
  OXCompositeFrame::MoveResize(x, y, w, h);
}

ODimension OXListBox::GetDefaultSize() const {
  int h;

  if (_integralHeight) {
    int ih = _insets.t + _insets.b;
    h = max(_itemVsize, ((_h - ih) / _itemVsize) * _itemVsize) + ih;
  } else {
    h = _h;
  }

  return ODimension(_w, h);
}

void OXListBox::RemoveEntry(int ID) {
  _lbc->RemoveEntry(ID);
  if (_autoUpdate) Update();
}

void OXListBox::RemoveEntries(int from_ID, int to_ID) {
  _lbc->RemoveEntries(from_ID, to_ID);
  if (_autoUpdate) Update();
}

void OXListBox::RemoveAllEntries() {
  _lbc->RemoveAllEntries();
  if (_autoUpdate) Update();
}

void OXListBox::Layout() {
  OXFrame *_container;
  int need_vsb, cw, ch, tch;

  need_vsb = False;

  _container = (OXFrame *) _vport->GetContainer();    

  // test whether we need vertical scrollbar or not

  cw = _w - _insets.l - _insets.r;
  ch = _h - _insets.t - _insets.b;

  _container->SetWidth(cw);
  _container->SetHeight(ch);

  if (_container->GetDefaultHeight() > ch) {
    need_vsb = True;
    cw = _w - _insets.l - _insets.r - _vscrollbar->GetDefaultWidth();
    _container->SetWidth(cw);
  }

  _vport->MoveResize(_insets.l, _insets.t, cw, ch);
  _container->Layout();
  tch = max(_container->GetDefaultHeight(), ch);
  _container->SetHeight(0); // force a resize in OXFrame::Resize
  _container->Resize(cw, tch);
//  _vport->SetPos(0, 0);

  if (need_vsb) {
    _vscrollbar->MoveResize(cw + _insets.l, _insets.t,
                            _vscrollbar->GetDefaultWidth(), ch);
    _vscrollbar->MapWindow();
  } else {
    _vscrollbar->UnmapWindow();
    _vscrollbar->SetPosition(0);
  }

  _vscrollbar->SetRange(_container->GetHeight()/_itemVsize, _vport->GetHeight()/_itemVsize);

  _EnsureVisible(GetSelectedEntry());
}

int OXListBox::GetSelected() const {
  OXLBContainer *ct = (OXLBContainer *) _vport->GetContainer();
  return ct->GetSelected();
}

int OXListBox::ProcessMessage(OMessage *msg) {
  OScrollBarMessage *sbmsg;
  OContainerMessage *cmsg;

  switch (msg->type) {
    case MSG_VSCROLL:
      switch (msg->action) {
        case MSG_SLIDERTRACK:
        case MSG_SLIDERPOS:
          sbmsg = (OScrollBarMessage *) msg;
          _vport->SetVPos(-sbmsg->pos * _itemVsize);
          break;
      }
      break;

    case MSG_CONTAINER:
      cmsg = (OContainerMessage *) msg;
      switch (msg->action) {
        case MSG_CLICK:
	  {
          if (TakesFocus()) RequestFocus();  // hmmmm.... here?!?
          OListBoxMessage message(MSG_LISTBOX, MSG_CLICK, _widgetID, cmsg->id);
          SendMessage(_msgObject, &message);
	  }
          break;

	case MSG_SELCHANGED:
        case MSG_SELCLEARED:
          {
          OListBoxMessage message(MSG_LISTBOX, msg->action, _widgetID,
                                  cmsg->id, cmsg->total, cmsg->selected);
          SendMessage(_msgObject, &message);
          }
          break;
      }
      break;

    default:
      break;
   
  }

  return True;
}

void OXListBox::_GotFocus() {
  OXFrame::_GotFocus();
  AddInput(KeyPressMask | KeyReleaseMask);
  _lbc->ShowFocusHilite(True);
}

void OXListBox::_LostFocus() {
  OXFrame::_LostFocus();
  RemoveInput(KeyPressMask | KeyReleaseMask);
  _lbc->ShowFocusHilite(False); 

  // Shouldn't have to send a listbox msg for this..
  // maybe a focus message would be better..
  
  OListBoxMessage message(MSG_LISTBOX, MSG_FOCUSLOST, _widgetID);
  SendMessage(_msgObject, &message);
}

int OXListBox::HandleKey(XKeyEvent *event) {
  if (_lbc->HandleKey(event)) {
    _EnsureVisible(GetSelectedEntry());
    return True;
  }
  return False;
}

void OXListBox::_EnsureVisible(OXLBEntry *e) {
  if (e) {
    if (e->GetY() < -_lbc->GetY()) {
      int vpos = e->GetY() / _itemVsize;
      _vscrollbar->SetPosition(vpos);
    } else if ((e->GetY() + e->GetHeight()) >
               (_vport->GetHeight() - _lbc->GetY())) {
      int h = ((_vport->GetHeight()) / _itemVsize) * _itemVsize;
      int vpos = (e->GetY() + e->GetHeight() - h) / _itemVsize;
      _vscrollbar->SetPosition(vpos);
    }
  }
}
