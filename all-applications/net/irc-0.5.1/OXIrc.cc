#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <X11/keysym.h>

#include <xclass/utils.h>
#include <xclass/version.h>
#include <xclass/OXFontDialog.h>

#include "IRCcodes.h"
#include "OIrcMessage.h"
#include "OXChannel.h"
#include "OXChatChannel.h"
#include "OXMsgChannel.h"
#include "OXDCCChannel.h"
#include "OXDCCFile.h"
#include "OXPreferences.h"
#include "OXIrc.h"
#include "OXServerDlg.h"
#include "OXConfirmDlg.h"
#include "OXCommandDlg.h"
#include "OXPasswdDlg.h"
#include "OXChannelDialog.h"
#include "OXChannelList.h"
#include "OXServerTree.h"
#include "OXViewLogFile.h"

#include "versiondef.h"

#include "pixmaps/tb-open.xpm"
#include "pixmaps/tb-save.xpm"
#include "pixmaps/tb-print.xpm"
#include "pixmaps/tb-conn.xpm"
#include "pixmaps/tb-dconn.xpm"
#include "pixmaps/tb-join.xpm"
#include "pixmaps/tb-invisible.xpm"
#include "pixmaps/tb-wallops.xpm"
#include "pixmaps/tb-srvmsgs.xpm"
#include "pixmaps/tb-ircop.xpm"
#include "pixmaps/tb-setup.xpm"


//----------------------------------------------------------------------

// Menu and toolbar definitions

#define M_IRC_CONNECT     1001
#define M_IRC_DISCONNECT  1002
#define M_IRC_AWAY        1003
#define M_IRC_EXIT        1004

#define M_LOG_OPEN        2000
#define M_LOG_FLUSH       2001  // not used anymore
#define M_LOG_PRINT       2002
#define M_LOG_CLOSE       2003
#define M_LOG_EMPTY       2004
#define M_LOG_VIEW        2005

#define M_SERV_RAW        3001
#define M_SERV_NICK       3002
#define M_SERV_WALLOPS    3003
#define M_SERV_LINKS      3004
#define M_SERV_MAP        3005
#define M_SERV_VERSION    3006
#define M_SERV_MOTD       3007
#define M_SERV_TIME       3008
#define M_SERV_TRACE      3009
#define M_SERV_ADMIN      3010
#define M_SERV_LUSERS     3011
#define M_SERV_INFO       3012
#define M_SERV_STATS      3013
#define M_SERV_OPER       3014
#define M_SERV_REHASH     3015
#define M_SERV_RESTART    3016
#define M_SERV_SQUIT      3017

#define M_USERS_WHO       4001
#define M_USERS_WHOIS     4002
#define M_USERS_WHOWAS    4003
#define M_USERS_MODE      4004
#define M_USERS_CTCP      4005
#define M_USERS_DCC       4006
#define M_USERS_INVITE    4007
#define M_USERS_MSG       4008
#define M_USERS_NOTICE    4009
#define M_USERS_TIME      4010
#define M_USERS_TRACE     4011
#define M_USERS_UHOST     4012
#define M_USERS_KILL      4013

#define M_CHAN_JOIN       5001
#define M_CHAN_WHO        5002
#define M_CHAN_LIST       5003
#define M_CHAN_NAMES      5004
#define M_CHAN_NOTICE     5005
#define M_CHAN_MONITOR    5006
#define M_CHAN_CTCP       5007

#define M_EDIT_COPY       6001
#define M_EDIT_SELECTALL  6002
#define M_EDIT_INVERTSEL  6003
#define M_EDIT_PREFS      6004

#define M_VIEW_TOOLBAR    7001
#define M_VIEW_STATUSBAR  7002
#define M_VIEW_FONT       7003
#define M_VIEW_COLORS     7004

#define M_HELP_INDEX      8001
#define M_HELP_ABOUT      8002

#define TB_INVISIBLE      9001
#define TB_WALLOPS        9002
#define TB_SNOTICES       9003
#define TB_IRCOP          9004


SToolBarData tdata[] = {
  { "tb-open.xpm",      tb_open,      "View Log",                 BUTTON_NORMAL, M_LOG_VIEW,       NULL },
  { "tb-save.xpm",      tb_save,      "Save Log",                 BUTTON_NORMAL, M_LOG_OPEN,       NULL },
  { "tb-print.xpm",     tb_print,     "Print Log",                BUTTON_NORMAL, M_LOG_PRINT,      NULL },
  { "",                 NULL,         0,                          0,             -1,               NULL },
  { "tb-conn.xpm",      tb_conn,      "Connect to Server",        BUTTON_NORMAL, M_IRC_CONNECT,    NULL },
  { "tb-dconn.xpm",     tb_dconn,     "Close connection",         BUTTON_NORMAL, M_IRC_DISCONNECT, NULL },
  { "",                 NULL,         0,                          0,             -1,               NULL },
  { "tb-join.xpm",      tb_join,      "Join channel...",          BUTTON_NORMAL, M_CHAN_JOIN,      NULL },
  { "",                 NULL,         0,                          0,             -1,               NULL },
  { "tb-invisible.xpm", tb_invisible, "Toggle Invisible mode",    BUTTON_ONOFF,  TB_INVISIBLE,     NULL },
  { "tb-wallops.xpm",   tb_wallops,   "Toggle Wallops mode",      BUTTON_ONOFF,  TB_WALLOPS,       NULL },
  { "tb-srvmsgs.xpm",   tb_srvmsgs,   "Toggle Receive of Server Notices", BUTTON_ONOFF, TB_SNOTICES, NULL },
  { "tb-ircop.xpm",     tb_ircop,     "Toggle IRC Operator Mode", BUTTON_ONOFF,  TB_IRCOP,         NULL },
  { "",                 NULL,         0,                          0,             -1,               NULL },
  { "tb-setup.xpm",     tb_setup,     "Setup Preferences",        BUTTON_NORMAL, M_EDIT_PREFS,     NULL },
  { NULL,               NULL,         NULL,                       0,             0,                NULL }
};

SPopupData plogdata = {
  NULL, {
  { "&View...",    M_LOG_VIEW,  0,             NULL },
  { "",            -1,          -1,            NULL },
  { "&Save as...", M_LOG_OPEN,  0,             NULL },
  { "&Close",      M_LOG_CLOSE, MENU_DISABLED, NULL },
  { "",            -1,          -1,            NULL },
  { "&Empty",      M_LOG_EMPTY, MENU_DISABLED, NULL },
  { NULL,          -1,          -1,            NULL } }
};


SPopupData pservdata = {
  NULL, {
  { "&Raw...",      M_SERV_RAW,     0,             NULL },
  { "",             -1,             0,             NULL },
  { "&Nick...",     M_SERV_NICK,    0,             NULL },
  { "&Wallops...",  M_SERV_WALLOPS, 0,             NULL },
  { "&Links...",    M_SERV_LINKS,   0,             NULL },
  { "M&ap...",      M_SERV_MAP,     0,             NULL },
  { "&Version...",  M_SERV_VERSION, 0,             NULL },
  { "&Motd...",     M_SERV_MOTD,    0,             NULL },
  { "&Time...",     M_SERV_TIME,    0,             NULL },
  { "Tra&ce...",    M_SERV_TRACE,   0,             NULL },
  { "A&dmin...",    M_SERV_ADMIN,   0,             NULL },
  { "L&users...",   M_SERV_LUSERS,  0,             NULL },
  { "&Info...",     M_SERV_INFO,    0,             NULL },
  { "&Stats...",    M_SERV_STATS,   0,             NULL },
  { "&Oper...",     M_SERV_OPER,    0,             NULL },
  { "Re&hash",      M_SERV_REHASH,  MENU_DISABLED, NULL },
  { "R&estart",     M_SERV_RESTART, MENU_DISABLED, NULL },
  { "S&quit",       M_SERV_SQUIT,   MENU_DISABLED, NULL },
  { NULL,           -1,             -1,            NULL } }
};

SPopupData pusersdata = {
  NULL, {
  { "W&ho...",      M_USERS_WHO,    0,  NULL },
  { "&Whois...",    M_USERS_WHOIS,  0,  NULL },
  { "Whow&as...",   M_USERS_WHOWAS, 0,  NULL },
  { "&Mode...",     M_USERS_MODE,   0,  NULL },
  { "&CTCP...",     M_USERS_CTCP,   0,  NULL },
  { "&DCC...",      M_USERS_DCC,    0,  NULL },
  { "&Invite...",   M_USERS_INVITE, 0,  NULL },
  { "M&essage...",  M_USERS_MSG,    0,  NULL },
  { "&Notice...",   M_USERS_NOTICE, 0,  NULL },
  { "&Time...",     M_USERS_TIME,   0,  NULL },
  { "T&race...",    M_USERS_TRACE,  0,  NULL },
  { "&Userhost...", M_USERS_UHOST,  0,  NULL },
  { "&Kill...",     M_USERS_KILL,   0,  NULL },
  { NULL,           -1,             -1, NULL } }
};

SPopupData pchandata = {
  NULL, {
  { "&Join...",        M_CHAN_JOIN,       0,  NULL },
  { "",                -1,                0,  NULL },
  { "&Who...",         M_CHAN_WHO,        0,  NULL },
  { "&List...",        M_CHAN_LIST,       0,  NULL },
  { "&Names...",       M_CHAN_NAMES,      0,  NULL },
  { "N&otice...",      M_CHAN_NOTICE,     0,  NULL },
  { "&Monitor...",     M_CHAN_MONITOR,    0,  NULL },
  { "&CTCP...",        M_CHAN_CTCP,       0,  NULL },
  { NULL,              -1,                -1, NULL } }
};

SPopupData mircdata = {
  NULL, {
  { "&Connect to...", M_IRC_CONNECT,    0,              NULL },
  { "&Disconnect",    M_IRC_DISCONNECT, MENU_DISABLED,  NULL },
  { "&Away",          M_IRC_AWAY,       MENU_DISABLED,  NULL },
  { "",               -1,               0,              NULL },
  { "&Log",           -1,               0,              &plogdata   },
  { "&Servers",       -1,               0,              &pservdata  },
  { "&Users",         -1,               0,              &pusersdata },
  { "C&hannels",      -1,               0,              &pchandata  },
  { "",               -1,               0,              NULL },
  { "E&xit",          M_IRC_EXIT,       0,              NULL },
  { NULL,             -1,               -1,             NULL } }
};

SPopupData meditdata = {
  NULL, {
  { "&Copy",             M_EDIT_COPY,      MENU_DISABLED, NULL },
  { "",                  -1,               0,             NULL },
  { "Select &All",       M_EDIT_SELECTALL, MENU_DISABLED, NULL },
  { "&Invert Selection", M_EDIT_INVERTSEL, MENU_DISABLED, NULL },
  { "",                  -1,               0,             NULL },
  { "&Preferences...",   M_EDIT_PREFS,     0,             NULL },
  { NULL,                -1,               -1,            NULL } }
};

SPopupData mviewdata = {
  NULL, {
  { "&Toolbar",    M_VIEW_TOOLBAR,   MENU_CHECKED, NULL },
  { "Status &Bar", M_VIEW_STATUSBAR, MENU_CHECKED, NULL },
  { "",            -1,               0,            NULL },
  { "&Font...",    M_VIEW_FONT,      0,            NULL },
  { "&Colors...",  M_VIEW_COLORS,    0,            NULL },
  { NULL,          -1,               -1,           NULL } }
};

SPopupData mhelpdata = {
  NULL, {
  { "&Index...",    M_HELP_INDEX,    0,            NULL },
  { "",             -1,              0,            NULL },
  { "&About...",    M_HELP_ABOUT,    0,            NULL },
  { NULL,           -1,              -1,           NULL } }
};


//----------------------------------------------------------------------

OLayoutHints *leftexpandylayout =
    new OLayoutHints(LHINTS_LEFT | LHINTS_EXPAND_Y, 3, 0, 0, 0);

OLayoutHints *leftcenterylayout =
    new OLayoutHints(LHINTS_LEFT | LHINTS_CENTER_Y);

OLayoutHints *topleftlayout =
    new OLayoutHints(LHINTS_TOP | LHINTS_LEFT, 1, 1, 1, 1);

OLayoutHints *topexpandxlayout =
    new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 0, 0, 0, 3);

