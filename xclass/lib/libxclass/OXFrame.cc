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
   
#include <xclass/utils.h>
#include <xclass/OXClient.h>
#include <xclass/OPicture.h>
#include <xclass/OXFrame.h>
#include <xclass/OXMainFrame.h>
#include <xclass/OXTip.h>
#include <xclass/OLayout.h>
#include <xclass/OResourcePool.h>
#include <xclass/OFocusManager.h>
#include <xclass/OGC.h>


// These are needed for processing double-clicks

Time OXFrame::_lastclick = 0;
unsigned int OXFrame::_lastbutton = 0;
unsigned int OXFrame::_dbx = 0;
unsigned int OXFrame::_dby = 0;
unsigned int OXFrame::_dbw = 0;
unsigned int OXFrame::_clickcount = 0;

const OPicture *OXFrame::_defaultBackgroundPicture;
unsigned long OXFrame::_defaultFrameBackground;
unsigned long OXFrame::_defaultFrameHilite;
unsigned long OXFrame::_defaultFrameShadow;
unsigned long OXFrame::_defaultSelectedBackground;
unsigned long OXFrame::_defaultSelectedForeground;
unsigned long OXFrame::_defaultDocumentBackground;
unsigned long OXFrame::_defaultDocumentForeground;
unsigned long OXFrame::_whitePixel;
unsigned long OXFrame::_blackPixel;
GC OXFrame::_blackGC;
GC OXFrame::_whiteGC;
GC OXFrame::_hilightGC;
GC OXFrame::_shadowGC;
GC OXFrame::_bckgndGC;

int OXFrame::_init = False;

//----------------------------------------------------------------

OXFrame::OXFrame(const OXWindow *p, int w, int h,
                 unsigned int options, unsigned long back) : 
  OXWindow(p, 0, 0, w, h, 0, CopyFromParent, CopyFromParent,
           CopyFromParent, 0, NULL) {

    XSetWindowAttributes wattr;
    unsigned long mask;

    if (!_init) {
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

      back = _defaultFrameBackground; // hmmm... perhaps we should take
                                      // 'back' out of the param list to
                                      // the constructor
      _init = True;
    }

    _options = options;
    _backPixel = back;
    _hilitePixel = _defaultFrameHilite;
    _shadowPixel = _defaultFrameShadow;
    _flags = IS_ENABLED | IS_VISIBLE;

    _w = w; _h = h; _x = _y = 0;

    _ComputeInsets();

    mask = CWBackPixel | CWEventMask;
    wattr.background_pixel = _backPixel;
    wattr.event_mask = ExposureMask;
    if (_options & MAIN_FRAME) {
      wattr.event_mask |= StructureNotifyMask;
      XChangeWindowAttributes(GetDisplay(), _id, mask, &wattr); 
      if (_defaultBackgroundPicture)
        SetBackgroundPixmap(_defaultBackgroundPicture->GetPicture());
    } else {
      XChangeWindowAttributes(GetDisplay(), _id, mask, &wattr); 
#if 0
      if (_defaultBackgroundPicture && !(_options & OWN_BKGND))
#else
      if (!(_options & OWN_BKGND))
#endif
        SetBackgroundPixmap(ParentRelative);
    }

    _eventMask = wattr.event_mask;
    _compressConfigureEvents = True;
    _compressMotionEvents = True;

    _tip = NULL;

    _insets = OInsets(_bw, _bw, _bw, _bw);
}

OXFrame::OXFrame(OXClient *c, Window id, OXWindow *parent) :
  OXWindow(c, id, parent) {

  if (id != None) {
    XWindowAttributes wattr;

    XGetWindowAttributes(GetDisplay(), id, &wattr);
    _x = wattr.x;
    _y = wattr.y;
    _w = wattr.width;
    _h = wattr.height;
    _backPixel = wattr.backing_pixel;
  } else {
    _x = _y = 0;
    _w = _h = 1;
    _backPixel = 0;
  }

  _flags = IS_VISIBLE;
  _options = 0;
  _tip = NULL;
  _eventMask = 0;

  _ComputeInsets();
}

OXFrame::~OXFrame() {
  if (_tip) delete _tip;
}

void OXFrame::ChangeOptions(unsigned int options) {
  _options = options;
  _OptionsChanged(); 
}
 
void OXFrame::SetOptions(unsigned int options) {
  _options |= options;
  _OptionsChanged();  
}
 
void OXFrame::ClearOptions(unsigned int options) {
  _options &= ~options;
  _OptionsChanged();   
}
 
void OXFrame::_OptionsChanged() {
  _ComputeInsets();
}
 
