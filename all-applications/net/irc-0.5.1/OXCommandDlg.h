#ifndef __OXCOMMANDDLG_H
#define __OXCOMMANDDLG_H

#include <xclass/OXTransientFrame.h>
#include <xclass/OXTextEntry.h>
#include <xclass/OXTextButton.h>
#include <xclass/OString.h>


//----------------------------------------------------------------------

class OXCommandDlg : public OXTransientFrame {
public:
  OXCommandDlg(const OXWindow *p, const OXWindow *main,
               OString *wname, OString *text,
               OString *cmdlb, OString *command,
               OString *argslb, OString *args,
               int *retc = NULL);
  virtual ~OXCommandDlg();

  virtual int ProcessMessage(OMessage *msg);

protected:
  int *_retc;

  OXTextEntry *_cmdEntry, *_argsEntry;
  OString *_command, *_args;
  OLayoutHints *L1, *L2, *L3;
};


#endif  // __OXCOMMANDDLG_H
