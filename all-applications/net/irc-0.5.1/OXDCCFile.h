#ifndef __OXDCCFILE_H
#define __OXDCCFILE_H

#include <stdlib.h>

#include <xclass/OXFileDialog.h>
#include <xclass/OXTransientFrame.h>
#include <xclass/OXButton.h>
#include <xclass/OXLabel.h>

#include "OTcp.h"

#define ID_DCC_SAVE		2001
#define ID_DCC_SAVE_AS		2002
#define ID_DCC_REJECT		2003
#define ID_DCC_IGNORE		2004
#define ID_DCC_CANCEL		2005


//----------------------------------------------------------------------

class OXIrc;
class OXProgressBar;
class OFileHandler;

class OXDCCFileConfirm : public OXTransientFrame {
public:
  OXDCCFileConfirm(const OXWindow *p, const OXWindow *main,
                   const char *nick, const char *filename,
                   const char *size, OFileInfo *retn, int *reti);
  virtual ~OXDCCFileConfirm();

  virtual int ProcessMessage(OMessage *msg);

protected:
  OFileInfo *fi;
  int *iret;
  char *name;
  OLayoutHints *L1, *L2;
  OXButton *_ok, *_saveas, *_reject, *_ignore;
};


//----------------------------------------------------------------------

class OXDCCFile : public OXMainFrame {
public:
  OXDCCFile(const OXWindow *p, OXIrc *irc,
            const char *nick, const char *filename, char *ip,
            char *port, char *size = 0, int *retc = 0);
  OXDCCFile(const OXWindow *p, OXIrc *irc,
            const char *nick, const char *filename);
  virtual ~OXDCCFile();
  
  virtual int CloseWindow();
  virtual int HandleFileEvent(OFileHandler *fh, unsigned int mask);
  virtual int ProcessMessage(OMessage *msg);

  int Listen(unsigned long *host, unsigned short *port);

protected:
  bool _OpenFile(char *name);
  bool _FetchSomeData();
  bool _SendSomeData();

  OTcp *_tcp;
  OXIrc *_irc;
  OFileInfo fi;
  OFileHandler *_fh;
  OXProgressBar *_prog;
  OXButton *_cancel;
  OXLabel *_t1, *_t2, *_t3;
  OLayoutHints *L1, *L2;

  int _file, _mode, *_retc;
  bool _connected, _serverSocket;
  unsigned long _filesize, _bytesread, _bytessent, _acksize;
  char *_filename;
  char _dir[PATH_MAX];
};


#endif  // __OXDCCFILE_H