OLayoutHints *topexpandxlayout1 =
    new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 0, 0, 3, 0);

OLayoutHints *topexpandxlayout2 =
    new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 0, 0, 3, 3);

OLayoutHints *topexpandxlayout3 =
    new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X);

OLayoutHints *topexpandxlayout0 =
    new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 1, 1, 0, 0);

OLayoutHints *expandxexpandylayout =
    new OLayoutHints(LHINTS_EXPAND_Y | LHINTS_EXPAND_X);

OLayoutHints *menubarlayout =
    new OLayoutHints(LHINTS_TOP | LHINTS_LEFT | LHINTS_EXPAND_X, 0, 0, 1, 1);

OLayoutHints *menuitemlayout =
    new OLayoutHints(LHINTS_TOP | LHINTS_LEFT, 0, 4, 0, 0);

extern OSettings  *foxircSettings;

char *filetypes[] = { "All files",  "*",
                      "Log files",  "*.log",
                      "Text files", "*.txt",
                      NULL,         NULL };

#define PING_INTERVAL  180


//----------------------------------------------------------------------

OXIrc::OXIrc(const OXWindow *p, char *nick, char *server, int port) :
  OXMainFrame(p, 400, 300) {

  _channels = new OXSList(GetDisplay(), "Channels");
  _transfers = new OXSList(GetDisplay(), "Transfers");
  _channelList = NULL;
  _serverTree = NULL;
  _viewLogFile = NULL;
  _logfile = NULL;
  _logfilename = NULL;
  _fh = NULL;
  _init = False;
  _helpWindow = NULL;

  _avcmode = 0x00444100L; // 0L;
  _avumode = 0x00088100L; // 0L;

  _umode = 0L;
  _lag = 0;

  SetLayoutManager(new OVerticalLayout(this));

  //---- menu bar

  _menubar = new OXMenuBar(this, 10, 10, HORIZONTAL_FRAME);
  AddFrame(_menubar, menubarlayout);

  _pserv  = MakePopup(&pservdata);
  _pusers = MakePopup(&pusersdata);
  _pchan  = MakePopup(&pchandata);
  _plog   = MakePopup(&plogdata);

  _mirc  = MakePopup(&mircdata);
  _medit = MakePopup(&meditdata);
  _mview = MakePopup(&mviewdata);
  _mhelp = MakePopup(&mhelpdata);

  _menubar->AddPopup(new OHotString("&IRC"),  _mirc,  menuitemlayout);
  _menubar->AddPopup(new OHotString("&Edit"), _medit, menuitemlayout);
  _menubar->AddPopup(new OHotString("&View"), _mview, menuitemlayout);
  _menubar->AddPopup(new OHotString("&Help"), _mhelp, menuitemlayout);

  //---- toolbar

  _toolBarSep = new OXHorizontal3dLine(this);

  _toolBar = new OXToolBar(this, 60, 20, HORIZONTAL_FRAME);
  _toolBar->AddButtons(tdata);

  AddFrame(_toolBarSep, topexpandxlayout3);
  AddFrame(_toolBar, topexpandxlayout2);

  //---- join channel entry

  OXHorizontalFrame *hf = new OXHorizontalFrame(this);
  _channelentry = new OXTextEntry(hf, new OTextBuffer(100));
  _channelentry->ChangeOptions(FIXED_WIDTH | SUNKEN_FRAME | DOUBLE_BORDER);
  hf->AddFrame(new OXLabel(hf, new OString("Join:")),
               new OLayoutHints(LHINTS_LEFT | LHINTS_CENTER_Y, 0, 5, 0, 0));
  hf->AddFrame(_channelentry, topexpandxlayout3);
  AddFrame(hf, topexpandxlayout);

  _channelentry->Associate(this);

  //---- log window

  _logw = new OXViewDoc(this, _log = new OTextDoc(), 10, 10,
                              SUNKEN_FRAME | DOUBLE_BORDER);
  _logw->SetScrollOptions(FORCE_VSCROLL | NO_HSCROLL);
  AddFrame(_logw, expandxexpandylayout);

  //------ status bar

  _statusBar = new OXStatusBar(this);
  AddFrame(_statusBar, new OLayoutHints(LHINTS_BOTTOM | LHINTS_EXPAND_X,
                                        0, 0, 3, 0));


  _mview->CheckEntry(M_VIEW_TOOLBAR);
  _mview->CheckEntry(M_VIEW_STATUSBAR);

  SetWindowName(FOXIRC_NAME);
  SetClassHints("fOXIrc", "foxirc");

  MapSubwindows();
//  Resize(512, 320);
  Resize(600, 380);

  if (getenv("USER"))
    _username = StrDup(getenv("USER"));
  else if (getenv("LOGNAME"))
    _username = StrDup(getenv("LOGNAME"));
  else
    _username = StrDup("unknown");

  if (nick && *nick)
    _nick = StrDup(nick);
  else
    _nick = StrDup(_username);

  if (getenv("HOSTNAME"))
    _hostname = StrDup(getenv("HOSTNAME"));
  else
    _hostname = StrDup("unknown.host");

  if (server)
    _server = StrDup(server);
  else
    _server = StrDup("");

  _realserver = StrDup(_server);

  if (getenv("LOGNAME"))
    _ircname = StrDup(getenv("LOGNAME"));
  else
    _ircname = StrDup(_username);

  _passwd = NULL;

  _port = port;

  _irc = new OIrc();
  _irc->Associate(this);

  _connected = False;
  _pingTimer = NULL;
  _pingTime = (time_t) 0;

  _statusBar->SetWidth(0, 300);
  _statusBar->AddLabel(100, LHINTS_RIGHT);
  _statusBar->AddLabel(500);

  _statusBar->SetText(0, new OString("Not connected."));
  _statusBar->SetText(1, new OString(""));
  _statusBar->SetText(2, new OString(nick));

  int keycode;

  keycode = XKeysymToKeycode(GetDisplay(), XK_F1);
  XGrabKey(GetDisplay(), keycode, AnyModifier, _id, True,
           GrabModeAsync, GrabModeAsync);

}

OXIrc::~OXIrc() {
  if (_channelList) _channelList->CloseWindow();
  if (_viewLogFile) _viewLogFile->CloseWindow();
  if (_serverTree) _serverTree->CloseWindow();
  if (_helpWindow) _helpWindow->CloseWindow();

  _irc->Close();
  if (_logfile) DoCloseLog();

  if (_nick) delete[] _nick;
  if (_ircname) delete[] _ircname;
  if (_username) delete[] _username;
  if (_hostname) delete[] _hostname;
  if (_server) delete[] _server;
  if (_realserver) delete[] _realserver;
  if (_passwd) delete[] _passwd;

  if (_pingTimer) delete _pingTimer;
}

int OXIrc::CloseWindow() {

  if (_connected) {
    OString *lmsg = new OString("QUIT");

    if (foxircSettings->Confirm(P_CONFIRM_QUIT)) {
      int retc;
      OString *msg = new OString(FOXIRC_NAME);
               msg->Append(" ");
               msg->Append(FOXIRC_HOMEPAGE);
      OString *title = new OString("Quit IRC?");
      OString *label = new OString("Message:");
      OString *text = new OString("Really Quit fOXirc?");

      new OXConfirmDlg(_client->GetRoot(), this, title, text, label,
                       msg, &retc);
      if (retc == ID_NO) return False;
      if (msg->GetLength() > 0) {
        lmsg->Append(" :");
        lmsg->Append(msg);
      }
    } else {
      lmsg->Append(" :");
      lmsg->Append(FOXIRC_NAME);
      lmsg->Append(" ");
      lmsg->Append(FOXIRC_HOMEPAGE);
    }

    SendRawCommand(lmsg->GetString());
    // sleep(2); or flush?
    Disconnect();
  }

  const OXSNode *ptr;
  while ((ptr = _channels->GetHead()) != NULL) {
    OXChannel *ch = (OXChannel *) ptr->data;
    ch->CloseWindow();
  }
  while ((ptr = _transfers->GetHead()) != NULL) {
    OXDCCFile *dcc = (OXDCCFile *) ptr->data;
    dcc->CloseWindow();
  }

  return OXMainFrame::CloseWindow();
}

OXPopupMenu *OXIrc::MakePopup(SPopupData *p) {

  OXPopupMenu *popup = new OXPopupMenu(_client->GetRoot());

  for (int i = 0; p->popup[i].name != NULL; ++i) {
    if (strlen(p->popup[i].name) == 0) {
      popup->AddSeparator();
    } else {
      if (p->popup[i].popup_ref == NULL) {
        popup->AddEntry(new OHotString(p->popup[i].name), p->popup[i].id);
      } else {
        SPopupData *p1 = p->popup[i].popup_ref;
        popup->AddPopup(new OHotString(p->popup[i].name), p1->ptr);
      }
      if (p->popup[i].state & MENU_DISABLED)
        popup->DisableEntry(p->popup[i].id);
      if (p->popup[i].state & MENU_CHECKED)
        popup->CheckEntry(p->popup[i].id);
      if (p->popup[i].state & MENU_RCHECKED)
        popup->RCheckEntry(p->popup[i].id, p->popup[i].id, p->popup[i].id);
    }
  }
  p->ptr = popup;
  popup->Associate(this);

  return popup;
}

//----------------------------------------------------------------------

void OXIrc::UpdateStatusBar(int field) {
  char tmp[256];

  // connection status
  if (field == 0 || field < 0) {
    if (_connected) {
      snprintf(tmp, 256, "Connected to %s:%d", _realserver, _port);
      _statusBar->SetText(0, new OString(tmp));
    } else {
      _statusBar->SetText(0, new OString("Not connected."));
    }
  }

  // lag time
  if (field == 1 || field < 0) {
    if (_connected) {
      snprintf(tmp, 256, "Lag: %ld sec", _lag);
      _statusBar->SetText(1, new OString(tmp));
    } else {
      _statusBar->SetText(1, new OString(""));
    }
  }

  // nick
  if (field == 2 || field < 0) {
    if (_connected) {
      strcpy(tmp, _nick);
      if (_umode != 0L) {
        strcat(tmp, " (");
        if (_umode & UMODE_INVISIBLE) strcat(tmp, "Invisible, ");
        if (_umode & UMODE_SNOTICES)  strcat(tmp, "SrvMsgs, ");
        if (_umode & UMODE_WALLOPS)   strcat(tmp, "Wallops, ");
        if (_umode & UMODE_IRCOP)     strcat(tmp, "IRC-op, ");
        if (_mirc->IsEntryChecked(M_IRC_AWAY)) strcat(tmp, "Away, ");
        tmp[strlen(tmp)-1] = '\0';
        tmp[strlen(tmp)-1] = '\0';
        strcat(tmp, ")");
      }
      _statusBar->SetText(2, new OString(tmp));

      // update toolbar mode buttons here too...
      tdata[9].button->SetState((_umode & UMODE_INVISIBLE) ? BUTTON_ENGAGED : BUTTON_UP);
      tdata[10].button->SetState((_umode & UMODE_WALLOPS) ? BUTTON_ENGAGED : BUTTON_UP);
      tdata[11].button->SetState((_umode & UMODE_SNOTICES) ? BUTTON_ENGAGED : BUTTON_UP);
      tdata[12].button->SetState((_umode & UMODE_IRCOP) ? BUTTON_ENGAGED : BUTTON_UP);

      if (_umode & UMODE_IRCOP) {
        _pserv->EnableEntry(M_SERV_REHASH);
        _pserv->EnableEntry(M_SERV_RESTART);
        _pserv->EnableEntry(M_SERV_SQUIT);
      } else {
        _pserv->DisableEntry(M_SERV_REHASH);
        _pserv->DisableEntry(M_SERV_RESTART);
        _pserv->DisableEntry(M_SERV_SQUIT);
      }
    } else {
      _statusBar->SetText(2, new OString(_nick));
    }
  }
}

void OXIrc::Log(const char *message) {
  Log(message, P_COLOR_TEXT);
}

void OXIrc::Log(const char *message, int color) {
  OLineDoc *l = new OLineDoc();
  l->SetCanvas(_log->GetTextFrame());
  l->SetDefaultColor(foxircSettings->GetPixelID(color));
  l->Fill((char *) message);
  _log->AddLine(l);
  _logw->ScrollUp();
  if (_logfile) {
    fprintf(_logfile, "%s\n", message);
    fflush(_logfile);
  }
}

