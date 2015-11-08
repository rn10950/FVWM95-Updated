#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>

#include <X11/keysym.h>

#include <xclass/utils.h>
#include <xclass/OXFileDialog.h>

#include "OXIrc.h"
#include "IRCcodes.h"
#include "OIrcMessage.h"
#include "OXChatChannel.h"
#include "OXConfirmDlg.h"
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

OXChatChannel::OXChatChannel(const OXWindow *p, const OXWindow *main, 
                             OXIrc *s, const char *ch) :
  OXChannel(p, main, s, ch, False) {

  _InitHistory();

  _cmode = _ecmode = _umode = _eumode = 0L;
  _chlimit = 0;
  _chkey = NULL;

  _idle = NULL;

  //---- nicklist menu

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
  _nick_actions->AddEntry(new OHotString("Whois"),    C_WHOIS);
  _nick_actions->AddEntry(new OHotString("Message"),  C_MESSAGE);
  _nick_actions->AddEntry(new OHotString("Notice"),   C_NOTICE);
  _nick_actions->AddPopup(new OHotString("CTCP"),     _nick_ctcp);
  _nick_actions->AddPopup(new OHotString("DCC"),      _nick_dcc);
  _nick_actions->AddEntry(new OHotString("Notify"),   C_NOTIFY);
  _nick_actions->AddPopup(new OHotString("Ignore"),   _nick_ignore);
  _nick_actions->AddSeparator();
  _nick_actions->AddEntry(new OHotString("Speak"),    C_SPEAK);
  _nick_actions->AddEntry(new OHotString("ChanOp"),   C_CHAN_OP);
  _nick_actions->AddEntry(new OHotString("Kick"),     C_KICK);
  _nick_actions->AddEntry(new OHotString("Ban"),      C_BAN);
  _nick_actions->AddEntry(new OHotString("Ban+Kick"), C_BANKICK);
  _nick_actions->AddEntry(new OHotString("Kill"),     C_KILL);

  _nick_actions->DisableEntry(C_SPEAK);
  _nick_actions->DisableEntry(C_CHAN_OP);
  _nick_actions->DisableEntry(C_KICK);
  _nick_actions->DisableEntry(C_BAN);
  _nick_actions->DisableEntry(C_BANKICK);
  _nick_actions->DisableEntry(C_KILL);

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
  _menumode->AddEntry(new OHotString("&Ban"),         M_BAN);
  _menumode->AddEntry(new OHotString("&Private"),     M_PRIVATE);
  _menumode->AddEntry(new OHotString("&Moderated"),   M_MODERATED);
  _menumode->AddEntry(new OHotString("&Secret"),      M_SECRET);
  _menumode->AddEntry(new OHotString("&Invite Only"), M_INVITEONLY);
  _menumode->AddEntry(new OHotString("&Topic"),       M_TOPIC);
  _menumode->AddEntry(new OHotString("&No Msg"),      M_NOMSG);
  _menumode->AddEntry(new OHotString("&Limit"),       M_LIMIT);
  _menumode->AddEntry(new OHotString("&Key"),         M_KEY);
  _menumode->DisableEntry(M_ACTIONS);
  _menumode->DisableEntry(M_BAN);
  _menumode->DisableEntry(M_PRIVATE);
  _menumode->DisableEntry(M_MODERATED);
  _menumode->DisableEntry(M_SECRET);
  _menumode->DisableEntry(M_INVITEONLY);
  _menumode->DisableEntry(M_TOPIC);
  _menumode->DisableEntry(M_NOMSG);
  _menumode->DisableEntry(M_LIMIT);
  _menumode->DisableEntry(M_KEY);

  _menuchannel = new OXPopupMenu(_client->GetRoot());
  _menuchannel->AddPopup(new OHotString("&Mode"),     _menumode);
  _menuchannel->AddSeparator();
  _menuchannel->AddPopup(new OHotString("L&og"),       _menulog);
  _menuchannel->AddEntry(new OHotString("&Who"),       CH_WHO);
  _menuchannel->AddEntry(new OHotString("&Invite..."), CH_INVITE);
  _menuchannel->AddEntry(new OHotString("&Notice"),    CH_NOTICE);
  _menuchannel->AddPopup(new OHotString("&CTCP"),      _nick_ctcp);
  _menuchannel->AddEntry(new OHotString("C&rypt"),     CH_CRYPT);
  _menuchannel->AddSeparator();
  _menuchannel->AddEntry(new OHotString("&Leave"),     CH_LEAVE);

  _menuedit = new OXPopupMenu(_client->GetRoot());
  _menuedit->AddEntry(new OHotString("&Copy"), M_EDIT_COPY);
  _menuedit->AddSeparator();
  _menuedit->AddEntry(new OHotString("Select &All"), M_EDIT_SELECTALL);
  _menuedit->AddEntry(new OHotString("&Invert Selection"), M_EDIT_INVERTSEL);
  
  _menuview = new OXPopupMenu(_client->GetRoot());
  _menuview->AddEntry(new OHotString("&Toolbar"),    M_VIEW_TOOLBAR);
  _menuview->AddEntry(new OHotString("Status &Bar"), M_VIEW_STATUSBAR);
  _menuview->AddEntry(new OHotString("T&opic"),      M_VIEW_TITLE);
  _menuview->AddSeparator();
  _menuview->AddEntry(new OHotString("&Font..."),    M_VIEW_FONT);
  _menuview->AddEntry(new OHotString("&Colors..."),  M_VIEW_COLORS);

  _menuview->CheckEntry(M_VIEW_STATUSBAR);
  _menuview->CheckEntry(M_VIEW_TITLE);

  _menuhelp = new OXPopupMenu(_client->GetRoot());   
  _menuhelp->AddEntry(new OHotString("&Index..."), M_HELP_INDEX);
  _menuhelp->AddSeparator();
  _menuhelp->AddEntry(new OHotString("&About..."), M_HELP_ABOUT);  

  _menulog->Associate(this);
  _menuchannel->Associate(this);
  _menumode->Associate(this);
  _menuedit->Associate(this);
  _menuview->Associate(this);
  _menuhelp->Associate(this);

  _menubar = new OXMenuBar(this, 1, 1, HORIZONTAL_FRAME);

  _menubar->AddPopup(new OHotString("&Channel"), _menuchannel, menuitemlayout);
  _menubar->AddPopup(new OHotString("&Edit"), _menuedit, menuitemlayout);
  _menubar->AddPopup(new OHotString("&View"), _menuview, menuitemlayout);
  _menubar->AddPopup(new OHotString("&Help"), _menuhelp, menuitemlayout);

  AddFrame(_menubar, menubarlayout);

  //---- topic entry

  _topicsep = new OXHorizontal3dLine(this);
  AddFrame(_topicsep, topexpandxlayout3);

  _ftopic = new OXCompositeFrame(this, 10, 10, HORIZONTAL_FRAME);

  _ltopic = new OXLabel(_ftopic, new OString("Topic: "));
  _ftopic->AddFrame(_ltopic, leftcenterylayout);

  _topic = new OXTextEntry(_ftopic, new OTextBuffer(IRC_MSG_LENGTH - 10),
                           T_TOPIC);
  _topic->Associate(this);

  _ftopic->AddFrame(_topic, expandxexpandylayout);

  AddFrame(_ftopic, topexpandxlayout2);

  //---- view and say entry

  _AddView();

  //---- name list

  _canvas = new OXCanvas(_hf, 110, 1, FIXED_WIDTH);
  _canvas->GetViewPort()->SetBackgroundPixmap(ParentRelative);
  _nlist = new OXNameList(_canvas->GetViewPort(), _nick_actions, 1, 110,
                                                  FIXED_WIDTH);
  _nlist->Associate(this);
  _canvas->SetContainer(_nlist);
  _canvas->SetScrollMode(SB_ACCELERATED);
  _canvas->SetScrollDelay(10, 10);
  _hf->AddFrame(_canvas, leftexpandylayout);

  //---- status bar

  _statusBar = new OXStatusBar(this);
  AddFrame(_statusBar, new OLayoutHints(LHINTS_BOTTOM | LHINTS_EXPAND_X,
                               0, 0, 3, 0));

  _statusBar->SetWidth(0, 350/*250*/);
  _statusBar->AddLabel(200);

  _statusBar->SetText(0, new OString("Unknown channel modes"));
  _statusBar->SetText(1, new OString("Unknown status"));

  _UpdateWindowName();

  SetFocusOwner(_sayentry);

  Resize(610, 362);

  MapSubwindows();
  Layout();

  CenterOnParent();

  MapWindow();
}

