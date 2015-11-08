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

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <time.h>
#include <grp.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <xclass/OMimeTypes.h>
#include <xclass/OXClient.h>
#include <xclass/OXWindow.h>
#include <xclass/OXTransientFrame.h>
#include <xclass/OXButton.h>
#include <xclass/OXTab.h>
#include <xclass/OXMsgBox.h>
#include <xclass/OString.h>
#include <xclass/OPicture.h>
#include <xclass/utils.h>

#include "OXOptions.h"
#include "OXEditMime.h"
#include "OXExplorer.h"

#include "icons/onewin.xpm"
#include "icons/multiwin.xpm"


extern OXExplorer *mainWindow;
extern int DispFullPath;
extern int NewBrowser;

extern struct _default_icon default_icon[];
extern OMimeTypes *MimeTypeList;


//-------------------------------------------------------------------

OXOptionsDialog::OXOptionsDialog(const OXWindow *p, const OXWindow *main,
                                 unsigned long options) :
  OXTransientFrame(p, main, 400, 200, options) {
    OXCompositeFrame *tf;
    int ax, ay, width, height;
    Window wdummy;
    OHotString *str;
    OXHorizontalFrame *bframe, *hf1, *hf2, *hf3, *shf1, *shf2;
    OXVerticalFrame *lf, *bf, *f3;
    OXGroupFrame *gf1, *gf2;
    const OPicture *pic;

    //=============== setup elements:

    //======= [1] this frame will contain the "OK Cancel Apply" buttons:
    bframe = new OXHorizontalFrame(this, 60, 20, FIXED_WIDTH);

    //--- create the buttons
    Ok     = new OXTextButton(bframe, new OHotString("OK"), ID_OK);
    Cancel = new OXTextButton(bframe, new OHotString("Cancel"), ID_CANCEL);
    Apply  = new OXTextButton(bframe, new OHotString("&Apply"), ID_APPLY);

    //--- Apply is initially disabled, OK is default
    Apply->Disable();

    SetDefaultAcceptButton(Ok);
    SetDefaultCancelButton(Cancel);

    //--- send button messages to this dialog
    Ok->Associate(this);
    Cancel->Associate(this);
    Apply->Associate(this);

    //--- layout for buttons: top align, equally expand horizontally
    bly = new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 5, 0, 0, 0);

    //--- layout for the frame: place at bottom, right aligned
    bfly = new OLayoutHints(LHINTS_BOTTOM | LHINTS_RIGHT, 0, 5, 0, 4);

    bframe->AddFrame(Ok, bly);
    bframe->AddFrame(Cancel, bly);
    bframe->AddFrame(Apply, bly);

    width = Ok->GetDefaultWidth();
    width = max(width, Cancel->GetDefaultWidth());
    width = max(width, Apply->GetDefaultWidth());
    bframe->Resize((width + 20) * 3, bframe->GetDefaultHeight());

    AddFrame(bframe, bfly);

    //====== [2] Tab widget

    OXTab *tab = new OXTab(this, 320, 350);
    Ltab = new OLayoutHints(LHINTS_EXPAND_Y | LHINTS_EXPAND_X, 5, 5, 5, 5);

    if (mainWindow->GetMainMode() == FILE_MGR_MODE) {

      //------ [2a] Tab 1: Folder

      tf = tab->AddTab(new OString("Folder"));

      //--- Tab 1 contents: a single group frame,
      //                    2 radio buttons, 2 labels and 2 icons

      //--- the group frame and its layout
      gf1 = new OXGroupFrame(tf, new OString("Browsing options"));
      Gly = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y,
                             10, 10, 10, 10);

      str  = new OHotString("Browse folders using a &separate window\n"
                            "for each folder.");
      rad1 = new OXRadioButton(gf1, str, 81);

      str  = new OHotString("Browse folders by using a si&ngle window\n"
                            "that changes as you open each folder.");
      rad2 = new OXRadioButton(gf1, str, 82);

      //--- layout for radio buttons and frames hf1, hf2
      Lr = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y, 5, 0, 0, 0);

      //--- layout for labels
      Lly = new OLayoutHints(LHINTS_CENTER_Y | LHINTS_LEFT, 22, 0, 0, 0);

      //--- layout for icons
      Ily = new OLayoutHints(LHINTS_CENTER_Y | LHINTS_LEFT, 15, 0, 0, 0);

      gf1->AddFrame(rad1, Lr);

      //--- horizontal frame holding the first label-icon group
      gf1->AddFrame(hf1 = new OXHorizontalFrame(gf1, 10, 10), Lr);

      pic = _client->GetPicture("multiwin.xpm", multiwin_xpm);
      if (!pic)
        FatalError("Bad or missing pixmap: multiwin.xpm");

      hf1->AddFrame(new OXLabel(hf1, new OString("Example:")), Lly);
      hf1->AddFrame(new OXIcon(hf1, pic, 40, 40), Ily);

      gf1->AddFrame(rad2, Lr);

      //--- horizontal frame holding the second label-icon group
      gf1->AddFrame(hf2 = new OXHorizontalFrame(gf1, 10, 10), Lr);

      pic = _client->GetPicture("onewin.xpm", onewin_xpm);
      if (!pic)
        FatalError("Bad or missing pixmap: onewin.xpm");

      hf2->AddFrame(new OXLabel(hf2, new OString("Example:")), Lly);
      hf2->AddFrame(new OXIcon(hf2, pic, 40, 40), Ily);

      tf->AddFrame(gf1, Gly);

      rad1->Associate(this);
      rad2->Associate(this);

      if (NewBrowser)
        rad1->SetState(BUTTON_DOWN);
      else
        rad2->SetState(BUTTON_DOWN);

      _NewBrowser = NewBrowser;
    }

    //------ [2b] Tab 2: View

    tf = tab->AddTab(new OString("View"));

    //--- Tab 2 contents: a group frame with 2 radio buttons and list box,
    //                    and a check button

    gf2 = new OXGroupFrame(tf, new OString("Files"));

    chk1 = new OXCheckButton(gf2, new OHotString("&Show hidden files and folders"), 83);
    chk2 = new OXCheckButton(gf2, new OHotString("Always place &folders first"), 84);

    chk1->Associate(this);
    chk2->Associate(this);
    chk1->SetState(BUTTON_DOWN);

    L3 = new OLayoutHints(LHINTS_LEFT | LHINTS_TOP, 0, 0, 5, 0);
    L4 = new OLayoutHints(LHINTS_LEFT | LHINTS_TOP, 5, 0, 0, 0);

    f3 = new OXVerticalFrame(tf, 60, 20);
    gf2->AddFrame(chk1, L3);
    gf2->AddFrame(chk2, L3);

    chk3 = new OXCheckButton(f3,
               new OHotString("Display the full path in the title bar"), 85);
    chk3->Associate(this);
    chk3->SetState(DispFullPath ? BUTTON_DOWN : BUTTON_UP);
    _DispFullPath = DispFullPath;

    f3->AddFrame(chk3, L4);

    L6 = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_TOP, 10, 10, 10, 0);
    tf->AddFrame(gf2, L6);
    tf->AddFrame(f3, L6);


    //------ [2c] Tab 3: File Types

    tf = tab->AddTab(new OString("File types"));

    lbly = new OLayoutHints(LHINTS_LEFT | LHINTS_TOP, 10, 10, 10, 0);
    tf->AddFrame(new OXLabel(tf, new OString("Registered file types:")), lbly);

    hf3 = new OXHorizontalFrame(tf, 10, 10);
    lfh = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y, 10, 10, 0, 10);

    gfly = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y);
    lgf3 = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_BOTTOM, 0, 0, 2, 0);
    lf = new OXVerticalFrame(hf3, 10, 10);
    lf->AddFrame(lbf = new OXListBox(lf, 801), gfly);
    lf->AddFrame(gf3 = new OXGroupFrame(lf,
                       new OString("File type details")), lgf3);

    shf1 = new OXHorizontalFrame(gf3, 10, 10, FIXED_HEIGHT);
    shf2 = new OXHorizontalFrame(gf3, 10, 10, FIXED_HEIGHT);
    lshf = new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 0, 0, 5, 0);

    lfd = new OLayoutHints(LHINTS_LEFT | LHINTS_EXPAND_Y, 0, 5, 2, 2);
    lex1 = new OLayoutHints(LHINTS_LEFT | LHINTS_EXPAND_Y, 10, 5, 2, 2);
    icf = new OXIcon(shf1, default_icon[ICON_APP].icon[ICON_BIG], 32, 32);
    lblex1 = new OXLabel(shf1, new OString("Extension:"));
    lblex2 = new OXLabel(shf1, new OString(""));
    shf1->AddFrame(icf, lfd);
    shf1->AddFrame(lblex1, lex1);
    shf1->AddFrame(lblex2, lfd);

    lblop1 = new OXLabel(shf2, new OString("Opens with:"));
    lblop2 = new OXLabel(shf2, new OString(""));
    shf2->AddFrame(lblop1, lfd);
    shf2->AddFrame(lblop2, lfd);

    shf1->Resize(shf1->GetDefaultWidth(), 38);
    shf2->Resize(shf1->GetDefaultWidth(), lblop1->GetDefaultHeight()+6);

    gf3->AddFrame(shf1, lshf);
    gf3->AddFrame(shf2, lshf);

    hf3->AddFrame(lf, gfly);

    UpdateListBox();

    lbf->Select(500);
    lbf->Associate(this);

    bf = new OXVerticalFrame(hf3, 10, 10, FIXED_WIDTH);

    nbly = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_TOP, 0, 0, 0, 5);
    nbfly = new OLayoutHints(LHINTS_LEFT | LHINTS_RIGHT, 10, 0, 0, 0);

    New    = new OXTextButton(bf, new OHotString("&New type..."), 801);
    Remove = new OXTextButton(bf, new OHotString("&Remove"), 802);
    Edit   = new OXTextButton(bf, new OHotString("&Edit..."), 803);

    New->Associate(this);
    Remove->Associate(this);
    Edit->Associate(this);

    Remove->Disable();
    Edit->Disable();

    bf->AddFrame(New, nbly);
    bf->AddFrame(Remove, nbly);
    bf->AddFrame(Edit, nbly);

    width = New->GetDefaultWidth();
    width = max(width, Remove->GetDefaultWidth());
    width = max(width, Edit->GetDefaultWidth());
    bf->Resize(width + 10, bf->GetDefaultHeight());

    hf3->AddFrame(bf, nbfly);

    tf->AddFrame(hf3, lfh);

    //-----------------------------

    AddFrame(tab, Ltab);

    MapSubwindows();

    width  = 330; // GetDefaultWidth();
    height = 386; // GetDefaultHeight();
    Resize(width, height);

    //---- position relative to the parent's window (the default was root 0,0!)
 
    XTranslateCoordinates(GetDisplay(),
                          main->GetId(), GetParent()->GetId(),
                          50, 50, &ax, &ay, &wdummy);

    Move(ax, ay);

    //---- make dialog non-resizable

    SetWMSize(width, height);
    SetWMSizeHints(width, height, width, height, 0, 0);

    SetWindowName("Options");
    SetIconName("Options");
    SetClassHints("Explorer", "Explorer");

    SetMWMHints(MWM_DECOR_ALL | MWM_DECOR_RESIZEH | MWM_DECOR_MAXIMIZE |
                                MWM_DECOR_MINIMIZE | MWM_DECOR_MENU,
                MWM_FUNC_ALL | MWM_FUNC_RESIZE | MWM_FUNC_MAXIMIZE | 
                               MWM_FUNC_MINIMIZE,
                MWM_INPUT_MODELESS);

    MapWindow();
    _client->WaitFor(this);
}

