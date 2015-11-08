/**************************************************************************

    This is a test program for the OXMdi set of widgets.
    Copyright (C) 1997, Harald Radke.

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
#include <xclass/OXTextButton.h>
#include <xclass/OXCheckButton.h>
#include <xclass/OXTextEntry.h>
#include <xclass/OXCanvas.h>
#include <xclass/OXMenu.h>
#include <xclass/OXMdi.h>
#include <xclass/OXMsgBox.h>
#include <xclass/OXSlider.h>
#include <xclass/OXDDListBox.h>
#include <xclass/OListViewLayout.h>


#define M_FILE_NEW         11
#define M_FILE_CLOSE       12
#define M_FILE_EXIT        13

#define M_WINDOW_HOR       21
#define M_WINDOW_VERT      22
#define M_WINDOW_CASCADE   23
#define M_WINDOW_OPAQUE    24
#define M_WINDOW_ARRANGE   25

#define M_HELP_CONTENTS    31
#define M_HELP_ABOUT       32


//----------------------------------------------------------------------

class OXMdiTestSubclass : public OXMdiFrame {
public:
  OXMdiTestSubclass(OXMdiMainFrame *main, int w, int h);

  virtual int CloseWindow();

protected:
  OXCanvas *_canvasWindow;
  OXCompositeFrame *_container;
};

class OXMdiHintTest : public OXMdiFrame {
public:
  OXMdiHintTest(OXMdiMainFrame *main, int w, int h);

  virtual int ProcessMessage(OMessage *msg);

protected:
  OXTextEntry *_wname;
  OXCheckButton *_close, *_menu, *_min, *_max, *_size, *_help;
};

class OXAppMainFrame : public OXMainFrame  {
public:
  OXAppMainFrame(const OXWindow *p, int w, int h);

  virtual int ProcessMessage(OMessage *msg);

protected:
  void InitMenu();

  OXMdiMainFrame *_mainFrame;
  OXMdiMenuBar *_menuBar;
  OLayoutHints *_menuBarItemLayout;
  OXPopupMenu *_menuFile, *_menuWindow, *_menuHelp;
};

#include "pbg.xpm"

//----------------------------------------------------------------------

OXAppMainFrame::OXAppMainFrame(const OXWindow *p, int w, int h) :
  OXMainFrame(p, w, h, VERTICAL_FRAME) {

  _menuBar = new OXMdiMenuBar(this, 10, 10);
  AddFrame(_menuBar, new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X));

  _mainFrame = new OXMdiMainFrame(this, _menuBar, 300, 300);
  AddFrame(_mainFrame, new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y));

  const OPicture *pbg = _client->GetPicture("pbg", pbg_xpm);
  _mainFrame->GetContainer()->SetBackgroundPixmap(pbg->GetPicture());

  OXMdiFrame *mdiFrame;

  //--- 1

  mdiFrame = new OXMdiTestSubclass(_mainFrame, 320, 240);
  mdiFrame->SetMdiHints(MDI_CLOSE | MDI_MENU);
  mdiFrame->SetWindowName("One");
  mdiFrame->MapSubwindows();
  mdiFrame->Layout();

  //--- 2

  mdiFrame = new OXMdiFrame(_mainFrame, 200, 200,
                            OWN_BKGND, _client->GetColorByName("red"));
  mdiFrame->AddFrame(new OXTextButton(mdiFrame, new OHotString("&Press me!"), 1), 
                 new OLayoutHints(LHINTS_CENTER_X | LHINTS_CENTER_Y));
  mdiFrame->SetMdiHints(MDI_DEFAULT_HINTS | MDI_HELP);
  mdiFrame->SetWindowName("Two");
  mdiFrame->MapSubwindows();
  mdiFrame->Layout();
  mdiFrame->Move(150, 200);

  //--- 3

  mdiFrame = new OXMdiFrame(_mainFrame, 200, 200,
                            OWN_BKGND, _client->GetColorByName("green"));
  mdiFrame->AddFrame(new OXTextButton(mdiFrame, new OHotString("Button 1"), 11), 
                 new OLayoutHints(LHINTS_CENTER_X | LHINTS_CENTER_Y));
  mdiFrame->AddFrame(new OXTextButton(mdiFrame, new OHotString("Button 2"), 12),
                 new OLayoutHints(LHINTS_CENTER_X | LHINTS_CENTER_Y));
  mdiFrame->SetMdiHints(MDI_DEFAULT_HINTS | MDI_HELP);
  mdiFrame->SetWindowName("Three");
  mdiFrame->MapSubwindows();
  mdiFrame->Layout();
  mdiFrame->Move(180, 220);

  //--- 4

  mdiFrame = new OXMdiFrame(_mainFrame, 200, 400, OWN_BKGND, _client->GetColorByName("blue"));
  mdiFrame->AddFrame(new OXDDListBox(mdiFrame,1),
        new OLayoutHints(LHINTS_EXPAND_X | LHINTS_CENTER_Y, 5, 5, 0, 0));
  mdiFrame->AddFrame(new OXHSlider(mdiFrame, 50, SLIDER_1, 1),
        new OLayoutHints(LHINTS_CENTER_X | LHINTS_CENTER_Y));
  mdiFrame->Move(400, 300);
  mdiFrame->SetWindowName("Four");
  mdiFrame->MapSubwindows();
  mdiFrame->Layout();

  //--- 5

  mdiFrame = new OXMdiHintTest(_mainFrame, 200, 200);

  InitMenu();

  SetWindowName("MDI test");
  SetClassHints("mdi test", "mdi test");

#if 0
  MoveResize(100, 100, 640, 400);
#else
  Resize(640, 400);
#endif

  MapSubwindows();
  MapWindow();
  Layout();
}


int OXAppMainFrame::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;

  switch (msg->action) {
    case MSG_CLICK:
      switch (msg->type) {
        case MSG_MENU:
          switch (wmsg->id) {
            case M_FILE_NEW:
              new OXMdiFrame(_mainFrame, 200, 100);
              break;

            case M_FILE_CLOSE:
#if 0
              if (_mainFrame->GetCurrent())
	        _mainFrame->GetCurrent()->CloseWindow();
#else
              _mainFrame->Close(_mainFrame->GetCurrent());
#endif
              break;

            case M_FILE_EXIT:
              CloseWindow();
              break;

            case M_WINDOW_HOR:
              _mainFrame->TileHorizontal();
              break;

            case M_WINDOW_VERT:
              _mainFrame->TileVertical();
              break;

            case M_WINDOW_CASCADE:
              _mainFrame->Cascade();
              break;

            case M_WINDOW_ARRANGE:
              _mainFrame->ArrangeMinimized();
              break;

            case M_WINDOW_OPAQUE:
              if (_menuWindow->IsEntryChecked(M_WINDOW_OPAQUE)) {
                _menuWindow->UnCheckEntry(M_WINDOW_OPAQUE);
                _mainFrame->SetResizeMode(MDI_NON_OPAQUE);
              } else {
                _menuWindow->CheckEntry(M_WINDOW_OPAQUE);
                _mainFrame->SetResizeMode(MDI_OPAQUE);
              }
              break;

            default:
              _mainFrame->SetCurrent(wmsg->id);
              break;

          }
          break;
      }
      break;
  }
  return True;
}

void OXAppMainFrame::InitMenu() {

  _menuBarItemLayout = new OLayoutHints(LHINTS_TOP | LHINTS_LEFT, 0, 4, 0, 0);

  _menuFile = new OXPopupMenu(_client->GetRoot());
  _menuFile->AddEntry(new OHotString("&New Window"), M_FILE_NEW);
  _menuFile->AddEntry(new OHotString("&Close Window"), M_FILE_CLOSE);
  _menuFile->AddSeparator();
  _menuFile->AddEntry(new OHotString("E&xit"), M_FILE_EXIT);

  _menuWindow = new OXPopupMenu(_client->GetRoot());
  _menuWindow->AddEntry(new OHotString("Tile &Horizontally"), M_WINDOW_HOR);
  _menuWindow->AddEntry(new OHotString("Tile &Vertically"), M_WINDOW_VERT);
  _menuWindow->AddEntry(new OHotString("&Cascade"), M_WINDOW_CASCADE);  
  _menuWindow->AddSeparator();
  _menuWindow->AddPopup(new OHotString("&Windows"), _mainFrame->GetWinListMenu());
  _menuWindow->AddSeparator();
  _menuWindow->AddEntry(new OHotString("&Arrange icons"), M_WINDOW_ARRANGE);
  _menuWindow->AddSeparator();
  _menuWindow->AddEntry(new OHotString("&Opaque resize"), M_WINDOW_OPAQUE);

  _menuWindow->CheckEntry(M_WINDOW_OPAQUE);

  _menuHelp = new OXPopupMenu(_client->GetRoot());
  _menuHelp->AddEntry(new OHotString("&Contents"), M_HELP_CONTENTS);
  _menuHelp->AddSeparator();
  _menuHelp->AddEntry(new OHotString("&About"), M_HELP_ABOUT);

  _menuHelp->DisableEntry(M_HELP_CONTENTS);
  _menuHelp->DisableEntry(M_HELP_ABOUT);

  _menuFile->Associate(this);
  _menuWindow->Associate(this);
  _menuHelp->Associate(this);

  _menuBar->AddPopup(new OHotString("&File"), _menuFile, _menuBarItemLayout);
  _menuBar->AddPopup(new OHotString("&Windows"),_menuWindow,_menuBarItemLayout);
  _menuBar->AddPopup(new OHotString("&Help"), _menuHelp, _menuBarItemLayout);
}

//----------------------------------------------------------------------

OXMdiTestSubclass::OXMdiTestSubclass(OXMdiMainFrame *main, int w, int h) :
  OXMdiFrame(main, w, h) {

    _canvasWindow = new OXCanvas(this, 400, 240);
    _container = new OXCompositeFrame(_canvasWindow->GetViewPort(), 10, 10,
                                      HORIZONTAL_FRAME | OWN_BKGND,
                                      _whitePixel);
    _container->SetLayoutManager(new OTileLayout(_container, 8));
    _canvasWindow->SetContainer(_container);
  
    for (int i = 0; i < 256; ++i)
      _canvasWindow->AddFrame(new OXFrame(_canvasWindow->GetContainer(),
                                          32, 32, OWN_BKGND, (i+1) & 255),
                              new OLayoutHints(LHINTS_NORMAL));
  
    AddFrame(_canvasWindow, new OLayoutHints(LHINTS_EXPAND_ALL));

    SetWindowIcon(_client->GetPicture("ofolder.t.xpm")); //"notepad.t.xpm"));
}

int OXMdiTestSubclass::CloseWindow() {
  int ret = 0;

  new OXMsgBox(_client->GetRoot(), GetTopLevel(),
               new OString(GetWindowName()),  
               new OString("Really want to close the window?"),
               MB_ICONEXCLAMATION, ID_YES | ID_NO, &ret);

  if (ret == ID_YES) return OXMdiFrame::CloseWindow();

  return False;
}


//----------------------------------------------------------------------

OXMdiHintTest::OXMdiHintTest(OXMdiMainFrame *main, int w, int h) :
  OXMdiFrame(main, w, h) {

    _close = new OXCheckButton(this, new OHotString("Close"), 11);
    _menu  = new OXCheckButton(this, new OHotString("Menu (left icon)"), 12);
    _min   = new OXCheckButton(this, new OHotString("Minimize"), 13);
    _max   = new OXCheckButton(this, new OHotString("Maximize"), 14);
    _size  = new OXCheckButton(this, new OHotString("Resize"), 15);
    _help  = new OXCheckButton(this, new OHotString("Help"), 16);

    OLayoutHints *lh = new OLayoutHints(LHINTS_LEFT | LHINTS_TOP, 5, 100, 5, 0);

    AddFrame(_close, lh);
    AddFrame(_menu, lh);
    AddFrame(_min, lh);
    AddFrame(_max, lh);
    AddFrame(_size, lh);
    AddFrame(_help, lh);

    _close->SetState(BUTTON_DOWN);
    _min->SetState(BUTTON_DOWN);
    _menu->SetState(BUTTON_DOWN);
    _max->SetState(BUTTON_DOWN);
    _size->SetState(BUTTON_DOWN);

    _close->Associate(this);
    _menu->Associate(this);
    _min->Associate(this);
    _max->Associate(this);
    _size->Associate(this);
    _help->Associate(this);

    _wname = new OXTextEntry(this, NULL, 20);
    AddFrame(_wname, new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X,
                                      5, 5, 5, 5));

    _wname->AddText(0, "MDI hints test");
    _wname->Associate(this);

    SetMdiHints(MDI_DEFAULT_HINTS);
    SetWindowName(_wname->GetString());

    SetWindowIcon(_client->GetPicture("app.t.xpm"));

    MapSubwindows();
    Resize(GetDefaultSize());
//    Layout();
}

int OXMdiHintTest::ProcessMessage(OMessage *msg) {

  if ((msg->type == MSG_CHECKBUTTON) &&
      (msg->action == MSG_CLICK)) {
    int hints = 0;

    if (_close->GetState() != BUTTON_UP) hints |= MDI_CLOSE;
    if (_menu->GetState() != BUTTON_UP) hints |= MDI_MENU;
    if (_min->GetState() != BUTTON_UP) hints |= MDI_MINIMIZE;
    if (_max->GetState() != BUTTON_UP) hints |= MDI_MAXIMIZE;
    if (_size->GetState() != BUTTON_UP) hints |= MDI_SIZE;
    if (_help->GetState() != BUTTON_UP) hints |= MDI_HELP;

    SetMdiHints(hints);

  } else if ((msg->type == MSG_TEXTENTRY) &&
             (msg->action == MSG_TEXTCHANGED)) {

    SetWindowName(_wname->GetString());
  }

  return True;
}

//----------------------------------------------------------------------

int main(int argc, char **argv) {

  OXClient *c = new OXClient(argc, argv);

  OXAppMainFrame *main = new OXAppMainFrame(c->GetRoot(), 640, 400);

  c->Run();

  return 0;
}
