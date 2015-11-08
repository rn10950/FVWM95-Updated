/**************************************************************************
 
    This file is part of OXMdi an extension to the xclass toolkit.
    Copyright (C) 1998-2002 by Harald Radke, Hector Peraza.
 
    This application is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
 
    This application is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.
 
    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 
**************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include <X11/keysym.h>
#include <X11/cursorfont.h>

#include <xclass/OXMainFrame.h>
#include <xclass/OXMdiMainFrame.h>
#include <xclass/OXMdiDecorFrame.h>
#include <xclass/OXMdiFrame.h>
#include <xclass/OXMdiMenu.h>
#include <xclass/OGC.h>
#include <xclass/OResourcePool.h>


//----------------------------------------------------------------------

OXMdiMainFrame::OXMdiMainFrame(const OXWindow *p, OXMdiMenuBar *menuBar,
                               int w, int h, unsigned int options,
                               unsigned long back) :
  OXCanvas(p, w, h, options | DOUBLE_BORDER | SUNKEN_FRAME | MDI_MAIN_FRAME) {

  // [we should use the specified back color -- HP]

  _container = new OXMdiContainer(this, 10, 10, OWN_BKGND,
                        _client->GetShadow(_defaultFrameBackground));
  SetContainer(_container);

  _numberOfFrames = 0;
  _menuBar = menuBar;
  _children = NULL;
  _current = NULL;

  const OResourcePool *res = GetResourcePool();
  bgCurrent = res->GetSelectedBgndColor();
  fgCurrent = res->GetSelectedFgndColor();
  fgNotCurrent = res->GetFrameBgndColor();
  bgNotCurrent = res->GetFrameShadowColor();
  _fontCurrent = res->GetMenuHiliteFont();
  _fontNotCurrent = _fontCurrent; //res->GetMenuFont();

  XGCValues gval;
  unsigned long gmask = GCForeground | GCBackground | GCFunction |
                        GCLineWidth | GCSubwindowMode | GCPlaneMask |
                        GCStipple | GCFillStyle;
  gval.foreground = fgNotCurrent;
  gval.background = bgNotCurrent;
  gval.function = /*GXinvert;*/GXxor;
  gval.line_width = MDI_BORDER_WIDTH;
  gval.subwindow_mode = IncludeInferiors;
  gval.plane_mask = AllPlanes;
  gval.stipple = _client->GetResourcePool()->GetCheckeredBitmap();
  gval.fill_style = FillOpaqueStippled;

  // must be a private (non-shared) GC!
  _boxGC = new OXGC(GetDisplay(), _id, gmask, &gval);

  _current_x = _current_y = 0;
  _resizeMode = MDI_DEFAULT_RESIZE_MODE;

  _winListMenu = new OXPopupMenu(_client->GetRoot());

  if (_toplevel) {
    OXMainFrame *main = (OXMainFrame *) _toplevel;
    int keycode = XKeysymToKeycode(GetDisplay(), XK_Tab);

    main->BindKey(this, keycode, ControlMask);
    main->BindKey(this, keycode, ControlMask | ShiftMask);

    keycode = XKeysymToKeycode(GetDisplay(), XK_F4);

    main->BindKey(this, keycode, ControlMask);

    _flags |= HANDLES_TAB;
  }

  MapSubwindows();
  Layout();
  MapWindow();
}

OXMdiMainFrame::~OXMdiMainFrame() {
  OMdiFrameList *tmp, *travel = _children;

  while (travel) {
    tmp = travel->next;
    delete travel;
    travel = tmp;
  }

  if (_fontCurrent) _client->FreeFont((OXFont *)_fontCurrent);
  if (_fontNotCurrent) _client->FreeFont((OXFont *)_fontNotCurrent);
  delete _boxGC;

  if (_toplevel) {
    OXMainFrame *main = (OXMainFrame *) _toplevel;
    int keycode = XKeysymToKeycode(GetDisplay(), XK_Tab);

    main->RemoveBind(this, keycode, ControlMask);
    main->RemoveBind(this, keycode, ControlMask | ShiftMask);

    keycode = XKeysymToKeycode(GetDisplay(), XK_F4);

    main->RemoveBind(this, keycode, ControlMask);
  }
}

