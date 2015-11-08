#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <X11/keysym.h>

#include "OIrcMessage.h"
#include "OXDCCChannel.h"
#include "OXIrc.h"


//----------------------------------------------------------------------

#define M_EDIT_COPY       2001
#define M_EDIT_SELECTALL  2002
#define M_EDIT_INVERTSEL  2003

#define M_VIEW_TOOLBAR    3001
#define M_VIEW_STATUSBAR  3002
#define M_VIEW_TITLE      3003
#define M_VIEW_FONT       3004
#define M_VIEW_COLORS     3005

#define M_HELP_INDEX      4001
#define M_HELP_ABOUT      4002


extern OLayoutHints *topleftlayout;
extern OLayoutHints *topexpandxlayout;
extern OLayoutHints *topexpandxlayout0;
extern OLayoutHints *topexpandxlayout1;
extern OLayoutHints *topexpandxlayout2;
extern OLayoutHints *topexpandxlayout3;
extern OLayoutHints *leftexpandylayout;
extern OLayoutHints *leftcenterylayout;
extern OLayoutHints *expandxexpandylayout;
extern OLayoutHints *menubarlayout;
extern OLayoutHints *menuitemlayout;

extern OSettings  *foxircSettings;

extern char *filetypes[];


//----------------------------------------------------------------------

OXDCCChannel::OXDCCChannel(const OXWindow *p, const OXWindow *main,
                           OXIrc *s, const char *ch) :
  OXChannel(p, main, s, ch, False) {

    _dccServer = new OTcp(); 
    _dccServer->Associate(this);
    _connected = False;
    _serverSocket = False;
    _fl = NULL;

    _serverName = NULL;
    _port = -1;

    //---- user menu

    _nick_ctcp = new OXPopupMenu(_client->GetRoot());
    _nick_ctcp->AddEntry(new OHotString("Clientinfo"), CTCP_CLIENTINFO);
    _nick_ctcp->AddEntry(new OHotString("Finger"),     CTCP_FINGER);
    _nick_ctcp->AddEntry(new OHotString("Ping"),       CTCP_PING);
    _nick_ctcp->AddEntry(new OHotString("Time"),       CTCP_TIME);
    _nick_ctcp->AddEntry(new OHotString("Version"),    CTCP_VERSION);
    _nick_ctcp->AddEntry(new OHotString("Userinfo"),   CTCP_USERINFO);
    _nick_ctcp->AddSeparator();
    _nick_ctcp->AddEntry(new OHotString("Other..."),   CTCP_OTHER);

    _nick_dcc = new OXPopupMenu(_client->GetRoot());
    _nick_dcc->AddEntry(new OHotString("Send..."), DCC_SEND);
//    _nick_dcc->AddEntry(new OHotString("Chat"),    DCC_CHAT);

    _nick_ignore = new OXPopupMenu(_client->GetRoot());
    _nick_ignore->AddEntry(new OHotString("All"),     0);
    _nick_ignore->AddSeparator();
    _nick_ignore->AddEntry(new OHotString("Clear"),   0);
    _nick_ignore->AddEntry(new OHotString("Msgs"),    0);
    _nick_ignore->AddEntry(new OHotString("Notices"), 0);
    _nick_ignore->AddEntry(new OHotString("Public"),  0);
    _nick_ignore->AddEntry(new OHotString("Invites"), 0);
    _nick_ignore->AddEntry(new OHotString("Wallops"), 0);
    _nick_ignore->AddEntry(new OHotString("Notes"),   0);
    _nick_ignore->AddEntry(new OHotString("CTCP"),    0);
    _nick_ignore->AddEntry(new OHotString("Others"),  0);

    _nick_actions = new OXPopupMenu(_client->GetRoot());
    _nick_actions->AddEntry(new OHotString("&Whois"),   C_WHOIS);
    _nick_actions->AddEntry(new OHotString("&Notice"),  CH_NOTICE);
    _nick_actions->AddPopup(new OHotString("&CTCP"),    _nick_ctcp);
    _nick_actions->AddPopup(new OHotString("DCC"),      _nick_dcc);
    _nick_actions->AddPopup(new OHotString("Ignore"),   _nick_ignore);

    //---- top-level menu

    _menulog = new OXPopupMenu(_client->GetRoot());
    _menulog->AddEntry(new OHotString("&Open..."),  L_OPEN);
    _menulog->AddEntry(new OHotString("&Close"), L_CLOSE);
    _menulog->AddSeparator();
    _menulog->AddEntry(new OHotString("&Empty"), L_EMPTY);
    _menulog->DisableEntry(L_CLOSE);
    _menulog->DisableEntry(L_EMPTY);

    _menumode = new OXPopupMenu(_client->GetRoot());
    _menumode->AddEntry(new OHotString("&Actions"),     M_ACTIONS);
    _menumode->DisableEntry(M_ACTIONS);

    _menuchannel = new OXPopupMenu(_client->GetRoot());
    _menuchannel->AddPopup(new OHotString("&Mode"),    _menumode);
    _menuchannel->AddPopup(new OHotString("&Peer"),    _nick_actions);
    _menuchannel->AddPopup(new OHotString("L&og"),     _menulog);
    _menuchannel->AddEntry(new OHotString("C&rypt"),   CH_CRYPT);
    _menuchannel->AddSeparator();
    _menuchannel->AddEntry(new OHotString("&Leave"),   CH_LEAVE);

    _menuedit = new OXPopupMenu(_client->GetRoot());
    _menuedit->AddEntry(new OHotString("&Copy"), M_EDIT_COPY);
    _menuedit->AddSeparator();
    _menuedit->AddEntry(new OHotString("Select &All"), M_EDIT_SELECTALL);
    _menuedit->AddEntry(new OHotString("&Invert Selection"), M_EDIT_INVERTSEL);
  
    _menuview = new OXPopupMenu(_client->GetRoot());
    _menuview->AddEntry(new OHotString("&Toolbar"),    M_VIEW_TOOLBAR);
    _menuview->AddEntry(new OHotString("Status &Bar"), M_VIEW_STATUSBAR);
    _menuview->AddSeparator();
    _menuview->AddEntry(new OHotString("&Font..."),    M_VIEW_FONT);
    _menuview->AddEntry(new OHotString("&Colors..."),  M_VIEW_COLORS);

    _menuview->CheckEntry(M_VIEW_STATUSBAR);
    _menuview->CheckEntry(M_VIEW_TITLE);

    _menuhelp = new OXPopupMenu(_client->GetRoot());   
    _menuhelp->AddEntry(new OHotString("&Index..."),    M_HELP_INDEX);
    _menuhelp->AddSeparator();
    _menuhelp->AddEntry(new OHotString("&About..."), M_HELP_ABOUT);  

    _menulog->Associate(this);
    _menuchannel->Associate(this);
    _menumode->Associate(this);
    _menuedit->Associate(this);
    _menuview->Associate(this);
    _menuhelp->Associate(this);

    _menubar = new OXMenuBar(this, 1, 1, HORIZONTAL_FRAME);

    _menubar->AddPopup(new OHotString("&Chat"), _menuchannel, menuitemlayout);
    _menubar->AddPopup(new OHotString("&Edit"), _menuedit, menuitemlayout);
    _menubar->AddPopup(new OHotString("&View"), _menuview, menuitemlayout);
    _menubar->AddPopup(new OHotString("&Help"), _menuhelp, menuitemlayout);

    AddFrame(_menubar, menubarlayout);

    //---- view and say entry

    _AddView();

    //---- status bar

    _statusBar = new OXStatusBar(this);
    AddFrame(_statusBar, new OLayoutHints(LHINTS_BOTTOM | LHINTS_EXPAND_X,
                                          0, 0, 3, 0));

    _UpdateStatusBar();
    SetFocusOwner(_sayentry);

    _UpdateWindowName();

    Resize(600, 350);

    MapSubwindows();
    Layout();

    CenterOnParent();

    MapWindow();
}

