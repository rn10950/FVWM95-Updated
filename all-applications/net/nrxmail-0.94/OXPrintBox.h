/**************************************************************************

    This file is part of NRX mail, a mail client using the XClass95 toolkit.
    Copyright (C) 1998 by Harald Radke.                 

    This application is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This application is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

**************************************************************************/

#ifndef __OXPRINTBOX_H
#define __OXPRINTBOX_H

#include <stdlib.h>
#include <stdio.h>

#include <xclass/OXTransientFrame.h>
#include <xclass/OXRadioButton.h>
#include <xclass/OXCheckButton.h>
#include <xclass/OXIcon.h>
#include <xclass/OXLabel.h>
#include <xclass/OXTextEntry.h>
#include <xclass/OString.h>


//----------------------------------------------------------------------

class OXPrintBox: public OXTransientFrame {                             
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
  OXTextEntry  *printer, *printCommand;
  OTextBuffer  *tbprinter, *tbprintCommand;
  OXLabel  *lprinter, *lprintCommand;
  int *ret;
};


#endif  // __OXPRINTBOX_H