int OXIrc::HandleMapNotify(XMapEvent *event) {
  if (!_init) {
    if (_server && *_server && _nick && *_nick)
      Connect(_server, _port);
    else if (foxircSettings->CheckMisc(P_MISC_POPUP_SERV_CN))
      DoConnect();
    _init = True;
  }
  return True;
}

int OXIrc::HandleKey(XKeyEvent *event) {
  if (event->type == KeyPress) {
    int keysym = XKeycodeToKeysym(GetDisplay(), event->keycode, 0);
    switch (keysym) {
      case XK_F1:
        DoHelp();
        break;

      default:
        return OXMainFrame::HandleKey(event);
    }
    return True;
  }
  return OXMainFrame::HandleKey(event);
}

int OXIrc::Connect(char *server, int port) {
  if (server) {
    char tmp[256];
    int retc;

    if (_connected) Disconnect();

    if (_server && (_server != server)) {
      delete[] _server;
      _server = StrDup(server);
    }

    if (_realserver) delete[] _realserver;
    _realserver = StrDup(_server);

    _port = port;

    if ((retc = _irc->Connect(_server, _port)) < 0) {
      snprintf(tmp, 256, "Connection to %s:%d failed (%s).",
                         _server, _port, strerror(errno));
      Log(tmp, P_COLOR_SERVER_1);
      return retc;
    }

    snprintf(tmp, 256, "Connecting to server %s, port %d",
                       _server, _port);
    Log(tmp, P_COLOR_SERVER_1);

    if (_fh) delete _fh;
    _fh = new OFileHandler(this, _irc->GetFD(), XCM_WRITABLE | XCM_EXCEPTION);

    _mirc->EnableEntry(M_IRC_DISCONNECT);
    _mirc->UnCheckEntry(M_IRC_AWAY);
    _mirc->EnableEntry(M_IRC_AWAY);
  }
  return -1;
}

int OXIrc::Disconnect() {
  _irc->Close();

  if (_connected) {
    _connected = False;
    Log("Connection closed.", P_COLOR_SERVER_1);
    Log(" ", P_COLOR_SERVER_1);
  }

  if (_fh) delete _fh;
  _fh = NULL;

  if (_pingTimer) delete _pingTimer;
  _pingTimer = NULL;

  _mirc->DisableEntry(M_IRC_DISCONNECT);
  _mirc->UnCheckEntry(M_IRC_AWAY);
  _mirc->DisableEntry(M_IRC_AWAY);

  UpdateStatusBar();

  // should close all channels, ask for confirmation, etc...
  // (if connecting to a different server, shall we re-join all active
  // channels?)
  return 0;
}

int OXIrc::HandleFileEvent(OFileHandler *fh, unsigned int mask) {
  if (!_connected) {
    char tmp[256];

    _connected = True;

    if (_fh) delete _fh;
    _fh = new OFileHandler(this, _irc->GetFD(), XCM_READABLE);

    Log("Connected, logging in...", P_COLOR_SERVER_1);

    _umode = 0L;
    _lag = 0;

    _mirc->EnableEntry(M_IRC_DISCONNECT);
    _mirc->UnCheckEntry(M_IRC_AWAY);
    _mirc->EnableEntry(M_IRC_AWAY);

    UpdateStatusBar(0);
    UpdateStatusBar(2);

    if (_passwd && *_passwd) {
      snprintf(tmp, 256, "PASS %s", _passwd);
      SendRawCommand(tmp);
    }

    snprintf(tmp, 256, "NICK %s", _nick);
    SendRawCommand(tmp);

    snprintf(tmp, 256, "USER %s %s %s :%s",
                       _username, _hostname, _server, _ircname);
    SendRawCommand(tmp);

    //Log(" ");

#if 0  // postpone this until we get the login confirmation
    if (foxircSettings->CheckMisc(P_MISC_POPUP_CHAN_CN))
      new OXChannelDialog(_client->GetRoot(), this);
#endif

  } else {
    _irc->Receive();
  }

  return True;
}

int OXIrc::HandleTimer(OTimer *t) {
  if (t != _pingTimer) return False;

  delete _pingTimer;
  _pingTimer = NULL;

  if (_connected) {
    char cmd[IRC_MSG_LENGTH];

    _pingTime = time(NULL);
    sprintf(cmd, "PING %lu %s", _pingTime, _realserver);
    SendRawCommand(cmd);
    _pingTimer = new OTimer(this, PING_INTERVAL * 1000);
  }

  return True;
}

// Find a channel window with the given name, if not found create it.

OXChannel *OXIrc::GetChannel(const char *channel) {
  OXChannel *ch;
  const OXWindow *main;

  if ((ch = FindChannel(channel)) != NULL) return ch;

  if (foxircSettings->CheckChannelFlags(channel, TRANSIENT_WINDOW))
    main = this;
  else
    main = NULL;

  if (channel && (*channel == '#' || *channel == '&' || *channel == '+')) {
    char str[IRC_MSG_LENGTH];

    ch = new OXChatChannel(_client->GetRoot(), main, this, channel);

    sprintf(str, "NAMES %s", channel);
    SendRawCommand(str);

    ((OXChatChannel *) ch)->EnableChannelMode(_avcmode);

  } else if (channel && *channel == '=') {
    ch = new OXDCCChannel(_client->GetRoot(), main, this, channel);

  } else {
    ch = new OXMsgChannel(_client->GetRoot(), main, this, channel);

  }

  _channels->Add(ch->GetId(), (XPointer) ch);

  return ch;
}

// Find a channel window with the given name, if not found return NULL.

OXChannel *OXIrc::FindChannel(const char *channel) {
  OXChannel *ch;
  const OXSNode *ptr;

  for (ptr = _channels->GetHead(); ptr; ptr = ptr->next) {
    ch = (OXChannel *) ptr->data;
    if (strcasecmp(channel, ch->GetName()) == 0) return ch;
  }

  return NULL;
}

// Remove the specified channel window from the chain.

void OXIrc::RemoveChannel(OXChannel *ch) {
  _channels->Remove(ch->GetId());
}

// Remove the specified DCC transfer window from the chain.

void OXIrc::RemoveDCC(OXDCCFile *dcc) {
  _transfers->Remove(dcc->GetId());
}

// Called when the user closes the channel list window.

void OXIrc::ChannelListClosed() {
  _channelList = NULL;
}

// Called when the user closes the server tree window.

void OXIrc::ServerTreeClosed() {
  _serverTree = NULL;
}

// Called when the user closes the external log view window.

void OXIrc::ViewLogFileClosed() {
  _viewLogFile = NULL;
}

// Return the channel list window, or create it if it does not exist.

OXChannelList *OXIrc::GetChannelList() {
  if (!_channelList)
    _channelList = new OXChannelList(_client->GetRoot(), NULL, this, 100, 100);
  return _channelList;
}

// Return the server tree window, or create it if it does not exist.

OXServerTree *OXIrc::GetServerTree() {
  if (!_serverTree)
    _serverTree = new OXServerTree(_client->GetRoot(), NULL, this, 100, 100);
  return _serverTree;
}

// Return the log file viewer window, or create it if it does not exist.

OXViewLogFile *OXIrc::GetViewLogFile() {
  if (!_viewLogFile)
    _viewLogFile = new OXViewLogFile(_client->GetRoot(), NULL, this);
  return _viewLogFile;
}

