/**************************************************************************

    This file is part of clock, a program for testing Xclass95 widgets.
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
  
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <xclass/OXClient.h>
#include <xclass/OXWindow.h>
#include <xclass/OXMainFrame.h>
#include <xclass/OXLabel.h>
#include <xclass/OString.h>
#include <xclass/OTimer.h>
#include <xclass/utils.h>

OXClient *clientX;

//---------------------------------------------------------------------

class OXMain : public OXMainFrame {
public:
  OXMain(const OXWindow *p, int w, int h);
  virtual ~OXMain();

  virtual int HandleTimer(OTimer *t);

protected:
  OXLabel *TimeLabel;
  OTimer *timer;
  OLayoutHints *lhints;
};

OXMain::OXMain(const OXWindow *p, int w, int h) :
  OXMainFrame(p, w, h) {

  TimeLabel = new OXLabel(this, new OString(" 00:00:00 AM "));

  lhints = new OLayoutHints(LHINTS_EXPAND_Y | LHINTS_EXPAND_X, 5, 5, 5, 5);

  AddFrame(TimeLabel, lhints);

  SetWindowName("Clock");
  SetIconName("Clock");
  SetClassHints("clock", "clock");

  SetMWMHints(MWM_DECOR_ALL | MWM_DECOR_RESIZEH | MWM_DECOR_MAXIMIZE |
                              MWM_DECOR_MINIMIZE | MWM_DECOR_MENU |
                              MWM_DECOR_TITLE,
              MWM_FUNC_ALL | MWM_FUNC_RESIZE | MWM_FUNC_MAXIMIZE | 
                             MWM_FUNC_MINIMIZE,
              MWM_INPUT_MODELESS);

  timer = new OTimer(this, 1000);

  Resize(GetDefaultSize());
  MapSubwindows();
}

OXMain::~OXMain() {
  delete timer;
  delete lhints;
}

int OXMain::HandleTimer(OTimer *t) {
  struct tm *tms;
  char   str[40];
  time_t tmt;

  if (t != timer) return False;

  time(&tmt);
  tms = localtime(&tmt);
  strftime(str, 15, "%I:%M:%S %p", tms);
  TimeLabel->SetText(new OString(str));
  Layout();

  delete timer;
  timer = new OTimer(this, 1000);

  return True;
}

//---------------------------------------------------------------------

main() {
  OXMain *mainWindow;

  clientX = new OXClient;
  mainWindow = new OXMain(clientX->GetRoot(), 10, 10);
  mainWindow->MapWindow();
  clientX->Run();

  return 0;
}
