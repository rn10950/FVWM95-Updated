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

#include <xclass/utils.h>
#include <xclass/OXTextEntry.h>
#include <xclass/OXClient.h>
#include <xclass/OResourcePool.h>
#include <xclass/OXFont.h>
#include <xclass/OGC.h>
#include <xclass/OString.h>

#include <X11/keysym.h>
#include <X11/Xatom.h>


#define TE_SCROLL_NONE   0
#define TE_SCROLL_LEFT   -1
#define TE_SCROLL_RIGHT  1

const OXFont *OXTextEntry::_defaultFont;
const OXGC *OXTextEntry::_defaultGC;
const OXGC *OXTextEntry::_defaultSelGC;
const OXGC *OXTextEntry::_defaultSelBgndGC;
int   OXTextEntry::_init = False;

// TODO:
// - fix all redraw bugs
// - shift-selection
// - set cursor width function, default = 2

//-----------------------------------------------------------------

OXTextEntry::OXTextEntry(const OXWindow *p, OTextBuffer *text, int ID,
                         unsigned int options, unsigned long back)
  : OXFrame(p, 1, 1, options | OWN_BKGND, back) {
    int tw;

    if (!_init) {
      _defaultFont = GetResourcePool()->GetDocumentPropFont();
      _defaultGC = GetResourcePool()->GetDocumentGC();
      _defaultSelGC = GetResourcePool()->GetSelectedGC();
      _defaultSelBgndGC = GetResourcePool()->GetSelectedBckgndGC();
      _init = True;
    }

    _widgetID = ID;
    _widgetType = "OXTextEntry";
    _msgObject = p;

    _font = _defaultFont;

    _normGC = new OXGC(_defaultGC);
    _selGC  = new OXGC(_defaultSelGC);
    _selbackGC = new OXGC(_defaultSelBgndGC);

    _normGC->SetFont(_font->GetId());
    _selGC->SetFont(_font->GetId());
    _selbackGC->SetFont(_font->GetId());

    _text = text ? text : new OTextBuffer(256);

    tw = _TextWidth(0, _text->GetTextLength());

    OFontMetrics fm;

    _font->GetFontMetrics(&fm);
    _th = fm.linespace;
    _ascent = fm.ascent;

    _w = tw + _insets.l + _insets.r + 4; //Resize(tw + 8, _th + 7);

    _cursor_x  = _vstart_x = _start_x = _end_x = 0;
    _cursor_ix = _start_ix = _end_ix = 0;
    _selection_on = _cursor_on = False;
    _curblink = _scrollselect = NULL;
    _blinkTime = 250;  // 500;
    _scrollDirection = TE_SCROLL_NONE;
    _dragging = False;
    _back = back;
    _select_on_focus = True;
    DefineCursor(GetResourcePool()->GetTextCursor());

    XGrabButton(GetDisplay(), AnyButton, AnyModifier, _id, False,
//		EnterWindowMask | LeaveWindowMask |
		ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
		GrabModeAsync, GrabModeAsync, None, None);

//    AddInput(KeyPressMask | FocusChangeMask);
    AddInput(FocusChangeMask);
}


OXTextEntry::~OXTextEntry() {
  delete _text;
  if (_curblink) delete _curblink;
  if (_scrollselect) delete _scrollselect;
  if (_normGC != _defaultGC) delete _normGC;
  if (_selGC != _defaultSelGC) delete _selGC;
  if (_selbackGC != _defaultSelBgndGC) delete _selbackGC;
  if (_font != _defaultFont) _client->FreeFont((OXFont *) _font);
}


void OXTextEntry::SetFont(const OXFont *f) {
  if (f) {
    OFontMetrics fm;

    const OXFont *oldfont = _font;
    _font = f;
    if (_normGC == _defaultGC) _normGC = new OXGC(_defaultGC);
    _normGC->SetFont(_font->GetId());  
    if (oldfont != _defaultFont) _client->FreeFont((OXFont *) oldfont);
    _font->GetFontMetrics(&fm);
    _th = fm.linespace;
    _ascent = fm.ascent;

    NeedRedraw();
  }
}


