#ifndef __OXMSGCHANNEL_H
#define __OXMSGCHANNEL_H

#include <stdlib.h>
#include <stdio.h>

#include <xclass/OXClient.h>
#include <xclass/OXTransientFrame.h>
#include <xclass/OXMsgBox.h>
#include <xclass/OXStatusBar.h>
#include <xclass/OXMenu.h>
#include <xclass/OString.h>

#include "OXIrc.h"
#include "OXChannel.h"


//---------------------------------------------------------------------

class OXMsgChannel : public OXChannel {
public:
  OXMsgChannel(const OXWindow *p, const OXWindow *main, OXIrc *s,
               const char *ch);
  virtual ~OXMsgChannel();

  virtual int ProcessMessage(OMessage *msg);

protected:
  void DoRequestCTCP(const char *target, int ctcp);
  void DoAskCTCP(const char *target);
  
  virtual void _UpdateWindowName();
  virtual void _ShowHelp();

  OXMenuBar *_menubar;
  OXPopupMenu *_menuchannel, *_menumode, *_menuedit,
              *_menuview, *_menuhelp, *_nick_ctcp, *_nick_dcc,
              *_nick_ignore, *_nick_actions;

  OXStatusBar *_statusBar;
};


#endif  // __OXMSGCHANNEL_H
