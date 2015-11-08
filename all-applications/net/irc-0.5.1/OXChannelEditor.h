#ifndef __OXCHANNELEDITOR_H
#define __OXCHANNELEDITOR_H

#include <xclass/OXTransientFrame.h>
#include <xclass/OXTextEntry.h>
#include <xclass/OXCheckButton.h>
#include <xclass/OXTextButton.h>

#include "OXPreferences.h"


//----------------------------------------------------------------------

class OXChannelEditor : public OXTransientFrame {
public:
  OXChannelEditor(const OXWindow *p, const OXWindow *main, OChannelInfo *ci,
                  int *retc);
  virtual ~OXChannelEditor();

  virtual int ProcessMessage(OMessage *msg);

protected:
  void _LoadInfo();
  void _SaveInfo();
  void _GetBack();
  void _GetLog();

  OXTextEntry *_nameTE, *_logfileTE, *_backTE;
  OXCheckButton *_autoCB, *_transCB, *_raiseCB, *_beepCB;
  OXTextButton *_ok,*_cancel;
  OXButton *_fileBT, *_logfileBT;
  int *_ret;
  OChannelInfo *_ci;
  OLayoutHints *_l1, *_l2, *_l3, *_l4, *_l5, *_l6, *_l7;
};


#endif  // __OXCHANNELEDITOR_H