void OXMdiMainFrame::SetResizeMode(int mode) {
  OMdiFrameList *travel;

  _resizeMode = mode;
  for (travel = _children; travel; travel = travel->next) {
    travel->decor->SetResizeMode(mode);
  }
}

int OXMdiMainFrame::HandleClientMessage(XClientMessageEvent *event) {

  OWidgetMessage msg(MSG_MDI, MDI_CURRENT, event->data.l[0]);
  SendMessage(_parent, &msg);

  return True;
}

int OXMdiMainFrame::HandleKey(XKeyEvent *event) {
  if (event->type == KeyPress) {
    if (XLookupKeysym(event, 0) == XK_Tab) {
      if (event->state & ControlMask) {
        if (event->state & ShiftMask) {
          CirculateUp();
        } else {
          CirculateDown();
        }
        return True;
      }
    } else if (XLookupKeysym(event, 0) == XK_F4) {
      if (event->state & ControlMask) {
        Close(GetCurrent());
        return True;
      }
    }
  }
  return False;
}

void OXMdiMainFrame::AddMdiFrame(OXMdiFrame *frame) {
  OMdiFrameList *travel;

  frame->UnmapWindow();

  travel = new OMdiFrameList;
  travel->cyclePrev = travel->cycleNext = travel;
  travel->prev = NULL;
  if (_children) _children->prev = travel;
  travel->next = _children;
  _children = travel;

  travel->decor = new OXMdiDecorFrame(this, frame, frame->GetWidth(),
                                      frame->GetHeight(), _boxGC);

  travel->frameid = frame->GetId();
  travel->decor->SetResizeMode(_resizeMode);

  if (_current_x + travel->decor->_w > _w) _current_x = 0;
  if (_current_y + travel->decor->_h > _h) _current_y = 0;
  travel->decor->Move(_current_x, _current_y);

  _current_x += travel->decor->_titlebar->_h + _bw * 2;
  _current_y += travel->decor->_titlebar->_h + _bw * 2;

  _numberOfFrames++;

  UpdateWinListMenu();
  SetCurrent(travel);
  Layout();

  OWidgetMessage msg(MSG_MDI, MDI_CREATE, travel->decor->GetId());
  SendMessage(_parent, &msg);      
}

bool OXMdiMainFrame::RemoveMdiFrame(OXMdiFrame *frame) {
  OMdiFrameList *travel = _children;

  while (travel && (travel->frameid != frame->GetId())) travel = travel->next;
  if (!travel) return False;

  if (travel == _current) _current = NULL;

  // unlink the element from the cycle list
  travel->cyclePrev->cycleNext = travel->cycleNext;
  travel->cycleNext->cyclePrev = travel->cyclePrev;

  // and from the main list
  if (travel->next) {
    travel->next->prev = travel->prev;
  }
  if (travel->prev) {
    travel->prev->next = travel->next;
  } else {
    _children = travel->next;
  }

  if (!_current) {
    if (_children) SetCurrent(travel->cyclePrev);
  }

  travel->decor->RemoveFrame(frame);
  //frame->DestroyWindow();

  XID old_id = travel->decor->GetId();

  delete travel->decor;
  delete travel;

  _numberOfFrames--;

  UpdateWinListMenu();
  Layout();

  OWidgetMessage msg(MSG_MDI, MDI_CLOSE, old_id);
  SendMessage(_parent, &msg);

  return True;
}

bool OXMdiMainFrame::SetCurrent(XID id) {
  if (_current && (_current->decor->GetId() == id)) {
    _current->decor->RaiseWindow();
    return True;
  }

  OMdiFrameList *travel = _children;
  while (travel && (travel->decor->GetId() != id)) travel = travel->next;
  if (!travel) return False;

  return SetCurrent(travel);
}

bool OXMdiMainFrame::SetCurrent(OXMdiFrame *f) {
  if (_current && (_current->decor->_frame == f)) {
    _current->decor->RaiseWindow();
    return True;
  }

  OMdiFrameList *travel = _children;
  while (travel && (travel->decor->_frame != f)) travel = travel->next;
  if (!travel) return False;

  return SetCurrent(travel);
}

