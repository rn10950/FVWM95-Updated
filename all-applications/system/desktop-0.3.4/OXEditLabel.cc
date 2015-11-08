/**************************************************************************

    This file is part of xclass.
    Copyright (C) 1996, 1997 David Barth, Hector Peraza.

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

#include <X11/keysym.h>

#include <xclass/utils.h>
#include <xclass/ODimension.h>
#include <xclass/OResourcePool.h>

#include "OXEditLabel.h"


//----------------------------------------------------------------------

OXEditLabel::OXEditLabel(const OXWindow *p, OString *text) :
  OXCompositeFrame(p, 10, 10, HORIZONTAL_FRAME | RAISED_FRAME) {

  XSetWindowAttributes attr;
  unsigned long mask;

  mask = CWOverrideRedirect | CWSaveUnder;
  attr.override_redirect = True;
  attr.save_under = True;

  XChangeWindowAttributes(GetDisplay(), _id, mask, &attr);

  _bg = _client->GetResourcePool()->GetDocumentBgndColor();
  _fg = _client->GetResourcePool()->GetDocumentFgndColor();

  SetBackgroundColor(_bg);

  _te = new OXTextEntry(this, NULL, -1, OWN_BKGND);
  _te->AddText(0, text->GetString());
  _te->SetFont(_client->GetFont((OXFont *) _client->GetResourcePool()->GetIconFont()));
  _te->Associate(this);

  _oldName = new OString(text);

  _bdown = false;
  _autosize = false;

  AddInput(StructureNotifyMask);

  AddFrame(_te, _ll = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_TOP,
                                       2, 2, 0, 0));
  MapSubwindows();
  Resize(GetDefaultSize());
}

OXEditLabel::~OXEditLabel() {
  delete _ll;
  delete _oldName;
  XDestroyWindow(GetDisplay(), _id);
}

void OXEditLabel::DrawBorder() {
  DrawRectangle(_blackGC, 0, 0, _w-1, _h-1);
}

void OXEditLabel::SetText(OString *text) {
  _te->Clear();
  _te->AddText(0, text->GetString());
  delete _oldName;
  _oldName = new OString(text);
  Resize(GetDefaultSize());
}

void OXEditLabel::Show(int x, int y, int w, int h, bool autosize) {
  _autosize = true; autosize;
  if (_autosize) {
    _te->Resize(_te->GetTextSize());
    w = GetDefaultWidth();
    h = GetDefaultHeight();
  } else {
    w += 8;
  }
  x -= 4;
  MoveResize(x, y, w, h);
  MapWindow();
  XGrabPointer(GetDisplay(), _id, False,
               ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
               GrabModeAsync, GrabModeAsync,
               None, None, CurrentTime);
  XGrabKeyboard(GetDisplay(), _id, False,
                GrabModeAsync, GrabModeAsync,
                CurrentTime);
  _bdown = false;
  RaiseWindow();
  XSync(GetDisplay(), False);

  XButtonEvent ev;
  ev.window = _te->GetId();
  ev.x = 0;
  ev.y = 0;
  ev.type = ButtonPress;
  _te->HandleButton(&ev);
  ev.type = ButtonRelease;
  _te->HandleButton(&ev);
  ev.type = ButtonPress;
  _te->HandleDoubleClick(&ev);
  ev.type = ButtonRelease;
  _te->HandleButton(&ev);
  XSync(GetDisplay(), False);
}

void OXEditLabel::Hide() {
  XUngrabPointer(GetDisplay(), CurrentTime);
  XUngrabKeyboard(GetDisplay(), CurrentTime);
  XSync(GetDisplay(), False);
  UnmapWindow();
}

int OXEditLabel::HandleButton(XButtonEvent *event) {
  if ((event->subwindow == _te->GetId()) || _bdown) {
    event->window = _te->GetId();
    event->x -= _te->GetX();
    event->y -= _te->GetY();
    _bdown = (event->type == ButtonPress);
    return _te->HandleButton(event);
  } else if (event->subwindow != _id) {
    Hide();
  }
  return True;
}

int OXEditLabel::HandleDoubleClick(XButtonEvent *event) {
  if (event->subwindow == _te->GetId()) {
    event->window = _te->GetId();
    event->x -= _te->GetX();
    event->y -= _te->GetY();
    return _te->HandleDoubleClick(event);
  }
  return False;
}

int OXEditLabel::HandleMotion(XMotionEvent *event) {
  if (_bdown) {
    event->window = _te->GetId();
    event->x -= _te->GetX();
    event->y -= _te->GetY();
    _te->HandleMotion(event);
  }
  return True;
}

int OXEditLabel::HandleKey(XKeyEvent *event) {
  return _te->HandleKey(event);
}

int OXEditLabel::ProcessMessage(OMessage *msg) {
  OTextEntryMessage *temsg = (OTextEntryMessage *) msg;

  if (msg->type == MSG_TEXTENTRY && msg->action == MSG_TEXTCHANGED) {
    if (_autosize) {
      _te->Resize(_te->GetTextSize());
      MoveResize(_x, _y, GetDefaultWidth(), GetDefaultHeight());
    }
    if ((temsg->keysym == XK_Return) ||
        (temsg->keysym == XK_KP_Enter) ||
        (temsg->keysym == XK_Escape)) {
      if (temsg->keysym == XK_Escape) {
        // should restore old text
        _te->Clear();
        _te->AddText(0, _oldName->GetString());
      }
      Hide();
    }
  }

  return True;
}

void OXEditLabel::Reconfig() {

  _bg = _client->GetResourcePool()->GetDocumentBgndColor();
  _fg = _client->GetResourcePool()->GetDocumentFgndColor();

  SetBackgroundColor(_bg);

  //_te->SetBackgroundColor(_bg);
  //_te->SetTextColor(_fg);
  //_te->SetFont(...);
  // resize, etc. if font changed

  _te->Reconfig();
  NeedRedraw(True);
}
