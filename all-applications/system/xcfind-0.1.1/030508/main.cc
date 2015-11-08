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

#include <xclass/OXClient.h>
#include <xclass/OXTransientFrame.h>
#include <xclass/OXTab.h>
#include <xclass/OXIcon.h>
#include <xclass/OXLabel.h>
#include <xclass/OXMsgBox.h>
#include <xclass/OXTextButton.h>
#include <xclass/OXCheckButton.h>
#include <xclass/OXTextEntry.h>
#include <xclass/OX3dLines.h>
#include <xclass/OXTextEdit.h>
#include <xclass/OXDDListBox.h>
#include <xclass/OXComboBox.h>
#include <xclass/OFileHandler.h>
#include <xclass/OString.h>
#include <xclass/OExec.h>
#include <xclass/utils.h>

#include "mg.xpm"
#include "file.xpm"
#include "user.xpm"
#include "date.xpm"


///////////////////////////////////////////////
// Globals (Ok, so I'm lazy)
///////////////////////////////////////////////

int xx;				// loop var  

char *string1;			// unconverted file search
char *user1;			// unconverted user search
char *group1;			// unconverted group search 

char string2[99];		// file search string from widget
char user2[99];			// user search string from widget
char group2[99];		// group search string from widget

char systemstring[150];		// formated string for system command

char dir1[35];			// value of starting directory
char sdir[35];

FILE *fp;			// file pointer used for reading results


//----------------------------------------------------------------------

class OXMain : public OXMainFrame {
public:
  OXMain(const OXWindow *p, int w, int h);
  virtual ~OXMain();

  virtual int ProcessMessage(OMessage *msg);
  virtual int HandleFileEvent(OFileHandler *fh, unsigned int mask);

protected:
  void StartSearch();
  void StopSearch();

  OFileHandler *_ifh, *_ofh, *_efh;
  OExec *_exec;
  OString *_cmd, *_output;
  int _exited, _retc;

  OLayoutHints *CommonCLayout, *CommonXLayout;
  OLayoutHints *CommonTLayout, *CommonTTLayout, *CommonBLayout;
  OLayoutHints *CommonLLayout, *CommonRLayout, *CommonSLayout;

  OTextBuffer *TBText1, *TBUser1, *TBGroup1;
  OTextBuffer *TBText2, *TBText3, *TBText4;
  OXTab *Main;

  OXCompositeFrame *F1, *F1A, *F1B, *F1C, *F1D;	//tab1 subframes
  OXCompositeFrame *F2, *F2A, *F2B, *F2C, *F2D;	//tab2 subframes 
  OXCompositeFrame *F3, *F3A, *F3B, *F3C, *F3D;	//tab3 subframes

  OLayoutHints *TabLayout;
  OXCheckButton *Subfolders;
  OXIcon *mainicon;
  OXListBox *Output;
  OXComboBox *SearchIn;

  OXCompositeFrame *Buttons, *Buttons1;	        // Top master frames
  OXCompositeFrame *Buttons2;			// Botton master frames
  OXTextEntry *TextE, *TextU, *TextG;
  OXButton *Find, *Stop, *NewSearch;
  OLayoutHints *ButtonLayout, *ButtonFrameLayout;
};


////////////////////////////////////////////////////////////
// main
////////////////////////////////////////////////////////////

int main() {

  OXClient *clientX = new OXClient;

  // pre-set variables
  strcpy(group2, "");
  strcpy(dir1, "/");

  OXMain *mainWindow = new OXMain(clientX->GetRoot(), 450, 250);
  mainWindow->MapWindow();

  clientX->Run();

  return 0;
}

///////////////////////////////////////////////
// Main Body of Program
///////////////////////////////////////////////

