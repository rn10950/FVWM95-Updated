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
#include <stdlib.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <xclass/OXClient.h>
#include <xclass/OXMainFrame.h>
#include <xclass/OXTextButton.h>
#include <xclass/OException.h>
#include <xclass/OXMsgBox.h>
#include <xclass/utils.h>


//---------------------------------------------------------------------

class OMyException : public OException {
public:
  OMyException() { _msg = "My Exception"; }

};

class OXMain : public OXMainFrame {
public:
  OXMain(const OXWindow *p, int w, int h);
  virtual ~OXMain();

  virtual int CloseWindow();
  virtual int ProcessMessage(OMessage *msg);

  int ExceptionTest();
  int BadFunction();

protected:
  OXTextButton *_button;
  int _exited;
};

OXMain::OXMain(const OXWindow *p, int w, int h) :
  OXMainFrame(p, w, h) {

  SetWindowName("Exception Test");

  _button = new OXTextButton(this, new OHotString("Generate &Exception"), 1);
  AddFrame(_button, new OLayoutHints(LHINTS_CENTER));

  _exited = False;

  MapSubwindows();
  Resize(300, 200);
  Layout();
}

OXMain::~OXMain() {
  _exited = True;
}

int OXMain::CloseWindow() {

  // This is a safety measure: we should keep the user from closing
  // (and thus deleting) OXMain if the message box is being displayed,
  // otherwise the program may crash if the code tries to access some of
  // the OXMain's properties or methods (which we do not do here anyway).
  // Note that this has nothing to do with the fact that we are using
  // exceptions.

  if (_tlist->NoOfItems() > 0) {
    XBell(GetDisplay(), 0);
    return False;
  }
  return OXMainFrame::CloseWindow();
}

int OXMain::ProcessMessage(OMessage *msg) {
  if (msg->type == MSG_BUTTON && msg->action == MSG_CLICK) {
    ExceptionTest();
  }
  return True;
}

int OXMain::ExceptionTest() {
  try {
    BadFunction();
  } catch (OException &e) {  // will catch OMyException as well
    new OXMsgBox(_client->GetRoot(), this, new OString("Exception"),
                 new OString(e.GetMessage()), MB_ICONSTOP, ID_OK);
  }
  return 0;
}

int OXMain::BadFunction() {
  throw OMyException();
}


//---------------------------------------------------------------------

main() {

  OXClient *clientX = new OXClient;
  OXMain *mainWindow = new OXMain(clientX->GetRoot(), 10, 10);
  mainWindow->MapWindow();
  clientX->Run();

  return 0;
}
