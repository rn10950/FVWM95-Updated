/**************************************************************************

    This file is part of xclass.
    Copyright (C) 1996-2000 Hector Peraza.

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
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <X11/Xatom.h>
#include <X11/cursorfont.h>

#include <xclass/utils.h>
#include <xclass/OXClient.h>
#include <xclass/OXMainFrame.h>
#include <xclass/ODNDmanager.h>
#include <xclass/OXDragWindow.h>


#define XDND_PROTOCOL_VERSION      4

Atom ODNDmanager::DNDaware         = None;
Atom ODNDmanager::DNDselection     = None;
Atom ODNDmanager::DNDproxy         = None;

Atom ODNDmanager::DNDenter         = None;
Atom ODNDmanager::DNDleave         = None;
Atom ODNDmanager::DNDposition      = None;
Atom ODNDmanager::DNDstatus        = None;
Atom ODNDmanager::DNDdrop          = None;
Atom ODNDmanager::DNDfinished      = None;
Atom ODNDmanager::DNDversion       = None;

Atom ODNDmanager::DNDactionCopy    = None;
Atom ODNDmanager::DNDactionMove    = None;
Atom ODNDmanager::DNDactionLink    = None;
Atom ODNDmanager::DNDactionAsk     = None;
Atom ODNDmanager::DNDactionPrivate = None;

Atom ODNDmanager::DNDtypeList      = None;
Atom ODNDmanager::DNDactionList    = None;
Atom ODNDmanager::DNDactionDescrip = None;

Atom ODNDmanager::_XA_WM_STATE     = None;
Atom ODNDmanager::_XC_DND_DATA     = None;

int ODNDmanager::_init = False;

// TODO:
// - add an X error handler to handle the cases when the target
//   application exits during drag-and-drop
// - add an OXFrame::HandleDNDstatus event handler?
// - implement INCR protocol
// - cache several requests?


//----------------------------------------------------------------------

ODNDmanager::ODNDmanager(OXClient *client, OXMainFrame *toplevel,
                         Atom *typelist) {

  _client = client;

  _main = toplevel;
  _dpy = _main->GetDisplay();
  _version = XDND_PROTOCOL_VERSION;
  _typelist = typelist;

  if (!_init) {
    _InitAtoms();
    _init = True;
  }

  //Reset();
  _dropTimeout = NULL;

  _source = None;
  _target = None;
  _targetIsDNDaware = False;
  _statusPending = False;
  _dropAccepted = False;  // this would become obsoleted by _acceptedAction
  _acceptedAction = None; // target's accepted action
  _localAction = None;    // our last specified action when we act as source
  _dragging = False;
  _dragWin = NULL;
  _localSource = NULL;
  _localTarget = NULL;
  _pic = _mask = None;
  _draggerTypes = NULL;
  _dropType = None;

  _grabEventMask = ButtonPressMask | ButtonReleaseMask | ButtonMotionMask;

  DND_NoDropCursor = XCreateFontCursor(_dpy, XC_pirate);

  // set the aware prop

  _SetAware(_typelist);
  _SetTypeList(_typelist);

  _proxy_ours = False;
}

ODNDmanager::~ODNDmanager() {
  // remove the proxy prop if we own it
  if (_proxy_ours) RemoveRootProxy();

  // remove the aware prop ant the types list, if any
  XDeleteProperty(_dpy, _main->GetId(), DNDaware);
  XDeleteProperty(_dpy, _main->GetId(), DNDtypeList);
  if (_dropTimeout) delete _dropTimeout;

  // delete the drag pixmap, if any
  if (_dragWin) {
    _dragWin->DestroyWindow();
    delete _dragWin;
  }
  if (_pic != None) XFreePixmap(_dpy, _pic);
  if (_mask != None) XFreePixmap(_dpy, _mask);

  if (_draggerTypes) delete[] _draggerTypes;
}

void ODNDmanager::_InitAtoms() {

  // awareness
  DNDaware = XInternAtom(_dpy, "XdndAware", False);
  
  // selection
  DNDselection = XInternAtom(_dpy, "XdndSelection", False);

  // proxy window
  DNDproxy = XInternAtom(_dpy, "XdndProxy", False);

  // messages
  DNDenter    = XInternAtom(_dpy, "XdndEnter", False);
  DNDleave    = XInternAtom(_dpy, "XdndLeave", False);
  DNDposition = XInternAtom(_dpy, "XdndPosition", False);
  DNDstatus   = XInternAtom(_dpy, "XdndStatus", False);
  DNDdrop     = XInternAtom(_dpy, "XdndDrop", False);
  DNDfinished = XInternAtom(_dpy, "XdndFinished", False);
  
  // actions
  DNDactionCopy    = XInternAtom(_dpy, "XdndActionCopy", False);
  DNDactionMove    = XInternAtom(_dpy, "XdndActionMove", False);
  DNDactionLink    = XInternAtom(_dpy, "XdndActionLink", False);
  DNDactionAsk     = XInternAtom(_dpy, "XdndActionAsk", False);
  DNDactionPrivate = XInternAtom(_dpy, "XdndActionPrivate", False);
  
  // types list
  DNDtypeList      = XInternAtom(_dpy, "XdndTypeList", False);
  DNDactionList    = XInternAtom(_dpy, "XdndActionList", False);
  DNDactionDescrip = XInternAtom(_dpy, "XdndActionDescription", False);

  // misc
  _XA_WM_STATE = XInternAtom(_dpy, "WM_STATE", False);
  _XC_DND_DATA = XInternAtom(_dpy, "_XC_DND_DATA", False);

}

static int _ArrayLength(Atom *a) {
  int n;

  for (n = 0; a[n]; n++);
  return n;
}

void ODNDmanager::_SetAware(Atom *typelist) {

  XChangeProperty(_dpy, _main->GetId(), DNDaware, XA_ATOM, 32,
                  PropModeReplace, (unsigned char *) &_version, 1);

  if (typelist) {
    int n = _ArrayLength(typelist);
    if (n > 0) {
      XChangeProperty(_dpy, _main->GetId(), DNDaware, XA_ATOM, 32,
                      PropModeAppend, (unsigned char *) typelist, n);
    }
  }
}

int ODNDmanager::_IsDNDAware(Window win, Atom *typelist) {
  Atom actual;
  int format;
  unsigned long count, remaining;
  unsigned char *data = 0;
  Atom *types, *t;
  int result = True;

  if (win == None) return False;

  XGetWindowProperty(_dpy, win, DNDaware, 0, 0x8000000L, False, XA_ATOM,
                     &actual, &format, &count, &remaining, &data);

  if ((actual != XA_ATOM) || (format != 32) || (count == 0) || !data) {
    if (data) XFree(data);
    return False;
  }

  types = (Atom *) data;
  _useVersion = min(_version, types[0]);
  Debug(DBG_MISC, "Using XDND version %d\n", _useVersion);

  if ((count > 1) && typelist) {
    result = False;
    for (t = typelist; *t; t++) {
      for (int j = 1; j < (int) count; j++) {
        if (types[j] == *t) {
          result = True;
          break;
        }
      }
      if (result) break;
    }
  }

  XFree(data);

  return result;
}

void ODNDmanager::_SetTypeList(Atom *typelist) {
  int n = _ArrayLength(typelist);
  XChangeProperty(_dpy, _main->GetId(), DNDtypeList, XA_ATOM, 32,
                  PropModeReplace, (unsigned char *) typelist, n);
}

Window ODNDmanager::_FindWindow(Window root, int x, int y, int maxd) {
  XWindowAttributes wattr;

  if (maxd <= 0) return None;

  if (_dragWin && _dragWin->HasWindow(root)) return None;

  XGetWindowAttributes(_dpy, root, &wattr);
  if (wattr.map_state != IsUnmapped &&
      x >= wattr.x && x < wattr.x + wattr.width &&
      y >= wattr.y && y < wattr.y + wattr.height) {

    // mapped and inside, is it a top-level?

    if (_IsTopLevel(root)) return root;

    Window r, p, *children;
    unsigned int numch;
    int i;

    if (XQueryTree(_dpy, root, &r, &p, &children, &numch)) {
      if (children && numch > 0) {
        r = None;

        // upon return from XQueryTree, children are listed in the current
        // stacking order, from bottom-most (first) to top-most (last)

        for (i = numch-1; i >= 0; --i) {
          r = _FindWindow(children[i], x - wattr.x, y - wattr.y, maxd-1); 
          if (r != None) break;
        }

        XFree(children);

        if (r != None) return r;
      }
      return root;   // ?!?
    }
  }

  return None;
}

int ODNDmanager::_IsTopLevel(Window win) {
  Atom type = None;
  int fmt;
  unsigned long nitems, after;
  unsigned char *data;

  XGetWindowProperty(_dpy, win, _XA_WM_STATE, 0L, 0L, False,
                     AnyPropertyType, &type, &fmt, &nitems, &after,
                     &data);

  if (data) XFree(data);
  if (type != None) return True;

  return False;
}

Window ODNDmanager::_GetRootProxy() {
  Atom actual;
  int format;
  unsigned long count, remaining;
  unsigned char *data = NULL;
  Window win, proxy = None;

  // search for XdndProxy property on the root window...

  XSync(_dpy, False);      // get to known state...
  oldhandler = XSetErrorHandler(ODNDmanager::CatchXError);
  target_error = False;

  XGetWindowProperty(_dpy, _client->GetRoot()->GetId(),
                     DNDproxy, 0, 1, False, XA_WINDOW,
                     &actual, &format, &count, &remaining, &data);

  if ((actual == XA_WINDOW) && (format == 32) && (count > 0) && data) {

    // found the XdndProxy property, now check for the proxy window...

    win = *((Window *) data);
    XFree(data);
    data = NULL;

    XGetWindowProperty(_dpy, win, DNDproxy, 0, 1, False, XA_WINDOW,
                       &actual, &format, &count, &remaining, &data);

    XSync(_dpy, False);      // force the error...

    if (!target_error && (actual == XA_WINDOW) &&
        (format == 32) && (count > 0) && data) {
      if (*((Window *) data) == win) {

        // proxy window exists and is correct

        proxy = win;
      }
    }

  }

  if (data) XFree(data);

  oldhandler = XSetErrorHandler(oldhandler);

  return proxy;
}

int ODNDmanager::target_error;
XErrorHandler ODNDmanager::oldhandler;

int ODNDmanager::CatchXError(Display *dpy, XErrorEvent *err) {
  if (err->error_code == BadWindow) {
    target_error = True;
  }
  if (target_error) return 0;
  return oldhandler(dpy, err);
}


//----------------------------------------------------------------------

int ODNDmanager::HandleClientMessage(XClientMessageEvent *event) {

  if (event->message_type == DNDenter) {
    _HandleDNDenter((Window) event->data.l[0], event->data.l[1],
                    /*(Atom[3])*/ (Atom *) &event->data.l[2]);

  } else if (event->message_type == DNDleave) {
    _HandleDNDleave((Window) event->data.l[0]);

  } else if (event->message_type == DNDposition) {
    _HandleDNDposition((Window) event->data.l[0],
                       (int) (event->data.l[2] >> 16) & 0xFFFF,  // x_root
                       (int) (event->data.l[2] & 0xFFFF),        // y_root
                       (Atom) event->data.l[4],                  // action
                       (Time) event->data.l[3]);                 // timestamp

  } else if (event->message_type == DNDstatus) {
    XRectangle skip;

    skip.x      = (event->data.l[2] >> 16) & 0xFFFF;
    skip.y      = (event->data.l[2] & 0xFFFF);
    skip.width  = (event->data.l[3] >> 16) & 0xFFFF;
    skip.height = (event->data.l[3] & 0xFFFF);

    _HandleDNDstatus((Window) event->data.l[0], 
                     (int) (event->data.l[1] & 0x1),
                     skip, (Atom) event->data.l[4]);

  } else if (event->message_type == DNDdrop) {
    _HandleDNDdrop((Window) event->data.l[0], (Time) event->data.l[2]);

  } else if (event->message_type == DNDfinished) {
    _HandleDNDfinished((Window) event->data.l[0]);

  } else {
    return False;  // not for us...

  }

  return True;
}

