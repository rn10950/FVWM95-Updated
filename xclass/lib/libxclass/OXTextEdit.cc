/**************************************************************************

    This is file is part of xclass.
    Copyright (C) 1998-2001, Harald Radke, Hector Peraza.

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

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <X11/keysym.h>

#include <xclass/utils.h>
#include <xclass/OXClient.h>
#include <xclass/OXTextEdit.h>


const OXFont *OXTextEdit::_defaultFont = NULL;
Atom  OXTextEdit::_XC_SELECTION_DATA = None;

#define TAB         '\t'
#define TAB_WIDTH   8

// TODO:
// - enable/disable
// - shift-click select
// - optimize/rewrite drawing/layout/selection code
// - add AutoUpdate() option
// - INCR-type selection

//----------------------------------------------------------------------

OXTextEdit::OXTextEdit(const OXWindow *p, int w, int h, int id,
                       unsigned int options, unsigned int sboptions) :
  OXItemView(p, w, h, id, options, sboptions) {

  if (!_defaultFont) {
    _defaultFont = _client->GetResourcePool()->GetDocumentFixedFont();
  }

  if (_XC_SELECTION_DATA == None) {
    _XC_SELECTION_DATA = XInternAtom(GetDisplay(), "_XC_SELECTION_DATA", False);
  }

  _flags |= HANDLES_TAB;

  _font = _defaultFont;

  _vsb->SetDelay(10, 10);
  _vsb->SetMode(SB_ACCELERATED);
  _hsb->SetDelay(10, 10);
  _hsb->SetMode(SB_ACCELERATED);

  XSetWindowAttributes wattr;
  unsigned long mask;

  mask = CWBitGravity | CWWinGravity;
  wattr.bit_gravity = NorthWestGravity;
  wattr.win_gravity = NorthWestGravity;

  XChangeWindowAttributes(GetDisplay(), _canvas->GetId(), mask, &wattr);

  XGCValues gcv;
  unsigned long gcm;

  gcm = GCFont | GCForeground | GCBackground | GCGraphicsExposures |
        GCFillStyle;
  gcv.font = _font->GetId();
  gcv.foreground = _client->GetResourcePool()->GetSelectedFgndColor();
  gcv.background = _client->GetResourcePool()->GetSelectedBgndColor();
  gcv.graphics_exposures = False;
  gcv.fill_style = FillSolid;

  // GC for selected text (white on blue)
  _selGC = new OXGC(GetDisplay(), _id, gcm, &gcv);

  gcv.foreground = _client->GetResourcePool()->GetSelectedBgndColor();
  gcv.background = _client->GetResourcePool()->GetSelectedFgndColor();

  // this one is for the blue rectangle behind selected text
  _hilightGC = new OXGC(GetDisplay(), _id, gcm, &gcv);

  gcv.background = GetResourcePool()->GetDocumentBgndColor();
  gcv.foreground = GetResourcePool()->GetDocumentFgndColor();

  // standard GC black on white
  _normGC = new OXGC(GetDisplay(), _id, gcm, &gcv);

  _canvas->DefineCursor(GetResourcePool()->GetTextCursor());

  _clearExposedArea = True;

  _cursorBlink = NULL;
  _cursorState = 0;
  _cursorPos = _textIndex = OPosition(0, 0);
  _cursorLastX = 0;

  _autoUpdate = True;

  _tabWidth = TAB_WIDTH;
  _blinkTime = 250;

  _msgType = MSG_TEXTEDIT;
  _widgetType = "OXTextEdit";

  _margin = ODimension(3, 3);

  Clear();
  _changed = False;
  _readOnly = False;

  _clipboardText = NULL;
}

OXTextEdit::~OXTextEdit() {
  if (_cursorBlink) delete _cursorBlink;
  if (_font != _defaultFont) _client->FreeFont((OXFont *) _font);
  if (_clipboardText) delete _clipboardText;
}

//----------------------------------------------------------------------

void OXTextEdit::SetFont(const OXFont *f) {
  if (f) {
    _font = (OXFont *) f;
    _normGC->SetFont(_font->GetId());
    _hilightGC->SetFont(_font->GetId());
    _selGC->SetFont(_font->GetId());
    CalcMaxItemSize();
    Layout();
    NeedRedraw(ORectangle(_visibleStart, _canvas->GetSize()));
  }
}

void OXTextEdit::SetTextColor(unsigned int color) {
  _normGC->SetForeground(color);
  NeedRedraw(ORectangle(_visibleStart, _canvas->GetSize()));
}

void OXTextEdit::SetTabWidth(unsigned int tabWidth) {
  _tabWidth = tabWidth;
  CalcMaxItemSize();
  Layout();
  NeedRedraw(ORectangle(_visibleStart, _canvas->GetSize()));
}

// Sets the OXTextEdit widget to read-only mode.
//
// The default read-write mode allows normal text edition using the
// the alphanumeric, arrow keys, backspace and delete.
//
// The read-only mode is a view-only mode. The blinking cursor is not
// displayed and text cannot be typed in or modified. The arrow keys work in
// somewhat different way, scrolling the text window around. Text can be
// selected for clipboard copy operations, but only by using the mouse.

void OXTextEdit::SetReadOnly(bool ro) {
  _readOnly = ro;
  if (_readOnly) {
    _flags &= ~HANDLES_TAB;
    DrawCursor(_cursorState = 0);
    if (_cursorBlink) delete _cursorBlink;
    _cursorBlink = NULL;
  } else {
    _flags |= HANDLES_TAB;
    if (HasFocus()) {
      DrawCursor(_cursorState = 1);
      if (_cursorBlink) delete _cursorBlink;
      _cursorBlink = new OTimer(this, _blinkTime);
    }
  }
}

// Empty the contents of the OXTextEdit widget.

void OXTextEdit::Clear(bool null_line) {
  DrawCursor(0);
  OXItemView::Clear();
  _maxItemSize = ODimension(0, 0);
  if (null_line) {
    AddLine("");
    Layout();
  }
  _cursorPos = _textIndex = OPosition(0, 0);
  _cursorLastX = 0;
  DrawCursor(_cursorState);
  _changed = True;
}

//----------------------------------------------------------------------

int OXTextEdit::HandleTimer(OTimer *t) {
  if (t == _cursorBlink) {
    if (_cursorState) {
      _cursorState = 0;
    } else {
      _cursorState = 1;
    }
    DrawCursor(_cursorState);
    delete _cursorBlink;
    _cursorBlink = new OTimer(this, _blinkTime);
    return True;
  } else {
    return OXItemView::HandleTimer(t);
  }
}

// unfortunately the mouse handling is different to the default one,
// so we have to override those methods...

int OXTextEdit::HandleButton(XButtonEvent *event) {
  int n;
  OTextLine *eventItem = NULL;

  if (OXView::HandleButton(event)) return True;  // wheel-mouse scroll

  OPosition eventPos = ToVirtual(OPosition(event->x, event->y));

  eventPos = AdjustCoord(eventPos);
  eventItem = (OTextLine *) GetItemByPos(eventPos, &n);

  if (event->type == ButtonPress) {

    if (TakesFocus()) RequestFocus();

    if (event->button == Button1) {
      if (_items.size() == 0) return True;

      int x = eventItem->GetCharIndexByCoord(_margin.w, eventPos.x);
      SetCursorPosition(OPosition(x, n), False);
      _cursorLastX = _cursorPos.x;

      _selAnchor = eventPos;
      if (!(event->state & ShiftMask) && (_hasSelection)) UnSelectAll();

      // adjust mouse position and find corresponding text line

      _dragStartLine = eventItem;
      _dragStartLine->SelectChars(_textIndex.x, _textIndex.x);
      _selAnchor.y = _dragStartLine->GetPosition().y;

      _selDragStart = _selAnchor;
      _selDragEnd = _selDragStart;

      OItemViewMessage message(_msgType, MSG_SELCHANGED, _id);
      SendMessage(_msgObject, &message);

      _lastDragPos = _selAnchor;
      if (_selectedItems.size() == 0) _dragSelecting = True;

    } else if (event->button == Button2) {

      if (!_readOnly) {
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

    }

  } else {

    if (_isScrolling) _isScrolling = False;
    if (_scrollTimer) delete _scrollTimer;
    _scrollTimer = NULL;

    if (_dragSelecting) {
      _dragSelecting = False;
    } else {
      OItemViewMessage message(_msgType, MSG_CLICK, _id, event->button,
                               OPosition(event->x_root, event->y_root));
      SendMessage(_msgObject, &message);
    }

  }
  return True;
}

// double click: select the word under cursor or the white space
// between words

int OXTextEdit::HandleDoubleClick(XButtonEvent *event) {
  int n;
  OTextLine *eventItem = NULL;

  if (event->button != Button1) return False;

  OPosition eventPos = ToVirtual(OPosition(event->x, event->y));

  eventPos = AdjustCoord(eventPos);
  eventItem = (OTextLine *) GetItemByPos(eventPos, &n);

  if (event->type == ButtonPress) {

    if (event->button == Button1) {
      if (_items.size() == 0) return True;

      int ix = eventItem->GetCharIndexByCoord(_margin.w, eventPos.x);
      const char *str = eventItem->GetString();

      int start = ix, end = ix;
      int len = eventItem->GetTextLength();

      if (str[ix] == ' ' || str[ix] == TAB) {
        while (start >= 0) {
          if (str[start] == ' ' || str[start] == TAB) --start; else break;
        }
        ++start;
        while (end < len) {
          if (str[end] == ' ' || str[end] == TAB) ++end; else break;
        }
      } else if (isalnum(str[ix])) {
        while (start >= 0) {
          if (isalnum(str[start])) --start; else break;
        }
        ++start;
        while (end < len) {
          if (isalnum(str[end])) ++end; else break;
        }
      } else {
        while (start >= 0) {
          if (isalnum(str[start]) || str[start] == ' ' || str[start] == TAB)
            break;
          else
            --start;
        }
        ++start;
        while (end < len) {
          if (isalnum(str[end]) ||str[end] == ' ' || str[end] == TAB)
            break;
          else
            ++end;
        }
      }

      eventItem->SelectChars(start, end);
      OXItemView::SelectItem(eventItem, True);

      SetCursorPosition(OPosition(end, n), False);
      _cursorLastX = _cursorPos.x;

      OItemViewMessage message(_msgType, MSG_SELCHANGED, _id);
      SendMessage(_msgObject, &message);

    }

  } else {

    OItemViewMessage message(_msgType, MSG_DBLCLICK, _id, event->button,
                             OPosition(event->x_root, event->y_root));
    SendMessage(_msgObject, &message);

  }

  return True;
}

// triple click: select the whole line under cursor

int OXTextEdit::HandleTripleClick(XButtonEvent *event) {
  int n;
  OTextLine *eventItem = NULL;

  if (event->button != Button1) return False;

  OPosition eventPos = ToVirtual(OPosition(event->x, event->y));

  eventPos = AdjustCoord(eventPos);
  eventItem = (OTextLine *) GetItemByPos(eventPos, &n);

  if (event->type == ButtonPress) {

    if (event->button == Button1) {
      if (_items.size() == 0) return True;

      eventItem->SelectChars(0, eventItem->GetTextLength());
      OXItemView::SelectItem(eventItem, True);

      int x = eventItem->GetTextLength();
      SetCursorPosition(OPosition(x, n), False);
      _cursorLastX = _cursorPos.x;

      OItemViewMessage message(_msgType, MSG_SELCHANGED, _id);
      SendMessage(_msgObject, &message);

    }
  }

  return True;
}


int OXTextEdit::HandleMotion(XMotionEvent *event) {
  OXItemView::HandleMotion(event);
  if (_dragSelecting) {
    int x, n;
    OPosition eventPos = ToVirtual(OPosition(event->x, event->y));

    eventPos = AdjustCoord(eventPos);
    OTextLine *line = (OTextLine *) GetItemByPos(eventPos, &n);

    x = line->GetCharIndexByCoord(_margin.w, eventPos.x);
    SetCursorPosition(OPosition(x, n), False);
    _cursorLastX = _cursorPos.x;
  }
  return True;
}

// Current key bindings:
//   Left, Right, Up, Down, PageUp, PageDown, Home, End
//   Ctrl-Home, Ctrl-End, Ctrl-X, Ctrl-C, Ctrl-V,
//   Ctrl-Ins, Shift-Ins, Backspace, Delete, Shift-Delete

int OXTextEdit::HandleKey(XKeyEvent *event) {
  char input[4] = { 0, 0, 0, 0 };
  int  charcount, update = _autoUpdate;
  KeySym keysym;
  static XComposeStatus compose = { NULL, 0 };
  OTextLine *line;

  if (_items.size() == 0) AddLine("");  //return True;

  line = (OTextLine *) _items[_textIndex.y];

  if (event->type == KeyPress) {
    charcount = XLookupString(event, input, sizeof(input)-1, &keysym, &compose);

    if (event->state & ControlMask) {
      switch (keysym) {
        case XK_X: case XK_x: if (!_readOnly) Cut(); return True;
        case XK_C: case XK_c: Copy(); return True;
        case XK_V: case XK_v: if (!_readOnly) Paste(); return True;
      }
    }

    _autoUpdate = True;

    switch (keysym) {
      case XK_Left:
        if (_readOnly) {
          ScrollToPosition(OPosition(_visibleStart.x - 20, _visibleStart.y));
        } else {
          if (event->state & ShiftMask) {
            if (!_hasSelection) SelectFrom(_textIndex);
            CursorLeft();
            SelectTo(_textIndex);
          } else {
            UnSelectAll();
            CursorLeft();
          }
        }
        break;

      case XK_Right:
        if (_readOnly) {
          ScrollToPosition(OPosition(_visibleStart.x + 20, _visibleStart.y));
        } else {
          if (event->state & ShiftMask) {
            if (!_hasSelection) SelectFrom(_textIndex);
            CursorRight();
            SelectTo(_textIndex);
          } else {
            UnSelectAll();
            CursorRight();
          }
        }
        break;

      case XK_Up:
        if (_readOnly) {
          ScrollToPosition(OPosition(_visibleStart.x,
                                     _visibleStart.y - line->GetSize().h));
        } else {
          if (event->state & ShiftMask) {
            if (!_hasSelection) SelectFrom(_textIndex);
            CursorUp();
            SelectTo(_textIndex);
          } else {
            UnSelectAll();
            CursorUp();
          }
        }
        break;

      case XK_Down:
        if (_readOnly) {
          ScrollToPosition(OPosition(_visibleStart.x,
                                     _visibleStart.y + line->GetSize().h));
        } else {
          if (event->state & ShiftMask) {
            if (!_hasSelection) SelectFrom(_textIndex);
            CursorDown();
            SelectTo(_textIndex);
          } else {
            UnSelectAll();
            CursorDown();
          }
        }
        break;

      case XK_Page_Up:
        if (_readOnly) {
          ScrollToPosition(OPosition(_visibleStart.x,
                                     _visibleStart.y - _canvas->GetSize().h +
                                     line->GetSize().h));
        } else {
          if (event->state & ShiftMask) {
            if (!_hasSelection) SelectFrom(_textIndex);
            CursorPageUp();
            SelectTo(_textIndex);
          } else {
            UnSelectAll();
            CursorPageUp();
          }
        }
        break;

      case XK_Page_Down:
        if (_readOnly) {
          ScrollToPosition(OPosition(_visibleStart.x,
                                     _visibleStart.y + _canvas->GetSize().h -
                                     line->GetSize().h));
        } else {
          if (event->state & ShiftMask) {
            if (!_hasSelection) SelectFrom(_textIndex);
            CursorPageDown();
            SelectTo(_textIndex);
          } else {
            UnSelectAll();
            CursorPageDown();
          }
        }
        break;

      case XK_Home:
        if (_readOnly) {
          ScrollToPosition(OPosition(0 /*_visibleStart.x*/, 0));
        } else {
          if (event->state & ShiftMask) {
            if (!_hasSelection) SelectFrom(_textIndex);
            event->state & ControlMask ? CursorTop() : CursorHome();
            SelectTo(_textIndex);
          } else {
            UnSelectAll();
            event->state & ControlMask ? CursorTop() : CursorHome();
          }
        }
        break;

      case XK_End:
        if (_readOnly) {
          // no need to be very accurate about this, since OXItemView
          // does a good job of correcting the limit coordinates...
          int end = (_items.size() > 0) ?
                    _items[_items.size()-1]->GetPosition().y : 0;
          ScrollToPosition(OPosition(_visibleStart.x, end));
        } else {
          if (event->state & ShiftMask) {
            if (!_hasSelection) SelectFrom(_textIndex);
            event->state & ControlMask ? CursorBottom() : CursorEnd();
            SelectTo(_textIndex);
          } else {
            UnSelectAll();
            event->state & ControlMask ? CursorBottom() : CursorEnd();
          }
        }
        break;

      default:
        if ((charcount == 1) && (event->state & Mod1Mask)) *input |= 0x80;
      case XK_Tab:
        if (_readOnly || charcount == 0) break;
        if (_hasSelection) {
          DeleteSelection();
          line = (OTextLine *) _items[_textIndex.y];
        }
        line->AddText(_textIndex.x, input);
        line->Resize(line->GetDefaultSize());
        _changed = True;
        CursorRight();
        _CheckLayout(line);
        break;

      case XK_BackSpace:
        if (_readOnly) break;
        if (!_hasSelection) {
          if (_textIndex.x == 0 && _textIndex.y == 0) break;
          CursorLeft();
        }
        /* fall thru */
      case XK_Delete:
        if (_readOnly) break;
        if (_hasSelection) {
          if (event->state & ShiftMask)
            Cut();
          else
            DeleteSelection();
        } else {
          DeleteChar(_textIndex.y, _textIndex.x);
        }
        _UpdateCursor(_textIndex);
        _cursorLastX = _cursorPos.x;
        break;

      case XK_Return:
      //case XK_Execute:
      case XK_KP_Enter:
        if (_readOnly) break;
        if (_hasSelection) {
          DeleteSelection();
        }
        SplitLine(_textIndex.y, _textIndex.x);
        CursorRight();
        break;

      case XK_Insert:
        if (event->state & ControlMask) {
          Copy();
        } else if (event->state & ShiftMask) {
          if (!_readOnly) Paste();
        }
        break;
    }

  }

  _autoUpdate = update;

  return True;
}

