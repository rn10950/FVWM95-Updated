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

#ifndef __OXCLIENT_H
#define __OXCLIENT_H

#include <X11/Xlib.h>

#include <xclass/utils.h>
#include <xclass/OBaseObject.h>
#include <xclass/OXSList.h>

#undef NULL
#define NULL 0

#ifdef __hpux
#define FD_MASK int
#else
#define FD_MASK fd_set
#endif

#define XCE_X_EVENT       (1<<0)
#define XCE_FILE_EVENT    (1<<1)
#define XCE_TIMER_EVENT   (1<<2)
#define XCE_IDLE_EVENT    (1<<3)
#define XCE_ALL_EVENTS    (XCE_X_EVENT | XCE_FILE_EVENT | \
                           XCE_TIMER_EVENT | XCE_IDLE_EVENT)
#define XCE_DONT_WAIT     (1<<4)


extern Atom WM_DELETE_WINDOW;
extern Atom _MOTIF_WM_HINTS;
extern Atom _XCLASS_MESSAGE;
extern Atom _XCLASS_COLORS;
extern Atom _XCLASS_RESOURCES;


// Forward declarations to avoid include files headaches :-(

class OXWindow;
class OXFrame;
class OColor;
class OGCPool;
class OXGC;
class OPicturePool;
class OPicture;
class OFontPool;
class OResourcePool;
class OXFont;
class OTimer;
class OFileHandler;
class OIdleHandler;
class OXSTimerList;


//----------------------------------------------------------------------
// X-Window client. An OXClient is an exception in that is not a real OX
// object (no XID member)

class OXClient : public OBaseObject {
public:
  OXClient(int argc, char *argv[]);
  OXClient(const char *DpyName = "") { _Init(DpyName); }
  virtual ~OXClient();

  Atom RegisterXAtom(const char *name, int make = false)
       { return XInternAtom(_dpy, name, make); };
  Display *GetDisplay() const { return _dpy; }
  int GetScreenNumber() const { return _screen; }
  int GetDisplayWidth() const { return XDisplayWidth(_dpy, _screen); }
  int GetDisplayHeight() const { return XDisplayHeight(_dpy, _screen); }
  int GetDisplayDepth() const { return _depth; }
  const OXWindow *GetRoot() const { return _Root; }
  OXWindow *GetWindowById(Window sw) const;

  unsigned long GetColor(OColor color) const;
  unsigned long GetColor(unsigned long color) const;
  unsigned long GetColorByName(const char *name) const;
  unsigned long GetHilite(unsigned long base_color) const;
  unsigned long GetShadow(unsigned long base_color) const;
  void FreeColor(unsigned long pixel) const;

  Colormap GetDefaultColormap() const { return _defaultColormap; }

  void NeedRedraw(OXWindow *w);

  virtual int RegisterWindow(OXWindow *w, char *name = NULL);
  virtual int UnregisterWindow(OXWindow *w);
  virtual int RegisterTopLevel(OXWindow *w, char *name = NULL);
  virtual int UnregisterTopLevel(OXWindow *w);

  int RegisterTimer(OTimer *t);
  int UnregisterTimer(OTimer *t);
  int RegisterFileEvent(OFileHandler *fh);
  int UnregisterFileEvent(OFileHandler *fh);
  int RegisterIdleEvent(OIdleHandler *ih);
  int UnregisterIdleEvent(OIdleHandler *ih);

  void Run();
  void WaitFor(OXWindow *w);
  void WaitForUnmap(OXWindow *w);
  int  ProcessOneEvent();

  void FlushRedraw();

  const OResourcePool *GetResourcePool() const { return _resourcePool; }

  void ReloadResources();

  const OPicture *GetPicture(const char *name);
  const OPicture *GetPicture(const char *name, char **data);
  const OPicture *GetPicture(const char *name, int new_width, int new_height);
  const OPicture *GetPicture(const char *name, char **data,
                                        int new_width, int new_height);
  const OPicture *GetPicture(const OPicture *pic);
  void  FreePicture(const OPicture *pic);

  const OXGC *GetGC(Drawable d, unsigned long mask, XGCValues *values);
  void  FreeGC(const OXGC *gc);
  void  FreeGC(const GC gc);

  OXFont *GetFont(const char *string);
  OXFont *GetFont(const char *family, int ptsize, int weight, int slant);
  OXFont *GetFont(OXFont *font);
  void   FreeFont(OXFont *font);
  OFontPool *GetFontPool() const { return _fontPool; }

  void StartTip(OXFrame *w);
  void StopTip(OXFrame *w);
  int  GetTipStatus() { return _tip_status; }

protected:
  void _Init(const char *DpyName = NULL);

  void MainLoop();
  int  ProcessOneEvent(int EventType, Window wid);
  int  HandleEvent(XEvent *Event);
  int  HandleMaskEvent(XEvent *Event, Window wid);
  int  _DoRedraw();

  int  _ProcessXEvent(int EventType, Window wid);
  int  _MakeFDSets();
  int  _ProcessFileEvent();
  int  _ProcessTimerEvent();
  int  _ProcessIdleEvent();

  Display  *_dpy;
  int _screen, _depth;
  OXWindow *_Root;
  Colormap _defaultColormap;

  int _x_fd;
  OPicturePool *_picturePool;
  OGCPool *_gcPool;
  OFontPool *_fontPool;
  OResourcePool *_resourcePool;
  int _globalNeedRedraw;        // at least one window needs to be redrawn
  OXSList *_wlist;              // list of frames
  OXSList *_mlist;              // list of top-level windows
  OXSList *_idlelist;           // list of idle events handlers
  OXSList *_iolist;             // list of file I/O handlers
  OXSList *_siglist;            // list of signal handlers
  OXSTimerList *_tlist;         // list of timers

  OTimer *_tip_timer;		// special timer for tips
  OXFrame *_tip_frame;		// currently tipped frame
  int _tip_status;

  fd_set readset, writeset, excptset;
};


#endif  // __OXCLIENT_H