OXMain::OXMain(const OXWindow *p, int w, int h) :
  OXMainFrame(p, w, h) {

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

  OXCompositeFrame *tmp_frame;

  CommonCLayout = new OLayoutHints(LHINTS_CENTER_X | LHINTS_CENTER_Y, 2, 2, 2, 2);
  CommonXLayout = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y, 2, 2, 2, 2);
  CommonTLayout = new OLayoutHints(LHINTS_TOP | LHINTS_CENTER_Y, 2, 2, 2, 2);
  CommonTTLayout = new OLayoutHints(LHINTS_TOP | LHINTS_LEFT, 2, 2, 2, 2);
  CommonBLayout = new OLayoutHints(LHINTS_BOTTOM | LHINTS_CENTER_X, 2, 2, 2, 2);
  CommonLLayout = new OLayoutHints(LHINTS_LEFT | LHINTS_CENTER_Y, 2, 2, 2, 2);
  CommonRLayout = new OLayoutHints(LHINTS_RIGHT | LHINTS_CENTER_Y, 2, 2, 2, 2);
  CommonSLayout = new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 2, 2, 2, 2);
  TabLayout = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y, 5, 5, 5, 5);

  Buttons = new OXCompositeFrame(this, 550, 220, HORIZONTAL_FRAME);

  Main = new OXTab(Buttons, 10, 10);
  Buttons->AddFrame(Main, CommonXLayout);

  ///// TAB 1 ///// 

  tmp_frame = Main->AddTab(new OString("Name & Location"));

  F1 = new OXCompositeFrame(tmp_frame, 1, 1, VERTICAL_FRAME);
  F1A = new OXCompositeFrame(F1, 1, 1, HORIZONTAL_FRAME);
  F1->AddFrame(F1A, CommonXLayout);
  F1A->AddFrame(new OXLabel(F1A, new OString("Search for file or directory:")),
	        CommonXLayout);

  TBText1 = new OTextBuffer(50);
  TextE = new OXTextEntry(F1A, TBText1);
  F1A->AddFrame(TextE, CommonXLayout);
  TextE->Associate(this);

  F1B = new OXCompositeFrame(F1, 1, 1, HORIZONTAL_FRAME);
  F1->AddFrame(F1B, CommonXLayout);
  F1B->AddFrame(new OXLabel(F1B, new OString("Start search in the root of: ")),
	        CommonXLayout);

  SearchIn = new OXComboBox(F1B, (char *) "", 5);
  F1B->AddFrame(SearchIn, CommonXLayout);

  SearchIn->AddEntry(new OString("/"), 1);
#if 0
  SearchIn->Select(1);
#else
  SearchIn->SetText((char *) "");
#endif
  SearchIn->AddEntry(new OString("/etc/"), 2);
  SearchIn->AddEntry(new OString("/home/"), 3);
  SearchIn->AddEntry(new OString("/usr/"), 4);
  SearchIn->AddEntry(new OString("/usr/local/"), 5);
  SearchIn->AddEntry(new OString("/var/"), 6);
  SearchIn->Associate(this);

  F1C = new OXCompositeFrame(F1, 1, 1, HORIZONTAL_FRAME);
  F1->AddFrame(F1C, CommonXLayout);

  Subfolders = new OXCheckButton(F1C, new OHotString("Search Sub&directories"), 6);
  F1C->AddFrame(Subfolders, CommonCLayout);
  Subfolders->Associate(this);

  F1D = new OXCompositeFrame(F1, 1, 1, HORIZONTAL_FRAME);
  F1->AddFrame(F1D, CommonXLayout);

  F1D->AddFrame(new OXIcon(F1D, FileP, 29, 29), CommonRLayout);
  tmp_frame->AddFrame(F1, TabLayout);

  //// TAB 2 ////

  tmp_frame = Main->AddTab(new OString("User & Group"));
  F2 = new OXCompositeFrame(tmp_frame, 1, 1, VERTICAL_FRAME);
  F2A = new OXCompositeFrame(F2, 1, 1, HORIZONTAL_FRAME);
  F2->AddFrame(F2A, CommonXLayout);

  F2A->AddFrame(new OXLabel(F2A, new OString("Search for file by user ownership:  ")),
	       CommonLLayout);

  TBUser1 = new OTextBuffer(50);
  TextU = new OXTextEntry(F2A, TBUser1);
  F2A->AddFrame(TextU, CommonXLayout);
  TextU->Associate(this);

  F2B = new OXCompositeFrame(F2, 1, 1, HORIZONTAL_FRAME);
  F2->AddFrame(F2B, CommonXLayout);
  F2B->AddFrame(new OXLabel(F2B, new OString("Search for file by group ownership:")),
	       CommonLLayout);

  TBGroup1 = new OTextBuffer(50);
  TextG = new OXTextEntry(F2B, TBGroup1);
  F2B->AddFrame(TextG, CommonXLayout);
  TextG->Associate(this);

  F2C = new OXCompositeFrame(F2, 1, 1, HORIZONTAL_FRAME);
  F2->AddFrame(F2C, CommonXLayout);
