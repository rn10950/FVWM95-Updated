/**************************************************************************

    This file is part of wintest, a program for testing Xclass95 widgets.
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

#include <xclass/OXClient.h>
#include <xclass/OXWindow.h>
#include <xclass/OXMainFrame.h>
#include <xclass/OXTransientFrame.h>
#include <xclass/OXTextButton.h>
#include <xclass/OXRadioButton.h>
#include <xclass/OXCheckButton.h>
#include <xclass/OXPictureButton.h>
#include <xclass/OXDDListBox.h>
#include <xclass/OXGroupFrame.h>
#include <xclass/OXTextEntry.h>
#include <xclass/OXTab.h>
#include <xclass/OXMenu.h>
#include <xclass/OXMsgBox.h>
#include <xclass/OXFileDialog.h>
#include <xclass/OXAboutDialog.h>
#include <xclass/OListViewLayout.h>
#include <xclass/OResourcePool.h>
#include <xclass/OString.h>
#include <xclass/OXSlider.h>
#include <xclass/utils.h>
#include <xclass/version.h>

#include "menu_file.xpm"
#include "menu_test.xpm"
#include "menu_help.xpm"


#define M_FILE_OPEN      1
#define M_FILE_SAVE      2
#define M_FILE_SAVEAS    3
#define M_FILE_EXIT      4

#define M_TEST_DLG       5
#define M_TEST_MSGBOX    6
#define M_TEST_SLIDER    7

#define M_HELP_CONTENTS  8
#define M_HELP_SEARCH    9
#define M_HELP_ABOUT     10

#define M_CASCADE_1      21
#define M_CASCADE_2      22
#define M_CASCADE_3      23


#define VId1             100
#define HId1             101
#define VId2             102
#define HId2             103

#define VSId1            110
#define HSId1            111
#define VSId2            112
#define HSId2            113


//---------------------------------------------------------------------

class OXTestMainFrame : public OXMainFrame {
public:
  OXTestMainFrame(const OXWindow *p, int w, int h);
  virtual ~OXTestMainFrame();

  virtual int ProcessMessage(OMessage *msg);

protected:
  OXCompositeFrame *_statusFrame;
  OXCanvas *_canvasWindow;
  OXCompositeFrame *_container;
  OXButton *_testButton;
  OXTextEntry *_testText;

  OLayoutHints *_menuBarLayout, *_menuBarItemLayout, *_menuBarHelpLayout;

  OXMenuBar *_menuBar;
  OXPopupMenu *_menuFile, *_menuTest, *_menuHelp;
  OXPopupMenu *_cascadeMenu, *_cascade1Menu, *_cascade2Menu;
};

class OXTestDialog : public OXTransientFrame {
public:
  OXTestDialog(const OXWindow *p, const OXWindow *main, int w, int h,
               unsigned long options = MAIN_FRAME | VERTICAL_FRAME);
  virtual ~OXTestDialog();

  virtual int ProcessMessage(OMessage *msg);

protected:
  OXCompositeFrame *Frame1, *f1, *f2, *f3, *f4;
  OXButton *OkButton, *CancelButton, *btn1, *btn2, *chk1, *chk2, *rad1, *rad2;
  OXDDListBox *_ddlb;
  OXTab *_tab;
  OXTextEntry *_txt1, *_txt2;
  OLayoutHints *L1, *L2, *L3;
};

class OXTestMsgBox : public OXTransientFrame {
public:
  OXTestMsgBox(const OXWindow *p, const OXWindow *main, int w, int h,
               unsigned long options = MAIN_FRAME | VERTICAL_FRAME);
  virtual ~OXTestMsgBox();

  virtual int ProcessMessage(OMessage *msg);

protected:
  OXCompositeFrame *f1, *f2, *f3, *f4, *f5;
  OXButton *TestButton, *CloseButton;
  OXRadioButton *r[4];
  OXCheckButton *c[10];
  OXGroupFrame *g1, *g2;
  OLayoutHints *L1, *L2, *L3, *L4, *L5, *L6, *L21;
  OXTextEntry *title, *msg;
  OTextBuffer *tbtitle, *tbmsg;
  OXLabel *ltitle, *lmsg;
};

class OXTestSliders : public OXTransientFrame {
public:
  OXTestSliders(const OXWindow *p, const OXWindow *main, int w, int h);
  virtual ~OXTestSliders();

  virtual int ProcessMessage(OMessage *msg);

protected:
  OXVerticalFrame *vframe1, *vframe2;
  OLayoutHints    *bly, *bfly1;
  OXHSlider	*hslider1, *hslider2;
  OXVSlider	*vslider1, *vslider2;
  OXTextEntry	*teh1, *tev1, *teh2, *tev2;
  OTextBuffer	*tbh1, *tbv1, *tbh2, *tbv2;
};

//---------------------------------------------------------------------

OXClient *clientX;
OXTestMainFrame *mainWindow;
OXTestDialog *Dialog;

int mb_button_id[10] = { ID_YES, ID_YESALL, ID_NO, ID_OK, ID_APPLY,
                         ID_RETRY, ID_IGNORE, ID_CANCEL,
                         ID_CLOSE, ID_DISMISS };

int mb_icon[4] = { MB_ICONSTOP, MB_ICONQUESTION,
                   MB_ICONEXCLAMATION, MB_ICONASTERISK };

main() {

  MatchXclassVersion(XCLASS_VERSION, XCLASS_RELEASE_DATE);

  clientX = new OXClient;

  Debug(DBG_INFO, "Creating main window...\n");

  mainWindow = new OXTestMainFrame(clientX->GetRoot(), 400, 200);

  Debug(DBG_INFO, "Running...\n");
  clientX->Run();

  return 0;
}

OXTestMainFrame::OXTestMainFrame(const OXWindow *p, int w, int h) :
  OXMainFrame(p, w, h) {
  char tmp[BUFSIZ];

  Debug(DBG_INFO, "Creating menus...\n");

  _menuBarLayout = new OLayoutHints(LHINTS_TOP | LHINTS_LEFT | LHINTS_EXPAND_X, 0, 0, 1, 1);
  _menuBarItemLayout = new OLayoutHints(LHINTS_TOP | LHINTS_LEFT, 0, 4, 0, 0);
  _menuBarHelpLayout = new OLayoutHints(LHINTS_TOP | LHINTS_RIGHT);

  _menuFile = new OXPopupMenu(_client->GetRoot());
  _menuFile->SetSidePic(_client->GetPicture("menu_file.xpm", menu_file_xpm),
                        _client->GetColorByName("#808080"));
  _menuFile->AddEntry(new OHotString("&Open..."), M_FILE_OPEN);
  _menuFile->AddEntry(new OHotString("&Save"), M_FILE_SAVE);
  _menuFile->AddEntry(new OHotString("S&ave as..."), M_FILE_SAVEAS);
  _menuFile->AddEntry(new OHotString("&Close"), 101);
  _menuFile->AddSeparator();
  _menuFile->AddEntry(new OHotString("&Print"), 102);
  _menuFile->AddEntry(new OHotString("P&rint setup..."), 102);
  _menuFile->AddSeparator();
  _menuFile->AddEntry(new OHotString("E&xit"), M_FILE_EXIT);

  _menuFile->DisableEntry(M_FILE_SAVEAS);

  _cascade2Menu = new OXPopupMenu(_client->GetRoot());
  _cascade2Menu->AddEntry(new OHotString("ID = 2&1"), M_CASCADE_1);
  _cascade2Menu->AddEntry(new OHotString("ID = 2&2"), M_CASCADE_2);
  _cascade2Menu->AddEntry(new OHotString("ID = 2&3"), M_CASCADE_3);

  _cascade1Menu = new OXPopupMenu(_client->GetRoot());
  _cascade1Menu->AddEntry(new OHotString("ID = 4&1"), 41);
  _cascade1Menu->AddEntry(new OHotString("ID = 4&2"), 42);
  _cascade1Menu->AddEntry(new OHotString("ID = 4&3"), 43);
  _cascade1Menu->AddSeparator();
  _cascade1Menu->AddPopup(new OHotString("Cascade&d 2"), _cascade2Menu);

  _cascadeMenu = new OXPopupMenu(_client->GetRoot());
  _cascadeMenu->AddEntry(new OHotString("ID = 5&1"), 51);
  _cascadeMenu->AddEntry(new OHotString("ID = 5&2"), 52);
  _cascadeMenu->AddEntry(new OHotString("ID = 5&3"), 53);
  _cascadeMenu->AddSeparator();
  _cascadeMenu->AddPopup(new OHotString("&Cascaded 1"), _cascade1Menu);
  _cascadeMenu->AddPopup(new OHotString("C&ascaded 2"), _cascade2Menu);

  _menuTest = new OXPopupMenu(_client->GetRoot());
  _menuTest->SetSidePic(_client->GetPicture("menu_test.xpm", menu_test_xpm),
                        _client->GetColorByName("#808080"));
  _menuTest->AddEntry(new OHotString("&Dialog..."), M_TEST_DLG);
  _menuTest->AddEntry(new OHotString("&Message Box..."), M_TEST_MSGBOX);
  _menuTest->AddEntry(new OHotString("&Sliders..."), M_TEST_SLIDER);
  _menuTest->AddSeparator();
  _menuTest->AddPopup(new OHotString("&Cascaded menus"), _cascadeMenu);

  _menuHelp = new OXPopupMenu(_client->GetRoot());
  _menuHelp->SetSidePic(_client->GetPicture("menu_help.xpm", menu_help_xpm),
                        _client->GetColorByName("#808080"));
  _menuHelp->AddEntry(new OHotString("&Contents"), M_HELP_CONTENTS);
  _menuHelp->AddEntry(new OHotString("&Search..."), M_HELP_SEARCH);
  _menuHelp->AddSeparator();
  _menuHelp->AddEntry(new OHotString("&About"), M_HELP_ABOUT);

  _menuFile->Associate(this);
  _menuTest->Associate(this);
  _menuHelp->Associate(this);
  _cascadeMenu->Associate(this);
  _cascade1Menu->Associate(this);
  _cascade2Menu->Associate(this);

  Debug(DBG_INFO, "Creating menu bar...\n");

  _menuBar = new OXMenuBar(this, 1, 1, HORIZONTAL_FRAME);
  _menuBar->AddPopup(new OHotString("&File"), _menuFile, _menuBarItemLayout);
  _menuBar->AddPopup(new OHotString("&Test"), _menuTest, _menuBarItemLayout);
  _menuBar->AddPopup(new OHotString("&Help"), _menuHelp, _menuBarHelpLayout);

  Debug(DBG_INFO, "Creating subframes...\n");

  AddFrame(_menuBar, _menuBarLayout);

  _canvasWindow = new OXCanvas(this, 400, 240);
  _container = new OXCompositeFrame(_canvasWindow->GetViewPort(), 10, 10, 
                                    HORIZONTAL_FRAME | OWN_BKGND,
                                    _whitePixel);
  _container->SetLayoutManager(new OTileLayout(_container, 8));
  _canvasWindow->SetContainer(_container);

  _canvasWindow->SetScrollDelay(10, 10);
  _canvasWindow->SetScrollMode(SB_ACCELERATED);

  // test only...
  for (int i=0; i<256; ++i)
    _canvasWindow->AddFrame(new OXFrame(_canvasWindow->GetContainer(),
                                        32, 32, OWN_BKGND, (i+1)&255),
                            new OLayoutHints(LHINTS_EXPAND_Y | LHINTS_RIGHT));

  AddFrame(_canvasWindow, new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y,
 			                     0, 0, 2, 2));

  _statusFrame = new OXCompositeFrame(this, 60, 20, HORIZONTAL_FRAME | SUNKEN_FRAME);

  _testButton = new OXTextButton(_statusFrame, new OHotString("&Click here..."), 150);
  _testButton->Associate(this);
  _statusFrame->AddFrame(_testButton, new OLayoutHints(LHINTS_TOP | LHINTS_LEFT,
                                                       2, 0, 2, 2));
  _testText = new OXTextEntry(_statusFrame, new OTextBuffer(100));
  _testText->Resize(300, _testText->GetDefaultHeight());
  _statusFrame->AddFrame(_testText, new OLayoutHints(LHINTS_TOP | LHINTS_LEFT,
                                                     10, 2, 2, 2));
  AddFrame(_statusFrame, new OLayoutHints(LHINTS_BOTTOM | LHINTS_EXPAND_X,
			 	          0, 0, 1, 0));

  Debug(DBG_INFO, "Startup...\n");

  SetWindowName("WinTest");

  MapSubwindows();

  // we need to use GetDefault...() to initialize the layout algorithm...
  Resize(GetDefaultSize());
// Resize(400, 200);

  MapWindow();
}

OXTestMainFrame::~OXTestMainFrame() {
  delete _menuBarLayout;
  delete _menuBarItemLayout;
  delete _menuBarHelpLayout; 

  delete _menuFile;
  delete _menuTest;
  delete _menuHelp;
  delete _cascadeMenu;
  delete _cascade1Menu;
  delete _cascade2Menu;
}

int OXTestMainFrame::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;

  switch (msg->type) {

    case MSG_BUTTON:
    case MSG_MENU:
      switch (msg->action) {

        case MSG_SELECT:
          Debug(DBG_INFO, "Pointer over menu entry, ID=%d\n", wmsg->id);
          break;

        case MSG_CLICK:
          if (msg->type == MSG_BUTTON) {
            Debug(DBG_INFO, "\nButton was pressed, ID=%d\n", wmsg->id);
            if (wmsg->id == 150)
              new OXTestMsgBox(clientX->GetRoot(), this, 400, 200);
            break;
          }
          switch (wmsg->id) {

            case M_FILE_OPEN:
              {
              OFileInfo fi;

              fi.MimeTypesList = _client->GetResourcePool()->GetMimeTypes();
              fi.file_types = NULL;
              new OXFileDialog(_client->GetRoot(), this, FDLG_OPEN, &fi);
              }
              break;

            case M_TEST_DLG:
              Debug(DBG_INFO, "\nM_FILE_OPEN\n");
              Dialog = new OXTestDialog(clientX->GetRoot(), this, 400, 200);
              Dialog->MapWindow();
              clientX->WaitFor(Dialog);
              break;

            case M_FILE_SAVE:
              Debug(DBG_INFO, "\nM_FILE_SAVE\n");
              break;

            case M_FILE_SAVEAS:
              Debug(DBG_INFO, "\nM_FILE_SAVEAS\n");
              break;

            case M_FILE_EXIT:
              Debug(DBG_INFO, "\nM_FILE_EXIT\n");
              CloseWindow();
              break;

            case M_TEST_MSGBOX:
              new OXTestMsgBox(clientX->GetRoot(), this, 400, 200);
              break;

            case M_TEST_SLIDER:
              new OXTestSliders(clientX->GetRoot(), this, 400, 200);
              break;

            case M_HELP_CONTENTS:
              Debug(DBG_INFO, "\nM_HELP_CONTENTS\n");
              break;

            case M_HELP_SEARCH:
              Debug(DBG_INFO, "\nM_HELP_SEARCH\n");
              break;

            case M_HELP_ABOUT:
              Debug(DBG_INFO, "\nM_HELP_ABOUT\n");
              {
              OAboutInfo info;

              info.wname = "About wintest";
              info.title = "wintest\nA test application for xclass.\n"
                           "Compiled with xclass version "
                           XCLASS_VERSION
                           ",\nrelease date "
                           XCLASS_RELEASE_DATE;
              info.copyright = "Copyleft © 1998-1999 by H. Peraza";
              info.text = "This program is free software; you can "
                          "redistribute it and/or modify it under the "
                          "terms of the GNU General Public License.\n\n"
                          "http://www.foxproject.org";

              new OXAboutDialog(_client->GetRoot(), this, &info);
              }
              break;

            case M_CASCADE_1:
            case M_CASCADE_2:
            case M_CASCADE_3:
              Debug(DBG_INFO, "\nM_CASCADE: %d\n", wmsg->id);
              break;

            default:
              Debug(DBG_INFO, "\nID: %d\n", wmsg->id);
              break;
          } // switch (msg->id)
          break;

        default:
          break;

      } // switch (msg->action)
      break;

    default:
      break;

  } // switch (msg->type)

  return True;
}

//---------------------------------------------------------------------

OXTestDialog::OXTestDialog(const OXWindow *p, const OXWindow *main,
                    int w, int h, unsigned long options) :
  OXTransientFrame(p, main, w, h, options) {
    OXCompositeFrame *tf;
    int ax, ay;
    Window wdummy;

    Frame1 = new OXCompositeFrame(this, 60, 20, VERTICAL_FRAME);

    OkButton = new OXTextButton(Frame1, new OHotString("&Ok"), 1);
    OkButton->Associate(this);
    CancelButton = new OXTextButton(Frame1, new OHotString("&Cancel"), 2);
    CancelButton->Associate(this);

    SetDefaultAcceptButton(OkButton);
    SetDefaultCancelButton(CancelButton);

    L1 = new OLayoutHints(LHINTS_TOP | LHINTS_LEFT | LHINTS_EXPAND_X,
                          200, 2, 2, 2);
    L2 = new OLayoutHints(LHINTS_BOTTOM | LHINTS_EXPAND_X,
                          2, 2, 5, 1);

    Frame1->AddFrame(OkButton, L1);
    Frame1->AddFrame(CancelButton, L1);
    AddFrame(Frame1, L2);

    //--------- create Tab widget and some composite frames for Tab testing

    _tab = new OXTab(this, 300, 300);
    L3 = new OLayoutHints(LHINTS_TOP | LHINTS_LEFT, 5, 5, 5, 5);

    tf = _tab->AddTab(new OString("Tab 1"));
    f1 = new OXCompositeFrame(tf, 60, 20, VERTICAL_FRAME);
    f1->AddFrame(new OXTextButton(f1, new OHotString("&Test button"), 0), L3);
    f1->AddFrame(_txt1 = new OXTextEntry(f1, new OTextBuffer(100)), L3);
    f1->AddFrame(_txt2 = new OXTextEntry(f1, new OTextBuffer(100)), L3);
    tf->AddFrame(f1, L3);
    _txt1->Resize(150, _txt1->GetDefaultHeight());
    _txt2->Resize(150, _txt2->GetDefaultHeight());

    tf = _tab->AddTab(new OString("Tab 2"));
    f2 = new OXCompositeFrame(tf, 60, 20, VERTICAL_FRAME);
    f2->AddFrame(btn1 = new OXTextButton(f2, new OHotString("&Button 1"), 0), L1);
    f2->AddFrame(btn2 = new OXTextButton(f2, new OHotString("B&utton 2"), 0), L1);
    f2->AddFrame(chk1 = new OXCheckButton(f2, new OHotString("C&heck 1"), 0), L1);
    f2->AddFrame(chk2 = new OXCheckButton(f2, new OHotString("Chec&k 2"), 0), L1);
    f2->AddFrame(rad1 = new OXRadioButton(f2, new OHotString("&Radio 1"), 81), L1);
    f2->AddFrame(rad2 = new OXRadioButton(f2, new OHotString("R&adio 2"), 82), L1);
    f2->AddFrame(_ddlb = new OXDDListBox(f2, 88), L3);
    tf->AddFrame(f2, L3);

    for (int i=0; i<20; ++i) {
      char tmp[20];

      sprintf(tmp, "Entry %i", i+1);
      _ddlb->AddEntry(new OString(tmp), i+1);
    }

    _ddlb->Resize(150, 20);

    btn1->Associate(this);
    btn2->Associate(this);
    chk1->Associate(this);
    chk2->Associate(this);
    rad1->Associate(this);
    rad2->Associate(this);

    tf = _tab->AddTab(new OString("Tab 3"));
    f3 = new OXCompositeFrame(tf, 60, 20, HORIZONTAL_FRAME);
    f3->AddFrame(new OXTextButton(f3, new OHotString("Button &1"), 0), L3);
    f3->AddFrame(new OXTextButton(f3, new OHotString("Button &2"), 0), L3);
    tf->AddFrame(f3, L3);

    tf = _tab->AddTab(new OString("Tab 4"));
    f4 = new OXCompositeFrame(tf, 60, 20, VERTICAL_FRAME);
    f4->AddFrame(new OXTextButton(f4, new OHotString("T&est..."), 0), L3);
    tf->AddFrame(f4, L3);

    AddFrame(_tab, L2);

    MapSubwindows();
    Resize(GetDefaultSize());

    // position relative to the parent's window (the default was root 0,0!)
    XTranslateCoordinates(GetDisplay(),
                          main->GetId(), GetParent()->GetId(),
                          (((OXFrame *) main)->GetWidth() - _w) >> 1,
                          (((OXFrame *) main)->GetHeight() - _h) >> 1, 
                          &ax, &ay, &wdummy);

    Move(ax, ay);

    SetWindowName("Dialog");
}

OXTestDialog::~OXTestDialog() {
  delete L1;
  delete L2;
  delete L3;
}

int OXTestDialog::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;

  switch (msg->type) {

    case MSG_BUTTON:
      switch (msg->action) {
        case MSG_CLICK:
          switch (wmsg->id) {
            case 1:
              _tab->RemoveTab(1);
              break;
              
            case 2:
              Debug(DBG_INFO, "\nTerminating dialog: %s pressed\n",
                              (wmsg->id == 1) ? "OK" : "Cancel");
              CloseWindow();
              break;

          }
          break;
      }
      break;

    case MSG_RADIOBUTTON:
      switch (msg->action) {
        case MSG_CLICK:
          switch (wmsg->id) {
            case 81:
              rad2->SetState(BUTTON_UP);
              break;

            case 82:
              rad1->SetState(BUTTON_UP);
              break;

          }
          break;
      }
      break;

    default:
      break;
  }

  return True;
}

//---------------------------------------------------------------------

OXTestMsgBox::OXTestMsgBox(const OXWindow *p, const OXWindow *main,
                           int w, int h, unsigned long options) :
  OXTransientFrame(p, main, w, h, options) {

    int i, ax, ay;
    Window wdummy;

    ChangeOptions((GetOptions() & ~VERTICAL_FRAME) | HORIZONTAL_FRAME);

    f1 = new OXCompositeFrame(this, 60, 20, VERTICAL_FRAME | FIXED_WIDTH);
    f2 = new OXCompositeFrame(this, 60, 20, VERTICAL_FRAME);
    f3 = new OXCompositeFrame(f2, 60, 20, HORIZONTAL_FRAME);
    f4 = new OXCompositeFrame(f2, 60, 20, HORIZONTAL_FRAME);
    f5 = new OXCompositeFrame(f2, 60, 20, HORIZONTAL_FRAME);

    TestButton = new OXTextButton(f1, new OHotString("&Test"), 1);
    CloseButton = new OXTextButton(f1, new OHotString("&Close"), 2);

    f1->Resize(TestButton->GetDefaultWidth()+40, GetDefaultHeight());

    TestButton->Associate(this);
    CloseButton->Associate(this);

    SetDefaultAcceptButton(TestButton);
    SetDefaultCancelButton(CloseButton);

    L1 = new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 
                          2, 2, 3, 0);
    L2 = new OLayoutHints(LHINTS_TOP | LHINTS_RIGHT | LHINTS_EXPAND_X,
                          2, 5, 0, 2);
    L21 = new OLayoutHints(LHINTS_TOP | LHINTS_RIGHT,
                          2, 5, 10, 0);

    f1->AddFrame(TestButton, L1);
    f1->AddFrame(CloseButton, L1);
    AddFrame(f1, L21);

    //--------- create check and radio buttons groups

    g1 = new OXGroupFrame(f3, new OString("Buttons"));
    g2 = new OXGroupFrame(f3, new OString("Icons"));

    L3 = new OLayoutHints(LHINTS_TOP | LHINTS_LEFT | 
                          LHINTS_EXPAND_X | LHINTS_EXPAND_Y,
                          2, 2, 2, 2);
    L4 = new OLayoutHints(LHINTS_TOP | LHINTS_LEFT,
                          0, 0, 5, 0);

    c[0] = new OXCheckButton(g1, new OHotString("Yes"),     -1);
    c[1] = new OXCheckButton(g1, new OHotString("Yes all"), -1);
    c[2] = new OXCheckButton(g1, new OHotString("No"),      -1);
    c[3] = new OXCheckButton(g1, new OHotString("OK"),      -1);
    c[4] = new OXCheckButton(g1, new OHotString("Apply"),   -1);
    c[5] = new OXCheckButton(g1, new OHotString("Retry"),   -1);
    c[6] = new OXCheckButton(g1, new OHotString("Ignore"),  -1);
    c[7] = new OXCheckButton(g1, new OHotString("Cancel"),  -1);
    c[8] = new OXCheckButton(g1, new OHotString("Close"),   -1);
    c[9] = new OXCheckButton(g1, new OHotString("Dismiss"), -1);

    for (i=0; i<10; ++i) g1->AddFrame(c[i], L4);

    r[0] = new OXRadioButton(g2, new OHotString("Stop"),        21);
    r[1] = new OXRadioButton(g2, new OHotString("Question"),    22);
    r[2] = new OXRadioButton(g2, new OHotString("Exclamation"), 23);
    r[3] = new OXRadioButton(g2, new OHotString("Asterisk"),    24);

    for (i=0; i<4; ++i) {
      g2->AddFrame(r[i], L4);
      r[i]->Associate(this);
    }

    c[3]->SetState(BUTTON_DOWN);
    r[0]->SetState(BUTTON_DOWN);

    f3->AddFrame(g1, L3);
    f3->AddFrame(g2, L3);

    ltitle = new OXLabel(f4, new OString("Title:"));
    lmsg   = new OXLabel(f5, new OString("Message:"));

    title = new OXTextEntry(f4, tbtitle = new OTextBuffer(100));
    msg   = new OXTextEntry(f5, tbmsg = new OTextBuffer(100));

    tbtitle->AddText(0, "MsgBox");
    tbmsg->AddText(0, "This is a test message box.");

    title->Resize(300, title->GetDefaultHeight());
    msg->Resize(300, msg->GetDefaultHeight());

    L5 = new OLayoutHints(LHINTS_LEFT | LHINTS_CENTER_Y,
                          3, 5, 0, 0);
    L6 = new OLayoutHints(LHINTS_RIGHT | LHINTS_CENTER_Y,
                          0, 2, 0, 0);

    f4->AddFrame(ltitle, L5);
    f4->AddFrame(title, L6);
    f5->AddFrame(lmsg, L5);
    f5->AddFrame(msg, L6);

    f2->AddFrame(f3, L1);
    f2->AddFrame(f4, L1);
    f2->AddFrame(f5, L1);

    AddFrame(f2, L2);

    MapSubwindows();
    Resize(GetDefaultSize());

    // position relative to the parent's window
    XTranslateCoordinates(GetDisplay(),
                          main->GetId(), GetParent()->GetId(),
                          (((OXFrame *) main)->GetWidth() - _w) >> 1,
                          (((OXFrame *) main)->GetHeight() - _h) >> 1, 
                          &ax, &ay, &wdummy);
    Move(ax, ay);

    SetWindowName("Message Box Test");

    MapWindow();
    _client->WaitFor(this);
}

// Order is important when deleting frames. Delete children first,
// parents last.

OXTestMsgBox::~OXTestMsgBox() {
  delete L1;
  delete L2;
  delete L3;
  delete L4;
  delete L5;
  delete L6;
  delete L21;
}

int OXTestMsgBox::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;
  int i, buttons, icontype, retval;

  OString stitle(tbtitle->GetString());
  OString smsg(tbmsg->GetString());

  switch (msg->action) {
    case MSG_CLICK:

      switch (msg->type) {
        case MSG_BUTTON:
          switch (wmsg->id) {
            case 1:
              buttons = 0;
              for (i=0; i<10; ++i)
                if (c[i]->GetState() == BUTTON_DOWN)
                  buttons |= mb_button_id[i];

              for (i=0; i<4; ++i)
                if (r[i]->GetState() == BUTTON_DOWN)
                  { icontype = mb_icon[i]; break; }

              new OXMsgBox(_client->GetRoot(), this,
                           &stitle, new OString(&smsg),
                           icontype, buttons, &retval);
              break;

            case 2:
              CloseWindow();
              break;

          }
          break;

        case MSG_RADIOBUTTON:
        case MSG_CHECKBUTTON:
          if (wmsg->id >= 21 && wmsg->id <= 24) {
            for (i=0; i<4; ++i)
              if (r[i]->WidgetID() != wmsg->id)
                r[i]->SetState(BUTTON_UP);
          }
          break;

        default:
          break;
      }
      break;

    default:
      break;
  }

  return True;
}

//---------------------------------------------------------------------

OXTestSliders::OXTestSliders(const OXWindow *p, const OXWindow *main,
                             int w, int h) :
  OXTransientFrame(p, main, w, h) {

    int ax, ay;
    Window wdummy;

    ChangeOptions((GetOptions() & ~VERTICAL_FRAME) | HORIZONTAL_FRAME);

    vframe1 = new OXVerticalFrame(this, 0, 0, 0);

    teh1 = new OXTextEntry(vframe1, tbh1 = new OTextBuffer(10), HId1);
    tev1 = new OXTextEntry(vframe1, tbv1 = new OTextBuffer(10), VId1);
    tbh1->AddText(0, "0");
    tbv1->AddText(0, "0");

    teh1->Associate(this);
    tev1->Associate(this);

    hslider1 = new OXHSlider(vframe1, 100, SLIDER_1 | SCALE_BOTH, HSId1);
    hslider1->Associate(this);
    hslider1->SetRange(0,50);

    vslider1 = new OXVSlider(vframe1, 100, SLIDER_2 | SCALE_BOTH, VSId1);
    vslider1->Associate(this);
    vslider1->SetRange(0,8);

    vframe1->Resize(100, 100);
 
    vframe2 = new OXVerticalFrame(this, 0, 0, 0);
    teh2 = new OXTextEntry(vframe2, tbh2 = new OTextBuffer(10), HId2);
    tev2 = new OXTextEntry(vframe2, tbv2 = new OTextBuffer(10), VId2);
    tbh2->AddText(0, "0");
    tbv2->AddText(0, "0");

    teh2->Associate(this);
    tev2->Associate(this);

    hslider2 = new OXHSlider(vframe2, 150, SLIDER_2 | SCALE_BOTH, HSId2);
    hslider2->Associate(this);
    hslider2->SetRange(0,3);

    vslider2 = new OXVSlider(vframe2, 150, SLIDER_1 | SCALE_DOWNRIGHT, VSId2);
    vslider2->Associate(this);
    vslider2->SetRange(-10,10);

    vframe2->Resize(100, 100);

    //--- layout for buttons: top align, equally expand horizontally
    bly = new OLayoutHints(LHINTS_TOP | LHINTS_LEFT | LHINTS_EXPAND_X, 0, 0, 3, 0);

    //--- layout for the frame: place at bottom, right aligned
    bfly1 = new OLayoutHints(LHINTS_TOP | LHINTS_RIGHT, 20, 10, 15, 0);

    vframe1->AddFrame(hslider1, bly);
    vframe1->AddFrame(vslider1, bly);
    vframe1->AddFrame(teh1, bly);
    vframe1->AddFrame(tev1, bly);

    vframe2->AddFrame(hslider2, bly);
    vframe2->AddFrame(vslider2, bly);
    vframe2->AddFrame(teh2, bly);
    vframe2->AddFrame(tev2, bly);

    AddFrame(vframe2, bfly1);
    AddFrame(vframe1, bfly1);

    SetWindowName("Slider Test");
    ODimension size = GetDefaultSize();
    Resize(size);
    SetWMSize(size.w, size.h);
    SetWMSizeHints(size.w, size.h, size.w, size.h, 0, 0);
    SetMWMHints(MWM_DECOR_ALL | MWM_DECOR_RESIZEH | MWM_DECOR_MAXIMIZE |
                                MWM_DECOR_MINIMIZE | MWM_DECOR_MENU,
                MWM_FUNC_ALL | MWM_FUNC_RESIZE | MWM_FUNC_MAXIMIZE | 
                               MWM_FUNC_MINIMIZE,
                MWM_INPUT_MODELESS);

    // position relative to the parent's window
    XTranslateCoordinates(GetDisplay(),
                          main->GetId(), GetParent()->GetId(),
                          (((OXFrame *) main)->GetWidth() - _w) >> 1,
                          (((OXFrame *) main)->GetHeight() - _h) >> 1, 
                          &ax, &ay, &wdummy);
    Move(ax, ay);

    MapSubwindows();
    MapWindow();

    _client->WaitFor(this);
}

OXTestSliders::~OXTestSliders() {
  delete bfly1; delete bly;
}

int OXTestSliders::ProcessMessage(OMessage *msg) {
  char buf[10];
  OTextEntryMessage *temsg;
  OSliderMessage *slmsg;

  switch (msg->type) {
    case MSG_TEXTENTRY:
      temsg = (OTextEntryMessage *) msg;
      switch (temsg->action) {
        case MSG_TEXTCHANGED:
	  switch (temsg->id) {
	    case HId1:
		hslider1->SetPosition(atoi(tbh1->GetString()));
		break;
	    case VId1:
		vslider1->SetPosition(atoi(tbv1->GetString()));
		break;
	    case HId2:
		hslider2->SetPosition(atoi(tbh2->GetString()));
		break;
	    case VId2:
		vslider2->SetPosition(atoi(tbv2->GetString()));
		break;
	  }
	  break;
      }
      break;

    case MSG_VSLIDER:
    case MSG_HSLIDER:
      slmsg = (OSliderMessage *) msg;
      switch (msg->action) {
        case MSG_SLIDERPOS:
	  sprintf(buf, "%d", slmsg->pos);
	  switch (slmsg->id) {
	    case HSId1:
	      tbh1->Clear();
	      tbh1->AddText(0, buf);
	      _client->NeedRedraw(teh1);
	      break;
	    case VSId1:
	      tbv1->Clear();
	      tbv1->AddText(0, buf);
	      _client->NeedRedraw(tev1);
	      break;
	    case HSId2:
	      tbh2->Clear();
	      tbh2->AddText(0, buf);
	      _client->NeedRedraw(teh2);
	      break;
	    case VSId2:
	      tbv2->Clear();
	      tbv2->AddText(0, buf);
	      _client->NeedRedraw(tev2);
	      break;
          }
        break;
      }
      break;
    default:
      break;
  }
  return True;
}