void OXFrame::SetBorderStyle(unsigned int border_bits) {
  _options &= ~(DOUBLE_BORDER | SUNKEN_FRAME | RAISED_FRAME);
  _options |= (border_bits & (DOUBLE_BORDER | SUNKEN_FRAME | RAISED_FRAME));
  _OptionsChanged();
  NeedRedraw();
}
 
void OXFrame::AddInput(unsigned long emask) {
  //if (_eventMask & emask) return; // avoid unnecessary calls to XSelectInput
  _eventMask |= emask;
  if (_eventMask & FocusChangeMask) _flags |= TAKES_FOCUS;
  if (_eventMask & SubstructureNotifyMask) _compressConfigureEvents = False;
  SelectInput(_eventMask);
}

void OXFrame::RemoveInput(unsigned long emask) {
  _eventMask &= ~emask;
  if (!(_eventMask & FocusChangeMask)) _flags &= ~TAKES_FOCUS;
  SelectInput(_eventMask);
}

void OXFrame::DrawBorder() {
  _Draw3dRectangle(_options & (SUNKEN_FRAME | RAISED_FRAME | DOUBLE_BORDER),
                   0, 0, _w, _h);
}

void OXFrame::_Draw3dRectangle(int type, int x, int y, int w, int h) {
  switch (type) {
  case SUNKEN_FRAME:
    DrawLine(_shadowGC,  x,     y,     x+w-2, y);
    DrawLine(_shadowGC,  x,     y,     x,     y+h-2);
    DrawLine(_hilightGC, x,     y+h-1, x+w-1, y+h-1);
    DrawLine(_hilightGC, x+w-1, y+h-1, x+w-1, y);
    break;

  case SUNKEN_FRAME | DOUBLE_BORDER:
    DrawLine(_shadowGC,  x,     y,     x+w-2, y);
    DrawLine(_shadowGC,  x,     y,     x,     y+h-2);
    DrawLine(_blackGC,   x+1,   y+1,   x+w-3, y+1);
    DrawLine(_blackGC,   x+1,   y+1,   x+1,   y+h-3);

    DrawLine(_hilightGC, x,     y+h-1, x+w-1, y+h-1);
    DrawLine(_hilightGC, x+w-1, y+h-1, x+w-1, y);
    DrawLine(_bckgndGC,  x+1,   y+h-2, x+w-2, y+h-2);
    DrawLine(_bckgndGC,  x+w-2, y+1,   x+w-2, y+h-2);
    break;

  case RAISED_FRAME:
    DrawLine(_hilightGC, x,     y,     x+w-2, y);
    DrawLine(_hilightGC, x,     y,     x,     y+h-2);
    DrawLine(_shadowGC,  x,     y+h-1, x+w-1, y+h-1);
    DrawLine(_shadowGC,  x+w-1, y+h-1, x+w-1, y);
    break;

  case RAISED_FRAME | DOUBLE_BORDER:
    DrawLine(_hilightGC, x,     y,     x+w-2, y);
    DrawLine(_hilightGC, x,     y,     x,     y+h-2);
    DrawLine(_bckgndGC,  x+1,   y+1,   x+w-3, y+1);
    DrawLine(_bckgndGC,  x+1,   y+1,   x+1,   y+h-3);

    DrawLine(_shadowGC,  x+1,   y+h-2, x+w-2, y+h-2);
    DrawLine(_shadowGC,  x+w-2, y+h-2, x+w-2, y+1);
    DrawLine(_blackGC,   x,     y+h-1, x+w-1, y+h-1);
    DrawLine(_blackGC,   x+w-1, y+h-1, x+w-1, y);
    break;

  default: break;
  }
}

void OXFrame::_ComputeInsets() {
  if (_options & (SUNKEN_FRAME | RAISED_FRAME))
    _bw = (_options & DOUBLE_BORDER) ? 2 : 1;
  else
    _bw = 0;
  _insets = OInsets(_bw, _bw, _bw, _bw);
}

void OXFrame::_DoRedraw() {
//  ClearWindow();
  if (_clearBgnd)
    ClearArea(_bw, _bw, _w - (_bw << 1), _h - (_bw << 1));
  // border will only be drawn if we have a 3D option hint
  // (RAISED_FRAME or SUNKEN_FRAME)
  DrawBorder();
}

void OXFrame::_GotFocus() {
  if (_toplevel) ((OXMainFrame *)_toplevel)->SetFocusOwner(this);
}

int OXFrame::HandleConfigureNotify(XConfigureEvent *event) {
  // this generated some oscillations...
  // Resize(event->width, event->height);
  // ...do this instead:
  if ((event->width != _w) || (event->height != _h)) {
    _w = event->width;
    _h = event->height;
    _Resized();
    Layout();
  }
  return True;
}

