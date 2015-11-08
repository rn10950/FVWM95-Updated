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
#include <sys/time.h>

#include <X11/cursorfont.h>

#include <xclass/OGC.h>
#include <xclass/OResourcePool.h>

#include <xclass/OXMdiFrame.h>
#include <xclass/OXMdiDecorFrame.h>
#include <xclass/OXMdiMainFrame.h>

#include "icons/mdi.xpm"


//----------------------------------------------------------------------

OXMdiDecorFrame::OXMdiDecorFrame(OXMdiMainFrame *main, OXMdiFrame *frame,
                                 int w, int h, const OXGC *boxGC,
                                 unsigned int options, unsigned long back) :
  OXCompositeFrame(main->GetContainer(), w, h,
                   options | OWN_BKGND | VERTICAL_FRAME | FIXED_SIZE) {

  _mdiMainFrame = main;
  _frame = frame;
  _isMinimized = _isMaximized = False;
  _minimizedX = _minimizedY = 0;
  _minimizedUserPlacement = False;
  _buttonMask = MDI_DEFAULT_HINTS;

  SetDecorBorderWidth(MDI_BORDER_WIDTH);

  _titlebar = new OXMdiTitleBar(this, _mdiMainFrame);

  _lHint = new OLayoutHints(LHINTS_EXPAND_X);
  _expandHint = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y);

  AddFrame(_titlebar, _lHint);
  _titlebar->LayoutButtons(_buttonMask, False, False);

  _upperHR = new OXMdiVerticalWinResizer(this, main, MDI_RESIZER_TOP,
                                         boxGC, MDI_BORDER_WIDTH);
  _lowerHR = new OXMdiVerticalWinResizer(this, main, MDI_RESIZER_BOTTOM,
                                         boxGC, MDI_BORDER_WIDTH);
  _leftVR = new OXMdiHorizontalWinResizer(this, main, MDI_RESIZER_LEFT,
                                          boxGC, MDI_BORDER_WIDTH);
  _rightVR = new OXMdiHorizontalWinResizer(this, main, MDI_RESIZER_RIGHT,
                                           boxGC, MDI_BORDER_WIDTH);

  _upperLeftCR = new OXMdiCornerWinResizer(this, main,
                                MDI_RESIZER_TOP | MDI_RESIZER_LEFT,
                                boxGC, MDI_BORDER_WIDTH);
  _lowerLeftCR = new OXMdiCornerWinResizer(this, main,
                                MDI_RESIZER_BOTTOM | MDI_RESIZER_LEFT,
                                boxGC, MDI_BORDER_WIDTH);
  _upperRightCR = new OXMdiCornerWinResizer(this, main,
                                 MDI_RESIZER_TOP | MDI_RESIZER_RIGHT,
                                 boxGC, MDI_BORDER_WIDTH);
  _lowerRightCR = new OXMdiCornerWinResizer(this, main,
                                 MDI_RESIZER_BOTTOM | MDI_RESIZER_RIGHT,
                                 boxGC, MDI_BORDER_WIDTH);

  _upperHR->SetMinSize(50, _titlebar->GetDefaultHeight() + 2 * _bw);
  _lowerHR->SetMinSize(50, _titlebar->GetDefaultHeight() + 2 * _bw);
  _leftVR->SetMinSize(50, _titlebar->GetDefaultHeight() + 2 * _bw);
  _rightVR->SetMinSize(50, _titlebar->GetDefaultHeight() + 2 * _bw);
  _upperLeftCR->SetMinSize(50, _titlebar->GetDefaultHeight() + 2 * _bw);
  _lowerLeftCR->SetMinSize(50, _titlebar->GetDefaultHeight() + 2 * _bw);
  _upperRightCR->SetMinSize(50, _titlebar->GetDefaultHeight() + 2 * _bw);
  _lowerRightCR->SetMinSize(50, _titlebar->GetDefaultHeight() + 2 * _bw);

  AddInput(SubstructureNotifyMask);

  _frame->ReparentWindow(this, _bw, _titlebar->GetDefaultHeight()); 
  _frame->_parent = this;
  AddFrame(_frame, _expandHint);

  MapSubwindows();
  Resize(GetDefaultSize());
  Layout();

  MapWindow();

  _frame->RaiseWindow();
  _titlebar->RaiseWindow();
}

OXMdiDecorFrame::~OXMdiDecorFrame() {
  delete _lHint;
  delete _expandHint;
  delete _upperHR;
  delete _lowerHR;
  delete _leftVR;
  delete _rightVR;
  delete _upperLeftCR;
  delete _lowerLeftCR;
  delete _upperRightCR;
  delete _lowerRightCR;
  DestroyWindow();
}