int OXTextEdit::HandleSelection(XSelectionEvent *event) {
  _PastePrimary(event->requestor, event->property, True);
  return True;
}

int OXTextEdit::HandleSelectionRequest(XSelectionRequestEvent *event) {

  if (_clipboardText && event->target == XA_STRING) {

    long len = _clipboardText->GetLength();
    const char *data = _clipboardText->GetString();

    if (event->property == None)
      event->property = event->target;  // obsolete client

    XChangeProperty(GetDisplay(), event->requestor, event->property,
                    event->target, 8, PropModeReplace,
                    (unsigned char *) data, len);

  } else {
    event->property = None;
  }

  XEvent reply;

  reply.xselection.type      = SelectionNotify;
  reply.xselection.property  = event->property;
  reply.xselection.display   = event->display;
  reply.xselection.requestor = event->requestor;
  reply.xselection.selection = event->selection;
  reply.xselection.target    = event->target;
  reply.xselection.time      = event->time;

  XSendEvent(GetDisplay(), event->requestor, True, NoEventMask, &reply);

  return True;
}


//----------------------------------------------------------------------

int OXTextEdit::Cut() {
  if (_hasSelection) {
    if (_clipboardText) delete _clipboardText;
    _clipboardText = GetSelectedText();
    DeleteSelection();
    XSetSelectionOwner(GetDisplay(), XA_PRIMARY, _id, CurrentTime);
    return True;
  }
  return False;
}