int ODNDmanager::HandleTimer(OTimer *t) {
  if (t == _dropTimeout) {

    // The drop operation timed out without receiving
    // status confirmation from the target. Send a
    // leave message instead (and notify the user or widget).

    delete _dropTimeout;
    _dropTimeout = NULL;

    _SendDNDleave(_target);
    _statusPending = False;

    if (_localSource) _localSource->HandleDNDfinished();

    return True;
  }

  return False;
}


//----------------------------------------------------------------------

void ODNDmanager::_SendDNDenter(Window target) {
  int i, n;
  XEvent xevent;

  xevent.xany.type      = ClientMessage;
  xevent.xany.display   = _dpy;
  xevent.xclient.window = target;
  xevent.xclient.message_type = DNDenter;
  xevent.xclient.format = 32;

  xevent.xclient.data.l[0] = _main->GetId();  // from;

  n = _ArrayLength(_typelist);

  xevent.xclient.data.l[1] = ((n > 3) ? 1L : 0L) | (_useVersion << 24);

  // set the first 1-3 data types

  for (i = 0; i < 3; ++i)
    xevent.xclient.data.l[2+i] = (i < n) ? _typelist[i] : None;

  XSendEvent(_dpy, target, False, NoEventMask, &xevent);
}

void ODNDmanager::_SendDNDleave(Window target) {
  XEvent xevent;

  xevent.xany.type      = ClientMessage;
  xevent.xany.display   = _dpy;
  xevent.xclient.window = target;
  xevent.xclient.message_type = DNDleave;
  xevent.xclient.format = 32;

  xevent.xclient.data.l[0] = _main->GetId();  // from;
  xevent.xclient.data.l[1] = 0L;

  xevent.xclient.data.l[2] = 0L;
  xevent.xclient.data.l[3] = 0L;
  xevent.xclient.data.l[4] = 0L;

  XSendEvent(_dpy, target, False, NoEventMask, &xevent);
}