void OXMdiDecorFrame::SetDecorBorderWidth(int bw) {
  _bw = bw;
  _insets = OInsets(_bw, _bw, _bw, _bw);
}

void OXMdiDecorFrame::SetMdiButtons(unsigned long buttons) {
  _buttonMask = buttons;
  _titlebar->LayoutButtons(_buttonMask, _isMinimized, _isMaximized);
  if (_buttonMask & MDI_SIZE) {
    _upperHR->Enable();
    _lowerHR->Enable();
    _leftVR->Enable();
    _rightVR->Enable();
    _upperLeftCR->Enable();
    _lowerLeftCR->Enable();
    _upperRightCR->Enable();
    _lowerRightCR->Enable();
  } else {
    _upperHR->Disable();
    _lowerHR->Disable();
    _leftVR->Disable();
    _rightVR->Disable();
    _upperLeftCR->Disable();
    _lowerLeftCR->Disable();
    _upperRightCR->Disable();
    _lowerRightCR->Disable();
  }
}

void OXMdiDecorFrame::SetResizeMode(int mode) {
  _upperHR->SetResizeMode(mode);
  _lowerHR->SetResizeMode(mode);
  _leftVR->SetResizeMode(mode);
  _rightVR->SetResizeMode(mode);
  _upperLeftCR->SetResizeMode(mode);
  _lowerLeftCR->SetResizeMode(mode);
  _upperRightCR->SetResizeMode(mode);
  _lowerRightCR->SetResizeMode(mode);
}

void OXMdiDecorFrame::Layout() {
  RemoveInput(SubstructureNotifyMask);
  OXCompositeFrame::Layout();
  AddInput(SubstructureNotifyMask);
  if (_isMaximized == False) {
    _upperLeftCR->Move(0, 0);
    _upperRightCR->Move(_w - _upperRightCR->GetWidth(), 0);
    _lowerLeftCR->Move(0, _h - _lowerLeftCR->GetHeight());
    _lowerRightCR->Move(_w - _lowerRightCR->GetWidth(),
                        _h - _lowerRightCR->GetHeight());

    _leftVR->MoveResize(0, _upperLeftCR->GetHeight(), _leftVR->GetWidth(),
                        _h - _upperLeftCR->GetHeight() - _lowerLeftCR->GetHeight());
    _upperHR->MoveResize(_upperLeftCR->GetWidth(), 0, 
                         _w - _upperRightCR->GetWidth() - _upperLeftCR->GetWidth(),
                         _upperHR->GetHeight());
    _rightVR->MoveResize(_w - _rightVR->GetWidth(), _upperRightCR->GetHeight(),
                         _rightVR->GetWidth(),
                         _h - _upperLeftCR->GetHeight() - _lowerLeftCR->GetHeight());
    _lowerHR->MoveResize(_lowerLeftCR->GetWidth(), _h - _lowerHR->GetHeight(),
                         _w - _lowerRightCR->GetWidth() - _lowerLeftCR->GetWidth(),
                         _lowerHR->GetHeight());
  }
}

void OXMdiDecorFrame::SetWindowName(const char *name) {
  _titlebar->_winName->SetText(new OString(name));
  _titlebar->Layout();
  //_titlebar->_winName->NeedRedraw();
}

void OXMdiDecorFrame::SetWindowIcon(const OPicture *icon) {
  _titlebar->_winIcon->SetPicture(icon);
  _titlebar->_winIcon->NeedRedraw();
}

void OXMdiDecorFrame::Move(int x, int y) {
  OXCompositeFrame::Move(x, y);
  if (IsMinimized()) _minimizedUserPlacement = True;
}

void OXMdiDecorFrame::MoveResize(int x, int y, int w, int h) {
  OXCompositeFrame::MoveResize(x, y, w, h);
  if (IsMinimized()) _minimizedUserPlacement = True;
}

int OXMdiDecorFrame::HandleConfigureNotify(XConfigureEvent *event) {
  if (event->window == _frame->GetId()) {
    int new_w = event->width + 2 * _bw;
    int new_h = event->height + 2 * _bw + _titlebar->GetDefaultHeight();

    //if (IsMinimized()) {
    //  _preResizeWidth = new_w;
    //  _preResizeHeight = new_h;
    //} else {
      if ((_w != new_w) || (_h != new_h)) Resize(new_w, new_h);
    //}

    return True;
  }
  return False;
}

