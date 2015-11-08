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
#include <xclass/OXMenu.h>
#include <xclass/OXMainFrame.h>
#include <xclass/OXButton.h>
#include <xclass/OXClient.h>
#include <xclass/OXFont.h>
#include <xclass/OGC.h>
#include <xclass/OResourcePool.h>
#include <xclass/OString.h>

#include <X11/keysym.h>


const OXGC *OXPopupMenu::_defaultGC;
const OXGC *OXPopupMenu::_defaultSelGC;
const OXGC *OXPopupMenu::_defaultSelBckgndGC;
const OXFont *OXPopupMenu::_defaultFont;
const OXFont *OXPopupMenu::_hilightFont;      
int   OXPopupMenu::_init = False;

const OXGC *OXMenuTitle::_defaultGC;
const OXGC *OXMenuTitle::_defaultSelGC;
const OXGC *OXMenuTitle::_defaultSelBckgndGC;
const OXFont *OXMenuTitle::_defaultFont;
int   OXMenuTitle::_init = False;


//-----------------------------------------------------------------

OXMenuBar::OXMenuBar(const OXWindow *p, int w, int h,
		     unsigned int options) :
  OXHorizontalFrame(p, w, h, options | HORIZONTAL_FRAME) {

    _stick = True;
    _current = NULL;

    XGrabButton(GetDisplay(), Button1, AnyModifier, _id, False,
		ButtonPressMask | ButtonReleaseMask |
		EnterWindowMask,
		GrabModeAsync, GrabModeAsync, None, None);
}

OXMenuBar::~OXMenuBar() {
  SListFrameElt *ptr;
  OXMenuTitle *t;
  int keycode;
  const OXMainFrame *main = (OXMainFrame *) _toplevel;

  // we shouldn't be accessing the _flist directly, but rather by means of
  // some access method and declaring _flist as private in OXCompositeFrame

  for (ptr = _flist; ptr; ptr = ptr->next) {
    t = (OXMenuTitle *) ptr->frame;
    if (main && (keycode = t->GetHotKeyCode()) != 0)
      main->RemoveBind(this, keycode, Mod1Mask);
  }
}

void OXMenuBar::AddPopup(OHotString *s, OXPopupMenu *menu, OLayoutHints *l) {
  OXMenuTitle *t;
  int keycode;

  AddFrame(t = new OXMenuTitle(this, s, menu), l);

  if ((keycode = t->GetHotKeyCode()) != 0) {
    const OXMainFrame *main = (OXMainFrame *) _toplevel;
    if (main) {
      main->BindKey(this, keycode, Mod1Mask);
      main->BindKey(this, keycode, Mod1Mask | Mod2Mask);
      main->BindKey(this, keycode, Mod1Mask | LockMask);
      main->BindKey(this, keycode, Mod1Mask | Mod2Mask | LockMask);
    }
  }
}

int OXMenuBar::HandleButton(XButtonEvent *event) {
  SListFrameElt *ptr;

  // Here we are looking for a _child_ of a menu bar, so it can _ONLY_ be
  // a OXMenuTitle: BEWARE!

  OXMenuTitle *target;
  
  // We don't need to check the button number as XGrabButton will
  // only allow button1 events

  if (event->type == ButtonPress) {

    if (_current && _current->HandleButton(event)) return True;

    target = (OXMenuTitle *) _client->GetWindowById(event->subwindow);

    _x0 = event->x;
    _y0 = event->y;

    if (target) {
      _stick = True;

      if (target != _current) {
        // Deactivate all others :
        for (ptr = _flist; ptr != NULL; ptr = ptr->next)
          ((OXMenuTitle *)ptr->frame)->SetState(False);

        target->SetState(True);
        _current = target;

        XGrabPointer(GetDisplay(), _id, False,
                     ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                     GrabModeAsync, GrabModeAsync,
                     None, GetResourcePool()->GetGrabCursor(), CurrentTime);
        XGrabKeyboard(GetDisplay(), _id, False,
                      GrabModeAsync, GrabModeAsync,
                      CurrentTime);
      }
    }
  }
  
  if (event->type == ButtonRelease) {

    if (_stick) {
      _stick = False;
      return True;
    }
    XUngrabPointer(GetDisplay(), CurrentTime);
    XUngrabKeyboard(GetDisplay(), CurrentTime);
    XSync(GetDisplay(), False);

    for (ptr = _flist; ptr != NULL; ptr = ptr->next)
      ((OXMenuTitle *)ptr->frame)->SetState(False);

    if (_current) {
      target = _current; // tricky, because WaitFor()
      _current = NULL;
      target->DoSendMessage();
    }
  }

  return True;
}

int OXMenuBar::HandleMotion(XMotionEvent *event) {
  SListFrameElt *ptr;
  int dummy;
  Window wtarget;
  OXMenuTitle *target;

  // use some threshold
  if (_stick &&
      (abs(event->x - _x0) < 5) &&
      (abs(event->y - _y0) < 5)) return True;

  _stick = False;

  if (_current && _current->HandleMotion(event)) return True;

  XTranslateCoordinates(GetDisplay(), _id, _id, event->x, event->y,
                        &dummy, &dummy, &wtarget);
  target = (OXMenuTitle *)_client->GetWindowById(wtarget);

  if (target && target != _current) {
    // Deactivate all others :
    for (ptr = _flist; ptr != NULL; ptr = ptr->next)
      ((OXMenuTitle *)ptr->frame)->SetState(False);

    target->SetState(True);
    _stick = True;
    _current = target;
  }

  return True;
}