void OXTextEntry::SetTextColor(unsigned int color) {
  if (_normGC == _defaultGC) _normGC = new OXGC(_defaultGC);
  _normGC->SetForeground(color);
  NeedRedraw(False);
}


void OXTextEntry::DrawCursor(bool mode) {
  int y = 2 + _insets.t;
  int x = 2 + _insets.l;

  int cursor_height = _th - 1;
  if (cursor_height > _h - _insets.t - _insets.b)
    cursor_height = _h - _insets.t - _insets.b;

  int cursor_width = 2;

  if (mode) {
    FillRectangle(_normGC->GetGC(), _cursor_x - _vstart_x + x, y,
                  cursor_width, cursor_height);
  } else {
    ClearArea(_cursor_x - _vstart_x + x, y, cursor_width, cursor_height);
    if (_text->GetTextLength() > _cursor_ix) {
      OXGC *drawGC = _normGC;
      Font oldfont = _selGC->GetFont();
      drawGC->SetBackground(_backPixel);

      if (_selection_on &&
          _cursor_ix >= min(_start_ix, _end_ix) && 
          _cursor_ix < max(_start_ix, _end_ix)) {
        drawGC = _selGC;
        drawGC->SetFont(_normGC->GetFont());
        drawGC->SetBackground(_defaultSelectedBackground);
      }

      XRectangle rect;
      rect.x = _insets.l;
      rect.y = _insets.t;
      rect.width = _w - _insets.l - _insets.r;
      rect.height = _h - _insets.t - _insets.b;
      XSetClipRectangles(GetDisplay(), drawGC->GetGC(),
                         0, 0, &rect, 1, Unsorted);

      _DrawImageString(drawGC->GetGC(),
                       _cursor_x - _vstart_x + x, y + _ascent,
                       _cursor_ix, 1);

      drawGC->SetClipMask(None);

      if (drawGC == _selGC) _selGC->SetFont(oldfont);
    }
  }
}


void OXTextEntry::_DoRedraw() {
  int tw, ww, x, y;
  int start_char, end_char;

  OXFrame::_DoRedraw();

  x = 2 + _insets.l; // do not center text
  y = 2 + _insets.t;

  tw = _TextWidth(0, _text->GetTextLength());
  ww = _w - 4 - _insets.l - _insets.r;

  if (_vstart_x > tw - ww) _vstart_x = tw - ww;
  if (_vstart_x < 0) _vstart_x = 0;

  start_char = _GetCharacterIndex(_vstart_x);
  end_char = _GetCharacterIndex(_vstart_x + ww);

  x = _TextWidth(0, start_char) - _vstart_x;

  XRectangle rect;
  rect.x = _insets.l;
  rect.y = _insets.t;
  rect.width = _w - _insets.l - _insets.r;
  rect.height = _h - _insets.t - _insets.b;
  XSetClipRectangles(GetDisplay(), _normGC->GetGC(),
                     0, 0, &rect, 1, Unsorted);
  XSetClipRectangles(GetDisplay(), _selGC->GetGC(),
                     0, 0, &rect, 1, Unsorted);
  XSetClipRectangles(GetDisplay(), _selbackGC->GetGC(),
                     0, 0, &rect, 1, Unsorted);

  ClearArea(_insets.l, rect.y, 4 - _insets.l, rect.height);
  _normGC->SetBackground(_backPixel);
  _DrawImageString(_normGC->GetGC(), x + _insets.l + 2, y + _ascent,
                   start_char, end_char - start_char + 1);
  int xr = _TextWidth(0, _text->GetTextLength()) - _vstart_x;
  if (xr + _insets.l + 2 < _w - _insets.r)
    ClearArea(xr + _insets.l + 2, rect.y,
              _w - _insets.r - (xr + _insets.l + 2), rect.height);

  if (_selection_on) {
    int xs, ws, ixs, iws;
    
    xs = min(_start_x, _end_x);
    ixs = min(_start_ix, _end_ix);

    if ((_vstart_x > xs) && (start_char > ixs)) {
      xs  = _vstart_x;
      ws  = abs(_end_x - xs);
      ixs = start_char;
      iws = abs(_end_x - ixs);
    } else {
      ws  = abs(_end_x - _start_x);
      iws = abs(_end_ix - _start_ix);
    }
    FillRectangle(_selbackGC->GetGC(),
                  xs - _vstart_x + _insets.l + 2, y+1/*-1*/, ws, _th-1 /*+1*/);
    Font oldfont = _selGC->GetFont();
    _selGC->SetFont(_normGC->GetFont());
    _DrawString(_selGC->GetGC(), xs - _vstart_x + _insets.l + 2, y + _ascent,
                ixs, iws);
    _selGC->SetFont(oldfont);
  }
  _normGC->SetClipMask(None);
  _selGC->SetClipMask(None);
  _selbackGC->SetClipMask(None);
  DrawCursor(_cursor_on);
}