void OXMdiDecorFrame::SetGrab() {
#if 1
  XGrabButton(GetDisplay(), Button1, AnyModifier, _id, True,
              ButtonPressMask, GrabModeSync, GrabModeAsync, None, None);
  XGrabButton(GetDisplay(), Button2, AnyModifier, _id, True,
              ButtonPressMask, GrabModeSync, GrabModeAsync, None, None);
  XGrabButton(GetDisplay(), Button3, AnyModifier, _id, True,
              ButtonPressMask, GrabModeSync, GrabModeAsync, None, None);
#else
  XGrabButton(GetDisplay(), AnyButton, AnyModifier, _id, True,
              ButtonPressMask, GrabModeSync, GrabModeAsync, None, None);
#endif
}

void OXMdiDecorFrame::UnsetGrab() {
  XUngrabButton(GetDisplay(), AnyButton, AnyModifier, _id);
}

// We need to handle OXClient masked button events during
// WaitFor loops, otherwise we would prevent the X server
// from dispatching further button events, possibly hanging
// the whole session.

int OXMdiDecorFrame::HandleMaskEvent(XEvent *event) {
  if (event->type == ButtonPress) {
    XSync(GetDisplay(), False);
    XAllowEvents(GetDisplay(), AsyncPointer, CurrentTime);
    XSync(GetDisplay(), False);
  }
  return False;
}

int OXMdiDecorFrame::HandleButton(XButtonEvent *event) {
  if (event->type == ButtonPress) {

    XSync(GetDisplay(), False);
#if 1  // pass click event to just clicked window
    XAllowEvents(GetDisplay(), ReplayPointer, CurrentTime);
#else  // do not pass click to clicked window
    XAllowEvents(GetDisplay(), AsyncPointer, CurrentTime);
#endif
    XSync(GetDisplay(), False);

    OWidgetMessage msg(MSG_MDI, MDI_CURRENT, _id);
    SendMessage(_mdiMainFrame, &msg);
  }
  return True;
}


//----------------------------------------------------------------------

OXMdiTitleBar::OXMdiTitleBar(const OXWindow *p, const OXWindow *mdiwin,
                             const char *name) :
  OXCompositeFrame(p, 10, 10, OWN_BKGND | HORIZONTAL_FRAME) {

  _mdiwin = mdiwin;
  _winName = NULL;
  buttonMiddlePressed = buttonLeftPressed = buttonRightPressed = False;

  AddInput(ButtonPressMask | ButtonReleaseMask | ButtonMotionMask);

  _lHint = new OLayoutHints(LHINTS_NORMAL);
  _leftHint = new OLayoutHints(LHINTS_LEFT | LHINTS_CENTER_Y, 3, 1, 1, 1);
  _middleHint = new OLayoutHints(LHINTS_LEFT | LHINTS_CENTER_Y, 1, 1, 1, 1);
  _rightHint = new OLayoutHints(LHINTS_RIGHT | LHINTS_CENTER_Y, 1, 2, 1, 1);

  _lframe = new OXCompositeFrame(this, 10, 10, HORIZONTAL_FRAME);
  _mframe = new OXCompositeFrame(this, 10, 10, HORIZONTAL_FRAME);
  _rframe = new OXCompositeFrame(this, 10, 10, HORIZONTAL_FRAME);
  AddFrame(_lframe, _leftHint);
  AddFrame(_mframe, _middleHint);
  AddFrame(_rframe, _rightHint);

  _winIcon = new OXMdiTitleIcon(_lframe, this,
                _client->GetPicture("defaultMdiIcon.xpm", defaultMdiIconData),
                16, 16);
  _lframe->AddFrame(_winIcon, _lHint);
  _winIcon->Associate(this);

  _winName = new OXLabel(_mframe, new OString(name));
  _winName->SetTextAlignment(TEXT_LEFT);
  _mframe->AddFrame(_winName, _lHint);

  _buttons = new OXMdiButtons(_rframe, this);
  _rframe->AddFrame(_buttons, _lHint);

  MapWindow();
  MapSubwindows();
  Layout();
}

OXMdiTitleBar::~OXMdiTitleBar() {
  delete _lHint;
  delete _leftHint;
  delete _middleHint;
  delete _rightHint;
}