int OXTextEdit::Copy() {
  if (_hasSelection) {
    if (_clipboardText) delete _clipboardText;
    _clipboardText = GetSelectedText();
    XSetSelectionOwner(GetDisplay(), XA_PRIMARY, _id, CurrentTime);
    return True;
  }
  return False;
}

void OXTextEdit::Paste() {
  XChangeProperty(GetDisplay(), _id, _XC_SELECTION_DATA, XA_STRING, 8,
                  PropModeReplace, (unsigned char *) 0, 0);

  XConvertSelection(GetDisplay(), XA_PRIMARY, XA_STRING, _XC_SELECTION_DATA,
                    _id, CurrentTime);
}

void OXTextEdit::_PastePrimary(Window wid, Atom property, int Delete) {
  Atom actual_type;
  int  i, actual_format, len;
  unsigned long nitems, bytes_after, nread;
  unsigned char *data, *data2;

  if (!IsEnabled() || _readOnly) return;

  if (property == None) return;

  // paste primary
  nread = 0;
  do {
    if (XGetWindowProperty(GetDisplay(), wid, property, nread/4, 1024,
                           Delete, AnyPropertyType, &actual_type,
                           &actual_format, &nitems, &bytes_after,
                           (unsigned char **) &data) != Success)
      break;

    if (actual_type != XA_STRING) break;

    data2 = data;

    // remove any previous selected text
    if (_hasSelection) DeleteSelection();

    InsertText((char *) data2, (int) nitems);
    _changed = True;

    nread += nitems;
    XFree(data2);  

  } while (bytes_after > 0);

  _UpdateCursor(_textIndex);
  Layout();

  //OTextEditMessage msg(_msgType, MSG_TEXTCHANGED, _widgetID, 0);
  //SendMessage(_msgObject, &msg);

  return;
}


//----------------------------------------------------------------------