int OXIrc::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;
  char str[512];

  switch (msg->type) {
    case MSG_BUTTON:
    case MSG_MENU:
      switch (msg->action) {
        case MSG_CLICK:
          switch (wmsg->id) {
            //------------------------------ Menu: IRC

            case M_IRC_CONNECT:
              DoConnect();
              break;

            case M_IRC_DISCONNECT:
              Disconnect();
              break;

            case M_IRC_AWAY:
              DoAway();
              break;

            case M_IRC_EXIT:
              CloseWindow();
              break;

            //------------------------------ Menu: Log

            case M_LOG_OPEN:
              DoOpenLog();
              break;

            case M_LOG_CLOSE:
              DoCloseLog();
              break;

            case M_LOG_EMPTY:
              DoEmptyLog();
              break;

            case M_LOG_VIEW:
              DoViewLog();
              break;

            //------------------------------ Menu: Edit

            case M_EDIT_PREFS:
              new OXPreferencesDialog(_client->GetRoot(), this, foxircSettings);
              break;

            //------------------------------ Menu: View

            case M_VIEW_TOOLBAR:
              DoToggleToolBar();
              break;

            case M_VIEW_STATUSBAR:
              DoToggleStatusBar();
              break;

            case M_VIEW_FONT:
              DoChangeFont();
              break;

	    case M_VIEW_COLORS:
              break;

            //------------------------------ Menu: Help

            case M_HELP_INDEX:
              DoHelp();
              break;

            case M_HELP_ABOUT:
              DoHelpAbout(this);
              break;

            //------------------------------ Menu: Channels

            case M_CHAN_JOIN:
              new OXChannelDialog(_client->GetRoot(), this);
	      break;
	      
            case M_CHAN_WHO:
              DoWho();
              break;

            case M_CHAN_LIST:
              DoList();
              break;

            case M_CHAN_NOTICE:
              DoNotice();
              break;

            //------------------------------ Menu: Users

            case M_USERS_WHO:
              DoWho();
              break;

            case M_USERS_WHOIS:
              DoWhois();
              break;

            case M_USERS_WHOWAS:
              DoWhowas();
              break;

            case M_USERS_MSG:
              DoMessage();
              break;

            case M_USERS_NOTICE:
              DoNotice();
              break;

            case M_USERS_INVITE:
              DoInvite();
              break;

            //------------------------------ Menu: Servers

            case M_SERV_RAW:
              DoRaw();
              break;

            case M_SERV_NICK:
              DoNick();
              break;

            case M_SERV_WALLOPS:
              DoWallops();
              break;

            case M_SERV_MOTD:
              DoMotd();
              break;

            case M_SERV_VERSION:
              DoVersion();
              break;

            case M_SERV_ADMIN:
              DoAdmin();
              break;

            case M_SERV_LINKS:
              DoLinks();
              break;

            case M_SERV_MAP:
              DoMap();
              break;

            case M_SERV_LUSERS:
              DoLusers();
              break;

            case M_SERV_TIME:
              DoTime();
              break;

            case M_SERV_INFO:
              DoInfo();
              break;

            case M_SERV_TRACE:
              DoTrace();
              break;

            case M_SERV_OPER:
              DoOper();
              break;

            case M_SERV_REHASH:
              SendRawCommand("REHASH");
              break;

            case M_SERV_RESTART:
              SendRawCommand("RESTART");
              break;

            //------------------------------ Menu: user modes

            case TB_INVISIBLE:
            case TB_WALLOPS:
            case TB_SNOTICES:
            case TB_IRCOP:
              ToggleUMode(wmsg->id);
              break;
          }
        break;
      }
      break;

    case MSG_TEXTENTRY:
      {
        OTextEntryMessage *tmsg = (OTextEntryMessage *) msg;
        switch (msg->action) {
          case MSG_TEXTCHANGED:
            switch (tmsg->keysym) {
              case XK_Return:
                strcpy(str, _channelentry->GetString());
                if (strlen(str) > 0) JoinChannel(str);
	        _channelentry->Clear();
                break;
            }
            break;
        }
      }
      break;

    case INCOMING_IRC_MSG:
      {
        OIrcMessage *ircmsg = (OIrcMessage *) msg;

        if (isdigit(*ircmsg->command)) {  // numeric response?
          int cmd = atoi(ircmsg->command);

          switch (cmd) {
            case RPL_WELCOME:         // 001 :Welcome...
              if (_realserver) {
                delete[] _realserver;
                _realserver = StrDup(ircmsg->prefix);
                UpdateStatusBar(0);
              }
              if (foxircSettings->CheckMisc(P_MISC_POPUP_CHAN_CN)) {
                new OXChannelDialog(_client->GetRoot(), this);
              }
              // fall thru...
            case RPL_YOURHOST:        // 002 :Your host is...
            case RPL_CREATED:         // 003 :This server was created...
            case RPL_LOCALUSERS:      // (265)
            case RPL_GLOBALUSERS:     // (266)
              Log(ircmsg->argv[1], P_COLOR_SERVER_2);
              break;

            case RPL_MYINFO:          // 004 - host, version, umodes, cmodes
              _avumode = ModeBits(ircmsg->argv[3]);
              _avcmode = ModeBits(ircmsg->argv[4]);
              sprintf(str, "user modes available: %s, channel modes available: %s",
                           ircmsg->argv[3], ircmsg->argv[4]);
              Log(str, P_COLOR_SERVER_2);
              if (!_pingTimer) {
                _pingTimer = new OTimer(this, 30000);  // 1st time
              }
              break;

            case RPL_PROTOCTL:        // 005 ... :are supported by this server
              str[0] = '\0';
              for (int i = 1; i < ircmsg->argc; ++i) {
                if (*ircmsg->argv[i]) {
                  if (i > 1) strcat(str, " ");
                  strcat(str, ircmsg->argv[i]);
                }
              }
              Log(str, P_COLOR_SERVER_2);
              break;

            //--------------------------------- MAP reply

            case 6:                   // (006) - map info (rush 2.7.5)
            case 15:                  // (015) - map info (u2.10.H.05.21)
            case 7:                   // (007) - end of MAP (rush 2.7.5)
            case 16:                  // (016) - end of MAP (u2.10.H.05.21)
              ProcessMap(cmd, ircmsg);
              break;

            //--------------------------------- USERHOST reply

            //case RPL_USERHOST:        // 302 - :nickname*=+/-hostname ...
            //  break;

            //--------------------------------- ISON

            case RPL_ISON:            // 303
              if (ircmsg->argv[1] && *ircmsg->argv[1]) {
                time_t now = time(NULL);
                if (foxircSettings->SendToInfo(P_LOG_ISON)) {
                  sprintf(str, "%s: signon by %s detected",
                               DateString(now), ircmsg->argv[1]);
                  Log(str, P_COLOR_NOTICE);
                } else {
                  sprintf(str, "%s:\nSignon by %s detected.", 
                               DateString(now), ircmsg->argv[1]);
                  new OXMsgBox(_client->GetRoot(), this,
                               new OString("Notify"),
                               new OString(str),
                               MB_ICONASTERISK, ID_OK);
                }
              }   // otherwise is a signoff. We should periodically issue
                  // ISON commands in order to detect signon/signoffs.
              break;

            //--------------------------------- AWAY

            case RPL_AWAY:            // 301 - this can be part of WHOIS
                                      //       responses as well...
              sprintf(str, "%s is away", ircmsg->argv[1]);
              if (ircmsg->argv[2] && *ircmsg->argv[2])
                sprintf(str, "%s (%s)", str, ircmsg->argv[2]);
              /*GetChannel(ircmsg->argv[0])->*/Log(str, P_COLOR_SERVER_2);
              break;

            case RPL_NOWAWAY:         // 306
              Log(ircmsg->argv[ircmsg->argc-1], P_COLOR_NOTICE);  // argv[1]
              _mirc->CheckEntry(M_IRC_AWAY);
              UpdateStatusBar(2);
              break;

            case RPL_UNAWAY:          // 305
              Log(ircmsg->argv[ircmsg->argc-1], P_COLOR_NOTICE);  // argv[1]
              _mirc->UnCheckEntry(M_IRC_AWAY);
              UpdateStatusBar(2);
              break;

            //--------------------------------- WHOIS responses

            case RPL_WHOISMODES:      // (377) (379 in rush 2.7.5)
              Log(ircmsg->argv[1], P_COLOR_SERVER_2);
              break;

            case RPL_WHOISCHANNELS:   // 319
            case RPL_WHOISIDLE:       // 317
            case RPL_ENDOFWHOIS:      // 318
            case RPL_ENDOFWHOWAS:     // 369
            //case RPL_WHOISREGNICK:    // (307)
            case RPL_WHOISADMIN:      // (308)
            case RPL_WHOISSADMIN:     // (309)
            case RPL_WHOISHELPOP:     // (310)
            case RPL_WHOISUSER:       // (311)
            case RPL_WHOISSERVER:     // (312)
            case RPL_WHOISOPERATOR:   // (313)
            case RPL_WHOISNETWORK:    // (380)
            case RPL_WHOWASUSER:      // 314
            case 379:                 // 379 :is a services admin
            case 320:                 // 320 :is an identified user
            case RPL_WHOISHOST:       // (378) : Real Hostname :
              ProcessWhois(cmd, ircmsg);
              break;

            //--------------------------------- LIST

            case RPL_LISTSTART:       // 321 (this RPL is said to be obsolete!)
              if (foxircSettings->SendToInfo(P_LOG_CHAN_LIST))
                ; //Log(ircmsg->argv[1], P_COLOR_SERVER_2);
              else
                GetChannelList()->ClearList();
              break;

            case RPL_LIST:            // 322 - list reply: #chan nnn :title
              if (foxircSettings->SendToInfo(P_LOG_CHAN_LIST)) {
                sprintf(str, "%-10s %3s %s",
                             ircmsg->argv[1],
                             ircmsg->argv[2],
                             ircmsg->argv[3]);
                Log(str, P_COLOR_SERVER_2);
              } else {
                GetChannelList()->AddChannel(ircmsg->argv[1],
                                             atoi(ircmsg->argv[2]),
                                             ircmsg->argv[3]);
              }
              break;

            case RPL_LISTEND:         // 323 - end of list
              if (foxircSettings->SendToInfo(P_LOG_CHAN_LIST))
                Log("End of LIST", P_COLOR_SERVER_2);
              break;

            //--------------------------------- TOPIC responses

            case RPL_TOPIC:           // 332
            case RPL_TOPICWHOTIME:    // (333)
              SendMessage(FindChannel(ircmsg->argv[1]), msg);
              break;

            //case RPL_NOTOPIC:         // 331
            //  break;

            //--------------------------------- misc channel replies

            //case RPL_UNIQOPIS:        // 325 ???
            //  break;

            case RPL_CHANNELMODEIS:   // 324
              if (FindChannel(ircmsg->argv[1])) {
                SendMessage(FindChannel(ircmsg->argv[1]), msg);
              } else {
                sprintf(str, "Mode for %s: %s",
                             ircmsg->argv[1], ircmsg->argv[2]);
                Log(str, P_COLOR_SERVER_2);
              }
              break;

            case RPL_CREATIONTIME:    // (329) channel creation time
              if (FindChannel(ircmsg->argv[1])) {
                SendMessage(FindChannel(ircmsg->argv[1]), msg);
              } else {
                sprintf(str, "The channel %s was created on %s",
                             ircmsg->argv[1], DateString(ircmsg->argv[2]));
                Log(str, P_COLOR_SERVER_2);
              }
              break;

            //--------------------------------- INVITE succeeded

            case RPL_INVITING:        // 341
              sprintf(str, "Inviting %s to %s",
                           ircmsg->argv[1], ircmsg->argv[2]);
              Log(str, P_COLOR_INVITE);
              break;
		
            //--------------------------------- SUMMON in progress

            //case RPL_SUMMONING:       // 342
            //  break;

            //--------------------------------- channel invitation list

            //case RPL_INVITELIST:      // 346
            //case RPL_ENDOFINVITELIST: // 347
            //  break;

            //--------------------------------- channel exception list

            //case RPL_EXCEPTLIST:      // 348
            //case RPL_ENDOFEXCEPTLIST: // 349
            //  break;

            //--------------------------------- VERSION

            case RPL_VERSION:         // 351
              if (ircmsg->argv[2] && *ircmsg->argv[2])
                sprintf(str, "%s is running %s",
                             ircmsg->argv[2], ircmsg->argv[1]);
              else
                sprintf(str, "%s is running %s",
                             _realserver, ircmsg->argv[1]);
              Log(str, P_COLOR_SERVER_2);
              break;

            //--------------------------------- WHO reply

            case RPL_WHOREPLY:        // 352 - #channel login host server nick
            case RPL_ENDOFWHO:        // 315
              ProcessWho(cmd, ircmsg);
              break;

            //--------------------------------- NAMES List

            case RPL_NAMREPLY:        // 353
              if (FindChannel(ircmsg->argv[2]))
                SendMessage(FindChannel(ircmsg->argv[2]), msg);
              else
                Log(ircmsg->argv[ircmsg->argc-1], P_COLOR_SERVER_2);
              break;

            case RPL_ENDOFNAMES:      // 366
              if (!FindChannel(ircmsg->argv[1]))
                Log("End of NAMES", P_COLOR_SERVER_2);
              break;

            //--------------------------------- LINKS

            case RPL_LINKS:           // 364
            case RPL_ENDOFLINKS:      // 365
              ProcessLink(cmd, ircmsg);
              break;

            //--------------------------------- channel active ban list

            case RPL_BANLIST:         // 367
              if (FindChannel(ircmsg->argv[1])) {
                SendMessage(FindChannel(ircmsg->argv[1]), msg);
              } else {
                if (ircmsg->argc == 5) {
                  sprintf(str, "%s is banned from %s (set by %s on %s)",
                               ircmsg->argv[2], ircmsg->argv[1],
                               ircmsg->argv[3], DateString(ircmsg->argv[4]));
                } else {
                  sprintf(str, "%s is banned from channel %s",
                               ircmsg->argv[1], ircmsg->argv[2]);
                }
                Log(str, P_COLOR_SERVER_2);
              }
              break;

            case RPL_ENDOFBANLIST:    // 368
              if (FindChannel(ircmsg->argv[1])) {
                SendMessage(FindChannel(ircmsg->argv[1]), msg);
              } else {
                Log("End of BAN list", P_COLOR_SERVER_2);
              }
              break;

            //--------------------------------- INFO

            case RPL_INFOMORE:        // (370) (rush 2.7.5)
            case RPL_INFO:            // 371
            case RPL_INFOSTART:       // (373)
              Log(ircmsg->argv[1], P_COLOR_SERVER_1);
              break;

            case RPL_ENDOFINFO:       // 374
              Log("End of INFO", P_COLOR_SERVER_2);
              break;

            //--------------------------------- MOTD

            case RPL_MOTDSTART:       // 375
            case RPL_MOTD:            // 372
              Log(ircmsg->argv[1], P_COLOR_SERVER_1);
              break;

            case RPL_ENDOFMOTD:       // 376
              Log("End of MOTD", P_COLOR_SERVER_2);
              break;

            case ERR_NOMOTD:          // 422
              sprintf(str, "%s", ircmsg->argv[1] ? ircmsg->argv[1] : "No MOTD");
              Log(str, P_COLOR_SERVER_2);
              break;

            //--------------------------------- misc responses

            case RPL_YOUREOPER:       // 381 :you are now an ircop..
            case 396:                 // 396 :info set to
              Log(ircmsg->argv[1], P_COLOR_NOTICE);
              _umode |= UMODE_IRCOP;
              UpdateStatusBar(2);
              break;

            case RPL_REHASHING:       // 382 - 'config-file' :rehashing
              sprintf(str, "Rehasing %s", ircmsg->argv[1]);
              Log(str, P_COLOR_NOTICE);
              break;

            //case RPL_YOURESERVICE:    // 383 - :you are service ...
            //  break;

            //case RPL_UMODEIS:         // 221
            //  break;

            //--------------------------------- TIME

            case RPL_TIME:            // 391
              sprintf(str, "It is %s at %s",
                           ircmsg->argv[ircmsg->argc-1], ircmsg->argv[1]);
              Log(str, P_COLOR_SERVER_2);
              break;

            //--------------------------------- USERS (if supported by server)

            //case RPL_USERSSTART:      // 392 :userID terminal host
            //case RPL_USERS:           // 393
            //case RPL_ENDOFUSERS:      // 394 :end of users
            //case RPL_NOUSERS:         // 395 :nobody logged in
            //  break;

            //--------------------------------- TRACE

            //case RPL_TRACELINK:       // 200
            //case RPL_TRACECONNECTING: // 201
            //case RPL_TRACEHANDSHAKE:  // 202
            //case RPL_TRACEUNKNOWN:    // 203
            //case RPL_TRACEOPERATOR:   // 204
            //case RPL_TRACEUSER:       // 205
            //case RPL_TRACESERVER:     // 206
            //case RPL_TRACESERVICE:    // 207
            //case RPL_TRACENEWTYPE:    // 208
            //case RPL_TRACECLASS:      // 209
            //case RPL_TRACERECONNECT:  // 210
            //case RPL_TRACELOG:        // 261
            //case RPL_TRACEEND:        // 262
            //  break;

            //--------------------------------- STATS

            //case RPL_STATSLINKINFO:   // 211
            //case RPL_STATSCOMMANDS:   // 212
            //case RPL_STATSUPTIME:     // 242
            //case RPL_STATSOLINE:      // 243
            //case RPL_STATSCLINE:      // (213)
            //case RPL_STATSNLINE:      // (214)
            //case RPL_STATSILINE:      // (215)
            //case RPL_STATSKLINE:      // (216)
            //case RPL_STATSQLINE:      // (217)
            //case RPL_STATSYLINE:      // (218)
            //case RPL_STATSBLINE:      // (220)
            //case RPL_STATSLLINE:      // (241)
            //case RPL_STATSHLINE:      // (244)
            //case RPL_STATSSLINE:      // (245)
            //case RPL_STATSXLINE:      // (247)
            //case RPL_STATSULINE:      // (248)
            //case RPL_STATSDEBUG:      // (249)
            //case RPL_ENDOFSTATS:      // 219
            //  break;

            case RPL_STATSCONN:       // (250) :Highest connection count...
              Log(ircmsg->argv[1], P_COLOR_SERVER_2);
              break;

            //--------------------------------- SERVLIST

            //case RPL_SERVLIST:        // 234
            //case RPL_SERVLISTEND:     // 235
            //  break;

            //--------------------------------- LUSERS

            case RPL_LUSERCLIENT:     // 251
            case RPL_LUSERME:         // 255
              Log(ircmsg->argv[1], P_COLOR_SERVER_2);
              break;

            case RPL_LUSEROP:         // 252
            case RPL_LUSERUNKNOWN:    // 253
            case RPL_LUSERCHANNELS:   // 254
              sprintf(str, "%s %s", ircmsg->argv[1], ircmsg->argv[2]);
              Log(str, P_COLOR_SERVER_2);
              break;

            //--------------------------------- ADMIN reply

            case RPL_ADMINME:         // 256
            case RPL_ADMINLOC1:       // 257
            case RPL_ADMINLOC2:       // 258
            case RPL_ADMINEMAIL:      // 259
              ProcessAdmin(cmd, ircmsg);
              break;

            //--------------------------------- server dropped command

            //case RPL_TRYAGAIN:        // 263
            //  break;

            //--------------------------------- errors

            //case ERR_BANLISTFULL:     // 478
            case ERR_CHANOPRIVSNEEDED: // 482
              if (FindChannel(ircmsg->argv[1]))
                SendMessage(FindChannel(ircmsg->argv[1]), msg);
              else
                ProcessIrcError(cmd, ircmsg);
              break;

            case ERR_NICKNAMEINUSE:   // 433 :Nick is already in use..
              {
                sprintf(str, "Nickname %s is already in use",
                             ircmsg->argv[1]);
                Log(str, P_COLOR_HIGHLIGHT);
                OString tmp("");
                int retn;
                strcat(str, "\nPlease enter another nick");
                new OXConfirmDlg(_client->GetRoot(), this,
                        new OString("Change Nick"), new OString(str),
                        new OString("Nick:"), &tmp, &retn, ID_OK | ID_CANCEL);
                if (retn != ID_CANCEL)
                  ChangeNick(tmp.GetString());
		else
                  ChangeNick(_nick);
              }
              break;

            //--------------------------------- All others/unknown

            default:
              if ((cmd >= 400) && (cmd <= 599)) {
                // generic error responses with no special handling
                ProcessIrcError(cmd, ircmsg);
              } else {
                sprintf(str, "Unknown reply (%03d - ", cmd);
                for (int i = 1; i < ircmsg->argc; ++i) {
                  if (*ircmsg->argv[i]) {
                    if (i > 1) strcat(str, " ");
                    strcat(str, ircmsg->argv[i]);
                  }
                }
                strcat(str, ")");
                Log(str, P_COLOR_HIGHLIGHT);
              }
              break;
          }

        } else if (strcasecmp(ircmsg->command, "PING") == 0) {

          sprintf(str, "PONG :%s", ircmsg->argv[0]);
          SendRawCommand(str);

        } else if (strcasecmp(ircmsg->command, "PONG") == 0) {

          // if this is a response to our PING command then calculate lag 
          time_t tm;

          tm = strtoul(ircmsg->argv[1], (char **) 0, 10);
          if (tm == _pingTime) {
            _lag = time(NULL) - tm;
            UpdateStatusBar(1);
          } else {
            sprintf(str, "PONG from %s (%s)",
                         ircmsg->argv[0], ircmsg->argv[1]);
            Log(str, P_COLOR_SERVER_2);
          }

        } else if (strcasecmp(ircmsg->command, "NOTICE") == 0) {

          if (ircmsg->argv[0] && (strcasecmp(ircmsg->argv[0], "AUTH") == 0)) {
            Log(ircmsg->argv[1], P_COLOR_NOTICE);
          } else if (!*ircmsg->nick) {
            const char *channel = ircmsg->argv[0];
            if (*channel == '@') {
              ++channel;
              GetChannel(channel)->Say("", ircmsg->argv[1], NOTICE);
            } else {
              Log(ircmsg->argv[1], P_COLOR_NOTICE);
            }
          } else {
            if (*ircmsg->argv[1] == '\001') {
              ProcessCTCPReply(ircmsg);
            } else {
              if (strcasecmp(ircmsg->argv[0], _nick) == 0) {
                GetChannel(ircmsg->nick)->Say(ircmsg->nick,
                                              ircmsg->argv[1], NOTICE);
              } else {
                GetChannel(ircmsg->argv[0])->Say(ircmsg->nick,
                                                 ircmsg->argv[1], NOTICE);
              }
            }
          }

        } else if ((strcasecmp(ircmsg->command, "PRIVMSG") == 0)) {

          if (*ircmsg->argv[1] == '\001') {
            ProcessCTCPRequest(ircmsg);
          } else {
            if (strcasecmp(ircmsg->argv[0], _nick) == 0) {
              GetChannel(ircmsg->nick)->Say(ircmsg->nick,
                                            ircmsg->argv[1], PRIVMSG);
            } else {
              GetChannel(ircmsg->argv[0])->Say(ircmsg->nick,
                                               ircmsg->argv[1], PRIVMSG);
            }
          }

        } else if ((strcasecmp(ircmsg->command, "MODE") == 0)) {

          if (strcmp(ircmsg->argv[0], _nick) == 0) {
            sprintf(str, "Mode change \"%s\" for user %s by %s", 
                         ircmsg->argv[1],
                         ircmsg->argv[0],
                         ircmsg->prefix);
            ProcessUMode(ircmsg->argv[1]);
            Log(str, P_COLOR_MODE);
          } else {
            // It is OK to pass a NULL ptr to SendMessage.
            SendMessage(FindChannel(ircmsg->argv[0]), msg);
          }

        } else if ((strcasecmp(ircmsg->command, "PART") == 0)) {

          SendMessage(FindChannel(ircmsg->argv[0]), msg);

        } else if ((strcasecmp(ircmsg->command, "KICK") == 0)) {

          SendMessage(FindChannel(ircmsg->argv[0]), msg);

        } else if ((strcasecmp(ircmsg->command, "TOPIC") == 0)) {

          SendMessage(FindChannel(ircmsg->argv[0]), msg);

        } else if ((strcasecmp(ircmsg->command, "JOIN") == 0)) {

          if (strcmp(ircmsg->nick, _nick) == 0) {
            // we just joined a new channel...
            SendMessage(GetChannel(ircmsg->argv[0]), msg);
            sprintf(str, "MODE :%s", ircmsg->argv[0]);
            SendRawCommand(str);
          } else {
            SendMessage(FindChannel(ircmsg->argv[0]), msg);
          }

        } else if ((strcasecmp(ircmsg->command, "NICK") == 0)) {

          if (strcmp(ircmsg->argv[0], _nick) == 0) {
            // the server decided to change *our* nick!
            // (some ChanServ/NickServ services might do that, although
            // most of them would notify us first)
            sprintf(str, "Your nick has been changed to %s", ircmsg->argv[0]);
            if (_nick) delete[] _nick;
            _nick = StrDup(ircmsg->argv[0]);
            Log(str, P_COLOR_NOTICE);
          }

          const OXSNode *ptr;
          for (ptr = _channels->GetHead(); ptr; ptr = ptr->next) {
            // it is OK to send the message to all windows:
            // - if a channel window does not contain the old nick on
            //   its list, it will ignore the message
            // - message and DCC chat windows with the nick will rename
            //   themselves.
            OXChannel *ch = (OXChannel *) ptr->data;
            SendMessage(ch, msg);
          }

        } else if ((strcasecmp(ircmsg->command, "QUIT") == 0)) {

          const OXSNode *ptr;
          for (ptr = _channels->GetHead(); ptr; ptr = ptr->next) {
            OXChannel *ch = (OXChannel *) ptr->data;
            SendMessage(ch, msg);
          }

        } else if ((strcasecmp(ircmsg->command, "KILL") == 0)) {

          if (foxircSettings->SendToInfo(P_LOG_KILL)) {
            sprintf(str, "You have been KILLED by %s (%s)",
                         *ircmsg->nick ? ircmsg->nick : ircmsg->prefix,
                         ircmsg->argv[ircmsg->argc-1]);
            Log(str, P_COLOR_NOTICE);
          } else {
            sprintf(str, "You have been KILLED by %s:\n%s",
                         *ircmsg->nick ? ircmsg->nick : ircmsg->prefix,
                         ircmsg->argv[ircmsg->argc-1]);
            new OXMsgBox(_client->GetRoot(), this,
                         new OString("KILL"),
                         new OString(str),
                         MB_ICONASTERISK, ID_OK);
          }

        } else if ((strcasecmp(ircmsg->command, "SILENCE") == 0)) {

          sprintf(str, "User %s %ssilenced",
                       ircmsg->argv[0] + 1,
                       *ircmsg->argv[0] == '-' ? "un" : "");
          Log(str, P_COLOR_MODE);

        } else if ((strcasecmp(ircmsg->command, "ERROR") == 0)) {

          Log(ircmsg->argv[0], P_COLOR_HIGHLIGHT);

        } else if ((strcasecmp(ircmsg->command, "ERROR:") == 0)) {

          Log(ircmsg->rawmsg, P_COLOR_HIGHLIGHT);

        } else if ((strcasecmp(ircmsg->command, "WALLOPS") == 0)) {

          char *who = ircmsg->nick;
          if (!*ircmsg->nick) who = ircmsg->prefix;

          sprintf(str, "Wallops from %s: %s", who, ircmsg->argv[0]);
          Log(str, P_COLOR_WALLOPS);

        } else if ((strcasecmp(ircmsg->command, "INVITE") == 0)) {

          int retval;
          char *who = ircmsg->nick;
          if (!*ircmsg->nick) who = ircmsg->prefix;

          sprintf(str, "%s invites you to join channel %s\n"
                       "Do you want to join the channel?", 
                       who, ircmsg->argv[1]);
          new OXMsgBox(_client->GetRoot(), this,
                       new OString("Invite Dialog"), new OString(str),
                       MB_ICONQUESTION, ID_YES|ID_IGNORE, &retval);
          if (retval == ID_YES) JoinChannel(ircmsg->argv[1]);

        } else {

          sprintf(str, "Unknown command (%s)", ircmsg->command);
          Log(str, P_COLOR_HIGHLIGHT);

        }

      }
      break;

    case OUTGOING_TCP_MSG:
      FatalError("OXIrc: OUTGOING_TCP_MSG");
      break;

    case BROKEN_PIPE:
      Log("Connection reset by peer", P_COLOR_SERVER_1);
      Disconnect();
      break;

    case MSG_HELP:
      if (msg->action == MSG_CLOSE) _helpWindow = 0;
      break;

    default:
      break;

  }

  return True;
}