int OXMenuBar::HandleKey(XKeyEvent *event) {
  SListFrameElt *ptr;
  OXMenuTitle *target;
  int  n, retc = 0;
  char tmp[10];
  KeySym keysym;
  XComposeStatus compose = { NULL, 0 };

  if (!_stick && _current) {
    retc = _current->HandleKey(event);
    if (retc == True) return True;
  }

  if (event->type == KeyPress) {

    //--- 1: check for hotkeys...

    for (ptr = _flist; ptr != NULL; ptr = ptr->next) {
      target = (OXMenuTitle *) ptr->frame;
      if (event->keycode == target->GetHotKeyCode()) break;
    }
    if (ptr == NULL) target = NULL;

    //--- 2: check for left/right keys...

    if (target == NULL) {
      n = XLookupString(event, tmp, sizeof(tmp)-1, &keysym, &compose);
      tmp[n] = 0;

      switch (keysym) {
        case XK_Left:
          for (ptr = _flist; ptr->next != NULL; ptr = ptr->next) {
            target = (OXMenuTitle *) ptr->next->frame;
            if (_current == target) {
              target = (OXMenuTitle *) ptr->frame;
              break;
            }
          }
          if (ptr && ptr->next == NULL) target = (OXMenuTitle *) ptr->frame;
          break;

        case XK_Right:
          for (ptr = _flist; ptr->next != NULL; ptr = ptr->next) {
            target = (OXMenuTitle *) ptr->frame;
            if (_current == target) {
              target = (OXMenuTitle *) ptr->next->frame;
              break;
            }
          }
          if (ptr->next == NULL) target = (OXMenuTitle *) _flist->frame;
          break;
      }
    }

    //--- 3: Switch to the target...

    if (target) {
      _stick = True;

      if (target != _current) {
        // Deactivate all others :
        for (ptr = _flist; ptr != NULL; ptr = ptr->next)
          ((OXMenuTitle *)ptr->frame)->SetState(False);

        target->SetState(True);
        _stick = True;
        _current = target;

        XGrabPointer(GetDisplay(), _id, True,
                     ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                     GrabModeAsync, GrabModeAsync,
                     None, GetResourcePool()->GetGrabCursor(), CurrentTime);

        // Because of the way the OXMainFrame key binding mechanism
        // currently works, we can't grab the keyboard right here.
        // We'll do it instead after the first key has been released
        // (see below).

      }
    }
  }
  
  if (event->type == KeyRelease) {
    if (_stick) {
      _stick = False;

      // Grab the keyboard HERE!

      XGrabKeyboard(GetDisplay(), _id, False,
                    GrabModeAsync, GrabModeAsync,
                    CurrentTime);
      return True;
    }

    if (retc != 2) return True;

    XUngrabPointer(GetDisplay(), CurrentTime);
    XUngrabKeyboard(GetDisplay(), CurrentTime);
    XSync(GetDisplay(), False);

    for (ptr = _flist; ptr != NULL; ptr = ptr->next)
      ((OXMenuTitle *)ptr->frame)->SetState(False);

    if (_current) {
      target = _current; // tricky, because WaitFor()
      _current = NULL;
      target->DoSendMessage();
    }
  }

  return True;
}


//-----------------------------------------------------------------

OMenuEntry::OMenuEntry(int type, int id, OHotString *s, const OPicture *p) {
  _type = type;
  _entryID = id;
  _label = s;
  _pic = p;
  _popup = NULL;
  _status = MENU_ENABLE_MASK;
  _hkeycode = 0;
  prev = next = NULL;
}

OMenuEntry::~OMenuEntry() {
  if (_label) delete _label;
}


//-----------------------------------------------------------------

// space for the radio- and checkmarks:
#define CHECKMARK_WIDTH  8

// where the label starts (left margin):
#define LEFT_GAP         (CHECKMARK_WIDTH + 8)

// space between the icon and the text:
#define PIC_GAP          6

// width of the placeholder for the icon:
#define PICHOLDER_WIDTH  (PIC_GAP + LEFT_GAP)   

// space between left and right labels:
#define TEXT_GAP         10

// right margin:
#define RIGHT_GAP        14

// width of the popup menu symbol:
#define POPUPMARK_WIDTH  8


OXPopupMenu::OXPopupMenu(const OXWindow *p, int w, int h, 
			 unsigned int options)
  : OXFrame(p, w, h, options | OWN_BKGND ) {
    XSetWindowAttributes wattr;
    unsigned long mask;

    if (!_init) {
      _defaultGC = GetResourcePool()->GetFrameGC();
      _defaultSelGC = GetResourcePool()->GetSelectedGC();
      _defaultSelBckgndGC = GetResourcePool()->GetSelectedBckgndGC();

      _defaultFont = GetResourcePool()->GetMenuFont();
      _hilightFont = GetResourcePool()->GetMenuHiliteFont();

      _init = True;
    }

    _normGC    = (OXGC *) new OXGC(_defaultGC);          // ==!==
    _selGC     = (OXGC *) new OXGC(_defaultSelGC);       // ==!==
    _selbackGC = (OXGC *) new OXGC(_defaultSelBckgndGC); // ==!==
    _font      = _defaultFont;
    _hifont    = _hilightFont;

    _delay = NULL;

    _bw = 3;
    _xl = _xr = LEFT_GAP;
    _first = _last = NULL;

    _sidePic = NULL;
    _sideBgnd = _backPixel;

    _msgObject = p;
    _stick = True;
    _current = NULL;
    _hasgrab = False;
    _popdown = False;

    _lastID = -1;

    mask = CWOverrideRedirect | CWSaveUnder;
    wattr.override_redirect = True;
    wattr.save_under = True;
    
    XChangeWindowAttributes(GetDisplay(), _id, mask, &wattr);
    if (_defaultBackgroundPicture)
        SetBackgroundPixmap(_defaultBackgroundPicture->GetPicture());

    AddInput(StructureNotifyMask);
    AddInput(PointerMotionMask | EnterWindowMask | LeaveWindowMask);
}

OXPopupMenu::~OXPopupMenu() {
  RemoveAllEntries();
  if (_delay) delete _delay;
  if (_normGC != _defaultGC) delete _normGC;
  if (_selGC != _defaultSelGC) delete _selGC;
  if (_selbackGC != _defaultSelBckgndGC) delete _selbackGC;
  if (_font != _defaultFont) _client->FreeFont((OXFont *) _font);
  if (_hifont != _hilightFont) _client->FreeFont((OXFont *) _hifont);
  if (_sidePic) {
    _client->FreePicture(_sidePic);
    _client->FreeColor(_sideBgnd);
  }
}

