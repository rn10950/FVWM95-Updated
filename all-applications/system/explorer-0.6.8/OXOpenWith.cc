/**************************************************************************

    This file is part of explorer95, a file manager for fvwm95.
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

#include <vector>
#include <algorithm>
#include <functional>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <xclass/OMimeTypes.h>
#include <xclass/OXClient.h>
#include <xclass/OXTransientFrame.h>
#include <xclass/OXCheckButton.h>
#include <xclass/OXMsgBox.h>
#include <xclass/OXFileDialog.h>
#include <xclass/utils.h>

#include "OXOpenWith.h"
#include "OXExplorer.h"


extern struct _default_icon default_icon[];
extern OMimeTypes *MimeTypeList;

#ifndef ID_OTHER
#define ID_OTHER (ID_OK+ID_CANCEL+5)
#endif


static char *filetypes[] = {
  "All files", "*",
  NULL, NULL
};

//-------------------------------------------------------------------

OXOpenWithDialog::OXOpenWithDialog(const OXWindow *p, const OXWindow *main,
                                   OString *docname, OString *progname,
                                   unsigned long options) :
  OXTransientFrame(p, main, 400, 200, options) {
    int ax, ay, width, height;
    Window wdummy;
    OXHorizontalFrame *bframe;

    _progname = progname;
    if (_progname) _progname->Clear();

    bframe = new OXHorizontalFrame(this, 60, 20, FIXED_WIDTH);

    Ok     = new OXTextButton(bframe, new OHotString("OK"), ID_OK);
    Cancel = new OXTextButton(bframe, new OHotString("Cancel"), ID_CANCEL);
    Other  = new OXTextButton(bframe, new OHotString("O&ther..."), ID_OTHER);

    Ok->Disable();

    SetDefaultAcceptButton(Ok);
    SetDefaultCancelButton(Cancel);

    Ok->Associate(this);
    Cancel->Associate(this);
    Other->Associate(this);

    bly = new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 5, 0, 0, 0);
    bfly = new OLayoutHints(LHINTS_BOTTOM | LHINTS_RIGHT, 0, 5, 0, 4);

    bframe->AddFrame(Ok, bly);
    bframe->AddFrame(Cancel, bly);
    bframe->AddFrame(Other, bly);

    width = Ok->GetDefaultWidth();
    width = max(width, Cancel->GetDefaultWidth());
    width = max(width, Other->GetDefaultWidth());
    bframe->Resize((width + 20) * 3, bframe->GetDefaultHeight());

    AddFrame(bframe, bfly);

    char tmp[1024];
    sprintf(tmp, "Click the program you want to use to open the file\n"
                 "\"%s\".\n\n"
                 "If the program you want is not in the list, click Other.",
                 docname->GetString());

    OXLabel *lbl1, *lbl2;

    lbl1 = new OXLabel(this, new OString(tmp));
    lbl2 = new OXLabel(this, new OHotString("&Choose the program you want to use:"));

    ly1 = new OLayoutHints(LHINTS_TOP | LHINTS_LEFT, 5, 5, 10, 10);
    AddFrame(lbl1, ly1);

    ly2 = new OLayoutHints(LHINTS_TOP | LHINTS_LEFT, 5, 5, 2, 2);
    AddFrame(lbl2, ly2);

    lbf = new OXListBox(this, 801);
//    ly3 = new OLayoutHints(LHINTS_EXPAND_Y | LHINTS_EXPAND_X, 5, 5, 0, 15);
    ly3 = new OLayoutHints(LHINTS_EXPAND_Y | LHINTS_EXPAND_X, 5, 5, 0, 0);
    AddFrame(lbf, ly3);

    UpdateListBox();

//    lbf->Select(0);
    lbf->Associate(this);

    SetFocusOwner(lbf);

    OXCheckButton *cb = new OXCheckButton(this,
      new OHotString("Always &use this program to open this type of file"), 101);
//    cb->Disable();
    OLayoutHints *ly4;
    ly4 = new OLayoutHints(LHINTS_TOP | LHINTS_LEFT, 5, 5, 0, 15);
    AddFrame(cb, ly4);

    MapSubwindows();

    width  = 320; // GetDefaultWidth();
    height = 320; // GetDefaultHeight();
    Resize(width, height);

    //---- position relative to the parent's window (the default was root 0,0!)
 
    XTranslateCoordinates(GetDisplay(),
                          main->GetId(), GetParent()->GetId(),
                          50, 50, &ax, &ay, &wdummy);

    Move(ax, ay);

    //---- make dialog non-resizable

    SetWMSize(width, height);
    SetWMSizeHints(width, height, width, height, 0, 0);

    SetWindowName("Open With");
    SetIconName("Open With");
    SetClassHints("Explorer", "Explorer");

    SetMWMHints(MWM_DECOR_ALL | MWM_DECOR_RESIZEH | MWM_DECOR_MAXIMIZE |
                                MWM_DECOR_MINIMIZE | MWM_DECOR_MENU,
                MWM_FUNC_ALL | MWM_FUNC_RESIZE | MWM_FUNC_MAXIMIZE | 
                               MWM_FUNC_MINIMIZE,
                MWM_INPUT_MODELESS);

    MapWindow();
    _client->WaitFor(this);
}

OXOpenWithDialog::~OXOpenWithDialog() {
  delete bly;
  delete bfly;
  delete ly1;
  delete ly2;
  delete ly3;
}

struct LBSortAscending : public std::binary_function<OString*, OString*, bool> {
public:
  bool operator()(const OString* item1, const OString* item2) const {
    return (strcmp(item1->GetString(), item2->GetString()) < 0);
  }
};

void OXOpenWithDialog::UpdateListBox() {
  OMime *MimeRunner;
  std::vector<OString *> progs;

#if 0
  lbf->RemoveAllEntries();

  MimeRunner = NULL;
  int i = 0;
  while ((MimeRunner = MimeTypeList->NextMime(MimeRunner))) {
    const OPicture *pic;
    char  fname[50];
    OString *prog;

    pic = MimeTypeList->GetIcon(MimeRunner, True);
    if (!pic) pic = default_icon[ICON_DOC].icon[ICON_SMALL];

    if (MimeRunner->action) {
      char *p, *str = StrDup(MimeRunner->action);
      if ((p = strchr(str, ' ')) != NULL) *p = '\0';
      prog = new OString(str);
      delete[] str;

      // Fake string value /********/
      sprintf(fname, "%ld", (long) MimeRunner);

      lbf->AddEntry(new OXTreeLBEntry(lbf->GetContainer(),
                        // new OString(ftypes[i].ft), 
                        prog, pic, i,
                        new OString(fname)),
                    new OLayoutHints(LHINTS_LEFT | LHINTS_TOP,
                        3, 0, 0, 0));
      i++;
    }
  }