//----------------------------------------------------------------------

// CTCP stuff...

void OXIrc::ProcessCTCPRequest(OIrcMessage *msg) {
  char m[IRC_MSG_LENGTH];
  const char *nick = msg->nick;
  char *cmd, *p, *message = StrDup(msg->argv[1]);

  if (message[0] == '\001') {

    cmd = message + 1;

    for (int i = 1; i < strlen(message); ++i)
      if (message[i] == '\001') message[i] = '\0';

    p = strchr(cmd, ' ');
    if (p) *p++ = '\0'; else p = "";
    while (*p == ' ') ++p;

    if (strcmp(cmd, "ACTION") == 0) {
      if (strcasecmp(msg->argv[0], _nick) == 0) {
        GetChannel(msg->nick)->Say(msg->nick, msg->argv[1], PRIVMSG);
      } else {
        GetChannel(msg->argv[0])->Say(msg->nick, msg->argv[1], PRIVMSG);
      }
      delete[] message;
      return;

    } else if (strcmp(cmd, "CLIENTINFO") == 0) {
      if (*p) {
        char *info;
        for (info = p; *info; ++info) *info = toupper(*info);
        if (strcmp(p, "ACTION") == 0) {
          info = "sends action description";
        } else if (strcmp(p, "CLIENTINFO") == 0) {
          info = "shows available CTCP commands on the remote client";
        } else if (strcmp(p, "DCC") == 0) {
          info = "requests a Direct Client Connection";
        } else if (strcmp(p, "ECHO") == 0) {
          info = "echoes back the argument it receives";
        } else if (strcmp(p, "ERRMSG") == 0) {
          info = "returns error messages";
        } else if (strcmp(p, "TIME") == 0) {
          info = "shows the time in the remote client host";
        } else if (strcmp(p, "PING") == 0) {
          info = "sends back the argument it receives";
        } else if (strcmp(p, "VERSION") == 0) {
          info = "shows client type and version";
        } else {
          cmd = "ERRMSG";
          info = p;
          p = "CLIENTINFO: No such command";
        }
        sprintf(m, "%s %s %s", cmd, p, info);
        CTCPReply(nick, m);
        sprintf(m, "%s requested CTCP CLIENTINFO %s", nick, p);
      } else {
        sprintf(m, "CLIENTINFO "
                   "ACTION CLIENTINFO DCC ECHO ERRMSG TIME PING VERSION "
                   ":Use CLIENTINFO <COMMAND> to get more specific information");
        CTCPReply(nick, m);
        sprintf(m, "%s requested CTCP CLIENTINFO", nick);
      }

    } else if (strcmp(cmd, "DCC") == 0) {
      sprintf(m, "%s requested CTCP DCC %s", nick, p);
      ProcessDCCRequest(nick, p);

    } else if ((strcmp(cmd, "PING") == 0) ||
               (strcmp(cmd, "ECHO") == 0)) {
      sprintf(m, "%s %s", cmd, p);
      CTCPReply(nick, m);
      sprintf(m, "%s requested CTCP %s %s", nick, cmd, p);

    } else if (strcmp(cmd, "ERRMSG") == 0) {
      sprintf(m, "ERRMSG %s :No error", p);
      CTCPReply(nick, m);
      sprintf(m, "%s requested CTCP ERRMSG %s", nick, p);

    } else if (strcmp(cmd, "TIME") == 0) {
      time_t now = time(NULL);
      sprintf(m, "TIME %s", DateString(now));
      CTCPReply(nick, m);
      sprintf(m, "%s requested CTCP TIME", nick);

    } else if (strcmp(cmd, "VERSION") == 0) {
      sprintf(m, "VERSION %s %s (compiled with xclass %s) %s",
                 FOXIRC_NAME, FOXIRC_VERSION, XCLASS_VERSION,
                 FOXIRC_HOMEPAGE);
      CTCPReply(nick, m);
      sprintf(m, "%s requested CTCP VERSION", nick);

    } else {
      sprintf(m, "%s requested unknown CTCP %s %s", nick, cmd, p);
    }

    if (foxircSettings->SendToInfo(P_LOG_CTCP)) {
      Log(m, P_COLOR_CTCP);
    } else {
      if (strcasecmp(msg->argv[0], _nick) == 0) {
        GetChannel(msg->nick)->Log(m, P_COLOR_CTCP);
      } else {
        GetChannel(msg->argv[0])->Log(m, P_COLOR_CTCP);
      }
    }

  }

  delete[] message;
}

