/**************************************************************************

    This is a program intended for testing the OExec class.
    Copyright (C) 2001, Hector Peraza.

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
#include <xclass/OXMenu.h>
#include <xclass/OXLabel.h>
#include <xclass/OXMsgBox.h>
#include <xclass/OXTextButton.h>
#include <xclass/OXTextEntry.h>
#include <xclass/OX3dLines.h>
#include <xclass/OXTextEdit.h>
#include <xclass/OFileHandler.h>
#include <xclass/OString.h>
#include <xclass/OExec.h>
#include <xclass/utils.h>


#define M_FILE_OPEN      1
#define M_FILE_EXEC      2
#define M_FILE_EXIT      3


//----------------------------------------------------------------------

class OXMain : public OXMainFrame {
public:
  OXMain(const OXWindow *p, int w, int h);
  virtual ~OXMain();

  virtual int ProcessMessage(OMessage *msg);
  virtual int HandleFileEvent(OFileHandler *fh, unsigned int mask);

  void DoExec();

protected:
  OXTextEdit *_te;
  OLayoutHints *_menuBarLayout, *_menuBarItemLayout;
  OXMenuBar *_menuBar;
  OXPopupMenu *_menuFile;
  OFileHandler *_ifh, *_ofh, *_efh;
  OExec *_exec;
  OString *_cmd;
  int _exited, _retc;
};


class OXCmdDlg : public OXTransientFrame {
public:
  OXCmdDlg(const OXWindow *p, const OXWindow *main,
           OString *title, OString *cmd = NULL, int *ret_code = NULL,
           unsigned long options = MAIN_FRAME | VERTICAL_FRAME);
  virtual ~OXCmdDlg();

  virtual int CloseWindow();
  virtual int ProcessMessage(OMessage *msg);

protected:
  OLayoutHints *L1, *L2, *L4, *L5;
  OXButton *Ok, *Cancel;
  OXTextEntry *_te;
  int *_ret_code;
  OString *_cmd;
};


//----------------------------------------------------------------------

main() {

  OXClient *clientX = new OXClient;

  OXMain *mainWindow = new OXMain(clientX->GetRoot(), 400, 200);
  mainWindow->MapWindow();

  clientX->Run();

  return 0;
}

OXMain::OXMain(const OXWindow *p, int w, int h) :
  OXMainFrame(p, w, h) {
  char tmp[BUFSIZ];

  _menuBarLayout = new OLayoutHints(LHINTS_TOP | LHINTS_LEFT | LHINTS_EXPAND_X, 0, 0, 1, 1);
  _menuBarItemLayout = new OLayoutHints(LHINTS_TOP | LHINTS_LEFT, 0, 4, 0, 0);

  _menuFile = new OXPopupMenu(_client->GetRoot());
  _menuFile->AddEntry(new OHotString("&Exec command..."), M_FILE_EXEC);
  _menuFile->AddSeparator();
  _menuFile->AddEntry(new OHotString("E&xit"), M_FILE_EXIT);

  _menuFile->Associate(this);

  _menuBar = new OXMenuBar(this, 1, 1, HORIZONTAL_FRAME);
  _menuBar->AddPopup(new OHotString("&File"), _menuFile, _menuBarItemLayout);

  AddFrame(_menuBar, _menuBarLayout);

  _te = new OXTextEdit(this, 500, 300, SUNKEN_FRAME | DOUBLE_BORDER);
  _te->SetReadOnly(True);

  AddFrame(_te, new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y));

  _exec = NULL;
  _ifh = NULL;
  _ofh = NULL;
  _efh = NULL;

  _cmd = new OString("ping -c 10 localhost");

  _exited = False;

  SetWindowName("Test");

  MapSubwindows();
  Resize(GetDefaultSize());
}

OXMain::~OXMain() {
  if (_exec) delete _exec;

  if (_ifh) delete _ifh;
  if (_ofh) delete _ofh;
  if (_efh) delete _efh;

  delete _menuBarLayout;
  delete _menuBarItemLayout;

  delete _menuFile;
}

int OXMain::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;

  switch (msg->type) {

    case MSG_MENU:
      switch (msg->action) {

        case MSG_CLICK:
          switch (wmsg->id) {

            case M_FILE_EXEC:
              DoExec();
              break;

            case M_FILE_EXIT:
              CloseWindow();
              break;

            default:
              break;

          }
          break;

        default:
          break;

      }
      break;

    case MSG_EXEC:
      if (msg->action == MSG_APPEXITED) {
        _retc = _exec->GetExitCode();
        if (_exited) {
          char msg[256];
          sprintf(msg, "Program exited normally (%d)\n", _retc);
          _te->InsertText(msg);
        } else {
          _exited = True;
        }
      }
      break;

    default:
      break;

  }

  return True;
}


int OXMain::HandleFileEvent(OFileHandler *fh, unsigned int mask) {
  char buf[256];
  int  retc;

  if (fh == _ifh) {
    if (mask != XCM_WRITABLE) return False;
    delete _ifh;
    _ifh = NULL;
    sprintf(buf, "Application wants input!");
    new OXMsgBox(_client->GetRoot(), this, new OString("Test"),
                 new OString(buf), MB_ICONSTOP, ID_OK);
    //retc = _exec->Write(buf, strlen(buf));
  } else if (fh == _ofh) {
    if (mask != XCM_READABLE) return False;
    retc = _exec->Read(buf, sizeof(buf));
    if ((retc == 0) && (errno != EWOULDBLOCK) && (errno != EAGAIN)) {
      if (_exited) {
        sprintf(buf, "Program exited normally (%d)\n", _retc);
        _te->InsertText(buf);
      } else {
        _exited = True;
      }
      delete _ofh;
      _ofh = NULL;
    } else if (retc > 0) {
      _te->InsertText(buf, retc);
    }
  } else if (fh == _efh) {
    if (mask != XCM_READABLE) return False;
    retc = _exec->ReadError(buf, sizeof(buf));
    if ((retc == 0) && (errno != EWOULDBLOCK) && (errno != EAGAIN)) {
      delete _efh;
      _efh = NULL;
    } else if (retc > 0) {
      _te->InsertText("\nstderr: ");
      _te->InsertText(buf, retc);
    }
  } else {
    return False;
  }

  return True;
}

void OXMain::DoExec() {
  char msg[1024], *prog, **argv;
  int  argc, argvsz, retc;

  if (_exec) {
    if (_exec->IsRunning()) {
      sprintf(msg, "Application still running, will be killed now!");
      new OXMsgBox(_client->GetRoot(), this, new OString("Test"),
                   new OString(msg), MB_ICONSTOP, ID_OK);
    }
    delete _exec; _exec = NULL;

    if (_ifh) delete _ifh; _ifh = NULL;
    if (_ofh) delete _ofh; _ofh = NULL;
    if (_efh) delete _efh; _efh = NULL;
  }

  new OXCmdDlg(_client->GetRoot(), this, new OString("Exec"),
               _cmd, &retc);
  if (retc == ID_CANCEL) return;

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

  _te->InsertText("Executing ");
  _te->InsertText(prog);
  _te->InsertText("...\n");

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


//---------------------------------------------------------------------

OXCmdDlg::OXCmdDlg(const OXWindow *p, const OXWindow *main,
                   OString *title, OString *cmd, int *ret_code,
                   unsigned long options) :
  OXTransientFrame(p, main, 10, 10, options) {

    int ax, ay, width = 0, height = 0;
    unsigned int root_w, root_h, dummy;
    Window wdummy;

    _ret_code = ret_code;
    _cmd = cmd;

    L1 = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_TOP, 3, 3, 5, 5);

    //--- create the frame for the message entry box

    OXCompositeFrame *hframe = new OXHorizontalFrame(this, 0, 0, 0);
    _te = new OXTextEntry(hframe, new OTextBuffer(100));
    _te->Resize(250, _te->GetDefaultHeight());
    if (_cmd) {
      _te->AddText(0, _cmd->GetString());
    }

    L4 = new OLayoutHints(LHINTS_CENTER_Y | LHINTS_LEFT, 0, 5, 5, 5);
    L5 = new OLayoutHints(LHINTS_RIGHT | LHINTS_EXPAND_X | LHINTS_CENTER_Y);

    hframe->AddFrame(new OXLabel(hframe, new OString("Command:")), L4);
    hframe->AddFrame(_te, L5);

    AddFrame(hframe, L1);
    AddFrame(new OXHorizontal3dLine(this), L1);

    //--- create the buttons and button frame

    OXCompositeFrame *ButtonFrame = new OXHorizontalFrame(this,
                                                    60, 20, FIXED_WIDTH);

    Ok = new OXTextButton(ButtonFrame, new OHotString("&Execute"), ID_OK);
    Ok->SetDefault();
    Ok->Associate(this);
    ButtonFrame->AddFrame(Ok, L1);
    width = max(width, Ok->GetDefaultWidth());

    Cancel = new OXTextButton(ButtonFrame, new OHotString("&Cancel"), ID_CANCEL);
    Cancel->Associate(this);
    ButtonFrame->AddFrame(Cancel, L1);
    width = max(width, Cancel->GetDefaultWidth());

    //--- place buttons at the bottom

    L2 = new OLayoutHints(LHINTS_BOTTOM | LHINTS_CENTER_X);
    AddFrame(ButtonFrame, L2);

    //--- Keep the buttons centered and with the same width

    ButtonFrame->Resize((width + 20) * 2, GetDefaultHeight());

    SetFocusOwner(_te);
    MapSubwindows();
    
    SetDefaultAcceptButton(Ok);
    SetDefaultCancelButton(Cancel);

    width  = GetDefaultWidth();
    height = GetDefaultHeight();

    Resize(width, height);

    //--- position relative to the parent's window

    if (main) {
      XTranslateCoordinates(GetDisplay(),
                            main->GetId(), GetParent()->GetId(),
                            (((OXFrame *) main)->GetWidth() - _w) >> 1,
                            (((OXFrame *) main)->GetHeight() - _h) >> 1,
                            &ax, &ay, &wdummy);
    } else {
      XGetGeometry(GetDisplay(), _client->GetRoot()->GetId(), &wdummy,
                   &ax, &ay, &root_w, &root_h, &dummy, &dummy);
      ax = (root_w - _w) >> 1;
      ay = (root_h - _h) >> 1;
    }

    Move(ax, ay);
    SetWMPosition(ax, ay);

    //---- make the dialog box non-resizable

    SetWMSize(width, height);
    SetWMSizeHints(width, height, width, height, 0, 0);

    //---- set names

    SetWindowName((char *) title->GetString());
    SetIconName((char *) title->GetString());
    SetClassHints("MsgBox", "MsgBox");

    SetMWMHints(MWM_DECOR_ALL | MWM_DECOR_RESIZEH | MWM_DECOR_MAXIMIZE |
                                MWM_DECOR_MINIMIZE | MWM_DECOR_MENU,
                MWM_FUNC_ALL | MWM_FUNC_RESIZE | MWM_FUNC_MAXIMIZE |
                               MWM_FUNC_MINIMIZE,
                MWM_INPUT_MODELESS);

    MapWindow();
    _client->WaitFor(this);
}

OXCmdDlg::~OXCmdDlg() {
  delete L1;
  delete L2;
  delete L4;
  delete L5;
}

int OXCmdDlg::CloseWindow() {
  if (_ret_code) *_ret_code = ID_CANCEL;
  return OXTransientFrame::CloseWindow();
}

int OXCmdDlg::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;

  switch (msg->type) {
  case MSG_BUTTON:
    switch (msg->action) {
    case MSG_CLICK:
      if (_ret_code) *_ret_code = wmsg->id;
      if (_cmd) {
        _cmd->Clear();
        _cmd->Append(_te->GetString());
      }
      delete this;
      break;

    default:
      break;
    }
    break;
  }
  return True;
}