void OXTextEdit::DrawCursor(int state) {
  if (_items.size() == 0) return;

  OTextLine *line = (OTextLine *) _items[_cursorPos.y];
  OPosition cpos = OPosition(line->GetTextWidth(0, _cursorPos.x) + _margin.w,
                             line->GetPosition().y);
  OPosition pos = ToPhysical(cpos);
  int curw = 2;
  int curh = line->GetTextHeight();

  // check first if cursor is in the visible area

  if ((pos.x < -curw) || (pos.x > _canvas->GetWidth()) ||
      (pos.y < -curh) || (pos.y > _canvas->GetHeight())) return;

  if (state) {
    _canvas->FillRectangle(_normGC->GetGC(), pos.x, pos.y, curw, curh);
  } else {
    DrawRegion(cpos, ODimension(curw, curh));
  }
}

void OXTextEdit::_GotFocus() {
  OXItemView::_GotFocus();
  if (!_readOnly) {
    DrawCursor(_cursorState = 1);
    if (_cursorBlink) delete _cursorBlink;
    _cursorBlink = new OTimer(this, _blinkTime);
  }
}

void OXTextEdit::_LostFocus() {
  OXItemView::_LostFocus();
  DrawCursor(_cursorState = 0);
  if (_cursorBlink) delete _cursorBlink;
  _cursorBlink = NULL;
}


// Read a file into the text editor widget. Any existing text is deleted.
// Return True on success.

int OXTextEdit::LoadFromFile(char *filename) {
  FILE *fp;
  char buf[8000];

  if ((fp = fopen(filename, "r")) == NULL) return False;

  _needItemLayout = False;

  Clear(False);
  while (fgets(buf, 8000, fp)) {
    if (buf[strlen(buf)-1] == '\n') buf[strlen(buf)-1] = '\0';
    if (buf[strlen(buf)-1] == '\r') buf[strlen(buf)-1] = '\0';
    AddLine(buf);
  }
  AddLine("");
  fclose(fp);

  CalcMaxItemSize();
  _needItemLayout = True;
  Layout();
  NeedRedraw(ORectangle(_visibleStart, _canvas->GetSize()));

  _changed = False;

  return True;
}

// Save the current contents of the editor into a file.
// Return True on success.

int OXTextEdit::SaveToFile(char *filename) {
  FILE *fp;
  OTextLine *ln;
  char buf[8000];

  if ((fp = fopen(filename, "w")) == NULL) return False;

  for (int i = 0; i < _items.size() - 1; ++i) {
    ln = (OTextLine *) _items[i];
    fprintf(fp, "%s\n", ln->GetString());
  }
  ln = (OTextLine *) _items[_items.size() - 1];
  if (ln->GetTextLength() > 0)
    fprintf(fp, "%s\n", ln->GetString());

  fclose(fp);
  _changed = False;

  return True;
}

// Use AddLine to add new lines at the end of the text.
// The lines should not contain any newline characters.

void OXTextEdit::AddLine(const OString *string) {
  AddLine(string->GetString());
}

void OXTextEdit::AddLine(const char *string) {
  AddItem(new OTextLine(this, string));
  _changed = True;
}

// Use InsertText to insert an arbitrary segment of text at the current
// cursor position. Newline characters are processed, causing a new
// OTextLine object to be created. After the insertion, the cursor is
// moved to the end of the just added text.

void OXTextEdit::InsertText(const OString *string) {
  InsertText(string->GetString());
}

void OXTextEdit::InsertText(const char *string, int len) {
  int  i, c, j;
  char buf[1024];
  OTextLine *line;

  if (_items.size() == 0) AddLine("");
  if (len < 0) len = strlen(string);

  line = (OTextLine *) _items[_textIndex.y];
  for (i = j = 0; i < len; ++i) {
    c = string[i];
    if (c == '\n') {
      if (j > 0) {
        buf[j] = '\0';
        line->AddText(_textIndex.x, buf);
        line->Resize(line->GetDefaultSize());
        _textIndex.x += j;
        j = 0;
      }
      SplitLine(_textIndex.y++, _textIndex.x);
      _textIndex.x = 0;
      line = (OTextLine *) _items[_textIndex.y];
    } else {
      buf[j++] = c;
      if (j == 1023) {
        buf[j] = '\0';
        line->AddText(_textIndex.x, buf);
        line->Resize(line->GetDefaultSize());
        _textIndex.x += j;
        j = 0;
      }
    }
  }
  if (j > 0) {
    buf[j] = '\0';
    line->AddText(_textIndex.x, buf);
    line->Resize(line->GetDefaultSize());
    _textIndex.x += j;
  }
  _changed = True;
  _UpdateCursor(_textIndex);
  _cursorLastX = _cursorPos.x;
  if (_autoUpdate) {
    CalcMaxItemSize();
    _needItemLayout = True;
    Layout();
    NeedRedraw(ORectangle(_visibleStart, _canvas->GetSize()));
  }
}

// Return the whole contents of the text widget in a single OString object.

OString *OXTextEdit::GetText() {
  OString *retString = new OString();

  for (unsigned int i = 0; i < _items.size(); i++) {
    retString->Append(((OTextLine *) _items[i])->GetString());
    retString->Append("\n");
  }

  return retString;
}

// Return the contents of the selection in a single OString object.
// If there is no current selection, NULL is returned.

OString *OXTextEdit::GetSelectedText() {

  if (!_hasSelection) return NULL;

  OString *retString = new OString();

#if 0 // hmmm... _selectedItems[] is not guaranteed to be in the correct order
  for (unsigned int i = 0; i < _selectedItems.size(); i++) {
    if (i > 0) retString->Append("\n");
    OTextLine *ln = (OTextLine *) _selectedItems[i];
    const char *str = ln->GetString() + ln->GetSelectionStart();
    retString->Append(str, ln->GetSelectionLength());
  }
#else
  int nl = 0;
  for (unsigned int i = 0; i < _items.size(); i++) {
    OTextLine *ln = (OTextLine *) _items[i];
    if (ln->IsSelected()) {
      if (nl) retString->Append("\n"); else ++nl;
      const char *str = ln->GetString() + ln->GetSelectionStart();
      retString->Append(str, ln->GetSelectionLength());
    }
  }
#endif

  return retString;
}

// Text search operation: the text widget is searched from the current
// cursor position in the specified direction, when a match is found it
// becomes selected, the cursor is positioned at the end of the selected
// segment (just as if it was selected with the mouse) and the view
// adjusted as to make the selection visible.
// 
// Current limitations:
// - the pattern must be fully contained in a single line (i.e. no '\n'
//   are allowed in the search pattern).

