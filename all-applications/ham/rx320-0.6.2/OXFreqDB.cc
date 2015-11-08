/**************************************************************************

    This file is part of rx320, a control program for the Ten-Tec RX320
    receiver. Copyright (C) 2000-2004, Hector Peraza.

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
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <xclass/utils.h>
#include <xclass/OXMsgBox.h>
#include <xclass/OX3dLines.h>
#include <xclass/OXMainFrame.h>
#include <xclass/OXMenu.h>
#include <xclass/OXFileDialog.h>
#include <xclass/OXListView.h>

#include "menudef.h"
#include "toolbardef.h"

#include "OXFreqDB.h"
#include "OXDialogs.h"
#include "main.h"

// TODO:
// - OFDBitem: highlight the full line. Select by clicking anywhere on the
//   line.
// - View options: font size, which columns.
// - Save: as *.320 or *.cdf file.
// - Print database.

char *filetypes[] = { "All files",       "*",
                      "Frequency files", "*.320",
                      "CDF files",       "*.cdf",
                      NULL,              NULL };


//----------------------------------------------------------------------

OFDBitem::OFDBitem(const OXListView *p, int id, OFreqRecord *fr,
                   std::vector<OString *> names) :
  OListViewItem(p, id, NULL, NULL, names, 0) {

    _freqRec = NULL;
    ChangeFreqRecord(fr);
}

OFDBitem::~OFDBitem() {
  if (_freqRec) delete _freqRec;
}

void OFDBitem::ChangeFreqRecord(OFreqRecord *fr) {
  char string[256];

  if (_freqRec) delete _freqRec;
  _freqRec = fr;

  _names.clear();

  // 0: name
  if (strlen(_freqRec->name) == 0)
    strcpy(string, "        ");
  else
    strcpy(string, _freqRec->name);
  _names.push_back(new OString(string));

  // 1: frequency
  sprintf(string, "%10.6f", _freqRec->freq);
  _names.push_back(new OString(string));

  // 2: mode
  if (_freqRec->mode == RX320_USB)
    strcpy(string, "USB");
  else if (_freqRec->mode == RX320_LSB)
    strcpy(string, "LSB");
  else if (_freqRec->mode == RX320_CW)
    strcpy(string, "CW");
  else
    strcpy(string, "AM");
  _names.push_back(new OString(string));

  // 3: bandwitdth
  sprintf(string, "%5d", _freqRec->filter_bw);
  _names.push_back(new OString(string));

  // 4: AGC
  if (_freqRec->agc == RX320_AGC_SLOW)
    strcpy(string, "slow");
  else if (_freqRec->agc == RX320_AGC_FAST)
    strcpy(string, "fast");
  else
    strcpy(string, "medium");
  _names.push_back(new OString(string));

  // 5: tuning step
  sprintf(string, "%5d", _freqRec->tuning_step);
  _names.push_back(new OString(string));

  // 6: PBT offset
  sprintf(string, "%5d", _freqRec->pbt_offset);
  _names.push_back(new OString(string));

  // 7: location
  _names.push_back(new OString(_freqRec->location));

  // 8: language
  _names.push_back(new OString(_freqRec->language));

  // 9: start time
  _names.push_back(new OString(_freqRec->start_time));

  // 10: end time
  _names.push_back(new OString(_freqRec->end_time));

  // 11: notes
  _names.push_back(new OString(_freqRec->notes));

  // 12: offset
  sprintf(string, "%d", _freqRec->offset);
  _names.push_back(new OString(string));

  // 13: lockout
  sprintf(string, "%d", _freqRec->lockout);
  _names.push_back(new OString(string));


  Resize(GetDefaultSize());
  int vm = _viewMode; ++_viewMode;
  SetViewMode(vm);  // hmmm... we need to do this to update _tw 
#if 0
  _parent->NeedRedraw(ORectangle(GetPosition(), GetSize()));
#else
  ((OXListView *)_parent)->Layout();
#endif
}

//----------------------------------------------------------------------

OXFreqDB::OXFreqDB(const OXWindow *p, OXMain *m, int w, int h) :
  OXMainFrame(p, w, h) {

  _exiting = False;

  _rxmain = m;

  prev = next = NULL;

  _menuBarLayout = new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 0, 0, 1, 1);
  _menuBarItemLayout = new OLayoutHints(LHINTS_TOP | LHINTS_LEFT, 0, 4, 0, 0);

  _menuFile = _MakePopup(&file_popup);
  _menuEdit = _MakePopup(&edit_popup);
  _menuView = _MakePopup(&view_popup);
  _menuHelp = _MakePopup(&help_popup);

  _menuFile->Associate(this);
  _menuEdit->Associate(this);
  _menuView->Associate(this);
  _menuHelp->Associate(this);

  //------ menu bar

  _menuBar = new OXMenuBar(this, 1, 1, HORIZONTAL_FRAME);
  _menuBar->AddPopup(new OHotString("&File"), _menuFile, _menuBarItemLayout);
  _menuBar->AddPopup(new OHotString("&Edit"), _menuEdit, _menuBarItemLayout);
  _menuBar->AddPopup(new OHotString("&View"), _menuView, _menuBarItemLayout);
  _menuBar->AddPopup(new OHotString("&Help"), _menuHelp, _menuBarItemLayout);

  AddFrame(_menuBar, _menuBarLayout);

  //---- toolbar

  _toolBarSep = new OXHorizontal3dLine(this);

  _toolBar = new OXToolBar(this);
  _toolBar->AddButtons(tb_data);

  AddFrame(_toolBarSep, new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X));
  AddFrame(_toolBar, new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 
                                      0, 0, 3, 3));

  //---- list view

  _listView = new OXListView(this, 10, 10, 1);

  _listView->AddColumn(new OString("Station name"), 0, TEXT_LEFT);
  _listView->AddColumn(new OString("Frequency"), 1);
  _listView->AddColumn(new OString("Mode"), 2);
  _listView->AddColumn(new OString("Bandwidth"), 3);
  _listView->AddColumn(new OString("AGC"), 4);
  _listView->AddColumn(new OString("Tuning step"), 5);
  _listView->AddColumn(new OString("PBT offset"), 6);
  _listView->AddColumn(new OString("Location"), 7, TEXT_LEFT);
  _listView->AddColumn(new OString("Language"), 8, TEXT_LEFT);
  _listView->AddColumn(new OString("Start time"), 9);
  _listView->AddColumn(new OString("End time"), 10);
  _listView->AddColumn(new OString("Notes"), 11, TEXT_LEFT);
//  _listView->AddColumn(new OString("Offset"), 12);
//  _listView->AddColumn(new OString("Lockout"), 13);
  _listView->AddColumn(new OString(""), 14);  // end dummy

  OLayoutHints *layout = new OLayoutHints(LHINTS_EXPAND_ALL);

  AddFrame(_listView, layout);
  _listView->SetViewMode(LV_DETAILS, True, True);
  _listView->Associate(this);

  //------ status bar

  _statusBar = new OXStatusBar(this);
  AddFrame(_statusBar, new OLayoutHints(LHINTS_BOTTOM | LHINTS_EXPAND_X,
                                        0, 0, 3, 0));

  _filename = NULL;
  _path = NULL;
  SetChanged(False);

  SetWindowTitle(NULL);
  SetClassHints("XCLASS", "RX320");

  MapSubwindows();
  Resize(560, 320);
  Layout();
  MapWindow();
}

OXFreqDB::~OXFreqDB() {
  ClearFreqList();

  delete _menuBarLayout;
  delete _menuBarItemLayout;

  delete _menuFile;
  delete _menuEdit;
  delete _menuView;
  delete _menuHelp;

  if (_filename) delete[] _filename;
  if (_path) delete[] _path;
}

int OXFreqDB::CloseWindow() {
  if (_exiting) {
    XBell(GetDisplay(), 0);
    return False;
  }
  _exiting = True;
  switch (IsSaved()) {
  case ID_CLOSE:
  case ID_CANCEL:
    break;
  case ID_YES:
    DoSave(_filename);
    if (_changed) break;
  case ID_NO:
    _rxmain->RemoveFreqDBwindow(this);
    return OXMainFrame::CloseWindow();
  }
  _exiting = False;
  return False;
}

OXPopupMenu *OXFreqDB::_MakePopup(struct _popup *p) {

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

int OXFreqDB::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;
  OItemViewMessage *vmsg = (OItemViewMessage *) msg;

  switch (msg->type) {

    case MSG_BUTTON:
    case MSG_MENU:
      switch (msg->action) {

        case MSG_CLICK:
          switch (wmsg->id) {

            //--------------------------------------- File

            //case M_FILE_NEW:
            //  switch (IsSaved()) {
            //    case ID_CLOSE:
            //    case ID_CANCEL:
            //      break;
            //    case ID_YES:
            //      DoSave(_filename);
            //      if (_changed) break;
            //    case ID_NO:
            //      //ClearFreqList();  // done in open anyway
            //  }
            //  break;

            case M_FILE_OPEN:
              switch (IsSaved()) {
                case ID_CLOSE:
                case ID_CANCEL:
                  break;
                case ID_YES:
                  DoSave(_filename);
                  if (_changed) break;
                case ID_NO:
                  DoOpen();
                  break;
              }
              break;

            case M_FILE_SAVE:
              DoSave(_filename);
              break;

            case M_FILE_SAVEAS:
              DoSave(NULL);
              break;

            case M_FILE_PRINT:
              DoPrint();
              break;

            case M_FILE_EXIT:
              CloseWindow();
              break;

            //--------------------------------------- Edit

            case M_EDIT_CHANGE:
              if (_listView->NumSelected() == 1) {
                OFDBitem *f;
                std::vector<OItem *> items;
                OFreqRecord *frec;
                int retc;

                items = _listView->GetSelectedItems();
                f = (OFDBitem *) items[0];

                frec = new OFreqRecord(f->GetFreqRecord());
                new OXEditStation(_client->GetRoot(), this, frec, &retc);
                if (retc == ID_OK) {
                  f->ChangeFreqRecord(frec);
                  //_listView->Layout();
                  SetChanged(True);
                } else {
                  delete frec;
                }
              }
              break;

            case M_EDIT_ADD:
              {
                int retc;
                OFreqRecord *frec = new OFreqRecord("");
                Svfo vfo = _rxmain->GetVFOA();

                frec->freq = (double) vfo.freq / 1000000.0;
                frec->tuning_step = vfo.step;
                frec->mode = vfo.mode;
                frec->agc = vfo.agc;
                frec->filter_bw = vfo.filter;
                frec->pbt_offset = vfo.pbt;

                new OXEditStation(_client->GetRoot(), this, frec, &retc, True);
                if (retc == ID_OK) {
                  AddStation(frec);
                  _listView->Layout();
                  SetChanged(True);
                } else {
                  delete frec;
                }
              }
              break;

            case M_EDIT_TUNE:
              if (_listView->NumSelected() == 1) {
                const OFDBitem *f;
                std::vector<OItem *> items;

                items = _listView->GetSelectedItems();
                f = (OFDBitem *) items[0];

                _rxmain->TuneTo(f->GetFreqRecord());
              }
              break;

            case M_EDIT_DELETE:
              _listView->DeleteSelection();
              SetChanged(True);
              break;

            //--------------------------------------- View

            case M_VIEW_TOOLBAR:
              DoToggleToolBar();
              break;

            case M_VIEW_STATUSBAR:
              DoToggleStatusBar();
              break;

            //--------------------------------------- Help

            case M_HELP_CONTENTS:
              _rxmain->ShowHelp();
              break;

            case M_HELP_SEARCH:
              break;

            case M_HELP_ABOUT:
              DoAbout();
              break;

            default:
              break;

          }

        default:
          break;

      }
      break;

    case MSG_LISTVIEW:
      switch (msg->action) {
        case MSG_DBLCLICK:
          if (vmsg->button == Button1) {
            if (_listView->NumSelected() == 1) {
              const OFDBitem *f;
              std::vector<OItem *> items;

              items = _listView->GetSelectedItems();
              f = (OFDBitem *) items[0];

              _rxmain->TuneTo(f->GetFreqRecord());
            }
          }
          break;

        case MSG_SELCHANGED:
          if (_listView->NumSelected() == 1) {
            _menuEdit->EnableEntry(M_EDIT_TUNE);
            _menuEdit->EnableEntry(M_EDIT_CHANGE);
          } else {
            _menuEdit->DisableEntry(M_EDIT_TUNE);
            _menuEdit->DisableEntry(M_EDIT_CHANGE);
          }
          if (_listView->NumSelected() > 0) {
            _menuEdit->EnableEntry(M_EDIT_DELETE);
          } else {
            _menuEdit->DisableEntry(M_EDIT_DELETE);
          }
          break;
      }
      break;

    default:
      break;

  }

  return True;
}

//----------------------------------------------------------------------

void OXFreqDB::SetWindowTitle(char *title) {
  static char *pname = "RX320 Frequency database";

  if (title) {
    char *wname = new char[strlen(title) + strlen(pname) + 10];
    sprintf(wname, "%s - %s%s", pname, title, _changed ? " *" : "");
    SetWindowName(wname);
    delete[] wname;
  } else {
    SetWindowName(pname);
  }
}

void OXFreqDB::UpdateStatus() {
  char tmp[256];

  int nrec = _listView->GetNumberOfItems();
  if (nrec == 0)
    strcpy(tmp, "Ready");
  else
    sprintf(tmp, "%d records.", nrec);

  _statusBar->SetText(0, new OString(tmp));
}

void OXFreqDB::DoToggleToolBar() {
  if (_toolBar->IsVisible()) {
    HideFrame(_toolBar);
    HideFrame(_toolBarSep);
    _menuView->UnCheckEntry(M_VIEW_TOOLBAR);
  } else {
    ShowFrame(_toolBar);
    ShowFrame(_toolBarSep);
    _menuView->CheckEntry(M_VIEW_TOOLBAR);
  }
}

void OXFreqDB::DoToggleStatusBar() {
  if (_statusBar->IsVisible()) {
    HideFrame(_statusBar);
    _menuView->UnCheckEntry(M_VIEW_STATUSBAR);
  } else {
    ShowFrame(_statusBar);
    _menuView->CheckEntry(M_VIEW_STATUSBAR);
  }
}

void OXFreqDB::DoAbout() {
  _rxmain->ShowAbout(this);
}


//----------------------------------------------------------------------

void OXFreqDB::DoOpen() {
  OFileInfo fi;

  fi.file_types = filetypes;
  fi.ini_dir = _path ? StrDup(_path) : NULL;
  new OXFileDialog(_client->GetRoot(), this, FDLG_OPEN, &fi);
  if (fi.filename) {
    ReadFile(fi.filename);
    if (_filename) delete[] _filename;
    _filename = StrDup(fi.filename);
    if (_path) delete[] _path;
    _path = StrDup(fi.ini_dir);
    SetWindowTitle(_filename);
    UpdateStatus();
    SetChanged(False);
  }
}

void OXFreqDB::DoSave(char *fname) {
  int retc, newfile = False;
  OFileInfo fi;

  if (_path) chdir(_path);

  if (!fname) {
    fi.file_types = filetypes;
    new OXFileDialog(_client->GetRoot(), this, FDLG_SAVE, &fi);
    fname = fi.filename;
    newfile = True;
  }

  if (fname) {

    // check whether the file already exists, and ask
    // permission to overwrite if so.

    if (newfile && (access(fname, F_OK) == 0)) {
      new OXMsgBox(_client->GetRoot(), this,
            new OString("Save"),
            new OString("A file with the same name already exists. Overwrite?"),
            MB_ICONQUESTION, ID_YES | ID_NO, &retc);
      if (retc == ID_NO) return;
    }

    WriteFile(fname);
    if (fname != _filename) {
      if (_filename) delete[] _filename;
      _filename = StrDup(fname);
      SetWindowTitle(_filename);
    }
    SetChanged(False);
  }
}

void OXFreqDB::DoPrint() {
}

void OXFreqDB::SetChanged(int onoff) {
  if (onoff) {
    if (_filename) _menuFile->EnableEntry(M_FILE_SAVE);
    _menuFile->EnableEntry(M_FILE_SAVEAS);
    _changed = True;
  } else {
    _menuFile->DisableEntry(M_FILE_SAVE);
    if (_listView->GetNumberOfItems() == 0)
      _menuFile->DisableEntry(M_FILE_SAVEAS);
    else
      _menuFile->EnableEntry(M_FILE_SAVEAS);
    _changed = False;
  }
  SetWindowTitle(_filename);
}

int OXFreqDB::IsSaved() {
  int ret;
  char tmp[2048];

  if (!_changed) {
    return ID_NO;
  } else {
    if (_filename) {
      sprintf(tmp, "The database \"%s\" has been modified.\n"
                   "Do you want to save the changes?", _filename);
    } else {
      sprintf(tmp, "The database has been modified.\n"
                   "Do you want to save the changes?");
    }
    MapRaised();
    new OXMsgBox(_client->GetRoot(), this, new OString("RX320"),
                 new OString(tmp), MB_ICONEXCLAMATION,
                 ID_YES | ID_NO | ID_CANCEL, &ret);
    return ret;
  }
}

void OXFreqDB::ClearFreqList() {
  _listView->Clear();
}

void OXFreqDB::ReadFile(char *fname) {
  FILE *f;
  char *p, str[256];
  int format;

  f = fopen(fname, "r");

  if (!f) {
    char tmp[PATH_MAX];
    sprintf(tmp, "Cannot open file \"%s\": %s.",
                 fname, strerror(errno));
    new OXMsgBox(_client->GetRoot(), this, new OString("RX320"),
                 new OString(tmp), MB_ICONSTOP, ID_OK);

    return;
  }

  ClearFreqList();  // unless we want to merge databases

  p = strrchr(fname, '.');
  if (p && (strcmp(p, ".cdf") == 0))
    format = FORMAT_TENTEC_CDF;
  else
    format = FORMAT_CLIFTON_320;

  while (1) {
    if (!fgets(str, 256, f)) break;
    p = strchr(str, '\n');
    if (p) *p = '\0';
    p = strchr(str, '\r');
    if (p) *p = '\0';
    AddStation(new OFreqRecord(str, format));
  }

  fclose(f);

  _listView->Layout();
}

void OXFreqDB::WriteFile(char *fname) {
  FILE *f;

  // should check whether the file already exists!

  f = fopen(fname, "w");
  if (!f) {
    char tmp[PATH_MAX];
    sprintf(tmp, "Cannot create file \"%s\": %s.",
                 fname, strerror(errno));
    new OXMsgBox(_client->GetRoot(), this, new OString("RX320"),
                 new OString(tmp), MB_ICONSTOP, ID_OK);

    return;
  }

  for (int i = 0; i < _listView->GetNumberOfItems(); ++i) {
    OFDBitem *fi = (OFDBitem *) _listView->GetItem(i);
    fprintf(f, "%s\n", fi->GetFreqRecord()->RecordString());
  }

  fclose(f);
}

void OXFreqDB::AddStation(OFreqRecord *frec, int recno) {
  OFDBitem *item;
  std::vector<OString *> dummy;

  dummy.clear();
  dummy.push_back(new OString(frec->name));
  item = new OFDBitem(_listView, recno, frec, dummy);
  _listView->AddItem(item);
}
