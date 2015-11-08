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

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <sys/time.h>
   
#include <X11/keysym.h>

#include <xclass/utils.h>
#include <xclass/OXClient.h>
#include <xclass/OXMainFrame.h>
#include <xclass/OFocusManager.h>
#include <xclass/OResourcePool.h>
#include <xclass/ODNDmanager.h>
#include <xclass/OXButton.h>
#include <xclass/OGC.h>


//----------------------------------------------------------------

OXMainFrame::OXMainFrame(const OXWindow *p, int w, int h,
                         unsigned long options) :
  OXCompositeFrame(p, w, h, options | MAIN_FRAME) {

    XSetWMProtocols(GetDisplay(), _id, &WM_DELETE_WINDOW, 1);

    _toplevel = this;
    _sizeHints.flags = 0;

    _bindlist = new OXSList(GetDisplay(), "Key bindings");
    _tlist = new OXSList(GetDisplay(), "Transients");
    _buttonlist = new OXSList(GetDisplay(), "Buttons");

    _currentAccept = _defaultAccept = _defaultCancel = NULL;

    _client->RegisterTopLevel(this);
    _focusMgr = new OFocusManager(_client, this);
    _dndManager = NULL;

    int keycode = XKeysymToKeycode(GetDisplay(), XK_Tab);
    XGrabKey(GetDisplay(), keycode, AnyModifier, _id, True,
             GrabModeAsync, GrabModeAsync);

    AddInput(FocusChangeMask);
    _flags &= ~TAKES_FOCUS;
} 

OXMainFrame::~OXMainFrame() {

  if (_tlist) {
    const OXSNode *e = _tlist->GetHead(), *next;
    OXWindow *w;

    while (e) {
      next = e->next;
      w = (OXWindow *) e->data;
      delete w;
      e = next;
    }
    delete _tlist;
  }

  int keycode = XKeysymToKeycode(GetDisplay(), XK_Tab);
  XUngrabKey(GetDisplay(), keycode, AnyModifier, _id);

  if (_focusMgr) delete _focusMgr;
  if (_dndManager) delete _dndManager;
  DestroyWindow();
  XSync(GetDisplay(), False);
  if (_bindlist) delete _bindlist;
  _bindlist = NULL; // =!= Deletion of buttons and other widgets during
                    // automatic OXCompositeFrame cleanup can cause
                    // trouble because the _bindlist pointer is no longer
                    // valid. Making it NULL will keep RemoveBind happy.
  if (_buttonlist) delete _buttonlist;
  _buttonlist = NULL;
  _client->UnregisterTopLevel(this);
}

int OXMainFrame::HandleKey(XKeyEvent *event) {
  OXSNode *e = NULL;
  OXFrame *f, *bindf = NULL;
  int retc;

  if (_bindlist) {
    e = _bindlist->GetNode(event->keycode);
    if (e) bindf = (OXFrame *) e->data;
  }

//  if (event->type == KeyPress)
//    XAutoRepeatOff(GetDisplay());
//  else
//    XAutoRepeatOn(GetDisplay());

  if ((XLookupKeysym(event, 0) == XK_Return) ||
      (XLookupKeysym(event, 0) == XK_KP_Enter)) {
    if (_currentAccept) {
      event->keycode = XKeysymToKeycode(GetDisplay(), 32);
      int takesFocus = _currentAccept->TakesFocus();
      _currentAccept->TakeFocus(False);
      _currentAccept->HandleKey(event);
      if (_currentAccept) _currentAccept->TakeFocus(takesFocus);
      return True;
    }
  }

  if (XLookupKeysym(event, 0) == XK_Escape) {
    if (_defaultCancel) {
      event->keycode = XKeysymToKeycode(GetDisplay(), 32);
      int takesFocus = _defaultCancel->TakesFocus();
      _defaultCancel->TakeFocus(False);
      _defaultCancel->HandleKey(event);
      if (_defaultCancel) _defaultCancel->TakeFocus(takesFocus);
      return True;
    }
  }

  if (_focusMgr && (event->type == KeyPress)) {
    if (XLookupKeysym(event, 0) == XK_Tab) {
      if (event->state == ShiftMask) {
        if (!_focusMgr->FocusPrevious())
          _focusMgr->FocusBackward(this);
      } else {
        if (bindf && bindf->HandlesTab())
          retc = bindf->HandleKey(event);
        if (!bindf || !retc) {
          f = _focusMgr->GetFocusOwner();
          if (f && f->HandlesTab()) {
            f->HandleKey(event);
          } else {
            if (!_focusMgr->FocusNext())
              _focusMgr->FocusForward(this);
          }
        }
      }
      return True;
    }
  }

  if (bindf && bindf->IsVisible() && bindf->IsEnabled())
    return bindf->HandleKey(event);

  return False;
}