int OXTextEdit::Search(const char *pattern, int direction, bool match_case) {
  OTextLine *line;
  OPosition ix = _textIndex;
  int patlen = strlen(pattern);
  const char *str, *p;

  if (_items.size() == 0 || patlen == 0) return False;

  line = (OTextLine *) _items[ix.y];

  if (direction > 0) {  // up
    while (1) {
      if (ix.x + patlen > line->GetTextLength()) {
        if (ix.y == _items.size() - 1) return False;
        line = (OTextLine *) _items[++ix.y];
        ix.x = 0;
      }
      str = line->GetString();
      while (str[ix.x] && (tolower(str[ix.x]) != tolower(pattern[0]))) ++ix.x;
      if (str[ix.x]) {
        p = str + ix.x;
        if ((match_case ? strncmp(p, pattern, patlen) :
                          strncasecmp(p, pattern, patlen)) == 0) {
          UnSelectAll();
          SelectFrom(OPosition(p - str, ix.y));
          SelectTo(OPosition(p - str + patlen, ix.y));
          SetCursorPosition(OPosition(p - str + patlen, ix.y), True);
          _cursorLastX = _cursorPos.x;
          return True;
        } else {
          ix.x = p - str + 1;
        }
      } else {
        ix.x = line->GetTextLength();
      }
    }
  } else {              // down
    /**/ ix.x -= patlen + 1;
    while (1) {
      if (ix.x < 0) {
        if (ix.y == 0) return False;
        line = (OTextLine *) _items[--ix.y];
        ix.x = line->GetTextLength() - 1;
      }
      str = line->GetString();
      while (tolower(str[ix.x]) != tolower(pattern[0]) && ix.x > 0) --ix.x;
      if (ix.x >= 0) {
        p = str + ix.x;
        if ((match_case ? strncmp(p, pattern, patlen) :
                          strncasecmp(p, pattern, patlen)) == 0) {
          UnSelectAll();
          SelectFrom(OPosition(p - str, ix.y));
          SelectTo(OPosition(p - str + patlen, ix.y));
          SetCursorPosition(OPosition(p - str + patlen, ix.y), True);
          return True;
        } else {
          --ix.x;
        }
      } else {
        ix.x = -1;
      }
    }
  }
}

void OXTextEdit::_CheckLayout(OTextLine *line) {
  if (line->GetSize().w > _maxItemSize.w) {
    _maxItemSize = line->GetSize();
    Layout();
  } else {
    CalcMaxItemSize();
    if (line->GetSize().w == _maxItemSize.w) Layout();
  }
line->Resize(line->GetDefaultSize());
//  NeedRedraw(ORectangle(line->GetPosition(), line->GetSize()));
  NeedRedraw(ORectangle(line->GetPosition().x + _visibleStart.x,
                        line->GetPosition().y,
                        _canvas->GetSize().w,
                        line->GetSize().h));
}

bool OXTextEdit::ItemLayout() {
  _virtualSize = ODimension(0, _margin.h);
  ODimension itemSize;

  for (unsigned int i = 0; i < _items.size(); i++) {

    _items[i]->Move(OPosition(_margin.w, _virtualSize.h));

    itemSize = _items[i]->GetDefaultSize();
    _items[i]->Resize(itemSize);

    _virtualSize.h += itemSize.h;
    if (_virtualSize.w < itemSize.w) _virtualSize.w = itemSize.w;
  }

  _virtualSize.w += 2 * _margin.w;
  _virtualSize.h += _margin.h;

  _needItemLayout = False;

  return True;
}

//unsigned int OXTextEdit::GetNumberOfLines() {
//  return (_virtualSize.h - 2 * _margin.h) / _font->TextHeight();
//}


void OXTextEdit::DrawItem(OItem *item, OPosition coord, ODimension size) {
  OTextLine *line = (OTextLine *) item;
  OPosition stringPos;
  int charStart, charEnd, charsWidth;

  char *ustr = line->GetUntabbedText(0, line->GetTextLength());
  if (!ustr) return;
  int ulen = strlen(ustr);

  OFontMetrics fm;

  _font->GetFontMetrics(&fm);

  int ta = fm.ascent;
  int th = line->GetTextHeight();

  // ok, this is the first version, no optimization but trying 
  // to make it work!

  int ci = line->GetCharIndexByCoord(_margin.w, coord.x);
  if (ci > 0) --ci;
  ci = line->Index2Column(ci);

  stringPos = ToPhysical(OPosition(line->GetPosition().x + 
                                   line->GetTextWidth(0, ci),
                                   line->GetPosition().y));

  // hack: draw one additional char at both ends
  // to avoid partially drawn chars when scrolling left/right
  // (this routine should be rewritten)

  int ci1 = line->GetCharIndexByCoord(_margin.w, coord.x + size.w);
  ci1 = line->Index2Column(ci1);
  int ci2 = line->Index2Column(line->Column2Index(ci1) + 2);

  charStart = ci;
  charEnd = ci2;

  if (line->IsSelected()) {

    int selStart = line->GetSelectionStart();
    int selEnd = selStart + line->GetSelectionLength();

    selStart = line->Index2Column(selStart);
    selEnd = line->Index2Column(selEnd);

    // possible string segment before selection

    if (selStart >= charStart) {

      int cStart = charStart;
      int cEnd = charEnd;

      if (selStart <= charEnd) cEnd = selStart;

      charsWidth = line->GetTextWidth(cStart, cEnd);

      _canvas->FillRectangle(_backGC, stringPos.x, stringPos.y,
                             charsWidth, th + 1);
      _canvas->DrawString(_normGC->GetGC(), stringPos.x, stringPos.y + ta,
                          ustr + cStart, cEnd - cStart);

      stringPos.x += charsWidth;
    }

    // selected segment

    if ((selStart <= charEnd) && (selEnd >= charStart)) {

      int cStart = charStart;
      int cEnd = charEnd;

      if (selStart >= charStart) cStart = selStart;
      if (selEnd <= charEnd) cEnd = selEnd;

      charsWidth = line->GetTextWidth(cStart, cEnd);

      _canvas->FillRectangle(_hilightGC->GetGC(), stringPos.x, stringPos.y,
                             charsWidth, th + 1);
      _canvas->DrawString(_selGC->GetGC(), stringPos.x, stringPos.y + ta,
                          ustr + cStart, cEnd - cStart);

      stringPos.x += charsWidth;
    }

    // posible segment after selection

    if (selEnd <= charEnd) {

      int cStart = charStart;
      int cEnd = charEnd;

      if (selEnd >= charStart) cStart = selEnd;
      if (cEnd < ulen - 1) ++cEnd;

      charsWidth = line->GetTextWidth(cStart, cEnd);

      _canvas->FillRectangle(_backGC, stringPos.x, stringPos.y,
                             charsWidth, th + 1);
      _canvas->DrawString(_normGC->GetGC(), stringPos.x, stringPos.y + ta,
                          ustr + cStart, cEnd - cStart);

    }

  } else {

    if (ulen >= charStart) {

      if (charEnd < ulen - 1) ++charEnd;

      charsWidth = line->GetTextWidth(charStart, charEnd);

      _canvas->FillRectangle(_backGC, stringPos.x, stringPos.y,
                             charsWidth, th + 1);
      _canvas->DrawString(_normGC->GetGC(), stringPos.x, stringPos.y + ta,
                          ustr + charStart, charEnd - charStart);

    }
  }

  delete[] ustr;

  if (_cursorState) DrawCursor(_cursorState);
}

// get nearest coordinate that is related to the textobject

