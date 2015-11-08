/**************************************************************************

    This program acts as a X shell for the 'find' command line utility.
    Copyright (C) 2001, Michael Gibson, Hector Peraza.

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
#include <errno.h>
#include <ctype.h>

#include <vector>

#include <xclass/OXClient.h>
#include <xclass/OXMsgBox.h>
#include <xclass/OX3dLines.h>
#include <xclass/O2ColumnsLayout.h>
#include <xclass/OString.h>

#include "main.h"
#include "menudef.h"

#include "mg.xpm"
#include "file.xpm"
#include "user.xpm"
#include "date.xpm"

#include "animation.xpm"

#define ANIM_DELAY  150


//----------------------------------------------------------------------

int main(int argc, char **argv) {

  OXClient *clientX = new OXClient(argc, argv);

  OXMain *mainWindow = new OXMain(clientX->GetRoot(), 450, 300);
  mainWindow->MapWindow();

  clientX->Run();

  return 0;
}

//----------------------------------------------------------------------

OXMain::OXMain(const OXWindow *p, int w, int h) :
  OXMainFrame(p, w, h) {

  //---- Load pictures...

  const OPicture *Glass = _client->GetPicture("mg.xpm", mg_xpm);
  if (!Glass)
    FatalError("Pixmap not found: mg.xpm");

  const OPicture *FileP = _client->GetPicture("file.xpm", file_xpm);
  if (!FileP)
    FatalError("Pixmap not found: file.xpm");

  const OPicture *UserP = _client->GetPicture("user.xpm", user_xpm);
  if (!UserP)
    FatalError("Pixmap not found: user.xpm");

  const OPicture *DateP = _client->GetPicture("date.xpm", date_xpm);
  if (!DateP)
    FatalError("Pixmap not found: date.xpm");

  _animation[0] = _client->GetPicture("anif01.xpm", anif01_xpm);
  _animation[1] = _client->GetPicture("anif02.xpm", anif02_xpm);
  _animation[2] = _client->GetPicture("anif03.xpm", anif03_xpm);
  _animation[3] = _client->GetPicture("anif04.xpm", anif04_xpm);
  _animation[4] = _client->GetPicture("anif05.xpm", anif05_xpm);
  _animation[5] = _client->GetPicture("anif06.xpm", anif06_xpm);
  _animation[6] = _client->GetPicture("anif07.xpm", anif07_xpm);
  _animation[7] = _client->GetPicture("anif08.xpm", anif08_xpm);
  _animation[8] = _client->GetPicture("anif09.xpm", anif09_xpm);
  _animation[9] = _client->GetPicture("anif10.xpm", anif10_xpm);
  _animation[10] = _client->GetPicture("anif11.xpm", anif11_xpm);
  _animation[11] = _client->GetPicture("anif12.xpm", anif12_xpm);
  _animation[12] = _client->GetPicture("anif21.xpm", anif21_xpm);
  _animation[13] = _client->GetPicture("anif22.xpm", anif22_xpm);
  _animation[14] = _client->GetPicture("anif23.xpm", anif23_xpm);
  _animation[15] = _client->GetPicture("anif24.xpm", anif24_xpm);
  _animation[16] = _client->GetPicture("anif25.xpm", anif25_xpm);
  _animation[17] = _client->GetPicture("anif26.xpm", anif26_xpm);
  _animation[18] = _client->GetPicture("anif27.xpm", anif27_xpm);
  _animation[19] = _client->GetPicture("anif28.xpm", anif28_xpm);
  _animation[20] = _client->GetPicture("anif29.xpm", anif29_xpm);
  _animation[21] = _client->GetPicture("anif30.xpm", anif30_xpm);
  _animation[22] = _client->GetPicture("anif31.xpm", anif31_xpm);
  _animation[23] = _client->GetPicture("anif32.xpm", anif32_xpm);

  OXCompositeFrame *tmp_frame, *f1;

  CommonCLayout = new OLayoutHints(LHINTS_CENTER_X | LHINTS_CENTER_Y, 2, 2, 2, 2);
  CommonXLayout = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y, 2, 2, 2, 2);
  CommonLLayout = new OLayoutHints(LHINTS_LEFT | LHINTS_CENTER_Y, 10, 2, 2, 2);
  CommonRLayout = new OLayoutHints(LHINTS_RIGHT | LHINTS_CENTER_Y, 2, 2, 2, 2);
  TabLayout = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_TOP, 10, 10, 10, 5);

  OXCompositeFrame *TabFrame, *ButtonFrame;

  TabFrame = new OXCompositeFrame(this, 550, 220, HORIZONTAL_FRAME);

  //---- Create menus and menu bar

  _menuBarLayout = new OLayoutHints(LHINTS_TOP | LHINTS_LEFT | LHINTS_EXPAND_X, 
                                    0, 0, 1, 1);
  _menuBarItemLayout = new OLayoutHints(LHINTS_TOP | LHINTS_LEFT,
                                    0, 4, 0, 0);

  _menuFile = _MakePopup(&file_popup);
  _menuEdit = _MakePopup(&edit_popup);
  _menuView = _MakePopup(&view_popup);
  _menuHelp = _MakePopup(&help_popup);

  _menuFile->Associate(this);
  _menuEdit->Associate(this);
  _menuView->Associate(this);
  _menuHelp->Associate(this);

  _menuBar = new OXMenuBar(this, 1, 1, HORIZONTAL_FRAME);
  _menuBar->AddPopup(new OHotString("&File"), _menuFile, _menuBarItemLayout);
  _menuBar->AddPopup(new OHotString("&Edit"), _menuEdit, _menuBarItemLayout);
  _menuBar->AddPopup(new OHotString("&View"), _menuView, _menuBarItemLayout);
  _menuBar->AddPopup(new OHotString("&Help"), _menuHelp, _menuBarItemLayout);

  AddFrame(_menuBar, _menuBarLayout);

  AddFrame(new OXHorizontal3dLine(this),
           new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X));

  //---- Create Tab widget

  _tab = new OXTab(TabFrame, 10, 10);
  TabFrame->AddFrame(_tab, CommonXLayout);

  //---- Tab 1: Name & Location 

  tmp_frame = _tab->AddTab(new OString("Name & Location"));

  f1 = new OXCompositeFrame(tmp_frame);
  f1->SetLayoutManager(new O2ColumnsLayout(f1, 5, 1));

  f1->AddFrame(new OXLabel(f1, new OString("Named:")));
  TextE = new OXTextEntry(f1, new OTextBuffer(50));
  f1->AddFrame(TextE);
  TextE->Associate(this);
  TextE->Resize(200, TextE->GetDefaultHeight());

  f1->AddFrame(new OXLabel(f1, new OString("Look in:")));
  SearchIn = new OXComboBox(f1, (char *) "", 5);
  f1->AddFrame(SearchIn);
  SearchIn->Associate(this);
  SearchIn->Resize(200, SearchIn->GetDefaultHeight());

  SearchIn->AddEntry(new OString("/"), 1);
  SearchIn->AddEntry(new OString("/etc"), 2);
  SearchIn->AddEntry(new OString("/home"), 3);
  char *home = getenv("HOME");
  if (home) SearchIn->AddEntry(new OString(home), 0);
  SearchIn->AddEntry(new OString("/usr"), 4);
  SearchIn->AddEntry(new OString("/usr/local"), 5);
  SearchIn->AddEntry(new OString("/var"), 6);
  SearchIn->AddEntry(new OString("/tmp"), 6);

  if (home)
    SearchIn->Select(0);
  else
    SearchIn->Select(1);

  tmp_frame->AddFrame(f1, TabLayout);

  Subfolders = new OXCheckButton(tmp_frame, new OHotString("Search Sub&directories"), 6);
  tmp_frame->AddFrame(Subfolders, CommonLLayout);
  Subfolders->SetState(BUTTON_DOWN);
  Subfolders->Associate(this);

  tmp_frame->AddFrame(new OXIcon(tmp_frame, FileP, 32, 32), CommonRLayout);

  //---- Tab 2: Ownsership

  tmp_frame = _tab->AddTab(new OString("Ownership"));

  f1 = new OXCompositeFrame(tmp_frame);
  f1->SetLayoutManager(new O2ColumnsLayout(f1, 5, 1));

  f1->AddFrame(new OXLabel(f1, new OString("File is owned by user")));
  TextU = new OXTextEntry(f1, new OTextBuffer(50));
  f1->AddFrame(TextU);
  TextU->Associate(this);
  TextU->Resize(150, TextU->GetDefaultHeight());

  f1->AddFrame(new OXLabel(f1, new OString("File belongs to group")));
  TextG = new OXTextEntry(f1, new OTextBuffer(50));
  f1->AddFrame(TextG);
  TextG->Associate(this);
  TextG->Resize(150, TextG->GetDefaultHeight());

  tmp_frame->AddFrame(f1, TabLayout);

  tmp_frame->AddFrame(new OXIcon(tmp_frame, UserP, 32, 32), CommonRLayout);

  //---- Tab 3: Date

  tmp_frame = _tab->AddTab(new OString("Date"));

  f1 = new OXCompositeFrame(tmp_frame, 1, 1, VERTICAL_FRAME);
  f1->SetLayoutManager(new O2ColumnsLayout(f1, 5, 1));

  f1->AddFrame(new OXLabel(f1, new OString("Days since last access")));
  Text2 = new OXTextEntry(f1, new OTextBuffer(50));
  f1->AddFrame(Text2);
  Text2->Associate(this);
  Text2->Resize(100, Text2->GetDefaultHeight());

  f1->AddFrame(new OXLabel(f1, new OString("Days since last modification")));
  Text3 = new OXTextEntry(f1, new OTextBuffer(50));
  f1->AddFrame(Text3);
  Text3->Associate(this);
  Text3->Resize(100, Text3->GetDefaultHeight());

  f1->AddFrame(new OXLabel(f1, new OString("Days since last status change")));
  Text4 = new OXTextEntry(f1, new OTextBuffer(50));
  f1->AddFrame(Text4);
  Text4->Associate(this);
  Text4->Resize(100, Text4->GetDefaultHeight());

  tmp_frame->AddFrame(f1, TabLayout);

  tmp_frame->AddFrame(new OXIcon(tmp_frame, DateP, 32, 32), CommonRLayout);

  //---- Buttons

  ButtonFrame = new OXCompositeFrame(TabFrame, 10, 10, VERTICAL_FRAME | FIXED_WIDTH);
  TabFrame->AddFrame(ButtonFrame, CommonRLayout);

  Find = new OXTextButton(ButtonFrame, new OHotString("Find &Now"), 1);
  Find->Associate(this);

  Stop = new OXTextButton(ButtonFrame, new OHotString("&Stop"), 2);
  Stop->Disable();
  Stop->Associate(this);

  NewSearch = new OXTextButton(ButtonFrame, new OHotString("Ne&w Search"), 3);
  NewSearch->Associate(this);

#if 0
  mainicon = new OXIcon(ButtonFrame, Glass, 32, 32);
#else
  mainicon = new OXIcon(ButtonFrame, _animation[0], 45, 45);
#endif

  int width = Find->GetDefaultWidth();
  width = max(width, Stop->GetDefaultWidth());
  width = max(width, NewSearch->GetDefaultWidth());

  ButtonLayout = new OLayoutHints(LHINTS_RIGHT, 5, 0, 0, 0);
  ButtonFrame->AddFrame(Find, CommonXLayout);
  ButtonFrame->AddFrame(Stop, CommonXLayout);
  ButtonFrame->AddFrame(NewSearch, CommonXLayout);
  ButtonFrame->AddFrame(mainicon, CommonCLayout);

  ButtonFrame->Resize(width + 20, ButtonFrame->GetDefaultHeight());

  //---- Output window

  Output = new OXListView(this, 450, 100, 12);
  Output->Associate(this);

  Output->SetViewMode(LV_DETAILS);
  Output->AddColumn(new OString("Name"),      0, TEXT_LEFT);
  Output->AddColumn(new OString("In Folder"), 1, TEXT_LEFT);
  Output->AddColumn(new OString("Size"),      2, TEXT_RIGHT);
  Output->AddColumn(new OString("Type"),      3, TEXT_LEFT);
  Output->AddColumn(new OString("Modified"),  4, TEXT_LEFT);

  //---- Place master frames on window

  AddFrame(TabFrame, new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 5, 5, 5, 5));
  AddFrame(Output, new OLayoutHints(LHINTS_EXPAND_ALL));

  //------ status bar

  _statusBar = new OXStatusBar(this);
  AddFrame(_statusBar, new OLayoutHints(LHINTS_BOTTOM | LHINTS_EXPAND_X,
                                        0, 0, 3, 0));

  //---- last touches

  _exec = NULL;
  _ifh = NULL;
  _ofh = NULL;
  _efh = NULL;

  _cmd = NULL;
  _output = new OString("");

  _exited = False;

  _nitems = 0;

  _animTimer = NULL;
  _animIndex = 0;

  SetDefaultAcceptButton(Find);
  SetFocusOwner(TextE);

  SetWindowName("Find");
  SetIconName("Find");
  SetClassHints("XCLASS", "Find");

  MapSubwindows();
  Layout();

  MapWindow();
}

OXMain::~OXMain() {
  delete _menuBarLayout;
  delete _menuBarItemLayout;

  delete _menuFile;
  delete _menuEdit;
  delete _menuView;
  delete _menuHelp;

  if (_exec) delete _exec;

  if (_ifh) delete _ifh;
  if (_ofh) delete _ofh;
  if (_efh) delete _efh;

  if (_cmd) delete _cmd;
  delete _output;

  if (_animTimer) delete _animTimer;
}

OXPopupMenu *OXMain::_MakePopup(struct _popup *p) {

  OXPopupMenu *popup = new OXPopupMenu(_client->GetRoot());

  for (int i=0; p->popup[i].name != NULL; ++i) {
    if (strlen(p->popup[i].name) == 0) {
      popup->AddSeparator();
    } else {
      if (p->popup[i].popup_ref == NULL) {
        popup->AddEntry(new OHotString(p->popup[i].name), p->popup[i].id);
      } else {
        struct _popup *p1 = p->popup[i].popup_ref;
        popup->AddPopup(new OHotString(p->popup[i].name), p1->ptr);
      }
      if (p->popup[i].state & MENU_DISABLED) popup->DisableEntry(p->popup[i].id);
      if (p->popup[i].state & MENU_CHECKED) popup->CheckEntry(p->popup[i].id);
      if (p->popup[i].state & MENU_RCHECKED) popup->RCheckEntry(p->popup[i].id,
                                                                p->popup[i].id,
                                                                p->popup[i].id);
    }
  }
  p->ptr = popup;

  return popup;
}


//----------------------------------------------------------------------

int OXMain::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;

  switch (msg->type) {
    case MSG_MENU:
    case MSG_BUTTON:
      switch (msg->action) {

        case MSG_CLICK:
          switch (wmsg->id) {

            //-------------------------------- File

            case M_FILE_EXIT:
              CloseWindow();
              break;

            //-------------------------------- Edit

            case M_EDIT_SELECTALL:
              break;

            case M_EDIT_INVERTSEL:
              break;

            //-------------------------------- View

            case M_VIEW_LARGE_ICONS:
            case M_VIEW_SMALL_ICONS:
            case M_VIEW_LIST:
            case M_VIEW_DETAILS:
              break;

            //-------------------------------- Help

            case M_HELP_ABOUT:
              break;

            //-------------------------------- Buttons

            case 1:   // Find Now
              StartSearch();
              break;

            case 2:   // Stop
              StopSearch();
              break;

            case 3:   // New Search
              Output->Clear();
              _statusBar->SetText(0, new OString(""));
              _nitems = 0;
	      StartSearch();
              break;
          }
          break;

      }
      break;

    case MSG_EXEC:
      if (msg->action == MSG_APPEXITED) {
        Stop->Disable();
        StopAnimation();
        // TODO: check return code to see if find failed to run!
        if (_nitems == 0) _statusBar->SetText(0, new OString("No files found"));
        _retc = _exec->GetExitCode();
        //if (_exited && (_retc != 0)) printf("exited with errc %d\n", _retc);
        if (_exited) {
          //char msg[256];
          //sprintf(msg, "Program exited normally (%d)\n", _retc);
        } else {
          _exited = True;
        }
      }
      break;
  }

  return True;
}

int OXMain::HandleFileEvent(OFileHandler *fh, unsigned int mask) {
  char buf[256];
  int  retc;

  if (fh == _ifh) {
    if (mask != XCM_WRITABLE) return False;
    // find wants input!?
    delete _ifh;
    _ifh = NULL;
    //retc = _exec->Write(buf, strlen(buf));
  } else if (fh == _ofh) {
    if (mask != XCM_READABLE) return False;
    retc = _exec->Read(buf, sizeof(buf)-1);
    if ((retc == 0) && (errno != EWOULDBLOCK) && (errno != EAGAIN)) {
      Stop->Disable();
      StopAnimation();
      if (_nitems == 0) _statusBar->SetText(0, new OString("No files found"));
      if (_exited) {
        //sprintf(buf, "find exited normally (%d)\n", _retc);
	//Output->AddEntry(new OString(buf), -1);
      } else {
        _exited = True;
      }
      delete _ofh;
      _ofh = NULL;
    } else if (retc > 0) {
      char *p = buf;
      for (int i = 0; i < retc; ++i) {
        if (buf[i] == '\0') {
          _output->Append(p);
          AddFileName();
          _output->Clear();
          p = &buf[++i];
        }
      }
      if (p < buf + retc) _output->Append(p, (buf + retc) - p);
    }
  } else if (fh == _efh) {
    if (mask != XCM_READABLE) return False;
    retc = _exec->ReadError(buf, sizeof(buf));
    if ((retc == 0) && (errno != EWOULDBLOCK) && (errno != EAGAIN)) {
      delete _efh;
      _efh = NULL;
    } else if (retc > 0) {
      // ignore stderr...
      // (buf of len retc)
    }
  } else {
    return False;
  }

  return True;
}

int OXMain::HandleTimer(OTimer *t) {

  if (t != _animTimer) return False;

  mainicon->SetPicture(_animation[_animIndex++]);
  if (_animIndex == 24) _animIndex = 0;

  delete _animTimer;
  _animTimer = new OTimer(this, ANIM_DELAY);

  return True;
}

void OXMain::StartSearch() {
  char msg[1024], *prog, **argv;
  int  argc, argvsz, retc;

  if (_cmd) delete _cmd;

  const char *pattern = TextE->GetString();
  const char *start_dir = SearchIn->GetText();
  if (strlen(start_dir) == 0) start_dir = ".";
  const char *user = TextU->GetString();
  const char *group = TextG->GetString();

  _cmd = new OString("find ");
  _cmd->Append(start_dir);
  if (strlen(pattern) > 0) {
    _cmd->Append(" -name ");
    _cmd->Append(pattern);
  }
  if (strlen(user) > 0) {
    _cmd->Append(" -user ");
    _cmd->Append(user);
  }
  if (strlen(group) > 0) {
    _cmd->Append(" -group ");
    _cmd->Append(group);
  }
#if 0
  _cmd->Append(" -type f -print0");
#else
  _cmd->Append(" -print0");
#endif

  //printf("%s\n", _cmd->GetString());

  if (_exec) {
    //if (_exec->IsRunning()) {
      // somehow a previous find is still running, terminate it
    //}
    delete _exec; _exec = NULL;

    if (_ifh) delete _ifh; _ifh = NULL;
    if (_ofh) delete _ofh; _ofh = NULL;
    if (_efh) delete _efh; _efh = NULL;
  }

  const char *s, *e, *str = _cmd->GetString();

  argc = 0;
  argvsz = 8;
  argv = new char*[argvsz];
  s = e = str;
  while (*e) {
    while (*e && !isspace(*e)) ++e;
    if (argc >= argvsz-1) {
      char **tmp = argv;
      argv = new char*[argvsz*2];
      for (int i = 0; i < argvsz; ++i) argv[i] = tmp[i];
      delete[] tmp;
      argvsz *= 2;
    }
    argv[argc] = new char[e - s + 1];
    strncpy(argv[argc], s, e - s);
    argv[argc][e - s] = '\0';
    ++argc;
    while (*e && isspace(*e)) ++e;
    s = e;
  }
  argv[argc] = NULL;

  s = strrchr(argv[0], '/');
  if (s) {
    prog = (char *) s + 1;
  } else {
    prog = argv[0];
  }

  _exec = new OExec(_client, argv[0], argv);
  _exec->Associate(this);
  _exited = False;

  Stop->Enable();
  StartAnimation();

  for (int i = 0; i < argc; ++i) delete[] argv[i];
  delete[] argv;

  if (_ifh) delete _ifh;
  if (_ofh) delete _ofh;
  if (_efh) delete _efh;

//  _ifh = new OFileHandler(this, _exec->GetInputFd(), XCM_WRITABLE | XCM_EXCEPTION);
  _ofh = new OFileHandler(this, _exec->GetOutputFd(), XCM_READABLE | XCM_EXCEPTION);
  _efh = new OFileHandler(this, _exec->GetErrorFd(), XCM_READABLE | XCM_EXCEPTION);

}

void OXMain::StopSearch() {
  if (_exec) {
    delete _exec; _exec = NULL;
    if (_ifh) delete _ifh; _ifh = NULL;
    if (_ofh) delete _ofh; _ofh = NULL;
    if (_efh) delete _efh; _efh = NULL;
  }
  _exited = True;

  Stop->Disable();
  StopAnimation();
}

void OXMain::AddFileName() {
  std::vector<OString *> names;
  const char *str = _output->GetString();

  const char *p = strrchr(str, '/');
  if (p) {
    names.push_back(new OString(p+1));
    OString *s = new OString("");
    s->Append(str, p - str);
    names.push_back(s);
  } else {
    names.push_back(new OString(str));
    names.push_back(new OString(""));
  }

  names.push_back(new OString(""));  // size
  names.push_back(new OString(""));  // type or attributes
  names.push_back(new OString(""));  // modified

  // TODO: add file pics

  OListViewItem *item = new OListViewItem(Output, _nitems++, NULL, NULL, names,
                                          Output->GetViewMode());
  Output->AddItem(item);
  Output->Layout();

  char tmp[100];
  sprintf(tmp, "%d file%s found", _nitems, (_nitems == 1) ? "" : "s");
  _statusBar->SetText(0, new OString(tmp));

  names.clear();
}

void OXMain::StartAnimation() {
  if (_animTimer) delete _animTimer;
  _animTimer = new OTimer(this, ANIM_DELAY);
}

void OXMain::StopAnimation() {
  if (_animTimer) {
    delete _animTimer;
    _animTimer = NULL;
    _animIndex = 0;
    mainicon->SetPicture(_animation[_animIndex]);
  }
}
