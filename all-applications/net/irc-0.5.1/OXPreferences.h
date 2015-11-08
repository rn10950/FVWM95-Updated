/**************************************************************************

    This file is part of foxirc, a cool irc client.
    Copyright (C) 1996, 1997 R. Suescun, Hector Peraza.

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

#ifndef __OXPREFERENCES_H
#define __OXPREFERENCES_H


#include <xclass/utils.h>
#include <xclass/OXClient.h>
#include <xclass/OXWindow.h>
#include <xclass/OXTransientFrame.h>
#include <xclass/OXTab.h>
#include <xclass/OXButton.h>
#include <xclass/OXListBox.h>
#include <xclass/OString.h>
#include <xclass/OPicture.h>
#include <xclass/OColor.h>
#include <xclass/OXFont.h>
#include <xclass/OXSList.h>


//----------------------------------------------------------------------

// This class contains all the user-changeable settings in fOXIrc
// Perhaps most of this settings should be implemented as member
// variables (properties) of the corresponding classes instead,
// but keeping them here makes it easier to load and save them.

#define P_CONFIRM_QUIT        (1<<0)
#define P_CONFIRM_LEAVE       (1<<1)
#define P_CONFIRM_SAVE        (1<<2)
#define P_CONFIRM_SQUIT       (1<<3)
#define P_CONFIRM_KILL        (1<<4)

#define P_LOG_CTCP            (1<<0)
#define P_LOG_SIGNOFF         (1<<1)
#define P_LOG_WHO             (1<<2)
#define P_LOG_WHOIS           (1<<3)
#define P_LOG_WHOWAS          (1<<4)
#define P_LOG_ERROR           (1<<5)
#define P_LOG_ISON            (1<<6)
#define P_LOG_KILL            (1<<7)
#define P_LOG_CLOSE           (1<<8)
#define P_LOG_CHAN_LIST       (1<<9)
#define P_LOG_LINKS           (1<<10)

#define P_MISC_ENABLE_CMD     (1<<0)
#define P_MISC_NOTIFY_ALL     (1<<1)
#define P_MISC_AUTO_REJOIN    (1<<2)
#define P_MISC_SHOW_OPS       (1<<3)
#define P_MISC_SHOW_FRIENDS   (1<<4)
#define P_MISC_SHOW_VOICED    (1<<5)
#define P_MISC_POPUP_WINDOW   (1<<6)
#define P_MISC_POPUP_SERV_CN  (1<<7)
#define P_MISC_POPUP_CHAN_CN  (1<<8)
#define P_MISC_TRANSIENT_CHW  (1<<9)

#define P_CHAN_AUTO_RAISE     (1<<0)
#define P_CHAN_AUTO_LOG       (1<<1)


class OServerInfo {
public:
  OServerInfo() : name(0), hostname(0), passwd(0), ircname(0),
                  nick(0), opnick(0), oppasswd(0), 
                  logfile(0), port(6667) {};
  ~OServerInfo();

public:
  char *name, *hostname, *passwd, *ircname, *nick, *opnick, *oppasswd;
  char *logfile;
  int  port;
};

class ONameInfo {
  char *name;
};

#define P_COLOR_BACKGROUND   0
#define P_COLOR_ACTION       1
#define P_COLOR_CTCP         2
#define P_COLOR_HIGHLIGHT    3
#define P_COLOR_SERVER_1     4
#define P_COLOR_SERVER_2     5
#define P_COLOR_INVITE       6
#define P_COLOR_JOIN         7
#define P_COLOR_KICK         8
#define P_COLOR_MODE         9
#define P_COLOR_NICK        10
#define P_COLOR_TEXT        11
#define P_COLOR_NOTICE      12
#define P_COLOR_NOTIFY      13
#define P_COLOR_OTHER       14
#define P_COLOR_OWN_TEXT    15
#define P_COLOR_PART        16
#define P_COLOR_QUIT        17
#define P_COLOR_TOPIC       18
#define P_COLOR_WALLOPS     19

class OColorsPref {
public:
  OColorsPref(const char *n, int r, int g, int b);
  ~OColorsPref();

public:
  char *name;
  OColor color;
  unsigned long pixel;
};


//typedef std::vector<ONameInfo *> ONameList;

#define TRANSIENT_WINDOW 	(1<<0)
#define USES_BACKGROUND_PIX 	(1<<1)
#define AUTO_RAISE_WINDOW	(1<<2) // raise,flash,beep are
#define AUTO_FLASH_WINDOW	(1<<3) // for auto notice in channels
#define AUTO_BEEP_WINDOW	(1<<4)
#define AUTO_LOG		(1<<5)

class OChannelInfo {
public:
  OChannelInfo() : name(0), logfile(0), background(0), flags(0) {};
  ~OChannelInfo() { if (name) delete[] name;
                    if (logfile) delete[] logfile;
		    if (background) delete[] background; }

  char *name;
  char *logfile;
  char *background;
  int   flags;
};

//typedef std::vector<OChannelInfo *> OChannelList;

class OSettings : public OBaseObject {
public:
  OSettings(OXClient *c);
  virtual ~OSettings();

  int Load();
  int Save();

//  void AddServer(char *server, int port, char *psw, char *oppsw,
//                 char *defnick, char *opnick);
//  void RemoveServer(char *server);
//  void AddNick(char *nick);
//  void RemoveNick(char *nick);
//  void AddName(char *name);
//  void RemoveName(char *name);

  OServerInfo *FindServer(const char *name);

  OChannelInfo *FindChannel(const char *name);
  int CheckChannelFlags(const char *name, int what);

  OXSList *GetServerList()  { return _servers; }
  OXSList *GetChannelList() { return _channels; }
  OXSList *GetNamesList()   { return _names; }
  OXSList *GetNickList()    { return _nicks; }
  OColorsPref **GetColorsList() { return _colors; }

  void SetFont(const char *name);
  const OXFont *GetFont() { return _font; }

  OColor GetColorID(int id) const { return _colors[id]->color; }
  unsigned long GetPixelID(int id) const { return _colors[id]->pixel; }
  void SetColor(int id, OColor color);

  OColor GetIrcColor(int num) const { return _ircColors[num]->color; }
  unsigned long GetIrcPixel(int num) const { return _ircColors[num]->pixel; }

  int Confirm(int what) const { return (_confirm & what); }
  int SendToInfo(int what) const { return (_sendToInfo & what); }
  int CheckMisc(int what) const { return (_misc & what); }

  int Changed() const { return _changed; }

  friend class OXPreferencesDialog;
  friend class OXIrcTab;
  friend class OXNamesTab;
  friend class OXServersTab;
  friend class OXChannelTab;
  friend class OXInfoTab;

protected:
  OXSList *_servers;
  OXSList *_nicks;
  OXSList *_names;
  OXSList *_channels;

  OColorsPref *_colors[20];
  OColorsPref *_ircColors[16];

  OXFont *_font;

  int _confirm;
  int _sendToInfo;
  int _misc;
  int _changed;
  
  OXClient *_client;
};


//----------------------------------------------------------------------

class OXPreferencesDialog : public OXTransientFrame {
public:
  OXPreferencesDialog(const OXWindow *p, const OXWindow *main,
                      OSettings *settings,
                      unsigned long options = MAIN_FRAME | VERTICAL_FRAME);
  virtual ~OXPreferencesDialog();

  virtual int ProcessMessage(OMessage *msg);

protected:
  void UpdateListBox();

  OXButton *Ok, *Cancel, *Apply;
  OXHorizontalFrame *bframe;
  OLayoutHints *bly, *bfly;

  OXTab *tab;
  OLayoutHints *Ltab;

  OSettings *_settings;
};


//----------------------------------------------------------------------

struct _ButtonDef {
  char *name;
  int  id, mask;
  OXButton *button;
};

class OXIrcTab : public OXHorizontalFrame {
public:
  OXIrcTab(const OXWindow *p, OSettings *settings);
  virtual ~OXIrcTab();

  OXFrame *InitButtons(char *gname, int ctlvar, struct _ButtonDef b[]);
  virtual int ProcessMessage(OMessage *msg);

protected:
  OLayoutHints *_lcb, *_lgf;
  OSettings *_settings;
};


//----------------------------------------------------------------------

class OXNamesTab : public OXHorizontalFrame {
public:
  OXNamesTab(const OXWindow *p, OSettings *settings);
  virtual ~OXNamesTab();

  virtual int ProcessMessage(OMessage *msg);

protected:
  OLayoutHints *_lgf, *_lbt, *_llb, *_lvf;
  OSettings *_settings;
};


//----------------------------------------------------------------------

class OXServersTab : public OXHorizontalFrame {
public:
  OXServersTab(const OXWindow *p, OSettings *settings);
  virtual ~OXServersTab();

  virtual int ProcessMessage(OMessage *msg);

protected:
  void _doAdd();
  void _doEdit();
  void _ReadList();

  OXButton *_add, *_edit, *_delete, *_up, *_down;
  OXListBox *lb;
  OLayoutHints *_lbt, *_llb, *_lvf;
  OSettings *_settings;
};

//----------------------------------------------------------------------

class OXColorsTab : public OXHorizontalFrame {
public:
  OXColorsTab(const OXWindow *p, OSettings *settings);
//  virtual ~OXNamesTab();

  virtual int ProcessMessage(OMessage *msg);

protected:
//  OLayoutHints *_lgf, *_lbt, *_llb, *_lvf;
  OSettings *_settings;
};

//----------------------------------------------------------------------

class OXChannelTab : public OXHorizontalFrame {
public:
  OXChannelTab(const OXWindow *p, OSettings *settings);
  virtual ~OXChannelTab();

  virtual int ProcessMessage(OMessage *msg);

protected:
  void _DoAdd();
  void _DoEdit();
  void _ReadList();

  OXButton *_add, *_edit, *_delete; //, *_up, *_down;
  OXListBox *lb;
  OLayoutHints *_lbt, *_llb, *_lvf;
  OSettings *_settings;
};

//----------------------------------------------------------------------

class OXInfoTab : public OXHorizontalFrame {
public:
  OXInfoTab(const OXWindow *p, OSettings *settings);
  virtual ~OXInfoTab();

  OXFrame *InitButtons(char *gname, int ctlvar, struct _ButtonDef b[]);
  virtual int ProcessMessage(OMessage *msg);

protected:
  OLayoutHints *_lcb, *_lgf;
  OSettings *_settings;
};


#endif   // __OXPREFERENCES_H