OPosition OXTextEdit::AdjustCoord(OPosition pos) {
  OPosition itemPos = pos;
  OItem *item;

  if (_items.size() == 0) return OPosition(_margin.w, _margin.h);

  item = GetItemByPos(itemPos);

  if (!item) {      // hm, outside any line...
                    // check if there is text left to it..
    itemPos.x = _margin.w;
    item = GetItemByPos(itemPos);

    if (!item) {     // ok, looks like we are totally outside the text area...
      if (itemPos.y < (int) _margin.h) {
        itemPos.x = _margin.w;
        itemPos.y = _margin.h;
      } else {
        itemPos.x = _margin.w;
        itemPos.y = _virtualSize.h - _margin.h;
#if 0
        item = GetItemByPos(itemPos);
#else
        item = (OTextLine *) _items[_items.size()-1];
#endif
        itemPos.x = item->GetSize().w + _margin.w;
      }
    } else {
      if (pos.x < (int) _margin.w)
        itemPos.x = _margin.w;
      else
        itemPos.x = item->GetSize().w + _margin.w;
    }

  }

  return itemPos;
}


//----------------------------------------------------------------------

void OXTextEdit::CursorLeft() {

  if (_items.size() == 0) return;

  OPosition newPos = _textIndex;
  OTextLine *line = (OTextLine *) _items[_textIndex.y];

  if (_textIndex.x > 0) {
    newPos.x = _textIndex.x - 1;
  } else if (_textIndex.y > 0) {
    line = (OTextLine *) _items[newPos.y-1];
    newPos.x = line->GetTextLength();
    --newPos.y;
  }

  _UpdateCursor(newPos);
  _cursorLastX = _cursorPos.x;
}

void OXTextEdit::CursorRight() {

  if (_items.size() == 0) return;

  OPosition newPos = _textIndex;
  OTextLine *line = (OTextLine *) _items[_textIndex.y];

  if (_textIndex.x < line->GetTextLength()) {
    newPos.x = _textIndex.x + 1;
  } else {
    if (newPos.y < _items.size()-1) {
      ++newPos.y;
      newPos.x = 0;
    }
  }

  _UpdateCursor(newPos);
  _cursorLastX = _cursorPos.x;
}

void OXTextEdit::CursorUp() {

  if (_items.size() == 0) return;

  OPosition newPos = _textIndex;

  if (newPos.y > 0) --newPos.y;
  OTextLine *line = (OTextLine *) _items[newPos.y];
  newPos.x = line->Column2Index(_cursorLastX);

  _UpdateCursor(newPos);
}

void OXTextEdit::CursorDown() {

  if (_items.size() == 0) return;

  OPosition newPos = _textIndex;

  if (newPos.y < _items.size() - 1) ++newPos.y;
  OTextLine *line = (OTextLine *) _items[newPos.y];
  newPos.x = line->Column2Index(_cursorLastX);

  _UpdateCursor(newPos);
}

void OXTextEdit::CursorPageUp() {

  if (_items.size() == 0) return;

  OPosition newPos = _textIndex;
  OTextLine *line = (OTextLine *) _items[newPos.y];

  int h = 0, th = line->GetSize().h;

  while ((h + th < _canvas->GetHeight()) && (newPos.y > 0)) {
    h += (th = _items[newPos.y--]->GetSize().h);
  }
  line = (OTextLine *) _items[newPos.y];
  newPos.x = line->Index2Column(_cursorLastX);

  _UpdateCursor(newPos);
}

void OXTextEdit::CursorPageDown() {

  if (_items.size() == 0) return;

  OPosition newPos = _textIndex;
  OTextLine *line = (OTextLine *) _items[newPos.y];

  int h = 0, th = line->GetSize().h;

  while ((h + th < _canvas->GetHeight()) && (newPos.y < _items.size()-1)) {
    h += (th = _items[newPos.y++]->GetSize().h);
  }

  line = (OTextLine *) _items[newPos.y];
  newPos.x = line->Index2Column(_cursorLastX);

  _UpdateCursor(newPos);
}

void OXTextEdit::CursorHome() {
  if (_items.size() == 0) return;

  OPosition newPos = _textIndex;

  newPos.x = 0;

  _UpdateCursor(newPos);
  _cursorLastX = _cursorPos.x;
}

void OXTextEdit::CursorEnd() {
  if (_items.size() == 0) return;

  OPosition newPos = _textIndex;
  OTextLine *line = (OTextLine *) _items[_textIndex.y];

  newPos.x = line->GetTextLength();

  _UpdateCursor(newPos);
  _cursorLastX = _cursorPos.x;
}

void OXTextEdit::CursorTop() {
  if (_items.size() == 0) return;

  _UpdateCursor(OPosition(0, 0));
  _cursorLastX = _cursorPos.x;
}

void OXTextEdit::CursorBottom() {
  if (_items.size() == 0) return;

  OPosition newPos = _textIndex;

  newPos.y = _items.size() - 1;
  OTextLine *line = (OTextLine *) _items[newPos.y];
  newPos.x = line->GetTextLength();

  _UpdateCursor(newPos);
  _cursorLastX = _cursorPos.x;
}

void OXTextEdit::_UpdateCursor(OPosition newPos) {

  if (_items.size() == 0) return;

  OTextLine *line = (OTextLine *) _items[newPos.y];
  if (!_readOnly && HasFocus()) _cursorState = 1;
  SetCursorPosition(newPos, True);

  if (_cursorBlink) {
    delete _cursorBlink;
    _cursorBlink = new OTimer(this, _blinkTime);
  }
}

//----------------------------------------------------------------------

// Set the cursor (insertion point) to the specified line and character.
// Note that the position is in text coordinates, i.e. TABs count as a
// single character.
//
// If ensure_visible is True, the view is scrolled if neccessary to make
// the cursor visible.

void OXTextEdit::SetCursorPosition(OPosition newpos, int ensure_visible) {

  if (_items.size() == 0) return;

  if (newpos.y < 0) newpos.y = 0;
  if (newpos.y > _items.size() - 1) newpos.y = _items.size() - 1;

  OTextLine *line = (OTextLine *) _items[newpos.y];

  if (newpos.x < 0) newpos.x = 0;
  if (newpos.x > line->GetTextLength()) newpos.x = line->GetTextLength();

  int oldstate = _cursorState;
  DrawCursor(_cursorState = 0);
  _textIndex = newpos;
  _cursorPos.x = line->Index2Column(_textIndex.x);
  _cursorPos.y = _textIndex.y;
  if (ensure_visible) EnsureVisible(_cursorPos);
  DrawCursor(_cursorState = oldstate);

  OItemViewMessage message(_msgType, MSG_CURSORPOS, _id, 0, _cursorPos);
  SendMessage(_msgObject, &message);
}


void OXTextEdit::EnsureVisible(OPosition cpos) {
  OPosition pos, lpos;
  OTextLine *line = (OTextLine *) _items[cpos.y];

  lpos = line->GetPosition();
  lpos.x = line->GetTextWidth(0, cpos.x);
  pos = ToPhysical(lpos);
  int lh = line->GetSize().h;

  if (pos.y < (int) _margin.h) {
    ScrollToPosition(OPosition(_visibleStart.x, lpos.y - _margin.h));
  } else if (pos.y > _canvas->GetHeight() - lh - _margin.h) {
    ScrollToPosition(OPosition(_visibleStart.x, lpos.y -
                               _canvas->GetHeight() + lh + _margin.h));
  }

  if (pos.x < 0) {
    ScrollToPosition(OPosition(lpos.x, _visibleStart.y));
  } else {
    lpos.x = line->GetTextWidth(0, cpos.x);
    pos = ToPhysical(lpos);
    if (pos.x >= _canvas->GetWidth() - 2 * _margin.w - 2) {
      ScrollToPosition(OPosition(lpos.x - _canvas->GetWidth() +
                                 2 * _margin.w + 2,
                                 _visibleStart.y));
    }
  }

}