void OXMdiTitleBar::LayoutButtons(unsigned int buttonmask,
                                  bool isMinimized, bool isMaximized) {

  _winIcon->_popup->EnableEntry(MDI_MOVE);

  if (buttonmask & MDI_SIZE) {
    _winIcon->_popup->EnableEntry(MDI_SIZE);
  } else {
    _winIcon->_popup->DisableEntry(MDI_SIZE);
  }

  if (buttonmask & MDI_MENU) {
    _lframe->ShowFrame(_winIcon);
  } else {
    _lframe->HideFrame(_winIcon);
  }

  if (buttonmask & MDI_CLOSE) {
    _buttons->ShowFrame(_buttons->_button5);
    _winIcon->_popup->EnableEntry(MDI_CLOSE);
  } else {
    _buttons->HideFrame(_buttons->_button5);
    _winIcon->_popup->DisableEntry(MDI_CLOSE);
  }

  if (buttonmask & MDI_HELP) {
    _buttons->ShowFrame(_buttons->_button4);
  } else {
    _buttons->HideFrame(_buttons->_button4);
  }

  if ((buttonmask & MDI_MAXIMIZE) && (!isMaximized)) {
    _buttons->ShowFrame(_buttons->_button3);
    _winIcon->_popup->EnableEntry(MDI_MAXIMIZE); 
  } else {
    _buttons->HideFrame(_buttons->_button3);
    _winIcon->_popup->DisableEntry(MDI_MAXIMIZE);
//    _winIcon->_popup->DisableEntry(MDI_SIZE);
  }

  if (isMinimized | isMaximized) {
    _buttons->ShowFrame(_buttons->_button2);
    _winIcon->_popup->EnableEntry(MDI_RESTORE);
    _winIcon->_popup->DisableEntry(MDI_SIZE);
    if (isMaximized) _winIcon->_popup->DisableEntry(MDI_MOVE);
  } else {
    _buttons->HideFrame(_buttons->_button2);
    _winIcon->_popup->DisableEntry(MDI_RESTORE);
  }

  if ((buttonmask & MDI_MINIMIZE) && (!isMinimized)) {
    _buttons->ShowFrame(_buttons->_button1);
    _winIcon->_popup->EnableEntry(MDI_MINIMIZE);
  } else {
    _buttons->HideFrame(_buttons->_button1);
    _winIcon->_popup->DisableEntry(MDI_MINIMIZE);
  }

  Layout();
//  _buttons->NeedRedraw();
}

void OXMdiTitleBar::SetTitleBarColors(unsigned int fore, unsigned int back,
                                      const OXFont *font) { 

  SetBackgroundColor(back);

  _client->GetFont(font->NameOfFont());
  _winName->SetFont(font);
  _winName->SetTextColor(fore);
  Layout();
  NeedRedraw();
  _winName->NeedRedraw();
  _buttons->NeedRedraw();
  _winIcon->NeedRedraw();
}

int OXMdiTitleBar::HandleDoubleClick(XButtonEvent *event) {
  if ((event->type == ButtonPress) && (event->button == Button1)) {
    OWidgetMessage msg(MSG_MDI, MDI_MAXIMIZE, _parent->GetId());
    SendMessage(_mdiwin, &msg);
  }
  return True;
}

int OXMdiTitleBar::HandleButton(XButtonEvent *event) {
  Cursor defaultCursor;

  if (event->type == ButtonPress) {
    defaultCursor = XCreateFontCursor(GetDisplay(), XC_fleur);
    DefineCursor(defaultCursor);
    switch (event->button) {
      case Button1: {
        x0 = event->x;
        y0 = event->y;
        buttonLeftPressed = True;
        OWidgetMessage msg(MSG_MDI, MDI_CURRENT, _parent->GetId());
        SendMessage(_mdiwin, &msg);
        break;
      }
      case Button2:
        buttonMiddlePressed = True;
        break;

      case Button3:
        XLowerWindow(GetDisplay(), _parent->GetId());
        buttonRightPressed = True;
        break;
    }
  } else {
    XUngrabPointer(GetDisplay(), CurrentTime);
    UndefineCursor();
    switch (event->button) {
      case Button1:
        buttonLeftPressed = False;
        break;

      case Button2:
        buttonMiddlePressed = False;
        break;

      case Button3:
        buttonRightPressed = False;
        break;
    }
  }
  return True;
}

int OXMdiTitleBar::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;

  OWidgetMessage tomsg(MSG_MDI, 0, _parent->GetId());
  switch (msg->action) {
    case MSG_CLICK:
      switch (msg->type) {
        case MSG_BUTTON:
        case MSG_MENU:
          tomsg.action = wmsg->id;
          SendMessage(_mdiwin, &tomsg);
          break;
      }
      break;
  }
  return True;
}