void ODNDmanager::_SendDNDposition(Window target, int x, int y,
                                   Atom action, Time timestamp) {
  XEvent xevent;

  xevent.xany.type      = ClientMessage;
  xevent.xany.display   = _dpy;
  xevent.xclient.window = target;
  xevent.xclient.message_type = DNDposition;
  xevent.xclient.format = 32;

  xevent.xclient.data.l[0] = _main->GetId();  // from;
  xevent.xclient.data.l[1] = 0L;

  xevent.xclient.data.l[2] = (x << 16) | y;   // root coodinates
  xevent.xclient.data.l[3] = timestamp;       // timestamp for retrieving data
  xevent.xclient.data.l[4] = action;          // requested action

  XSendEvent(_dpy, target, False, NoEventMask, &xevent);
}

void ODNDmanager::_SendDNDstatus(Window source, Atom action) {
  XEvent xevent;

  xevent.xany.type      = ClientMessage;
  xevent.xany.display   = _dpy;
  xevent.xclient.window = source;
  xevent.xclient.message_type = DNDstatus;
  xevent.xclient.format = 32;

  xevent.xclient.data.l[0] = _main->GetId();    // from;
  xevent.xclient.data.l[1] = (action == None) ? 0L : 1L;

  xevent.xclient.data.l[2] = 0L;                // empty rectangle
  xevent.xclient.data.l[3] = 0L;
  xevent.xclient.data.l[4] = action;            // accepted action

  XSendEvent(_dpy, source, False, NoEventMask, &xevent);
}