OXDCCChannel::~OXDCCChannel() {
  if (_connected) Disconnect();
  if (_fl) delete _fl;
  if (_dccServer) delete _dccServer;
  delete _nick_ignore;
  delete _nick_ctcp;
  delete _nick_dcc;
  delete _nick_actions;
  delete _menuchannel;
  delete _menumode;
  delete _menulog;
  delete _menuedit;
  delete _menuview;
  delete _menuhelp;
  if (_serverName) delete[] _serverName;
}

void OXDCCChannel::_UpdateWindowName() {
  char wname[100];

//  sprintf(wname, "DCC Chat with %s", &_name[1]);
  sprintf(wname, "=%s DCC Chat", &_name[1]);
  SetWindowName(wname);
}

void OXDCCChannel::_ShowHelp() {
  _server->DoHelp("dcc.html");
}

void OXDCCChannel::_UpdateStatusBar() {
  char tmp[256];

  if (_connected) {
    sprintf(tmp, "Connected to %s:%d", _serverName, _port);
    _statusBar->SetText(0, new OString(tmp));
  } else {
    _statusBar->SetText(0, new OString("Not connected."));
  }
}

int OXDCCChannel::Connect(const char *server, int port) {
  int retc;

  if (_connected) Disconnect();
  retc = _dccServer->Connect(strtoul(server, NULL, 10), port, True);

  if (_serverName) delete[] _serverName;
  _serverName = StrDup(_dccServer->GetAddress());
  _port = port;  //_dccServer->GetPort();

  if (_fl) delete _fl;
  _fl = NULL;

  if (retc > 0) {
    _fl = new OFileHandler(this, _dccServer->GetFD(),
                                 XCM_WRITABLE | XCM_EXCEPTION);
  }

  _serverSocket = False;

  _UpdateStatusBar();
  return retc;
}