void OXPopupMenu::AddEntry(OHotString *s, int ID, const OPicture *p) {
  OMenuEntry *e;
  int hotchar;

  e = new OMenuEntry(MENU_ENTRY, ID, s, p);

  e->_ex = _sidePic ? _sidePic->GetWidth() + 4 : 2;
  e->_ey = _h - 2;

  if ((hotchar = s->GetHotChar()) != 0)
    e->_hkeycode = XKeysymToKeycode(GetDisplay(), hotchar);
  else
    e->_hkeycode = 0;

  if (_last == NULL) {
    _first = _last = e;
  } else {
    _last->next = e;
    e->prev = _last;
    _last = e;
  }

  AdjustSize();
}

void OXPopupMenu::AddSeparator(int ID) {
  OMenuEntry *e;

  e = new OMenuEntry(MENU_SEPARATOR, ID);

  e->_ex = _sidePic ? _sidePic->GetWidth() + 4 : 2;
  e->_ey = _h - 2;

  if (_last == NULL) {
    _first = _last = e;
  } else {
    _last->next = e;
    e->prev = _last;
    _last = e;
  }

  AdjustSize();
}

void OXPopupMenu::AddPopup(OHotString *s, OXPopupMenu *popup, int ID) {
  OMenuEntry *e;
  int hotchar;

  e = new OMenuEntry(MENU_POPUP, ID, s);

  e->_popup = popup;
  e->_ex = _sidePic ? _sidePic->GetWidth() + 4 : 2;
  e->_ey = _h - 2;

  if ((hotchar = s->GetHotChar()) != 0)
    e->_hkeycode = XKeysymToKeycode(GetDisplay(), hotchar);
  else
    e->_hkeycode = 0;

  if (_last == NULL) {
    _first = _last = e;
  } else {
    _last->next = e;
    e->prev = _last;
    _last = e;
  }

  if (popup) popup->Associate(_msgObject);

  AdjustSize();
}

int OXPopupMenu::RenameEntry(int ID, OHotString *s) {
  OMenuEntry *ptr;
  int hotchar;

  if (!s) return False;

  for (ptr = _first; ptr; ptr = ptr->next) {
    if (ptr->_entryID == ID) break;
  }

  if (!ptr || (ptr->_type == MENU_SEPARATOR)) return False;

  if (ptr->_label) delete ptr->_label;
  ptr->_label = s;

  if ((hotchar = s->GetHotChar()) != 0)
    ptr->_hkeycode = XKeysymToKeycode(GetDisplay(), hotchar);
  else
    ptr->_hkeycode = 0;

  AdjustSize();

  return True;
}

int OXPopupMenu::SetEntryPic(int ID, const OPicture *p) {
  OMenuEntry *ptr;
  int hotchar;

  for (ptr = _first; ptr; ptr = ptr->next) {
    if (ptr->_entryID == ID) break;
  }

  if (!ptr || (ptr->_type == MENU_SEPARATOR)) return False;

  //if (ptr->_pic) _client->FreePicture(ptr->_pic);
  ptr->_pic = p;

  AdjustSize();

  return True;
}

void OXPopupMenu::RemoveEntry(int ID, int type) {
  OMenuEntry *ptr, *ptr2;

  for (ptr = _first; ptr; ptr = ptr->next) {
    if ((ptr->_entryID == ID) && (ptr->_type == type)) break;
  }
  if (ptr == NULL) return; // not found!

  if (ptr->prev == NULL) {
    _first = ptr->next;
    if (_first == NULL)
      _last = NULL;
    else
      _first->prev = NULL;
  } else if (ptr->next == NULL) {
    ptr2 = ptr->prev;
    ptr2->next = NULL;
    _last = ptr2;
  } else {
    ptr2 = ptr->prev;
    ptr2->next = ptr->next;
    ptr2 = ptr->next;
    ptr2->prev = ptr->prev;
  }
  if (ptr == _current) _current = NULL;
  delete ptr;

  AdjustSize();
}

void OXPopupMenu::RemoveAllEntries() {
  OMenuEntry *ptr, *next;

  ptr = _first;
  while (ptr) {
    next = ptr->next;
    delete ptr;
    ptr = next;
  }
  _first = _last = NULL;
  _current = NULL;

  AdjustSize();
}

void OXPopupMenu::SetSidePic(const OPicture *p, unsigned long color) {
  if (_sidePic) {
    _client->FreePicture(_sidePic);
    _client->FreeColor(_sideBgnd);
  }
  _sidePic = p;
  _sideBgnd = color;

  AdjustSize();
}

void OXPopupMenu::AdjustSize() {
  AdjustEntries();
  Resize(_mw, _mh);  // Resize(GetDefaultSize());
}

