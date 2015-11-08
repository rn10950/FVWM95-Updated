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

#include <xclass/utils.h>
#include <xclass/OXDDListBox.h>
#include <xclass/OXButton.h>
#include <xclass/OXTextEntry.h>
#include <xclass/OXClient.h>
#include <xclass/OResourcePool.h>
#include <xclass/OString.h>

#include <X11/keysym.h>

#include "icons/arrow-down.xpm"

const OPicture *OXDDListBox::_bpic = NULL;


//-----------------------------------------------------------------

OXDDPopup::OXDDPopup(const OXWindow *p, int w, int h,
                     unsigned int options, unsigned long back) :
  OXCompositeFrame(p, w, h, options, back) {
    XSetWindowAttributes wattr;
    unsigned long mask;

    mask = CWOverrideRedirect | CWSaveUnder | CWBorderPixel;
    wattr.override_redirect = True;
    wattr.save_under = True;
    wattr.border_pixel = _blackPixel;
    XChangeWindowAttributes(GetDisplay(), _id, mask, &wattr);
    XSetWindowBorderWidth(GetDisplay(), _id, 1);

    _isDown = False;
    _kev = False;

    AddInput(StructureNotifyMask);
}

int OXDDPopup::HandleButton(XButtonEvent *event) {
  if (Contains(event->x, event->y)) {
    int evx = event->x;
    int evy = event->y;
    OXFrame *f = GetFrameFromPoint(evx, evy);

    int i;
    unsigned int u;
    Window w, root;
    XQueryPointer(GetDisplay(), f->GetId(), &root, &w, &i, &i, &i, &i, &u);
    event->subwindow = w;

    while (f && (f != this)) {
      int x, y;

      TranslateCoordinates(f, evx, evy, &x, &y);
      event->window = f->GetId();
      event->x = x;
      event->y = y;
      f->HandleButton(event);

      event->subwindow = f->GetId();
      f = (OXFrame *) f->GetParent();
    }
    return True;
  }
  if (event->type == ButtonRelease) {
    EndPopup();
    OListBoxMessage msg(MSG_DDLISTBOX, MSG_CLICK, False, 0);
    SendMessage(_msgObject, &msg);
  }
  return True;
}

int OXDDPopup::HandleMotion(XMotionEvent *event) {
  if (Contains(event->x, event->y)) {
    int evx = event->x;
    int evy = event->y;
    OXFrame *f = GetFrameFromPoint(evx, evy);

    int i;
    unsigned int u;
    Window w, root;
    XQueryPointer(GetDisplay(), f->GetId(), &root, &w, &i, &i, &i, &i, &u);
    event->subwindow = w;

    while (f && (f != this)) {
      int x, y;

      TranslateCoordinates(f, evx, evy, &x, &y);
      event->window = f->GetId();
      event->x = x;
      event->y = y;
      f->HandleMotion(event);

      event->subwindow = f->GetId();
      f = (OXFrame *) f->GetParent();
    }
  }
  return True;
}

int OXDDPopup::HandleKey(XKeyEvent *event) {

  int keysym = XLookupKeysym(event, 0);

  if (event->type == KeyPress) {
    switch (keysym) {
      case XK_F4:
      //case XK_Execute:
      case XK_Return:
      case XK_KP_Enter:
      case XK_Escape:
        if (_isDown) {
          EndPopup();
	  OListBoxMessage msg(MSG_DDLISTBOX, MSG_CLICK, keysym != XK_Escape, 0);
          SendMessage(_msgObject, &msg);
        }
        break;

      default:
        _kev = True;
        return _flist->frame->HandleKey(event);
    }
  } else {
    _kev = True;
    return _flist->frame->HandleKey(event);
  }

  return True;
}

int OXDDPopup::ProcessMessage(OMessage *msg) {

  switch (msg->type) {
    case MSG_LISTBOX:
      switch (msg->action) {
        case MSG_CLICK:
          if (!_kev) {
            EndPopup();
            OListBoxMessage msg(MSG_DDLISTBOX, MSG_CLICK, True, 0);
            SendMessage(_msgObject, &msg);
          }
          _kev = False;
          break;
      }
      break;
  }

  return True;
}

void OXDDPopup::EndPopup() {
  XUngrabPointer(GetDisplay(), CurrentTime);
  XUngrabKeyboard(GetDisplay(), CurrentTime);
  RemoveInput(KeyPressMask | KeyReleaseMask);
  UnmapWindow();
  _isDown = False;
}