int OXMdiTitleBar::HandleCrossing(XCrossingEvent *event) {
  if (event->type == EnterNotify)
    if (buttonLeftPressed)
      buttonLeftPressed = False;
  return True;
}

int OXMdiTitleBar::HandleMotion(XMotionEvent *event) {

  if (event->window != _id) return True;
  if (!buttonLeftPressed) return True;

  int x = ((OXFrame *)_parent)->GetX();
  int y = ((OXFrame *)_parent)->GetY();
  ((OXFrame *)_parent)->Move(x + event->x - x0, y + event->y - y0); 

  return True;
}

// This is called from OXMdiMainFrame on Restore()

void OXMdiTitleBar::AddFrames(OXMdiTitleIcon *icon, OXMdiButtons *buttons) {
  icon->ReparentWindow(_lframe);
  buttons->ReparentWindow(_rframe);
  _lframe->AddFrame(icon, _lHint);
  _rframe->AddFrame(buttons, _lHint);
}

// This is called from OXMdiMainFrame on Maximize()

void OXMdiTitleBar::RemoveFrames(OXMdiTitleIcon *icon, OXMdiButtons *buttons) {
  _lframe->RemoveFrame(icon);
  _rframe->RemoveFrame(buttons);
}


//----------------------------------------------------------------------

OXMdiButtons::OXMdiButtons(const OXWindow *p, const OXWindow *titlebar) :
  OXCompositeFrame(p, 10, 10, HORIZONTAL_FRAME) {

  _defaultHint = new OLayoutHints(LHINTS_NORMAL, 0, 0, 1, 0);
  _closeHint = new OLayoutHints(LHINTS_NORMAL, 2, 0, 1, 0);
  
  //--- Minimize button

  _button1 = new OXPictureButton(this,
                     _client->GetPicture("minimizeButton.xpm", minimizeButtonIconData),
                     MDI_MINIMIZE);
  AddFrame(_button1, _defaultHint);
  _button1->SetBackgroundColor(_defaultFrameBackground);
  _button1->Associate(titlebar);

  //--- Restore button

  _button2 = new OXPictureButton(this,
                     _client->GetPicture("restoreButton.xpm", restoreButtonIconData),
                     MDI_RESTORE);
  AddFrame(_button2, _defaultHint);
  _button2->SetBackgroundColor(_defaultFrameBackground);
  _button2->Associate(titlebar);

  //--- Maximize button

  _button3 = new OXPictureButton(this,
                     _client->GetPicture("maximizeButton.xpm", maximizeButtonIconData),
                     MDI_MAXIMIZE);
  AddFrame(_button3, _defaultHint);
  _button3->SetBackgroundColor(_defaultFrameBackground);
  _button3->Associate(titlebar);

  //--- Help button

  _button4 = new OXPictureButton(this,
                     _client->GetPicture("helpButton.xpm", helpButtonIconData),
                     MDI_HELP);
  AddFrame(_button4, _defaultHint);
  _button4->SetBackgroundColor(_defaultFrameBackground);
  _button4->Associate(titlebar);

  //--- Close button

  _button5 = new OXPictureButton(this,
                     _client->GetPicture("closeButton.xpm", closeButtonIconData),
                     MDI_CLOSE);
  AddFrame(_button5, _closeHint);
  _button5->SetBackgroundColor(_defaultFrameBackground);
  _button5->Associate(titlebar);
}

OXMdiButtons::~OXMdiButtons() {
  delete _defaultHint;
  delete _closeHint;
} 


//----------------------------------------------------------------------

OXMdiTitleIcon::OXMdiTitleIcon(const OXWindow *p, const OXWindow *titlebar,
                               const OPicture *pic, int w, int h) :
  OXIcon(p, pic, w, h) {

  _msgObject = titlebar;

  //--- MDI system menu

  _popup = new OXPopupMenu(_client->GetRoot());
  _popup->AddEntry(new OHotString("&Restore"), MDI_RESTORE);
  _popup->AddEntry(new OHotString("&Move"), MDI_MOVE);
  _popup->AddEntry(new OHotString("&Size"), MDI_SIZE);
  _popup->AddEntry(new OHotString("Mi&nimize"), MDI_MINIMIZE);
  _popup->AddEntry(new OHotString("Ma&ximize"), MDI_MAXIMIZE);
  _popup->AddSeparator();
  _popup->AddEntry(new OHotString("&Close\tCtrl+F4"), MDI_CLOSE);
  _popup->DisableEntry(MDI_RESTORE);
  _popup->Associate(_msgObject);

  AddInput(ButtonPressMask | ButtonReleaseMask);
}