void ODNDmanager::_SendDNDdrop(Window target) {
  XEvent xevent;

  xevent.xany.type      = ClientMessage;
  xevent.xany.display   = _dpy;
  xevent.xclient.window = target;
  xevent.xclient.message_type = DNDdrop;
  xevent.xclient.format = 32;

  xevent.xclient.data.l[0] = _main->GetId();    // from;
  xevent.xclient.data.l[1] = 0L;                // reserved
  xevent.xclient.data.l[2] = CurrentTime;       // timestamp
  xevent.xclient.data.l[3] = 0L;
  xevent.xclient.data.l[4] = 0L;

  XSendEvent(_dpy, target, False, NoEventMask, &xevent);
}

void ODNDmanager::_SendDNDfinished(Window source) {
  XEvent xevent;

  xevent.xany.type      = ClientMessage;
  xevent.xany.display   = _dpy;
  xevent.xclient.window = source;
  xevent.xclient.message_type = DNDfinished;
  xevent.xclient.format = 32;

  xevent.xclient.data.l[0] = _main->GetId();    // from;
  xevent.xclient.data.l[1] = 0L;                // reserved
  xevent.xclient.data.l[2] = 0L;
  xevent.xclient.data.l[3] = 0L;
  xevent.xclient.data.l[4] = 0L;

  XSendEvent(_dpy, source, False, NoEventMask, &xevent);
}


