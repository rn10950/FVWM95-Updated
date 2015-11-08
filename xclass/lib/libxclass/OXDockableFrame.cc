/**************************************************************************

    This file is part of xclass.
    Copyright (C) 1997-2000, Harald Radke, Hector Peraza.

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

#include <X11/Xlib.h>

#include <xclass/OColor.h>
#include <xclass/OMessage.h>
#include <xclass/OXDockableFrame.h>


//----------------------------------------------------------------------

OXDockButton::OXDockButton(const OXCompositeFrame *p, int id) :
  OXButton (p, id, CHILD_FRAME) {

    Resize(10, GetDefaultHeight());
    _mouseOn = False;

    _normBg = _backPixel;
    OColor c;
    c.SetColor(_client, _normBg);
    int newL = c.GetL() + (255 - c.GetL()) * 45 / 100;
    c.SetHLS(c.GetH(), newL, c.GetS());

    _hiBg = c.GetDisplayPixel(_client);

    AddInput(EnterWindowMask | LeaveWindowMask);
}

OXDockButton::~OXDockButton() {
  _client->FreeColor(_hiBg);
}

int OXDockButton::HandleCrossing(XCrossingEvent *event) {
  OXButton::HandleCrossing(event);
  if (event->type == LeaveNotify) {
    _mouseOn = False;
  } else if (event->type == EnterNotify) {
    _mouseOn = True;
  }
  if (IsEnabled()) NeedRedraw();
  return True;
}

void OXDockButton::DrawBorder() {
  int options = GetOptions();

  if (_state == BUTTON_DOWN || _state == BUTTON_ENGAGED)
    ;
  else if (_mouseOn == True && IsEnabled())
    { SetBackgroundColor(_hiBg); ChangeOptions(CHILD_FRAME); }
  else
    { SetBackgroundColor(_normBg); ChangeOptions(CHILD_FRAME); }
  ClearWindow();
  OXFrame::DrawBorder();

  ChangeOptions(options);
}

void OXDockButton::_DoRedraw() {
  int x = 1, y = 0;

  DrawBorder();
  if (_state == BUTTON_DOWN || _state == BUTTON_ENGAGED) { ++x; ++y; }

  for (int i = 0; i < 5; i +=4) {
    DrawLine(_hilightGC, i+x,   y+1, i+x,   _h-y-3);
    DrawLine(_shadowGC,  i+x+1, y+1, i+x+1, _h-y-3);
  }
}


//----------------------------------------------------------------------

OXDockHideButton::OXDockHideButton(const OXCompositeFrame *p) :
  OXDockButton (p, 2) {

    Resize(10, 8);
    _aspect = 0;
}

void OXDockHideButton::_DoRedraw() {
  int x = 1, y = 0;

  DrawBorder();
  if (_state == BUTTON_DOWN || _state == BUTTON_ENGAGED) { ++x; ++y; }

  if (_aspect) {
    DrawLine(_blackGC, x+1, y+1, x+5, y+3);
    DrawLine(_blackGC, x+1, y+5, x+5, y+3);
    DrawLine(_hilightGC, x, y+1, x, y+5);
  } else {
    DrawLine(_hilightGC, x+5, y+1, x+1, y+3);
    DrawLine(_hilightGC, x+5, y+5, x+1, y+3);
    DrawLine(_blackGC, x+6, y+1, x+6, y+5);
  }
}


//----------------------------------------------------------------------

OXUndockedFrame::OXUndockedFrame(const OXWindow *p,
                                 OXDockableFrame *dockable) :
  OXTransientFrame(p, dockable->GetTopLevel(), 10, 10) {

    SetWindowName("");
    _dockable = dockable;

    SetMWMHints(MWM_DECOR_ALL | MWM_DECOR_RESIZEH | MWM_DECOR_MAXIMIZE |
                                MWM_DECOR_MINIMIZE | MWM_DECOR_MENU,
                MWM_FUNC_ALL | MWM_FUNC_RESIZE | MWM_FUNC_MAXIMIZE |
                               MWM_FUNC_MINIMIZE,
                MWM_INPUT_MODELESS);
}

OXUndockedFrame::~OXUndockedFrame() {
  _dockable->DockContainer(False);
}

void OXUndockedFrame::FixSize() {
  SetWMSize(_w, _h);
  SetWMSizeHints(_w, _h, _w, _h, 0, 0);
}


//---------------------------------------------------------------------------

OXDockableFrame::OXDockableFrame(const OXWindow *p, int id, 
                                 unsigned int options) :
  OXCompositeFrame(p, 10, 10, HORIZONTAL_FRAME),
  OXWidget(id, "OXDockableFrame") {

  _msgObject = _parent;

  _cl = new OLayoutHints(LHINTS_EXPAND_Y | LHINTS_EXPAND_X);

  _l1 = new OLayoutHints(LHINTS_TOP | LHINTS_LEFT);
  _l2 = new OLayoutHints(LHINTS_EXPAND_Y | LHINTS_LEFT);
  _lb = new OLayoutHints(LHINTS_EXPAND_Y | LHINTS_LEFT, 0, 2, 0, 0);

  _buttons = new OXCompositeFrame(this, 10, 10, VERTICAL_FRAME);
  _hidebutton = new OXDockHideButton(_buttons);
  _buttons->AddFrame(_hidebutton, _l1);

  _dockbutton = new OXDockButton(_buttons);
  _buttons->AddFrame(_dockbutton, _l2);

  OXCompositeFrame::AddFrame(_buttons, _lb);

  _container = new OXCompositeFrame(this, 10, 10);

  OXCompositeFrame::AddFrame(_container, _cl);

  _enableHide = True;
  _enableUndock = True;

  _tframe = NULL;
  _hidden = False;

  _dockName = NULL;

  _dockbutton->Associate(this);
  _hidebutton->Associate(this);

  MapSubwindows();
  Resize(GetDefaultSize());
}

OXDockableFrame::~OXDockableFrame() {
  if (_tframe) delete _tframe;
  delete _cl;
  delete _l1;
  delete _l2;
  delete _lb;
  if (_dockName) delete[] _dockName;
}

void OXDockableFrame::AddFrame(OXCompositeFrame *f, OLayoutHints *hints) {
  f->ReparentWindow(_container);
  _container->AddFrame(f, _hints = hints);
  _container->Layout();
}

void OXDockableFrame::UndockContainer() {
  int ax, ay;
  Window wdummy;

  if (_tframe || !_enableUndock) return;

  _tframe = new OXUndockedFrame(_client->GetRoot(), this);

  RemoveFrame(_container);
  _container->ReparentWindow(_tframe);
  _tframe->AddFrame(_container, _cl);  // _hints

  XTranslateCoordinates(GetDisplay(), _id, _client->GetRoot()->GetId(),
                        _x, _y + _tframe->GetHeight(), &ax, &ay, &wdummy);

  if (_dockName) _tframe->SetWindowName(_dockName);

  _tframe->MapSubwindows();
  _tframe->Resize(_tframe->GetDefaultSize());
  _tframe->FixSize();
  _tframe->Move(ax, ay);
  _tframe->SetWMPosition(ax, ay);
  _tframe->SetWMGravity(StaticGravity);
  _tframe->MapWindow();

  if (((OXFrame *)_parent)->IsComposite())           // paranoia check
    ((OXCompositeFrame *)_parent)->HideFrame(this);

  Layout();

  OWidgetMessage msg(MSG_DOCKABLE, MSG_UNDOCK, _widgetID);
  SendMessage(_msgObject, &msg);
}

void OXDockableFrame::DockContainer(int del) {

  if (!_tframe) return;
  if (del) {
    delete _tframe;  // this will call DockContainer again with del = False
    return;
  }

  _tframe->RemoveFrame(_container);
  _container->ReparentWindow(this);
  OXCompositeFrame::AddFrame(_container, _cl);  // _hints

  // kludge! (for special case)
  _dockbutton->Resize(_dockbutton->GetDefaultWidth(), 1);

  Layout();
  if (((OXFrame *)_parent)->IsComposite())           // paranoia check
    ((OXCompositeFrame *)_parent)->ShowFrame(this);

  // _tframe is just being deleted (we're here called by OXUndockedFrame's
  // destructor) so just set it NULL below to avoid eventual problems in
  // OXDockableFrame's destructor.

  _tframe = NULL;

  OWidgetMessage msg(MSG_DOCKABLE, MSG_DOCK, _widgetID);
  SendMessage(_msgObject, &msg);
}

void OXDockableFrame::ShowContainer() {

  if (!_hidden) return;

  ShowFrame(_container);
  if (_enableUndock) _buttons->ShowFrame(_dockbutton);
  _hidebutton->SetAspect(0);
  if (((OXFrame *)_parent)->IsComposite())           // paranoia check
    ((OXCompositeFrame *)_parent)->Layout();
  _hidden = False;

  OWidgetMessage msg(MSG_DOCKABLE, MSG_SHOW, _widgetID);
  SendMessage(_msgObject, &msg);
}

void OXDockableFrame::HideContainer() {

  if (_hidden || !_enableHide) return;

  HideFrame(_container);
  _buttons->HideFrame(_dockbutton);
  _hidebutton->SetAspect(1);
  if (((OXFrame *)_parent)->IsComposite())           // paranoia check
    ((OXCompositeFrame *)_parent)->Layout();
  _hidden = True;

  OWidgetMessage msg(MSG_DOCKABLE, MSG_HIDE, _widgetID);
  SendMessage(_msgObject, &msg);
}

int OXDockableFrame::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;

  switch (msg->type) {
    case MSG_BUTTON:
      switch (msg->action) {
        case MSG_CLICK:
          switch (wmsg->id) {
            case 1:
              if (!_hidden) UndockContainer();
              break;

            case 2:
              if (!_hidden)
                HideContainer();
              else
                ShowContainer();
              break;
          }
          break;
      }
      break;
  }

  return True;
}

void OXDockableFrame::EnableUndock(int onoff) {
  _enableUndock = onoff;
  if (onoff)
    _buttons->ShowFrame(_dockbutton);
  else
    _buttons->HideFrame(_dockbutton);
  Layout();
}

void OXDockableFrame::EnableHide(int onoff) {
  _enableHide = onoff;
  if (onoff)
    _buttons->ShowFrame(_hidebutton);
  else
    _buttons->HideFrame(_hidebutton);
  Layout();
}

void OXDockableFrame::SetWindowName(const char *name) {
  if (_dockName) delete[] _dockName;
  _dockName = NULL;
  if (name) {
    _dockName = StrDup(name);
    if (_tframe) _tframe->SetWindowName(_dockName);
  }
}
