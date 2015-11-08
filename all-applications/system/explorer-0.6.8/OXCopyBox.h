#ifndef __OXCOPYBOX_H
#define __OXCOPYBOX_H

#include <xclass/utils.h>
#include <xclass/OXTransientFrame.h>
#include <xclass/OXLabel.h>
#include <xclass/OXIcon.h>
#include <xclass/OXProgressBar.h>
#include <xclass/OFileHandler.h>
#include <xclass/OTimer.h>
#include <xclass/OMessage.h>

//----------------------------------------------------------------------

class OXCopyBox : public OXTransientFrame {
public:
  OXCopyBox(const OXWindow *p, const OXWindow *t,
            const char *frompath, const char *topath);
  virtual ~OXCopyBox();
  
  virtual int CloseWindow();
  virtual int HandleMapNotify(XMapEvent *event);
  virtual int HandleFileEvent(OFileHandler *f, unsigned int mask);
  virtual int HandleTimer(OTimer *t);
  virtual int ProcessMessage(OMessage *msg);

protected:
  int FileError(const char *msg, const char *fname, int errc);

  OXLabel *_fnum, *_from, *_to;
  OXProgressBar *_pb;
  OFileHandler *_fread, *_fwrite;
  OTimer *_t;
  OLayoutHints *_lh1, *_lh2, *_lh3, *_lb, *_lp;

  int _init, _from_fd, _to_fd;
  unsigned long _fsize, _nread, _nwriten, _count, _bufsz;
  char *_from_path, *_to_path;
  char *_buf;
};


#endif  // __OXCOPYBOX_H