ODimension OXTextEntry::GetDefaultSize() const {
  return ODimension(_w /*tw + 4 + _insets.l + _insets.r*/,
                    _th + 3 + _insets.t + _insets.b);
}


ODimension OXTextEntry::GetTextSize() {
  return ODimension(_TextWidth(0, _text->GetTextLength()) + 4 +
                              _insets.l + _insets.r,
                    _th + 3 + _insets.t + _insets.b);
}


int OXTextEntry::_TextWidth(int begin, int length) {
  return _font->XTextWidth(_text->GetString() + begin, length);
}


void OXTextEntry::_DrawString(GC gc, int x, int y,
                              int begin, int length) {
  DrawString(gc, x, y, _text->GetString() + begin, length);
}


void OXTextEntry::_DrawImageString(GC gc, int x, int y,
                                   int begin, int length) {
  DrawImageString(gc, x, y, (char *) _text->GetString() + begin, length);
}


int OXTextEntry::_GetCharacterIndex(int xcoord) {
  int tw, ix, up, down, len;

  // check for out of boundaries first...
  len = _text->GetTextLength();
  tw = _TextWidth(0, len);
  if (xcoord < 0) return 0;
  if (xcoord > tw) return len; // len-1

  // do a binary approximation
  up = len; //-1
  down = 0;
  while (up-down > 1) {
    ix = (up+down) >> 1;
    tw = _TextWidth(0, ix);
    if (tw > xcoord)
      up = ix;
    else
      down = ix;
    if (tw == xcoord) break;
  }
  ix = down;

  // safety check...
  ix = max(ix, 0);
  ix = min(ix, len); // len-1
  return ix;
}


int OXTextEntry::HandleCrossing(XCrossingEvent *event) {
/*
  if (event->type == EnterNotify) {
    delete _scrollselect;
    _scrollselect = NULL;
    _scrollDirection = TE_SCROLL_NONE;
    RemoveInput(ButtonPressMask | ButtonReleaseMask);
  } else {
//    if ((event->y > 0) && (event->y < _h)) {
    AddInput(ButtonPressMask | ButtonReleaseMask);
    if (event->x < 0)
      _scrollDirection = TE_SCROLL_LEFT;
    else if (event->x >= _w)
      _scrollDirection = TE_SCROLL_RIGHT;
    if (_scrollselect) delete _scrollselect;
    _scrollselect = new OTimer(this, 50);
//    } else {
//      RemoveInput(ButtonPressMask | ButtonReleaseMask);
//    }
  }
*/
  return True;
}


int OXTextEntry::HandleButton(XButtonEvent *event) {
  int x;
  
  if (!IsEnabled()) return True;

  if (event->type == ButtonPress) {

    _select_on_focus = False;
    if (TakesFocus()) RequestFocus();

    if (event->button == Button1) {

      x = 2 + _insets.l;
      _start_ix = _GetCharacterIndex(event->x - x +_vstart_x);
      _start_x  = _TextWidth(0, _start_ix);
      SetCursor(_start_ix);
      _selection_on = False;
      _dragging = True;
      _DoRedraw();
   
    } else if (event->button == Button2) {

      // request selection
      Atom sel_property;

      if (XGetSelectionOwner(GetDisplay(), XA_PRIMARY) == None) {
        // No primary selection, so use the cut buffer
        _PastePrimary(_client->GetRoot()->GetId(), XA_CUT_BUFFER0, False);
      } else {
        sel_property = XInternAtom(GetDisplay(), "VT_SELECTION", False);
        XConvertSelection(GetDisplay(), XA_PRIMARY, XA_STRING,
                          sel_property, _id, event->time);
      }

    }

  } else {
    _dragging = False;
  }

  if (_scrollselect) {
    delete _scrollselect;
    _scrollselect = NULL;
    _scrollDirection = TE_SCROLL_NONE; 
  }

  return True;
}