int OXDCCChannel::Listen(unsigned long *host, unsigned short *port) {
  int ret;

  if (_connected) Disconnect();
  ret = _dccServer->Listen(host, port, _server->GetOIrc()->GetOTcp());

  if (_fl) delete _fl;
  _fl = NULL;

  if (ret < 0) return ret;
  _fl = new OFileHandler(this, _dccServer->GetFD(), XCM_READABLE);
  _serverSocket = True;
  Log("Waiting for connection...", P_COLOR_SERVER_1);

  return ret;
}

void OXDCCChannel::Disconnect(int log) {
  if (_connected) {
    _dccServer->Close();
    if (_fl) delete _fl;
    _fl = NULL;
    _connected = False;
    if (log) Log("Connection closed.", P_COLOR_SERVER_1);
    _UpdateStatusBar();
  }
}

int OXDCCChannel::HandleFileEvent(OFileHandler *fh, unsigned int mask) {

  if (fh != _fl) return False;

  if (_serverSocket) {
    if (_fl) { delete _fl; _fl = NULL; }
    OTcp *newconn = new OTcp();
    int ret = newconn->Accept(_dccServer->GetFD());  // write this better...
    delete _dccServer;  // we do not have to continue
                        // listening on that port...
    _dccServer = newconn;
    _dccServer->Associate(this);
    if (ret >= 0) {
      char s[256];

      if (_serverName) delete[] _serverName;
      _serverName = StrDup(_dccServer->GetAddress());
      _port = _dccServer->GetPort();

      _fl = new OFileHandler(this, _dccServer->GetFD(), XCM_READABLE);
      sprintf(s, "Connection to %s:%d established.", _serverName, _port);

      Log(s, P_COLOR_SERVER_1);
      _connected = True;
    } else {
      Log("Connection failed.", P_COLOR_SERVER_1);
    }
    _serverSocket = False;
    _UpdateStatusBar();

  } else {
    switch (mask) {
      case XCM_WRITABLE:
        if (!_connected) {
          _connected = True;
          Log("Connection established.", P_COLOR_SERVER_1);
          if (_fl) delete _fl;
          _fl = new OFileHandler(this, _dccServer->GetFD(), XCM_READABLE);
          _UpdateStatusBar();
        }
        break;

      case XCM_READABLE:
        _dccServer->Receive();
        break;
    }
  }


  return True;
}

int OXDCCChannel::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;
  char char1[IRC_MSG_LENGTH];

  switch (msg->type) {
    case MSG_MENU:
      switch (msg->action) {
        case MSG_CLICK:
          switch (wmsg->id) {
            case L_OPEN:
              DoOpenLog();
              break;

            case L_CLOSE:
              DoCloseLog();
              break;

            case L_EMPTY:
              DoEmptyLog();
              break;

            case CH_LEAVE:
              CloseWindow();
              break;

            case M_VIEW_FONT:
              DoChangeFont();
              break;

            case M_HELP_INDEX:
              _ShowHelp();
              break;

            case M_HELP_ABOUT:
              _server->DoHelpAbout(this);
              break;

            default:
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
	      sprintf(char1, "%s\n", _sayentry->GetString());
              if (strlen(_sayentry->GetString()) > 0) {
                if (_connected) {
		  if (!ProcessCommand(char1)) {
                    OTcpMessage message(OUTGOING_TCP_MSG, 0, char1);
                    SendMessage(_dccServer, &message);
                    char1[strlen(char1)-1] = '\0';  // strip the '\n'
                    Say(_server->GetNick(), char1, DCC);
		  }
                  _AddToHistory(_sayentry->GetString());
                }
	      }
	      _sayentry->Clear();
              _historyCurrent = _history.size();
              break;

            default:
              return OXChannel::ProcessMessage(msg);
          }
          break;
      }
      }
      break;

    case INCOMING_TCP_MSG:
      {
      OTcpMessage *tmsg = (OTcpMessage *) msg;
      Say(_name, (char *) tmsg->string->GetString(), DCC);
      }
      break;

    case BROKEN_PIPE:
      Log("Remote end closed connection.", P_COLOR_SERVER_1);
      Disconnect(False);
      break;

    case INCOMING_IRC_MSG:
      OXChannel::ProcessMessage(msg);
      break;
  }

  return True;
}

int OXDCCChannel::ProcessCommand(char *cmd) {
  char char1[IRC_MSG_LENGTH];
  int  cmdsize;
  
  if (!cmd || *cmd != '/') return False;
  cmdsize = strlen(cmd);
  if (cmd[--cmdsize] == '\n') cmd[cmdsize] = 0;

  if (strncmp(cmd, "/me ", 4) == 0) {
    sprintf(char1, "\1ACTION %s\1\n", &cmd[4]);
    {
      OTcpMessage message(OUTGOING_TCP_MSG, 0, char1);
      SendMessage(_dccServer, &message);
    }
    Say(_server->GetNick(), char1, PRIVMSG);
  } else {
    sprintf(char1, "Unknown command \"%s\"", &cmd[1]);
    Log(char1, P_COLOR_HIGHLIGHT);
  }

  return True;
}