int OXMainFrame::RegisterTransient(OXWindow *w) {
  return (_tlist->Add(w->GetId(), (XPointer) w) != NULL);
}
  
int OXMainFrame::UnregisterTransient(OXWindow *w) {
  return (_tlist->Remove(w->GetId()));
}

int OXMainFrame::RegisterButton(OXButton *b) {
  return (_buttonlist->Add(b->GetId(), (XPointer) b) != NULL);
}

int OXMainFrame::UnregisterButton(OXButton *b) {
  if (_currentAccept == b) _currentAccept = NULL;
  if (_defaultAccept == b) _defaultAccept = NULL;
  if (_defaultCancel == b) _defaultCancel = NULL;
  return (_buttonlist ? _buttonlist->Remove(b->GetId()) : False);
}

void OXMainFrame::SetDefaultAcceptButton(OXButton *b) {
  int keycode;

  if (_defaultAccept) {
    _defaultAccept->SetDefault(False);
    keycode = XKeysymToKeycode(GetDisplay(), XK_Return);
    XUngrabKey(GetDisplay(), keycode, AnyModifier, _id);
    keycode = XKeysymToKeycode(GetDisplay(), XK_KP_Enter);
    XUngrabKey(GetDisplay(), keycode, AnyModifier, _id);
  }

  if (_defaultAccept == _currentAccept) _currentAccept = b;
  _defaultAccept = b;

  if (_defaultAccept && (_defaultAccept == _currentAccept)) {
    _defaultAccept->SetDefault();
    keycode = XKeysymToKeycode(GetDisplay(), XK_Return);
    XGrabKey(GetDisplay(), keycode, AnyModifier, _id, True,
             GrabModeAsync, GrabModeAsync);
    keycode = XKeysymToKeycode(GetDisplay(), XK_KP_Enter);
    XGrabKey(GetDisplay(), keycode, AnyModifier, _id, True,
             GrabModeAsync, GrabModeAsync);
  }
}

void OXMainFrame::SetDefaultCancelButton(OXButton *b) {
  if (_defaultCancel) {
    int keycode = XKeysymToKeycode(GetDisplay(), XK_Escape);
    XUngrabKey(GetDisplay(), keycode, AnyModifier, _id);
  }
  _defaultCancel = b;
  if (_defaultCancel) {
    int keycode = XKeysymToKeycode(GetDisplay(), XK_Escape);
    XGrabKey(GetDisplay(), keycode, AnyModifier, _id, True,
             GrabModeAsync, GrabModeAsync);
  }
}

int OXMainFrame::BindKey(const OXWindow *w, int keycode, int modifier) const {
  if (_bindlist)
    if (_bindlist->Add(keycode, (XPointer) w) != NULL) {
      XGrabKey(GetDisplay(), keycode, modifier, _id, True,
               GrabModeAsync, GrabModeAsync);
      return True;
    }
  return False;
}

void OXMainFrame::RemoveBind(const OXWindow *w, int keycode, int modifier) const {
  if (_bindlist) {
    _bindlist->Remove(keycode);
    XUngrabKey(GetDisplay(), keycode, modifier, _id);
  }
}

