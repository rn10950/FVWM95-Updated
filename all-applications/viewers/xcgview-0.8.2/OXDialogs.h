#ifndef __OXDIALOGS_H
#define __OXDIALOGS_H

#include <stdlib.h>
#include <stdio.h>

#include <xclass/OXClient.h>
#include <xclass/OXWindow.h>
#include <xclass/OXTransientFrame.h>
#include <xclass/OXTextButton.h>
#include <xclass/OXRadioButton.h>
#include <xclass/OXIcon.h>
#include <xclass/OXLabel.h>
#include <xclass/OXTextEntry.h>
#include <xclass/OX3dLines.h>
#include <xclass/OXMsgBox.h>
#include <xclass/OString.h>
#include <xclass/utils.h>

#include "OXTextView.h"


#define PB_OK           (1<<0)
#define PB_CANCEL       (1<<1)
#define PB_PRINTALL     (1<<2)
#define PB_PRINTMARKED  (1<<3)


//---------------------------------------------------------------------

class OXOutput : public OXTransientFrame {
public:
  OXOutput(const OXWindow *p, const OXWindow *t, int w, int h);
  virtual ~OXOutput();

  virtual int ProcessMessage(OMessage *msg);
  virtual int CloseWindow();

  void AddText(char *text) { _text->AddText(text); }
  void Clear() { _text->Clear(); }
  int printf(char *fmt, ...);

protected:
  OXTextView *_text;
  OXTextButton *_closeButton;
};

//---------------------------------------------------------------------

class OXPrintBox: public OXTransientFrame {
public:
  OXPrintBox(const OXWindow *p, const OXWindow *main, int w, int h,
             char **printerName, char **printProg, int *ret_val,
             unsigned long options = MAIN_FRAME | VERTICAL_FRAME);
  virtual ~OXPrintBox();

  virtual int ProcessMessage(OMessage *msg);

protected:
  char **pprinter, **pprintCommand;
  int *ret;   

  OXButton *PrintButton, *CancelButton;
  OXCompositeFrame *f1, *f2, *f3, *f4, *f5, *f6;
  const OPicture *printerPicture;
  OXIcon *printerIcon;

  OLayoutHints *L1, *L2, *L3, *L4, *L5, *L6, *L7, *L21;
  OXTextEntry  *printer, *printCommand;   
  OTextBuffer  *tbprinter, *tbprintCommand;
  OXLabel  *lprinter, *lprintCommand; 

  OXHorizontal3dLine *_hline;
  OXRadioButton *_all, *_marked;
};


#endif  // __OXDIALOGS_H