void OXDDPopup::PlacePopup(int x, int y, int w, int h) {
  int rx, ry;
  unsigned int rw, rh;
  unsigned int dummy;
  Window wdummy;

  // Parent is root window for the popup:
  XGetGeometry(GetDisplay(), _parent->GetId(), &wdummy,
               &rx, &ry, &rw, &rh, &dummy, &dummy);

  if (x < 0) x = 0;
  if (x + _w > rw) x = rw - _w;
  if (y < 0) y = 0;
  if (y + _h > rh) y = rh - _h;

  MoveResize(x, y, w, h);
  MapSubwindows();
  Layout();
  MapRaised();

  AddInput(KeyPressMask | KeyReleaseMask);

  XGrabPointer(GetDisplay(), _id, False, //True,
               ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
               GrabModeAsync, GrabModeAsync, None,
               GetResourcePool()->GetGrabCursor(), CurrentTime);

  XGrabKeyboard(GetDisplay(), _id, False,
                GrabModeAsync, GrabModeAsync,
                CurrentTime);

  _isDown = True;

  _client->WaitForUnmap(this);
  EndPopup();
}

//-----------------------------------------------------------------

OXDDListBox::OXDDListBox(const OXWindow *p, int ID,
                         unsigned int options, unsigned long back) :
  OXCompositeFrame(p, 200, 10, options | OWN_BKGND, back) {

    _widgetID = ID;
    _widgetType = "OXDDListBox";
    _msgObject = p;

    if (!_bpic) {
      _bpic = _client->GetPicture("arrow-down.xpm", XCP_arrow_down);
    }

    _selentry = new OXTextLBEntry(this, new OString(""), 0);
    _ddbutton = new OXScrollBarElt(this, _bpic, SB_WIDTH, SB_WIDTH,
                                   RAISED_FRAME);

    _selentry->ChangeOptions(_selentry->GetOptions() | OWN_BKGND);

    AddFrame(_selentry, _lhs = new OLayoutHints(LHINTS_LEFT |
                                   LHINTS_EXPAND_Y | LHINTS_EXPAND_X));
                                   //0, 0, 1, 0));
    AddFrame(_ddbutton, _lhb = new OLayoutHints(LHINTS_RIGHT |
                                   LHINTS_EXPAND_Y));

    _ddframe = new OXDDPopup(_client->GetRoot(), 100, 100, VERTICAL_FRAME);

    _lb = new OXListBox(_ddframe, _widgetID, CHILD_FRAME);
    _lb->Resize(100, 100);
    _lb->Associate(_ddframe);
    _lb->TakeFocus(False);
    _ddframe->Associate(this);

    _ddframe->AddFrame(_lb, _lhdd = new OLayoutHints(LHINTS_EXPAND_X |
                                        LHINTS_EXPAND_Y));
    _ddframe->MapSubwindows();
    _ddframe->Resize(_ddframe->GetDefaultSize());

    XGrabButton(GetDisplay(), Button1, AnyModifier, _id, False,
                ButtonPressMask | ButtonReleaseMask,
                GrabModeAsync, GrabModeAsync, None, None);

    _message = NULL;
    AddInput(FocusChangeMask);
}

OXDDListBox::~OXDDListBox() {
  delete _ddframe;
  delete _lhs;
  delete _lhb;
  delete _lhdd;
  if (_message) delete _message;
}

ODimension OXDDListBox::GetDefaultSize() const {
  return ODimension (_w, _selentry->GetDefaultHeight() + _insets.t + _insets.b);
}

void OXDDListBox::SetTopEntry(OXLBEntry *e, OLayoutHints *lh) {
  RemoveFrame(_selentry);
  _selentry->DestroyWindow();
  delete _selentry;
  delete _lhs;
  _selentry = e;
  _lhs = lh;
  AddFrame(_selentry, _lhs);
  Layout();
}

void OXDDListBox::Select(int ID) {
  OXLBEntry *e;

  e = _lb->Select(ID);
  if (e) {
    _selentry->UpdateEntry(e);
    Layout();
  }
}

int OXDDListBox::HandleButton(XButtonEvent *event) {
  if (event->type == ButtonPress) {
    if (TakesFocus()) RequestFocus();
    if (event->subwindow == _ddbutton->GetId()) 
      _ddbutton->SetState(BUTTON_DOWN);
  } else {
    if (event->subwindow == _ddbutton->GetId()) 
      _PopDown();
  }
  return True;
}

