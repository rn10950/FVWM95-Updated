/**************************************************************************

    This file is part of xclass, a Win95-looking GUI toolkit.
    Copyright (C) 1996, 1997 David Barth, Ricky Ralston, Hector Peraza.

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
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>

#include <xclass/utils.h>
#include <xclass/OXClient.h>
#include <xclass/OXRootWindow.h>
#include <xclass/OXFrame.h>
#include <xclass/OGC.h>
#include <xclass/OColor.h>
#include <xclass/OPicture.h>
#include <xclass/OResourcePool.h>
#include <xclass/OXFont.h>
#include <xclass/OTimer.h>
#include <xclass/OFileHandler.h>
#include <xclass/OIdleHandler.h>


Atom WM_DELETE_WINDOW;
Atom _MOTIF_WM_HINTS;
Atom _XCLASS_MESSAGE;
Atom _XCLASS_COLORS;
Atom _XCLASS_RESOURCES;
Atom _XA_INCR;


//----------------------------------------------------------------------

OXClient::OXClient(int argc, char *argv[]) {
  int  i, sync = False;
  char *dpname = "";

  for (i = 0; i < argc - 1; ++i) {
    if (strcmp(argv[i], "-display") == 0) {
      dpname = argv[i+1];
      ++i;
    } else if (strcmp(argv[i], "-synchronize") == 0) {
      sync = True;
    }
  }
  _Init(dpname);
  if (sync) XSynchronize(_dpy, True);
}

void OXClient::_Init(const char *DpyName) {
  XWindowAttributes root_attr;

  _Root = NULL;

  //--- Open the connection to the display

  if (!(_dpy = XOpenDisplay(DpyName)))
    FatalError("Can't open display \"%s\"", XDisplayName(DpyName));

  _screen = DefaultScreen(_dpy);
  _depth = DefaultDepth(_dpy, _screen);

  _x_fd = ConnectionNumber(_dpy);

  //--- Initialize internal lists

  _wlist    = new OXSList(_dpy, "Application Frames");
  _mlist    = new OXSList(_dpy, "Top-Level Frames");
  _tlist    = new OXSTimerList(_dpy, "Timers");
  _idlelist = new OXSList(_dpy, "Idle Handlers");
  _iolist   = new OXSList(_dpy, "I/O Handlers");
  _siglist  = new OXSList(_dpy, "Signal Handlers");

  if (!_wlist || !_tlist || !_idlelist || !_iolist || !_siglist)
    FatalError("Not enough memory");

  _globalNeedRedraw = False;

  _tip_timer = NULL;
  _tip_status = False;

  FD_ZERO(&readset);
  FD_ZERO(&writeset);
  FD_ZERO(&excptset);


  //--- Setup some atoms...

  WM_DELETE_WINDOW = RegisterXAtom("WM_DELETE_WINDOW");
  _MOTIF_WM_HINTS = RegisterXAtom("_MOTIF_WM_HINTS");
  _XCLASS_MESSAGE = RegisterXAtom("_XCLASS_MESSAGE");
  _XCLASS_COLORS = RegisterXAtom("_XCLASS_COLORS");
  _XCLASS_RESOURCES = RegisterXAtom("_XCLASS_RESOURCES");
  _XA_INCR = RegisterXAtom("INCR");


  //--- Create an object for the root window, create picture pool, etc...

  _Root = new OXRootWindow(this, XDefaultRootWindow(_dpy), NULL);
  XGetWindowAttributes(_dpy, _Root->GetId(), &root_attr);
  _defaultColormap = root_attr.colormap;

  _resourcePool = new OResourcePool(this, NULL);

  _picturePool = _resourcePool->GetPicturePool();
  _gcPool = _resourcePool->GetGCPool();
  _fontPool = _resourcePool->GetFontPool();

}

OXClient::~OXClient() {

  //--- Going down: cleanup, close X connection but don't exit

  Debug(DBG_MISC, "OXClient: terminating!\n");

  delete _resourcePool;

  delete _Root;        // before _wlist gets destroyed!

  delete _wlist;
  delete _tlist;
  delete _idlelist;
  delete _iolist;
  delete _siglist;

  XSync(GetDisplay(), False);

  XCloseDisplay(_dpy); // this should do a cleanup of the remaining
                       // X allocated objects...
}

//----------------------------------------------------------------------

// Color management routines...

unsigned long OXClient::GetColorByName(const char *name) const {
  XColor color;

  color.pixel = 0;
  if (!XParseColor(_dpy, _defaultColormap, name, &color))  {
    Debug(DBG_WARN, "OXClient: Couldn't parse color %s\n", name);
  } else if (!XAllocColor(_dpy, _defaultColormap, &color)) {
    Debug(DBG_WARN, "OXClient: Couldn't retrieve color %s\n", name);
    // force allocation of pixel 0
    XQueryColor(_dpy, _defaultColormap, &color);
    if (!XAllocColor(_dpy, _defaultColormap, &color)) {
      Debug(DBG_WARN, "OXClient: failed to allocate pixel 0!!!\n");
    }
  }
  return color.pixel;
}

unsigned long OXClient::GetColor(OColor c) const {
  XColor color;

  color.pixel = 0;
  color.red   = c.GetR() * 256;
  color.green = c.GetG() * 256;
  color.blue  = c.GetB() * 256;

  if (!XAllocColor(_dpy, _defaultColormap, &color)) {
    Debug(DBG_WARN, "OXClient: Couldn't retrieve color #%02x%02x%02x\n",
          c.GetR(), c.GetG(), c.GetB());
    // force allocation of pixel 0
    XQueryColor(_dpy, _defaultColormap, &color);
    if (!XAllocColor(_dpy, _defaultColormap, &color)) {
      Debug(DBG_WARN, "OXClient: failed to allocate pixel 0!!!\n");
    }
  }

  return color.pixel;
}

// this is used to increase X's ref count of a pixel

unsigned long OXClient::GetColor(unsigned long pixel) const {
  XColor color;

  color.pixel = pixel;

  XQueryColor(_dpy, _defaultColormap, &color);
  if (!XAllocColor(_dpy, _defaultColormap, &color)) {
    Debug(DBG_WARN, "OXClient: failed to re-allocate pixel!!!\n");
  }

  return color.pixel;
}

unsigned long OXClient::GetHilite(unsigned long base_color) const {
  XColor color, white_p;
  
  color.pixel = base_color;
  XQueryColor(_dpy, _defaultColormap, &color);

  white_p.pixel = GetColorByName("white");
  XQueryColor(_dpy, _defaultColormap, &white_p);
  
#if 1
  color.red   = max((white_p.red/5),   color.red);
  color.green = max((white_p.green/5), color.green);
  color.blue  = max((white_p.blue/5),  color.blue);
  
  color.red   = min(white_p.red,   (color.red*140)/100);
  color.green = min(white_p.green, (color.green*140)/100);
  color.blue  = min(white_p.blue,  (color.blue*140)/100);
#else
  color.red   = white_p.red   - (white_p.red   - color.red   - 1) / 2;
  color.green = white_p.green - (white_p.green - color.green - 1) / 2;
  color.blue  = white_p.blue  - (white_p.blue  - color.blue  - 1) / 2;
#endif
  
  if (!XAllocColor(_dpy, _defaultColormap, &color)) {
    Debug(DBG_WARN, "OXClient: Couldn't allocate hilight color\n");
    // force allocation of pixel 0
    XQueryColor(_dpy, _defaultColormap, &color);
    if (!XAllocColor(_dpy, _defaultColormap, &color)) {
      Debug(DBG_WARN, "OXClient: failed to allocate pixel 0!!!\n");
    }
  }

  return color.pixel;
}

unsigned long OXClient::GetShadow(unsigned long base_color) const {
  XColor color;
  
  color.pixel = base_color;
  XQueryColor(_dpy, _defaultColormap, &color);
  
#if 1
  color.red   = (unsigned short)((color.red*60)/100); /* was 50% */
  color.green = (unsigned short)((color.green*60)/100);
  color.blue  = (unsigned short)((color.blue*60)/100);