bool OXMdiMainFrame::SetCurrent(OMdiFrameList *newcurrent) {

  if (_current && (_current == newcurrent)) {
    _current->decor->RaiseWindow();
    return True;
  }

  if (_current) {
    if (!_current->decor->IsMaximized())
      _current->decor->_titlebar->SetTitleBarColors(fgNotCurrent,
                                                    bgNotCurrent,
                                                    _fontNotCurrent);
    _current->decor->SetGrab();
  }

  if (newcurrent) {
    if (_current) {
      // unlink the element from the old position
      newcurrent->cyclePrev->cycleNext = newcurrent->cycleNext;
      newcurrent->cycleNext->cyclePrev = newcurrent->cyclePrev;
      // and link it to the top of the window cycle stack
      newcurrent->cyclePrev = _current;
      newcurrent->cycleNext = _current->cycleNext;
      _current->cycleNext = newcurrent;
      newcurrent->cycleNext->cyclePrev = newcurrent;
    } else {
      // no current? well, put it at the head of the list...
      if (_children && newcurrent != _children) {
        // unlink the element from the old position
        newcurrent->cyclePrev->cycleNext = newcurrent->cycleNext;
        newcurrent->cycleNext->cyclePrev = newcurrent->cyclePrev;
        // and link it to the beginning of the window list
        newcurrent->cyclePrev = _children;
        newcurrent->cycleNext = _children->cycleNext;
        _children->cycleNext = newcurrent;
        newcurrent->cycleNext->cyclePrev = newcurrent;
      }
    }
  }

  _current = newcurrent;

  if (_current) {
    if (!_current->decor->IsMaximized())
      _current->decor->_titlebar->SetTitleBarColors(fgCurrent,
                                                    bgCurrent,
                                                    _fontCurrent);
    _current->decor->UnsetGrab();
  }

  _current->decor->RaiseWindow();

  // send a client message to myself
  // [what for? -- HP]

  XEvent xevent;
  xevent.xany.type = ClientMessage;
  xevent.xany.display = GetDisplay();
  xevent.xclient.window = _id;
  xevent.xclient.format = 32;
  xevent.xclient.data.l[0] = _current->decor->GetId();
  XSendEvent(GetDisplay(), _id, 0, 0, &xevent);

  _winListMenu->RCheckEntry(_current->decor->GetId(), 0, INT_MAX);

  return True;
}

// Bring the lowest window to the top

void OXMdiMainFrame::CirculateUp() {
  if (_current) {
    _current->decor->_titlebar->SetTitleBarColors(fgNotCurrent,
                                                  bgNotCurrent,
                                                  _fontNotCurrent);

    _current = _current->cycleNext;

    _current->decor->RaiseWindow();
    _current->decor->_titlebar->SetTitleBarColors(fgCurrent,
                                                  bgCurrent,
                                                  _fontCurrent);

  } else if (_children) {
    SetCurrent(_children);
  }
}

// Send the highest window to the bottom

void OXMdiMainFrame::CirculateDown() {
  if (_current) {
    _current->decor->LowerWindow();
    _current->decor->_titlebar->SetTitleBarColors(fgNotCurrent,
                                                  bgNotCurrent,
                                                  _fontNotCurrent);

    _current = _current->cyclePrev;   // do not call SetCurrent in order
                                      // to not to alter the stacking order
    _current->decor->RaiseWindow();
    _current->decor->_titlebar->SetTitleBarColors(fgCurrent,
                                                  bgCurrent,
                                                  _fontCurrent);

  } else if (_children) {
    SetCurrent(_children);
  }
}

OXMdiDecorFrame *OXMdiMainFrame::GetDecorFrame(OXMdiFrame *frame) const {
  OMdiFrameList *travel = _children;
  while (travel && (travel->decor->_frame != frame)) travel = travel->next;
  if (!travel) return NULL;
  return travel->decor;
}

OXMdiDecorFrame *OXMdiMainFrame::GetDecorFrame(XID id) const {
  OMdiFrameList *travel = _children;
  while (travel && (travel->decor->_id != id)) travel = travel->next;
  if (!travel) return NULL;
  return travel->decor;
}

