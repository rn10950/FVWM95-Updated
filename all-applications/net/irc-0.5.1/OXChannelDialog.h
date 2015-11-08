#ifndef __OXCHANNELDIALOG_H
#define __OXCHANNELDIALOG_H

#include <xclass/OXTransientFrame.h>
#include <xclass/OXButton.h>
#include <xclass/OXTextButton.h>
#include <xclass/OXCheckButton.h>
#include <xclass/OXLabel.h>
#include <xclass/OXTextEntry.h>

#include "OXPreferences.h"
#include "OXIrc.h"
#include "OTcp.h"
#include "OIrcMessage.h"


//----------------------------------------------------------------------

class OXChannelDialog : public OXTransientFrame {
public:
  OXChannelDialog(const OXWindow *p, OXIrc *main,
                  unsigned long options = MAIN_FRAME | HORIZONTAL_FRAME);

  virtual int CloseWindow();
  virtual int ProcessMessage(OMessage *msg);

protected:
  void _DoJoin();
  void _DoAdd();
  void _DoRemove();

  OXIrc *_irc;
  OXLabel *_lab;
  OXButton *_add, *_edit, *_delete, *_join, *_names, *_ok;
  OXCheckButton *_onconnect;
  OXListBox *_lb;
  OXTextEntry *_te;
  OXCompositeFrame *_v1, *_v2;
  int active;
};

#endif  // __OXCHANNELDIALOG_H
