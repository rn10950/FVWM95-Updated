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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <xclass/utils.h>
#include <xclass/OXClient.h>
#include <xclass/OXWindow.h>
#include <xclass/OXMainFrame.h>
#include <xclass/OXPictureButton.h>
#include <xclass/OXIcon.h>
#include <xclass/OXLabel.h>
#include <xclass/OXShutter.h>
#include <xclass/OString.h>


#include "default.xpm"

#define MAIN_WIDTH  60
#define MAIN_HEIGHT 500


//-----------------------------------------------------------------

struct _sdata {
  char *pixmap_name;
  char *tip_text;
  int  id;
  OXButton *button;
};

class OXMain : public OXMainFrame {
public:
  OXMain(const OXWindow *p, int w, int h);
  virtual ~OXMain();

//  virtual int ProcessMessage(OMessage *msg);
  void AddShutterItem(char *name, struct _sdata data[]);

protected:
  OXShutter *_shutter;
  OLayoutHints *_l1, *_l2;
  const OPicture *_defaultpic;
};

// this is to prevent the buttons from inheriting the container color!

class OXMyPicButton : public OXPictureButton {
public:
  OXMyPicButton(const OXWindow *p, const OPicture *pic, int ID,
                unsigned int option = RAISED_FRAME | DOUBLE_BORDER |
                                      OWN_BKGND) :
    OXPictureButton(p, pic, ID, option) {
      SetBackgroundColor(_defaultFrameBackground);
  }
};


//-----------------------------------------------------------------

// --- Shells

struct _sdata shells_data[] = {
  { "app.s.xpm",       "Xterm",     1001, NULL },
  { NULL,              NULL,        -1,   NULL }
};

// --- Editors

struct _sdata editors_data[] = {
  { "emacs.xpm",       "Emacs",     2001, NULL },
  { "notepad.xpm",     "Notepad",   2002, NULL },
  { NULL,              NULL,        -1,   NULL }
};
// --- Viewers

struct _sdata viewers_data[] = {
  { "Ghostview.s.xpm", "Ghostview", 3001, NULL },
  { "xv.xpm",          "XView",     3002, NULL },
  { NULL,              NULL,        -1,   NULL }
};

// --- Internet

struct _sdata internet_data[] = {
  { "Netscape.s.xpm",  "Netscape",  4001, NULL },
  { NULL,              NULL,        -1,   NULL }
};

// --- Misc

struct _sdata misc_data[] = {
  { "Lock screen.s.xpm", "Lock screen",   5001, NULL },
  { "folder.s.xpm",      "Explorer",      5002, NULL },
  { "help.xpm",          "Help",          5003, NULL },
  { "find1.xpm",         "Find files",    5004, NULL },
  { "run-config.xpm",    "Control Panel", 5005, NULL },
  { NULL,                NULL,            -1,   NULL }
};


//-----------------------------------------------------------------

main() {
  OXClient *clientX = new OXClient;

  OXMain *mainWindow = new OXMain(clientX->GetRoot(), MAIN_WIDTH, MAIN_HEIGHT);
  mainWindow->MapWindow();

  clientX->Run();
}

OXMain::OXMain(const OXWindow *p, int w, int h) : OXMainFrame(p, w, h) {
  OXShutterItem *item;

  _l1 = new OLayoutHints(LHINTS_TOP | LHINTS_CENTER_X, 5, 5, 5, 0);

  _defaultpic = _client->GetPicture("default.xpm", default_xpm);

  if (!_defaultpic) FatalError("Failed to load default pixmap");

  _shutter = new OXShutter(this);

  AddShutterItem("Shells",   shells_data);
  AddShutterItem("Editors",  editors_data);
  AddShutterItem("Viewers",  viewers_data);
  AddShutterItem("Internet", internet_data);
  AddShutterItem("Misc",     misc_data);

  _l2 = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y);
  AddFrame(_shutter, _l2);

  SetWindowName("Apps");
//  SetClassHints("Fvwm shutter", "Fvwm shutter");

  SetMWMHints(MWM_DECOR_ALL | MWM_DECOR_RESIZEH | MWM_DECOR_MAXIMIZE |
                              MWM_DECOR_MINIMIZE | MWM_DECOR_MENU,
              MWM_FUNC_ALL | MWM_FUNC_RESIZE | MWM_FUNC_MAXIMIZE | 
                             MWM_FUNC_MINIMIZE,
              MWM_INPUT_MODELESS);

  MapSubwindows();
  Resize(MAIN_WIDTH, MAIN_HEIGHT);
  Layout();
}

OXMain::~OXMain() {
  delete _l1;
  delete _l2;
}

void OXMain::AddShutterItem(char *name, struct _sdata data[]) {
  OXShutterItem *item;
  OXCompositeFrame *container;
  OXButton *button;
  const OPicture *buttonpic;
  int i;
  static int id = 5001;

  item = new OXShutterItem(_shutter, new OHotString(name), id++);
  container = (OXCompositeFrame *) item->GetContainer();

  for (i=0; data[i].pixmap_name != NULL; ++i) {
    buttonpic = _client->GetPicture(data[i].pixmap_name);
    if (!buttonpic) {
      Debug(DBG_WARN, "Missing pixmap \"%s\", using default.\n",
                      data[i].pixmap_name);
      buttonpic = _defaultpic;
    }

#if 0
    button = new OXMyPicButton(container, buttonpic, data[i].id);
    container->AddFrame(button, _l1);
    button->Associate(this);
    button->SetTip(data[i].tip_text);
#else
    OXIcon *icon = new OXIcon(container, buttonpic, 32, 32);
    container->AddFrame(icon, _l1);
    icon->SetTip(data[i].tip_text);
#endif
  }

  _shutter->AddItem(item);
}
