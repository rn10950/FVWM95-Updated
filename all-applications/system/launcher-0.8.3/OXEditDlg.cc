/**************************************************************************

    This file is part of xclauncher.
    Copyright (C) 1998-2002 Hector Peraza.

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
#include <sys/types.h>
#include <sys/stat.h>

#include <xclass/utils.h>
#include <xclass/OXTextButton.h>
#include <xclass/OXTextEntry.h>
#include <xclass/OX3dLines.h>
#include <xclass/OXMsgBox.h>
#include <xclass/OXIcon.h>
#include <xclass/OXLabel.h>

#include "OXEditDlg.h"
#include "OXIconSelDlg.h"
#include "main.h"


#define ID_CHANGEICON  (ID_OK+ID_CANCEL+100)


//-------------------------------------------------------------------

OXEditButtonDialog::OXEditButtonDialog(const OXWindow *p,
            const OXWindow *main, SButton *button, int *retc,
            unsigned long options) :
  OXTransientFrame(p, main, 400, 200, options) {
    int width;
    const OPicture *pic;
    char name[1024];

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

    _button = button;
    pic = _client->GetPicture(_button->icon);
    if (!pic) {
      pic = _client->GetPicture("default.xpm");  // already loaded by main
      if (!pic) FatalError("Failed to load default pixmap"); // must never happen
    }
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
    lbl1 = new OXLabel(hf1, new OHotString("&Name:"));
    txt1 = new OXTextEntry(hf1, NULL, 101);
    txt1->Resize(200, txt1->GetDefaultHeight());
    txt1->AddText(0, _button->name);
    hf1->AddFrame(lbl1, lyl);
    hf1->AddFrame(txt1, lyr);

    hf2 = new OXHorizontalFrame(this, 10, 10);
    lbl2 = new OXLabel(hf2, new OHotString("Command:"));
    txt2 = new OXTextEntry(hf2, NULL, 103);
    txt2->Resize(200, txt2->GetDefaultHeight());
    txt2->AddText(0, _button->command);
    hf2->AddFrame(lbl2, lyl);
    hf2->AddFrame(txt2, lyr);

    hf3 = new OXHorizontalFrame(this, 10, 10);
    lbl3 = new OXLabel(hf3, new OHotString("Start on &dir:"));
    txt3 = new OXTextEntry(hf3, new OTextBuffer(PATH_MAX), 102);
    txt3->Resize(200, txt3->GetDefaultHeight());
    txt3->AddText(0, _button->start_dir);
    hf3->AddFrame(lbl3, lyl);
    hf3->AddFrame(txt3, lyr);

    lv = new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 5, 5, 2, 2);
    llv = new OLayoutHints(LHINTS_TOP | LHINTS_LEFT, 5, 5, 2, 0);

    AddFrame(Ihf, llv);
    AddFrame(new OXHorizontal3dLine(this), lyln);
    AddFrame(hf1, lv);
    AddFrame(hf2, lv);
    AddFrame(hf3, lv);
    AddFrame(new OXHorizontal3dLine(this), lyln);

    MapSubwindows();
    ODimension size = GetDefaultSize();
    size.w += 10;
    size.h += 15;
    Resize(size);

    CenterOnParent();

    //---- make dialog non-resizable

    SetWMSize(size.w, size.h);
    SetWMSizeHints(size.w, size.h, size.w, size.h, 0, 0);

    sprintf(name, "%s Button", _button ? "Edit" : "Add New");
    SetWindowName(name);
    SetIconName(name);
    SetClassHints("XCLASS", "dialog");

    SetMWMHints(MWM_DECOR_ALL | MWM_DECOR_RESIZEH | MWM_DECOR_MAXIMIZE |
                                MWM_DECOR_MINIMIZE | MWM_DECOR_MENU,
                MWM_FUNC_ALL | MWM_FUNC_RESIZE | MWM_FUNC_MAXIMIZE | 
                               MWM_FUNC_MINIMIZE,
                MWM_INPUT_MODELESS);

    SetFocusOwner(txt1);

    MapWindow();
    _client->WaitFor(this);
}

OXEditButtonDialog::~OXEditButtonDialog() {

  delete bly; delete bfly;

  delete Ily;
  delete lyln;

  delete lyl; delete lyr;
  delete lv; delete llv;
}

int OXEditButtonDialog::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;
  OString str("");
  const OPicture *pic;

  switch (msg->action) {
    case MSG_CLICK:

      switch (msg->type) {
        case MSG_BUTTON:
          switch (wmsg->id) {
            case ID_OK:
              if (!UpdateButton()) break;
              if (_retc) *_retc = ID_OK;
            case ID_CANCEL:
              CloseWindow();
              break;

            case ID_CHANGEICON:
              new OXIconSelDialog(_client->GetRoot(), this, &str);
              pic = _client->GetPicture(str.GetString());
              if (pic) {
                icon1->SetPicture(pic);
                Ihf->Layout();
              }
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

int OXEditButtonDialog::UpdateButton() {
  char *name, *command, *sdir, *icon;

  // get values
  name    = (char *) txt1->GetString();
  command = (char *) txt2->GetString();
  sdir    = (char *) txt3->GetString();

  if (icon1->GetPicture())
    icon = (char *) icon1->GetPicture()->GetName();
  else
    icon = "default.xpm";

  if (!*name) {
    new OXMsgBox(_client->GetRoot(), this,
                 new OString("Edit Button"),
                 new OString("Please enter a name for this button"),
                 MB_ICONSTOP, ID_OK);
    return False;
  }

  if (!*command) {
    new OXMsgBox(_client->GetRoot(), this,
                 new OString("Edit Button"),
                 new OString("Please enter a valid command for this button"),
                 MB_ICONSTOP, ID_OK);
    return False;
  }

  if (_button) {
    delete[] _button->name;
    _button->name = StrDup(name);

    delete[] _button->command;
    _button->command = StrDup(command);

    delete[] _button->icon;
    _button->icon = StrDup(icon);

    delete[] _button->start_dir;
    _button->start_dir = StrDup(sdir);
  }

  return True;
}