// Split the specified line at charpos character into a pair of new lines.
// The current cursor position in NOT changed.

void OXTextEdit::SplitLine(int lnum, int charpos) {
  OTextLine *line = (OTextLine *) _items[lnum];

  char *tmp = StrDup(line->GetString() + charpos);
  line->RemoveText(charpos, strlen(tmp));
  line->Resize(line->GetDefaultSize());

  OTextLine *newline = new OTextLine(this, tmp);

  if (lnum < _items.size()-1)
    _items.insert(_items.begin() + lnum + 1, newline); // .insert(&_items[lnum+1], newline);
  else
    _items.push_back(newline);

  _changed = True;
  if (_autoUpdate) {
    NeedRedraw(ORectangle(line->GetPosition(), _canvas->GetSize()));
    CalcMaxItemSize();
    Layout();
  }
}

// Join the specified line and the next into a new single one.
// The cursor position in not changed.

void OXTextEdit::JoinLine(int lnum) {
  OTextLine *line = (OTextLine *) _items[lnum];

  if (lnum < _items.size()-1) {
    line->Append(((OTextLine *) _items[lnum+1])->GetString());
    line->Resize(line->GetDefaultSize());
    delete _items[lnum+1];
    _items.erase(_items.begin() + lnum + 1); // .erase(&_items[lnum+1]);
    _changed = True;
    if (_autoUpdate) {
      Layout();
      NeedRedraw(ORectangle(line->GetPosition(), _canvas->GetSize()));
    }
  }
}

// Delete the char at the specified line and position
// The cursor position in not changed.

void OXTextEdit::DeleteChar(int lnum, int charpos) {
  OTextLine *line = (OTextLine *) _items[lnum];

  if (charpos < line->GetTextLength()) {
    line->RemoveChar(charpos);
    line->Resize(line->GetDefaultSize());
    _changed = True;
    if (_autoUpdate) _CheckLayout(line);
  } else if (lnum < _items.size()-1) {
    JoinLine(lnum);
  }
}

// Delete the current selected text. The cursor position
// is updated accordingly

void OXTextEdit::DeleteSelection() {
  OTextLine *line;
  int i, join = -1;

  if (!_hasSelection) return;

  for (i = 0; i < _items.size(); ++i)
    if (_items[i]->IsSelected()) break;
  line = (OTextLine *) _items[i];
  _textIndex.x = line->GetSelectionStart();
  _textIndex.y = i;

  for (i = 0; i < _items.size(); ++i) {
    line = (OTextLine *) _items[i];
    if (line->IsSelected()) {
      int charStart = line->GetSelectionStart();
      int charEnd = line->GetSelectionEnd();
      if ((join < 0) && (_selectedItems.size() > 1) &&
          (charStart != 0) && (charEnd == line->GetTextLength())) {
        join = i;
      }
      if ((charStart == 0) && (charEnd == line->GetTextLength())
          && (i < _items.size()-1) && _items[i+1]->IsSelected()) {
        delete _items[i];  // delete line;
        _items.erase(_items.begin() + i);   // .erase(&_items[i]);
        --i;
      } else {
        line->RemoveText(charStart, charEnd - charStart);
        line->Select(False);
      }
    }
  }
  if (join >= 0) JoinLine(join);

  _selectedItems.clear();
  _changed = True;
  _hasSelection = False;

  CalcMaxItemSize();
  _UpdateCursor(_textIndex);
  _cursorLastX = _cursorPos.x;
  _needItemLayout = True;
  Layout();
  NeedRedraw(ORectangle(_visibleStart, _canvas->GetSize()));

  OItemViewMessage message(_msgType, MSG_SELCHANGED, _id);
  SendMessage(_msgObject, &message);
}

//----------------------------------------------------------------------

void OXTextEdit::SelectToPos(OPosition mousePos) {
  OTextLine *item;
  OPosition itemPos;
  ODimension itemSize;
  OTextLine *mouseLine;
  int lineSelStart, lineSelEnd;

  if (_items.size() == 0) return;

  if (_dragSelecting) {

    // adjust mouse coordinates if necessary
    // and find the apropriate text line

    mousePos = AdjustCoord(mousePos);
    mouseLine = (OTextLine *) GetItemByPos(mousePos);
    mousePos.y = mouseLine->GetPosition().y;

    if (mousePos.y > _selAnchor.y) {

      lineSelStart = 0;
      lineSelEnd = mousePos.x;

      // if True this indicates that the drag point was passed (in y coord)
      // during sel

      if (_lastDragPos.y == _selAnchor.y) {
        int from = _dragStartLine->GetCharIndexByCoord(_margin.w, _selAnchor.x);
        _dragStartLine->SelectChars(from, _dragStartLine->GetTextLength());
        OXItemView::SelectItem(_dragStartLine, True);
      }

    } else if (mousePos.y < _selAnchor.y) {

      lineSelStart = mousePos.x;
      lineSelEnd = mouseLine->GetSize().w + _margin.w;

      // same as above, again the drag starting line must be adjusted...

      if (_lastDragPos.y == _selAnchor.y) {
        int to = _dragStartLine->GetCharIndexByCoord(_margin.w, _selAnchor.x);
        _dragStartLine->SelectChars(0, to);
        OXItemView::SelectItem(_dragStartLine, True);
      }

    } else {

      lineSelStart = min(mousePos.x, _selAnchor.x);
      lineSelEnd = max(mousePos.x, _selAnchor.x);

    }

    // ...done, now select it !

    _lastDragPos = mousePos;

    mouseLine->SelectChars(mouseLine->GetCharIndexByCoord(_margin.w, lineSelStart),
                           mouseLine->GetCharIndexByCoord(_margin.w, lineSelEnd));

    OXItemView::SelectItem(mouseLine, True);

    // update the coordinates of the selection area

    _selDragStart.x = min(_selAnchor.x, mousePos.x);
    _selDragStart.y = min(_selAnchor.y, mousePos.y);
    _selDragEnd.x = max(_selAnchor.x, mousePos.x);
    _selDragEnd.y = max(_selAnchor.y, mousePos.y);

    // all lines that are inside the selection area are selected and their
    // selection is adjusted

    for (unsigned int i = 0; i < _items.size(); i++) {
      item = (OTextLine *) _items[i];
      itemPos = item->GetPosition();
      itemSize = item->GetSize();
      if ((item != mouseLine)
          && (itemPos.y <= _selDragEnd.y)
          && (itemPos.y + itemSize.h > _selDragStart.y)) {
        if (!item->IsSelected()) {
          item->SelectChars(0, item->GetTextLength());
          OXItemView::SelectItem(item, True);
        } else {
          // the drag starting line was already adjusted above (if necessary)
          if (item != _dragStartLine) {
            if ((item->GetSelectionStart() != 0) ||
                (item->GetSelectionLength() != item->GetTextLength())) {
              item->SelectChars(0, item->GetTextLength());
              //if (!item->IsSelected()) {
                OXItemView::SelectItem(item, True);
              //}
            }
          }
        }
      } else if ((item != mouseLine) && (item->IsSelected())) {
        OXItemView::SelectItem(item, False);
      }
    }

    OItemViewMessage message(_msgType, MSG_SELCHANGED, _id);
    SendMessage(_msgObject, &message);
  }

}