#else
  color.red   = (unsigned short)(color.red   * 2 / 3);
  color.green = (unsigned short)(color.green * 2 / 3);
  color.blue  = (unsigned short)(color.blue  * 2 / 3);
#endif
  
  if (!XAllocColor(_dpy, _defaultColormap, &color)) {
    Debug(DBG_WARN, "OXClient: Couldn't allocate shadow color\n");
    // force allocation of pixel 0
    XQueryColor(_dpy, _defaultColormap, &color);
    if (!XAllocColor(_dpy, _defaultColormap, &color)) {
      Debug(DBG_WARN, "OXClient: failed to allocate pixel 0!!!\n");
    }
  }
  
  return color.pixel;
}

void OXClient::FreeColor(unsigned long pixel) const {
  XFreeColors(_dpy, _defaultColormap, &pixel, 1, 0);
}


//----------------------------------------------------------------------

// Picture pool management...

const OPicture *OXClient::GetPicture(const char *name, char **data) {
  return _picturePool->GetPicture(name, data);
}

const OPicture *OXClient::GetPicture(const char *name, char **data,
                                     int new_width, int new_height) {
  return _picturePool->GetPicture(name, data,new_width, new_height);
}

const OPicture *OXClient::GetPicture(const char *name) {
  return _picturePool->GetPicture(name);
}