void OXMainFrame::SetFocusOwner(OXFrame *f) {
  if (_focusMgr)
    _focusMgr->SetFocusOwner(f);

  if (_currentAccept) {
    _currentAccept->SetDefault(False);
    int keycode = XKeysymToKeycode(GetDisplay(), XK_Return);
    XUngrabKey(GetDisplay(), keycode, AnyModifier, _id);
    keycode = XKeysymToKeycode(GetDisplay(), XK_KP_Enter);
    XUngrabKey(GetDisplay(), keycode, AnyModifier, _id);
  }

  const OXSNode *e = f ? _buttonlist->GetNode(f->GetId()) : NULL;
  if (e)
    _currentAccept = (OXButton *) e->data;
  else
    _currentAccept = _defaultAccept;

  if (_currentAccept) {
    _currentAccept->SetDefault();
    int keycode = XKeysymToKeycode(GetDisplay(), XK_Return);
    XGrabKey(GetDisplay(), keycode, AnyModifier, _id, True,
             GrabModeAsync, GrabModeAsync);
    keycode = XKeysymToKeycode(GetDisplay(), XK_KP_Enter);
    XGrabKey(GetDisplay(), keycode, AnyModifier, _id, True,
             GrabModeAsync, GrabModeAsync);
  }

}

int OXMainFrame::HandleConfigureNotify(XConfigureEvent *event) {
  if (event->send_event) {
    _x = event->x;
    _y = event->y;
    _Moved();
  }
  OXCompositeFrame::HandleConfigureNotify(event);
  return True;
}

int OXMainFrame::HandleClientMessage(XClientMessageEvent *event) {

  if (_dndManager) {
    if (_dndManager->HandleClientMessage(event)) return True;
  }

  if ((event->format == 32) && (event->data.l[0] == WM_DELETE_WINDOW)) {
    CloseWindow();
    return True;
  }

  return OXCompositeFrame::HandleClientMessage(event);
}

int OXMainFrame::HandleSelection(XSelectionEvent *event) {
  if (event->selection == ODNDmanager::DNDselection) {
    if (_dndManager)
      return _dndManager->HandleSelection(event);
  }
  return OXCompositeFrame::HandleSelection(event);
}

int OXMainFrame::HandleSelectionRequest(XSelectionRequestEvent *event) {
  if (event->selection == ODNDmanager::DNDselection) {
    if (_dndManager)
      return _dndManager->HandleSelectionRequest(event);
  }
  return OXCompositeFrame::HandleSelectionRequest(event);
}

int OXMainFrame::HandleSelectionClear(XSelectionClearEvent *event) {
  return OXCompositeFrame::HandleSelectionClear(event);
}


// Override this to intercept close.
// The function returns True if the window was successfully closed,
// False otherwise.

int OXMainFrame::CloseWindow() {
  delete this;  // note that the destructor calls DestroyWindow()
  return True;
}

void OXMainFrame::SetWindowName(const char *Name) {
  XTextProperty wname;

  if (XStringListToTextProperty((char **) &Name, 1, &wname) == 0) {
    Debug(DBG_ERR, "OXMainFrame: Cannot allocate window name!\n");
    return;
  }
  XSetWMName(GetDisplay(), _id, &wname);
  XFree(wname.value);
}

void OXMainFrame::SetIconName(const char *Name) {
  XTextProperty wname;

  if (XStringListToTextProperty((char **) &Name, 1, &wname) == 0) {
    Debug(DBG_ERR, "OXMainFrame: Cannot allocate icon name!\n");
    return;
  }
  XSetWMIconName(GetDisplay(), _id, &wname);
  XFree(wname.value);
}

void OXMainFrame::SetClassHints(const char *ClassName,
                                const char *ResourceName) {
  XClassHint class_hints;
 
  class_hints.res_class = (char *) ClassName;
  class_hints.res_name  = (char *) ResourceName;
  XSetClassHint(GetDisplay(), _id, &class_hints);
}


// Set decoration style for MWM-compatible wm (mwm, ncdwm, fvwm?).
// Shall we check first for the presence of MWM_RUNNING property?

void OXMainFrame::SetMWMHints(unsigned int value, unsigned int funcs, unsigned int input) {
  MWMHintsProperty prop;

  prop.decorations = value;
  prop.functions   = funcs;
  prop.inputMode   = input;
  prop.flags       = MWM_HINTS_DECORATIONS | 
                     MWM_HINTS_FUNCTIONS   |
                     MWM_HINTS_INPUT_MODE;

  XChangeProperty(GetDisplay(), _id, _MOTIF_WM_HINTS, _MOTIF_WM_HINTS, 32,
                  PropModeReplace, (unsigned char *)&prop, PROP_MWM_HINTS_ELEMENTS);
}

