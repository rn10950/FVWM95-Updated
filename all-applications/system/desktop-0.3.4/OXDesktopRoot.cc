/**************************************************************************

    This file is part of a xclass desktop manager.
    Copyright (C) 1996-2000 David Barth, Hector Peraza.

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

#include <stdlib.h>

#include <X11/X.h>
#include <X11/Xlib.h>

#include <xclass/OGC.h>
#include <xclass/ORectangle.h>
#include <xclass/OResourcePool.h>
#include <xclass/utils.h>

#include "OXDesktopRoot.h"
#include "OXDesktopContainer.h"
#include "main.h"


#ifndef min
#define min(a,b) (((a)<(b)) ? (a) : (b))
#endif

#ifndef max
#define max(a,b) (((a)>(b)) ? (a) : (b))
#endif


int OXDesktopRoot::_errorCode;


//----------------------------------------------------------------------

OXDesktopRoot::OXDesktopRoot(OXClient *c, OXDesktopMain *main, int *retc) :
  OXRootWindow(c, c->GetRoot()->GetId()) {

    _main = main;
    _errorCode = None;

    // grab all buttons, setup an error handler, since the grab might fail

    XSync(GetDisplay(), False);
    XErrorHandler oldHandler = XSetErrorHandler(OXDesktopRoot::ErrorHandler);

#if 1
    XGrabButton(GetDisplay(), AnyButton, AnyModifier, _id,
                False, EnterWindowMask | LeaveWindowMask |
                ButtonPressMask | ButtonReleaseMask | ButtonMotionMask,
                GrabModeSync, GrabModeAsync, None,
                _client->GetResourcePool()->GetGrabCursor());
#else
    XSelectInput(GetDisplay(), _id, ButtonPressMask | ButtonReleaseMask);
#endif

    XSync(GetDisplay(), False);
    XSetErrorHandler(oldHandler);

    *retc = _errorCode;

    // create a GC for the selection box

    XGCValues gcv;
    unsigned long gcm;

    unsigned int _fore = _client->GetResourcePool()->GetDocumentFgndColor();
//    unsigned int _back = _client->GetResourcePool()->GetDocumentBgndColor();
    unsigned int _back = _client->GetColorByName("turquoise4");

    gcm = GCForeground | GCBackground | GCFunction | GCFillStyle |
          GCLineWidth | GCLineStyle | GCSubwindowMode | GCGraphicsExposures;
    gcv.foreground = _fore ^ _back;
    gcv.background = _back;
    gcv.function = GXxor;
    gcv.line_width = 0;
    gcv.line_style = LineOnOffDash;
    gcv.fill_style = FillSolid;
    gcv.subwindow_mode = ClipByChildren; //IncludeInferiors;
    gcv.graphics_exposures = False;
    _lineGC = new OXGC(GetDisplay(), _id, gcm, &gcv);

    XSetDashes(GetDisplay(), _lineGC->GetGC(), 0, "\x1\x1", 2);

    _x0 = _y0 = _x1 = _y1 = 0;
    _dragSelecting = False;
}

OXDesktopRoot::~OXDesktopRoot() {
  XUngrabButton(GetDisplay(), AnyButton, AnyModifier, _id);
  if (_dragSelecting) DrawDragOutline();
  delete _lineGC;
}

int OXDesktopRoot::ErrorHandler(Display *dpy, XErrorEvent *event) {
  _errorCode = event->error_code;
  return 0;
}

int OXDesktopRoot::HandleEvent(XEvent *event) {

  switch (event->xany.type) {
    case ButtonPress:
    case ButtonRelease:
      HandleButton((XButtonEvent *) event);
      break;

    case MotionNotify:
      HandleMotion((XMotionEvent *) event);
      break;

    default:
      return OXRootWindow::HandleEvent(event);
      break;
  }

  return True;
}

int OXDesktopRoot::HandleMaskEvent(XEvent *event) {

  switch (event->xany.type) {
    case ButtonPress:
    case ButtonRelease:
      XSync(GetDisplay(), False);
      XAllowEvents(GetDisplay(), ReplayPointer, event->xbutton.time/*CurrentTime*/);
      XSync(GetDisplay(), False);
      break;

    default:
      return OXRootWindow::HandleMaskEvent(event);
      break;
  }

  return False;
}

int OXDesktopRoot::HandleButton(XButtonEvent *event) {

  if ((event->subwindow != None) && !_dragSelecting) {

    // pass the click event to the WM
    XSync(GetDisplay(), False);
    XAllowEvents(GetDisplay(), ReplayPointer, event->time/*CurrentTime*/);
    XSync(GetDisplay(), False);

  } else {

    // process the click, but do not pass it to the WM
    XSync(GetDisplay(), False);
    XAllowEvents(GetDisplay(), AsyncPointer, event->time/*CurrentTime*/);
    XSync(GetDisplay(), False);

    if (event->type == ButtonRelease) {
      ODesktopRootMessage msg(MSG_DESKTOPROOT, MSG_CLICK, event->button,
                              event->x_root, event->y_root);
      SendMessage(_msgObject, &msg);
    }

    if (event->button == Button1) {
      if (event->type == ButtonPress) {
        _dragSelecting = True;
        _x0 = _x1 = event->x_root;
        _y0 = _y1 = event->y_root;
        DrawDragOutline();
      } else {
        DrawDragOutline();
        _dragSelecting = False;
      }
    }

  }

  return True;
}

int OXDesktopRoot::HandleMotion(XMotionEvent *event) {

  if (_dragSelecting) {
    DrawDragOutline();
    _x1 = event->x_root;
    _y1 = event->y_root;
    DrawDragOutline();
  }
  
  return True;
}

void OXDesktopRoot::DrawDragOutline() {

  _x0 = max(_x0, 0);
  _y0 = max(_y0, 0);
  _y0 = min(_y0, _client->GetDisplayHeight());
  _x0 = min(_x0, _client->GetDisplayWidth());

  _y1 = max(_y1, 0);
  _x1 = max(_x1, 0);
  _y1 = min(_y1, _client->GetDisplayHeight());
  _x1 = min(_x1, _client->GetDisplayWidth());

  ORectangle rect(min(_x0, _x1), min(_y0, _y1),
                  abs(_x1 - _x0), abs(_y1 - _y0));

  DrawRectangle(_lineGC->GetGC(), rect.x, rect.y, rect.w, rect.h);

  _main->GetDesktopContainer()->SelectInRectangle(rect);
}