#else
  MimeRunner = NULL;
  progs.clear();
  while ((MimeRunner = MimeTypeList->NextMime(MimeRunner))) {
    OString *prog;

    if (MimeRunner->action) {
      char *p, *str = StrDup(MimeRunner->action);
      if ((p = strchr(str, ' ')) != NULL) *p = '\0';
      bool found = false;
      for (int i = 0; i < progs.size(); ++i) {
        if (strcmp(progs[i]->GetString(), str) == 0) {
          found = true;
          break;
        }
      }
      if (!found) progs.push_back(new OString(str));
      delete[] str;
    }
  }

  std::stable_sort(progs.begin(), progs.end(), LBSortAscending());

  const OPicture *pic;
  int i;

  lbf->RemoveAllEntries();
  for (i = 0; i < progs.size(); ++i) {

    pic = GetFilePic(progs[i]->GetString());

    lbf->AddEntry(new OXTreeLBEntry(lbf->GetContainer(),
                      progs[i], pic, i, new OString("")),
                  new OLayoutHints(LHINTS_LEFT | LHINTS_TOP, 3, 0, 0, 0));
  }

  _maxEntry = i;
#endif
}

const OPicture *OXOpenWithDialog::GetFilePic(const char *name) {
  char tmp[1024];
  const OPicture *pic;

  sprintf(tmp, "%s.t.xpm", name);
  pic = _client->GetPicture(tmp, 16, 16);
  if (!pic) {
    sprintf(tmp, "%s.s.xpm", name);
    pic = _client->GetPicture(tmp, 16, 16);
  }
  if (!pic) {
    sprintf(tmp, "%s.xpm", name);
    pic = _client->GetPicture(tmp, 16, 16);
  }
  if (!pic) pic = default_icon[ICON_APP].icon[ICON_SMALL];

  return pic;
}

void OXOpenWithDialog::AddOther() {
  OFileInfo fi;

  fi.filename = NULL;
  fi.file_types = filetypes;
  fi.MimeTypesList = NULL;

  new OXFileDialog(_client->GetRoot(), this, FDLG_OPEN, &fi);

  if (fi.filename) {

    // TODO:
    // - stat fi.filename
    // - check that it is a regular file and has exec permissions
    // - access(fi.filename, X_OK) ?

    const OPicture *pic = GetFilePic(fi.filename);

    lbf->AddEntry(new OXTreeLBEntry(lbf->GetContainer(),
                      new OString(fi.filename), pic, _maxEntry,
                      new OString("")),
                  new OLayoutHints(LHINTS_LEFT | LHINTS_TOP,
                      3, 0, 0, 0));

    lbf->Select(_maxEntry);
    ++_maxEntry;

    Ok->Enable();
    SetFocusOwner(lbf);
  }
}

int OXOpenWithDialog::ProcessMessage(OMessage *msg) {
  OXTreeLBEntry *e;
  OWidgetMessage *wmsg;
  OListBoxMessage *lbmsg;

  switch (msg->action) {
    case MSG_CLICK:

      wmsg = (OWidgetMessage *) msg;
      switch (msg->type) {
        case MSG_BUTTON:
          switch (wmsg->id) {
            case ID_OK:
              if (_progname) {
                e = (OXTreeLBEntry *) lbf->GetSelectedEntry();
                if (e) _progname->Append(e->GetText());
              }
            case ID_CANCEL:
              CloseWindow();
              break;

            case ID_OTHER:
              AddOther();
              break;

            default:
              break;

          }
          break;

        case MSG_LISTBOX:
          if (msg->action == MSG_CLICK) Ok->Enable();
          lbmsg = (OListBoxMessage *) msg;
          if (lbmsg->id == 801) {
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
