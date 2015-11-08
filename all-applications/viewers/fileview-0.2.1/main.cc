#include <stdio.h>
#include <string.h>
#include <limits.h>

#include <xclass/OXMsgBox.h>
#include <xclass/OMimeTypes.h>
#include <xclass/OXFileDialog.h>

#include "OXTextView.h"
#include "main.h"


OMimeTypes *MimeTypeList;  // currently we need this for OXFileDialog

char *filetypes[] = { "All files",      "*",
                      "Document files", "*.doc",
                      "Text files",     "*.txt",
                      NULL,             NULL };

//----------------------------------------------------------------------

int main(int argc, char **argv) {
  char mimerc[PATH_MAX];

  OXClient clientX;

  OXAppMainFrame *main = new OXAppMainFrame(clientX.GetRoot(), 10, 10);

  sprintf(mimerc, "%s/.mime.types", getenv("HOME"));
  MimeTypeList = new OMimeTypes(&clientX, mimerc);

  if (argc > 1) main->LoadFile(argv[1]);

  main->Resize(600, 400);
  main->MapWindow();

  clientX.Run();

  exit(0);
}

OXAppMainFrame::OXAppMainFrame(const OXWindow *p, int w, int h) :
  OXMainFrame(p, w, h) {
  char tmp[BUFSIZ];

  _menuBarLayout = new OLayoutHints(LHINTS_TOP | LHINTS_LEFT | LHINTS_EXPAND_X, 
                                    0, 0, 1, 1);
  _menuBarItemLayout = new OLayoutHints(LHINTS_TOP | LHINTS_LEFT,
                                    0, 4, 0, 0);

  _menuFile = new OXPopupMenu(_client->GetRoot());
  _menuFile->AddEntry(new OHotString("&Open..."), M_FILE_OPEN);
  _menuFile->AddSeparator();
  _menuFile->AddEntry(new OHotString("&Print"), M_FILE_PRINT);
  _menuFile->AddEntry(new OHotString("P&rint setup..."), M_FILE_PRINTSETUP);
  _menuFile->AddSeparator();
  _menuFile->AddEntry(new OHotString("E&xit"), M_FILE_EXIT);

  _menuFile->DisableEntry(M_FILE_PRINT);
  _menuFile->DisableEntry(M_FILE_PRINTSETUP);

  _menuEdit = new OXPopupMenu(_client->GetRoot());
  _menuEdit->AddEntry(new OHotString("C&opy"), M_EDIT_COPY);

  _menuEdit->DisableEntry(M_EDIT_COPY);

  _menuView = new OXPopupMenu(_client->GetRoot());
  _menuView->AddEntry(new OHotString("&Status bar"), M_VIEW_STATUSBAR);
  _menuView->AddSeparator();
  _menuView->AddEntry(new OHotString("Change &Font..."), M_VIEW_CHANGEFONT);
  _menuView->CheckEntry(M_VIEW_STATUSBAR);

  _menuView->DisableEntry(M_VIEW_CHANGEFONT);

  _menuHelp = new OXPopupMenu(_client->GetRoot());
  _menuHelp->AddEntry(new OHotString("&Contents"), M_HELP_CONTENTS);
  _menuHelp->AddEntry(new OHotString("&Search..."), M_HELP_SEARCH);
  _menuHelp->AddSeparator();
  _menuHelp->AddEntry(new OHotString("&About"), M_HELP_ABOUT);

  _menuHelp->DisableEntry(M_HELP_CONTENTS);
  _menuHelp->DisableEntry(M_HELP_SEARCH);
  _menuHelp->DisableEntry(M_HELP_ABOUT);

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

  _tv = new OXTextView(this, 10, 10, SUNKEN_FRAME | DOUBLE_BORDER);
  AddFrame(_tv, new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y));

  _status = new OXStatusBar(this);
  AddFrame(_status, new OLayoutHints(LHINTS_BOTTOM | LHINTS_EXPAND_X,
                                     0, 0, 3, 0));

  SetWindowName("File Viewer");
  SetClassHints("FileView", "FileView");

  MapSubwindows();
}

OXAppMainFrame::~OXAppMainFrame() {
  delete _menuBarLayout;
  delete _menuBarItemLayout;

  delete _menuFile;
  delete _menuEdit;
  delete _menuView;
  delete _menuHelp;
}

void OXAppMainFrame::LoadFile(char *filename) {
  char *p, tmp[1024];

  if (!_tv->LoadFile(filename)) {
    sprintf (tmp, "Error opening file \"%s\"", filename);
    new OXMsgBox(_client->GetRoot(), this, new OString("File Viewer"),
                 new OString(tmp), MB_ICONSTOP, ID_OK);
    return;
  }

  if ((p = strrchr(filename, '/')) == NULL) {
    p = filename;
  } else {
    ++p;
  }
  sprintf(tmp, "%s: %d lines read.", p, _tv->GetTextFrame()->GetLines());
  _status->SetText(0, new OString(tmp));
}

int OXAppMainFrame::ProcessMessage(OMessage *msg) {
  OFileInfo fi;
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;

  switch (msg->action) {

    case MSG_CLICK:
      switch(msg->type) {

        case MSG_MENU:
          switch(wmsg->id) {

            case M_FILE_OPEN:
              fi.MimeTypesList = MimeTypeList;
              fi.file_types = filetypes;
              new OXFileDialog(_client->GetRoot(), this, FDLG_OPEN, &fi);
              if (fi.filename) {
                LoadFile(fi.filename);
                // delete fi.filename; this is deleted on OFileInfo destructor
              }
              break;

            case M_FILE_EXIT:
              CloseWindow();
              break;

            case M_VIEW_STATUSBAR:
              if (_status->IsVisible()) {
                HideFrame(_status);
                _menuView->UnCheckEntry(M_VIEW_STATUSBAR);
              } else {
                ShowFrame(_status);
                _menuView->CheckEntry(M_VIEW_STATUSBAR);
              }
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