OXMdiFrame *OXMdiMainFrame::GetMdiFrame(XID id) const {
  OXMdiDecorFrame *frame = GetDecorFrame(id);
  if (!frame) return NULL;
  return frame->_frame;
}

ORectangle OXMdiMainFrame::GetBBox() const {
  if (_current && _current->decor->IsMaximized()) {
    return ORectangle(0, 0, _w - 2 * _bw, _h - 2 * _bw);
  } else {
    ORectangle rect;
    OMdiFrameList *travel;

    for (travel = _children; travel; travel = travel->next) {
      ORectangle wrect(travel->decor->GetPosition(),
                       travel->decor->GetSize());
      rect.merge(wrect);
    }

    return rect;
  }
}

ORectangle OXMdiMainFrame::GetMinimizedBBox() const {
  ORectangle rect;
  OMdiFrameList *travel;
  int first = True;

  for (travel = _children; travel; travel = travel->next) {
    if (travel->decor->IsMinimized()) {
      ORectangle wrect(travel->decor->GetPosition(),
                       travel->decor->GetSize());
      if (first) rect = wrect; else rect.merge(wrect);
      first = False;
    }
  }

  return rect;
}

void OXMdiMainFrame::UpdateWinListMenu() {
  char buf[256], scut;
  OMdiFrameList *travel;
  const OPicture *pic;

  _winListMenu->RemoveAllEntries();
  scut = '0';

  if (!_children) {
    _winListMenu->AddEntry(new OHotString("(None)"), 1000);
    _winListMenu->DisableEntry(1000);
    return;
  }

  for (travel = _children; travel; travel = travel->next) {
    scut++;
    if (scut == ('9' + 1)) scut = 'A';
    sprintf(buf, "&%c. %s", scut, travel->decor->GetWindowName());
    if (travel->decor->_buttonMask & MDI_MENU)
      pic = travel->decor->GetWindowIcon();
    else
      pic = NULL;
    _winListMenu->AddEntry(new OHotString(buf), travel->decor->GetId(), pic);
  }

  if (_current)
    _winListMenu->RCheckEntry(_current->decor->GetId(), 0, INT_MAX);
}

void OXMdiMainFrame::Layout() {
  OXCanvas::Layout();
  if (_current && _current->decor->IsMaximized())
    _current->decor->MoveResize(0, 0, _w - 2 *_bw, _h - 2 * _bw);
}

void OXMdiMainFrame::ArrangeFrames(int mode) {
  int factor_x = 0;
  int factor_y = 0;
  int num_mapped = 0;
  int x = 0;
  int y = 0;
  int w = _w - 2 * _bw;  //GetContainer()->GetWidth();
  int h = _h - 2 * _bw;  //GetContainer()->GetHeight();

  OMdiFrameList *tmp, *travel;

  for (travel = _children; travel; travel = travel->next) {
    if (travel->decor->IsMaximized())
      Restore(travel->decor->_frame);
    if (!travel->decor->IsMinimized())
      ++num_mapped;
  }

  // must also restore view to 0,0
  GetViewPort()->SetHPos(0);
  GetViewPort()->SetVPos(0);

  ArrangeMinimized();

  travel = _children;

  if (num_mapped == 0) return;

  ORectangle irect = GetMinimizedBBox();
  h -= irect.h;

  switch (mode) {
    case MDI_TILE_HORIZONTAL:
      factor_y = h / num_mapped;
      for (travel = _children; travel; travel = travel->next) {
        if (!travel->decor->IsMinimized()) {
          travel->decor->MoveResize(x, y, w, factor_y);
          y = y + factor_y;
        }
      }
      break;

    case MDI_TILE_VERTICAL:
      factor_x = w / num_mapped;
      for (travel = _children; travel; travel = travel->next) {
        if (!travel->decor->IsMinimized()) {
          travel->decor->MoveResize(x, y, factor_x, h);
          x = x + factor_x;
        }
      }
      break;

    case MDI_CASCADE:
      y = travel->decor->_titlebar->GetX() +
          travel->decor->_titlebar->GetHeight();
      x = y;
      factor_y = (h * 2) / 3;
      factor_x = (w * 2) / 3;

      travel = _current;
      if (!travel) travel = _children;
      tmp = travel;
      if (travel) {
        do {
          travel = travel->cycleNext;
          if (!travel->decor->IsMinimized()) {
            travel->decor->MoveResize(x - y, x - y, factor_x, factor_y);
            x += y;
          }
        } while (travel != tmp);
      }
      break;
  }

  Layout();
}