//  F2C->AddFrame(new OXLabel(F2C, new OString("  ")), CommonXLayout);

  F2D = new OXCompositeFrame(F2, 1, 1, HORIZONTAL_FRAME);
  F2->AddFrame(F2D, CommonXLayout);

  F2D->AddFrame(new OXIcon(F2D, UserP, 29, 29), CommonRLayout);
  tmp_frame->AddFrame(F2, TabLayout);

  //// Tab 3 /////

  tmp_frame = Main->AddTab(new OString("Time"));

  F3 = new OXCompositeFrame(tmp_frame, 1, 1, VERTICAL_FRAME);
  F3A = new OXCompositeFrame(F3, 1, 1, HORIZONTAL_FRAME);
  F3->AddFrame(F3A, CommonXLayout);

  F3A->AddFrame(new OXLabel(F3A, new OString("Search by the number of days since last accessed:")),
                CommonLLayout);

  TBText2 = new OTextBuffer(50);
  TextU = new OXTextEntry(F3A, TBText2);
  F3A->AddFrame(TextU, CommonXLayout);
  TextU->Associate(this);

  F3B = new OXCompositeFrame(F3, 1, 1, HORIZONTAL_FRAME);
  F3->AddFrame(F3B, CommonXLayout);
  F3B->AddFrame(new OXLabel(F3B, new OString("Search by the number of days since last modified:  ")),
	       CommonLLayout);

  TBText3 = new OTextBuffer(50);
  TextG = new OXTextEntry(F3B, TBText3);
  F3B->AddFrame(TextG, CommonXLayout);

  F3C = new OXCompositeFrame(F3, 1, 1, HORIZONTAL_FRAME);
  F3->AddFrame(F3C, CommonXLayout);
  F3C->AddFrame(new OXLabel(F3C, new OString("Search by the number of days since last altered:    ")),
	       CommonLLayout);

  TBText4 = new OTextBuffer(50);
  TextG = new OXTextEntry(F3C, TBText4);
  F3C->AddFrame(TextG, CommonXLayout);

  F3D = new OXCompositeFrame(F3, 1, 1, HORIZONTAL_FRAME);
  F3->AddFrame(F3D, CommonXLayout);
  F3D->AddFrame(new OXIcon(F3D, DateP, 29, 29), CommonRLayout);

  tmp_frame->AddFrame(F3, TabLayout);

  ////// Buttons ///////////

  Buttons2 = new OXCompositeFrame(Buttons, 10, 10, VERTICAL_FRAME | FIXED_WIDTH);
  Buttons->AddFrame(Buttons2, CommonRLayout);

  Find = new OXTextButton(Buttons2, new OHotString("&Find"), 1);
  Find->Associate(this);

  Stop = new OXTextButton(Buttons2, new OHotString("&Stop"), 2);
  Stop->Associate(this);

  NewSearch = new OXTextButton(Buttons2, new OHotString("&New Search"), 3);
  NewSearch->Associate(this);

  mainicon = new OXIcon(Buttons2, Glass, 39, 39);

  int width = Find->GetDefaultWidth();
  width = max(width, Stop->GetDefaultWidth());
  width = max(width, NewSearch->GetDefaultWidth());

  ButtonLayout = new OLayoutHints(LHINTS_RIGHT, 5, 0, 0, 0);
  Buttons2->AddFrame(Find, CommonXLayout);
  Buttons2->AddFrame(Stop, CommonXLayout);
  Buttons2->AddFrame(NewSearch, CommonXLayout);
  Buttons2->AddFrame(mainicon, CommonCLayout);

  Buttons2->Resize(width + 30, Buttons2->GetDefaultHeight());