//---------------------------------------------------------------------

int ODNDmanager::_HandleDNDenter(Window src, long vers, Atom dataTypes[3]) {

  _source = src;

  if (_draggerTypes) delete[] _draggerTypes;

  if (vers & 1) {  // more than 3 data types?
    Atom type, *a;
    int format, i, j;
    unsigned long count, remaining;
    unsigned char *data = NULL;

    XGetWindowProperty(_dpy, src, DNDtypeList,
                       0, 0x8000000L, False, XA_ATOM,
                       &type, &format, &count, &remaining, &data);

    if (type != XA_ATOM || format != 32 || !data) {
      count = 0;
    }

    _draggerTypes = new Atom[count+4];

    j = 0;
#if 0
    _draggerTypes[j++] = dataTypes[0];
    _draggerTypes[j++] = dataTypes[1];
    _draggerTypes[j++] = dataTypes[2];
#endif

    a = (Atom *) data;
    for (i = 0; i < (int) count; i++)
      _draggerTypes[i+j] = a[i];

    _draggerTypes[i+j] = None;

    if (data) XFree (data);

  } else {
    _draggerTypes = new Atom[4];

    _draggerTypes[0] = dataTypes[0];
    _draggerTypes[1] = dataTypes[1];
    _draggerTypes[2] = dataTypes[2];

    _draggerTypes[3] = None;
  }

  // the following is not strictly neccessary, unless the previous
  // dragging application crashed without sending XdndLeave

  if (_localTarget) _localTarget->HandleDNDleave();
  _localTarget = NULL;

  return True;
}

int ODNDmanager::_HandleDNDleave(Window src) {

  _source = None;
  if (_localTarget) _localTarget->HandleDNDleave();
  _localTarget = NULL;

  if (_draggerTypes) delete[] _draggerTypes;
  _draggerTypes = NULL;

  return True;
}

int ODNDmanager::_HandleDNDposition(Window source, int x_root, int y_root,
                                    Atom action, Time timestamp) {
  int x, y;
  Window child;

  XTranslateCoordinates(_dpy, _client->GetRoot()->GetId(),
                        _main->GetId(), x_root, y_root, &x, &y, &child);

  OXFrame *f = _main->GetFrameFromPoint(x, y);

  if (f != _localTarget) {
    if (_localTarget) _localTarget->HandleDNDleave();
    _localTarget = f;
    if (_localTarget) _dropType = _localTarget->HandleDNDenter(_draggerTypes);
//FIXME:
//else if (_proxy_ours) { _localTarget = _main; _dropType = ... ; }
  }

  // query the target widget to determine whether it accepts the
  // required action

  if (_localTarget) {
    action = (_dropType == None) ? None :
              _localTarget->HandleDNDposition(x, y, action, x_root, y_root);
  } else if (_proxy_ours) {
    action = _main->HandleDNDposition(x, y, action, x_root, y_root);
  } else {
    action = None;
  }

  _SendDNDstatus(source, _localAction = action);

  return True;
}

int ODNDmanager::_HandleDNDstatus(Window target, int accepted,
                                  XRectangle area, Atom action) {
  if (target == _target) {  // ignore old windows
    _statusPending = False;
    if (accepted) {
      _dropAccepted = True;
      _acceptedAction = action;
      XChangeActivePointerGrab(_dpy, _grabEventMask,
                               None, CurrentTime);
    } else {
      _dropAccepted = False;
      _acceptedAction = None;
      XChangeActivePointerGrab(_dpy, _grabEventMask,
                               DND_NoDropCursor, CurrentTime);
    }
    if (_dropTimeout) {   // were we waiting for this to do the drop?
      delete _dropTimeout;
      _dropTimeout = NULL;
      _SendDNDdrop(_target);
    }
  }

  return True;
}