// This is an attempt to an "smart" minimized window re-arrangement.

void OXMdiMainFrame::ArrangeMinimized() {
  OMdiFrameList *travel, *closest;
  int x, y, w, h;

  bool arranged = True;

  for (travel = _children; travel && arranged; travel = travel->next)
    if (travel->decor->IsMinimized()) arranged = False;

  // return if there is nothing to do

  if (arranged || !_children) return;

  h = _children->decor->_titlebar->GetDefaultHeight() 
      + 2 * _children->decor->_bw;
  w = MINIMIZED_WIDTH * h;

  x = 0;
  y = GetViewPort()->GetHeight() - h;

  // we'll use the _minimizedUserPlacement variable as a "not arranged" flag

  for (travel = _children; travel; travel = travel->next)
    travel->decor->_minimizedUserPlacement = True;

  do {
    closest = NULL;
    int cdist;
    for (travel = _children; travel; travel = travel->next) {
      if (travel->decor->IsMinimized()) {
        if (travel->decor->_minimizedUserPlacement) {
          int dx = travel->decor->GetX() - x;
          int dy = y - travel->decor->GetY();
          int dist = dx * dx + dy * dy;
          if (!closest || (dist < cdist)) {
            closest = travel;
            cdist = dist;
          }
        }
      }
    }

    if (closest) {
      closest->decor->_minimizedX = x;
      closest->decor->_minimizedY = y;
      closest->decor->MoveResize(x, y, w, h);
      closest->decor->_minimizedUserPlacement = False;

      x += w;
      if (x + w > GetViewPort()->GetWidth()) {
        x = 0;
        y -= h;
      }
    }

  } while (closest);

  // reset the _minimizedUserPlacement settings for all windows

  for (travel = _children; travel; travel = travel->next)
    travel->decor->_minimizedUserPlacement = False;
}

int OXMdiMainFrame::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;

  switch (msg->type) {
    case MSG_MDI:
      SetCurrent(wmsg->id);
      switch (msg->action) {
        case MDI_MINIMIZE:
          Minimize(GetCurrent());
          break;

        case MDI_MAXIMIZE:
          Maximize(GetCurrent());
          break;

        case MDI_RESTORE:
          Restore(GetCurrent());
          break;

        case MDI_CLOSE:
          Close(GetCurrent());
          break;

        case MDI_MOVE:
          FreeMove(GetCurrent());  
          break;

        case MDI_SIZE:
          FreeSize(GetCurrent());
          break;

        case MDI_HELP:
          ContextHelp(GetCurrent());
          break;
      }
      break;

    default:
      return OXCanvas::ProcessMessage(msg);
  }

  return True;
}

void OXMdiMainFrame::Maximize(OXMdiFrame *mdiframe) {
  OXMdiDecorFrame *frame = GetDecorFrame(mdiframe);

  if (!frame) return;

  if (frame->_isMaximized) return;

  if (frame->_isMinimized) Restore(mdiframe);

  frame->SetDecorBorderWidth(0);
  frame->_preResizeX = frame->_x;
  frame->_preResizeY = frame->_y;
  frame->_preResizeWidth = frame->_w;
  frame->_preResizeHeight = frame->_h;
  frame->_upperHR->UnmapWindow();
  frame->_lowerHR->UnmapWindow();
  frame->_leftVR->UnmapWindow();
  frame->_rightVR->UnmapWindow();  
  frame->_upperLeftCR->UnmapWindow();
  frame->_upperRightCR->UnmapWindow();
  frame->_lowerLeftCR->UnmapWindow();
  frame->_lowerRightCR->UnmapWindow();

  frame->MoveResize(_bw, _bw, _w - 2 *_bw, _h - 2 * _bw);
  frame->_isMaximized = True;
  frame->_titlebar->LayoutButtons(frame->_buttonMask, frame->_isMinimized,
                                  frame->_isMaximized);
  frame->_titlebar->RemoveFrames(frame->_titlebar->_winIcon,
                                 frame->_titlebar->_buttons);
  frame->HideFrame(frame->_titlebar);

  if (_menuBar) {
    _menuBar->AddFrames(frame->_titlebar->_winIcon,
                        frame->_titlebar->_buttons);
    _menuBar->Layout();
  }

  OWidgetMessage msg(MSG_MDI, MDI_MAXIMIZE, frame->GetId());
  SendMessage(_parent, &msg);      

  Layout();
}