const OPicture *OXClient::GetPicture(const char *name,
                                     int new_width, int new_height) {
  return _picturePool->GetPicture(name, new_width, new_height);
}

const OPicture *OXClient::GetPicture(const OPicture *pic) {
  return _picturePool->GetPicture(pic);
}

void OXClient::FreePicture(const OPicture *pic) {
  _picturePool->FreePicture(pic);
}


//----------------------------------------------------------------------

// GC pool management...

const OXGC *OXClient::GetGC(Drawable d, unsigned long mask,
                            XGCValues *values) {
  return _gcPool->GetGC(d, mask, values);
}

void OXClient::FreeGC(const OXGC *gc) {
  _gcPool->FreeGC(gc);
}

void OXClient::FreeGC(const GC gc) {
  _gcPool->FreeGC(gc);
}


//----------------------------------------------------------------------

// Font pool management...

OXFont *OXClient::GetFont(const char *string) {
  return _fontPool->GetFont(string);
}

OXFont *OXClient::GetFont(const char *family, int ptsize, int weight,
                          int slant) {
  return _fontPool->GetFont(family, ptsize, weight, slant);
}

OXFont *OXClient::GetFont(OXFont *font) {
  return _fontPool->GetFont(font);
}

void OXClient::FreeFont(OXFont *font) {
  _fontPool->FreeFont(font);
}


//----------------------------------------------------------------------

// Register / unregister windows...

int OXClient::RegisterWindow(OXWindow *w, char *name) {
  if (_Root && (w->_id == _Root->_id)) {
    // someone has created a new OXRootWindow object...
    delete _Root;
    _Root = w;
  }
  return (_wlist->Add(w->_id, (XPointer) w) != NULL);
}

int OXClient::UnregisterWindow(OXWindow *w) {
  return (_wlist->Remove(w->_id));
}

int OXClient::RegisterTopLevel(OXWindow *w, char *name) {
  return (_mlist->Add(w->_id, (XPointer) w) != NULL);
}

int OXClient::UnregisterTopLevel(OXWindow *w) {
  return (_mlist->Remove(w->_id));
}


//----------------------------------------------------------------------

// Register / unregister timers...

int OXClient::RegisterTimer(OTimer *t) {
  return (_tlist->Add(t->_id, (XPointer) t) != NULL);
}

