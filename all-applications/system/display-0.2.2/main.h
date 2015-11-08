#ifndef __MAIN_H
#define __MAIN_H

#include <xclass/OXClient.h>
#include <xclass/OXWindow.h>
#include <xclass/OXTransientFrame.h>
#include <xclass/OXTab.h>
#include <xclass/OXTextButton.h>
#include <xclass/OMessage.h>
#include <xclass/utils.h>


#define MSG_FVWM      (MSG_USERMSG + 100)
#define MSG_RECEIVED  12
#define MSG_SEND      34


//----------------------------------------------------------------------

class OFvwmMessage : public OWidgetMessage {
public:
  OFvwmMessage(int typ, int act, int id,
               unsigned long t, unsigned long *b = NULL,
               const char *txt = NULL) :
    OWidgetMessage(typ, act, id) { ftype = t; body = b; text = txt; }

public:
  unsigned long ftype, *body;
  const char *text;
};

//----------------------------------------------------------------------

class OXMain : public OXMainFrame {
public:
  OXMain(const OXWindow *p, int w, int h, char *modname, int inp, int outp);
  virtual ~OXMain();

  virtual int HandleFileEvent(OFileHandler *fh, unsigned int mask);
  virtual int ProcessMessage(OMessage *msg);

  int  ReadFvwmPacket(unsigned long *header, unsigned long **body);
  void SendFvwmPipe(const char *message, unsigned long window = 0);
  void SendFvwmText(const char *message, unsigned long window = 0);
  void ProcessFvwmMessage(unsigned long type, unsigned long *body);

protected:
  void AddTabs();

  OXTab *_tab;
  OLayoutHints *TabLayout, *TabItemLayout;

  int _ntabs;
  OXFrame **_tabs;

  OXButton *OKButton, *CancelButton, *ApplyButton;
  OLayoutHints *ButtonLayout, *ButtonFrameLayout;

  const OPicture *Energy_Star, *Monitor;

  char *_moduleName;
  int _pipein, _pipeout;
  OFileHandler *_fvwmPipe;
};


#endif  // __MAIN_H