OXMdiTitleIcon::~OXMdiTitleIcon() {
  delete _popup;
}

void OXMdiTitleIcon::_DoRedraw() {
  ClearArea(0, 0, _w, _h, False);
  OXIcon::_DoRedraw();
}

int OXMdiTitleIcon::HandleButton(XButtonEvent *event) {

  OWidgetMessage msg(MSG_MENU, MSG_CLICK, MDI_CURRENT); 
  SendMessage(_msgObject, &msg);

  if (event->type == ButtonPress) {
    struct timeval before, after;
    int bx, by;

    bx = event->x_root;
    by = event->y_root;

    gettimeofday(&before, NULL);

    _popup->PopupMenu(event->x_root - event->x,
                      event->y_root + (_h - event->y));

    gettimeofday(&after, NULL);

    // compute time difference

    after.tv_sec -= before.tv_sec;
    after.tv_usec -= before.tv_usec;
    if (after.tv_usec < 0) {
      after.tv_usec += 1000000;
      after.tv_sec--;
    }

    if (after.tv_sec == 0 && after.tv_usec < 350000) {
      Window dummy;
      int rx, ry, px, py;
      unsigned int mask;

      XQueryPointer(GetDisplay(), _id, &dummy, &dummy,
                    &rx, &ry, &px, &py, &mask);

      if ((abs(bx - rx) < 3) && (abs(by - ry) < 3)) {

        // we got a double-click

        OWidgetMessage msg(MSG_MENU, MSG_CLICK, MDI_CLOSE);
        SendMessage(_msgObject, &msg);
      }
    }
  }

  return True;
}


//----------------------------------------------------------------------

OXMdiWinResizer::OXMdiWinResizer(const OXWindow *p, const OXWindow *mdiwin,
                   int pos, const OXGC *boxgc, int linew,
                   int mdioptions, int w, int h, unsigned int options) :
  OXFrame(p, w, h, options) {

  _mdiwin = mdiwin;
  _mdioptions = mdioptions;
  _pos = pos;

  _boxGC = boxgc;
  _lnw = linew;

  min_w = 50;
  min_h = 20;

  buttonMiddlePressed = buttonLeftPressed = buttonRightPressed = False;

  XGrabButton(GetDisplay(), Button1, AnyModifier, _id, False,
              ButtonPressMask | ButtonReleaseMask | ButtonMotionMask,
              GrabModeAsync, GrabModeAsync, None, None);

  //_compressMotionEvents = False;
}

int OXMdiWinResizer::HandleButton(XButtonEvent *event) {

  if (!IsEnabled()) return True;

  if (event->type == ButtonPress) {   
    switch (event->button) {
      case Button1: {
        OWidgetMessage msg(MSG_MDI, MDI_CURRENT, _parent->GetId());
        SendMessage(_mdiwin, &msg);
        new_x = old_x = winx = ((OXFrame *)_parent)->GetX();
        new_y = old_y = winy = ((OXFrame *)_parent)->GetY();
        winw = ((OXFrame *)_parent)->GetWidth();
        winh = ((OXFrame *)_parent)->GetHeight();
        x0 = event->x_root;
        y0 = event->y_root;
        new_w = winw;
        new_h = winh;
        if (_mdioptions != MDI_OPAQUE) {
          DrawBox(new_x, new_y, new_w, new_h);
        }
        buttonLeftPressed = True;
        break;
      }

      case Button2:
        buttonMiddlePressed = True;
        break;

      case Button3:
        buttonRightPressed = True;
        break;
    }
  } else {
    XUngrabPointer(GetDisplay(), CurrentTime);
    switch (event->button) {
      case Button1:
        if (_mdioptions != MDI_OPAQUE) {
          DrawBox(new_x, new_y, new_w, new_h);
          ((OXFrame *)_parent)->MoveResize(new_x, new_y, new_w, new_h);
        }
        buttonLeftPressed = False;
        break;
 
      case Button2:
        buttonMiddlePressed = False;
        break;

      case Button3:
        buttonRightPressed = False;
        break;
    }
  }

  return True;
}
  
void OXMdiWinResizer::DrawBox(int x, int y, unsigned int width,
                              unsigned int height) {

  OXMdiMainFrame *m = (OXMdiMainFrame *) _mdiwin;

  XDrawRectangle(GetDisplay(), m->GetContainer()->GetId(), _boxGC->GetGC(),
                 x + _lnw / 2, y + _lnw / 2,
                 width - _lnw, height - _lnw);
}

