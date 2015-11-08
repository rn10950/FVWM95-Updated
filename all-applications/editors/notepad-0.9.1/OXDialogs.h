/**************************************************************************

    This file is part of notepad, a simple text editor.
    Copyright (C) 1997-2001, Harald Radke, Hector Peraza.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

**************************************************************************/

#ifndef __OXDIALOGS_H
#define __OXDIALOGS_H


#include <stdlib.h>
#include <stdio.h>

#include <xclass/utils.h>
#include <xclass/OXWindow.h>
#include <xclass/OXTransientFrame.h>
#include <xclass/OXLabel.h>
#include <xclass/OXIcon.h>
#include <xclass/OXTextEntry.h>
#include <xclass/OXCheckButton.h>
#include <xclass/OXRadioButton.h>
#include <xclass/OXGroupFrame.h>
#include <xclass/OXMsgBox.h>
#include <xclass/OString.h>

struct search_struct {
  bool direction;
  bool caseSensitive;
  char *buffer;
};


//----------------------------------------------------------------------

class OXGotoBox : public OXTransientFrame {
public:
  OXGotoBox(const OXWindow *p, const OXWindow *main,
            int w, int h, long *ret_val,
	    unsigned long options = MAIN_FRAME | VERTICAL_FRAME);
  virtual ~OXGotoBox();

  virtual int ProcessMessage(OMessage *msg);

protected:
  OXCompositeFrame *f1, *f2;
  OXButton *GotoButton, *CancelButton;
  OLayoutHints *L1, *L5, *L6, *L21;
  OXTextEntry *goTo;
  OTextBuffer *tbgoTo;
  long *ret;
};

class OXSetTabsBox : public OXTransientFrame {
public:
  OXSetTabsBox(const OXWindow *p, const OXWindow *main,
               int w, int h, int *ret_val,
	       unsigned long options = MAIN_FRAME | VERTICAL_FRAME);
  virtual ~OXSetTabsBox();

  virtual int ProcessMessage(OMessage *msg);

protected:
  OXCompositeFrame *f1, *f2;
  OXButton *OkButton, *CancelButton;
  OLayoutHints *L1, *L5, *L6, *L21;
  OXTextEntry *_te;
  OTextBuffer *_tb;
  int *ret;
};

class OXSearchBox : public OXTransientFrame {
public:
  OXSearchBox(const OXWindow *p, const OXWindow *main,
              int w, int h, search_struct *sstruct, int *ret_val,
	      unsigned long options = MAIN_FRAME | VERTICAL_FRAME);
  virtual ~OXSearchBox();

  virtual int ProcessMessage(OMessage *msg);

protected:
  OXCompositeFrame *f1, *f2, *f3, *f4;
  OXButton *SearchButton, *CancelButton;
  OXRadioButton *direction_radio[2];
  OXCheckButton *case_check;
  OXGroupFrame *g1, *g2;
  OLayoutHints *L1, *L2, *L3, *L4, *L5, *L6, *L21, *L9, *L10;
  OXTextEntry *search;
  OTextBuffer *tbsearch;
  OXLabel *lsearch;
  search_struct *s;
  int *ret;
};

class OXPrintBox : public OXTransientFrame {
public:
  OXPrintBox(const OXWindow *p, const OXWindow *main, int w, int h,
	     char **printerName, char **printProg, int *ret_val,
	     unsigned long options = MAIN_FRAME | VERTICAL_FRAME);
  virtual ~OXPrintBox();

  virtual int ProcessMessage(OMessage *msg);

protected:
  char **pprinter, **pprintCommand;
  OXCompositeFrame *f1, *f2, *f3, *f4, *f5;
  const OPicture *printerPicture;
  OXIcon *printerIcon;
  OXButton *PrintButton, *CancelButton;
  OLayoutHints *L1, *L2, *L3, *L5, *L6, *L21;
  OXTextEntry *printer, *printCommand;
  OTextBuffer *tbprinter, *tbprintCommand;
  OXLabel *lprinter, *lprintCommand;
  int *ret;
};

#endif  // __OXDIALOGS_H