OXChatChannel::~OXChatChannel() {
  if (_idle) delete _idle;
  delete _nick_actions;
  delete _nick_ignore;
  delete _nick_ctcp;
  delete _nick_dcc;
  delete _menuchannel;
  delete _menumode;
  delete _menulog;
  delete _menuedit;
  delete _menuview;
  delete _menuhelp;
  if (_chkey) delete _chkey;
}

int OXChatChannel::CloseWindow() {
  return DoLeave();
}

void OXChatChannel::_UpdateWindowName() {
  char wname[100];

//  sprintf(wname, "IRC channel %s", _name);
  sprintf(wname, "%s IRC channel", _name);
  SetWindowName(wname);
}

void OXChatChannel::_ShowHelp() {
  _server->DoHelp("channel.html");
}

int OXChatChannel::HandleIdleEvent(OIdleHandler *i) {
  if (i != _idle) return False;
  delete _idle;
  _idle = NULL;
  OXChannel::CloseWindow();
  return True;
}

int OXChatChannel::ProcessMessage(OMessage *msg) {
  char char1[IRC_MSG_LENGTH],
       char3[IRC_MSG_LENGTH],
       m[IRC_MSG_LENGTH];
  char *strtmp;
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;
  OTextEntryMessage *tmsg = (OTextEntryMessage *) msg;

  switch (msg->type) {
    case MSG_MENU:
      switch (msg->action) {
        case MSG_CLICK:
          switch (wmsg->id) {

            //----------------------------------------- Menu: Channel

            case CH_WHO:
              sprintf(m, "WHO :%s", _name);
              _server->SendRawCommand(m);
              break;

            case CH_INVITE:
              {
              int retc;
              OString *wtitle = new OString("Invite to ");
                       wtitle->Append(_name);
              OString *text = new OString("Enter the nick name of the user\n"
                                          "you want to invite to this channel");
              OString arg("");

              new OXConfirmDlg(_client->GetRoot(), this,
                               wtitle, text, new OString("Nick:"),
                               &arg, &retc, ID_OK | ID_CANCEL);

              if ((retc == ID_OK) && (arg.GetLength() > 0)) {
                sprintf(m, "INVITE %s %s", arg.GetString(), _name);
                _server->SendRawCommand(m);
              }
              }
              break;

            case CH_NOTICE:
              DoSendNotice(_name);
              break;

            case CH_LEAVE:
              DoLeave();
              break;

            //----------------------------------------- Menu: Channel/Log

            case L_OPEN:
              DoOpenLog();
              break;

            case L_CLOSE:
              DoCloseLog();
              break;

            case L_EMPTY:
              DoEmptyLog();
              break;

            //----------------------------------------- Menu: Channel/Mode

            case M_NOMSG:
              _server->SendMode(_name,
                         _menumode->IsEntryChecked(M_NOMSG) ? "-n" : "+n");
              break;

            case M_PRIVATE:
              _server->SendMode(_name,
                         _menumode->IsEntryChecked(M_PRIVATE) ? "-p" : "+p");
              break;

            case M_SECRET:
              _server->SendMode(_name,
                         _menumode->IsEntryChecked(M_SECRET) ? "-s" : "+s");
              break;

            case M_MODERATED:
              _server->SendMode(_name,
                         _menumode->IsEntryChecked(M_MODERATED) ? "-m" : "+m");
              break;

            case M_INVITEONLY:
              _server->SendMode(_name,
                         _menumode->IsEntryChecked(M_INVITEONLY) ? "-i" : "+i");
              break;

            case M_TOPIC:
              _server->SendMode(_name,
                         _menumode->IsEntryChecked(M_TOPIC) ? "-t" : "+t");
              break;

            case M_LIMIT:
              {
              int retc;
              OString *wtitle = new OString("Limit");
              OString *text = new OString("Enter limit value for channel ");
                       text->Append(_name);
                       text->Append("\n(zero or empty value for unlimited)");
              sprintf(m, "%d", _chlimit);
              OString arg(m);

              new OXConfirmDlg(_client->GetRoot(), this,
                               wtitle, text, new OString("Limit:"),
                               &arg, &retc, ID_OK | ID_CANCEL);

              if (retc == ID_OK) {
                if ((arg.GetLength() > 0) && (atoi(arg.GetString()) > 0)) {
                  sprintf(m, "+l %s", arg.GetString());
                  _server->SendMode(_name, m);
                } else {
                  _server->SendMode(_name, "-l");
                }
              }
              }
              break;

            case M_KEY:
              {
              int retc;
              OString *wtitle = new OString("Key");
              OString *text = new OString("Enter key to set for channel ");
                       text->Append(_name);
                       text->Append("\n(empty value clears key)");
              OString arg(_chkey ? _chkey : "");

              new OXConfirmDlg(_client->GetRoot(), this,
                               wtitle, text, new OString("Key:"),
                               &arg, &retc, ID_OK | ID_CANCEL);

              if (retc == ID_OK) {
                if (_chkey) {
                  sprintf(m, "-k %s", _chkey);
                  _server->SendMode(_name, m);
                }
                if (arg.GetLength() > 0) {
                  sprintf(m, "+k %s", arg.GetString());
                  _server->SendMode(_name, m);
                }
              }
              }
              break;

            //----------------------------------------- Menu: Channel/CTCP

            case CTCP_CLIENTINFO:
            case CTCP_FINGER:
            case CTCP_PING:
            case CTCP_TIME:
            case CTCP_VERSION:
            case CTCP_USERINFO:
            case CTCP_OTHER:
              DoRequestCTCP(_name, wmsg->id);
              break;

            //----------------------------------------- Menu: View

            case M_VIEW_STATUSBAR:
              DoToggleStatusBar();
              break;

            case M_VIEW_TITLE:
              DoToggleTopicBar();
              break;

            case M_VIEW_FONT:
              DoChangeFont();
              break;

            //----------------------------------------- Menu: Help

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

        default:
          break;
      }
      break;

    case MSG_TEXTENTRY:
      if (msg->action == MSG_TEXTCHANGED) {
        if (tmsg->id == T_SAY) {

          return OXChannel::ProcessMessage(msg);

        } else if (tmsg->id == T_TOPIC) {

          if (tmsg->keysym == XK_Return) {
            sprintf(char3, "TOPIC %s :%s", _name, _topic->GetString());
            _server->SendRawCommand(char3);
            _topic->Clear();
          }
        }

      }
      break;

    case IRC_NICKLIST:
      {
        ONickListMessage *nlmsg = (ONickListMessage *) msg;
        OXName *e;

        switch (msg->action) {
          case MSG_CLICK:
            switch (nlmsg->id) {

              //--------------------------------------- Nick list

              case C_WHOIS:
                sprintf(char1, "WHOIS %s", nlmsg->name);
                _server->SendRawCommand(char1);
                break;

              case C_MESSAGE:
                _server->GetChannel(nlmsg->name)->MapRaised();
                break;

              case C_NOTICE:
                DoSendNotice(nlmsg->name);
                break;

              //--------------------------------------- Nick list: CTCP

              case CTCP_PING:
              case CTCP_VERSION:
              case CTCP_TIME:
              case CTCP_FINGER:
              case CTCP_USERINFO:
              case CTCP_CLIENTINFO:
              case CTCP_OTHER:
                DoRequestCTCP(nlmsg->name, wmsg->id);
                break;

              //--------------------------------------- Nick list: DCC

              case DCC_CHAT:
                _server->StartDCCChat(nlmsg->name);
                break;

              case DCC_SEND:
                {
                OFileInfo fi;
                fi.file_types = filetypes;
                new OXFileDialog(_client->GetRoot(), this, FDLG_OPEN, &fi);
                if (fi.filename) {
                  _server->StartDCCSend(nlmsg->name, fi.filename);
                }
                }
                break;

              //--------------------------------------- Nick list: Mode

              case C_SPEAK:
                e = _nlist->GetName(nlmsg->name);
                if (e) {
                  sprintf(m, "%s %s",
                             e->IsVoiced() ? "-v" : "+v",
                             nlmsg->name);
                  _server->SendMode(_name, m);
                }
                break;

              case C_CHAN_OP:
                e = _nlist->GetName(nlmsg->name);
                if (e) {
                  sprintf(m, "%s %s",
                             e->IsOp() ? "-o" : "+o",
                             nlmsg->name);
                  _server->SendMode(_name, m);
                }
                break;

              case C_KICK:
                sprintf(m, "KICK %s %s", _name, nlmsg->name);
                _server->SendRawCommand(m);
                break;

              case C_BAN:
                sprintf(m, "MODE %s +b %s!*@*", _name, nlmsg->name);
                _server->SendRawCommand(m);
                break;

              case C_BANKICK:
                sprintf(m, "MODE %s +b %s!*@*", _name, nlmsg->name);
                _server->SendRawCommand(m);
                sprintf(m, "KICK %s %s", _name, nlmsg->name);
                _server->SendRawCommand(m);
                break;

              case C_KILL:
                break;

              default:
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
            case RPL_NAMREPLY:        // 353
              {
                char *s;
                int i, j;

                if (_nlist) {
                  for (i = 0, s = (char *) ircmsg->argv[3];
                       sscanf(s, "%s %n", m, &j) == 1;
                       s += j)
                    _nlist->AddName(m);
                  _canvas->Layout();
                }
                UpdateUserMode();
              }
              break;

            case RPL_CREATIONTIME:    // (329) channel creation time
              sprintf(m, "The channel was created on %s",
                         _server->DateString(ircmsg->argv[2]));
              Log(m, P_COLOR_TOPIC);
              break;

            case RPL_TOPICWHOTIME:    // (333)
              sprintf(m, "The topic was set by %s on %s", ircmsg->argv[2],
                         _server->DateString(ircmsg->argv[3]));
              Log(m, P_COLOR_TOPIC);
              break;

            case RPL_CHANNELMODEIS:   // 324
              char1[0] = '\0';
              for (int i = 2; i < ircmsg->argc; ++i) {
                if (*ircmsg->argv[i]) {
                  if (i > 2) strcat(char1, " ");
                  strcat(char1, ircmsg->argv[i]);
                }
              }
              DoChannelMode(char1);
              break;

            case RPL_TOPIC:           // 332
              _topic->Clear();
              _topic->AddText(0, ircmsg->argv[2]);
#if 1
              sprintf(m, "The topic for this channel is \"%s\"", 
                         ircmsg->argv[2]);
              Log(m, P_COLOR_TOPIC);
#endif
              break;

            case RPL_BANLIST:         // 367
              if (ircmsg->argc == 5) {
                sprintf(m, "%s is banned (set by %s on %s).",
                           ircmsg->argv[2], ircmsg->argv[3],
                           _server->DateString(ircmsg->argv[4]));
              } else {
                sprintf(m, "%s is banned.", ircmsg->argv[1]);
              }
              Log(m, P_COLOR_MODE);
              break;

            case RPL_ENDOFBANLIST:    // 368
              Log("End of BAN list.", P_COLOR_MODE);
              break;

            case ERR_CHANOPRIVSNEEDED:
              Log("You are not a channel operator.", P_COLOR_HIGHLIGHT);
              break;

            default:
              break;
          }

        } else if (strcasecmp(ircmsg->command, "JOIN") == 0) {

          _nlist->AddName(ircmsg->nick);
          if (*ircmsg->ircname)
            sprintf(m, "%s (%s) joined the channel.",
                       ircmsg->nick, ircmsg->ircname);
          else
            sprintf(m, "%s joined the channel.", ircmsg->nick);
          Log(m, P_COLOR_JOIN);
          _canvas->Layout();

        } else if (strcasecmp(ircmsg->command, "PART") == 0) {

          _nlist->RemoveName(ircmsg->nick);
          if (ircmsg->argv[1] && *ircmsg->argv[1])
            sprintf(m, "%s left the channel (%s).",
                       ircmsg->nick, ircmsg->argv[1]);
          else
            sprintf(m, "%s left the channel.", ircmsg->nick);
          Log(m, P_COLOR_PART);
          _canvas->Layout();
          if (strcmp(ircmsg->nick, _server->GetNick()) == 0)
            OXChannel::CloseWindow();

        } else if (strcasecmp(ircmsg->command, "KICK") == 0) {

          char *who = ircmsg->nick;
          if (!*who) who = ircmsg->prefix;

          _nlist->RemoveName(ircmsg->argv[1]);
          if (strcmp(ircmsg->argv[1], _server->GetNick()) == 0) {
            sprintf(m, "You have been kicked off the channel by %s (%s).", 
                       who, ircmsg->argv[2]);

           } else {
            sprintf(m, "%s has been kicked off the channel by %s (%s).", 
                       ircmsg->argv[1], who, ircmsg->argv[2]);
          }
          Log(m, P_COLOR_KICK);
          _canvas->Layout();

          if (strcmp(ircmsg->argv[1], _server->GetNick()) == 0) {
            if (foxircSettings->CheckMisc(P_MISC_AUTO_REJOIN)) {
              _server->JoinChannel(_name);
            } else {
              int retc;

              sprintf(m, "You have been kicked off channel %s by %s\n(%s).\n"
                         "Do you want to rejoin the channel?",
                         _name, who, ircmsg->argv[2]);
              OString *wtitle = new OString("Kicked from ");
                       wtitle->Append(_name);
              new OXMsgBox(_client->GetRoot(), this, wtitle, new OString(m),
                           MB_ICONEXCLAMATION, ID_YES | ID_NO, &retc);
              if (retc == ID_YES) {
                _server->JoinChannel(_name);
              } else {
                OXChannel::CloseWindow();  // skip the leave dialog
              }
            }  
          }    

        } else if (strcasecmp(ircmsg->command, "QUIT") == 0) {

          if (_nlist->GetName(ircmsg->nick)) {
            _nlist->RemoveName(ircmsg->nick);
            if (ircmsg->argv[0] && *ircmsg->argv[0])
              sprintf(m, "SignOff: %s (%s).", ircmsg->nick, ircmsg->argv[0]);
            else
              sprintf(m, "SignOff: %s", ircmsg->nick);
            Log(m, P_COLOR_QUIT);
            _canvas->Layout();
          }

        } else if (strcasecmp(ircmsg->command, "NICK") == 0) {

          if (_nlist && _nlist->GetName(ircmsg->nick)) {
            _nlist->ChangeName(ircmsg->nick, ircmsg->argv[0]);
            sprintf(m, "%s is now known as %s.", ircmsg->nick, ircmsg->argv[0]);
            Log(m, P_COLOR_NICK);
          }

        } else if (strcasecmp(ircmsg->command, "MODE") == 0) {

          char *who = ircmsg->nick;
          if (!*who) who = ircmsg->prefix;

          char1[0] = '\0';
          for (int i = 1; i < ircmsg->argc; ++i) {
            if (*ircmsg->argv[i]) {
              if (i > 1) strcat(char1, " ");
              strcat(char1, ircmsg->argv[i]);
            }
          }
          sprintf(m, "Mode change \"%s\" by %s", char1, who);
          Log(m, P_COLOR_MODE);
          DoChannelMode(char1);
          //HandleModeChange(char1);

        } else if (strcasecmp(ircmsg->command, "TOPIC") == 0) {

          char *who = ircmsg->nick;
          if (!*who) who = ircmsg->prefix;

          _topic->Clear();
          _topic->AddText(0, ircmsg->argv[1]);
          sprintf(m, "%s has set the topic: \"%s\"", who, ircmsg->argv[1]);
          Log(m, P_COLOR_TOPIC);

        }

      }
      break;
  }

  return True;
}

int OXChatChannel::ProcessCommand(char *cmd) {
  char char1[IRC_MSG_LENGTH],
       char2[IRC_MSG_LENGTH];

  if (!cmd || *cmd != '/') return False;

  if (!strncmp(cmd, "/me ", 4)) {
    sprintf(char1, "\1ACTION %s\1", &cmd[4]);
    sprintf(char2, "PRIVMSG %s :%s", _name, char1);
    _server->SendRawCommand(char2);
    Say(_server->GetNick(), char1, PRIVMSG);

  } else if (!strncmp(cmd, "/notice ", 8)) {
    sprintf(char1, "NOTICE %s :%s", _name, &cmd[8]);
    _server->SendRawCommand(char1);
    Say(_server->GetNick(), &cmd[8], NOTICE);

  } else if (!strncmp(cmd, "/names", 6)) {
    sprintf(char1, "NAMES %s", _name);
    _server->SendRawCommand(char1);

  } else if (!strncmp(cmd, "/leave", 6)) {
    DoLeave();

  } else if (!strncmp(cmd, "/msg ", 5)) {
    if (strlen(&cmd[5]) > 0)
      _server->GetChannel(&cmd[5])->MapRaised();

  } else if (!strncmp(cmd, "/nick ", 6)) {
    if (strlen(&cmd[6]) > 0)
      _server->ChangeNick(&cmd[6]);

  } else if (!strncmp(cmd, "/raw ", 5)) {
    if (strlen(&cmd[5]) > 0)
      _server->SendRawCommand(&cmd[5]);

  } else if (!strncmp(cmd, "/quote ", 7)) {
    if (strlen(&cmd[7]) > 0)
      _server->SendRawCommand(&cmd[7]);

  } else if (!strncmp(cmd, "/wallops ", 9)) {
    if (strlen(&cmd[9]) > 0)
      _server->SendWallops(&cmd[9]);

  } else if (!strncmp(cmd, "/mode ", 6)) {
    if (strlen(&cmd[6]) > 0)
      _server->SendMode(_name, &cmd[6]);

  } else if (!strncmp(cmd, "/umode ", 7)) {
    if (strlen(&cmd[7]) > 0)
      _server->SendUMode(&cmd[7]);

  } else if (!strncmp(cmd, "/whois ", 7)) {
    if (strlen(&cmd[7]) > 0)
      _server->SendRawCommand(&cmd[1]);

  } else if (!strncmp(cmd, "/whowas ", 8)) {
    if (strlen(&cmd[8]) > 0)
      _server->SendRawCommand(&cmd[1]);

  } else if (!strncmp(cmd, "/invite ", 8)) {
    if (strlen(&cmd[8]) > 0) {
      sprintf(char1, "INVITE %s %s", &cmd[8], _name);
      _server->SendRawCommand(char1);
    }

  } else if (!strncmp(cmd, "/kick ", 6)) {
    if (strlen(&cmd[6]) > 0) {
      sprintf(char1, "KICK %s %s", _name, &cmd[6]);
      _server->SendRawCommand(char1);
    }

  } else if (!strncmp(cmd, "/ctcp ", 6)) {
    if (strlen(&cmd[6]) > 0) {
      int n;
      char m[IRC_MSG_LENGTH];
      if (sscanf(&cmd[6], "%s %s %n", char1, char2, &n) == 2) {
        char *p = char2;
        while (*p) *p = toupper(*p), ++p;
        sprintf(m, "%s %s", char2, cmd+n+6);
        _server->CTCPRequest(char1, m);
      }
    }

  } else if (!strncmp(cmd, "/dcc ", 5)) {

  } else {
    if (strlen(cmd) > 1) _server->SendRawCommand(&cmd[1]);

  }

  return True;
}

void OXChatChannel::SetChannelMode(const char *mode_str) {
  int i, bit;
  unsigned long mode_bits = 0L;

  if (!mode_str) return;

  for (i = 0; i < strlen(mode_str); ++i) {
    bit = mode_str[i] - 'a';  // use tolower here?
    if (bit >= 0 && bit <= 26) mode_bits |= (1 << bit);
  }
  SetChannelMode(mode_bits);
}

void OXChatChannel::SetChannelMode(unsigned long mode_bits) {
  char msg[256];

  if (_cmode != mode_bits) {

    _cmode = mode_bits;

    if (_cmode & CMODE_SETBAN)     _menumode->CheckEntry(M_BAN);
    else                           _menumode->UnCheckEntry(M_BAN);
    if (_cmode & CMODE_INVITEONLY) _menumode->CheckEntry(M_INVITEONLY);
    else                           _menumode->UnCheckEntry(M_INVITEONLY);
    if (_cmode & CMODE_SETKEY)     _menumode->CheckEntry(M_KEY);
    else                           _menumode->UnCheckEntry(M_KEY);
    if (_cmode & CMODE_SETULIMIT)  _menumode->CheckEntry(M_LIMIT);
    else                           _menumode->UnCheckEntry(M_LIMIT);
    if (_cmode & CMODE_MODERATED)  _menumode->CheckEntry(M_MODERATED);
    else                           _menumode->UnCheckEntry(M_MODERATED);
    if (_cmode & CMODE_NOMSGS)     _menumode->CheckEntry(M_NOMSG);
    else                           _menumode->UnCheckEntry(M_NOMSG);
    if (_cmode & CMODE_PRIVATE)    _menumode->CheckEntry(M_PRIVATE);
    else                           _menumode->UnCheckEntry(M_PRIVATE);
    if (_cmode & CMODE_SECRET)     _menumode->CheckEntry(M_SECRET);
    else                           _menumode->UnCheckEntry(M_SECRET);
    if (_cmode & CMODE_OPTOPIC)    _menumode->CheckEntry(M_TOPIC);
    else                           _menumode->UnCheckEntry(M_TOPIC);
  }

  msg[0] = '\0';
  if (_cmode & CMODE_SETBAN)     strcat(msg, "Ban ");
  if (_cmode & CMODE_INVITEONLY) strcat(msg, "Invite-Only ");
  if (_cmode & CMODE_SETKEY)     strcat(msg, "Key ");
  if (_cmode & CMODE_SETULIMIT)  sprintf(msg, "%sLimit=%d ", msg, _chlimit);
  if (_cmode & CMODE_MODERATED)  strcat(msg, "Moderated ");
  if (_cmode & CMODE_NOMSGS)     strcat(msg, "No-Msgs ");
  if (_cmode & CMODE_PRIVATE)    strcat(msg, "Private ");
  if (_cmode & CMODE_SECRET)     strcat(msg, "Secret ");
  if (_cmode & CMODE_OPTOPIC)    strcat(msg, "Topic ");

  _statusBar->SetText(0, new OString(msg));
}

void OXChatChannel::UpdateUserMode() {
  unsigned long mode_bits = 0L;
  OXName *e = _nlist->GetName(_server->GetNick());

  if (e) {
    if (e->IsOp()) mode_bits |= UMODE_CHANOP;
    if (e->IsVoiced()) mode_bits |= UMODE_VOICED;
    SetUserMode(mode_bits);
  }
}

void OXChatChannel::SetUserMode(unsigned long mode_bits) {
  char msg[256];

  _umode = mode_bits;

  if (_umode & UMODE_CHANOP) {
    strcpy(msg, "Channel Op");
    _nick_actions->EnableEntry(C_SPEAK);
    _nick_actions->EnableEntry(C_CHAN_OP);
    _nick_actions->EnableEntry(C_KICK);
    _nick_actions->EnableEntry(C_BAN);
    _nick_actions->EnableEntry(C_BANKICK);
    //_nick_actions->EnableEntry(C_KILL);
  } else {
    strcpy(msg, "Not an Op");
    _nick_actions->DisableEntry(C_SPEAK);
    _nick_actions->DisableEntry(C_CHAN_OP);
    _nick_actions->DisableEntry(C_KICK);
    _nick_actions->DisableEntry(C_BAN);
    _nick_actions->DisableEntry(C_BANKICK);
    //_nick_actions->DisableEntry(C_KILL);
  }

  if (_umode & UMODE_VOICED) {
    strcat(msg, ", Voiced");
  }

  _statusBar->SetText(1, new OString(msg));
}

void OXChatChannel::EnableChannelMode(const char *mode_str) {
  int i, bit;
  unsigned long mode_bits = 0L;

  if (!mode_str) return;

  for (i = 0; i < strlen(mode_str); ++i) {
    bit = mode_str[i] - 'a';  // use tolower here?
    if (bit >= 0 && bit <= 26) mode_bits |= (1 << bit);
  }
  SetChannelMode(mode_bits);
}

void OXChatChannel::EnableChannelMode(unsigned long mode_bits) {
  if (_ecmode != mode_bits) {
    _ecmode = mode_bits;
    if (_ecmode & CMODE_SETBAN)     _menumode->EnableEntry(M_BAN);
    if (_ecmode & CMODE_INVITEONLY) _menumode->EnableEntry(M_INVITEONLY);
    if (_ecmode & CMODE_SETKEY)     _menumode->EnableEntry(M_KEY);
    if (_ecmode & CMODE_SETULIMIT)  _menumode->EnableEntry(M_LIMIT);
    if (_ecmode & CMODE_MODERATED)  _menumode->EnableEntry(M_MODERATED);
    if (_ecmode & CMODE_NOMSGS)     _menumode->EnableEntry(M_NOMSG);
    if (_ecmode & CMODE_PRIVATE)    _menumode->EnableEntry(M_PRIVATE);
    if (_ecmode & CMODE_SECRET)     _menumode->EnableEntry(M_SECRET);
    if (_ecmode & CMODE_OPTOPIC)    _menumode->EnableEntry(M_TOPIC);
  }
}

void OXChatChannel::EnableUserMode(unsigned long mode_bits) {
  _eumode = mode_bits;
  // update menu...
}

void OXChatChannel::DoToggleToolBar() {
}

void OXChatChannel::DoToggleStatusBar() {
  if (_statusBar->IsVisible()) {
    HideFrame(_statusBar);
    _menuview->UnCheckEntry(M_VIEW_STATUSBAR);
  } else {
    ShowFrame(_statusBar);
    _menuview->CheckEntry(M_VIEW_STATUSBAR);
  }
}

void OXChatChannel::DoToggleTopicBar() {
  if (_ftopic->IsVisible()) {
    HideFrame(_ftopic);
    HideFrame(_topicsep);
    _menuview->UnCheckEntry(M_VIEW_TITLE);
  } else {
    ShowFrame(_ftopic);
    ShowFrame(_topicsep);
    _menuview->CheckEntry(M_VIEW_TITLE);
  }
}

int OXChatChannel::DoLeave() {
  char str[IRC_MSG_LENGTH];
  OString msg("");

  if (_server->Connected()) {
    if (foxircSettings->Confirm(P_CONFIRM_LEAVE)) {
      int retc;
      OString *title = new OString("Leaving "); title->Append(_name);
      OString *label = new OString("Message:");
      OString *text = new OString("Really leave channel ");
               text->Append(_name); text->Append("?");

      new OXConfirmDlg(_client->GetRoot(), this,
                       title, text, label, &msg, &retc);
      if (retc == ID_NO) return False;
    }

    if (msg.GetLength() > 0)
      sprintf(str, "PART %s :%s", _name, msg.GetString());
    else
      sprintf(str, "PART %s", _name);

    _server->SendRawCommand(str);

    // We need this for the case the /leave command is entered from
    // the text entry box. The command processing is done in OXChannel
    // which calls ProcessCommand(), which in turn calls DoLeave (i.e.
    // this function), which causes the window to be closed (and thus
    // deleted). The control then goes back to OXChannel, which tries to
    // save the command into the just deleted history buffer and to clean 
    // the also deleted text entry. That would cause the program to
    // crash. So, instead we schedule an idle event and then close the
    // window when it is safe to do so.

    if (!_idle) _idle = new OIdleHandler(this);
    return False;
  }

  return OXChannel::CloseWindow();
}

void OXChatChannel::DoChannelMode(const char *mode) {
  std::vector<char *> sl;
  char *temp = StrDup(mode);
  const char *mode_str, *who, *ban_mask;
  int i, bit, param;
  bool add = true;
  unsigned long mode_bits = _cmode;

  sl.clear();

  char *ptr = strtok(temp, " ");
  while (ptr) {
    if (ptr) sl.push_back(ptr);
    ptr = strtok(0, " ");
  }

  if (sl.size() == 0) {
    delete[] temp;
    return;
  }

  mode_str = sl[0];
  param = 1;

  sl.push_back("");    // this is just a safety measure...

  for (i = 0; i < strlen(mode_str); ++i) {
    bit = mode_str[i];
    if (bit == '+') {
      add = true;
    } else if (bit == '-') {
      add = false;
    } else {
      switch (bit) {
        case 'i':  // invite-only
          if (add) mode_bits |= CMODE_INVITEONLY; else mode_bits &= ~CMODE_INVITEONLY;
          break;

        case 't':  // title
          if (add) mode_bits |= CMODE_OPTOPIC; else mode_bits &= ~CMODE_OPTOPIC;
          break;

        case 'm':  // moderated
          if (add) mode_bits |= CMODE_MODERATED; else mode_bits &= ~CMODE_MODERATED;
          break;

        case 'n':  // no msgs
          if (add) mode_bits |= CMODE_NOMSGS; else mode_bits &= ~CMODE_NOMSGS;
          break;

        case 's':  // secret
          if (add) mode_bits |= CMODE_SECRET; else mode_bits &= ~CMODE_SECRET;
          break;

        case 'p':  // private
          if (add) mode_bits |= CMODE_PRIVATE; else mode_bits &= ~CMODE_PRIVATE;
          break;

        case 'K':  // Knock
          break;

        case 'o':  // op
          who = sl[param++];
          if (add) _nlist->OpNick(who); else _nlist->OpNick(who, False);
          if (strcmp(_server->GetNick(), who) == 0) UpdateUserMode();
          break;

        case 'v':  // voiced
          who = sl[param++];
          if (add) _nlist->VoiceNick(who); else _nlist->VoiceNick(who, False);
          if (strcmp(_server->GetNick(), who) == 0) UpdateUserMode();
          break;

        case 'l':  // limit
          if (add) _chlimit = atoi(sl[param++]);
          if (add) mode_bits |= CMODE_SETULIMIT; else mode_bits &= ~CMODE_SETULIMIT;
          break;

        case 'b':  // ban
          if (add) ban_mask = sl[param++];
          if (add) mode_bits |= CMODE_SETBAN; else mode_bits &= ~CMODE_SETBAN;
          break;

        case 'k':  // key
          if (_chkey) delete[] _chkey;
          if (add) _chkey = StrDup(sl[param++]); else _chkey = NULL;
          if (add) mode_bits |= CMODE_SETKEY; else mode_bits &= ~CMODE_SETKEY;
          break;

        default:
          break;
      }
      if (param > sl.size() - 1) param = sl.size() - 1;
    }
  }

  SetChannelMode(mode_bits);

  delete[] temp;
}

void OXChatChannel::DoRequestCTCP(const char *target, int ctcp) {
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

void OXChatChannel::DoAskCTCP(const char *target) {
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

void OXChatChannel::DoSendNotice(const char *target) {
  int  retc;
  char m[IRC_MSG_LENGTH];

  OString *wtitle = new OString("Notice to ");
           wtitle->Append(target);
  OString *text = new OString("Enter your notice text");
  OString arg("");

  new OXConfirmDlg(_client->GetRoot(), this,
                   wtitle, text, new OString("Notice:"),
                   &arg, &retc, ID_OK | ID_CANCEL);

  if ((retc == ID_OK) && (arg.GetLength() > 0)) {
    sprintf(m, "NOTICE %s :%s", target, arg.GetString());
    _server->SendRawCommand(m);
    _server->GetChannel(target)->Say(_server->GetNick(),
                                     (char *) arg.GetString(), NOTICE);
  }
}