int OXTextEntry::HandleDoubleClick(XButtonEvent *event) {
      
  if (!IsEnabled()) return True;

  _start_x = _start_ix = 0;
  _end_ix = _text->GetTextLength();
  _end_x  = _TextWidth(0, _end_ix);
  SetCursor(_end_ix);
  _selection_on = True;
//  _vstart_x = _TextWidth(0, _end_ix) - _w;
//  _vstart_x = _end_x - (_w - (_insets.l + _insets.r + 4));
  _DoRedraw();

  return True;
}


int OXTextEntry::HandleMotion(XMotionEvent *event) {
  int x, y;   
   
  if (!IsEnabled()) return True;
  if ((event->state & Button1Mask) == 0) return True;

  if (_dragging) {
    if (event->x < 0) {
      _scrollDirection = TE_SCROLL_LEFT;
      if (!_scrollselect) _scrollselect = new OTimer(this, 50);
      return True;
    } else if (event->x >= _w) {
      _scrollDirection = TE_SCROLL_RIGHT;
      if (!_scrollselect) _scrollselect = new OTimer(this, 50);
      return True;
    } else {
      _scrollDirection = TE_SCROLL_NONE;
      if (_scrollselect) delete _scrollselect;
      _scrollselect = NULL;
    }
  } else {
    _scrollDirection = TE_SCROLL_NONE;
    if (_scrollselect) delete _scrollselect;
    _scrollselect = NULL;
  }

/*
  if ((event->x < (2 + _insets.l)) ||
      (_scrollDirection != TE_SCROLL_NONE))
    return True;
*/

  x = 2 + _insets.l;
  y = 2 + _insets.t;
  _end_ix = _GetCharacterIndex(_vstart_x + event->x - x); // + 1;
  if (_end_ix < _GetCharacterIndex(_vstart_x))
    _end_ix = _GetCharacterIndex(_vstart_x);
  _end_x = _TextWidth(0, _end_ix);
  SetCursor(_end_ix);
  _selection_on = True;
  _DoRedraw();

  return True;
}