void OXPopupMenu::AdjustEntries() {
  int lsize, rsize, sepw, tw, ltw, rtw, x0, spw, pw, w, h;
  const char *lstr, *rstr;
  OMenuEntry *ptr;

  if (_sidePic) {
    x0 = _sidePic->GetWidth() + 4;
    spw = _sidePic->GetWidth();
    w = spw + 8;
  } else {
    x0 = 2;
    spw = 0;
    w = 8;
  }
  h = 6;
  _xl = _xr = LEFT_GAP;
  sepw = TEXT_GAP;
  lsize = rsize = 0;

  for (ptr = _first; ptr != NULL; ptr = ptr->next) {
    ptr->_ex = x0;
    ptr->_ey = h - 2;
    switch (ptr->_type) {
      case MENU_ENTRY:
        lstr = ptr->_label->GetString();
        rstr = strchr(lstr, '\t');
        if (rstr) {
          ltw = _hifont->TextWidth(lstr, rstr - lstr);
          ++rstr;
          rtw = _hifont->TextWidth(rstr, strlen(rstr));
        } else {
          ltw = _hifont->TextWidth(lstr, strlen(lstr));
          rtw = 0;
        }
        if (ptr->_pic) {
          pw = ptr->_pic->GetWidth() + PIC_GAP;
          if (pw + LEFT_GAP > _xl) _xl = pw + LEFT_GAP;
        } else {
          pw = 0;
        }
        lsize = max(lsize, ltw);
        rsize = max(rsize, rtw);
        tw = (rsize > 0) ? lsize + rsize + sepw : ltw;
        w = max(w, spw + _xl + tw + RIGHT_GAP); // _xl already includes pw
        h += _hifont->TextHeight() + 3;
        break;

      case MENU_POPUP:
        lstr = ptr->_label->GetString();
        rstr = strchr(lstr, '\t');
        if (rstr) {
          ltw = _hifont->TextWidth(lstr, rstr - lstr);
          ++rstr;
          rtw = _hifont->TextWidth(rstr, strlen(rstr));
        } else {
          ltw = _hifont->TextWidth(lstr, strlen(lstr));
          rtw = 0;
        }
        tw = (rsize > 0) ? lsize + rsize + sepw : ltw;
        w = max(w, spw + _xl + tw + RIGHT_GAP + POPUPMARK_WIDTH);
        h += _hifont->TextHeight() + 3;
        break;

      case MENU_SEPARATOR:
        h += 4;
        break;
    }
  }

  if (rsize > 0) _xr = _xl + lsize + sepw;

  _mw = w;
  _mh = h;
}

ODimension OXPopupMenu::GetDefaultSize() const {
//  AdjustEntries();
  return ODimension(_mw, _mh);
}

void OXPopupMenu::PlaceMenu(int x, int y, int stick_mode, int grab_pointer) {
  int rx, ry;
  unsigned int rw, rh;
  unsigned int dummy;
  Window wdummy;
  
  _stick = stick_mode;
  Activate(NULL);
  _current = NULL;

  // Parent is root window for a popup menu:
  XGetGeometry(GetDisplay(), _parent->GetId(), &wdummy,
               &rx, &ry, &rw, &rh, &dummy, &dummy);

  if (x + _w > rw) x -= _w;  //x = rw - _w;
  if (x < 0) x = 0;
  if (y + _h > rh) y -= _h;  //y = rh - _h;
  if (y < 0) y = 0;

  Move(x, y);
  MapRaised();

  if (grab_pointer) {
    XGrabPointer(GetDisplay(), _id, False,
 		 ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
		 GrabModeAsync, GrabModeAsync, 
                 None, GetResourcePool()->GetGrabCursor(), CurrentTime);
    XGrabKeyboard(GetDisplay(), _id, False,
		  GrabModeAsync, GrabModeAsync,
                  CurrentTime);
    _hasgrab = True;
  } else {
    _hasgrab = False;
  }
}

int OXPopupMenu::PopupMenu(int x, int y) {
  PlaceMenu(x, y, True, True);
  _client->WaitForUnmap(this);
  return _lastID;
}

int OXPopupMenu::EndMenu() {

  if (_delay) { delete _delay; _delay = NULL; }

  // destroy any cascaded child menus and get any ID back

  if (_current) {

    // deactivate the entry
    _current->_status &= ~MENU_ACTIVE_MASK;

    if (_current->_type == MENU_POPUP) {
      _lastID = _current->_popup->EndMenu();
    } else if (_current->_type == MENU_ENTRY) {
      // return the ID if the entry is enabled, otherwise -1
      if (_current->_status & MENU_ENABLE_MASK) {
        _lastID = _current->_entryID;
      } else {
        _lastID = -1;
      }
    } else {  // separator
      _lastID = -1;
    }

  } else {
    // if no entry selected...
    _lastID = -1;
  }

  // then unmap itself
  UnmapWindow();
  return _lastID;
}

int OXPopupMenu::HandleButton(XButtonEvent *event) {
  
  if (event->type == ButtonRelease) {
    if (_stick) {
      _stick = False;
      return True;
    }

    int ID = EndMenu();
    if (_hasgrab) {
      XUngrabPointer(GetDisplay(), CurrentTime);
      XUngrabKeyboard(GetDisplay(), CurrentTime);
      XSync(GetDisplay(), False);
    }
    if (_current) {
      _current->_status &= ~MENU_ACTIVE_MASK;
      if (_current->_status & MENU_ENABLE_MASK) {
        OMenuMessage message(MSG_MENU, MSG_CLICK, ID);
        SendMessage(_msgObject, &message);
      }
    }
  }
  return True;
}

int OXPopupMenu::HandleMotion(XMotionEvent *event) {
  OMenuEntry *ptr;
  int h;
  
  OXFrame::HandleMotion(event);
  _stick = False;

  if (_current && _current->_type == MENU_POPUP && _popdown) {
    OXPopupMenu *p = _current->_popup;
    if (p) {
      XMotionEvent ev = *event;
      ev.x = ev.x_root - p->GetX();
      ev.y = ev.y_root - p->GetY();
      int retc = p->HandleMotion(&ev);
      if (retc) return retc;
      if (!Contains(event->x, event->y)) return False;
    }
  }

  if (!Contains(event->x, event->y)) {
    Activate(NULL);
    return False;
  }

  for (ptr = _first; ptr != NULL; ptr = ptr->next) {
    if (ptr->_type == MENU_SEPARATOR) {
      h = 4;
    } else {
      h = _hifont->TextHeight() + 3 +2;
    }
    if ((event->x >= ptr->_ex) && (event->x <= ptr->_ex+_w-4) &&  //_w-10??
        (event->y >= ptr->_ey) && (event->y <= ptr->_ey+h)) break;
  }
  if (ptr != _current) Activate(ptr);
  return True;
}