int ODNDmanager::_HandleDNDdrop(Window source, Time timestamp) {

  // to get the data, we must call XConvertSelection with
  // the timestamp in XdndDrop, wait for SelectionNotify
  // to arrive to retrieve the data, and when we are finished,
  // send a XdndFinished message to the source.

  if (_dropType != None) {
    XChangeProperty(_dpy, _main->GetId(), _XC_DND_DATA, _dropType, 8,
                    PropModeReplace, (unsigned char *) 0, 0);

    XConvertSelection(_dpy, DNDselection, _dropType, _XC_DND_DATA,
                      _main->GetId(), timestamp);
  }

  _source = source;
//  _SendDNDfinished(source);

  return True;
}

int ODNDmanager::_HandleDNDfinished(Window target) {

  if (_localSource) _localSource->HandleDNDfinished();

  return True;
}


//----------------------------------------------------------------------

int ODNDmanager::HandleSelectionRequest(XSelectionRequestEvent *event) {
  if (event->selection == DNDselection) {
    XEvent xevent;
    ODNDdata *DNDdata = NULL;
    char *data;
    int len;

    // get the data from the drag source widget

    if (_localSource)
      DNDdata = _localSource->GetDNDdata(event->target);

    data = DNDdata ? (char *) DNDdata->data : (char *) "";
    len  = DNDdata ? DNDdata->dataLength : 0;

    if (event->property == None) {
      //printf("warning: None property specified in SelectionRequest\n");
      event->property = _XC_DND_DATA;
    }

    XChangeProperty(_dpy, event->requestor, event->property,
                    event->target, 8, PropModeReplace,
                    (unsigned char *) data, len);

    xevent.xselection.type      = SelectionNotify;
    xevent.xselection.property  = event->property;
    xevent.xselection.display   = event->display;
    xevent.xselection.requestor = event->requestor;
    xevent.xselection.selection = event->selection;
    xevent.xselection.target    = event->target;
    xevent.xselection.time      = event->time;
    XSendEvent(_dpy, event->requestor, False, NoEventMask, &xevent);

    return True;

  } else {
    return False;  // not for us...

  }
}

int ODNDmanager::HandleSelection(XSelectionEvent *event) {
  if (event->selection == DNDselection) {

    Atom actual;
    int format;
    unsigned long count, remaining;
    unsigned char *data = 0;

    XGetWindowProperty(_dpy, event->requestor, event->property,
                       0, 0x8000000L, True, event->target,
                       &actual, &format, &count, &remaining, &data);

    if ((actual != _dropType) || (format != 8) || (count == 0) || !data) {
      if (data) XFree(data);
      return False;
    }

    if (_source != None) _SendDNDfinished(_source);

    // send the data to the target widget

    if (_localTarget) {
      ODNDdata dndData(actual, data, count, _localAction);
      _localTarget->HandleDNDdrop(&dndData);
      if (_draggerTypes) delete[] _draggerTypes;
      _draggerTypes = NULL;
    }

    _source = None;
    _localAction = None;

    XFree(data);

    return True;

  } else {
    return False;  // not for us...

  }
}


//----------------------------------------------------------------------

void ODNDmanager::SetDragPixmap(Pixmap pic, Pixmap mask,
                                int hot_x, int hot_y) {

  if (_pic != None) XFreePixmap(_dpy, _pic);
  if (_mask != None) XFreePixmap(_dpy, _mask);

  _pic = pic;
  _mask = mask;
  _hx = hot_x;
  _hy = hot_y;
}

