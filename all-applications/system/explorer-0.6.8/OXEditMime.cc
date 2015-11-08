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

#include <xclass/OXTextButton.h>
#include <xclass/OXTextEntry.h>
#include <xclass/OX3dLines.h>
#include <xclass/OXMsgBox.h>
#include <xclass/OXIcon.h>
#include <xclass/OXLabel.h>
#include <xclass/utils.h>

#include "OXEditMime.h"
#include "OXIconSelDialog.h"
#include "OXExplorer.h"


extern struct _default_icon default_icon[];
extern OMimeTypes *MimeTypeList;


#define ID_CHANGEICON  (ID_OK+ID_CANCEL+100)


//-------------------------------------------------------------------

OXEditMimeDialog::OXEditMimeDialog(const OXWindow *p, const OXWindow *main,
                              OMime *mime, int *retc, unsigned long options) :
  OXTransientFrame(p, main, 400, 200, options) {
    int ax, ay, width;
    Window wdummy;
    const OPicture *pic;
    char name[1024], tmp[1024];

    _retc = retc;
    if (_retc) *_retc = ID_CANCEL;

    //--- create the OK / Cancel buttons and button frame:
    bframe = new OXHorizontalFrame(this, 60, 20, FIXED_WIDTH);

    Ok = new OXTextButton(bframe, new OHotString("OK"), ID_OK);
    Cancel = new OXTextButton(bframe, new OHotString("Cancel"), ID_CANCEL);

    Ok->Associate(this);
    Cancel->Associate(this);

    SetDefaultAcceptButton(Ok);
    SetDefaultCancelButton(Cancel);

    //--- layout for buttons: top align, equally expand horizontally
    bly = new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 5, 0, 0, 0);

    //--- layout for the frame: place at bottom, right-aligned
    bfly = new OLayoutHints(LHINTS_BOTTOM | LHINTS_RIGHT, 0, 5, 0, 4);

    bframe->AddFrame(Ok, bly);
    bframe->AddFrame(Cancel, bly);

    width = Ok->GetDefaultWidth();
    width = max(width, Cancel->GetDefaultWidth());
    bframe->Resize((width + 20) * 2, bframe->GetDefaultHeight());

    AddFrame(bframe, bfly);

    //--- icon frame
    Ihf = new OXHorizontalFrame(this, 10, 10, FIXED_WIDTH);

    //sprintf(name, "x.%s", ext);
    //pic = MimeTypeList->GetIcon(name, False);
    EditingMime = mime;
    pic = mime ? MimeTypeList->GetIcon(mime, False) :
                 default_icon[ICON_DOC].icon[ICON_BIG];
    if (!pic) pic = default_icon[ICON_DOC].icon[ICON_BIG];
    icon1 = new OXIcon(Ihf, pic, 32, 32);
    Ily = new OLayoutHints(LHINTS_LEFT | LHINTS_CENTER_Y, 10, 10, 15, 15);

    SetIcon = new OXTextButton(Ihf, new OHotString("Change &Icon..."), ID_CHANGEICON);
    SetIcon->Associate(this);
    lyl = new OLayoutHints(LHINTS_LEFT | LHINTS_CENTER_Y);
    lyr = new OLayoutHints(LHINTS_RIGHT | LHINTS_CENTER_Y);

    Ihf->AddFrame(icon1, Ily);
    Ihf->AddFrame(SetIcon, lyr);

    Ihf->Resize(SetIcon->GetDefaultWidth() + icon1->GetDefaultWidth() +40,
                Ihf->GetDefaultHeight());

    //--- layout for separator lines...

    lyln = new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 5, 5, 5, 5);

    //--- text entry frames...

    hf1 = new OXHorizontalFrame(this, 10, 10);
    lbl1 = new OXLabel(hf1, new OHotString("&Description of type:"));
    txt1 = new OXTextEntry(hf1, NULL, 101);
    txt1->Resize(180, txt1->GetDefaultHeight());
    if (mime) txt1->AddText(0, mime->description);
    hf1->AddFrame(lbl1, lyl);
    hf1->AddFrame(txt1, lyr);

    hf2 = new OXHorizontalFrame(this, 10, 10);
    lbl2 = new OXLabel(hf2, new OHotString("Content &Type (MIME):"));
    txt2 = new OXTextEntry(hf2, NULL, 103);
    txt2->Resize(180, txt2->GetDefaultHeight());
#if 0
    MimeTypeList->GetType(name, tmp);
    txt2->AddText(0, tmp);
#else
    if (mime) txt2->AddText(0, mime->type);