int OXPopupMenu::HandleKey(XKeyEvent *event) {
  OMenuEntry *ptr;
  int  n, retc = 0;
  char tmp[10];
  KeySym keysym;
  XComposeStatus compose = { NULL, 0 };

  n = XLookupString(event, tmp, sizeof(tmp)-1, &keysym, &compose);
  tmp[n] = 0;

  //--- 1: Pass the key event recursively to any nested active popups...

  if (_current && _current->_type == MENU_POPUP && _popdown) {
    OXPopupMenu *p = _current->_popup;
    if (p) {
      retc = p->HandleKey(event);
      if (retc) return retc;
    }
  }

  //--- 2: Process key release events (hotkeys and return only)...

  if (event->type == KeyRelease) {
/**/
    for (ptr = _first; ptr != NULL; ptr = ptr->next) {
      if (event->keycode == ptr->_hkeycode) {
        Activate(ptr);
        if (ptr->_type == MENU_ENTRY && ptr->_status & MENU_ENABLE_MASK) {
          break;
        } else {
          return False; //True;
        }
      }
    }
/**/
    if (keysym == XK_Escape) { Activate(NULL); return 2; }

    if (keysym == XK_Return ||
        //keysym == XK_Execute ||
        keysym == XK_KP_Enter) ptr = _current;

    if (ptr && _hasgrab) {
      int ID = EndMenu();
      XUngrabPointer(GetDisplay(), CurrentTime);
      XUngrabKeyboard(GetDisplay(), CurrentTime);
      XSync(GetDisplay(), False);
      if (ptr) {
        ptr->_status &= ~MENU_ACTIVE_MASK;
        if (ptr->_status & MENU_ENABLE_MASK) {
          OMenuMessage message(MSG_MENU, MSG_CLICK, ID);
          SendMessage(_msgObject, &message);
        }
      }
    } else {
      if (ptr && ptr->_type == MENU_ENTRY &&
          ptr->_status & MENU_ENABLE_MASK) return 2;
      return False; //True;
    }

    return False;
  }

  //--- 3: Process key press events (arrows/home/end)...

  if (event->type == KeyPress) {
    for (ptr = _first; ptr != NULL; ptr = ptr->next) {
      if ((ptr->_status & MENU_ENABLE_MASK) &&
          (event->keycode == ptr->_hkeycode)) {
        Activate(ptr);
        return True;
      }
    }

    switch (keysym) {
      case XK_Left:
        retc = _popdown;
        _popdown = False;
        //Activate(NULL); //EndMenu();
        if (_current && _current->_type == MENU_POPUP) {
          OXPopupMenu *p = _current->_popup;
          if (p) p->EndMenu();
        }
        return retc;
        break;

      case XK_Right:
        if (!_popdown && _current && _current->_type == MENU_POPUP) {
          if (_current->_status & MENU_ENABLE_MASK) {
            Activate(_current, False);
            _popdown = True;
            OXPopupMenu *p = _current->_popup;
            if (p) p->Activate(p->_first);
            return True;
          }
        }
        return False;
        break;

      case XK_Up:
        if (_current) {
          for (ptr = _current->prev;
               ptr != NULL && ptr->_type == MENU_SEPARATOR;
               ptr = ptr->prev);
          if (ptr) Activate(ptr); else Activate(_last);
        } else {
          Activate(_last);
        }
        break;

      case XK_Down:
        if (_current) {
          for (ptr = _current->next;
               ptr != NULL && ptr->_type == MENU_SEPARATOR;
               ptr = ptr->next);
          if (ptr) Activate(ptr); else Activate(_first);
        } else {
          Activate(_first);
        }
        break;

      case XK_Home:
        Activate(_first);
        break;

      case XK_End:
        Activate(_last);
        break;

      case XK_Escape:
        break;

      default:
        return False;
    }

    return True;

  }
  
  return False;
}

void OXPopupMenu::Activate(OMenuEntry *entry, int delayed) {

//  if (entry == _current) return;

  //-- Deactivate the current entry

  if (_current && (_current != entry)) {
    if (entry == NULL && _current->_type == MENU_POPUP) return;
    if (_current->_type == MENU_POPUP) _current->_popup->EndMenu();
    _popdown = False;
    _current->_status &= ~MENU_ACTIVE_MASK;
    DrawEntry(_current);
  }

  if (_delay) { delete _delay; _delay = NULL; }

  //-- Activate the new one

  if (entry) {
    entry->_status |= MENU_ACTIVE_MASK;
    DrawEntry(entry);
    if ((entry->_type == MENU_POPUP) && (entry->_status & MENU_ENABLE_MASK)) {
      if (delayed) {
        _delay = new OTimer(this, 350);
      } else {
        int ax, ay;
        Window wdummy;

        XTranslateCoordinates(GetDisplay(), _id,
                              (_current->_popup->GetParent())->GetId(),
                              _w, _current->_ey, &ax, &ay, &wdummy);

        ODimension ps = _current->_popup->GetDefaultSize();

        if (ax + ps.w > _client->GetDisplayWidth())
          ax -= _w + ps.w - 7;

        if (ay + ps.h > _client->GetDisplayHeight())
          ay = _client->GetDisplayHeight() - ps.h - 3;

        _current->_popup->PlaceMenu(ax-3, ay-1, False, False);
        _popdown = True;
      }
    } else if (entry->_type == MENU_ENTRY) {
      // test...
      OMenuMessage message(MSG_MENU, MSG_SELECT, entry->_entryID);
      SendMessage(_msgObject, &message);
    }
  }

  _current = entry;
}

int OXPopupMenu::HandleTimer(OTimer *t) {
  if (t != _delay) return False;

  if (_current && 
      (_current->_type == MENU_POPUP) &&
      (_current->_status & MENU_ENABLE_MASK)) {
    int ax, ay;
    Window wdummy;

    XTranslateCoordinates(GetDisplay(), _id,
                          (_current->_popup->GetParent())->GetId(),
                          _w, _current->_ey, &ax, &ay, &wdummy);

    ODimension ps = _current->_popup->GetDefaultSize();

    if (ax + ps.w > _client->GetDisplayWidth())
      ax -= _w + ps.w - 7;

    if (ay + ps.h > _client->GetDisplayHeight())
      ay = _client->GetDisplayHeight() - ps.h - 3;

    _current->_popup->PlaceMenu(ax-3, ay-1, False, False);
    _popdown = True;
  }

  delete _delay;
  _delay = NULL;

  return True;
}