int OXTextEntry::HandleKey(XKeyEvent *event) {
  int  n, len, text_changed = False;
  char tmp[10];
  KeySym keysym;
  static XComposeStatus compose = { NULL, 0 };

  if (!IsEnabled()) return True;

  if (event->type == KeyRelease) return False;

  len = _text->GetTextLength();
  n = XLookupString(event, tmp, sizeof(tmp)-1, &keysym, &compose);
  tmp[n] = 0;

  switch (keysym) {
    case XK_Right:
      if (_selection_on) { _selection_on = False; _DoRedraw(); }
      SetCursor(_cursor_ix + 1);
      break;

    case XK_Left:  
      if (_selection_on) { _selection_on = False; _DoRedraw(); }
      SetCursor(_cursor_ix - 1);
      break;

    case XK_Home:
      if (_selection_on) { _selection_on = False; _DoRedraw(); }
      SetCursor(0);
      break;
    
    case XK_End:
      if (_selection_on) { _selection_on = False; _DoRedraw(); }
      SetCursor(len);
      break;

    case XK_Delete:
      if (_selection_on) {
        int start, ns;   
       
        start = min(_start_ix, _end_ix);
        ns = abs(_end_ix - _start_ix);
        _text->RemoveText(start, ns);
        _selection_on = False;
        SetCursor(start);
        _DoRedraw();
      } else {
        _text->RemoveText(_cursor_ix, 1);
      }
      text_changed = True;
      break;
    
    case XK_BackSpace:
      if (_selection_on) {
        int start, ns;
      
        start = min(_start_ix, _end_ix);
        ns = abs(_end_ix - _start_ix);
        _text->RemoveText(start, ns);
        _selection_on = False;
        SetCursor(start);
      } else {
        if (_cursor_ix > 0) {
          _text->RemoveText(_cursor_ix - 1, 1);
          SetCursor(_cursor_ix - 1);
        }
      }
      text_changed = True;
      break;

    case XK_Up:
    case XK_Down:
    case XK_Escape:
    //case XK_Execute:
    case XK_KP_Enter:
    case XK_Return:
    case XK_Tab:     // ==!== this is temp here!
      {
        OTextEntryMessage msg(MSG_TEXTENTRY, MSG_TEXTCHANGED, _widgetID,
                              keysym);
        SendMessage(_msgObject, &msg);
      }
      break;
      
    default:
      if ((n > 0) && (event->state & Mod1Mask)) *tmp |= 0x80;
      if (_selection_on && (strlen(tmp) > 0)) {
        int start, ns;
        
        start = min(_start_ix, _end_ix);
        ns = abs(_end_ix - _start_ix);
        _text->RemoveText(start, ns);
        _selection_on = False;
        _cursor_ix = start;  
      }
      _text->AddText(_cursor_ix, tmp);
      SetCursor(_cursor_ix + n);
      text_changed = True;
      break;
  }

  if (text_changed) {
    NeedRedraw();
    OTextEntryMessage msg(MSG_TEXTENTRY, MSG_TEXTCHANGED, _widgetID, keysym);
    SendMessage(_msgObject, &msg);
  }

  return True;
}


void OXTextEntry::SetCursor(int newpos) {
  int old_cursor = _cursor_x;
  int len = _text->GetTextLength();

  if (newpos < 0) newpos = 0;
  if (newpos > len) newpos = len;

  DrawCursor(False);
  _cursor_ix = newpos;
  _cursor_x = _TextWidth(0, _cursor_ix);
  if (_cursor_x > _vstart_x + (_w - (_insets.l + _insets.r + 4))) {
    _vstart_x += _cursor_x - old_cursor;
    _DoRedraw();
  } else if (_cursor_x < _vstart_x) {
    _vstart_x -= old_cursor - _cursor_x;
    _DoRedraw();
  }
  DrawCursor(True);
}   


void OXTextEntry::_GotFocus() {
  OXFrame::_GotFocus();
  AddInput(KeyPressMask);
  _cursor_on = True;
  if (_select_on_focus) SelectAll();
  _select_on_focus = True;
  DrawCursor(True);
  if (!_curblink) _curblink = new OTimer(this, _blinkTime);
  NeedRedraw(False);
}


void OXTextEntry::_LostFocus() {
  OXFrame::_LostFocus();
  RemoveInput(KeyPressMask);
  _cursor_on = False;
  _selection_on = False;
  if (_curblink) { delete _curblink; _curblink = NULL; }
  NeedRedraw(False);
}


void OXTextEntry::_Enable(int onoff) {
  if (onoff)
    SetBackgroundColor(_back);
  else
    SetBackgroundColor(_defaultFrameBackground);
  NeedRedraw();
}


int OXTextEntry::HandleSelection(XSelectionEvent *event) {
  _PastePrimary(event->requestor, event->property, True);
  return True;
}


