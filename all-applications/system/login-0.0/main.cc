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
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <xclass/OXClient.h>
#include <xclass/OXWindow.h>
#include <xclass/OXMainFrame.h>
#include <xclass/OXTextButton.h>
#include <xclass/OXIcon.h>
#include <xclass/OXLabel.h>
#include <xclass/OXSecretTextEntry.h>
#include <xclass/O2ColumnsLayout.h>
#include <xclass/OString.h>
#include <xclass/OTimer.h>
#include <xclass/utils.h>

#include "login-bckgnd.xpm"
#include "login-key.xpm"


OXClient *clientX;

#define ID_OK      100
#define ID_CANCEL  101


//---------------------------------------------------------------------

class OXMain : public OXMainFrame {
public:
  OXMain(const OXWindow *p, int w, int h);
  virtual ~OXMain();

protected:
  OLayoutHints *bly, *bfly1, *bfly2, *lly, 
               *tly, *ply, *hly;
  OXButton     *Ok, *Cancel;
  OXTextEntry  *user, *passwd;
};


//---------------------------------------------------------------------

OXMain::OXMain(const OXWindow *p, int w, int h) :
  OXMainFrame(p, w, h) {

  int i, ax, ay;
  Window wdummy;
  const OPicture *pic;

  ChangeOptions((GetOptions() & ~VERTICAL_FRAME) | HORIZONTAL_FRAME);

  OXVerticalFrame *vframe1 = new OXVerticalFrame(this, 0, 0, FIXED_WIDTH);
  OXVerticalFrame *vframe2 = new OXVerticalFrame(this, 0, 0, 0);

  Ok     = new OXTextButton(vframe1, new OHotString("OK"), ID_OK);
  Cancel = new OXTextButton(vframe1, new OHotString("Cancel"), ID_CANCEL);
  Ok->Associate(this);
  Cancel->Associate(this);

  SetDefaultAcceptButton(Ok);
  SetDefaultCancelButton(Cancel);

  vframe1->Resize(max(Ok->GetDefaultWidth(),
                      Cancel->GetDefaultWidth())+20,
                  vframe1->GetDefaultHeight());

  OXCompositeFrame *eframe = new OXCompositeFrame(vframe2, 0, 0, 0);
  eframe->SetLayoutManager(new O2ColumnsLayout(eframe, 5, 5));
  lly    = new OLayoutHints(LHINTS_TOP | LHINTS_LEFT, 0, 0, 15, 15);
  user   = new OXTextEntry(eframe, new OTextBuffer(50));
  passwd = new OXSecretTextEntry(eframe, new OTextBuffer(50));
  pic    = clientX->GetPicture("login-key.xpm", login_key_xpm);
  tly    = new OLayoutHints(LHINTS_TOP | LHINTS_LEFT | LHINTS_EXPAND_X, 0, 20, 0, 10);
  ply    = new OLayoutHints(LHINTS_CENTER_Y | LHINTS_LEFT, 30, 30, 15, 15);

  //--- layout for buttons: top align, equally expand horizontally
  bly = new OLayoutHints(LHINTS_TOP | LHINTS_LEFT | LHINTS_EXPAND_X, 0, 0, 3, 0);

  //--- layout for the frame: place at bottom, right aligned
  hly  = new OLayoutHints(LHINTS_BOTTOM | LHINTS_RIGHT, 0, 0, 0, 0);
  bfly1 = new OLayoutHints(LHINTS_TOP | LHINTS_RIGHT, 20, 10, 15, 0);
  bfly2 = new OLayoutHints(LHINTS_TOP | LHINTS_LEFT | LHINTS_EXPAND_Y | LHINTS_EXPAND_X, 0, 0, 0, 15);

  user->Resize(200, user->GetDefaultHeight());
  passwd->Resize(200, passwd->GetDefaultHeight());

  eframe->AddFrame(new OXLabel(eframe, new OHotString("&User Name:")), NULL);
  eframe->AddFrame(user, NULL);
  eframe->AddFrame(new OXLabel(eframe, new OHotString("&Password:")), NULL);
  eframe->AddFrame(passwd, NULL);

  vframe1->AddFrame(Ok, bly);
  vframe1->AddFrame(Cancel, bly);

  vframe2->AddFrame(new OXLabel(vframe2, 
           new OString("Type a user name and password to log on to Linux.")),
           lly);
  vframe2->AddFrame(eframe, tly);

  AddFrame(new OXIcon(this, pic, 48, 48), ply);
  AddFrame(vframe2, bfly2);
  AddFrame(vframe1, bfly1);

  SetFocusOwner(user);

  SetWindowName("Welcome to Linux");
  SetIconName("Login");
  SetClassHints("Login", "Login");
  ODimension size = GetDefaultSize();
  Resize(size);
  SetWMSize(size.w, size.h);
  SetWMSizeHints(size.w, size.h, size.w, size.h, 0, 0);
  SetMWMHints(MWM_DECOR_ALL | MWM_DECOR_RESIZEH | MWM_DECOR_MAXIMIZE |
                              MWM_DECOR_MINIMIZE | MWM_DECOR_MENU,
              MWM_FUNC_ALL | MWM_FUNC_RESIZE | MWM_FUNC_MAXIMIZE | 
                             MWM_FUNC_MINIMIZE,
              MWM_INPUT_MODELESS);

  MapSubwindows();

  const OPicture *bckgnd = _client->GetPicture("login-bckgnd.xpm", login_bckgnd_xpm);
  if (bckgnd) {
    //SetBackgroundPixmap(bckgnd->GetPicture());
  }
}

OXMain::~OXMain() {
  delete bly; delete lly;
  delete bfly1; delete bfly2;
  delete tly;
}

//---------------------------------------------------------------------

main() {
  OXMain *mainWindow;

  clientX = new OXClient;

  mainWindow = new OXMain(clientX->GetRoot(), 100, 100);
  mainWindow->MapWindow();
  clientX->Run();
  exit(0);
}
