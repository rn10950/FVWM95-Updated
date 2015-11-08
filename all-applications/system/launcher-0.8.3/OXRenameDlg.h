/**************************************************************************

    This file is part of xclauncher.
    Copyright (C) 1998-2002 Hector Peraza.

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
  
#ifndef __OXRENAMEDLG_H
#define __OXRENAMEDLG_H

#include <xclass/utils.h>
#include <xclass/OXClient.h>
#include <xclass/OXTransientFrame.h>
#include <xclass/OXTextButton.h>
#include <xclass/OXLabel.h>
#include <xclass/OXTextEntry.h>
#include <xclass/OX3dLines.h>
#include <xclass/OXMsgBox.h>
#include <xclass/OString.h>


//---------------------------------------------------------------------

class OXRenameDlg : public OXTransientFrame {
public:
  OXRenameDlg(const OXWindow *p, const OXWindow *main,
              OString *wname, OString *title, OString *text,
              OString *msg = NULL, int *ret_code = NULL,
              unsigned long options = MAIN_FRAME | VERTICAL_FRAME);
  virtual ~OXRenameDlg();

  virtual int CloseWindow();
  virtual int ProcessMessage(OMessage *msg);

protected:
  OLayoutHints *L1, *L2, *L3, *L4, *L5;
  OXButton *Ok, *Cancel;
  OXTextEntry *msge;
  int *_ret_code;
  OString *_msg;
};


#endif  // __OXRENAMEDLG_H