OXOptionsDialog::~OXOptionsDialog() {

  // Need to delete OLayoutHints objects only, OXCompositeFrame 
  // takes care of the rest...

  delete bly; delete bfly;

  if (mainWindow->GetMainMode() == FILE_MGR_MODE) {
    delete Lr;  delete Lly; 
    delete Ily; delete Gly;
  }

  delete L3; delete L4; delete L6;

  delete lshf; delete lfd; delete lex1;
  delete lgf3;
  delete lfh; 
  delete lbly; delete gfly;
  delete nbly; delete nbfly;

  delete Ltab;
}

void OXOptionsDialog::UpdateListBox() {
  OMime *MimeRunner;
  int i;

  lbf->RemoveAllEntries();

  lbf->AddEntry(new OXTreeLBEntry(lbf->GetContainer(),
                      new OString("Application"),
                      default_icon[ICON_APP].icon[ICON_SMALL], 500, NULL),
                new OLayoutHints(LHINTS_LEFT | LHINTS_TOP,
                      3, 0, 0, 0));
  lbf->AddEntry(new OXTreeLBEntry(lbf->GetContainer(),
                      new OString("File folder"),
                      default_icon[ICON_FOLDER].icon[ICON_SMALL], 501, NULL),
                new OLayoutHints(LHINTS_LEFT | LHINTS_TOP,
                      3, 0, 0, 0));

  MimeRunner = NULL;
  i = 0;

  while ((MimeRunner = MimeTypeList->NextMime(MimeRunner))) {
    const OPicture *pic;
    char  fname[50];
    OString *descr;

    pic = MimeTypeList->GetIcon(MimeRunner, True);
    if (!pic) pic = default_icon[ICON_DOC].icon[ICON_SMALL];

    if (MimeRunner->description) {
      descr = new OString(MimeRunner->description);
    } else {
      char default_descr[1000];
      // generate default description
      sprintf(default_descr, "Files matching %s", MimeRunner->pattern);
      descr = new OString(default_descr);
    }

    // Fake string value /********/
    sprintf(fname, "%ld", (long) MimeRunner);

    lbf->AddEntry(new OXTreeLBEntry(lbf->GetContainer(),
                        // new OString(ftypes[i].ft), 
                        descr, pic, 503+i, 
                        new OString(fname)),
                  new OLayoutHints(LHINTS_LEFT | LHINTS_TOP,
                        3, 0, 0, 0));
    i++;
  }
}