//  Buttons->Resize((width + 30) * 3, Buttons->GetDefaultHeight());

  ////// Output area ///////

  Buttons1 = new OXCompositeFrame(this, 450, 100, HORIZONTAL_FRAME);
  Output = new OXListBox(Buttons1, 12);
  Buttons1->AddFrame(Output, CommonXLayout);
  Output->Associate(this);

  //// Place master frames on window

  AddFrame(Buttons, CommonSLayout);
  AddFrame(Buttons1, CommonXLayout);

  //// last touches

  _exec = NULL;
  _ifh = NULL;
  _ofh = NULL;
  _efh = NULL;

  _cmd = NULL;
  _output = new OString("");

  _exited = False;

  SetWindowName("Find: All Files");

  MapSubwindows();
  Layout();

  MapWindow();
}


OXMain::~OXMain() {

  if (_exec) delete _exec;

  if (_ifh) delete _ifh;
  if (_ofh) delete _ofh;
  if (_efh) delete _efh;

  if (_cmd) delete _cmd;
  delete _output;
}

///////////////////////////////////////
// handle widget giving input
///////////////////////////////////////

int OXMain::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;

  switch (msg->type) {
    case MSG_BUTTON:
      switch (msg->action) {

      case MSG_CLICK:
        switch (wmsg->id) {
          case 1:	//// If find button was presed
            string1 = (char *) TBText1->GetString();
            strcpy(string2, string1);
            strcpy(dir1, SearchIn->GetText());

	    if (strlen(string2) > 0) {	// make sure something was input
	      // Do search 
              StartSearch();

#if 0
	      //// Resize Widgets for output
	      //// Loop used to look like motion.... 
	      for (xx = 0; xx < 80; xx++) {
	        Resize(444, 280 + xx - 55);
	        Buttons1->Resize(444, xx - 55);
	        Output->Resize(444, xx);
	      }
#endif

#if 0
	      //// Read Input
	      fp = fopen("/tmp/.tmp.find", "r");
	      xx = 0;
	      do {
	        xx++;
	        fscanf(fp, "%s", &fileinput);
	        Output->AddEntry(new OString(fileinput), xx);
	      } while (strcmp(fileinput, "End") != 0);
	      fclose(fp);
#else
              xx = 0;
#endif
	    }
            break;

          case 0:
	    printf("Do something \n");
            break;

          case 2:  // stop search
            StopSearch();
            //CloseWindow();
	    //exit(0);
            break;

          case 3:
            Output->RemoveAllEntries();
            string1 = (char *) TBText1->GetString();
            strcpy(string2, string1);
            strcpy(dir1, SearchIn->GetText());
	    if (strlen(string2) > 0) StartSearch();
            break;
        }
        break;

      }
      break;

    case MSG_EXEC:
      if (msg->action == MSG_APPEXITED) {
        _retc = _exec->GetExitCode();
        if (_exited) {
          char msg[256];
          sprintf(msg, "Program exited normally (%d)\n", _retc);
	  Output->AddEntry(new OString(msg), -1);
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
      if (_exited) {
        sprintf(buf, "find exited normally (%d)\n", _retc);
	Output->AddEntry(new OString(buf), -1);
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
          Output->AddEntry(new OString(_output), xx++);
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

void OXMain::StartSearch() {
  char msg[1024], *prog, **argv;
  int  argc, argvsz, retc;

  if (_cmd) delete _cmd;

  // find <dir1> -name <string2>

  _cmd = new OString("find ");
  _cmd->Append(dir1);
  _cmd->Append(" -name ");
  _cmd->Append(string2);
  _cmd->Append(" -print0");

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
}
