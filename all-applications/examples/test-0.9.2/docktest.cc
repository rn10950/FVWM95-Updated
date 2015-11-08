/**************************************************************************

    This file is part of xclass.
    Copyright (C) 1997-2000, Harald Radke.

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

#include <xclass/OXClient.h>
#include <xclass/OXMainFrame.h>
#include <xclass/OXCanvas.h>
#include <xclass/OXMenu.h>
#include <xclass/OListViewLayout.h>
#include <xclass/OXDockableFrame.h>


#define M_FILE_OPEN        11
#define M_FILE_SAVE        12
#define M_FILE_SAVEAS      13
#define M_FILE_PRINT       14
#define M_FILE_PRINTSETUP  15
#define M_FILE_EXIT        16

#define M_VIEW_ENBL_DOCK   21
#define M_VIEW_ENBL_HIDE   22
#define M_VIEW_DOCK        23
#define M_VIEW_UNDOCK      24

#define M_HELP_CONTENTS    31
#define M_HELP_SEARCH      32
#define M_HELP_ABOUT       33


//----------------------------------------------------------------------

class OXMain : public OXMainFrame  {
public:
  OXMain(const OXWindow *p, int w, int h);
  virtual ~OXMain();

  virtual int ProcessMessage(OMessage *msg);

protected:
  OXDockableFrame *_dock;
  OXMenuBar *_menuBar;
  OLayoutHints *_menuBarItemLayout;
  OXPopupMenu *_menuFile, *_menuView, *_menuHelp;
  OXCanvas *_canvasWindow;
  OXCompositeFrame *_container;

  int _allowUndock, _allowHide;
};


//----------------------------------------------------------------------

int main(int argc, char **argv) {
  OXClient *clientX = new OXClient;

  OXMain *mainWindow = new OXMain(clientX->GetRoot(), 300, 200);
  mainWindow->Resize(300, 200);
  mainWindow->MapWindow();

  clientX->Run();

  return 0;
}

OXMain::OXMain(const OXWindow *p, int w, int h) :
  OXMainFrame(p, w, h, VERTICAL_FRAME) {

  _dock = new OXDockableFrame(this);
  AddFrame(_dock, new OLayoutHints(LHINTS_EXPAND_X, 0, 0, 1, 0));

  _menuBar = new OXMenuBar(_dock, 10, 10, HORIZONTAL_FRAME);
  _menuBarItemLayout = new OLayoutHints(LHINTS_TOP | LHINTS_LEFT, 0, 4, 0, 0);

  _menuFile = new OXPopupMenu(_client->GetRoot());
  _menuFile->AddEntry(new OHotString("&Open..."), M_FILE_OPEN);
  _menuFile->AddEntry(new OHotString("&Save"), M_FILE_SAVE);
  _menuFile->AddEntry(new OHotString("Save &as..."), M_FILE_SAVEAS);
  _menuFile->AddSeparator();
  _menuFile->AddEntry(new OHotString("&Print"), M_FILE_PRINT);
  _menuFile->AddEntry(new OHotString("P&rint setup..."), M_FILE_PRINTSETUP);
  _menuFile->AddSeparator();
  _menuFile->AddEntry(new OHotString("E&xit"), M_FILE_EXIT);

  _menuFile->DisableEntry(M_FILE_OPEN);
  _menuFile->DisableEntry(M_FILE_SAVE);
  _menuFile->DisableEntry(M_FILE_SAVEAS);
  _menuFile->DisableEntry(M_FILE_PRINT);
  _menuFile->DisableEntry(M_FILE_PRINTSETUP);

  _menuView = new OXPopupMenu(_client->GetRoot());
  _menuView->AddEntry(new OHotString("&Dock"), M_VIEW_DOCK);
  _menuView->AddEntry(new OHotString("&Undock"), M_VIEW_UNDOCK);
  _menuView->AddSeparator();
  _menuView->AddEntry(new OHotString("Enable U&ndock"), M_VIEW_ENBL_DOCK);
  _menuView->AddEntry(new OHotString("Enable &Hide"), M_VIEW_ENBL_HIDE);

  _menuView->DisableEntry(M_VIEW_DOCK);

  _menuHelp = new OXPopupMenu(_client->GetRoot());
  _menuHelp->AddEntry(new OHotString("&Contents"), M_HELP_CONTENTS);
  _menuHelp->AddEntry(new OHotString("&Search..."), M_HELP_SEARCH);
  _menuHelp->AddSeparator();
  _menuHelp->AddEntry(new OHotString("&About"), M_HELP_ABOUT);

  _menuHelp->DisableEntry(M_HELP_CONTENTS);
  _menuHelp->DisableEntry(M_HELP_SEARCH);
  _menuHelp->DisableEntry(M_HELP_ABOUT);

  _menuFile->Associate(this);
  _menuView->Associate(this);
  _menuHelp->Associate(this);

  _menuBar->AddPopup(new OHotString("&File"), _menuFile, _menuBarItemLayout);
  _menuBar->AddPopup(new OHotString("&View"), _menuView, _menuBarItemLayout);
  _menuBar->AddPopup(new OHotString("&Help"), _menuHelp, _menuBarItemLayout);

  _dock->AddFrame(_menuBar, new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X));

  _canvasWindow = new OXCanvas(this, 400, 240);
  _container = new OXCompositeFrame(_canvasWindow->GetViewPort(), 10, 10,
                                    HORIZONTAL_FRAME | OWN_BKGND, _whitePixel);
  _container->SetLayoutManager(new OTileLayout(_container, 6));
  _canvasWindow->SetContainer(_container);

  // test only...
  for (int i = 0; i < 256; ++i)
    _canvasWindow->AddFrame(new OXFrame(_canvasWindow->GetContainer(),
                                        32, 24, OWN_BKGND, (i+1)&255),
                            new OLayoutHints(LHINTS_EXPAND_Y | LHINTS_RIGHT));

  AddFrame(_canvasWindow, new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y));

  _allowUndock = True;
  _allowHide = True;

  _menuView->CheckEntry(M_VIEW_ENBL_DOCK);
  _menuView->CheckEntry(M_VIEW_ENBL_HIDE);

  _dock->Associate(this);
  _dock->SetWindowName("Main Menu");

  SetWindowName("Dockable Frame test");
  SetClassHints("docktest", "docktest");

  MapSubwindows();
  Layout();
}

OXMain::~OXMain() {
  delete _menuBarItemLayout;
}

int OXMain::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;

  switch (msg->type) {
    case MSG_MENU:
      if (msg->action == MSG_CLICK) {
        switch (wmsg->id) {
          case M_FILE_EXIT:
            CloseWindow();
            break;

          case M_VIEW_ENBL_DOCK:
            _allowUndock = !_allowUndock;
            if (_allowUndock) {
              _menuView->CheckEntry(M_VIEW_ENBL_DOCK);
            } else {
              _menuView->UnCheckEntry(M_VIEW_ENBL_DOCK);
              _menuView->DisableEntry(M_VIEW_UNDOCK);
            }
            _dock->EnableUndock(_allowUndock);
            break;

          case M_VIEW_ENBL_HIDE:
            _allowHide = !_allowHide;
            if (_allowHide) {
              _menuView->CheckEntry(M_VIEW_ENBL_HIDE);
            } else {
              _menuView->UnCheckEntry(M_VIEW_ENBL_HIDE);
            }
            _dock->EnableHide(_allowHide);
            break;

          case M_VIEW_DOCK:
            _dock->DockContainer();
            break;

          case M_VIEW_UNDOCK:
            _dock->UndockContainer();
            break;
        }
      }
      break;

    case MSG_DOCKABLE:
      if (msg->action == MSG_DOCK) {
        _menuView->DisableEntry(M_VIEW_DOCK);
        if (_allowUndock) _menuView->EnableEntry(M_VIEW_UNDOCK);
      } else if (msg->action == MSG_UNDOCK) {
        _menuView->EnableEntry(M_VIEW_DOCK);
        _menuView->DisableEntry(M_VIEW_UNDOCK);
      }
      break;
  }

  return True;
}
