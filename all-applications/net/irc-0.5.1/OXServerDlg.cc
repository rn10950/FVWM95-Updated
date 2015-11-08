/**************************************************************************

    This file is part of foxirc, a cool irc client.
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
  
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <xclass/utils.h>

#include "OXServerDlg.h"

extern OSettings  *foxircSettings;

//---------------------------------------------------------------------

OXServerDlg::OXServerDlg(const OXWindow *p, const OXWindow *main,
                         OString *title, OServerInfo *info,
                         int *ret_code, int connect, unsigned long options) :
  OXTransientFrame(p, main, 10, 10, options) {

    int width = 0, height = 0;

    _ret_code = ret_code;
    _info = info;
    _connect = connect;

    if (_ret_code) *_ret_code = ID_NO;

    L1 = new OLayoutHints(LHINTS_CENTER_Y | LHINTS_EXPAND_X, 3, 3, 0, 0);

    //--- create the label for the top text

    L3 = new OLayoutHints(LHINTS_CENTER_X | LHINTS_CENTER_Y,
                          5, 5, 15, 15);
//    AddFrame(new OXLabel(this, text), L3);

    //--- create the frame for the entry boxes

    OXCompositeFrame *f1 = new OXHorizontalFrame(this, 0, 0, 0);
    f1->SetLayoutManager(new O2ColumnsLayout(f1, 5, 5));

    f1->AddFrame(new OXLabel(f1, new OString("Name")), NULL);

    if (_connect) {
      _name = new OXComboBox(f1, "", -1);
      const OXSNode *ptr;
      int   i;

      for (ptr = foxircSettings->GetServerList()->GetHead(), i = 1;
           ptr != NULL;
           ptr = ptr->next, i++) {
        OServerInfo *e = (OServerInfo *) ptr->data;
        if (e->name) _name->AddEntry(new OString(e->name), i);
      }
      _name->Associate(this);
      _name->Resize(200, _name->GetDefaultHeight());
      f1->AddFrame(_name, NULL);

    } else {
      _nameedit = new OXTextEntry(f1, new OTextBuffer(100));
      _nameedit->Associate(this);
      _nameedit->Resize(200, _nameedit->GetDefaultHeight());
      f1->AddFrame(_nameedit, NULL);
    }

    f1->AddFrame(new OXLabel(f1, new OString("Hostname")), NULL);
    _hname = new OXTextEntry(f1, new OTextBuffer(100));
    _hname->Associate(this);
    _hname->Resize(200, _hname->GetDefaultHeight());
    f1->AddFrame(_hname, NULL);

    f1->AddFrame(new OXLabel(f1, new OString("Port")), NULL);
    _port = new OXTextEntry(f1, new OTextBuffer(100));
    _port->Associate(this);
    _port->Resize(200, _port->GetDefaultHeight());
    f1->AddFrame(_port, NULL);

    f1->AddFrame(new OXLabel(f1, new OString("Server passwd")), NULL);
    _psw = new OXTextEntry(f1, new OTextBuffer(100));
    _psw->Associate(this);
    _psw->Resize(200, _psw->GetDefaultHeight());
    f1->AddFrame(_psw, NULL);

    f1->AddFrame(new OXLabel(f1, new OString("Nick")), NULL);
    _nick = new OXTextEntry(f1, new OTextBuffer(100));
    _nick->Associate(this);
    _nick->Resize(200, _nick->GetDefaultHeight());
    f1->AddFrame(_nick, NULL);

    f1->AddFrame(new OXLabel(f1, new OString("IRC name")), NULL);
    _uname = new OXTextEntry(f1, new OTextBuffer(100));
    _uname->Associate(this);
    _uname->Resize(200, _uname->GetDefaultHeight());
    f1->AddFrame(_uname, NULL);

    f1->AddFrame(new OXLabel(f1, new OString("Op nick")), NULL);
    _opnick= new OXTextEntry(f1, new OTextBuffer(100));
    _opnick->Associate(this);
    _opnick->Resize(200, _opnick->GetDefaultHeight());
    f1->AddFrame(_opnick, NULL);

    f1->AddFrame(new OXLabel(f1, new OString("Op passwd")), NULL);
    _oppsw = new OXTextEntry(f1, new OTextBuffer(100));
    _oppsw->Associate(this);
    _oppsw->Resize(200, _oppsw->GetDefaultHeight());
    f1->AddFrame(_oppsw, NULL);

    L4 = new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 3, 3, 5, 5);
    L5 = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_CENTER_Y, 3, 3, 0, 10);

    AddFrame(f1, L4);
    AddFrame(new OXHorizontal3dLine(this), L5);

    //--- create the buttons and button frame

    OXCompositeFrame *ButtonFrame = new OXHorizontalFrame(this,
                                                    60, 20, FIXED_WIDTH);

    if (_connect)
      OK = new OXTextButton(ButtonFrame, new OHotString("C&onnect"), ID_OK);
    else
      OK = new OXTextButton(ButtonFrame, new OHotString("&OK"), ID_OK);

    OK->Associate(this);
    ButtonFrame->AddFrame(OK, L1);
    width = max(width, OK->GetDefaultWidth());

    Cancel = new OXTextButton(ButtonFrame, new OHotString("&Cancel"), ID_CANCEL);
    Cancel->Associate(this);
    ButtonFrame->AddFrame(Cancel, L1);
    width = max(width, Cancel->GetDefaultWidth());

    //--- place buttons at the bottom

    L2 = new OLayoutHints(LHINTS_BOTTOM | LHINTS_CENTER_X, 0, 0, 0, 5);
    AddFrame(ButtonFrame, L2);

    //--- Keep the buttons centered and with the same width

    ButtonFrame->Resize((width + 20) * 2, GetDefaultHeight());

    if (_info) {
      if (_info->name) {
        if (_connect)
          _name->SetText(_info->name);
        else
          _nameedit->AddText(0, _info->name);
      }
      if (_info->hostname) _hname->AddText(0, _info->hostname);
      if (_info->port > 0) {
        char tmp[20]; sprintf(tmp, "%d", _info->port);
        _port->AddText(0, tmp);
      } else {
        _port->AddText(0, "6667");
      }
      if (_info->passwd)   _psw->AddText(0, _info->passwd);
      if (_info->nick)     _nick->AddText(0, _info->nick);
      if (_info->ircname)  _uname->AddText(0, _info->ircname);
      if (_info->opnick)   _opnick->AddText(0, _info->opnick);
      if (_info->oppasswd) _oppsw->AddText(0, _info->oppasswd);
    }

    if (_connect)
      SetFocusOwner(_name->GetTextEntry());
    else
      SetFocusOwner(_nameedit);

    SetDefaultAcceptButton(OK);
    SetDefaultCancelButton(Cancel);

    MapSubwindows();
    
    width  = GetDefaultWidth();
    height = GetDefaultHeight();

    Resize(width, height);

    //--- position relative to the parent's window

    CenterOnParent();

    //---- make the dialog box non-resizable

    SetWMSize(width, height);
    SetWMSizeHints(width, height, width, height, 0, 0);

    //---- set names

    SetWindowName((char *) title->GetString());
    SetIconName((char *) title->GetString());
    SetClassHints("fOXirc", "Dialog");

    SetMWMHints(MWM_DECOR_ALL | MWM_DECOR_RESIZEH | MWM_DECOR_MAXIMIZE |
                                MWM_DECOR_MINIMIZE | MWM_DECOR_MENU,
                MWM_FUNC_ALL | MWM_FUNC_RESIZE | MWM_FUNC_MAXIMIZE |
                               MWM_FUNC_MINIMIZE,
                MWM_INPUT_MODELESS);

    MapWindow();
    _client->WaitFor(this);
}

OXServerDlg::~OXServerDlg() {
  delete L1;
  delete L2;
  delete L3;
  delete L4;
  delete L5;
}

int OXServerDlg::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;

  switch (msg->action) {
  case MSG_CLICK:
    switch (msg->type) {
    case MSG_LISTBOX:
    case MSG_COMBOBOX:
      {
      OListBoxMessage *lbmsg = (OListBoxMessage *) msg;
      //OXLBEntry *
      const OXSNode *nptr = foxircSettings->GetServerList()->GetNode(lbmsg->entryID);
      if (!nptr) return true;
      OServerInfo *ptr = (OServerInfo *) (nptr->data);

      if (ptr->hostname) { _hname->Clear(); _hname->AddText(0, ptr->hostname); }
      if (ptr->passwd)   { _psw->Clear(); _psw->AddText(0, ptr->passwd); }
      if (ptr->ircname)  { _uname->Clear(); _uname->AddText(0, ptr->ircname); }
      if (ptr->nick)     { _nick->Clear(); _nick->AddText(0, ptr->nick); }
      if (ptr->opnick)   { _opnick->Clear(); _opnick->AddText(0, ptr->opnick); }
      if (ptr->oppasswd) { _oppsw->Clear(); _oppsw->AddText(0, ptr->oppasswd); }
      if (ptr->port) {
        char tmp[20]; 
        _port->Clear();
        sprintf(tmp, "%d", ptr->port);
        _port->AddText(0, tmp);
      }
      }
      break;

    case MSG_BUTTON:
      if (_ret_code) *_ret_code = wmsg->id;
      if (_info && (wmsg->id == ID_OK)) {

        if (_info->name) delete[] _info->name;
        if (_connect) {
          _info->name = StrDup(_name->GetText());
        } else {
          _info->name = StrDup(_nameedit->GetString());
        }

        if (_info->hostname) delete[] _info->hostname;
        _info->hostname = StrDup(_hname->GetString());

        _info->port = atoi(_port->GetString());

        if (_info->passwd) delete[] _info->passwd;
        _info->passwd = StrDup(_psw->GetString());

        if (_info->nick) delete[] _info->nick;
        _info->nick = StrDup(_nick->GetString());

        if (_info->ircname) delete[] _info->ircname;
        _info->ircname = StrDup(_uname->GetString());

        if (_info->opnick) delete[] _info->opnick;
        _info->opnick = StrDup(_opnick->GetString());

        if (_info->oppasswd) delete[] _info->oppasswd;
        _info->oppasswd = StrDup(_oppsw->GetString());
      }
      CloseWindow();
      break;

    default:
      break;
    }
    break;
  }
  return True;
}
