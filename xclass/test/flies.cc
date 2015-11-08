/**************************************************************************

    This is a test program for xclass.
    Copyright (C) 2002, Hector Peraza.

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

#include <stdio.h>
#include <math.h>

#include <xclass/OXClient.h>
#include <xclass/OXMainFrame.h>
#include <xclass/OTimer.h>
#include <xclass/utils.h>


//---------------------------------------------------------------------

class OXMain : public OXMainFrame {
public:
  OXMain(const OXWindow *p, int w, int h);
  virtual ~OXMain();

  virtual int HandleMotion(XMotionEvent *event);
  virtual int HandleMapNotify(XMapEvent *event);
  virtual int HandleTimer(OTimer *t);

protected:
  OXFrame *f[6];
  OTimer *timer;

private:
  int Xpos, Ypos, delay;
  double step, currStep;
};

OXMain::OXMain(const OXWindow *p, int w, int h) :
  OXMainFrame(p, w, h) {

  f[0] = new OXFrame(this, 3, 3, OWN_BKGND, _client->GetColorByName("#000000"));
  f[1] = new OXFrame(this, 3, 3, OWN_BKGND, _client->GetColorByName("#0000ff"));
  f[2] = new OXFrame(this, 3, 3, OWN_BKGND, _client->GetColorByName("#00ff00"));
  f[3] = new OXFrame(this, 3, 3, OWN_BKGND, _client->GetColorByName("#ff0000"));
  f[4] = new OXFrame(this, 3, 3, OWN_BKGND, _client->GetColorByName("#ffff00"));
  f[5] = new OXFrame(this, 3, 3, OWN_BKGND, _client->GetColorByName("#ffffff"));

  delay = 15;

  step = 0.2;
  currStep = 0.0;

  Xpos = 50;
  Ypos = 50;

  SetWindowName("Animation test");

  Resize(w, h);
  MapSubwindows();

  AddInput(PointerMotionMask);

  timer = new OTimer(this, delay);
}

OXMain::~OXMain() {
  delete timer;
  for (int i = 0; i < 6; ++i) delete f[i];
}

int OXMain::HandleMapNotify(XMapEvent *event) {
  Window wdummy;
  int dummy;

  XQueryPointer(GetDisplay(), _id, &wdummy, &wdummy, &dummy, &dummy,
                &Xpos, &Ypos, (unsigned int *) &dummy);

  return True;
}

int OXMain::HandleMotion(XMotionEvent *event) {
  Xpos = event->x;
  Ypos = event->y;
  return True;
}

int OXMain::HandleTimer(OTimer *t) {
  int i, x, y, xBase, yBase;

  if (t != timer) return False;

  xBase = _w / 4;
  yBase = _h / 4;

  for (i = 0; i < 6; ++i) {

    x = Xpos + (int) (sin((20.0 * sin(currStep / 20.0)) + i * 70.0) * xBase * 
                      (sin(10.0 + currStep / (10.0 + i)) + 0.2) * 
                      cos((currStep + i * 25.0) / 10.0));

    y = Ypos + (int) (cos((20.0 * sin(currStep / (20.0 + i))) + i * 70.0) * yBase * 
                      (sin(10.0 + currStep / 10.0) + 0.2) * 
                      cos((currStep + i * 25.0) / 10.0));

    f[i]->Move(x, y);
  }

  currStep += step;

  delete timer;
  timer = new OTimer(this, delay);

  return True;
}

//---------------------------------------------------------------------

int main(int argc, char *argv[]) {
  OXMain *mainWindow;

  OXClient *clientX = new OXClient(argc, argv);
  mainWindow = new OXMain(clientX->GetRoot(), 640, 400);
  mainWindow->MapWindow();
  clientX->Run();

  return 0;
}