void OXMdiMainFrame::Restore(OXMdiFrame *mdiframe) {
  OXMdiDecorFrame *frame = GetDecorFrame(mdiframe);

  if (!frame) return;

  if (frame->_isMinimized == False && frame->_isMaximized == False) return;

  if (frame->_isMinimized) {
    frame->_minimizedX = frame->_x; 
    frame->_minimizedY = frame->_y; 
    frame->_isMinimized = False;
    SListFrameElt *ptr;
    for (ptr = frame->_flist->next; ptr != NULL; ptr = ptr->next) {   
      ptr->frame->SetVisible(true);
      ptr->frame->MapWindow();
    }
  } else if (frame->_isMaximized) {
    frame->SetDecorBorderWidth(MDI_BORDER_WIDTH);
    // shouldn't we better be calling MapSubwindows() here?
    frame->_upperHR->MapWindow();
    frame->_lowerHR->MapWindow();   
    frame->_leftVR->MapWindow();
    frame->_rightVR->MapWindow();
    frame->_upperLeftCR->MapWindow();
    frame->_upperRightCR->MapWindow();
    frame->_lowerLeftCR->MapWindow();
    frame->_lowerRightCR->MapWindow();

    if (_menuBar) {
      _menuBar->RemoveFrames(frame->_titlebar->_winIcon,
                             frame->_titlebar->_buttons);
      _menuBar->Layout();
    }

    frame->_titlebar->AddFrames(frame->_titlebar->_winIcon,
                                frame->_titlebar->_buttons);
    frame->ShowFrame(frame->_titlebar);
  }
  frame->_isMinimized = frame->_isMaximized = False;
  frame->_titlebar->LayoutButtons(frame->_buttonMask, False, False);
  frame->MoveResize(frame->_preResizeX, frame->_preResizeY,
                    frame->_preResizeWidth, frame->_preResizeHeight);

  OWidgetMessage msg(MSG_MDI, MDI_RESTORE, frame->GetId());
  SendMessage(_parent, &msg);

  Layout();
}

void OXMdiMainFrame::Minimize(OXMdiFrame *mdiframe) {
  int x, y, w, h;
  OXMdiDecorFrame *frame = GetDecorFrame(mdiframe);

  if (!frame) return;

  if (frame->_isMinimized) return;
 
  if (frame->_isMaximized) Restore(mdiframe);

  frame->_preResizeX = frame->_x;
  frame->_preResizeY = frame->_y;
  frame->_preResizeWidth = frame->_w;
  frame->_preResizeHeight = frame->_h;

  h = frame->_titlebar->GetDefaultHeight() + 2 * frame->_bw;
  w = MINIMIZED_WIDTH * h;

  if (!frame->_minimizedUserPlacement) {

    x = 0;
    y = GetViewPort()->GetHeight() - h;

    while (1) {
      OMdiFrameList *travel;
      bool taken = false;

      // find an empty spot...
      for (travel = _children; travel; travel = travel->next) {
        if (travel->decor->IsMinimized()) {
          OPosition p = travel->decor->GetPosition();
          ODimension s = travel->decor->GetSize();
          if ((x <= p.x + (int) s.w - 1) && (x + w - 1 >= p.x) &&
              (y <= p.y + (int) s.h - 1) && (y + h - 1 >= p.y)) {
            taken = true;
            break;
          }
        }
      }
      if (!taken) break;

      x += w;
      if (x + w > GetViewPort()->GetWidth()) {
        x = 0;
        y -= h;
      }
    }

    frame->_minimizedX = x;
    frame->_minimizedY = y;
  }

  frame->_isMinimized = True;

  SListFrameElt *ptr;
  for (ptr = frame->_flist->next; ptr != NULL; ptr = ptr->next) { 
    ptr->frame->SetVisible(false);
    ptr->frame->UnmapWindow();
  }
  frame->MoveResize(frame->_minimizedX, frame->_minimizedY, w, h);
  frame->LowerWindow();
  frame->_titlebar->LayoutButtons(frame->_buttonMask, frame->_isMinimized,
                                  frame->_isMaximized);
  frame->Layout();

  OWidgetMessage msg(MSG_MDI, MDI_MINIMIZE, frame->GetId());
  SendMessage(_parent, &msg);      

  Layout();
}