int OXFrame::HandleEvent(XEvent *event) {

  switch (event->xany.type) {

  case Expose:
    HandleExpose((XExposeEvent *) event);
    break;
  
  case GraphicsExpose:
    HandleGraphicsExpose((XGraphicsExposeEvent *) event);
    break;
  
  case ConfigureNotify:
    if (_compressConfigureEvents)
      while (XCheckTypedWindowEvent(GetDisplay(), _id, ConfigureNotify, event));
    HandleConfigureNotify((XConfigureEvent *) event);
    break;

  case CreateNotify:
    HandleCreateNotify((XCreateWindowEvent *) event);
    break;

  case DestroyNotify:
    HandleDestroyNotify((XDestroyWindowEvent *) event);
    break;

  case MapNotify:
    HandleMapNotify((XMapEvent *) event);
    break;

  case UnmapNotify:
    HandleUnmapNotify((XUnmapEvent *) event);
    break;

  case KeyRelease:
    //if (_smartAutoRepeat)
    if (XPending(GetDisplay())) {
      XEvent ev;
      XPeekEvent(GetDisplay(), &ev);
      if (ev.xany.type == KeyPress &&
          ev.xkey.keycode == event->xkey.keycode)
        break;
    }
  case KeyPress:
    HandleKey((XKeyEvent *) event);
    break;
  
  case FocusIn:
  case FocusOut:
    HandleFocusChange((XFocusChangeEvent *) event);
    break;

  case ButtonPress:
    if ((event->xbutton.time - _lastclick < 350) &&
        (event->xbutton.button == _lastbutton) &&
        (abs(event->xbutton.x_root - _dbx) < 3) &&
        (abs(event->xbutton.y_root - _dby) < 3) &&
        (event->xbutton.window == _dbw))
      ++_clickcount;
    else
      _clickcount = 1;

    if (_clickcount > 3) _clickcount = 3;

    _lastclick = event->xbutton.time;
    _lastbutton = event->xbutton.button;
    _dbx = event->xbutton.x_root;
    _dby = event->xbutton.y_root;
    _dbw = event->xbutton.window;

    if (_clickcount == 3) {
      if (!HandleTripleClick((XButtonEvent *) event))
        HandleButton((XButtonEvent *) event);
    } else if (_clickcount == 2) {
      if (!HandleDoubleClick((XButtonEvent *) event))
        HandleButton((XButtonEvent *) event);
    } else {
      HandleButton((XButtonEvent *) event);
    }
    break;

  case ButtonRelease:
    HandleButton((XButtonEvent *) event);
    break;

  case EnterNotify:
  case LeaveNotify:
    HandleCrossing((XCrossingEvent *) event);
    break;

  case MotionNotify:
    if (_compressMotionEvents)
      while (XCheckTypedWindowEvent(GetDisplay(), _id, MotionNotify, event));
    HandleMotion((XMotionEvent *) event);
    break;

  case ClientMessage:
    HandleClientMessage((XClientMessageEvent *) event);
    break;

  case SelectionNotify:
    HandleSelection((XSelectionEvent *) event);
    break;

  case SelectionRequest:
    HandleSelectionRequest((XSelectionRequestEvent *) event);
    break;

  case SelectionClear:
    HandleSelectionClear((XSelectionClearEvent *) event);
    break;

  case PropertyNotify:
    HandlePropertyChange((XPropertyEvent *) event);
    break;

  default:
    Debug(DBG_EVENT, "OXFrame: Unknown event %#x for window %#x\n",
          event->xany.type, _id);
    break;
  }

  return True;
}

void OXFrame::Move(int x, int y) {
  if (x != _x || y != _y) {
    OXWindow::Move(x, y);
    _x = x; _y = y;
    _Moved();
  }
}

void OXFrame::Resize(int w, int h) {
  if (w != _w || h != _h) {
    OXWindow::Resize(w, h);
    _w = w; _h = h;
    _Resized();
    Layout();
  }
}

void OXFrame::Resize(ODimension size) {
  Resize(size.w, size.h);
}

void OXFrame::MoveResize(int x, int y, int w, int h) {
  // we do it anyway as we don't know if it's only a move or only a resize
  OXWindow::MoveResize(x, y, w, h);
#if 1  // OLayout wants this...
  _x = x; _y = y;
  _w = w; _h = h;
  _Moved();
  _Resized();
  Layout();
#else
  if (x != _x || y != _y) {
    _x = x; _y = y;
    _Moved();
  }
  if (w != _w || h != _h) {
    _w = w; _h = h;
    _Resized();
    Layout();
  }
#endif
}