int OXClient::UnregisterTimer(OTimer *t) {
  return (_tlist->Remove(t->_id));
}


//----------------------------------------------------------------------

// Register / unregister file events...

int OXClient::RegisterFileEvent(OFileHandler *fh) {
  return (_iolist->Add(fh->_fd, (XPointer) fh) != NULL);
}

int OXClient::UnregisterFileEvent(OFileHandler *fh) {
  return (_iolist->Remove(fh->_fd));
}


//----------------------------------------------------------------------

// Register / unregister idle events...

int OXClient::RegisterIdleEvent(OIdleHandler *ih) {
  return (_idlelist->Add(ih->_id, (XPointer) ih) != NULL);
}

int OXClient::UnregisterIdleEvent(OIdleHandler *ih) {
  return (_idlelist->Remove(ih->_id));
}


//----------------------------------------------------------------------

// Event loop stuff...

void OXClient::Run() {
  MainLoop();
}

void OXClient::MainLoop() {
  while (_mlist->NoOfItems() > 0) {
    ProcessOneEvent(0, None);
  }
}

int OXClient::ProcessOneEvent() {
  if (_mlist->NoOfItems() > 0) {
    ProcessOneEvent(0, None);
    return True;
  }
  return False;
}

void OXClient::WaitFor(OXWindow *w) {
  Window wid;

  //_tip_status = 1;
  if (_tip_timer) {
    delete _tip_timer;
    _tip_timer = NULL;
  }
  wid = w->GetId();
  while (ProcessOneEvent(DestroyNotify, wid)) {
    // safety check!
    const OXWindow *w1 = GetWindowById(wid);
    if (!w1 || (w1 != w)) break;
  }
  _tip_status = 0;
}

void OXClient::WaitForUnmap(OXWindow *w) {
  Window wid;

  //_tip_status = 1;
  //if (_tip_timer) {
  //  delete _tip_timer;
  //  _tip_timer = NULL;
  //  }
  wid = w->GetId();
  while (ProcessOneEvent(UnmapNotify, wid)) {
    // safety check!
    const OXWindow *w1 = GetWindowById(wid);
    if (!w1 || (w1 != w)) break;
  }
  //_tip_status = 0;
}

void OXClient::NeedRedraw(OXWindow *w) {
  w->_needRedraw = True;
  _globalNeedRedraw = True;
}


// Now has an idle-time event dispatcher...

int OXClient::ProcessOneEvent(int EventType, Window wid) {
  int fd_width, nf;
  struct timeval now, tmout, *tptr;

  //--- 1: process pending X events...

  if (XPending(_dpy)) {
    return _ProcessXEvent(EventType, wid);
  }

  //--- 2: process pending file events from the previous call

  if (_ProcessFileEvent()) return True;

  //--- 3: check whether there are any X events ready...

  fd_width = _x_fd + 1;

  FD_ZERO(&readset);
  FD_SET(_x_fd, &readset);

  tmout.tv_sec = tmout.tv_usec = 0;

  nf = select(fd_width, (FD_MASK *) &readset, NULL, NULL, &tmout);
  if (nf <= 0) {
    FD_ZERO(&readset);
    FD_ZERO(&writeset);
    FD_ZERO(&excptset);
  }

  while (True) {

    if ((nf > 0) || ((nf == -1) && (errno == EINTR))) {
      if (FD_ISSET(_x_fd, &readset)) {
        return _ProcessXEvent(EventType, wid);
      } else {
        // leave the processing of other file events for the
        // next call, otherwise things like pending redraws
        // or timers wont get a chance against continuous file
        // operations...
        return True;
      }
    }

    //--- 4: check timers...

    if (_ProcessTimerEvent()) return True;

    //--- 5: do any pending redraws...

    if (_DoRedraw()) return True;

    //--- 6: check for ready file events...

    fd_width = _MakeFDSets();

    tmout.tv_sec = tmout.tv_usec = 0;

    nf = select(fd_width, (FD_MASK *) &readset, NULL, NULL, &tmout);
    if (nf <= 0) {
      FD_ZERO(&readset);
      FD_ZERO(&writeset);
      FD_ZERO(&excptset);
    } else {
      continue;
    }

    //--- 7: dispatch idle events if there is nothing else to do...

    if (_ProcessIdleEvent()) return True;

    //--- 8: sleep until next event arrives or timeout expires...

    fd_width = _MakeFDSets();

    if (_tlist->GetHead() == NULL) {
      tptr = NULL;
    } else {
      tptr = &tmout;
      OTimer *t = (OTimer *) _tlist->GetHead()->data;
      gettimeofday(&now, NULL);
      tmout.tv_sec  = t->_tm.tv_sec  - now.tv_sec;
      tmout.tv_usec = t->_tm.tv_usec - now.tv_usec;
      if (tmout.tv_usec < 0) {
        tmout.tv_usec += 1000000;
        tmout.tv_sec--;
      }
    }

    nf = select(fd_width, (FD_MASK *) &readset, &writeset, &excptset, tptr);

    if (nf == -1) {
      FD_ZERO(&readset);
      FD_ZERO(&writeset);
      FD_ZERO(&excptset);
    }

  }

  return True;
}