void OXPopupMenu::_DoRedraw() {
  OMenuEntry *ptr;
  
  OXFrame::_DoRedraw();

  if (_sidePic) {
    unsigned long fg = _normGC->GetForeground();
    _normGC->SetForeground(_sideBgnd);
    FillRectangle(_normGC->GetGC(), 3, 3, _sidePic->GetWidth(), _h - 6);

    _normGC->SetClipMask(_sidePic->GetMask());
    int clip_x = 3;
    int clip_y = _h - _sidePic->GetHeight() - 3;
    _normGC->SetClipXOrigin(clip_x);
    _normGC->SetClipYOrigin(clip_y);

    XCopyArea(GetDisplay(), _sidePic->GetPicture(), _id, _normGC->GetGC(),
              0, 0, _sidePic->GetWidth(), _sidePic->GetHeight(),
              clip_x, clip_y);

    _normGC->SetClipMask(None);
    _normGC->SetForeground(fg);
  }

  for (ptr = _first; ptr != NULL; ptr = ptr->next) {
    DrawEntry(ptr);
  }
}

void OXPopupMenu::DrawEntry(OMenuEntry *entry) {
  const char *lstr, *rstr;
  const OXFont *font;
  OFontMetrics fm;

  if (entry->_status & MENU_DEFAULT_MASK) {
    font = _hifont;
    _normGC->SetFont(font->GetId());
    _selGC->SetFont(font->GetId());
  } else {
    font = _font;
  }

  font->GetFontMetrics(&fm);

  int tx = entry->_ex + _xl;
  int tr = entry->_ex + _xr;
  int ty = entry->_ey + fm.ascent;
  int th = font->TextHeight() + 3;
  int px = entry->_ex + LEFT_GAP;
  int ew = _w - 6 - (_sidePic ? _sidePic->GetWidth() + 2 : 0);

  GC sGC  = _selGC->GetGC();
  GC sbGC = _selbackGC->GetGC();
  GC nGC  = _normGC->GetGC();

  switch (entry->_type) {
  case MENU_POPUP:
  case MENU_ENTRY:
      lstr = entry->_label->GetString();
      rstr = strchr(lstr, '\t');

      if (entry->_status & MENU_ACTIVE_MASK) {
        XFillRectangle(GetDisplay(), _id, sbGC,
                       entry->_ex+1, entry->_ey-1, ew, th);
        if (entry->_type == MENU_POPUP)
          DrawTrianglePattern(sGC, _w-10, entry->_ey+3, _w-6, entry->_ey+11);
        if (entry->_status & MENU_CHECKED_MASK)
          DrawCheckMark(sGC, 6, entry->_ey+3, 14, entry->_ey+11);
        if (entry->_status & MENU_RCHECKED_MASK)
          DrawRCheckMark(sGC, 6, entry->_ey+3, 14, entry->_ey+11);
        if (entry->_pic)
          entry->_pic->Draw(GetDisplay(), _id, sGC, px, entry->_ey+1);

        if (rstr) {
          entry->_label->Draw(GetDisplay(), _id,
                       (entry->_status & MENU_ENABLE_MASK) ? sGC : _shadowGC,
                       tx, ty, rstr - lstr);
          ++rstr;
          DrawString((entry->_status & MENU_ENABLE_MASK) ? sGC : _shadowGC,
                       tr, ty, rstr, strlen(rstr));
        } else {
          entry->_label->Draw(GetDisplay(), _id,
                       (entry->_status & MENU_ENABLE_MASK) ? sGC : _shadowGC,
                       tx, ty);
        }

      } else {
//        FillRectangle(_bckgndGC,
        ClearArea(entry->_ex+1, entry->_ey-1, ew, th, False);

        if (entry->_type == MENU_POPUP)
          DrawTrianglePattern(nGC, _w-10, entry->_ey+3, _w-6, entry->_ey+11);
        if (entry->_status & MENU_CHECKED_MASK)
          DrawCheckMark(nGC, 6, entry->_ey+3, 14, entry->_ey+11);
        if (entry->_status & MENU_RCHECKED_MASK)
          DrawRCheckMark(nGC, 6, entry->_ey+3, 14, entry->_ey+11);
        if (entry->_pic)
          entry->_pic->Draw(GetDisplay(), _id, nGC, px, entry->_ey+1);

        if (rstr) {
          if (entry->_status & MENU_ENABLE_MASK) {
            entry->_label->Draw(GetDisplay(), _id, nGC, tx, ty, rstr - lstr);
            ++rstr;
            DrawString(nGC, tr, ty, rstr, strlen(rstr));
          } else {
            entry->_label->Draw(GetDisplay(), _id, _hilightGC, tx+1, ty+1, rstr - lstr);
            entry->_label->Draw(GetDisplay(), _id, _shadowGC, tx, ty, rstr - lstr);
            ++rstr;
            DrawString(_hilightGC, tr+1, ty+1, rstr, strlen(rstr));
            DrawString(_shadowGC, tr, ty, rstr, strlen(rstr));
          }
        } else {
          if (entry->_status & MENU_ENABLE_MASK) {
            entry->_label->Draw(GetDisplay(), _id, nGC, tx, ty);
          } else {
            entry->_label->Draw(GetDisplay(), _id, _hilightGC, tx+1, ty+1);
            entry->_label->Draw(GetDisplay(), _id, _shadowGC, tx, ty);
          }
        }

      }
      break;

   case MENU_SEPARATOR:
      DrawLine(_shadowGC,  entry->_ex, entry->_ey, _w-3, entry->_ey);
      DrawLine(_hilightGC, entry->_ex, entry->_ey+1, _w-3, entry->_ey+1);
      break;
   }

  // restore font
  if (entry->_status & MENU_DEFAULT_MASK) {
    _normGC->SetFont(_font->GetId());
    _selGC->SetFont(_font->GetId());
  }
}