void OXIrc::ProcessCTCPReply(OIrcMessage *msg) {
  char m[IRC_MSG_LENGTH];
  const char *nick = msg->nick;
  char *cmd, *p, *message = StrDup(msg->argv[1]);

  if (message[0] == '\001') {

    cmd = message + 1;

    for (int i = 1; i < strlen(message); ++i)
      if (message[i] == '\001') message[i] = '\0';

    p = strchr(cmd, ' ');
    if (p) *p++ = '\0'; else p = "";

    if (strcmp(cmd, "PING") == 0) {
      time_t tm = strtoul(p, (char **) 0, 10);
      sprintf(m, "CTCP PING reply from %s: %ld seconds", nick, time(NULL) - tm);
    } else {
      sprintf(m, "CTCP %s reply from %s: %s", cmd, nick, p);
    }

    if (foxircSettings->SendToInfo(P_LOG_CTCP)) {
      Log(m, P_COLOR_CTCP);
    } else {
      if (strcasecmp(msg->argv[0], _nick) == 0) {
        GetChannel(msg->nick)->Log(m, P_COLOR_CTCP);
      } else {
        GetChannel(msg->argv[0])->Log(m, P_COLOR_CTCP);
      }
    }

  }

  delete[] message;
}

void OXIrc::CTCPRequest(const char *target, const char *command) {
  char str[512];

  snprintf(str, 512, "PRIVMSG %s :\001%s\001", target, command);
  SendRawCommand(str);
}

void OXIrc::CTCPReply(const char *target, const char *command) {
  char str[512];

  snprintf(str, 512, "NOTICE %s :\001%s\001", target, command);
  SendRawCommand(str);
}


//----------------------------------------------------------------------

// DCC stuff...

void OXIrc::ProcessDCCRequest(const char *nick, const char *string) {
  int  ret;
  char *str = StrDup(string);
  char *p, *cmd, *arg;
  char char1[IRC_MSG_LENGTH], char2[IRC_MSG_LENGTH],
       char3[IRC_MSG_LENGTH], char4[IRC_MSG_LENGTH];

  p = str;

  cmd = p;
  while (*p && *p != ' ') ++p;
  if (*p) *p++ = '\0';
  while (*p && *p == ' ') ++p;
  arg = p;

  if (strcmp(cmd, "SEND") == 0) {

    // DCC SEND filename ip_address port size

    if (sscanf(arg, "%s %s %s %s", char1, char2, char3, char4) == 4) {
      OXDCCFile *dcc = new OXDCCFile(_client->GetRoot(), this, nick,
                                     char1, char2, char3, char4, &ret);
      _transfers->Add(dcc->GetId(), (XPointer) dcc);

      if (ret == ID_DCC_REJECT) {
        sprintf(char2, "DCC REJECT SEND %s", char1);
        CTCPReply(nick, char2);
      }
    } else {
      sprintf(char2, "Invalid DCC SEND from %s: %s\n", nick, arg);
      Log(char2, P_COLOR_NOTICE);
    }

  } else if (strcmp(cmd, "CHAT") == 0) {

    // DCC CHAT chat ip_address port

    if (sscanf(arg, "%s %s %s", char1, char2, char3) == 3) {
      sprintf(char4, "%s wishes to chat with you ", nick);
      new OXMsgBox(_client->GetRoot(), this, new OString("DCC Confirm"), 
                   new OString(char4), MB_ICONQUESTION,
                   ID_YES | ID_NO | ID_IGNORE, &ret);

      if (ret == ID_YES) {
        AcceptDCCChat(nick, char2, atoi(char3));
      } else if (ret == ID_NO) {
        CTCPReply(nick, "DCC REJECT CHAT chat"); // CTCPRequest?
      } else {
        // CTCP chat ignored
      }
    } else {
      sprintf(char2, "Invalid DCC CHAT from %s: %s\n", nick, arg);
      Log(char2, P_COLOR_NOTICE);
    }

  } else if (strcmp(cmd, "REJECT") == 0) {

    if (sscanf(arg, "%s %s", char1, char2) == 2) {
      if (strcasecmp(char1, "CHAT") == 0) {
        CTCPReply(nick, "DCC REJECT CHAT chat");
        sprintf(char3, "DCC REJECT CHAT from %s\n", nick);
        Log(char3, P_COLOR_CTCP);
        sprintf(char3, "=%s", nick);
        OXChannel *ch = FindChannel(char3);
        if (ch) ((OXDCCChannel *) ch)->Disconnect();
      } else {
        sprintf(char3, "DCC REJECT %s from %s\n", char1, nick);
        Log(char3, P_COLOR_CTCP);
      }
    } else {
      sprintf(char3, "Unknown DCC REJECT from %s: %s\n", nick, arg);
      Log(char3, P_COLOR_NOTICE);
    }

  }
}

void OXIrc::StartDCCChat(const char *nick) {
  char char1[20], char2[256];
  unsigned long hostnum;
  unsigned short portnum;

  sprintf(char1, "=%s", nick);
  OXDCCChannel *xdcc = (OXDCCChannel *) GetChannel(char1);

  if (xdcc->Listen(&hostnum, &portnum)) {
    sprintf(char2, "DCC CHAT chat %lu %u", ntohl(hostnum), ntohs(portnum));
    CTCPRequest(nick, char2);  
  } else {
    sprintf(char2, "Failed to create server socket (%s)", strerror(errno));
    xdcc->Log(char2, P_COLOR_NOTIFY);
  }
}

void OXIrc::AcceptDCCChat(const char *nick, const char *server, int port) {
  char char1[IRC_MSG_LENGTH];

  sprintf(char1, "=%s", nick);
  OXDCCChannel *xdcc = (OXDCCChannel *) GetChannel(char1);
  if (xdcc->Connect(server, port) < 0) {
    sprintf(char1, "Failed to connect to host %s (%s)", 
                   xdcc->GetServerName(), strerror(errno));
    xdcc->Log(char1, P_COLOR_NOTIFY);
    xdcc->Disconnect();
  }
}

void OXIrc::StartDCCSend(const char *nick, const char *filename) {
  char char1[256];
  unsigned long hostnum, filesize;
  unsigned short portnum;
  struct stat stbuf;

  if (stat(filename, &stbuf) != 0) {
    return;
  }

  OXDCCFile *dcc = new OXDCCFile(_client->GetRoot(), this, nick, filename);
  _transfers->Add(dcc->GetId(), (XPointer) dcc);

  if (dcc->Listen(&hostnum, &portnum)) {
    sprintf(char1, "DCC SEND %s %lu %u %lu",
                   filename, ntohl(hostnum), ntohs(portnum), stbuf.st_size);
    CTCPRequest(nick, char1);
  } else {
    sprintf(char1, "DCC SEND: Failed to create server socket (%s)",
                   strerror(errno));
    Log(char1, P_COLOR_NOTIFY);
  }
}

//----------------------------------------------------------------------

// Change the current nick name, and signal the server accordingly.

void OXIrc::ChangeNick(const char *nick) {
  char str[IRC_MSG_LENGTH];

  if (nick && *nick) {
    if (nick != _nick) {  // see ERR_NICKNAMEINUSE case above...
      if (_nick) delete[] _nick;
      _nick = StrDup(nick);
    }
    sprintf(str, "NICK %s", _nick);
    SendRawCommand(str);
    UpdateStatusBar(2);
  }
}

void OXIrc::JoinChannel(const char *channel) {
  char str[IRC_MSG_LENGTH];

  sprintf(str, "JOIN %s", channel);
  SendRawCommand(str);
  // wait until we actually join the channel...
  //sprintf(str, "MODE :%s", channel);
  //SendRawCommand(str);
}

void OXIrc::ToggleUMode(int which) {
  OXButton *b;
  char m[3];

  switch (which) {
    case TB_INVISIBLE:
      b = tdata[9].button;
      m[1] = 'i';
      break;

    case TB_WALLOPS:
      b = tdata[10].button;
      m[1] = 'w';
      break;

    case TB_SNOTICES:
      b = tdata[11].button;
      m[1] = 's';
      break;

    case TB_IRCOP:
      b = tdata[12].button;
      m[1] = 'o';
      if (b->GetState() != BUTTON_UP) {
        b->SetState(BUTTON_UP);
        DoOper();
        return;
      }
      break;
  }

  m[0] = (b->GetState() == BUTTON_UP) ? '-' : '+';
  m[2] = '\0';

  SendUMode(m);
}

