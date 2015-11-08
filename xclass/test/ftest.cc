/**************************************************************************

    This is a program intended for testing xclass file events.
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
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <xclass/OXClient.h>
#include <xclass/OXWindow.h>
#include <xclass/OXFrame.h>
#include <xclass/OXMenu.h>
#include <xclass/OXLabel.h>
#include <xclass/OXMsgBox.h>
#include <xclass/OMimeTypes.h>
#include <xclass/OXFileDialog.h>

#include <xclass/OTimer.h>
#include <xclass/OFileHandler.h>
#include <xclass/OIdleHandler.h>
#include <xclass/OString.h>

#include <xclass/utils.h>


#define M_FILE_OPEN      1
#define M_FILE_IDLE      2
#define M_FILE_EXIT      3


//---------------------------------------------------------------------

class OXMain : public OXMainFrame {
public:
  OXMain(const OXWindow *p, int w, int h);
  virtual ~OXMain();

  virtual int ProcessMessage(OMessage *msg);
  virtual int HandleTimer(OTimer *t);
  virtual int HandleFileEvent(OFileHandler *fh, unsigned int mask);
  virtual int HandleIdleEvent(OIdleHandler *ih);

  void DoLoad();
  void DoToggleIdle();
  void UpdateStatus();

protected:
  OXCanvas *_canvas;
  OXCompositeFrame *_container;
  OXLabel *_time, *_status;

  OLayoutHints *_menuBarLayout, *_menuBarItemLayout;
  OLayoutHints *_stLayout, *_tLayout;

  OXMenuBar *_menuBar;
  OXPopupMenu *_menuFile;

  OTimer *_timer;
  OFileHandler *_fh;
  OIdleHandler *_ih;

  int _fd, _count, _reading, _wastingTime;
};

//---------------------------------------------------------------------

OXClient *clientX;
OXMain *mainWindow;

OMimeTypes *MimeTypeList;  // currently we need this for OXFileDialog

char *filetypes[] = { "All files",      "*",
                      NULL,             NULL };

main() {
  char mimerc[PATH_MAX];

  clientX = new OXClient;

  sprintf(mimerc, "%s/.mime.types", getenv("HOME"));
  MimeTypeList = new OMimeTypes(clientX, mimerc);

  mainWindow = new OXMain(clientX->GetRoot(), 400, 200);
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
  _menuFile->AddEntry(new OHotString("&Open..."), M_FILE_OPEN);
  _menuFile->AddEntry(new OHotString("&Toggle idle event"), M_FILE_IDLE);
  _menuFile->AddSeparator();
  _menuFile->AddEntry(new OHotString("E&xit"), M_FILE_EXIT);

  _menuFile->Associate(this);

  _menuBar = new OXMenuBar(this, 1, 1, HORIZONTAL_FRAME);
  _menuBar->AddPopup(new OHotString("&File"), _menuFile, _menuBarItemLayout);

  AddFrame(_menuBar, _menuBarLayout);

  _canvas = new OXCanvas(this, 200, 100);
  _container = new OXCompositeFrame(_canvas->GetViewPort(), 10, 10, 
                                    VERTICAL_FRAME | OWN_BKGND,
                                    _whitePixel);
  _canvas->SetContainer(_container);

  _time = new OXLabel(_container, new OString(" --:--:-- AM "));
  _tLayout = new OLayoutHints(LHINTS_EXPAND_Y | LHINTS_EXPAND_X, 5, 5, 5, 5);

  _status = new OXLabel(_container, new OString("Ready"));
  _stLayout = new OLayoutHints(LHINTS_EXPAND_Y | LHINTS_EXPAND_X, 5, 5, 5, 5);

  _canvas->AddFrame(_time, _tLayout);
  _canvas->AddFrame(_status, _stLayout);

  AddFrame(_canvas, new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y));

  _timer = new OTimer(this, 1000);
  _ih = NULL;
  _fh = NULL;
  _count = 0;
  _reading = False;
  _wastingTime = False;

  SetWindowName("Test");

  MapSubwindows();
  Resize(GetDefaultSize());
}

OXMain::~OXMain() {
  if (_timer) delete _timer;
  if (_fh) delete _fh;
  if (_ih) delete _ih;

  delete _stLayout;
  delete _tLayout;
  delete _menuBarLayout;
  delete _menuBarItemLayout;

  delete _menuFile;
}

int OXMain::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;

  switch (msg->action) {

    case MSG_CLICK:
      switch(msg->type) {

        case MSG_MENU:
          switch(wmsg->id) {

            case M_FILE_OPEN:
              DoLoad();
              break;

            case M_FILE_IDLE:
              DoToggleIdle();
              break;

            case M_FILE_EXIT:
              CloseWindow();
              break;

            default:
              break;

          } // switch(id)
          break;

        default:
          break;

      } // switch(type)
      break;

    default:
      break;

  } // switch(action)

  return True;
}

void OXMain::UpdateStatus() {
  char str[40];

  sprintf(str, "%d bytes read, idle %s", _count, _wastingTime ? "on" : "off");
  _status->SetText(new OString(str));
  Layout();
}

int OXMain::HandleTimer(OTimer *t) {
  struct tm *tms;
  char   str[40];
  time_t tmt;

  if (t != _timer) return False;

  time(&tmt);
  tms = localtime(&tmt);
  strftime(str, 15, "%I:%M:%S %p", tms);
  _time->SetText(new OString(str));
  Layout();

  delete _timer;
  _timer = new OTimer(this, 1000);

  return True;
}

int OXMain::HandleFileEvent(OFileHandler *fh, unsigned int mask) {
  char bfr[256];
  int  retc;

//  if (fh->GetFd())...
  if (fh != _fh) return False;
  if (mask != XCM_READABLE) return False;

  retc = read(_fd, bfr, sizeof(bfr));

  if (retc > 0) {
    _count += retc;
  } else if (retc == 0) { // end of file
    close(_fd);
    _reading = False;
    delete _fh;
    _fh = NULL;
  }

  UpdateStatus();

  return True;
}

int OXMain::HandleIdleEvent(OIdleHandler *ih) {

  if (_ih) delete _ih;
  _ih = new OIdleHandler(this);

  UpdateStatus();

  return True;
}

void OXMain::DoLoad() {
  OFileInfo fi;
  char msg[1024];
  int retc;

  if (_reading) {
    // hold this read opertaion...
    if (_fh) delete _fh;
    _fh = NULL;
    // ...and ask the user what to do:
    sprintf (msg, "Still reading, Cancel?");
    new OXMsgBox(_client->GetRoot(), this, new OString("Test"),
                 new OString(msg), MB_ICONSTOP, ID_YES | ID_NO, &retc);
    if (retc == ID_NO) {
      // resume...
      _fh = new OFileHandler(this, _fd, XCM_READABLE);
      return;
    }
    // otherwise finish.
    close(_fd);
    _reading = False;
  }

  fi.MimeTypesList = MimeTypeList;
  fi.file_types = filetypes;
  new OXFileDialog(_client->GetRoot(), this, FDLG_OPEN, &fi);
  if (fi.filename) {
    _fd = open(fi.filename, O_RDONLY | O_NONBLOCK);
    if (_fd == -1) {
      sprintf (msg, "Could not open \"%s\"", fi.filename);
      new OXMsgBox(_client->GetRoot(), this, new OString("Test"),
                   new OString(msg), MB_ICONSTOP, ID_OK);
      return;
    }
    _count = 0;
    _reading = True;
    _fh = new OFileHandler(this, _fd, XCM_READABLE);
  }
}

void OXMain::DoToggleIdle() {
  if (_wastingTime) {
    _wastingTime = False;
    if (_ih) delete _ih;
    _ih = NULL;
  } else {
    _wastingTime = True;
    if (_ih) delete _ih;
    _ih = new OIdleHandler(this);
  }
}
