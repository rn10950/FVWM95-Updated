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

#ifndef __OXPREFERENCES_H
#define __OXPREFERENCES_H

#include <xclass/OXTransientFrame.h>
#include <xclass/OXLabel.h>
#include <xclass/OXTextEntry.h>
#include <xclass/OXTextButton.h>
#include <xclass/OXTab.h>

#include "OSession.h"

//----------------------------------------------------------------------

class OXPreferences : public OXTransientFrame {
public:
  OXPreferences(const OXWindow *p, const OXWindow *main,
                int w, int h, OSession *session, int *ret,
                unsigned long options = MAIN_FRAME | VERTICAL_FRAME);
  virtual ~OXPreferences();
  
  virtual int ProcessMessage(OMessage *msg);

protected:
  int *_ret;
  OSession *_session;
  OXTab *_tab;
  char *itoa(int num);
  void SaveData();

  OXCompositeFrame *_mailOptions, *_viewerOptions;
  OXCompositeFrame *_f1, *_f2, *_f3, *_f4, *_f5, *_f6, *_f7;
  OXCompositeFrame *_f8, *_f9, *_f10;
  OXCompositeFrame *_bf1, *_bf2;
  OLayoutHints *_frameLayout, *_frametLayout, *_framebLayout,
               *_labelLayout, *_teLayout;
  OXTextEntry *_realName, *_returnPath, *_smtpServer, *_smtpPort, *_inbox;
  OXTextEntry *_printer, *_printCommand, *_signature;

  OXLabel *_lrealName, *_lreturnPath, *_lsmtpServer, *_lsmtpPort, *_linbox;
  OXLabel *_lprinter, *_lprintCommand, *_lsignature;

  OTextBuffer *_tbrealName, *_tbreturnPath, *_tbsmtpServer, *_tbsmtpPort,
              *_tbinbox;
  OTextBuffer *_tbprinter, *_tbprintCommand, *_tbsignature;
  OXTextButton *_okButton, *_cancelButton;
};

#endif  // __OXPREFERENCES_H