int OXMdiMainFrame::Close(OXMdiFrame *mdiframe) {
  OXMdiDecorFrame *frame = GetDecorFrame(mdiframe);
  Restore(mdiframe);
  return frame->CloseWindow();
}

void OXMdiMainFrame::FreeMove(OXMdiFrame *mdiframe) {
  OXMdiDecorFrame *frame = GetDecorFrame(mdiframe);
  if (!frame) return;

  int x = frame->_titlebar->_w / 2;
  int y = frame->_titlebar->_h / 2;

  XWarpPointer(GetDisplay(), None, frame->_titlebar->_id,
               0, 0, 0, 0, x, y);

  frame->_titlebar->buttonLeftPressed = True;
  frame->_titlebar->x0 = x;
  frame->_titlebar->y0 = y;
  Cursor cursor = XCreateFontCursor(GetDisplay(), XC_fleur);

  XGrabPointer(GetDisplay(), frame->_titlebar->_id, False,
               ButtonReleaseMask | PointerMotionMask,
	       GrabModeAsync, GrabModeAsync, None, cursor, CurrentTime);
}

void OXMdiMainFrame::FreeSize(OXMdiFrame *mdiframe) {
  OXMdiDecorFrame *frame = GetDecorFrame(mdiframe);
  if (!frame) return;

  int x = frame->_lowerRightCR->_w - 2;
  int y = frame->_lowerRightCR->_h - 2;


  XWarpPointer(GetDisplay(), None, frame->_lowerRightCR->_id, 
               0, 0, 0, 0, x, y); 

  XButtonEvent event;

  int xroot, yroot;
  Window win;

  XTranslateCoordinates(GetDisplay(), frame->_lowerRightCR->GetId(),
                        _client->GetRoot()->GetId(), x, y, &xroot, &yroot,
                        &win);

  event.type = ButtonPress;
  event.window = frame->_lowerRightCR->GetId();
  event.button = Button1;
  event.x = x;
  event.y = y;
  event.x_root = xroot;
  event.y_root = yroot;
  
  Cursor cursor = XCreateFontCursor(GetDisplay(), XC_bottom_right_corner);

  XGrabPointer(GetDisplay(), frame->_lowerRightCR->_id, False,
               ButtonReleaseMask | PointerMotionMask,
	       GrabModeAsync, GrabModeAsync, None, cursor, CurrentTime);

  frame->_lowerRightCR->HandleButton(&event);
}

int OXMdiMainFrame::ContextHelp(OXMdiFrame *mdiframe) {
  if (mdiframe)
    return mdiframe->Help();
  else
    return False;
}

OXMdiFrame *OXMdiMainFrame::GetCurrent() const {
  if (_current)
    return _current->decor->_frame;
  else
    return NULL;
}

