#ifndef __OXIRC_H
#define __OXIRC_H

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <xclass/OXClient.h>
#include <xclass/OXMainFrame.h>
#include <xclass/OXToolBar.h>
#include <xclass/OXStatusBar.h>
#include <xclass/OXMenu.h>
#include <xclass/OXButton.h>
#include <xclass/OXMsgBox.h>
#include <xclass/OX3dLines.h>
#include <xclass/OString.h>
#include <xclass/OXFileDialog.h>
#include <xclass/OXAboutDialog.h>
#include <xclass/OXHelpWindow.h>
#include <xclass/OFileHandler.h>

#include "OIrc.h"
#include "OTextDoc.h"
#include "OXViewDoc.h"
#include "OXPreferences.h"


#define MENU_DISABLED     (1<<0)
#define MENU_CHECKED      (1<<1)
#define MENU_RCHECKED     (1<<2)

struct SPopupData {
  OXPopupMenu *ptr;
  struct {
    char *name;
    int  id, state;
    SPopupData *popup_ref;
  } popup[20];
};

class OIrcMessage;
class OXChannelList;
class OXServerTree;
class OXViewLogFile;
class OXChannel;
class OXDCCFile;


//----------------------------------------------------------------------

class OXIrc : public OXMainFrame {
public:
  OXIrc(const OXWindow *p, char *nick = NULL,
        char *server = NULL, int port = 6667);
  virtual ~OXIrc();

  virtual int CloseWindow();
  virtual int ProcessMessage(OMessage *msg);

  virtual int HandleKey(XKeyEvent *event);
  virtual int HandleMapNotify(XMapEvent *event);
  virtual int HandleFileEvent(OFileHandler *fh, unsigned int mask);
  virtual int HandleTimer(OTimer *t);

  int Connect(char *server, int port = 6667);
  int Disconnect();
  int Connected() const { return _connected; }

  void Log(const char *message);
  void Log(const char *message, int color);

  OIrc *GetOIrc() const { return _irc; }
  const char *GetNick() const { return _nick; }

  unsigned long ModeBits(char *mode_str);

  OXChannel *GetChannel(const char *channel);
  OXChannel *FindChannel(const char *channel);
  void RemoveChannel(OXChannel *channel);
  
  void RemoveDCC(OXDCCFile *dcc);

  OXChannelList *GetChannelList();
  void ChannelListClosed();

  OXServerTree *GetServerTree();
  void ServerTreeClosed();

  OXViewLogFile *GetViewLogFile();
  void ViewLogFileClosed();

  void DoConnect();

  void DoAway();
  void DoNick();
  void DoWho();
  void DoWhois();
  void DoWhowas();
  void DoWallops();
  void DoMotd();
  void DoVersion();
  void DoLinks();
  void DoMap();
  void DoLusers();
  void DoAdmin();
  void DoTime();
  void DoInfo();
  void DoList();
  void DoTrace();
  void DoOper();
  void DoNotice();
  void DoMessage();
  void DoInvite();
  void DoRaw();

  void DoOpenLog();
  void DoCloseLog();
  void DoEmptyLog();
  void DoPrintLog();
  void DoViewLog();
  void DoChangeFont();
  void DoToggleToolBar();
  void DoToggleStatusBar();
  void DoHelp(const char *topic = NULL);
  void DoHelpAbout(const OXWindow *t);

  void ProcessWho(int cmd, OIrcMessage *msg);
  void ProcessWhois(int cmd, OIrcMessage *msg);
  void ProcessAdmin(int cmd, OIrcMessage *msg);
  void ProcessIrcError(int cmd, OIrcMessage *msg);
  void ProcessUMode(const char *modestr);
  void ProcessLink(int cmd, OIrcMessage *msg);
  void ProcessMap(int cmd, OIrcMessage *msg);

  void ProcessCTCPRequest(OIrcMessage *msg);
  void ProcessCTCPReply(OIrcMessage *msg);
  void CTCPReply(const char *target, const char *command);
  void CTCPRequest(const char *target, const char *command);

  void ProcessDCCRequest(const char *nick, const char *string);
  void StartDCCChat(const char *nick);
  void AcceptDCCChat(const char *nick, const char *server, int port);
  void StartDCCSend(const char *nick, const char *filename);

  void ChangeNick(const char *nick);
  void JoinChannel(const char *channel);
  void ToggleUMode(int which);
  void SendWallops(const char *mess);
  void SendUMode(const char *mod);
  void SendMode(const char *chan, const char *mod);
  void SendRawCommand(const char *command);

  char *BuildSecondTime(time_t tm);
  char *DateString(time_t tm);
  char *DateString(const char *tmstr);

protected:
  OXPopupMenu *MakePopup(SPopupData *p);
  void UpdateStatusBar(int field = -1);
  int  PromptServer(OString *cmd, OString *arg);

  OIrc *_irc;

  OFileHandler *_fh;
  OTimer *_pingTimer;
  
  time_t _pingTime, _lag;

  OXTextEntry *_channelentry;

  OTextDoc *_log;
  OXViewDoc *_logw;

  char *_nick, *_passwd, *_ircname, *_username;
  char *_server, *_realserver, *_hostname;
  int _init, _port, _fd, _connected;

  OXSList *_channels;
  OXSList *_transfers;
  OXChannelList *_channelList;
  OXViewLogFile *_viewLogFile;
  OXServerTree *_serverTree;

  OXMenuBar *_menubar;
  OXPopupMenu *_mirc, *_medit, *_mview, *_mhelp;
  OXPopupMenu *_pserv, *_pusers, *_pchan, *_plog;

  OXToolBar *_toolBar;
  OXHorizontal3dLine *_toolBarSep;
  OXStatusBar *_statusBar;

  unsigned long _avcmode, _avumode;   // available channel and user modes
  unsigned long _umode;               // current user mode
  
  char *_logfilename;
  FILE *_logfile;

  OXHelpWindow *_helpWindow;
};


#endif  // __OXIRC_H