void OXIrc::DoConnect() {
  int retc;

  OString *title = new OString("Connect to Server");
  OServerInfo *info = new OServerInfo();

  if (_server  && *_server)  info->hostname = StrDup(_server);  // _realserver?
  if (_nick    && *_nick)    info->nick = StrDup(_nick);
  if (_ircname && *_ircname) info->ircname = StrDup(_ircname);
  if (_passwd  && *_passwd)  info->passwd = StrDup(_passwd);

  info->port = _port;

  new OXServerDlg(_client->GetRoot(), this, title, info, &retc);
  if (retc == ID_OK) {
    if (_nick) delete[] _nick;
    _nick = StrDup(info->nick);
    if (_ircname) delete[] _ircname;
    _ircname = StrDup(info->ircname);
    if (_passwd) delete[] _passwd;
    _passwd = NULL;
    if (info->passwd) _passwd = StrDup(info->passwd);
    Connect(info->hostname, info->port);
  }

  delete info;
}

void OXIrc::DoAway() {
  // TODO: get responses from a dialog box
  if (_mirc->IsEntryChecked(M_IRC_AWAY)) {
    SendRawCommand("AWAY");
    _mirc->UnCheckEntry(M_IRC_AWAY);
  } else {
    SendRawCommand("AWAY :Back soon.");
    _mirc->CheckEntry(M_IRC_AWAY);
  }
}

void OXIrc::DoList() {
  SendRawCommand("LIST");
}

void OXIrc::DoMotd() {
  OString cmd("MOTD"), arg("");

  if (PromptServer(&cmd, &arg)) {
    if (arg.GetLength() > 0) {
      cmd.Append(" ");
      cmd.Append(arg.GetString());
    }
    SendRawCommand(cmd.GetString());
  }
}

void OXIrc::DoVersion() {
  OString cmd("VERSION"), arg("");

  if (PromptServer(&cmd, &arg)) {
    if (arg.GetLength() > 0) {
      cmd.Append(" ");
      cmd.Append(arg.GetString());
    }
    SendRawCommand(cmd.GetString());
  }
}

void OXIrc::DoLinks() {
  OString cmd("LINKS"), arg("");

  if (PromptServer(&cmd, &arg)) {
    if (arg.GetLength() > 0) {
      cmd.Append(" ");
      cmd.Append(arg.GetString());
    }
    SendRawCommand(cmd.GetString());
  }
}

void OXIrc::DoMap() {
  OString cmd("MAP"), arg("");

  if (PromptServer(&cmd, &arg)) {
    if (arg.GetLength() > 0) {
      cmd.Append(" ");
      cmd.Append(arg.GetString());
    }
    SendRawCommand(cmd.GetString());
  }
}

void OXIrc::DoLusers() {
  OString cmd("LUSERS"), arg("");

  if (PromptServer(&cmd, &arg)) {
    if (arg.GetLength() > 0) {
      cmd.Append(" ");
      cmd.Append(arg.GetString());
    }
    SendRawCommand(cmd.GetString());
  }
}

void OXIrc::DoAdmin() {
  OString cmd("ADMIN"), arg("");

  if (PromptServer(&cmd, &arg)) {
    if (arg.GetLength() > 0) {
      cmd.Append(" ");
      cmd.Append(arg.GetString());
    }
    SendRawCommand(cmd.GetString());
  }
}

void OXIrc::DoTime() {
  OString cmd("TIME"), arg("");

  if (PromptServer(&cmd, &arg)) {
    if (arg.GetLength() > 0) {
      cmd.Append(" ");
      cmd.Append(arg.GetString());
    }
    SendRawCommand(cmd.GetString());
  }
}

void OXIrc::DoInfo() {
  OString cmd("INFO"), arg("");

  if (PromptServer(&cmd, &arg)) {
    if (arg.GetLength() > 0) {
      cmd.Append(" ");
      cmd.Append(arg.GetString());
    }
    SendRawCommand(cmd.GetString());
  }
}

void OXIrc::DoNick() {
  OString tmp(_nick ? _nick : "");
  int retn;

  new OXConfirmDlg(_client->GetRoot(), this,
                   new OString("Change Nick"),
                   new OString("Please enter Nick"),
                   new OString("Nick:"),
                   &tmp, &retn, ID_OK | ID_CANCEL);

  if (retn != ID_CANCEL) ChangeNick(tmp.GetString());
}

void OXIrc::DoWho() {
  OString cmd("WHO"), arg("");
  int retn;

  new OXConfirmDlg(_client->GetRoot(), this,
                   &cmd, new OString("Enter query mask"),
                   new OString("Mask:"),
                   &arg, &retn, ID_OK | ID_CANCEL);

  if (retn == ID_OK) {
    if (arg.GetLength() > 0) {
      cmd.Append(" ");
      cmd.Append(arg.GetString());
    }
    SendRawCommand(cmd.GetString());
  }
}

void OXIrc::DoWhois() {
  OString cmd("WHOIS"), arg("");
  int retn;

  new OXConfirmDlg(_client->GetRoot(), this,
                   &cmd, new OString("Enter nick name"),
                   new OString("Nick:"),
                   &arg, &retn, ID_OK | ID_CANCEL);

  if (retn == ID_OK) {
    if (arg.GetLength() > 0) {
      cmd.Append(" ");
      cmd.Append(arg.GetString());
    }
    SendRawCommand(cmd.GetString());
  }
}

void OXIrc::DoWhowas() {
  OString cmd("WHOWAS"), arg("");
  int retn;

  new OXConfirmDlg(_client->GetRoot(), this,
                   &cmd, new OString("Enter nick name"),
                   new OString("Nick:"),
                   &arg, &retn, ID_OK | ID_CANCEL);

  if (retn == ID_OK) {
    if (arg.GetLength() > 0) {
      cmd.Append(" ");
      cmd.Append(arg.GetString());
    }
    SendRawCommand(cmd.GetString());
  }
}

void OXIrc::DoRaw() {
  if (_connected) {
    OString tmp("");
    int ret;

    new OXConfirmDlg(_client->GetRoot(), this,
                     new OString("Send Raw Command"),
	             new OString("Enter raw command to send to the server"),
                     new OString("Command:"),
                     &tmp, &ret, ID_OK | ID_CANCEL);

    if (ret != ID_CANCEL) SendRawCommand(tmp.GetString());
  }
}

void OXIrc::DoWallops() {
  OString tmp("");
  int retn;

  new OXConfirmDlg(_client->GetRoot(), this,
                   new OString("Send Wallops"), 
                   new OString("Please enter Wallops Message"),
                   new OString("Message:"),
                   &tmp, &retn, ID_OK | ID_CANCEL);

  if (retn != ID_CANCEL) SendWallops(tmp.GetString());
}

void OXIrc::DoTrace() {
  OString cmd("TRACE"), arg("");
  int retc;

  OString *text = new OString("Enter nick or server name\n(default is ");
           text->Append(_realserver);
           text->Append(")");

  new OXConfirmDlg(_client->GetRoot(), this,
                   new OString(&cmd), text, new OString("Name:"),
                   &arg, &retc, ID_OK | ID_CANCEL);

  if (retc == ID_OK) {
    if (arg.GetLength() > 0) {
      cmd.Append(" ");
      cmd.Append(arg.GetString());
    }
    SendRawCommand(cmd.GetString());
  }
}

void OXIrc::DoOper() {
  int retc;

  OString wname("OPER");
  OString *text = new OString("Enter name and password");
  if (_realserver && *_realserver) {
    text->Append("\nfor ");
    text->Append(_realserver);
  }
  OString login("");
  OString passwd("");

  new OXPasswdDlg(_client->GetRoot(), this, &wname, text,
                  &login, &passwd, &retc);

  if (retc == ID_OK) {
    char str[256];
    sprintf(str, "OPER %s %s", login.GetString(), passwd.GetString());
    SendRawCommand(str);
  }
}

void OXIrc::DoNotice() {
  OString wtitle("Send Notice");
  OString channel(""), text("");
  int retc;

  new OXCommandDlg(_client->GetRoot(), this, &wtitle,
               new OString("Enter channel or nick name and notice to send"),
               new OString("Target:"), &channel,
               new OString("Notice:"), &text, &retc);

  if (retc == ID_OK) {
    if ((channel.GetLength() > 0) && (text.GetLength() > 0)) {
      char str[512];
      snprintf(str, 512, "NOTICE %s :%s",
                         channel.GetString(), text.GetString());
      SendRawCommand(str);
    }
  }
}

void OXIrc::DoMessage() {
  OString user("");
  int retc;

  new OXConfirmDlg(_client->GetRoot(), this,
                   new OString("Message"),
                   new OString("Enter nick name"),
                   new OString("Nick:"),
                   &user, &retc, ID_OK | ID_CANCEL);

  if (retc == ID_OK) {
    if (user.GetLength() > 0) 
      GetChannel(user.GetString());
  }
}

void OXIrc::DoInvite() {
  OString wtitle("Invite");
  OString nick(""), channel("");
  int retc;

  new OXCommandDlg(_client->GetRoot(), this, &wtitle,
               new OString("Enter nick and channel"),
               new OString("Nick:"), &nick,
               new OString("Channel:"), &channel, &retc);

  if (retc == ID_OK) {
    if ((nick.GetLength() > 0) && (channel.GetLength() > 0)) {
      char str[512];
      snprintf(str, 512, "INVITE %s :%s",
                         nick.GetString(), channel.GetString());
      SendRawCommand(str);
    }
  }
}

void OXIrc::DoOpenLog() {
  OFileInfo fi;

  if (_logfile) DoCloseLog();  // perhaps we should ask first...

  // always open the file in append mode,
  // perhaps we should ask for this too...

  fi.MimeTypesList = NULL;
  fi.file_types = filetypes;
  new OXFileDialog(_client->GetRoot(), this, FDLG_SAVE, &fi);
  if (fi.filename) {
    _logfilename = StrDup(fi.filename);
    _logfile = fopen(_logfilename, "a");
    if (!_logfile) {
      OString stitle("File Open Error");
      OString smsg("Failed to open log file: ");
      smsg.Append(strerror(errno));
      new OXMsgBox(_client->GetRoot(), this, &stitle, new OString(&smsg),
                   MB_ICONSTOP, ID_OK);
      delete[] _logfilename;
      _logfilename = NULL;
    }
    time_t now = time(NULL);
    fprintf(_logfile, "****** IRC log started %s", ctime(&now));
    _plog->EnableEntry(M_LOG_CLOSE);  // close
    _plog->EnableEntry(M_LOG_EMPTY);  // empty
  }
}

void OXIrc::DoCloseLog() {
  if (_logfile) {
    time_t now = time(NULL);
    fprintf(_logfile, "****** IRC log ended %s", ctime(&now));
    fclose(_logfile);
    _logfile = NULL;
    delete[] _logfilename;
    _logfilename = NULL;
    _plog->DisableEntry(M_LOG_CLOSE);  // close
    _plog->DisableEntry(M_LOG_EMPTY);  // empty
  }
}

void OXIrc::DoEmptyLog() {
}

void OXIrc::DoPrintLog() {
}

void OXIrc::DoViewLog() {
  OFileInfo fi;

  fi.MimeTypesList = NULL;
  fi.file_types = filetypes;
  new OXFileDialog(_client->GetRoot(), this, FDLG_OPEN, &fi);
  if (fi.filename) {
    GetViewLogFile()->Load(fi.filename);
  }
}

void OXIrc::DoChangeFont() {
  OString f(foxircSettings->GetFont()->NameOfFont());

  new OXFontDialog(_client->GetRoot(), this, &f);

  foxircSettings->SetFont(f.GetString());
  foxircSettings->Save();
  _logw->SetFont(f.GetString());
}

void OXIrc::DoToggleToolBar() {
  if (_toolBar->IsVisible()) {
    HideFrame(_toolBar);
    HideFrame(_toolBarSep);
    _mview->UnCheckEntry(M_VIEW_TOOLBAR);
  } else {
    ShowFrame(_toolBar);
    ShowFrame(_toolBarSep);
    _mview->CheckEntry(M_VIEW_TOOLBAR);
  }
}

void OXIrc::DoToggleStatusBar() {
  if (_statusBar->IsVisible()) {
    HideFrame(_statusBar);
    _mview->UnCheckEntry(M_VIEW_STATUSBAR);
  } else {
    ShowFrame(_statusBar);
    _mview->CheckEntry(M_VIEW_STATUSBAR);
  }
}

