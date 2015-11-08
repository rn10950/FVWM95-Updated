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
#include <xclass/OXProgressBar.h>
#include <xclass/OTimer.h>
#include <xclass/OString.h>


//---------------------------------------------------------------------

class OXMain : public OXMainFrame {
public:
  OXMain(const OXWindow *p, int w, int h);
  virtual ~OXMain();

  virtual int ProcessMessage(OMessage *msg);
  virtual int HandleTimer(OTimer *t);

protected:
  OXButton *_close;
  OXProgressBar *_pbar1, *_pbar2, *_pbar3;
  OLayoutHints *_lh;
  OTimer *_timer;
  int _cnt;
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

  _lh = new OLayoutHints(LHINTS_CENTER_X | LHINTS_CENTER_Y, 10, 10, 10, 10);

  _pbar1 = new OXProgressBar(this, 200, 25, PROGBAR_MODE1);
  _pbar1->SetRange(0, 120);
  _pbar1->ShowPercentage(True);
  _pbar1->SetFloatingText(True);
  AddFrame(_pbar1, _lh);

  _pbar2 = new OXProgressBar(this, 200, 25, PROGBAR_MODE2);
  _pbar2->SetRange(0, 120);
  _pbar2->ShowPercentage(True);
  _pbar2->SetColor(_client->GetColorByName("red"));
  _pbar2->SetFloatingText(True);
  AddFrame(_pbar2, _lh);

  _pbar3 = new OXProgressBar(this, 200, 25, PROGBAR_MODE2);
  _pbar3->SetRange(0, 120);
  _pbar3->ShowPercentage(True);
  _pbar3->SetColor(_client->GetColorByName("green"));
  _pbar3->SetFloatingText(False);
  AddFrame(_pbar3, _lh);

  _close = new OXTextButton(this, new OString("Close"), 101);
  AddFrame(_close, _lh);

  SetWindowName("Test");

  _timer = new OTimer(this, 250);
  _cnt = 0;

  MapSubwindows();
  Resize(GetDefaultSize());
}

OXMain::~OXMain() {
  delete _lh;
  delete _timer;
}

int OXMain::ProcessMessage(OMessage *msg) {
  if ((msg->type == MSG_BUTTON) &&
      (msg->action == MSG_CLICK) &&
      (((OWidgetMessage *)msg)->id == 101)) CloseWindow();

  return True;
}

int OXMain::HandleTimer(OTimer *t) {
  if (t == _timer) {
    _pbar1->SetPosition(_cnt);
    _pbar2->SetPosition(_cnt);
    _pbar3->SetPosition(_cnt);
    if (_cnt++ == 120) _cnt = 0;
    delete _timer;
    _timer = new OTimer(this, 250);
    return True;
  }
  return False;
}
