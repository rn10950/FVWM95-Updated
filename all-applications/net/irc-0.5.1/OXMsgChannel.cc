#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>

#include "OXIrc.h"
#include "OIrcMessage.h"
#include "OXMsgChannel.h"
#include "OXCommandDlg.h"


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

OXMsgChannel::OXMsgChannel(const OXWindow *p, const OXWindow *main,
                           OXIrc *s, const char *ch) :
  OXChannel(p, main, s, ch, False) {

    _InitHistory();

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
    _nick_dcc->AddEntry(new OHotString("Chat"),    DCC_CHAT);

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

    SetFocusOwner(_sayentry);

    _UpdateWindowName();

    Resize(560, 350);

    MapSubwindows();
    Layout();

    CenterOnParent();

    MapWindow();
}

OXMsgChannel::~OXMsgChannel() {
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
}

void OXMsgChannel::_UpdateWindowName() {
  char wname[100];

  //sprintf(wname, "Conversation with %s", _name);
  sprintf(wname, "%s Private Chat", _name);
  SetWindowName(wname);
}

void OXMsgChannel::_ShowHelp() {
  _server->DoHelp("message.html");
}
 
int OXMsgChannel::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;

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

            case CTCP_PING:
            case CTCP_VERSION:
            case CTCP_TIME:
            case CTCP_FINGER:
            case CTCP_USERINFO:
            case CTCP_CLIENTINFO:
            case CTCP_OTHER:
              DoRequestCTCP(_name, wmsg->id);
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
          }
          break;
      }
      break;

    default:
      return OXChannel::ProcessMessage(msg);
  }

  return True;
}

void OXMsgChannel::DoRequestCTCP(const char *target, int ctcp) {
  char char1[IRC_MSG_LENGTH];

  switch (ctcp) {
    case CTCP_CLIENTINFO:
      _server->CTCPRequest(target, "CLIENTINFO");
      break;

    case CTCP_FINGER:
      _server->CTCPRequest(target, "FINGER");
      break;

    case CTCP_PING:
      sprintf(char1, "PING %ld", time(NULL));
      _server->CTCPRequest(target, char1);
      break;

    case CTCP_TIME:
      _server->CTCPRequest(target, "TIME");
      break;

    case CTCP_VERSION:
      _server->CTCPRequest(target, "VERSION");
      break;

    case CTCP_USERINFO:
      _server->CTCPRequest(target, "USERINFO");
      break;

    case CTCP_OTHER:
      DoAskCTCP(target);
      break;
  }
}

void OXMsgChannel::DoAskCTCP(const char *target) {
  OString wtitle("CTCP");
  OString cmd(""), args("");
  int retc;

  new OXCommandDlg(_client->GetRoot(), this, &wtitle,
                   new OString("Enter CTCP command and arguments"),
                   new OString("Command:"), &cmd,
                   new OString("Arguments:"), &args, &retc);

  if (retc == ID_OK) {
    char *p = (char *) cmd.GetString();
    while (*p) *p = toupper(*p), ++p;
    if (args.GetLength() > 0) {
      cmd.Append(" ");
      cmd.Append(args.GetString());
    }
    _server->CTCPRequest(target, cmd.GetString());
  }
}
