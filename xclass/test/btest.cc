/**************************************************************************

    This is a program intended for testing xclass file events.
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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <xclass/utils.h>
#include <xclass/OXClient.h>
#include <xclass/OXWindow.h>
#include <xclass/OXMainFrame.h>
#include <xclass/OXTextButton.h>
#include <xclass/OString.h>


//---------------------------------------------------------------------

class OXMain : public OXMainFrame {
public:
  OXMain(const OXWindow *p, int w, int h);
  virtual ~OXMain();

  virtual int ProcessMessage(OMessage *msg);
  virtual int HandleMotion(XMotionEvent *event);

protected:
  OXButton *_ok;
  OLayoutHints *_lh;

  int _px, _py;
};

//---------------------------------------------------------------------

main() {
  OXClient *clientX = new OXClient;

  OXMain *mainWindow = new OXMain(clientX->GetRoot(), 400, 200);
  mainWindow->MapWindow();

  clientX->Run();

  return 0;
}

OXMain::OXMain(const OXWindow *p, int w, int h) :
  OXMainFrame(p, w, h) {

  _ok = new OXTextButton(this, new OString("Click me!"), 101);
  _lh = new OLayoutHints(LHINTS_CENTER_X | LHINTS_CENTER_Y, 50, 50, 50, 50);

  AddFrame(_ok, _lh);

  AddInput(PointerMotionMask);

  SetWindowName("Test");

  MapSubwindows();
  Resize(GetDefaultSize());

  _px = _py = 0;
}

OXMain::~OXMain() {
  delete _lh;
}

int OXMain::ProcessMessage(OMessage *msg) {
  if ((msg->type == MSG_BUTTON) &&
      (msg->action == MSG_CLICK) &&
      (((OWidgetMessage *)msg)->id == 101)) CloseWindow();

  return True;
}

#define IN_BUTTON(x, y) \
  ((x >= _ok->GetX()-1) && \
   (x <= _ok->GetX()+_ok->GetWidth()) && \
   (y >= _ok->GetY()-1) && \
   (y <= _ok->GetY()+_ok->GetHeight()))

int OXMain::HandleMotion(XMotionEvent *event) {
  int x = event->x;
  int y = event->y;
  int nx = _ok->GetX();
  int ny = _ok->GetY();

  if (IN_BUTTON(x, y)) {

    if ((_py >= _ok->GetY()) && (_py < _ok->GetY()+_ok->GetHeight())) {
      if (x > (_ok->GetX()+_ok->GetWidth()/2))
        nx = x - _ok->GetWidth() -2;
      else
        nx = x +2;
    } else {
      if (y > (_ok->GetY()+_ok->GetHeight()/2))
        ny = y - _ok->GetHeight() -2;
      else
        ny = y +2;
    }

    if (nx < 2)
      nx = _w-_ok->GetWidth()-2;
    else if (nx+_ok->GetWidth()+2 >= _w)
      nx = 2;

    if (ny < 2)
      ny = _h-_ok->GetHeight()-2;
    else if (ny+_ok->GetHeight()+2 >= _h)
      ny = 2;

    _ok->Move(nx, ny);
  }

  _px = event->x;
  _py = event->y;

  return True;
}
