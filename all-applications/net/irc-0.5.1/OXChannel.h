#ifndef __OXCHANNEL_H
#define __OXCHANNEL_H

#include <stdlib.h>
#include <stdio.h>

#include <xclass/OXClient.h>
#include <xclass/OXTransientFrame.h>
#include <xclass/OXToolBar.h>
#include <xclass/OXStatusBar.h>
#include <xclass/OXMsgBox.h>
#include <xclass/OXMenu.h>
#include <xclass/OX3dLines.h>
#include <xclass/OString.h>

#include <vector>

#include "OXViewDoc.h"
#include "OXNameList.h"
#include "OXIrc.h"

#define M_ACTIONS    101
#define M_NOCASE     102
#define M_BAN        103
#define M_PRIVATE    104
#define M_MODERATED  105
#define M_SECRET     106
#define M_INVITEONLY 107
#define M_TOPIC      108
#define M_NOMSG      109
#define M_LIMIT      110
#define M_KEY        111

#define CH_WHO       201
#define CH_INVITE    202
#define CH_NOTICE    203
#define CH_CRYPT     204
#define CH_LEAVE     205

#define L_OPEN       301
#define L_CLOSE      302
#define L_EMPTY      303

#define CTCP_CLIENTINFO 401
#define CTCP_FINGER     402
#define CTCP_PING       403
#define CTCP_TIME       404
#define CTCP_VERSION    405
#define CTCP_USERINFO   406
#define CTCP_OTHER      407

#define DCC_SEND     501
#define DCC_CHAT     502


#define T_SAY        101
#define T_TOPIC      102

//--- Mode bits are: ...O ..zy xwvu tsrq ponm lkji hgfe dcba

//--- User modes

#define UMODE_INVISIBLE  (1<<8)
#define UMODE_CHANOP     (1<<14)
#define UMODE_SNOTICES   (1<<18)
#define UMODE_VOICED     (1<<21)
#define UMODE_WALLOPS    (1<<22)
#define UMODE_IRCOP      (1<<28)

//--- Channel modes

#define CMODE_SETBAN     (1<<1)
#define CMODE_INVITEONLY (1<<8)
#define CMODE_SETKEY     (1<<10)
#define CMODE_SETULIMIT  (1<<11)
#define CMODE_MODERATED  (1<<12)
#define CMODE_NOMSGS     (1<<13)
#define CMODE_SETCHANOP  (1<<14)
#define CMODE_PRIVATE    (1<<15)
#define CMODE_SECRET     (1<<18)
#define CMODE_OPTOPIC    (1<<19)
#define CMODE_SETSPEAK   (1<<21)


//---------------------------------------------------------------------

class OXChannel : public OXTransientFrame {
public:
  OXChannel(const OXWindow *p, const OXWindow *main, OXIrc *s,
            const char *ch, int init = True);
  virtual ~OXChannel();

  virtual int HandleKey(XKeyEvent *event);

  virtual int CloseWindow();
  virtual int ProcessMessage(OMessage *msg);

  virtual void Say(const char *nick, char *message, int mode);
  virtual void Log(const char *message);
  virtual void Log(const char *message, int color);

  virtual int ProcessCommand(char *cmd);

  const char *GetName() const { return _name; }
  void ChangeName(const char *name);

  void DoOpenLog();
  void DoCloseLog();
  void DoEmptyLog();
  void DoPrintLog();
  void DoChangeFont();
  
protected:
  void _AddView();
  void _InitHistory();
  void _AddToHistory(const char *str);
  void _ClearHistory();
  
  virtual void _UpdateWindowName();
  virtual void _ShowHelp();

  OXIrc *_server;

  OTextDoc *_log;
  OXViewDoc *_logw;
  std::vector<char *> _history;
  int _historyCurrent;

  OXCompositeFrame *_hf, *_vf;
  OXTextEntry *_sayentry;

  char *_name;
  OChannelInfo *_cinfo;

  char *_logfilename;
  FILE *_logfile;
  OXPopupMenu *_menulog;
};


#endif  // __OXCHANNEL_H
