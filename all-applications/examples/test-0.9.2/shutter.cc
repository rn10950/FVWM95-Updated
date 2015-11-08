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
#include <xclass/OXGroupFrame.h>
#include <xclass/OString.h>
#include <xclass/OXPictureButton.h>
#include <xclass/OXShutter.h>

#include "b1.xpm"
#include "b2.xpm"


//-----------------------------------------------------------------

class OXMain : public OXMainFrame {
public:
  OXMain(const OXWindow *p, int w, int h);
  virtual ~OXMain();

//  virtual int ProcessMessage(OMessage *msg);

protected:
  OXShutter *_shutter;
  OXGroupFrame *_gframe;
  OLayoutHints *_l1, *_l2;
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

main() {
  OXClient *clientX = new OXClient;

  OXMain *mainWindow = new OXMain(clientX->GetRoot(), 140, 600);
  mainWindow->MapWindow();

  clientX->Run();
}

OXMain::OXMain(const OXWindow *p, int w, int h) : OXMainFrame(p, w, h) {
  const OPicture *pic1, *pic2;
  OXShutterItem *item;
  OXCompositeFrame *container;

  pic1 = _client->GetPicture("b1.xpm", b1_xpm);
  pic2 = _client->GetPicture("b2.xpm", b2_xpm);

  if (!pic1 || !pic2) FatalError("Failed to create pixmaps");

  _gframe = new OXGroupFrame(this, new OString("Shutter"));

  _shutter = new OXShutter(_gframe);

  _l1 = new OLayoutHints(LHINTS_TOP | LHINTS_CENTER_X, 5, 5, 5, 0);

  item = new OXShutterItem(_shutter, new OHotString("Test 1"), 2001);
  container = (OXCompositeFrame *) item->GetContainer();
  container->AddFrame(new OXMyPicButton(container, pic1, 1001), _l1);
  container->AddFrame(new OXMyPicButton(container, pic2, 1002), _l1);
  container->AddFrame(new OXMyPicButton(container, pic1, 1003), _l1);
  container->AddFrame(new OXMyPicButton(container, pic2, 1004), _l1);
  container->SetBackgroundColor(_client->GetShadow(_defaultFrameBackground));
  _shutter->AddItem(item);

  item = new OXShutterItem(_shutter, new OHotString("Test 2"), 2002);
  container = (OXCompositeFrame *) item->GetContainer();
  container->AddFrame(new OXMyPicButton(container, pic1, 1005), _l1);
  container->AddFrame(new OXMyPicButton(container, pic2, 1006), _l1);
  container->AddFrame(new OXMyPicButton(container, pic1, 1007), _l1);
  container->SetBackgroundColor(_client->GetColorByName("#80c0c0"));
  _shutter->AddItem(item);

  item = new OXShutterItem(_shutter, new OHotString("Test 3"), 2003);
  container = (OXCompositeFrame *) item->GetContainer();
  container->AddFrame(new OXMyPicButton(container, pic1, 1008), _l1);
  container->AddFrame(new OXMyPicButton(container, pic2, 1009), _l1);
  _shutter->AddItem(item);

  item = new OXShutterItem(_shutter, new OHotString("Test 4"), 2004);
  container = (OXCompositeFrame *) item->GetContainer();
  container->AddFrame(new OXMyPicButton(container, pic1, 1010), _l1);
  _shutter->AddItem(item);

  _l2 = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y, 5, 5, 5, 5);

  _gframe->AddFrame(_shutter, _l2);
  AddFrame(_gframe, _l2);

  SetWindowName("Shutter");

  MapSubwindows();
  Resize(140, 600);
  Layout();
}

OXMain::~OXMain() {
  delete _l1;
  delete _l2;
}