int OXFrame::HandleClientMessage(XClientMessageEvent *event) {

  if (event->message_type == _XCLASS_MESSAGE) {
    //OMessage msg(GET_MSG(event->data.l[0]),
    //             GET_SUBMSG(event->data.l[0]),
    //             event->data.l[1], event->data.l[2]);
    //ProcessMessage(&msg);
    return True;
  }

  return False;
}

int OXFrame::HandleFocusChange(XFocusChangeEvent *event) {

  if ((event->detail != NotifyPointer) &&
      (event->detail != NotifyVirtual) &&
      (event->detail != NotifyNonlinearVirtual)) {
    if (event->type == FocusIn) {
      _flags |= HAS_FOCUS;
      _GotFocus();
    } else {
      _flags &= ~HAS_FOCUS;
      if (_windowExists) _LostFocus();
    }
    return True;
  }

  return False;
}

int OXFrame::HandleCrossing(XCrossingEvent *event) {

  if (_tip) {
    if ((event->type == EnterNotify) && 
        (!(event->state & (Button1Mask | Button2Mask |
                           Button3Mask | Button4Mask | Button5Mask)))) {
      if (_client->GetTipStatus() == 2)
        ShowTip();
      _client->StartTip(this);
    } else {
      HideTip();
    }
    return True;
  } else
    return False;
}

int OXFrame::HandleButton(XButtonEvent *event) {
  if (_tip) {
    if (event->type == ButtonPress) {
      HideTip();
    }
    return True;
  } else
    return False;
}

int OXFrame::HandleDestroyNotify(XDestroyWindowEvent *event) {
  if (event->window == _id) {
    _windowExists = False;
    _needRedraw = False;
  }
  return True;
}

void OXFrame::SetTip(char *text) {
#if 0
  if (_tip) delete _tip;
  _tip = new OXTip(_client->GetRoot(), new OString(text));
#else
  if (_tip)
    _tip->SetText(new OString(text));
  else
    _tip = new OXTip(_client->GetRoot(), new OString(text));
#endif
  AddInput(EnterWindowMask | LeaveWindowMask); 
}

void OXFrame::RemoveTip() {
  if (_tip) delete _tip;
  _tip = NULL;
}

void OXFrame::ShowTip() {
  int x, y, tx, ty;
  Window wtarget;
  ODimension tipsz = _tip->GetDefaultSize();

  XTranslateCoordinates(GetDisplay(), _id, _tip->GetParent()->GetId(),
                        0, 0, &x, &y, &wtarget);

  // Try to position it frist below the frame, if it is not fully visible
  // there, then try above. Do a similar thing for left and right

  ty = y + _h + 4;
  if (ty + tipsz.h > _client->GetDisplayHeight())
    ty = y - tipsz.h - 4;

  tx = x - 4;
  if (tx + tipsz.w > _client->GetDisplayWidth()) 
    tx = _client->GetDisplayWidth() - tipsz.w - 3;
  if (tx < 3) tx = 3;

  _tip->Show(tx, ty);
}

void OXFrame::HideTip() {
  _tip->Hide();
  _client->StopTip(this);
}

int OXFrame::IsVisible() const {
  if (!(_flags & IS_VISIBLE)) return False;
  const OXWindow *p = GetParent();
  if (p && (p != _toplevel) && (p != _client->GetRoot()))
    return ((OXFrame *)p)->IsVisible();
  return True;
}

void OXFrame::Reconfig() {

  if (!(_options & OWN_BKGND)) {
    _backPixel = _defaultFrameBackground;
    _hilitePixel = _defaultFrameHilite;
    _shadowPixel = _defaultFrameShadow;
  }

  if (_options & MAIN_FRAME) {
    SetBackgroundColor(_backPixel);
    if (_defaultBackgroundPicture) {
      SetBackgroundPixmap(_defaultBackgroundPicture->GetPicture());
    } else {
      SetBackgroundPixmap(None);
      SetBackgroundColor(_backPixel);
    }
  } else {
    if (_options & OWN_BKGND) {
      //SetBackgroundColor(_backPixel);
    } else {
#if 0
      if (_defaultBackgroundPicture) {
        SetBackgroundPixmap(ParentRelative);
      } else {
        SetBackgroundPixmap(None);
        SetBackgroundColor(_backPixel);
      }
#else
      SetBackgroundPixmap(None);
      SetBackgroundPixmap(ParentRelative);
#endif
    }
  }

  if (_tip) _tip->Reconfig();

#if 0
  NeedRedraw(True);
#else
  ClearWindow();
  NeedRedraw(True/*False*/);
#endif
}