#endif
    hf2->AddFrame(lbl2, lyl);
    hf2->AddFrame(txt2, lyr);

    hf3 = new OXHorizontalFrame(this, 10, 10);
    lbl3 = new OXLabel(hf3, new OHotString("Matching &Pattern:"));

    txt3 = new OXTextEntry(hf3, NULL, 102);
    txt3->Resize(180, txt3->GetDefaultHeight());
    if (mime) txt3->AddText(0, mime->pattern);
    hf3->AddFrame(lbl3, lyl);
    hf3->AddFrame(txt3, lyr);

    hf4 = new OXHorizontalFrame(this, 10, 10);
    lbl4 = new OXLabel(hf4, new OHotString("&Handle by:"));
    txt4 = new OXTextEntry(hf4, NULL, 104);
    txt4->Resize(250, txt4->GetDefaultHeight());
#if 0
    MimeTypeList->GetAction(name, tmp);
    txt4->AddText(0, tmp);
#else
    if (mime) txt4->AddText(0, mime->action);
#endif
    hf4->AddFrame(lbl4, lyl);
    hf4->AddFrame(txt4, lyr);

    lv = new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 5, 5, 2, 2);
    llv = new OLayoutHints(LHINTS_TOP | LHINTS_LEFT, 5, 5, 2, 0);

    AddFrame(Ihf, llv);
    AddFrame(new OXHorizontal3dLine(this), lyln);
    AddFrame(hf1, lv);
    AddFrame(hf2, lv);
    AddFrame(hf3, lv);
    AddFrame(hf4, lv);
    AddFrame(new OXHorizontal3dLine(this), lyln);

    MapSubwindows();
    ODimension size = GetDefaultSize();
    size.w += 10;
    size.h += 15;
    Resize(size);

    //---- position relative to the parent's window (the default was root 0,0!)
 
    XTranslateCoordinates(GetDisplay(),
                          main->GetId(), GetParent()->GetId(),
                          50, 50, &ax, &ay, &wdummy);

    Move(ax, ay);

    //---- make dialog non-resizable

    SetWMSize(size.w, size.h);
    SetWMSizeHints(size.w, size.h, size.w, size.h, 0, 0);

    sprintf(name, "%s File Type", EditingMime ? "Edit" : "Add New");
    SetWindowName(name);
    SetIconName(name);
    SetClassHints("Explorer", "Explorer");

    SetMWMHints(MWM_DECOR_ALL | MWM_DECOR_RESIZEH | MWM_DECOR_MAXIMIZE |
                                MWM_DECOR_MINIMIZE | MWM_DECOR_MENU,
                MWM_FUNC_ALL | MWM_FUNC_RESIZE | MWM_FUNC_MAXIMIZE | 
                               MWM_FUNC_MINIMIZE,
                MWM_INPUT_MODELESS);

    SetFocusOwner(txt1);

    MapWindow();
    _client->WaitFor(this);
}

OXEditMimeDialog::~OXEditMimeDialog() {

  delete bly; delete bfly;

  delete Ily;
  delete lyln;

  delete lyl; delete lyr;
  delete lv; delete llv;
}

int OXEditMimeDialog::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;

  switch (msg->action) {
    case MSG_CLICK:

      switch (msg->type) {
        case MSG_BUTTON:
          switch (wmsg->id) {
            case ID_OK:
              if (!UpdateMime()) break;
              if (_retc) *_retc = ID_OK;
            case ID_CANCEL:
              CloseWindow();
              break;

            case ID_CHANGEICON:
              new OXIconSelDialog(_client->GetRoot(), this, NULL);
              break;

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

int OXEditMimeDialog::UpdateMime() {
  char *descr, *type, *pattern, *action, *icon;

  // get values
  descr   = (char *) txt1->GetString();
  type    = (char *) txt2->GetString();
  pattern = (char *) txt3->GetString();
  action  = (char *) txt4->GetString();
  icon    = (char *) icon1->GetPicture()->GetName();

  if (!*descr) {
    new OXMsgBox(_client->GetRoot(), this,
                 new OString("Edit Mime"),
                 new OString("Please enter a Description for this MIME"),
                 MB_ICONSTOP, ID_OK);
    return False;
  }

  if (!*type) {
    new OXMsgBox(_client->GetRoot(), this,
                 new OString("Edit Mime"),
                 new OString("Please enter a valid Content Type for this MIME"),
                 MB_ICONSTOP, ID_OK);
    return False;
  }

  if (EditingMime) {
    // modify 
    MimeTypeList->Modify(EditingMime, descr, type, pattern, icon, action);
  } else {
    // update 
    MimeTypeList->AddType(descr, type, pattern, icon, action);
  }

  return True;
}
