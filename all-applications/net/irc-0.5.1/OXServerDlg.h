/**************************************************************************

    This file is part of foxirc, a cool irc client.
    Copyright (C) 1996, 1997 R. Suescun, Hector Peraza.

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
  
#ifndef __OXSERVERDLG_H
#define __OXSERVERDLG_H

#include <xclass/OXClient.h>
#include <xclass/O2ColumnsLayout.h>
#include <xclass/OXTransientFrame.h>
#include <xclass/OXTextButton.h>
#include <xclass/OXLabel.h>
#include <xclass/OXTextEntry.h>
#include <xclass/OXComboBox.h>
#include <xclass/OX3dLines.h>
#include <xclass/OXMsgBox.h>
#include <xclass/OString.h>
#include <xclass/utils.h>

#include "OXPreferences.h"


//---------------------------------------------------------------------

class OXServerDlg : public OXTransientFrame {
public:
  OXServerDlg(const OXWindow *p, const OXWindow *main, OString *title,
              OServerInfo *info = NULL, int *ret_code = NULL,
              int connect = True,
              unsigned long options = MAIN_FRAME | VERTICAL_FRAME);
  virtual ~OXServerDlg();

  virtual int ProcessMessage(OMessage *msg);

protected:
  OLayoutHints *L1, *L2, *L3, *L4, *L5;
  OXButton *OK, *Cancel;
  OXComboBox *_name;
  OXTextEntry *_nameedit, *_hname, *_port, *_psw, *_nick, *_uname,
              *_opnick, *_oppsw;
  int *_ret_code, _connect;
  OServerInfo *_info;
};


#endif  // __OXSERVERDLG_H
