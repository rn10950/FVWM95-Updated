#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <X11/keysym.h>

#include <xclass/OXFileDialog.h>

#include "OXViewLogFile.h"
#include "OXPreferences.h"
#include "OXIrc.h"

#include "versiondef.h"


//----------------------------------------------------------------------

extern OLayoutHints *topleftlayout;
extern OLayoutHints *topexpandxlayout;
extern OLayoutHints *topexpandxlayout0;
extern OLayoutHints *topexpandxlayout1;
extern OLayoutHints *topexpandxlayout2;
extern OLayoutHints *topexpandxlayout3;
extern OLayoutHints *leftexpandylayout;
extern OLayoutHints *leftcenterylayout;
extern OLayoutHints *expandxexpandylayout;
extern OLayoutHints *menubarlayout;
extern OLayoutHints *menuitemlayout;

extern OSettings *foxircSettings;

extern char *filetypes[];

#define M_FILE_OPEN        101
#define M_FILE_PRINT       102
#define M_FILE_EXIT        103

#define M_EDIT_COPY        201
#define M_EDIT_SELECTALL   202
#define M_EDIT_INVERTSEL   203
#define M_EDIT_FIND        204

#define M_VIEW_TOOLBAR     301
#define M_VIEW_STATUSBAR   302
#define M_VIEW_FONT        303
#define M_VIEW_COLORS      304

#define M_HELP_ABOUT       401


//----------------------------------------------------------------------

OXViewLogFile::OXViewLogFile(const OXWindow *p, const OXWindow *main,
                             OXIrc *irc, const char *fname) :
  OXTransientFrame(p, main, 200, 100) {

  _irc = irc;

  SetLayoutManager(new OVerticalLayout(this));

  _menuFile = new OXPopupMenu(_client->GetRoot());
  _menuFile->AddEntry(new OHotString("&Open..."),  M_FILE_OPEN);
  _menuFile->AddEntry(new OHotString("&Print..."), M_FILE_PRINT);
  _menuFile->AddSeparator();
  _menuFile->AddEntry(new OHotString("E&xit"), M_FILE_EXIT);
  _menuFile->DisableEntry(M_FILE_PRINT);

  _menuEdit = new OXPopupMenu(_client->GetRoot());
  _menuEdit->AddEntry(new OHotString("&Copy"),  M_EDIT_COPY);
  _menuEdit->AddSeparator();
  _menuEdit->AddEntry(new OHotString("Select &all"), M_EDIT_SELECTALL);
  _menuEdit->AddEntry(new OHotString("In&vert selection"), M_EDIT_INVERTSEL);
  _menuEdit->AddSeparator();
  _menuEdit->AddEntry(new OHotString("&Find..."), M_EDIT_FIND);

  _menuView = new OXPopupMenu(_client->GetRoot());
  _menuView->AddEntry(new OHotString("&Toolbar"),  M_VIEW_TOOLBAR);
  _menuView->AddEntry(new OHotString("Status &Bar"), M_VIEW_STATUSBAR);
  _menuView->AddSeparator();
  _menuView->AddEntry(new OHotString("&Font..."), M_VIEW_FONT);
  _menuView->AddEntry(new OHotString("&Colors..."), M_VIEW_COLORS);

  _menuHelp = new OXPopupMenu(_client->GetRoot());
  _menuHelp->AddEntry(new OHotString("&About..."),  M_HELP_ABOUT);

  _menuFile->Associate(this);
  _menuEdit->Associate(this);
  _menuView->Associate(this);
  _menuHelp->Associate(this);

  _menuBar = new OXMenuBar(this, 1, 1, HORIZONTAL_FRAME);

  _menuBar->AddPopup(new OHotString("&File"), _menuFile, menuitemlayout);
  _menuBar->AddPopup(new OHotString("&Edit"), _menuEdit, menuitemlayout);
  _menuBar->AddPopup(new OHotString("&View"), _menuView, menuitemlayout);
  _menuBar->AddPopup(new OHotString("&Help"), _menuHelp, menuitemlayout);

  AddFrame(_menuBar, menubarlayout);

  _logw = new OXViewDoc(this, _log = new OTextDoc(), 10, 10,
                        SUNKEN_FRAME | DOUBLE_BORDER);
  _logw->SetScrollOptions(FORCE_VSCROLL | NO_HSCROLL);

  AddFrame(_logw, expandxexpandylayout);

  _statusBar = new OXStatusBar(this);
  AddFrame(_statusBar, new OLayoutHints(LHINTS_BOTTOM | LHINTS_EXPAND_X,
                               0, 0, 3, 0));

  Resize(610, 360);

  MapSubwindows();
  Layout();

  SetWindowName(FOXIRC_NAME);

  if (fname) Load(fname);

  CenterOnParent();
  MapWindow();
}

OXViewLogFile::~OXViewLogFile() {
  delete _menuFile;
  delete _menuView;
  delete _menuHelp;
}

//------------------------------------------------------------------------

int OXViewLogFile::CloseWindow() {
  if (_irc) _irc->ViewLogFileClosed();
  return OXTransientFrame::CloseWindow();
}

int OXViewLogFile::Load(const char *fname) {
  char tmp[256];

  int retc = _logw->LoadFile(fname);

  snprintf(tmp, 256, "%lu lines read", _log->NumLines());
  _statusBar->SetText(0, new OString(tmp));

  snprintf(tmp, 256, "%s - %s", FOXIRC_NAME, fname);
  SetWindowName(tmp);

  return retc;
}

int OXViewLogFile::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;

  switch (msg->type) {
    case MSG_MENU:
      switch (msg->action) {
        case MSG_CLICK:
          switch (wmsg->id) {
            case M_FILE_OPEN:
              {
                OFileInfo fi;

                fi.MimeTypesList = NULL;
                fi.file_types = filetypes;
                new OXFileDialog(_client->GetRoot(), this, FDLG_OPEN, &fi);
                if (fi.filename) Load(fi.filename);
              }
              break;

            case M_FILE_EXIT:
              CloseWindow();
              break;
          }
          break;
      }
      break;
  }

  return True;
}