int OXClient::_ProcessXEvent(int EventType, Window wid) {
  XEvent Event;

  XNextEvent(_dpy, &Event);
  if (wid == None) {
    HandleEvent(&Event);
  } else {
    HandleMaskEvent(&Event, wid);
    if ((Event.xany.window == wid) &&
        (Event.xany.type == EventType)) return False;
  }

  return True;
}

int OXClient::_MakeFDSets() {
  const OXSNode *ptr;

  FD_ZERO(&readset);
  FD_ZERO(&writeset);
  FD_ZERO(&excptset);
  FD_SET(_x_fd, &readset);

  int fd_width = _x_fd + 1;

  for (ptr=_iolist->GetHead(); ptr!=NULL; ptr=ptr->next) {
    OFileHandler *fh = (OFileHandler *) ptr->data;
    int fd = fh->_fd;

    if (fh->_eventMask & XCM_READABLE) FD_SET(fd, &readset);
    if (fh->_eventMask & XCM_WRITABLE) FD_SET(fd, &writeset);
    if (fh->_eventMask & XCM_EXCEPTION) FD_SET(fd, &excptset);
    if (fd_width <= fd) fd_width = fd + 1;
  }

  return fd_width;
}

int OXClient::_ProcessFileEvent() {
  const OXSNode *ptr;

  for (ptr = _iolist->GetHead(); ptr != NULL; ptr = ptr->next) {
    OFileHandler *fh = (OFileHandler *) ptr->data;
    int fd = fh->_fd;

    if (fh->_eventMask & XCM_READABLE) {
      if (FD_ISSET(fd, &readset)) {
        FD_CLR(fd, &readset);
        fh->_c->HandleFileEvent(fh, XCM_READABLE);
        return True;
      }
    }

    if (fh->_eventMask & XCM_WRITABLE) {
      if (FD_ISSET(fd, &writeset)) {
        FD_CLR(fd, &writeset);
        fh->_c->HandleFileEvent(fh, XCM_WRITABLE);
        return True;
      }
    }

    if (fh->_eventMask & XCM_EXCEPTION) {
      if (FD_ISSET(fd, &excptset)) {
        FD_CLR(fd, &excptset);
        fh->_c->HandleFileEvent(fh, XCM_EXCEPTION);
        return True;
      }
    }

  }

  return False;
}

int OXClient::_ProcessTimerEvent() {
  struct timeval now;

  if (_tlist->GetHead()) {
    OTimer *t = (OTimer *) _tlist->GetHead()->data;

    gettimeofday(&now, NULL);
    if ((t->_tm.tv_sec < now.tv_sec) || 
        ((t->_tm.tv_sec == now.tv_sec) &&
         (t->_tm.tv_usec < now.tv_usec))) {
      UnregisterTimer(t); // _tlist = GetHead()->next;
      // not sure if this belongs here, but...
      if (t == _tip_timer) {
        delete _tip_timer;
        _tip_timer = NULL;
        if ((_tip_status = 2-_tip_status) == 2)
          _tip_frame->ShowTip();
      } else {
        t->_c->HandleTimer(t);
      }
      return True;
    }
  }

  return False;
}