void OXTextEdit::SelectFrom(OPosition tindex) {
  OTextLine *line = (OTextLine *) _items[tindex.y];
  int x = line->GetTextWidth(0, line->Index2Column(tindex.x)) + _margin.w;
  int y = line->GetPosition().y + line->GetSize().h / 2;
  _selAnchor = OPosition(x, y);

  _dragStartLine = line;
  _dragStartLine->SelectChars(tindex.x, tindex.x);
  _selAnchor.y = _dragStartLine->GetPosition().y;

  _selDragStart = _selAnchor;
  _selDragEnd = _selDragStart;
}

void OXTextEdit::SelectTo(OPosition tindex) {
  int old = _dragSelecting;
  _dragSelecting = True;
  OTextLine *line = (OTextLine *) _items[tindex.y];
  int x = line->GetTextWidth(0, line->Index2Column(tindex.x)) + _margin.w;
  int y = line->GetPosition().y + line->GetSize().h / 2;
  SelectToPos(OPosition(x, y));
  _dragSelecting = old;
}


void OXTextEdit::SelectAll() {
  for (unsigned int i = 0; i < _items.size(); ++i) {
    OTextLine *item = (OTextLine *) _items[i];
    item->SelectChars(0, item->GetTextLength());
  }
  OXItemView::SelectAll();  // here and not before!

  OItemViewMessage message(_msgType, MSG_SELCHANGED, _id);
  SendMessage(_msgObject, &message);
}

void OXTextEdit::UnSelectAll() {
  if (_hasSelection) {
    OXItemView::UnSelectAll();

    OItemViewMessage message(_msgType, MSG_SELCHANGED, _id);
    SendMessage(_msgObject, &message);
  }
}

void OXTextEdit::AutoUpdate(int onoff) {
  if (_autoUpdate != onoff) {
    _autoUpdate = onoff;
    if (_autoUpdate) {
      _UpdateCursor(_textIndex);
      _cursorLastX = _cursorPos.x;
      CalcMaxItemSize();
      _needItemLayout = True;
      Layout();
      NeedRedraw(ORectangle(_visibleStart, _canvas->GetSize()));
    }
  }
}


//----------------------------------------------------------------------

OTextLine::OTextLine(OXTextEdit *p, const char *string) :
  OItem(p), OTextBuffer(strlen(string)+1, 32) {
    AddText(0, string);
    _selStart = 0;
    _selEnd = 0;
    const OXFont *font = p->GetFont();
    _size = ODimension(GetTextWidth(0, Index2Column(GetTextLength())),
                       font->TextHeight() + 1);
}

OTextLine::OTextLine(OXTextEdit *p, OTextLine *line) :
  OTextBuffer(line->GetTextLength()+1, 32), OItem(p) {
    AddText(0, line->GetString());
    _selStart = 0;
    _selEnd = 0;
    const OXFont *font = p->GetFont();
    _size = ODimension(GetTextWidth(0, Index2Column(GetTextLength())),
                       font->TextHeight() + 1);
}

void OTextLine::SelectChars(int from, int to) {
  _selStart = from;
  _selEnd = to;
}

void OTextLine::Select(bool state) {
  if (!state) {
    _selStart = 0;
    _selEnd = 0;
  }
  OItem::Select(state);
}

ODimension OTextLine::GetDefaultSize() const {
  const OXFont *font = ((OXTextEdit *)_parent)->GetFont();
  return ODimension(GetTextWidth(0, Index2Column(GetTextLength())),
                    font->TextHeight() + 1);
}

int OTextLine::GetCharIndexByCoord(int x0, int xCoord) const {
  const OXFont *font = ((OXTextEdit *)_parent)->GetFont();
  unsigned int tabw = ((OXTextEdit *)_parent)->GetTabWidth();
  int i, col, tw, len, incr, spcw = font->XTextWidth(" ", 1);

  len = GetTextLength();
  tw = 0;
  xCoord -= x0;

  for (col = i = 0; i < len; ++i) {
    if (_buffer[i] == TAB) {
      incr = tabw - (col % tabw);
      tw += incr * spcw;
      col += incr;
    } else {
      tw += font->XTextWidth(&_buffer[i], 1);
      ++col;
    }
    if (tw > xCoord) return i;
  }

  return len;
}

// The coordinates pased to GetTextWidth() are in screen text
// (i.e. untabbed text) units

int OTextLine::GetTextWidth(int from, int to) const {
  const OXFont *font = ((OXTextEdit *)_parent)->GetFont();
  unsigned int tabw = ((OXTextEdit *)_parent)->GetTabWidth();
  int len = GetTextLength();

  if (from < 0) from = 0;
  if (from >= to) return 0;

  int i, col, tw1, tw2, incr, spcw = font->XTextWidth(" ", 1);

  tw2 = 0;
  col = 0;

  for (i = 0; i < len; ++i) {
    if (col >= from) break;
    if (_buffer[i] == TAB) {
      incr = tabw - (col % tabw);
      tw2 += incr * spcw;
      col += incr;
    } else {
      tw2 += font->XTextWidth(&_buffer[i], 1);
      ++col;
    }
  }
  tw1 = tw2;
  for (; i < len; ++i) {
    if (col >= to) break;
    if (_buffer[i] == TAB) {
      incr = tabw - (col % tabw);
      tw2 += incr * spcw;
      col += incr;
    } else {
      tw2 += font->XTextWidth(&_buffer[i], 1);
      ++col;
    }
  }
  return tw2 - tw1;
}

// Translate (untabbed) string column position to (tabbed) string index

int OTextLine::Column2Index(int col) const {
  int i, vpos, slen = GetTextLength();
  unsigned int tabw = ((OXTextEdit *)_parent)->GetTabWidth();

  for (vpos = 0, i = 0; i < slen; ++i) {
    vpos += (_buffer[i] == TAB) ? tabw - (vpos % tabw) : 1;
    if (vpos > col) break;
  }

  return i;
}

// Translate (tabbed) string index to (untabbed) string column position

int OTextLine::Index2Column(int index) const {
  int i, vpos, slen = GetTextLength();
  unsigned int tabw = ((OXTextEdit *)_parent)->GetTabWidth();

  for (vpos = 0, i = 0; (i < index) && (i < slen); ++i)
    vpos += (_buffer[i] == TAB) ? tabw - (vpos % tabw) : 1;

  return vpos;
}

char *OTextLine::GetUntabbedText(int pos, int length) const {
  int  i, j, k, ix, ulen, slen = GetTextLength();
  unsigned int tabw = ((OXTextEdit *)_parent)->GetTabWidth();
  char *retstring;

  ix = Column2Index(pos);

//  if ((ix >= slen) || (length > slen) || (ix + length > slen))
//    return NULL;
  if (ix >= slen) return NULL;

  ulen = Index2Column(length);

  retstring = new char[ulen + 1];

  for (i = 0, j = 0; i < length; ++i) {
    if (_buffer[i+ix] == TAB) {
      k = tabw - (pos % tabw);
      memset(retstring + j, ' ', k);
      j += k; pos += k;
    } else {
      retstring[j++] = _buffer[i+ix];
      ++pos;
    }
  }

  retstring[j] = '\0';

  return retstring;
}