void OXPopupMenu::DrawBorder() {
  DrawLine(_bckgndGC, 0, 0, _w-2, 0);
  DrawLine(_bckgndGC, 0, 0, 0, _h-2);
  DrawLine(_hilightGC, 1, 1, _w-3, 1);
  DrawLine(_hilightGC, 1, 1, 1, _h-3);
    
  DrawLine(_shadowGC,  1, _h-2, _w-2, _h-2);
  DrawLine(_shadowGC,  _w-2, _h-2, _w-2, 1);
  DrawLine(_blackGC, 0, _h-1, _w-1, _h-1);
  DrawLine(_blackGC, _w-1, _h-1, _w-1, 0);
}

void OXPopupMenu::DrawTrianglePattern(GC gc, int l, int t, int r, int b) {
  XPoint points[3];

  int m = (t + b) >> 1;

  points[0].x = l;
  points[0].y = t;
  points[1].x = l;
  points[1].y = b;
  points[2].x = r;
  points[2].y = m;

  XFillPolygon(GetDisplay(), _id, gc, points, 3, Convex, CoordModeOrigin);
}

void OXPopupMenu::DrawCheckMark(GC gc, int l, int t, int r, int b) {
  XSegment seg[6];

  t = (t + b - 8) >> 1; ++t;

  seg[0].x1 = 1+l; seg[0].y1 = 3+t; seg[0].x2 = 3+l; seg[0].y2 = 5+t;
  seg[1].x1 = 1+l; seg[1].y1 = 4+t; seg[1].x2 = 3+l; seg[1].y2 = 6+t;
  seg[2].x1 = 1+l; seg[2].y1 = 5+t; seg[2].x2 = 3+l; seg[2].y2 = 7+t;
  seg[3].x1 = 3+l; seg[3].y1 = 5+t; seg[3].x2 = 7+l; seg[3].y2 = 1+t;
  seg[4].x1 = 3+l; seg[4].y1 = 6+t; seg[4].x2 = 7+l; seg[4].y2 = 2+t;
  seg[5].x1 = 3+l; seg[5].y1 = 7+t; seg[5].x2 = 7+l; seg[5].y2 = 3+t;

  XDrawSegments(GetDisplay(), _id, gc, seg, 6);
//  XCopyPlane(GetDisplay(), _checkmark, _id, gc, 0, 0, 13, 13, l, t, 1);
}

void OXPopupMenu::DrawRCheckMark(GC gc, int l, int t, int r, int b) {
  XSegment seg[5];

  t = (t + b - 5) >> 1; ++t;
  l = (l + r - 5) >> 1; ++l;

  seg[0].x1 = 1+l; seg[0].y1 = 0+t; seg[0].x2 = 3+l; seg[0].y2 = 0+t;
  seg[1].x1 = 0+l; seg[1].y1 = 1+t; seg[1].x2 = 4+l; seg[1].y2 = 1+t;
  seg[2].x1 = 0+l; seg[2].y1 = 2+t; seg[2].x2 = 4+l; seg[2].y2 = 2+t;
  seg[3].x1 = 0+l; seg[3].y1 = 3+t; seg[3].x2 = 4+l; seg[3].y2 = 3+t;
  seg[4].x1 = 1+l; seg[4].y1 = 4+t; seg[4].x2 = 3+l; seg[4].y2 = 4+t;

  XDrawSegments(GetDisplay(), _id, gc, seg, 5);
//  XCopyPlane(GetDisplay(), _radiomark, _id, gc, 0, 0, 12, 12, l, t, 1);
}


//---- set default entry (default entries are drawn with bold text)

void OXPopupMenu::SetDefaultEntry(int ID) {
  OMenuEntry *ptr;
  
  for (ptr = _first; ptr != NULL; ptr = ptr->next) {
    if (ptr->_entryID == ID)
      ptr->_status |= MENU_DEFAULT_MASK;
    else
      ptr->_status &= ~MENU_DEFAULT_MASK;
  }
}


//---- enable/disable entry (disabled entries appear in a sunken relief)

void OXPopupMenu::EnableEntry(int ID, int enabled) {
  OMenuEntry *ptr;

  if (!enabled) { DisableEntry(ID); return; }

  for (ptr = _first; ptr != NULL; ptr = ptr->next)
    if (ptr->_entryID == ID) { ptr->_status |= MENU_ENABLE_MASK; break; }
}

void OXPopupMenu::DisableEntry(int ID) {
  OMenuEntry *ptr;

  for (ptr = _first; ptr != NULL; ptr = ptr->next)
    if (ptr->_entryID == ID) { ptr->_status &= ~MENU_ENABLE_MASK; break; }
}

int OXPopupMenu::IsEntryEnabled(int ID) {
  OMenuEntry *ptr;

  for (ptr = _first; ptr != NULL; ptr = ptr->next)
    if (ptr->_entryID == ID) return (ptr->_status & MENU_ENABLE_MASK);
  return 0;
}


//---- check/uncheck entry

void OXPopupMenu::CheckEntry(int ID, int checked) {
  OMenuEntry *ptr;

  if (!checked) { UnCheckEntry(ID); return; }

  for (ptr = _first; ptr != NULL; ptr = ptr->next)
    if (ptr->_entryID == ID) { ptr->_status |= MENU_CHECKED_MASK; break; }
}

void OXPopupMenu::UnCheckEntry(int ID) {
  OMenuEntry *ptr;

  for (ptr = _first; ptr != NULL; ptr = ptr->next)
    if (ptr->_entryID == ID) { ptr->_status &= ~MENU_CHECKED_MASK; break; }
}

int OXPopupMenu::IsEntryChecked(int ID) {
  OMenuEntry *ptr;

  for (ptr = _first; ptr != NULL; ptr = ptr->next)
    if (ptr->_entryID == ID) return (ptr->_status & MENU_CHECKED_MASK);
  return 0;
}


//---- radio-select entry (note that they cannot be unselected,
//                         the selection must be moved to another entry instead)

