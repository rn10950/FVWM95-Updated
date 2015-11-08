/**************************************************************************

    This file is part of foxirc, a cool irc client :)
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

#include <xclass/OXClient.h>
#include <xclass/OXWindow.h>
#include <xclass/OXTransientFrame.h>
#include <xclass/OXGroupFrame.h>
#include <xclass/OXMsgBox.h>
#include <xclass/OXColorSelect.h>
#include <xclass/OString.h>
#include <xclass/OPicture.h>
#include <xclass/OIniFile.h>
#include <xclass/OResourcePool.h>
#include <xclass/utils.h>

#include "OXPreferences.h"
#include "OXServerDlg.h"
#include "OXChannelEditor.h"

#define SETTINGS_CHANGED 	(MSG_USERMSG+100)

extern unsigned long IRCcolors[];

struct SColors {
  const char *name;
  int r, g, b;
};

SColors cpref[] = {
  { "Background",  0x00, 0x00, 0x00 },
  { "Action",      0xff, 0x00, 0xff },
  { "CTCP",        0xff, 0xff, 0x00 },
  { "Highlight",   0x00, 0xff, 0xff },
  { "Server 1",    0x00, 0xff, 0x00 },
  { "Server 2",    0xff, 0xff, 0xff },
  { "Invite",      0xff, 0x80, 0x00 },
  { "Join",        0xc0, 0xc0, 0xc0 },
  { "Kick",        0xff, 0x00, 0x00 },
  { "Mode",        0xff, 0x00, 0x00 },
  { "Nick",        0x00, 0x00, 0xff },
  { "Normal text", 0xff, 0xff, 0xff },
  { "Notice",      0xff, 0xff, 0x00 },
  { "Notify",      0xff, 0x80, 0x00 },
  { "Other",       0x00, 0xff, 0xff },
  { "Own text",    0xff, 0xff, 0xff },
  { "Part",        0x00, 0x00, 0xff },
  { "Quit",        0xff, 0x80, 0x00 },
  { "Topic",       0x00, 0xff, 0x00 },
  { "Wallops",     0xff, 0x00, 0x00 }
};

#define NUM_COLORS  (sizeof(cpref)/sizeof(cpref[0]))

// color table, similar to the one used by mIRC and cIRCus

SColors mircColors[16] = {
  { "white",         0xff, 0xff, 0xff },
  { "black",         0x00, 0x00, 0x00 },
  { "royalblue",     0x00, 0x00, 0x80 },
  { "darkgreen",     0x00, 0x80, 0x00 },
  { "red3",          0xff, 0x00, 0x00 },
  { "brown",         0x80, 0x00, 0x00 },
  { "mediumpurple",  0x80, 0x00, 0x80 },
  { "orange",        0xff, 0x80, 0x00 },
  { "yellow",        0xff, 0xff, 0x00 },
  { "lightgreen",    0x00, 0xff, 0x00 },
  { "cyan",          0x00, 0x80, 0x80 },
  { "lightcyan",     0x00, 0xff, 0xff },
  { "lightblue",     0x00, 0x00, 0xff },
  { "pink",          0xff, 0x00, 0xff },
  { "darkslategray", 0x80, 0x80, 0x80 },
  { "lightgray",     0xcf, 0xcf, 0xcf }
};

//----------------------------------------------------------------------

OServerInfo::~OServerInfo() {
  if (name)     delete[] name;
  if (hostname) delete[] hostname;
  if (ircname)  delete[] ircname;
  if (nick)     delete[] nick;
  if (opnick)   delete[] opnick;
  if (passwd)   delete[] passwd;
  if (oppasswd) delete[] oppasswd;
}


//----------------------------------------------------------------------

OColorsPref::OColorsPref(const char *n, int r, int g, int b) {
  if (n) name = StrDup(n); else n = NULL;
  color = OColor(r, g, b);
}

OColorsPref::~OColorsPref() {
  if (name) delete[] name;
}


//----------------------------------------------------------------------

OSettings::OSettings(OXClient *c) {
  int i;

  _client = c;

  _confirm    = P_CONFIRM_QUIT | P_CONFIRM_LEAVE | P_CONFIRM_SQUIT;
  _sendToInfo = P_LOG_WHO  | P_LOG_WHOIS | P_LOG_WHOWAS | P_LOG_ERROR |
                P_LOG_CLOSE;
  _misc = P_MISC_ENABLE_CMD | P_MISC_SHOW_OPS | P_MISC_AUTO_REJOIN;

  _changed = False;

  _servers  = new OXSList(_client->GetDisplay(), "Servers");
  _nicks    = new OXSList(_client->GetDisplay(), "Nicks");
  _names    = new OXSList(_client->GetDisplay(), "Names");
  _channels = new OXSList(_client->GetDisplay(), "Channels");

  _font = _client->GetFont("lucidatypewriter -12");

  for (i = 0; i < NUM_COLORS; ++i) {
    _colors[i] = new OColorsPref(cpref[i].name,
                                 cpref[i].r,
                                 cpref[i].g,
                                 cpref[i].b);
    _colors[i]->pixel = _client->GetColor(_colors[i]->color);
  }

  for (i = 0; i < 16; ++i) {
    _ircColors[i] = new OColorsPref(mircColors[i].name,
                                    mircColors[i].r,
                                    mircColors[i].g,
                                    mircColors[i].b);
    _ircColors[i]->pixel = _client->GetColor(_ircColors[i]->color);
  }
}

OSettings::~OSettings() {
  delete _servers;
  delete _nicks;
  delete _names;
  delete _channels;
  for (int i = 0; i < NUM_COLORS; ++i) delete _colors[i];
  for (int i = 0; i < 16; ++i) delete _ircColors[i];
  _client->FreeFont(_font);
}

int OSettings::Load() {
  char *inipath, line[1024], arg[256];
  int  sc = 1, cc = 1;

  inipath = _client->GetResourcePool()->FindIniFile("foxircrc", INI_READ);
  if (!inipath) return False;

  OIniFile ini(inipath, INI_READ);

  while (ini.GetNext(line)) {
    if (strcasecmp(line, "foxirc") == 0) {

      if (ini.GetItem("Confirm", arg))
        _confirm = atoi(arg);

      if (ini.GetItem("SendToInfo", arg))
        _sendToInfo = atoi(arg);

      if (ini.GetItem("Misc", arg))
        _misc = atoi(arg);

      if (ini.GetItem("Font", arg)) {
        OXFont *f = _client->GetFont(arg);
        if (f) {
          _client->FreeFont(_font);
          _font = f;
        }
      }

    } else if (strcasecmp(line, "server") == 0) {

      // Loading a server...
      OServerInfo *e = new OServerInfo();

      if (ini.GetItem("name", arg))     e->name     = StrDup(arg);
      if (ini.GetItem("hostname", arg)) e->hostname = StrDup(arg);
      if (ini.GetItem("passwd", arg))   e->passwd   = StrDup(arg);
      if (ini.GetItem("nick", arg))     e->nick     = StrDup(arg);
      if (ini.GetItem("ircname", arg))  e->ircname  = StrDup(arg);
      if (ini.GetItem("opnick", arg))   e->opnick   = StrDup(arg);
      if (ini.GetItem("oppasswd", arg)) e->oppasswd = StrDup(arg);
      if (ini.GetItem("logfile", arg))  e->logfile  = StrDup(arg);
      if (ini.GetItem("port", arg))     e->port     = atoi(arg);
      _servers->Add(sc++, (XPointer) e);

    } else if (strcasecmp(line, "channel") == 0) {

      // Loading a channel...
      OChannelInfo *e = new OChannelInfo();

      if (ini.GetItem("name", arg))    e->name    = StrDup(arg);
      if (ini.GetItem("logfile", arg)) e->logfile = StrDup(arg);
      if (ini.GetItem("background", arg)) e->background = StrDup(arg);
      if (ini.GetItem("flags", arg))   e->flags   = atoi(arg);
      _channels->Add(cc++, (XPointer) e);

    } else if (strcasecmp(line, "colors") == 0) {

      int i, r, g, b;

      for (i = 0; i < NUM_COLORS; ++i) {
        if (ini.GetItem(_colors[i]->name, arg)) {
          if (sscanf(arg, "%d,%d,%d", &r, &g, &b) == 3) {
            if (r >= 0 && r <= 255 &&
                g >= 0 && g <= 255 &&
                b >= 0 && b <= 255) {
              _colors[i]->color.SetRGB(r, g, b);
              _client->FreeColor(_colors[i]->pixel);
              _colors[i]->pixel = _client->GetColor(_colors[i]->color);
            }
          }
        }
      }

    }
  }

  return True;
}

int OSettings::CheckChannelFlags(const char *name, int what) {
  OChannelInfo *ptr = FindChannel(name);
  if (!ptr) return false;
  return (ptr->flags & what);
}

int OSettings::Save() {
  char  *inipath, arg[256], port[40];
  const OXSNode *ptr;
  int   i;

  inipath = _client->GetResourcePool()->FindIniFile("foxircrc", INI_WRITE);
  if (!inipath) return False;

  OIniFile ini(inipath, INI_WRITE);

  ini.PutNext("foxirc");
  sprintf(arg, "%d", _confirm);    ini.PutItem("Confirm", arg);
  sprintf(arg, "%d", _sendToInfo); ini.PutItem("SendToInfo", arg);
  sprintf(arg, "%d", _misc);       ini.PutItem("Misc", arg);
  ini.PutItem("Font", _font->NameOfFont());
  ini.PutNewLine();

  for (ptr = _servers->GetHead(), i = 1;
       ptr != NULL;
       ptr = ptr->next, ++i) {
    ini.PutNext("server");
    OServerInfo *e = (OServerInfo *) ptr->data;
    if (e->name)     ini.PutItem("name",     e->name);
    if (e->hostname) ini.PutItem("hostname", e->hostname);
    if (e->passwd)   ini.PutItem("passwd",   e->passwd);
    if (e->nick)     ini.PutItem("nick",     e->nick);
    if (e->ircname)  ini.PutItem("ircname",  e->ircname);
    if (e->opnick)   ini.PutItem("opnick",   e->opnick);
    if (e->oppasswd) ini.PutItem("oppasswd", e->oppasswd);
    if (e->logfile)  ini.PutItem("logfile",  e->logfile);
    if (e->port) {
      sprintf(port, "%d", e->port);
      ini.PutItem("port", port);
    }
    ini.PutNewLine();
  }

  for (ptr = _channels->GetHead(), i = 1;
       ptr != NULL;
       ptr = ptr->next, ++i) {
    ini.PutNext("channel");
    OChannelInfo *e = (OChannelInfo *) ptr->data;
    if (e->name)       ini.PutItem("name", e->name);
    if (e->logfile)    ini.PutItem("logfile", e->logfile);
    if (e->background) ini.PutItem("background", e->background);
    if (e->flags) {
      sprintf(arg, "%d", e->flags);
      ini.PutItem("flags", arg);
    }
    ini.PutNewLine();
  }

  ini.PutNext("colors");
  for (i = 0; i < NUM_COLORS; ++i) {
    sprintf(arg, "%d,%d,%d",
                 _colors[i]->color.GetR(),
                 _colors[i]->color.GetG(),
                 _colors[i]->color.GetB());
    ini.PutItem(_colors[i]->name, arg);
  }
  ini.PutNewLine();    

  _changed = False;

  return True;
}

void OSettings::SetColor(int id, OColor color) {
  _colors[id]->color.SetRGB(color.GetR(), color.GetG(), color.GetB());
  _client->FreeColor(_colors[id]->pixel);
  _colors[id]->pixel = _client->GetColor(_colors[id]->color);
  _changed = True;
}

void OSettings::SetFont(const char *name) {
  OXFont *f = _client->GetFont(name);
  if (f) {
    _client->FreeFont(_font);
    _font = f;
  }
}

OServerInfo *OSettings::FindServer(const char *name) {
  int i;
  OServerInfo *ep;
  const OXSNode *ptr;

  for (ptr = _servers->GetHead(), i = 1;
       ptr != NULL;
       ptr = ptr->next, ++i) {
    ep = (OServerInfo *) ptr->data;
    if (strcasecmp(ep->name, name) == 0) return ep;
  }

  return (OServerInfo *) 0;
}

OChannelInfo *OSettings::FindChannel(const char *name) {
  int i;
  OChannelInfo *ep;
  const OXSNode *ptr;

  for (ptr = _channels->GetHead(), i = 1;
       ptr != NULL;
       ptr = ptr->next, ++i) {
    ep = (OChannelInfo *) ptr->data;
    if (strcasecmp(ep->name, name) == 0) return ep;
  }

  return (OChannelInfo *) 0;
}

//----------------------------------------------------------------------

OXPreferencesDialog::OXPreferencesDialog(const OXWindow *p,
                                         const OXWindow *main,
                                         OSettings *settings,
                                         unsigned long options) :
  OXTransientFrame(p, main, 400, 200, options) {
    OXCompositeFrame *tf;
    int width, height;

    _settings = settings;

    //=============== setup elements:

    //====== [1] Tab widget

    tab = new OXTab(this, 320, 350);
    //Ltab = new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 5, 5, 5, 5);
    Ltab = new OLayoutHints(LHINTS_EXPAND_Y | LHINTS_EXPAND_X, 5, 5, 5, 5);

    tf = tab->AddTab(new OString("fOXirc"));
    tf->AddFrame(new OXIrcTab(tf, _settings), Ltab);

    tf = tab->AddTab(new OString("Names"));
    tf->AddFrame(new OXNamesTab(tf, _settings), Ltab);

    tf = tab->AddTab(new OString("Servers"));
    tf->AddFrame(new OXServersTab(tf, _settings), Ltab);

    tf = tab->AddTab(new OString("Channels"));
    tf->AddFrame(new OXChannelTab(tf, _settings), Ltab);

    tf = tab->AddTab(new OString("Colors"));
    tf->AddFrame(new OXColorsTab(tf, _settings), Ltab);

    tf = tab->AddTab(new OString("People"));
    tf = tab->AddTab(new OString("Ignores"));

    tf = tab->AddTab(new OString("Info"));
    tf->AddFrame(new OXInfoTab(tf, _settings), Ltab);

    AddFrame(tab, Ltab);

    //======= [2] this frame will contain the "OK Cancel Apply" buttons:

    bframe = new OXHorizontalFrame(this, 60, 20, FIXED_WIDTH);

    //--- create the buttons
    Ok     = new OXTextButton(bframe, new OHotString("OK"), ID_OK);
    Cancel = new OXTextButton(bframe, new OHotString("Cancel"), ID_CANCEL);
    Apply  = new OXTextButton(bframe, new OHotString("&Apply"), ID_APPLY);

    //--- Apply is initially disabled, OK is the default
    Apply->Disable();

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

    //-----------------------------

    SetDefaultAcceptButton(Ok);
    SetDefaultCancelButton(Cancel);

    MapSubwindows();

    width  = 500; // GetDefaultWidth();
    height = 350; // GetDefaultHeight();
    Resize(width, height);

    CenterOnParent();

    //---- make dialog non-resizable

    SetWMSize(width, height);
    SetWMSizeHints(width, height, width, height, 0, 0);

    SetWindowName("Preferences");
    SetIconName("Preferences");
    SetClassHints("fOXirc", "fOXirc");

    SetMWMHints(MWM_DECOR_ALL | MWM_DECOR_RESIZEH | MWM_DECOR_MAXIMIZE |
                                MWM_DECOR_MINIMIZE | MWM_DECOR_MENU,
                MWM_FUNC_ALL | MWM_FUNC_RESIZE | MWM_FUNC_MAXIMIZE | 
                               MWM_FUNC_MINIMIZE,
                MWM_INPUT_MODELESS);

    MapWindow();
    _client->WaitFor(this);
}

OXPreferencesDialog::~OXPreferencesDialog() {
  delete bly;
  delete bfly;
  delete Ltab;
}

int OXPreferencesDialog::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;

  switch (msg->type) {
    case SETTINGS_CHANGED:
      Apply->Enable();
      break;
   
    case MSG_BUTTON:
      switch (msg->action) {
        case MSG_CLICK:
          switch (wmsg->id) {
            case ID_OK:
              _settings->Save();
              CloseWindow();
              break;

            case ID_CANCEL:
              // restore old value of settings...
              CloseWindow();
              break;

            case ID_APPLY:
              _settings->Save();
              Apply->Disable();
              break;
          }
          break;
      }
      break;

    case MSG_CHECKBUTTON:
    case MSG_RADIOBUTTON:
      switch (msg->action) {
        case MSG_CLICK:
          //switch (msg->id) {
          //}
          Apply->Enable();
          break;
      }
      break;

    default:
      break;
  }

  return True;
}

//----------------------------------------------------------------------

struct _ButtonDef confirm[] = {
  { "Quit",       1001, P_CONFIRM_QUIT,  NULL },
  { "Leave",      1002, P_CONFIRM_LEAVE, NULL },
  { "Kill",       1003, P_CONFIRM_KILL,  NULL },
  { "Save conf",  1004, P_CONFIRM_SAVE,  NULL },
  { "Squit",      1005, P_CONFIRM_SQUIT, NULL },
  { NULL,         -1,   -1,              NULL }
};

struct _ButtonDef stinfo[] = {
  { "CTCP",       1101, P_LOG_CTCP,      NULL },
  { "Signoff",    1102, P_LOG_SIGNOFF,   NULL },
  { "Who",        1103, P_LOG_WHO,       NULL },
  { "Whois",      1104, P_LOG_WHOIS,     NULL },
  { "Whowas",     1105, P_LOG_WHOWAS,    NULL },
  { "List",       1106, P_LOG_CHAN_LIST, NULL },
  { "Links",      1107, P_LOG_LINKS,     NULL },
  { "Error",      1108, P_LOG_ERROR,     NULL },
  { "Ison",       1109, P_LOG_ISON,      NULL },
  { "Kill",       1110, P_LOG_KILL,      NULL },
  { "Close",      1111, P_LOG_CLOSE,     NULL },
  { NULL,         -1,   -1,              NULL }
};

struct _ButtonDef misc[] = {
  { "Enable ircII commands",      1201, P_MISC_ENABLE_CMD,    NULL },
  { "Re-join after kick",         1202, P_MISC_AUTO_REJOIN,   NULL },
  { "Notify all friends",         1203, P_MISC_NOTIFY_ALL,    NULL },
  { "Show ops",                   1204, P_MISC_SHOW_OPS,      NULL },
  { "Show friends",               1205, P_MISC_SHOW_FRIENDS,  NULL },
  { "Show voiced",                1206, P_MISC_SHOW_VOICED,   NULL },
  { "Auto-raise windows",         1207, P_MISC_POPUP_WINDOW,  NULL },
  { "Connect on Start",           1208, P_MISC_POPUP_SERV_CN, NULL },
  { "Channel on Connect",         1209, P_MISC_POPUP_CHAN_CN, NULL },
  { "Transient channel windows",  1210, P_MISC_TRANSIENT_CHW, NULL },
  { NULL,                         -1,   -1,                   NULL }
};

OXIrcTab::OXIrcTab(const OXWindow *p, OSettings *settings) :
  OXHorizontalFrame(p, 10, 10, CHILD_FRAME) {

  _settings = settings;

  _lcb = new OLayoutHints(LHINTS_TOP | LHINTS_LEFT, 15, 2, 2, 2);
  _lgf = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y, 5, 5, 5, 5);

  AddFrame(InitButtons("Confirm", _settings->_confirm, confirm), _lgf);
//  AddFrame(InitButtons("Send to Info", _settings->_sendToInfo, stinfo), _lgf);
  AddFrame(InitButtons("Misc", _settings->_misc, misc), _lgf);
}

OXIrcTab::~OXIrcTab() {
  delete _lcb;
  delete _lgf;
}

OXFrame *OXIrcTab::InitButtons(char *gname, int ctlvar,
                               struct _ButtonDef b[]) {
  int i;
  OXButton *button;

  OXGroupFrame *gf = new OXGroupFrame(this, new OString(gname));
  gf->OldInsets(False);
  for (i = 0; b[i].name != NULL; ++i) {
    button = new OXCheckButton(gf, new OHotString(b[i].name), b[i].id);
    if (ctlvar & b[i].mask) button->SetState(BUTTON_DOWN);
    b[i].button = button;
    button->Associate(this);
    gf->AddFrame(button, _lcb);
  }

  return gf;
}

int OXIrcTab::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;
  int n;

  switch (msg->type) {
    case MSG_CHECKBUTTON:

      switch (msg->action) {
        case MSG_CLICK:
          {
          n = wmsg->id;

          if (n > 1200) {
            if (misc[n-1201].button->GetState() == BUTTON_DOWN)
              _settings->_misc |= misc[n-1201].mask;
            else
              _settings->_misc &= ~misc[n-1201].mask;
            _settings->_changed = True;

          } else if (n > 1100) {
            if (stinfo[n-1101].button->GetState() == BUTTON_DOWN)
              _settings->_sendToInfo |= stinfo[n-1101].mask;
            else
              _settings->_sendToInfo &= ~stinfo[n-1101].mask;
            _settings->_changed = True;

          } else if (n > 1000) {
            if (confirm[n-1001].button->GetState() == BUTTON_DOWN)
              _settings->_confirm |= confirm[n-1001].mask;
            else
              _settings->_confirm &= ~confirm[n-1001].mask;
            _settings->_changed = True;
          }
          OMessage msg(SETTINGS_CHANGED);
          SendMessage(GetTopLevel(), &msg);
          }
          break;

        default:
          break;
      }
      break;

    default:
      break;
  }

  return False;
}


//----------------------------------------------------------------------

OXNamesTab::OXNamesTab(const OXWindow *p, OSettings *settings) :
    OXHorizontalFrame(p, 10, 10, CHILD_FRAME) {
  OXListBox *lb;
  OXGroupFrame *gf;
  OXVerticalFrame *vf;
  OXButton *b;
  int width;

  _settings = settings;

  _lgf = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y, 5, 5, 5, 5);
  _lbt = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_TOP, 0, 0, 0, 2);
  _llb = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y, 7, 0, 7, 7);
  _lvf = new OLayoutHints(LHINTS_TOP | LHINTS_LEFT, 7, 10, 7, 0);

  gf = new OXGroupFrame(this, new OString("Nick names"));
  gf->OldInsets(False);
  gf->SetLayoutManager(new OHorizontalLayout(gf));
  lb = new OXListBox(gf, 1000);
  lb->AddEntry(new OString("foxirc"), 1);
  gf->AddFrame(lb, _llb);
  vf = new OXVerticalFrame(gf, 10, 10, VERTICAL_FRAME | FIXED_WIDTH);
  b = new OXTextButton(vf, new OHotString("Add..."), 1001);
  b->Associate(this);
  vf->AddFrame(b, _lbt);
  width = b->GetDefaultWidth();
  b = new OXTextButton(vf, new OHotString("Edit..."), 1002);
  b->Associate(this);
  b->Disable();
  vf->AddFrame(b, _lbt);
  width = max(width, b->GetDefaultWidth());
  b = new OXTextButton(vf, new OHotString("Remove"), 1003);
  b->Associate(this);
  b->Disable();
  vf->AddFrame(b, _lbt);
  width = max(width, b->GetDefaultWidth());
  vf->Resize(width+6, GetDefaultHeight());
  gf->AddFrame(vf, _lvf);
  AddFrame(gf, _lgf);

  gf = new OXGroupFrame(this, new OString("IRC names"));
  gf->OldInsets(False);
  gf->SetLayoutManager(new OHorizontalLayout(gf));
  lb = new OXListBox(gf, 1100);
  lb->AddEntry(new OString("fOX IRC user"), 1);
  gf->AddFrame(lb, _llb);
  vf = new OXVerticalFrame(gf, 10, 10, VERTICAL_FRAME | FIXED_WIDTH);
  b = new OXTextButton(vf, new OHotString("Add..."), 1101);
  b->Associate(this);
  vf->AddFrame(b, _lbt);
  width = b->GetDefaultWidth();
  b = new OXTextButton(vf, new OHotString("Edit..."), 1102);
  b->Associate(this);
  b->Disable();
  vf->AddFrame(b, _lbt);
  width = max(width, b->GetDefaultWidth());
  b = new OXTextButton(vf, new OHotString("Remove"), 1103);
  b->Associate(this);
  b->Disable();
  vf->AddFrame(b, _lbt);
  width = max(width, b->GetDefaultWidth());
  vf->Resize(width+6, GetDefaultHeight());
  gf->AddFrame(vf, _lvf);
  AddFrame(gf, _lgf);
}

OXNamesTab::~OXNamesTab() {
  delete _lgf;
  delete _lbt;
  delete _llb;
  delete _lvf;
}

int OXNamesTab::ProcessMessage(OMessage *msg) {
  return False;
}


//----------------------------------------------------------------------

OXServersTab::OXServersTab(const OXWindow *p, OSettings *settings) :
    OXHorizontalFrame(p, 10, 10, CHILD_FRAME) {
  OXVerticalFrame *vf;
  int width;

  _settings = settings;

  _lbt = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_TOP, 0, 0, 0, 2);
  _llb = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y, 7, 0, 7, 0);
  _lvf = new OLayoutHints(LHINTS_TOP | LHINTS_LEFT, 7, 7, 7, 0);

  lb = new OXListBox(this, 1000);
  _ReadList();
  AddFrame(lb, _llb);

  vf = new OXVerticalFrame(this, 10, 10, VERTICAL_FRAME | FIXED_WIDTH);
  _add = new OXTextButton(vf, new OHotString("Add..."), 1001);
  _add->Associate(this);
  vf->AddFrame(_add, _lbt);
  width = _add->GetDefaultWidth();

  _edit = new OXTextButton(vf, new OHotString("Edit..."), 1002);
  _edit->Associate(this);
  _edit->Disable();
  vf->AddFrame(_edit, _lbt);
  width = max(width, _edit->GetDefaultWidth());

  _delete = new OXTextButton(vf, new OHotString("Remove"), 1003);
  _delete->Associate(this);
  _delete->Disable();
  vf->AddFrame(_delete, _lbt);
  width = max(width, _delete->GetDefaultWidth());

  _up = new OXTextButton(vf, new OHotString("Move Up"), 1004);
  _up->Associate(this);
  _up->Disable();
  vf->AddFrame(_up, _lbt);
  width = max(width, _up->GetDefaultWidth());

  _down = new OXTextButton(vf, new OHotString("Move Down"), 1005);
  _down->Associate(this);
  _down->Disable();
  vf->AddFrame(_down, _lbt);
  width = max(width, _down->GetDefaultWidth());

  vf->Resize(width+6, GetDefaultHeight());

  AddFrame(vf, _lvf);
}

OXServersTab::~OXServersTab() {
  delete _lbt;
  delete _llb;
  delete _lvf;
}

void OXServersTab::_ReadList() {
  const OXSNode *ptr; int i;

  for (ptr = _settings->_servers->GetHead(), i = 1;
       ptr != NULL;
       ptr = ptr->next, ++i) {
    OServerInfo *e = (OServerInfo *) ptr->data;
    lb->AddEntry(new OString(e->name), i);
  }
}

void OXServersTab::_doAdd() {
  int retc;

  OString *title = new OString("Add Server");
  OServerInfo *info = new OServerInfo();

  info->port = 6667;

  new OXServerDlg(_client->GetRoot(), GetTopLevel(), title, info, &retc, False);
  if (retc == ID_OK) {
    int i = _settings->_servers->NoOfItems();
    _settings->_servers->Add(i+1, (XPointer) info);
    lb->AddEntry(new OString(info->name), i+1);

    OMessage msg(SETTINGS_CHANGED);
    SendMessage(GetTopLevel(), &msg);

  } else {
    delete info;
  }
}

void OXServersTab::_doEdit() {
  OXTextLBEntry *te = (OXTextLBEntry *) lb->GetSelectedEntry();
  if (!te) return;

  OServerInfo *o2 = _settings->FindServer(te->GetText()->GetString());
  if (!o2) return;

  int retc;

  OServerInfo *info = new OServerInfo();

  if (o2->name)     info->name     = StrDup(o2->name);
  if (o2->hostname) info->hostname = StrDup(o2->hostname);
  if (o2->passwd)   info->passwd   = StrDup(o2->passwd);
  if (o2->ircname)  info->ircname  = StrDup(o2->ircname);
  if (o2->nick)     info->nick     = StrDup(o2->nick);
  if (o2->opnick)   info->opnick   = StrDup(o2->opnick);
  if (o2->oppasswd) info->oppasswd = StrDup(o2->oppasswd);
  if (o2->logfile)  info->logfile  = StrDup(o2->logfile);
  if (o2->port)     info->port     = o2->port;

  OString *title = new OString("Edit Server");
  new OXServerDlg(_client->GetRoot(), GetTopLevel(), title, info, &retc, False);
  if (retc == ID_OK) {
    if (o2->name) delete[] o2->name;
    if (info->name) {
      o2->name = StrDup(info->name);
      te->SetText(new OString(info->name));
      lb->Layout();
    }

    if (o2->hostname)   delete[] o2->hostname;
    if (info->hostname) o2->hostname = StrDup(info->hostname);
    if (o2->passwd)     delete[] o2->passwd;
    if (info->passwd)   o2->passwd   = StrDup(info->passwd);
    if (o2->ircname)    delete[] o2->ircname;
    if (info->ircname)  o2->ircname  = StrDup(info->ircname);
    if (o2->nick)       delete[] o2->nick;
    if (info->nick)     o2->nick     = StrDup(info->nick);
    if (o2->opnick)     delete[] o2->opnick;
    if (info->opnick)   o2->opnick   = StrDup(info->opnick);
    if (o2->oppasswd)   delete[] o2->oppasswd;
    if (info->oppasswd) o2->oppasswd = StrDup(info->oppasswd);
    if (o2->logfile)    delete[] o2->logfile;
    if (info->logfile)  o2->logfile  = StrDup(info->logfile);
    if (info->port)     o2->port     = info->port;

    OMessage msg(SETTINGS_CHANGED);
    SendMessage(GetTopLevel(), &msg);

  } 

  delete info;
}

int OXServersTab::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;

  switch (msg->action) {
    case MSG_CLICK:

      switch (msg->type) {
        case MSG_LISTBOX:
          _edit->Enable();
          _delete->Enable();
//        _up->Enable();
//        _down->Enable();
          break;

        case MSG_BUTTON:
          switch (wmsg->id) {
            case 1001:      // Add...
              _doAdd();
              break;

            case 1002:      // Edit...
              _doEdit();
              break;

            case 1004:      // Move Up
              {
              int item;
              item = lb->GetSelectedEntry()->ID();
//              _settings->GetServerList()->SwapUp(item);
              lb->RemoveAllEntries();
              _ReadList();
              }
              break;

            case 1005:      // Move Down
              {
              int item;
              item = lb->GetSelectedEntry()->ID();
//              _settings->GetServerList()->SwapDown(item);
              lb->RemoveAllEntries();
              _ReadList();
              }
              break;

            case 1003:      // Delete
              {
              int item;
              item = lb->GetSelectedEntry()->ID();
              if (_settings->GetServerList()->Remove(item)){
                lb->RemoveEntry(item);
                OMessage msg(SETTINGS_CHANGED);
                SendMessage(GetTopLevel(), &msg);
                }
              }
              break;

            default:
              //SendMessage(new OMessage(SETTINGS_CHANGED));
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

  return False;
}


//----------------------------------------------------------------------

OXColorsTab::OXColorsTab(const OXWindow *p, OSettings *settings) :
  OXHorizontalFrame(p, 10, 10, CHILD_FRAME) {

    _settings = settings;

    OXCompositeFrame *x1 = new OXCompositeFrame(this, 10, 10);
    x1->SetLayoutManager(new O2ColumnsLayout(x1, 5, 5));
    OXCompositeFrame *x2 = new OXCompositeFrame(this, 10, 10);
    x2->SetLayoutManager(new O2ColumnsLayout(x2, 5, 5));

    AddFrame(x1, new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y,
                                  10, 5, 10, 5));
    AddFrame(x2, new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y,
                                  10, 5, 10, 5));

    int i = 0;
    OColorsPref **cp;
    OString *st;
    OXColorSelect *tr;

    cp = _settings->GetColorsList();
    OXCompositeFrame *xf;

    for (i = 0, xf = x1; i < NUM_COLORS; ++i) {
      st = new OString(cp[i]->name);
      tr = new OXColorSelect(xf, cp[i]->color, i + 4100, False);

      xf->AddFrame(new OXLabel(xf, st), NULL);
      xf->AddFrame(tr, NULL);
      tr->Associate(this);

      xf = (xf == x1) ? x2 : x1;
    }

    MapSubwindows();
    Layout();
    MapWindow();

}

int OXColorsTab::ProcessMessage(OMessage *msg) {
  OColorSelMessage *cmsg = (OColorSelMessage *) msg;

  switch (msg->type) {
    case MSG_COLORSEL:
      switch (msg->action) {
        case MSG_CLICK:
          {
          _settings->SetColor(cmsg->id-4100, cmsg->color);
          OMessage msg(SETTINGS_CHANGED);
          SendMessage(GetTopLevel(), &msg);
          }
          break;
      }
  }

  return True;
}

//----------------------------------------------------------------------

OXChannelTab::OXChannelTab(const OXWindow *p, OSettings *settings) :
    OXHorizontalFrame(p, 10, 10, CHILD_FRAME) {
  OXVerticalFrame *vf;
  int width;

  _settings = settings;

  _lbt = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_TOP, 0, 0, 0, 2);
  _llb = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y, 7, 0, 7, 0);
  _lvf = new OLayoutHints(LHINTS_TOP | LHINTS_LEFT, 7, 7, 7, 0);

  lb = new OXListBox(this, 1000);
  _ReadList();
  AddFrame(lb, _llb);

  vf = new OXVerticalFrame(this, 10, 10, VERTICAL_FRAME | FIXED_WIDTH);
  _add = new OXTextButton(vf, new OHotString("Add..."), 1001);
  _add->Associate(this);
  vf->AddFrame(_add, _lbt);
  width = _add->GetDefaultWidth();

  _edit = new OXTextButton(vf, new OHotString("Edit..."), 1002);
  _edit->Associate(this);
  _edit->Disable();
  vf->AddFrame(_edit, _lbt);
  width = max(width, _edit->GetDefaultWidth());

  _delete = new OXTextButton(vf, new OHotString("Remove"), 1003);
  _delete->Associate(this);
  _delete->Disable();
  vf->AddFrame(_delete, _lbt);
  width = max(width, _delete->GetDefaultWidth());

//  _up = new OXTextButton(vf, new OHotString("Move Up"), 1004);
//  _up->Associate(this);
//  _up->Disable();
//  vf->AddFrame(_up, _lbt);
//  width = max(width, _up->GetDefaultWidth());

//  _down = new OXTextButton(vf, new OHotString("Move Down"), 1005);
//  _down->Associate(this);
//  _down->Disable();
//  vf->AddFrame(_down, _lbt);
//  width = max(width, _down->GetDefaultWidth());

  vf->Resize(width+6, GetDefaultHeight());

  AddFrame(vf, _lvf);
}

OXChannelTab::~OXChannelTab() {
  delete _lbt;
  delete _llb;
  delete _lvf;
}

void OXChannelTab::_ReadList() {
  const OXSNode *ptr; int i;

  for (ptr = _settings->GetChannelList()->GetHead(), i = 1;
       ptr != NULL;
       ptr = ptr->next, ++i) {
    OChannelInfo *e = (OChannelInfo *) ptr->data;
    lb->AddEntry(new OString(e->name), i);
  }
}

int OXChannelTab::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;

  switch (msg->action) {
    case MSG_CLICK:
      switch (msg->type) {
        case MSG_LISTBOX:
          _edit->Enable();
          _delete->Enable();
          break;

        case MSG_BUTTON:
          switch (wmsg->id) {
            case 1001:      // Add...
              _DoAdd();
              break;
            case 1002:      // Edit...
              _DoEdit();
              break;

            case 1003:      // Delete
              {
              int item;
              item = lb->GetSelectedEntry()->ID();
              if (_settings->GetChannelList()->Remove(item)){
                lb->RemoveEntry(item);
                OMessage msg(SETTINGS_CHANGED);
                SendMessage(GetTopLevel(), &msg);
                }
              }
              break;

            default:
              //SendMessage(new OMessage(SETTINGS_CHANGED));
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

  return False;
}

void OXChannelTab::_DoAdd() {
  int r;
  OChannelInfo *ct = new OChannelInfo();

  new OXChannelEditor(_client->GetRoot(), GetTopLevel(), ct, &r);
  if (r == true) {
    int i = _settings->_channels->NoOfItems();
    _settings->_channels->Add(i+1, (XPointer) ct);
    lb->AddEntry(new OString(ct->name), i+1);
    OMessage msg(SETTINGS_CHANGED);
    SendMessage(GetTopLevel(), &msg);
  } else {
    delete ct;        
  }
}

void OXChannelTab::_DoEdit(){
  OXTextLBEntry *te = (OXTextLBEntry *)lb->GetSelectedEntry();
  if (!te) return;

  OChannelInfo *o2 = _settings->FindChannel(te->GetText()->GetString());
  if (!o2) return;

  int retc;

  OChannelInfo *info = new OChannelInfo();

  if (o2->name)       info->name       = StrDup(o2->name);
  if (o2->logfile)    info->logfile    = StrDup(o2->logfile);
  if (o2->background) info->background = StrDup(o2->background);
  info->flags  = o2->flags;

  new OXChannelEditor(_client->GetRoot(),
                      GetTopLevel(),info,&retc);
  if (retc) {
    if (o2->name) delete[] o2->name;
    if (info->name)
      o2->name = StrDup(info->name);
    else
      o2->name = 0;

    if (o2->logfile) delete[] o2->logfile;
    if (info->logfile)
      o2->logfile = StrDup(info->logfile);
    else
      o2->logfile = 0;

    if (o2->background) delete[] o2->background;
    if (info->background)
      o2->background = StrDup(info->background);
    else
      o2->background = 0;
  
    o2->flags = info->flags;

    OMessage msg(SETTINGS_CHANGED);
    SendMessage(GetTopLevel(), &msg);

  }

  delete info;
}


//----------------------------------------------------------------------

OXInfoTab::OXInfoTab(const OXWindow *p, OSettings *settings) :
  OXHorizontalFrame(p, 10, 10, CHILD_FRAME) {

  _settings = settings;

  _lcb = new OLayoutHints(LHINTS_TOP | LHINTS_LEFT, 15, 2, 2, 2);
  _lgf = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y, 5, 5, 5, 5);

  AddFrame(InitButtons("Send to Info", _settings->_sendToInfo, stinfo), _lgf);
}

OXInfoTab::~OXInfoTab() {
  delete _lcb;
  delete _lgf;
}

OXFrame *OXInfoTab::InitButtons(char *gname, int ctlvar,
                                struct _ButtonDef b[]) {
  int i;
  OXButton *button;

  OXGroupFrame *gf = new OXGroupFrame(this, new OString(gname));
  gf->OldInsets(False);
  for (i = 0; b[i].name != NULL; ++i) {
    button = new OXCheckButton(gf, new OHotString(b[i].name), b[i].id);
    if (ctlvar & b[i].mask) button->SetState(BUTTON_DOWN);
    b[i].button = button;
    button->Associate(this);
    gf->AddFrame(button, _lcb);
  }

  return gf;
}

int OXInfoTab::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;
  int n;

  switch (msg->type) {
    case MSG_CHECKBUTTON:

      switch (msg->action) {
        case MSG_CLICK:
          {
          n = wmsg->id;

          if (n > 1100) {
            if (stinfo[n-1101].button->GetState() == BUTTON_DOWN)
              _settings->_sendToInfo |= stinfo[n-1101].mask;
            else
              _settings->_sendToInfo &= ~stinfo[n-1101].mask;
            _settings->_changed = True;
          }

          OMessage msg(SETTINGS_CHANGED);
          SendMessage(GetTopLevel(), &msg);
          }
          break;

        default:
          break;
      }
      break;

    default:
      break;
  }

  return False;
}