int OXDDListBox::ProcessMessage(OMessage *msg) {
  OXLBEntry *e;
  OListBoxMessage *lbmsg;

  if (_message) delete _message;
  _message = NULL;

  switch (msg->type) {
    case MSG_DDLISTBOX:
      lbmsg = (OListBoxMessage *) msg;
      if (lbmsg->id == 0) return True;
      switch (msg->action) {
        case MSG_CLICK:
          e = _lb->GetSelectedEntry();
          if (!e) return True;
          _selentry->UpdateEntry(e);
          Layout();

          // We can have a potential problem here if we send a message as
          // this point and if, as a result of that action, the destination
          // widget creates some transient window which in order issues a
          // call to WaitFor. At this point we're still inside a
          // WaitForUnmap loop, and the WaitFor call will cause OXClient to
          // miss the UnmapNotify event for the OXDDPopup window, looping
          // here forever. So, as a solution, we create the message here and
          // defer the send operation until we just exit the WaitForUnmap
          // loop (i.e. after returning from PlacePopup).

	  _message = new OListBoxMessage(MSG_DDLISTBOX, MSG_CLICK,
                                         _widgetID, e->ID());
          break;
      }
      break;
  }
  return True;
}

void OXDDListBox::_LostFocus() {
  OXFrame::_LostFocus();
  RemoveInput(KeyPressMask | KeyReleaseMask);
  _selentry->Activate(False);
  _selentry->ShowFocusHilite(False); 

  // Shouldn't have to send a listbox msg for this..
  // maybe a focus message would be better..
  
  OListBoxMessage msg(MSG_DDLISTBOX, MSG_FOCUSLOST, _widgetID, -1);
  SendMessage(_msgObject, &msg);
}

void OXDDListBox::_GotFocus() {
  OXFrame::_GotFocus();
  AddInput(KeyPressMask | KeyReleaseMask);
  _selentry->Activate(True);
  _selentry->ShowFocusHilite(True);
}

void OXDDListBox::_PopDown() {
  int ax, ay;
  Window wdummy;

  _ddbutton->SetState(BUTTON_UP);
  XTranslateCoordinates(GetDisplay(), _id, (_ddframe->GetParent())->GetId(),
                        0, _h, &ax, &ay, &wdummy);

  _ddframe->PlacePopup(ax, ay, _w-2, _ddframe->GetDefaultHeight());
  if (_message) {
    SendMessage(_msgObject, _message);
    delete _message;
    _message = NULL;
  }
}

int OXDDListBox::HandleKey(XKeyEvent *event) {
  OXLBEntry *e = NULL;

  if (event->type != KeyPress) return True;
  
  int keysym = XLookupKeysym(event, 0);
  OListBoxMessage msg(MSG_DDLISTBOX, MSG_CLICK, _widgetID, 0);

  switch (keysym) {
    case XK_Up:
      _lb->MoveSelectUp(GetSelectedEntry());
      e = _lb->GetSelectedEntry();
      if (!e) {
        e = _lb->GetFirstEntry();
        if (e) _lb->Select(e->ID());
      }
      if (!e) return True;
      _selentry->UpdateEntry(e);
      Layout();
      msg.entryID = e->ID();
      SendMessage(_msgObject, &msg);
      break;

    case XK_Down: 
      _lb->MoveSelectDown(GetSelectedEntry());
      e = _lb->GetSelectedEntry();
      if (!e) {
        e = _lb->GetFirstEntry();
        if (e) _lb->Select(e->ID());
      }
      if (!e) return True;
      _selentry->UpdateEntry(e);
      Layout();
      msg.entryID = e->ID();
      SendMessage(_msgObject, &msg);
      break;

    case XK_F4:
      if (!_ddframe->IsDown()) _PopDown();
      break;

    case XK_Escape:
    //case XK_Execute:
    case XK_Return:
    case XK_KP_Enter:
    case XK_Tab:
      {
        OTextEntryMessage msg(MSG_TEXTENTRY, MSG_TEXTCHANGED, _widgetID,
                              keysym);
        SendMessage(_msgObject, &msg);
      }
      break;

   }
   return true;
}

void OXDDListBox::Reconfig() {
  OXCompositeFrame::Reconfig();
  _ddframe->Reconfig();
}