int OXOptionsDialog::ProcessMessage(OMessage *msg) {
  const OPicture *pic;
  OXTreeLBEntry *e;
  int retc;
  char *p, fname[256], action[256];
  OWidgetMessage *wmsg;
  OListBoxMessage *lbmsg;

  switch (msg->action) {
    case MSG_CLICK:

      wmsg = (OWidgetMessage *) msg;
      switch (msg->type) {
        case MSG_BUTTON:
          switch (wmsg->id) {
            case ID_OK:
              NewBrowser = _NewBrowser;
              if (DispFullPath != _DispFullPath) {
                DispFullPath = _DispFullPath;
                mainWindow->SetTitle();
              }
            case ID_CANCEL:
              CloseWindow();
              break;

            case ID_APPLY:
              NewBrowser = _NewBrowser;
              if (DispFullPath != _DispFullPath) {
                DispFullPath = _DispFullPath;
                mainWindow->SetTitle();
              }
              Apply->Disable();
              break;

            case 801:
              new OXEditMimeDialog(_client->GetRoot(), this, NULL, &retc);
              if (retc == ID_OK) {
                MimeTypeList->SaveMimes();
                UpdateListBox();
              }
              break;

            case 802:
              break;

            case 803:
              e = (OXTreeLBEntry *) lbf->GetSelectedEntry();
              if (e && (e->ID() > 501)) {
                const char *str;
                OMime *mime;

                if (e->GetPath()) {
                  str = e->GetPath()->GetString();
                  sscanf(str, "%ld", (long *)&(mime)); /********/
                  new OXEditMimeDialog(_client->GetRoot(), this, mime, &retc);
                  if (retc == ID_OK) {
                    MimeTypeList->SaveMimes();
                    UpdateListBox();
                  }
                }
              }
              break;

          }
          break;

        case MSG_CHECKBUTTON:
        case MSG_RADIOBUTTON:
          switch (wmsg->id) {

            // process the radio buttons
            case 81:
              rad2->SetState(BUTTON_UP);
              _NewBrowser = True;
              break;

            case 82:
              rad1->SetState(BUTTON_UP);
              _NewBrowser = False;
              break;

            case 85:
              _DispFullPath = (chk3->GetState() == BUTTON_DOWN);
              break;

          }
          Apply->Enable();
          break;

        case MSG_LISTBOX:
          lbmsg = (OListBoxMessage *) msg;
          if (lbmsg->id == 801) {
            if (lbmsg->entryID == 500) {

              icf->SetPicture(default_icon[ICON_APP].icon[ICON_FOLDER]);
              lblex2->SetText(new OString(""));
              lblop2->SetText(new OString(""));
              Remove->Disable();
              Edit->Disable();

            } else if (lbmsg->entryID == 501) {

              icf->SetPicture(default_icon[ICON_FOLDER].icon[ICON_FOLDER]);
              lblex2->SetText(new OString(""));
              lblop2->SetText(new OString("explorer"));
              Remove->Disable();
              Edit->Disable();

            } else {

              char *str;
              OMime *mime;
  
              e = (OXTreeLBEntry *) lbf->GetSelectedEntry();
              if (e->GetPath()) {
                str = (char *) e->GetPath()->GetString();
                sscanf(str, "%ld", (long *)&(mime)); /********/
                pic = MimeTypeList->GetIcon(mime, False);
                str = StrDup(mime->action);
                if ((p = strchr(str, ' ')) != NULL) *p = '\0';
                if (!pic) pic = default_icon[ICON_DOC].icon[ICON_BIG];
                icf->SetPicture(pic);
                lblex2->SetText(new OString(mime->pattern));
                lblop2->SetText(new OString(str));
                delete[] str;
              } else {
                lblex2->SetText(new OString(""));
                lblop2->SetText(new OString(""));
              }
              Remove->Enable();
              Edit->Enable();

            }
          gf3->Layout();
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