void OXIrc::DoHelp(const char *topic) {
  if (!_helpWindow) {
    _helpWindow = new OXHelpWindow(_client->GetRoot(), NULL, 600, 650,
                                   "index.html", topic, "foxirc");
    _helpWindow->Associate(this);
  } else {
    _helpWindow->DoSetCurrent(topic);
  }
  _helpWindow->MapRaised();
}

void OXIrc::DoHelpAbout(const OXWindow *t) {
  OAboutInfo info;
  
  info.wname = "About "FOXIRC_NAME;
  info.title = FOXIRC_NAME" "FOXIRC_VERSION" ("FOXIRC_RELEASE_DATE")\n"
               "A cool IRC client program";
  info.copyright = "Copyright © 1998-1999 by the fOX Project Team,\n"
                   "Copyright © 2000-2004 by Héctor Peraza.";
  info.text = "This program is free software; you can redistribute it "
              "and/or modify it under the terms of the GNU "
              "General Public License.\n\n"
              FOXIRC_HOMEPAGE;
  info.title_font = (OXFont *) _client->GetFont("Times -14 bold");

  new OXAboutDialog(_client->GetRoot(), t, &info);
}

int OXIrc::PromptServer(OString *cmd, OString *arg) {
  int retc;
  OString *text = new OString("Enter server pattern\n(default is ");
           text->Append(_realserver);
           text->Append(")");

  new OXConfirmDlg(_client->GetRoot(), this,
                   new OString(cmd), text, new OString("Pattern:"),
                   arg, &retc, ID_OK | ID_CANCEL);

  return (retc == ID_OK);
}


//----------------------------------------------------------------------

void OXIrc::ProcessWhois(int cmd, OIrcMessage *msg) {
  char str[512];
  time_t tm;

  switch (cmd) {
    case RPL_WHOISUSER:
      sprintf(str, "%s is %s@%s", msg->argv[1], msg->argv[2], msg->argv[3]);
      if (msg->argv[5] && *msg->argv[5])
        sprintf(str, "%s (%s)", str, msg->argv[5]);
      break;

    case RPL_WHOWASUSER:
      sprintf(str, "%s was %s@%s", msg->argv[1], msg->argv[2], msg->argv[3]);
      if (msg->argv[5] && *msg->argv[5])
        sprintf(str, "%s (%s)", str, msg->argv[5]);
      break;

    case RPL_WHOISSERVER:
      sprintf(str, "%s is on irc via server %s", msg->argv[1], msg->argv[2]);
      if (msg->argv[3] && *msg->argv[3])
        sprintf(str, "%s (%s)", str, msg->argv[3]);
      break;

    case RPL_WHOISCHANNELS:
      sprintf(str, "%s is on channels: %s", msg->argv[1], msg->argv[2]);
      break;

    case RPL_WHOISIDLE:
      if (msg->argc < 4) {
        tm = strtoul(msg->argv[2], (char **) 0, 10);
        char *itime = BuildSecondTime(tm);
        sprintf(str, "%s has been idle %s", msg->argv[1], itime);
      } else {
        tm = strtoul(msg->argv[2], (char **) 0, 10);
        char *itime = BuildSecondTime(tm);
        sprintf(str, "%s has been idle %s and signon was at %s", 
                     msg->argv[1], itime, DateString(msg->argv[3]));
      }
      break;

    case RPL_ENDOFWHOIS:
      sprintf(str, "End of WHOIS");
      break;

    case RPL_ENDOFWHOWAS:
      sprintf(str, "End of WHOWAS");
      break;

    case RPL_WHOISOPERATOR:
    case RPL_WHOISHELPOP:
    case RPL_WHOISSADMIN:
    case RPL_WHOISADMIN:
    case RPL_WHOISNETWORK:
    case 379:
    case 320:
      strcpy(str, msg->argv[1]);
      if (msg->argv[2] && *msg->argv[2])
        sprintf(str, "%s %s", str, msg->argv[2]);
      break;

    case RPL_WHOISHOST: // Real Hostname
      if (msg->argc > 1)
        sprintf(str, "Real Hostname is %s", msg->argv[msg->argc-1]);
      break;

  }

  if (foxircSettings->SendToInfo(P_LOG_WHOIS)) {
    Log(str, P_COLOR_SERVER_2);
  } else {
  }
}

//#fOX Zapper NetAdmin.FoxChat.Net roadkill.FoxChat.net Zapper G*@ :1 High Voltage
//#fOX Big_Mac foxproject.org Wild.FoxChat.Net Poki H :0 "Poki the wonderbot"
//* services foxchat.net services.foxchat.net ChanServ H* :3 Channel Server
//#fOX mike itchy2-16-70.ionsys.com Wild.FoxChat.Net Foxxer H :0 Me

void OXIrc::ProcessWho(int cmd, OIrcMessage *msg) {
  const char *chan, *user, *host, *server, *nick, *mode, *name;
  char str[512];
  int  hopcnt;

  switch (cmd) {
    case RPL_WHOREPLY:   // #channel login host server nick

      //chan login   host  server  nick  status

      chan = msg->argv[1];
      user = msg->argv[2];
      host = (msg->argc == 8) ? msg->argv[3] : "";
      server = msg->argv[msg->argc - 4];  // msg->argv[4];
      nick = msg->argv[msg->argc - 3];    // msg->argv[5];
      mode = msg->argv[msg->argc - 2];    // msg->argv[6];
      hopcnt = atoi(msg->argv[msg->argc-1]);  // msg->argv[7]

      sprintf(str, "%-10s %-10s %-4s %s@%s [%d:%s]",
                   chan, nick, mode, user, host, hopcnt, server);

      name = msg->argv[msg->argc-1];  // msg->argv[7]
      while (isdigit(*name)) name++;
      while (*name && (*name == ' ')) ++name;

      if (*name) sprintf(str, "%s (%s)", str, name);

      // if (foxircSettings->SendToInfo(P_LOG_WHOIS)) ...
      Log(str, P_COLOR_SERVER_2);
      break;

    case RPL_ENDOFWHO:
      Log("End of WHO", P_COLOR_SERVER_2);
      break;

  }
}

void OXIrc::ProcessAdmin(int cmd, OIrcMessage *msg) {
  char str[512];

  switch (cmd) {
    case RPL_ADMINME:
    case RPL_ADMINLOC1:
    case RPL_ADMINLOC2:
    case RPL_ADMINEMAIL:  // this isn't neccessarily an e-mail address...
      Log(msg->argv[1], P_COLOR_SERVER_2);
      break;
  }
}

void OXIrc::ProcessUMode(const char *modestr) {
  const char *p;
  int mode, add = false;

  for (p = modestr; *p; ++p) {
    if (*p == '+') {
      add = true;
      continue;
    } else if (*p == '-') {
      add = false;
      continue;
    } else {
      switch (*p) {
        case 'i': mode = UMODE_INVISIBLE; break;
        case 's': mode = UMODE_SNOTICES; break;
        case 'w': mode = UMODE_WALLOPS; break;
        case 'o':
        case 'O': mode = UMODE_IRCOP; break;
        default: mode = 0; break;
      }
      if (add) _umode |= mode; else _umode &= ~mode;
    }
  }

  UpdateStatusBar(2);
}

void OXIrc::ProcessLink(int cmd, OIrcMessage *msg) {
  char str[512];
  OServerLink *link;

  switch (cmd) {
    case RPL_LINKS:
      if (msg->argc == 4) {

        char *s = msg->argv[3];
        while (isdigit(*s)) ++s;
        while (*s == ' ') ++s;

        if (foxircSettings->SendToInfo(P_LOG_LINKS)) {
          sprintf(str, "%s (%s) is connected to %s [%d]",
                       msg->argv[1], s, msg->argv[2], atoi(msg->argv[3]));
          Log(str, P_COLOR_SERVER_2);
        } else {
          link = new OServerLink(msg->argv[1], msg->argv[2],
                                 s, atoi(msg->argv[3]));

          GetServerTree()->AddLink(link);
        }

      } else {
        sprintf(str, "Invalid Link (argc %d)", msg->argc);
        Log(str, P_COLOR_SERVER_2);
      }
      break;

    case RPL_ENDOFLINKS:
      if (foxircSettings->SendToInfo(P_LOG_LINKS)) {
        Log("End of LINKS", P_COLOR_SERVER_2);
      } else {
        GetServerTree()->BuildTree();
      }
      break;
  }
}

void OXIrc::ProcessMap(int cmd, OIrcMessage *msg) {
  char str[512];
  OServerLink *link;

  switch (cmd) {
    case 6:
    case 15:
      Log(msg->argv[1], P_COLOR_SERVER_2);
      break;

    case 7:
    case 17:
      Log("End of MAP", P_COLOR_SERVER_2);
      break;
  }
}

void OXIrc::ProcessIrcError(int cmd, OIrcMessage *msg) {
  char str[512];

  if (msg->argc > 2) {
    str[0] = '\0';
    for (int i = 1; i < msg->argc - 1; ++i) {
      if (i > 1) strcat(str, " ");
      strcat(str, msg->argv[i]);
    }
    sprintf(str, "%s: %s", str, msg->argv[msg->argc-1]);
  } else {
    strcpy(str, msg->argv[1]);
  }

  if (foxircSettings->SendToInfo(P_LOG_ERROR)) {
    Log(str, P_COLOR_HIGHLIGHT);
  } else {
    char tmp[256];

    sprintf(tmp, "IRC error %03d", cmd);
    new OXMsgBox(_client->GetRoot(), this,
                 new OString(tmp),
                 new OString(str),
                 MB_ICONSTOP, ID_OK);
  }
}

unsigned long OXIrc::ModeBits(char *mode_str) {
  int i, bit;
  unsigned long mode_bits = 0L;
    
  if (!mode_str) return 0L;
    
  for (i = 0; i < strlen(mode_str); ++i) {
    if (mode_str[i] == ' ') break;
    bit = mode_str[i] - 'a';  // use tolower here?
    if (bit >= 0 && bit <= 26) mode_bits |= (1 << bit);
  }

  return mode_bits;
}

#define DAYSECS 86400
#define HOURSECS 3600
#define MINUTESECS 60

char *OXIrc::BuildSecondTime(time_t tm) {
  static char str[256];
  unsigned long tem = tm;

  str[0] = 0;

  // days ?
  if (tem / DAYSECS) {
    sprintf(str, "%ld Days ", tem / DAYSECS);
    tem %= DAYSECS;
  }

  // hours ?
  if (tem / HOURSECS) {
    sprintf(str, "%s%ld Hours ", str, tem / HOURSECS);
    tem %= HOURSECS;
  }

  // minutes ?
  if (tem / MINUTESECS) {
    sprintf(str, "%s%ld Minutes ", str, tem / MINUTESECS);
    tem %= MINUTESECS;
  }

  if (tem > 0)
    sprintf(str, "%s%ld Seconds", str, tem);

  return str;
}

char *OXIrc::DateString(time_t tm) {
  static char str[256];

  strcpy(str, ctime(&tm));
  if (str[strlen(str)-1] == '\n') str[strlen(str)-1] = '\0';

  return str;
}

char *OXIrc::DateString(const char *tmstr) {
  return DateString((time_t) strtoul(tmstr, (char **) 0, 10));
}

void OXIrc::SendWallops(const char *mess) {
  char m[IRC_MSG_LENGTH];

  snprintf(m, IRC_MSG_LENGTH, "WALLOPS :%s", mess);
  SendRawCommand(m);
}

void OXIrc::SendUMode(const char *mod) {
  char m[IRC_MSG_LENGTH];

  snprintf(m, IRC_MSG_LENGTH, "MODE %s %s", _nick, mod);
  SendRawCommand(m);
}

void OXIrc::SendMode(const char *chan, const char *mod) {
  char m[IRC_MSG_LENGTH];

  snprintf(m, IRC_MSG_LENGTH, "MODE %s %s", chan, mod);
  SendRawCommand(m);
}

void OXIrc::SendRawCommand(const char *command) {
  if (_connected) {
    char m[IRC_MSG_LENGTH];

    snprintf(m, IRC_MSG_LENGTH, "%s\r\n", command);
    OTcpMessage msg(OUTGOING_TCP_MSG, 0, m);
    SendMessage(_irc, &msg);
  }
}
