/**************************************************************************

    This file is part of xclass.
    Copyright (C) 1996-2001 Hector Peraza.

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
#include <string.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>

#include <xclass/OXClient.h>
#include <xclass/OXWindow.h>
#include <xclass/OXMainFrame.h>
#include <xclass/OXCheckButton.h>
#include <xclass/OResourcePool.h>
#include <xclass/OXColorSelect.h>


//-----------------------------------------------------------------------

class OXMain : public OXMainFrame {
public:
  OXMain(const OXWindow *p, int w, int h);
  virtual ~OXMain();

  virtual int ProcessMessage(OMessage *msg);

protected:
  OXColorSelect *_tr;
  OXCheckButton *_cb;
};


//-----------------------------------------------------------------------

OXMain::OXMain(const OXWindow *p, int w, int h) :
  OXMainFrame(p, w, h) {

  _tr = new OXColorSelect(this, OColor(255, 0, 0), 4100);
  AddFrame(_tr, new OLayoutHints(LHINTS_CENTER, 20, 20, 20, 20));

  _cb = new OXCheckButton(this, new OHotString("&Enable"), 4200);
  AddFrame(_cb, new OLayoutHints(LHINTS_CENTER, 20, 20, 20, 20));

  _cb->Associate(this);
  _cb->SetState(BUTTON_DOWN);

  MapSubwindows();
  Resize(GetDefaultSize());
}

OXMain::~OXMain() {
}

int OXMain::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;

  switch (msg->type) {
    case MSG_CHECKBUTTON:
      if ((msg->action == MSG_CLICK) && (wmsg->id == 4200)) {
        if (_cb->GetState() == BUTTON_DOWN)
          _tr->Enable();
        else
          _tr->Disable();
      }
      break;

    default:
      break;
  }

  return True;
}


//-----------------------------------------------------------------------

main() {
  OXClient *clientX = new OXClient("");

  OXMain *mainWindow = new OXMain(clientX->GetRoot(), 200, 200);
  mainWindow->MapWindow();

  clientX->Run();
  return 0;
}