void OXMdiWinResizer::MoveResizeIt() {
  if (_mdioptions == MDI_OPAQUE) {
    ((OXFrame *)_parent)->MoveResize(new_x, new_y, new_w, new_h);   
  } else {
    DrawBox(old_x, old_y, old_w, old_h);
    DrawBox(new_x, new_y, new_w, new_h);
  }
}


//----------------------------------------------------------------------

OXMdiVerticalWinResizer::OXMdiVerticalWinResizer(const OXWindow *p,
             const OXWindow *mdiwin, int pos, const OXGC *boxgc, int linew,
             int mdioptions, int w, int h) :
  OXMdiWinResizer(p, mdiwin, pos, boxgc, linew, mdioptions,
                  w, h, FIXED_HEIGHT | OWN_BKGND) {

  Cursor defaultCursor = XCreateFontCursor(GetDisplay(), XC_sb_v_double_arrow);
  DefineCursor(defaultCursor);
}
   
int OXMdiVerticalWinResizer::HandleMotion(XMotionEvent *event) {

  if (!IsEnabled()) return True;
  if (((OXMdiDecorFrame *)_parent)->IsMinimized()) return True;

  old_x = new_x;
  old_y = new_y;
  old_w = new_w;
  old_h = new_h;

  int dy = event->y_root - y0;

  if (!buttonLeftPressed) return True;

  switch (_pos) {
    case MDI_RESIZER_TOP: 
      if (winh - dy < min_h) dy = winh - min_h;
      new_y = winy + dy;
      new_h = winh - dy;
      break;

    case MDI_RESIZER_BOTTOM:
      if (winh + dy < min_h) dy = min_h - winh;
      new_y = winy;
      new_h = winh + dy;
      break;
  }  

  MoveResizeIt();

  return True;
} 

void OXMdiVerticalWinResizer::DrawBorder() {
  ClearArea(0, 0, _w, _h, False);
  if (_pos == MDI_RESIZER_TOP) {
    DrawLine(_hilightGC, 0, 1, _w - 1, 1);
  } else {
    DrawLine(_shadowGC, 0, _h - 2, _w - 1, _h - 2);
    DrawLine(_blackGC, 0, _h - 1, _w - 1, _h - 1);
  }
}   
 

//----------------------------------------------------------------------
    
OXMdiCornerWinResizer::OXMdiCornerWinResizer(const OXWindow *p,
           const OXWindow *mdiwin, int pos, const OXGC *boxgc, int linew,
           int mdioptions, int w, int h) :
  OXMdiWinResizer(p, mdiwin, pos, boxgc, linew, mdioptions,
                  w, h, FIXED_SIZE | OWN_BKGND) {

  Cursor defaultCursor = None;

  switch (_pos) {
    case (MDI_RESIZER_TOP | MDI_RESIZER_LEFT):
      defaultCursor = XCreateFontCursor(GetDisplay(), XC_top_left_corner);
      break;

    case (MDI_RESIZER_BOTTOM | MDI_RESIZER_LEFT):
      defaultCursor = XCreateFontCursor(GetDisplay(), XC_bottom_left_corner);
      break;

    case (MDI_RESIZER_TOP | MDI_RESIZER_RIGHT):
      defaultCursor = XCreateFontCursor(GetDisplay(), XC_top_right_corner);
      break;

    case (MDI_RESIZER_BOTTOM | MDI_RESIZER_RIGHT):
      defaultCursor = XCreateFontCursor(GetDisplay(), XC_bottom_right_corner);
      break;
  }

  DefineCursor(defaultCursor);
}

