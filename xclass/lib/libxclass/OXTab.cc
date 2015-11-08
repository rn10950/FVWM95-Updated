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

#include <string.h>

#include <xclass/utils.h>
#include <xclass/OXTab.h>
#include <xclass/OXClient.h>
#include <xclass/OXFont.h>
#include <xclass/OGC.h>
#include <xclass/OResourcePool.h>
#include <xclass/OString.h>

#include <X11/cursorfont.h>
#include <X11/keysym.h>


const OXGC *OXTab::_defaultGC;
const OXFont *OXTab::_defaultFont;
int   OXTab::_init;


//-----------------------------------------------------------------

OXTabElt::OXTabElt(const OXWindow *p, OString *text, int w, int h,
                   GC norm, const OXFont *font,
                   unsigned int options, unsigned long back) :
  OXFrame(p, w, h, options, back) {
    OFontMetrics fm;

    _text = text;
    _bw = 0;
    _normGC = norm;
    _font = font;
    _focused = False;

    _font->GetFontMetrics(&fm);
    _tw = _font->TextWidth(_text->GetString(), _text->GetLength());
    _th = _font->TextHeight();
    _yt = fm.ascent;
    Resize(max(_tw+12, 45), _th+6);
}

OXTabElt::~OXTabElt() {
  if (_text) delete _text;
}

void OXTabElt::DrawBorder() {
  DrawLine(_hilightGC, 0, _h-1, 0, 2);
  DrawLine(_hilightGC, 0, 2, 2, 0);
  DrawLine(_hilightGC, 2, 0, _w-3, 0);    
  DrawLine(_shadowGC,  _w-2, 1, _w-2, _h-1);
  DrawLine(_blackGC, _w-2, 1, _w-1, 2);
  DrawLine(_blackGC, _w-1, 2, _w-1, _h-2);

  if (_text) {
    _text->Draw(GetDisplay(), _id, _normGC, 6, _yt+3);
    if (_focused)
      DrawRectangle(GetResourcePool()->GetFocusHiliteGC()->GetGC(),
                    3, 3, _w-7, _h-7);

  }
}

ODimension OXTabElt::GetDefaultSize() const {
  return ODimension(max(_tw+12, 40), _th+6);
}

void OXTabElt::ShowFocusHilite(int onoff) {
  if (onoff != _focused) {
    _focused = onoff;
    NeedRedraw(!_focused);
  }
}


//-----------------------------------------------------------------

OTabLayout::OTabLayout(OXTab *main) {
  _main = main;
  _list = _main->GetListAddr();
}

void OTabLayout::Layout() {
  SListFrameElt *ptr;
  int i, tw, xtab;
  int _tabh = ((OXTab *)_main)->GetTabHeight(), _bw = _main->GetBorderWidth();
  int _w = _main->GetWidth();
  int _h = _main->GetHeight();

  xtab = 2;

  ((OXTab *)_main)->GetContainer()->MoveResize(0, _tabh, _w, _h - _tabh);

  // first frame is the container, so take next...
  for (ptr=(*_list)->next, i=0; ptr != NULL; ptr=ptr->next->next, ++i) {
    tw = ptr->frame->GetDefaultWidth();
    if (i == ((OXTab *)_main)->GetCurrent()) {
      ptr->frame->MoveResize(xtab-2, 0, tw+3, _tabh+1);
      ptr->next->frame->RaiseWindow();
      ptr->next->frame->SetVisible(True);
      ptr->frame->RaiseWindow();
    } else {
      ptr->frame->MoveResize(xtab, 2, tw, _tabh-1);
      ptr->frame->LowerWindow();
      ptr->next->frame->SetVisible(False);
    }
    //ptr->next->frame->MoveResize(_bw, _tabh + _bw, _w - (_bw << 1), _h - _tabh - (_bw << 1));
    ptr->next->frame->MoveResize(_bw-1, _tabh + _bw-1, _w+1 - (_bw << 1), _h+1 - _tabh - (_bw << 1));
    ptr->next->frame->Layout();
    xtab += tw;
  }
}

ODimension OTabLayout::GetDefaultSize() const {
  SListFrameElt *ptr;
  ODimension dsize;
  ODimension size(0,0);
  int state, tab_elt_width = 0;
 
  for (ptr=(*_list)->next; ptr; ptr=ptr->next->next) {
    state = ptr->next->frame->IsVisible();
    ptr->next->frame->SetVisible(True);
    dsize = ptr->next->frame->GetDefaultSize();
    ptr->next->frame->SetVisible(state);
    if (size.w < dsize.w) size.w = dsize.w;
    if (size.h < dsize.h) size.h = dsize.h;
    tab_elt_width += ptr->frame->GetDefaultSize().w;
  }
  if (size.w < tab_elt_width)
    size.w = tab_elt_width;
  size.w += _main->GetBorderWidth() << 1;
  size.h += ((OXTab *)_main)->GetTabHeight() + 
            (_main->GetBorderWidth() << 1);
  return size;
}


//-----------------------------------------------------------------

OXTab::OXTab(const OXWindow *p, int w, int h, int ID,
             unsigned int options, unsigned long back) :
  OXCompositeFrame(p, w, h, options, back), OXWidget(ID, "OXTab") {

    if (!_init) {
      _defaultFont = GetResourcePool()->GetDefaultFont();
      _defaultGC = GetResourcePool()->GetFrameGC();
      _init = True;
    }

    _msgObject = p;

    _bw = 2;
    _numtabs = 0;
    _current = 0;

    _font = _defaultFont;
    _normGC = (OXGC *) _defaultGC;

    _tabh = _font->TextHeight() + 6;

    SetLayoutManager(new OTabLayout(this));
    // we need this in order to avoid border blinking when switching tabs...
    _container = new OXCompositeFrame(this, _w, _h - _tabh,
                       VERTICAL_FRAME | RAISED_FRAME | DOUBLE_BORDER);
    OXCompositeFrame::AddFrame(_container, NULL);

    AddInput(ButtonPressMask | FocusChangeMask);
}

