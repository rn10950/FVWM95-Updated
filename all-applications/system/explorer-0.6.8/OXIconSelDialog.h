#ifndef __OXICONSELDIALOG_H
#define __OXICONSELDIALOG_H

#include <xclass/utils.h>
#include <xclass/OXFileDialog.h>
#include <xclass/OXLabel.h>
#include <xclass/OXIcon.h>
#include <xclass/OMessage.h>


//----------------------------------------------------------------------

class OXIconSelDialog : public OXFileDialog {
public:
  OXIconSelDialog(const OXWindow *p, const OXWindow *t, OString *ret);
  virtual ~OXIconSelDialog();
  
  virtual int ProcessMessage(OMessage *msg);
  
protected:
  OXLabel *_name, *_size;
  OXIcon *_smallIcon, *_bigIcon;
  const OPicture *_small, *_big;
  OLayoutHints *_lh1, *_lh2, *_lh3, *_lb, *_lp;
  OPicturePool *_picPool;
};


#endif  // __OXICONSELDIALOG_H
