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

#ifndef __ODNDMANAGER_H
#define __ODNDMANAGER_H

#include <X11/X.h>

#include <xclass/utils.h>
#include <xclass/OComponent.h>

class OXMainFrame;
class OXDragWindow;
class OTimer;


//----------------------------------------------------------------------

class ODNDdata : public OBaseObject {
public:
  ODNDdata(Atom dt = None, void *d = NULL, int len = 0, Atom act = None)
     { dataType = dt; data = d; dataLength = len; action = act; }
  ~ODNDdata() {}

  Atom dataType;
  Atom action;
  void *data;
  int dataLength;
};

class ODNDmanager : public OComponent {
protected:
  static int target_error;
  static XErrorHandler oldhandler;
  static int CatchXError(Display *dpy, XErrorEvent *err);

public:
  static Atom DNDaware, DNDselection, DNDproxy;
  static Atom DNDenter, DNDleave, DNDposition, DNDstatus;
  static Atom DNDdrop, DNDfinished;
  static Atom DNDversion;
  static Atom DNDactionCopy, DNDactionMove, DNDactionLink;
  static Atom DNDactionAsk, DNDactionPrivate;
  static Atom DNDtypeList, DNDactionList, DNDactionDescrip;
  static Atom _XC_DND_DATA;
        
protected:
  static int _init;
  static Atom _XA_WM_STATE;

public:
  ODNDmanager(OXClient *client, OXMainFrame *toplevel, Atom *typelist);
  virtual ~ODNDmanager();
  
  int HandleClientMessage(XClientMessageEvent *e);
  //int HandleDestroyNotify(XDestroyWindowEvent *e);
  int HandleSelectionRequest(XSelectionRequestEvent *event);
  int HandleSelection(XSelectionEvent *event);
  
  int HandleTimer(OTimer *t);
  
  //--- called by widgets

  void SetDragPixmap(Pixmap pic, Pixmap mask, int hot_x, int hot_y);

  int SetRootProxy();
  int RemoveRootProxy();

  int StartDrag(OXFrame *src, int x_root, int y_root, Window grabWin = None);
  int Drag(int x_root, int y_root, Atom action, Time timestamp);
  int Drop();
  int EndDrag();
  
  Window GetSource() const { return _source; }
  Window GetTarget() const { return _target; }
  
protected:
  void _InitAtoms();
  void _SetAware(Atom *typelist);
  void _SetTypeList(Atom *typelist);
  Window _GetRootProxy();
  Window _FindWindow(Window root, int x, int y, int maxd);
  int _IsDNDAware(Window win, Atom *typelist = NULL);
  int _IsTopLevel(Window win);

  void _SendDNDenter(Window target);
  void _SendDNDleave(Window target);
  void _SendDNDposition(Window target, int x, int y,
                        Atom action, Time timestamp);
  void _SendDNDstatus(Window target, Atom action);
  void _SendDNDdrop(Window target);
  void _SendDNDfinished(Window src);

  int _HandleDNDenter(Window src, long vers, Atom dataTypes[3]);
  int _HandleDNDleave(Window src);
  int _HandleDNDposition(Window src, int x_root, int y_root,
                         Atom action, Time timestamp);
  int _HandleDNDstatus(Window from, int accepted, 
                       XRectangle skip, Atom action);
  int _HandleDNDdrop(Window src, Time timestamp);
  int _HandleDNDfinished(Window target);

protected:
  OXMainFrame *_main;
  Atom _version;           // not really an Atom, but a long
  Atom *_typelist, *_draggerTypes, _dropType, _acceptedAction, _localAction;
  Display *_dpy;

  int _dragging, _dropAccepted, _statusPending, _useVersion, _proxy_ours;
  Window _source, _target;
  int _targetIsDNDaware;
  unsigned int _grabEventMask;
  OXFrame *_localSource, *_localTarget;

  OTimer *_dropTimeout;
  OXDragWindow *_dragWin;

  Pixmap _pic, _mask;
  int _hx, _hy;
  Cursor DND_NoDropCursor;
};


#endif  // __ODNDMANAGER_H