void OXMainFrame::SetWMPosition(int x, int y) {

  _sizeHints.flags |= USPosition | PPosition;
  _sizeHints.x = x;
  _sizeHints.y = y;

  XSetWMNormalHints(GetDisplay(), _id, &_sizeHints);
}

void OXMainFrame::SetWMSize(int w, int h) {

  _sizeHints.flags |= USSize | PSize | PBaseSize;
  _sizeHints.width  = _sizeHints.base_width  = w;
  _sizeHints.height = _sizeHints.base_height = h;

  XSetWMNormalHints(GetDisplay(), _id, &_sizeHints);
}

void OXMainFrame::SetWMMinSize(int wmin, int hmin) {

  _sizeHints.flags |= PMinSize;
  _sizeHints.min_width  = wmin;
  _sizeHints.min_height = hmin;

  XSetWMNormalHints(GetDisplay(), _id, &_sizeHints);
}

void OXMainFrame::SetWMMaxSize(int wmax, int hmax) {

  _sizeHints.flags |= PMaxSize;
  _sizeHints.max_width  = wmax;
  _sizeHints.max_height = hmax;

  XSetWMNormalHints(GetDisplay(), _id, &_sizeHints);
}

void OXMainFrame::SetWMResizeIncrement(int winc, int hinc) {

  _sizeHints.flags |= PResizeInc;
  _sizeHints.width_inc  = winc;
  _sizeHints.height_inc = hinc;

  XSetWMNormalHints(GetDisplay(), _id, &_sizeHints);
}

void OXMainFrame::SetWMSizeHints(int wmin, int hmin, int wmax, int hmax,
                                 int winc, int hinc) {

  _sizeHints.flags |= PMinSize | PMaxSize | PResizeInc;
  _sizeHints.min_width  = wmin;
  _sizeHints.max_width  = wmax;
  _sizeHints.min_height = hmin;
  _sizeHints.max_height = hmax;
  _sizeHints.width_inc  = winc;
  _sizeHints.height_inc = hinc;

  XSetWMNormalHints(GetDisplay(), _id, &_sizeHints);
}

void OXMainFrame::SetWMGravity(int gravity) {

  _sizeHints.flags |= PWinGravity;
  _sizeHints.win_gravity = gravity;

  XSetWMNormalHints(GetDisplay(), _id, &_sizeHints);
}

void OXMainFrame::SetWMState(int state) {
  XWMHints hints;

  hints.flags = StateHint;
  hints.initial_state = state;

  XSetWMHints(GetDisplay(), _id, &hints);
}

void OXMainFrame::_GotFocus() {
  if (_focusMgr) _focusMgr->FocusCurrent();
}

void OXMainFrame::Reconfig() {
  const OResourcePool *res = GetResourcePool();

  _whitePixel = res->GetWhiteColor();
  _blackPixel = res->GetBlackColor();
  _defaultFrameBackground = res->GetFrameBgndColor();
  _defaultFrameHilite = res->GetFrameHiliteColor();
  _defaultFrameShadow = res->GetFrameShadowColor();
  _defaultSelectedBackground = res->GetSelectedBgndColor();
  _defaultDocumentBackground = res->GetDocumentBgndColor();
  _defaultSelectedForeground = res->GetSelectedFgndColor();
  _defaultDocumentForeground = res->GetDocumentFgndColor();

  _defaultBackgroundPicture = res->GetFrameBckgndPicture();

  _blackGC = res->GetBlackGC()->GetGC();
  _whiteGC = res->GetWhiteGC()->GetGC();
  _hilightGC = res->GetFrameHiliteGC()->GetGC();
  _shadowGC = res->GetFrameShadowGC()->GetGC();
  _bckgndGC = res->GetFrameBckgndGC()->GetGC();

  OXCompositeFrame::Reconfig();
}