void OXPopupMenu::RCheckEntry(int ID, int IDfirst, int IDlast) {
  OMenuEntry *ptr;

  for (ptr = _first; ptr != NULL; ptr = ptr->next)
    if (ptr->_entryID == ID)
      ptr->_status |= MENU_RCHECKED_MASK;
    else
      if (ptr->_entryID >= IDfirst && ptr->_entryID <= IDlast) 
        ptr->_status &= ~MENU_RCHECKED_MASK;
}

int OXPopupMenu::IsEntryRChecked(int ID) {
  OMenuEntry *ptr;

  for (ptr = _first; ptr != NULL; ptr = ptr->next)
    if (ptr->_entryID == ID) return (ptr->_status & MENU_RCHECKED_MASK);
  return 0;
}

void OXPopupMenu::Reconfig() {
  OMenuEntry *ptr;

  _backPixel = _defaultFrameBackground;
  OXFrame::Reconfig();
  SetBackgroundColor(_backPixel);

  _normGC->SetForeground(_client->GetResourcePool()->GetFrameFgndColor());
  _normGC->SetBackground(_client->GetResourcePool()->GetFrameBgndColor());

  _selGC->SetForeground(_client->GetResourcePool()->GetSelectedFgndColor());
  _selGC->SetBackground(_client->GetResourcePool()->GetSelectedBgndColor());

  _selbackGC->SetForeground(_client->GetResourcePool()->GetSelectedBgndColor());
  _selbackGC->SetBackground(_client->GetResourcePool()->GetSelectedFgndColor());

  for (ptr = _first; ptr != NULL; ptr = ptr->next)
    if (ptr->_type == MENU_POPUP) ptr->_popup->Reconfig();

  AdjustEntries();
  Resize(_mw, _mh);
}

//-----------------------------------------------------------------

OXMenuTitle::OXMenuTitle(const OXWindow *p, OHotString *s, OXPopupMenu *menu, 
	                 unsigned int options)
  : OXFrame(p, 1, 1, options) {
    int tw, hotchar;
 
    if (!_init) {
      _defaultGC = GetResourcePool()->GetFrameGC();
      _defaultSelGC = GetResourcePool()->GetSelectedGC();
      _defaultSelBckgndGC = GetResourcePool()->GetSelectedBckgndGC();

      _defaultFont = GetResourcePool()->GetMenuFont();

      _init = True;
    }

    _normGC    = (OXGC *) _defaultGC;          // ==!==
    _selGC     = (OXGC *) _defaultSelGC;       // ==!==
    _selbackGC = (OXGC *) _defaultSelBckgndGC; // ==!==
    _font      = _defaultFont;

    _label = s;
    _menu = menu;
    _state = BUTTON_UP;
    _ID = -1;

    if ((hotchar = s->GetHotChar()) != 0)
      _hkeycode = XKeysymToKeycode(GetDisplay(), hotchar);
    else
      _hkeycode = 0;

    tw = _font->TextWidth(_label->GetString(), _label->GetLength());
  
    Resize(tw + 8, _font->TextHeight() + 7);
} 

OXMenuTitle::~OXMenuTitle() {
  if (_normGC != _defaultGC) delete _normGC;
  if (_selGC != _defaultSelGC) delete _selGC;
  if (_selbackGC != _defaultSelBckgndGC) delete _selbackGC;
  if (_font != _defaultFont) _client->FreeFont((OXFont *) _font);
}

void OXMenuTitle::SetState(int state) {
  _state = state;
  if (state) {
    if (_menu) {
      int ax, ay, dpyw, dpyh;
      Window wdummy;

      XTranslateCoordinates(GetDisplay(), _id, (_menu->GetParent())->GetId(),
                            0, _h, &ax, &ay, &wdummy);

      ODimension ps = _menu->GetDefaultSize();

      dpyw = _client->GetDisplayWidth();
      dpyh = _client->GetDisplayHeight();

      if (ax + ps.w > dpyw) ax -= ps.w - _w - 2;
      if (ax + ps.w > dpyw) ax = dpyw - ps.w - 1;  // if still off-screen
      if (ay + ps.h > dpyh) ay -= ps.h + _h;

      // Place the menu just under the window:
      _menu->PlaceMenu(ax-1, ay, True, False);
    }
  } else {
    if (_menu) {
      _ID = _menu->EndMenu();
    }
  }
  _options &= ~(SUNKEN_FRAME | RAISED_FRAME);
  NeedRedraw();
}

void OXMenuTitle::_DoRedraw() {
  int x, y;
  OFontMetrics fm;

  _font->GetFontMetrics(&fm);

  OXFrame::_DoRedraw();
  x = 4; y = 4;
  if (_state) {
    XFillRectangle(GetDisplay(), _id, _selbackGC->GetGC(), 0, 0, _w, _h);
    _label->Draw(GetDisplay(), _id, _selGC->GetGC(), x, y + fm.ascent);
  } else {
    XClearWindow(GetDisplay(), _id);
    _label->Draw(GetDisplay(), _id, _normGC->GetGC(), x, y + fm.ascent);
  }
}

void OXMenuTitle::DoSendMessage() {
  if (_menu)
    if (_ID != -1) {
      OMenuMessage message(MSG_MENU, MSG_CLICK, _ID);
      SendMessage(_menu->_msgObject, &message);
    }
}

int OXMenuTitle::HandleKey(XKeyEvent *event) {
  if (_menu && _state) return _menu->HandleKey(event);
  return False;
}

int OXMenuTitle::HandleButton(XButtonEvent *event) {
  if (_menu && _state) return _menu->HandleButton(event);
  return False;
}

int OXMenuTitle::HandleMotion(XMotionEvent *event) {
  if (_menu && _state) {
    XMotionEvent ev = *event;
    ev.x = ev.x_root - _menu->GetX();
    ev.y = ev.y_root - _menu->GetY();
    ev.window = _menu->GetId();
    return _menu->HandleMotion(&ev);
  }
  return False;
}

void OXMenuTitle::Reconfig() {
  OXFrame::Reconfig();
  if (_menu) _menu->Reconfig();
}