int OXMdiCornerWinResizer::HandleMotion(XMotionEvent *event) {

  if (!IsEnabled()) return True;
  if (((OXMdiDecorFrame *)_parent)->IsMinimized()) return True;

  old_x = new_x;
  old_y = new_y;
  old_w = new_w;
  old_h = new_h;

  int dx = event->x_root - x0;
  int dy = event->y_root - y0;

  if (!buttonLeftPressed) return True;

  switch (_pos) {
    case (MDI_RESIZER_TOP | MDI_RESIZER_LEFT): 
      if (winw - dx < min_w) dx = winw - min_w;
      if (winh - dy < min_h) dy = winh - min_h;
      new_x = winx + dx;
      new_w = winw - dx;
      new_y = winy + dy;
      new_h = winh - dy;
      break;

    case (MDI_RESIZER_BOTTOM | MDI_RESIZER_LEFT): 
      if (winw - dx < min_w) dx = winw - min_w;
      if (winh + dy < min_h) dy = min_h - winh;
      new_x = winx + dx;
      new_w = winw - dx;
      new_y = winy;
      new_h = winh + dy;
      break;

    case (MDI_RESIZER_TOP | MDI_RESIZER_RIGHT):
      if (winw + dx < min_w) dx = min_w - winw;
      if (winh - dy < min_h) dy = winh - min_h;
      new_x = winx;
      new_w = winw + dx;
      new_y = winy + dy;
      new_h = winh - dy;
      break;

    case (MDI_RESIZER_BOTTOM | MDI_RESIZER_RIGHT):
      if (winw + dx < min_w) dx = min_w - winw;
      if (winh + dy < min_h) dy = min_h - winh;
      new_x = winx;
      new_w = winw + dx;
      new_y = winy;
      new_h = winh + dy;
      break;
  }

  MoveResizeIt();

  return True;
}

void OXMdiCornerWinResizer::DrawBorder() {

  ClearArea(0, 0, _w, _h, False);

  switch (_pos) {
    case (MDI_RESIZER_TOP | MDI_RESIZER_LEFT):
      DrawLine(_hilightGC, 1, 1, _w - 1, 1);
      DrawLine(_hilightGC, 1, 1, 1, _h - 1);
      break;

    case (MDI_RESIZER_BOTTOM | MDI_RESIZER_LEFT):
      DrawLine(_hilightGC, 1, 0, 1, _h - 1);
      DrawLine(_shadowGC, 1, _h - 2, _w - 1, _h - 2);
      DrawLine(_blackGC, 0, _h - 1, _w - 1, _h - 1);
      break;

    case (MDI_RESIZER_TOP | MDI_RESIZER_RIGHT):
      DrawLine(_hilightGC, 0, 1, _w - 1, 1);
      DrawLine(_shadowGC, _w - 2, 1, _w - 2, _h - 1);
      DrawLine(_blackGC, _w - 1, 0, _w - 1, _h - 1);
      break;

    case (MDI_RESIZER_BOTTOM | MDI_RESIZER_RIGHT):
      DrawLine(_shadowGC, _w - 2, 0, _w - 2, _h - 2);
      DrawLine(_shadowGC, 0, _h - 2, _w - 1, _h - 2);
      DrawLine(_blackGC, _w - 1, 0, _w - 1, _h - 1);
      DrawLine(_blackGC, 0, _h - 1, _w - 1, _h - 1);
      break;   
  }
}

    
//----------------------------------------------------------------------

OXMdiHorizontalWinResizer::OXMdiHorizontalWinResizer(const OXWindow *p,
               const OXWindow *mdiwin, int pos, const OXGC *boxgc, int linew,
               int mdioptions, int w, int h) :
  OXMdiWinResizer(p, mdiwin, pos, boxgc, linew, mdioptions,
                  w, h, FIXED_WIDTH | OWN_BKGND) {

  Cursor defaultCursor = XCreateFontCursor(GetDisplay(), XC_sb_h_double_arrow);
  DefineCursor(defaultCursor);
}   

int OXMdiHorizontalWinResizer::HandleMotion(XMotionEvent *event) {

  if (!IsEnabled()) return True;
  if (((OXMdiDecorFrame *)_parent)->IsMinimized()) return True;

  old_x = new_x;
  old_y = new_y;
  old_w = new_w;
  old_h = new_h;

  int dx = event->x_root - x0;

  if (!buttonLeftPressed) return True;

  switch (_pos) {   
    case (MDI_RESIZER_LEFT):
      if (winw - dx < min_w) dx = winw - min_w;
      new_x = winx + dx;
      new_w = winw - dx;
      break;

    case (MDI_RESIZER_RIGHT):
      if (winw + dx < min_w) dx = min_w - winw;
      new_x = winx;
      new_w = winw + dx;
      break;
  }

  MoveResizeIt();

  return True;
}
 
void OXMdiHorizontalWinResizer::DrawBorder() {
  ClearArea(0, 0, _w, _h, False);
  if (_pos == MDI_RESIZER_LEFT) {
    DrawLine(_hilightGC, 1, 0, 1, _h - 1);
  } else {
    DrawLine(_shadowGC, _w - 2, 0, _w - 2, _h - 1);
    DrawLine(_blackGC, _w - 1, 0, _w - 1, _h - 1);
  } 
}