// Remove tab windows, etc. Labels are destroyed by OXTabElt

OXTab::~OXTab() {
/* =!= 
  SListFrameElt *ptr, *next;

  ptr = _flist;
  while (ptr) {
    next = ptr->next;
    delete ptr->frame;
    // delete ptr->layout; // ptr->layout is NULL!
    delete ptr;
    ptr = next;
  }
  _flist = _ftail = NULL; // so not to confuse OXCompositeFrame's cleanup...
*/
  if (_normGC != _defaultGC) delete _normGC;
  if (_font != _defaultFont) _client->FreeFont((OXFont *) _font);
}

OXCompositeFrame *OXTab::AddTab(OString *text) {
  OXCompositeFrame *cf;

  OXCompositeFrame::AddFrame(new OXTabElt(this, text, 50, 20,
                                          _normGC->GetGC(), _font), NULL);
  cf = new OXCompositeFrame(this, _w, _h-21);
  OXCompositeFrame::AddFrame(cf, NULL);

  ++_numtabs;

  return cf;
}

void OXTab::RemoveTab(int tabnum) {
  SListFrameElt *ptr;
  OXFrame *f;
  int i;

  for (ptr=_flist->next, i=0;
       ptr != NULL;
       ptr=ptr->next->next, ++i)
    if (i == tabnum) break;

  if (ptr == NULL) return;

  // Remove the container frame
  f = ptr->next->frame;
  XDestroyWindow(GetDisplay(), f->GetId());
  RemoveFrame(f);
  delete f;

  // Remove the tab
  f = ptr->frame;
  XDestroyWindow(GetDisplay(), f->GetId());
  RemoveFrame(f);
  delete f;

  --_numtabs;
  if (_current >= _numtabs && _current > 0) --_current; 

  Layout();
}

void OXTab::RemoveTab(char *tabname) {
  int tab = GetTabNumber(tabname);
  if (tab >= 0) RemoveTab(tab);
}

int OXTab::GetTabNumber(char *name) const {
  SListFrameElt *ptr;
  int i;

  if (!name) return -1;

  for (ptr=_flist->next, i=0; ptr != NULL; ptr=ptr->next->next, ++i) {
    OXTabElt *te = (OXTabElt *) ptr->frame;
    if (strcmp(te->GetText()->GetString(), name) == 0) return i;
  }

  return -1;
}

/*
const OString *GetCurrent() const {
  SListFrameElt *ptr;
  OXTabElt *te;

  for (ptr=_flist->next, int i=0;
       ptr != NULL; i != _current;
       ptr=ptr->next->next, ++i);

  if (ptr == NULL) return NULL;

  te = (OXTabElt *) ptr->frame;
  return te->GetText();
}
*/

int OXTab::HandleButton(XButtonEvent *event) {
  SListFrameElt *ptr;
  int i, c, xtab;

  xtab = 2;
  if (event->type == ButtonPress) {

    c = _current;
    for (ptr=_flist->next, i=0; ptr != NULL; ptr=ptr->next->next, ++i)
      if (ptr->frame->GetId() == event->subwindow) c = i;

    if (c == _current) {
      if (TakesFocus()) RequestFocus();
    } else {
      SelectTab(c, True);
    }
  }

  return True;
}

int OXTab::HandleKey(XKeyEvent *event) {
  int i = _current, keysym = XLookupKeysym(event, 0);

  if (event->type == KeyPress) {
//    if (TakesFocus()) RequestFocus();
    switch (keysym) {
      case XK_Left:
        if (--i < 0) i = _numtabs - 1;
        break;

      case XK_Right:
        if (++i >= _numtabs) i = 0;
        break;

      case XK_Home:
        i = 0;
        break;

      case XK_End:
        i = _numtabs - 1;
        break;

      default:
        break;
    }
    SelectTab(i, True);
  }

  return True;
}

void OXTab::SelectTab(int tabnum, int sendmsg) {
  if ((tabnum < 0) || (tabnum >= _numtabs) || (tabnum == _current))
    return;
  _current = tabnum;
  if (HasFocus()) _ShowFocusHilite();
  Layout();
  if (sendmsg) {
    OTabMessage message(MSG_TAB, MSG_TABCHANGED, _widgetID, _current);
    SendMessage(_msgObject, &message);
  }
}

void OXTab::_GotFocus() {
  OXFrame::_GotFocus();
  AddInput(KeyPressMask | KeyReleaseMask);
  _ShowFocusHilite();
}

void OXTab::_LostFocus() {
  OXFrame::_LostFocus();
  RemoveInput(KeyPressMask | KeyReleaseMask);
  _ShowFocusHilite();
}

void OXTab::_ShowFocusHilite() {
  SListFrameElt *ptr;
  int i;

  for (ptr=_flist->next, i=0; ptr != NULL; ptr=ptr->next->next, ++i) {
    OXTabElt *te = (OXTabElt *) ptr->frame;
    te->ShowFocusHilite((i == _current) && HasFocus());
  }
}

void OXTab::DrawBorder() {
  DrawRectangle(_bckgndGC, 0, 0, _w-1, _h-1);
  DrawRectangle(_bckgndGC, 1, 1, _w-3, _h-3);
}
