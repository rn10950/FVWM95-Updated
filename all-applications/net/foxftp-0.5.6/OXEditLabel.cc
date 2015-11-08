/**************************************************************************

    This file is part of foxftp, a ftp client for fOX.
    Copyright (C) 1998, 1999, G. Matzka.

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

#include <X11/cursorfont.h>
#include <X11/keysym.h>

#include <xclass/OXTextEntry.h>
#include <xclass/ODimension.h>
#include <xclass/OString.h>

#include "OXEditLabel.h"

//-----------------------------------------------------------------

Cursor OXEditLabel::_defaultCursor = None;
GC OXEditLabel::_defaultGC = None;
XFontStruct *OXEditLabel::_defaultFontStruct = NULL;

OXEditLabel::OXEditLabel(const OXWindow *p, OString *text, int ID,
                 GC norm, XFontStruct *font,
                 unsigned int options, unsigned long back) :
    OXFrame(p, 1, 1, options, back) {

    if (_defaultCursor == None) {
      _defaultCursor = XCreateFontCursor(GetDisplay(), XC_xterm);
    }
    if (_defaultFontStruct == NULL) {
      _defaultFontStruct = XLoadQueryFont(GetDisplay(),
             "-adobe-helvetica-medium-r-*-*-10-*-*-*-*-*-iso8859-1");
    }
    if (_defaultGC == None) {
      XGCValues gval;
      unsigned long gmask = GCForeground | GCBackground | GCFont;
      gval.font = _defaultFontStruct->fid;
      gval.background = _defaultFrameBackground;
      gval.foreground = _blackPixel;
      _defaultGC = XCreateGC(GetDisplay(), _id, gmask, &gval);
    }

    _fontStruct = font;
    if (_fontStruct == NULL)
      _fontStruct = _defaultFontStruct;

    _normGC = norm;
    if (_normGC == None)
      _normGC = _defaultGC;

    _msgObject = NULL;
    _ID = ID;
    _text = new OTextBuffer(256);
    _text->AddText(0, text->GetString());
    _alignment = TEXT_LEFT | TEXT_TOP;
    _ml = _mr = _mt = _mb = 2;

    _edit = false;
    _selection_on = false;
    _dragging = false;
    _sel_start = _sel_end = -1;
    _cursor_pos = -1;
    _width_min = -1;

    _FontSizeUp();
}

OXEditLabel::~OXEditLabel() {
  if (_text) delete _text;
}

void OXEditLabel::SetText(char *new_text) {
  _text->Clear();
  _text->AddText(0, new_text);
  _width_min = -1;
  _FontSizeUp();
  _client->NeedRedraw(this);
}

void OXEditLabel::_FontSizeUp() {
  _tw = XTextWidth(_fontStruct, _text->GetString(), _text->GetTextLength());
  _th = _fontStruct->max_bounds.ascent + _fontStruct->max_bounds.descent;
  if (_width_min == -1)
    _width_min = GetDefaultWidth();
  if (_width_min > _tw)
    Resize(_width_min, GetDefaultHeight());
  else
    Resize(GetDefaultSize());
}

void OXEditLabel::_DoRedraw() {
  int x, y;

  OXFrame::_DoRedraw();
    
  if (_alignment & TEXT_LEFT)
    x = _ml;
  else if (_alignment & TEXT_RIGHT)
    x = _w - _tw - _mr;
  else
    x = (_w - _tw + _ml - _mr) >> 1;

  if (_alignment & TEXT_TOP)
    y = _mt;
  else if (_alignment & TEXT_BOTTOM)
    y = _h - _th - _mb;
  else
    y = (_h - _th + _mt - _mb) >> 1;

  XSetForeground(GetDisplay(), _normGC, _blackPixel);
  XDrawString(GetDisplay(), _id, _normGC, x, y + _fontStruct->max_bounds.ascent,
                _text->GetString(), _text->GetTextLength());
  if (_selection_on) {
    int s, e, start, end, ns;
    start = min(_sel_start, _sel_end);
    end = max(_sel_start, _sel_end);
    ns = abs(_sel_end - _sel_start);
    s = XTextWidth(_fontStruct, _text->GetString(), start);
    e = XTextWidth(_fontStruct, _text->GetString(), end);
    XSetForeground(GetDisplay(), _normGC, _defaultSelectedBackground);
    XFillRectangle(GetDisplay(), _id, _normGC, s + 1, y, e - s, _th + 1);
    XSetForeground(GetDisplay(), _normGC, _whitePixel);
    XDrawString(GetDisplay(), _id, _normGC, x + s, y + _fontStruct->max_bounds.ascent,
                _text->GetString() + start, ns);
  } else if (_cursor_pos >= 0) {
    int pos = XTextWidth(_fontStruct, _text->GetString(), _cursor_pos) + 1;
    XDrawLine(GetDisplay(), _id, _blackGC, pos, y, pos, _th);
  }
  if (_edit)
    XDrawRectangle(GetDisplay(), _id, _blackGC, 0, 0, _tw + 3, _th + 3);
}

void OXEditLabel::Edit(bool select) {
  _edit = true;
  XDefineCursor(GetDisplay(), _id, _defaultCursor);
  XGrabPointer(GetDisplay(), _id, True,
		EnterWindowMask | LeaveWindowMask |
		ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
		GrabModeAsync, GrabModeAsync, False, None, None);
  AddInput(KeyPressMask | FocusChangeMask);
  XSetInputFocus(GetDisplay(), _id, RevertToParent, CurrentTime);
  if (select) {
    _selection_on = true;
    _dragging = false;
    _sel_start = 0;
    _sel_end = _text->GetTextLength();
    _cursor_pos = _sel_end;
  } else {
    _selection_on = false;
    _dragging = false;
    _sel_start = _sel_end = -1;
    _cursor_pos = _text->GetTextLength();
  }
  _client->NeedRedraw(this);
}

void OXEditLabel::UnEdit() {
  XSetInputFocus(GetDisplay(), None, RevertToParent, CurrentTime);
  XUngrabPointer(GetDisplay(), CurrentTime);
  _selection_on = false;
  _dragging = false;
}

int OXEditLabel::HandleKey(XKeyEvent *event) {
  int  n, len, text_changed = false;
  char tmp[10];
  KeySym keysym;
  XComposeStatus compose = { NULL, 0 };

  len = _text->GetTextLength();
  n = XLookupString(event, tmp, sizeof(tmp)-1, &keysym, &compose);
  tmp[n] = 0;

  switch (keysym) {
    case XK_Right:
      if (_selection_on) { _selection_on = false; _DoRedraw(); }
      SetCursor(_cursor_pos + 1);
      break;

    case XK_Left:  
      if (_selection_on) { _selection_on = false; _DoRedraw(); }
      SetCursor(_cursor_pos - 1);
      break;

    case XK_Home:
      if (_selection_on) { _selection_on = false; _DoRedraw(); }
      SetCursor(0);
      break;
    
    case XK_End:
      if (_selection_on) { _selection_on = false; _DoRedraw(); }
      SetCursor(len);
      break;

    case XK_Delete:
      if (_selection_on) {
        int start, ns;   
        start = min(_sel_start, _sel_end);
        ns = abs(_sel_end - _sel_start);
        _text->RemoveText(start, ns);
        _selection_on = false;
        SetCursor(start);
      } else {
        _text->RemoveText(_cursor_pos, 1);
      }
      text_changed = true;
      break;
    
    case XK_BackSpace:
      if (_selection_on) {
        int start, ns;
        start = min(_sel_start, _sel_end);
        ns = abs(_sel_end - _sel_start);
        _text->RemoveText(start, ns);
        _selection_on = false;
        SetCursor(start);
      } else {
        if (_cursor_pos > 0) {
          _text->RemoveText(_cursor_pos - 1, 1);
          SetCursor(_cursor_pos - 1);
        }
      }
      text_changed = true;
      break;

    case XK_Up:
    case XK_Down:
    case XK_Escape:
    case XK_Execute:
    case XK_Return:
    case XK_Tab:
      {
        OTextEntryMessage msg(MSG_TEXTENTRY, MSG_TEXTCHANGED, _ID, keysym);
        SendMessage(_msgObject, &msg);
      }
      break;
      
    default:
      if (_selection_on) {
        int start, ns;
        start = min(_sel_start, _sel_end);
        ns = abs(_sel_end - _sel_start);
        _text->RemoveText(start, ns);
        _selection_on = false;
        _cursor_pos = start;  
      }
      _text->AddText(_cursor_pos, tmp);
      SetCursor(_cursor_pos + n);
      text_changed = true;
      break;
  }
  if (text_changed) {
    _client->NeedRedraw(this);
    _FontSizeUp();
    OTextEntryMessage msg(MSG_TEXTENTRY, MSG_TEXTCHANGED, _ID, keysym);
    SendMessage(_msgObject, &msg);
  }
  return true;
}

int OXEditLabel::HandleButton(XButtonEvent *event) {
  if (event->type == ButtonPress) {
    if (event->x >= 0 && event->x < _w &&
       event->y >= 0 && event->y < _h) {
      XSetInputFocus(GetDisplay(), _id, RevertToParent, CurrentTime);
      int x = 0, i = 0;
      do {
        i++;
        x = XTextWidth(_fontStruct, _text->GetString(), i);
      } while (x < event->x);
      SetCursor(i - 1);
      _sel_start = _sel_end = _cursor_pos;
      _selection_on = false; 
      _dragging = true;
      _DoRedraw();
    } else {
      OTextEntryMessage msg(MSG_TEXTENTRY, MSG_TEXTCHANGED, _ID, XK_Escape);
      SendMessage(_msgObject, &msg);
    }
  } else {
    _dragging = false;
  }
  return true;
}

int OXEditLabel::HandleDoubleClick(XButtonEvent *event) {
  _selection_on = true;
  _sel_start = 0;
  _sel_end = _text->GetTextLength();
  _client->NeedRedraw(this);
  return true;
}

int OXEditLabel::HandleMotion(XMotionEvent *event) {
  if (_dragging) {
    int x = 0, i = 0;
    do {
      i++;
      x = XTextWidth(_fontStruct, _text->GetString(), i);
    } while (x < event->x);
    SetCursor(i - 1);
    _sel_end = i - 1;
    if (_sel_end != _sel_start)
      _selection_on = true;
    _DoRedraw();
  }
  return true;
}

void OXEditLabel::SetCursor(int newpos) {
  int len = _text->GetTextLength();

  if (newpos < 0) newpos = 0;
  if (newpos > len) newpos = len;
  _client->NeedRedraw(this);
  _cursor_pos = newpos;
}   