OMdiGeometry OXMdiMainFrame::GetWindowGeometry(OXMdiFrame *f) const {
  OMdiGeometry geom;

  geom.value_mask = 0;

  const OXMdiDecorFrame *frame = GetDecorFrame(f);
  if (frame) {
    int th = frame->_titlebar->GetDefaultHeight();
    int bw = frame->_bw;

    if (frame->_isMinimized || frame->_isMaximized) {
      geom.decoration = ORectangle(frame->_preResizeX,
                                   frame->_preResizeY,
                                   (unsigned) frame->_preResizeWidth,
                                   (unsigned) frame->_preResizeHeight);
    } else {
      geom.decoration = ORectangle(frame->_x,
                                   frame->_y,
                                   (unsigned) frame->_w,
                                   (unsigned) frame->_h);
    }
    geom.value_mask |= MDI_DECOR_GEOMETRY;

    geom.client = ORectangle(geom.decoration.x + bw,
                             geom.decoration.y + bw + th,
                             (unsigned) (geom.decoration.w - 2 * bw),
                             (unsigned) (geom.decoration.h - 2 * bw - th));
    geom.value_mask |= MDI_CLIENT_GEOMETRY;

    if (frame->_minimizedUserPlacement) {
      int mh = th + 2 * bw;
      int mw = MINIMIZED_WIDTH * mh;

      geom.icon = ORectangle(frame->_minimizedX,
                             frame->_minimizedY,
                             (unsigned) mw,
                             (unsigned) mh);
      geom.value_mask |= MDI_ICON_GEOMETRY;
    }

  }

  return geom;
}

void OXMdiMainFrame::ConfigureWindow(OXMdiFrame *f, OMdiGeometry &geom) {
  OXMdiDecorFrame *frame = GetDecorFrame(f);
  if (frame) {
    if (geom.value_mask & MDI_DECOR_GEOMETRY) {
      if (frame->_isMinimized || frame->_isMaximized) {
        frame->_preResizeX = geom.decoration.x;
        frame->_preResizeY = geom.decoration.y;
        frame->_preResizeWidth = geom.decoration.w;
        frame->_preResizeHeight = geom.decoration.h;
      } else {
        frame->MoveResize(geom.decoration.x, geom.decoration.y,
                          geom.decoration.w, geom.decoration.h);
      }
    } else if (geom.value_mask & MDI_CLIENT_GEOMETRY) {
    }
    if (geom.value_mask & MDI_ICON_GEOMETRY) {
      frame->_minimizedX = geom.icon.x;
      frame->_minimizedY = geom.icon.y;
      frame->_minimizedUserPlacement = True;
      if (frame->_isMinimized)
        frame->Move(frame->_minimizedX, frame->_minimizedY);
    }
    Layout();
  }
}

bool OXMdiMainFrame::IsMaximized(OXMdiFrame *f) {
  OXMdiDecorFrame *frame = GetDecorFrame(f);
  if (frame) return frame->IsMaximized();
  return false;
}

bool OXMdiMainFrame::IsMinimized(OXMdiFrame *f) {
  OXMdiDecorFrame *frame = GetDecorFrame(f);
  if (frame) return frame->IsMinimized();
  return false;
}

//----------------------------------------------------------------------

OXMdiContainer::OXMdiContainer(const OXMdiMainFrame *p, int w, int h,
                               unsigned int options, unsigned long back) :
  OXFrame(p->GetViewPort(), w, h, options, back) {

    _main = p;

    AddInput(SubstructureNotifyMask);
    _compressConfigureEvents = True;
}

ODimension OXMdiContainer::GetDefaultSize() const {
  ORectangle rect = _main->GetBBox();

  int xpos = -_main->GetViewPort()->GetHPos();
  int ypos = -_main->GetViewPort()->GetVPos();

  return ODimension(max(xpos + _w, rect.right_bottom().x + 1),
                    max(ypos + _h, rect.right_bottom().y + 1));
}

int OXMdiContainer::HandleConfigureNotify(XConfigureEvent *event) {
  if (event->window != _id) {
    ORectangle rect = _main->GetBBox();

    int xpos = -_main->GetViewPort()->GetHPos();
    int ypos = -_main->GetViewPort()->GetVPos();

    int vw = _main->GetViewPort()->GetWidth();
    int vh = _main->GetViewPort()->GetHeight();

    int w = max(vw, rect.right_bottom().x + 1);
    int h = max(vh, rect.right_bottom().y + 1);

    if ((w != _w) || (h != _h)) {
      ((OXMainFrame *)_main)->Layout();
      return True;
    }
  }

  return False;
}
