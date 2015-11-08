#ifndef __OXPASSWDDLG_H
#define __OXPASSWDDLG_H

#include <xclass/OXTransientFrame.h>
#include <xclass/OXTextEntry.h>
#include <xclass/OXSecretTextEntry.h>
#include <xclass/OXTextButton.h>
#include <xclass/OString.h>


//----------------------------------------------------------------------

class OXPasswdDlg : public OXTransientFrame {
public:
  OXPasswdDlg(const OXWindow *p, const OXWindow *main, OString *wtitle,
              OString *text, OString *login, OString *pwd, int *retc = NULL);
  virtual ~OXPasswdDlg();

  virtual int ProcessMessage(OMessage *msg);

protected:
  int *_retc;

  OXTextEntry *_loginEntry;
  OXSecretTextEntry *_passwdEntry;
  OString *_login, *_passwd;
  OLayoutHints *L1, *L2, *L3;
};


#endif  // __OXPASSWDDLG_H