int ODNDmanager::StartDrag(OXFrame *src, int x_root, int y_root,
                           Window grabWin) {

  if (_dragging) return True;

  _localSource = src;

  if (!XSetSelectionOwner(_dpy, DNDselection, _main->GetId(), CurrentTime)) {
    // hmmm... failed to acquire ownership of XdndSelection!
    return False;
  }

  if (grabWin == None) grabWin = _main->GetId();

  if (XGrabPointer(_dpy, grabWin, False, _grabEventMask,
                   GrabModeAsync, GrabModeAsync, None, None,
                   CurrentTime) != GrabSuccess) {
    return False;
  }

  _dragging = True;
  _target = None;
  _targetIsDNDaware = False;
  _statusPending = False;
  if (_dropTimeout) delete _dropTimeout;
  _dropTimeout = NULL;
  _dropAccepted = False;
  _acceptedAction = None;
  _localAction = None;

  if (!_dragWin && _pic != None && _mask != None) {
    _dragWin = new OXDragWindow(_client->GetRoot(), _pic, _mask);
    _dragWin->Move((x_root-_hx)|1, (y_root-_hy)|1);
    _dragWin->MapSubwindows();
    _dragWin->MapRaised();
  }

  return True;
}

int ODNDmanager::Drop() {

  if (!_dragging) return False;

  if (_targetIsDNDaware) {
    if (_dropAccepted) {
      if (_statusPending) {
        if (_dropTimeout) delete _dropTimeout;
        _dropTimeout = new OTimer(this, 5000);
      } else {
        _SendDNDdrop(_target);
      }
    } else {
      _SendDNDleave(_target);
      _statusPending = False;
    }
  }

  EndDrag();

  return True;
}

int ODNDmanager::EndDrag() {

  if (!_dragging) return False;

  XUngrabPointer(_dpy, CurrentTime);

  _dragging = False;
  if (_dragWin) {
    _dragWin->DestroyWindow();
    delete _dragWin;
    _dragWin = NULL;
  }

  return True;
}

int ODNDmanager::Drag(int x_root, int y_root, Atom action, Time timestamp) {

  if (!_dragging) return False;

  Window newTarget = _FindWindow(_client->GetRoot()->GetId(),
                                 x_root, y_root, 4);

  if (newTarget == _client->GetRoot()->GetId()) {
    Window t = _GetRootProxy();
    if (t != None) newTarget = t;
  }

  if (_target != newTarget) {

    if (_targetIsDNDaware) _SendDNDleave(_target);

    _target = newTarget;
    _targetIsDNDaware = _IsDNDAware(_target);
    _statusPending = False;
    _dropAccepted = False;
    _acceptedAction = None;

    if (_targetIsDNDaware) _SendDNDenter(_target);

    XChangeActivePointerGrab(_dpy, _grabEventMask,
                             /*None*/DND_NoDropCursor, CurrentTime);
  }

  if (_targetIsDNDaware && !_statusPending) {
    _SendDNDposition(_target, x_root, y_root, action, timestamp);

    // this is to avoid sending XdndPosition messages over and over
    // if the target is not responding

    _statusPending = True;
  }

  if (_dragWin) {
    _dragWin->RaiseWindow();
    _dragWin->Move((x_root-_hx)|1, (y_root-_hy)|1);
  }

  return True;    
}

int ODNDmanager::SetRootProxy() {
  Window mainw = _main->GetId();
  int result = False;

  XGrabServer(_dpy);

  if (_GetRootProxy() == None) {
    XChangeProperty(_dpy, _client->GetRoot()->GetId(),
                    DNDproxy, XA_WINDOW, 32,
                    PropModeReplace, (unsigned char *) &mainw, 1);

    XChangeProperty(_dpy, mainw, DNDproxy, XA_WINDOW, 32,
                    PropModeReplace, (unsigned char *) &mainw, 1);

    _proxy_ours = True;
    result = True;
  }

  XUngrabServer(_dpy);
  XSync(_dpy, False);

  return result;
}

int ODNDmanager::RemoveRootProxy() {
  if (!_proxy_ours) return False;

  XDeleteProperty(_dpy, _main->GetId(), DNDproxy);
  XDeleteProperty(_dpy, _client->GetRoot()->GetId(), DNDproxy);

  // the following is to ensure that the properties
  // (specially the one on the root window) are deleted
  // in case the application is exiting...

  XSync(_dpy, False);

  _proxy_ours = False;

  return True;
}