int OXClient::_ProcessIdleEvent() {
  if (_idlelist->GetHead()) {
    OIdleHandler *ih = (OIdleHandler *) _idlelist->GetHead()->data;
    UnregisterIdleEvent(ih); // _idlelist = GetHead()->next;
    ih->_c->HandleIdleEvent(ih);
    return True;
  }
  return False;
}

// Check the whole frame list and do all pending redrawings...

int OXClient::_DoRedraw() { 
  const OXSNode *e = _wlist->GetHead();
  OXWindow *w;
 
  if (_globalNeedRedraw == False) return False;
  while (e) {
    w = (OXWindow *) e->data;
    if (w->_needRedraw) {
      w->_DoRedraw();
      w->_needRedraw = False;
    }
    e = e->next;
  }

  _globalNeedRedraw = False;
  return True;
}

void OXClient::FlushRedraw() {
#if 1
  XEvent event;
  while (XCheckMaskEvent(GetDisplay(), ExposureMask, &event))
    HandleEvent(&event);
#endif
  _DoRedraw();
  XSync(GetDisplay(), False);
}

int OXClient::HandleEvent(XEvent *Event) {
  OXWindow *w;
  
  if ((w = GetWindowById(Event->xany.window)) == NULL) return False;

  w->HandleEvent(Event);
  return True;
}

int OXClient::HandleMaskEvent(XEvent *Event, Window wid) {
  OXWindow *w, *ptr;

  if ((w = GetWindowById(Event->xany.window)) == NULL) return False;

  if ((Event->type != ButtonPress) &&
      (Event->type != ButtonRelease) &&
      (Event->type != KeyPress) &&
      (Event->type != KeyRelease) &&
#if 0
      (Event->type != EnterNotify) &&
      (Event->type != LeaveNotify) &&
#endif
      (Event->type != MotionNotify)) {

    w->HandleEvent(Event);
    return True;

  } else {

    // This breaks class member protection, but OXClient is a friend of all
    // classes and _should_ know what to do and what *not* to do...

    for (ptr = w; ptr != NULL; ptr = (OXWindow *) ptr->_parent) {
      if (ptr->_id == wid) {
        w->HandleEvent(Event);
        return True;
      }
    }
    if (w->HandleMaskEvent(Event)) {
      w->HandleEvent(Event);
      return True;
    }
  }

  if (Event->type == ButtonPress ||
      Event->type == KeyPress)
    XBell(_dpy, 0);

  return False;
}

OXWindow *OXClient::GetWindowById(Window wid) const {
  OXSNode *e;

  e = _wlist->GetNode(wid);
  if (!e) return NULL;
  return ((OXWindow *) e->data);
}

void OXClient::StartTip(OXFrame * w) {
  if (_tip_status == 2) {
    if (_tip_timer) {
      delete _tip_timer;
      _tip_timer = NULL;
    }
  } else {
    _tip_frame = w;
    if (_tip_timer)
      delete _tip_timer;
    _tip_timer = new OTimer(w, 1500);
  }
}

void OXClient::StopTip(OXFrame * w) {
  if (_tip_status == 2) {
    if (!_tip_timer)
      _tip_timer = new OTimer(w, 200);
  } else if (_tip_timer) {
    delete _tip_timer;
    _tip_timer = NULL;
  }
}

void OXClient::ReloadResources() {

  _resourcePool->Reload();

  const OXSNode *e = _mlist->GetHead();
  OXWindow *w;

  while (e) {
    w = (OXWindow *) e->data;
    w->Reconfig();
    e = e->next;
  }
}