void OXTextEntry::_PastePrimary(Window wid, Atom property, int Delete) {
  Atom actual_type;
  int  actual_format, len;
  unsigned long nitems, bytes_after, nread;
  unsigned char *data, *data2;

  if (!IsEnabled()) return;

  if (property == None) return;

  // paste primary
  nread = 0;
  do {
    if (XGetWindowProperty(GetDisplay(), wid, property,
                           nread/4, 1024, Delete,
                           AnyPropertyType,
                           &actual_type, &actual_format,
                           &nitems, &bytes_after,
                           (unsigned char **) &data)
        != Success)
      break;

    if (actual_type != XA_STRING) break;

    data2 = data;

    // remove any previous selected text
    if (_selection_on) {
      int start, ns;

      start = min(_start_ix, _end_ix);
      ns = abs(_end_ix - _start_ix);
      _text->RemoveText(start, ns);
      _selection_on = False;
      _cursor_ix = start;
    }

    _text->AddText(_cursor_ix, (char *) data2, (int) nitems);
    len = _text->GetTextLength();
    _cursor_ix += nitems;
    if (_cursor_ix > len) _cursor_ix = len;
                           
    nread += nitems;
    XFree(data2);  

  } while (bytes_after > 0);

  SetCursor(_cursor_ix);
  NeedRedraw();

  OTextEntryMessage msg(MSG_TEXTENTRY, MSG_TEXTCHANGED, _widgetID, 0);
  SendMessage(_msgObject, &msg);

  return;
}


int OXTextEntry::HandleTimer(OTimer *t) {

 if (t == _curblink) {
   _cursor_on = !_cursor_on;
   DrawCursor(_cursor_on);//_DoRedraw();
   delete _curblink;
   _curblink = new OTimer(this, _blinkTime);
   return True;

 } else if (t == _scrollselect) {
   switch (_scrollDirection) {
     case TE_SCROLL_LEFT:
       delete _scrollselect;
       _scrollselect = NULL;
       if (_cursor_ix == 0) {
         _scrollDirection = TE_SCROLL_NONE;
       } else {
         _end_ix--;
         _end_x = _TextWidth(0, _end_ix);
         SetCursor(_end_ix);
         _scrollselect = new OTimer(this, 50);
       }
       break;

     case TE_SCROLL_RIGHT:
       if (_cursor_ix == _text->GetTextLength()) {
         _scrollDirection = TE_SCROLL_NONE;
       } else {
         _end_ix++;
         _end_x = _TextWidth(0, _end_ix);
         SetCursor(_end_ix);
         _scrollselect = new OTimer(this, 50);
       }
       break;
    }
    return True;
  }

  return False;
}


void OXTextEntry::AddText(int pos, const char *text, int sendmsg) {
  _selection_on = False;

  _text->AddText(pos, text);

  SetCursor(_cursor_ix);
  NeedRedraw();

  if (sendmsg) {
    OTextEntryMessage msg(MSG_TEXTENTRY, MSG_TEXTCHANGED, _widgetID, 0);
    SendMessage(_msgObject, &msg);
  }
}


void OXTextEntry::AddText(int pos, const char *text, int length, int sendmsg) {
  _selection_on = False;

  _text->AddText(pos, text, length);

  SetCursor(_cursor_ix);
  NeedRedraw();

  if (sendmsg) {
    OTextEntryMessage msg(MSG_TEXTENTRY, MSG_TEXTCHANGED, _widgetID, 0);
    SendMessage(_msgObject, &msg);
  }
}


void OXTextEntry::RemoveText(int pos, int length, int sendmsg) {
  _selection_on = False;

  _text->RemoveText(pos, length);

  SetCursor(_cursor_ix);
  NeedRedraw();

  if (sendmsg) {
    OTextEntryMessage msg(MSG_TEXTENTRY, MSG_TEXTCHANGED, _widgetID, 0);
    SendMessage(_msgObject, &msg);
  }
}


void OXTextEntry::Clear(int sendmsg) {
  _selection_on = False;

  _text->Clear();

  SetCursor(0);
  NeedRedraw();

  if (sendmsg) {
    OTextEntryMessage msg(MSG_TEXTENTRY, MSG_TEXTCHANGED, _widgetID, 0);
    SendMessage(_msgObject, &msg);
  }
}

void OXTextEntry::SelectAll() {
  if (!IsEnabled()) return;
// no!  if (TakesFocus()) RequestFocus();
  _start_x = _start_ix = 0;
  _end_ix = _text->GetTextLength();
  _end_x  = _TextWidth(0, _end_ix);
  SetCursor(_end_ix);
  _selection_on = True;
//  _vstart_x = _TextWidth(0, _end_ix) - _w;
//  _vstart_x = _end_x - (_w - (_insets.l + _insets.r + 4)); 
  _DoRedraw();

  return;
}
