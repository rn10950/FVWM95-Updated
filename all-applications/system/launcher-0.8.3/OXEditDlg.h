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

#ifndef __OXEDITDLG_H
#define __OXEDITDLG_H

#include <xclass/utils.h>
#include <xclass/OXTransientFrame.h>

class OXLabel;
class OXIcon;
class OXTextEntry;
class OXTextButton;
class SButton;


//---------------------------------------------------------------------

class OXEditButtonDialog : public OXTransientFrame {
public:
  OXEditButtonDialog(const OXWindow *p, const OXWindow *main,
                     SButton *button, int *retc = NULL,
                     unsigned long options = MAIN_FRAME | VERTICAL_FRAME);
  virtual ~OXEditButtonDialog();
  
  virtual int ProcessMessage(OMessage *msg);

  int UpdateButton();
  
protected:
  OXTextButton *Ok, *Cancel, *SetIcon;
  OXHorizontalFrame *bframe;
  OLayoutHints *bly, *bfly;
  
  OXIcon *icon1;
  OXLabel *lbl1, *lbl2, *lbl3, *lbl4;
  OXTextEntry *txt1, *txt2, *txt3, *txt4;
  OXHorizontalFrame *Ihf, *hf1, *hf2, *hf3, *hf4;
  OLayoutHints *Ily, *lyl, *lyr, *lyln, *lv, *llv;

  int *_retc;
  SButton *_button;
};

#endif   // __OXEDITDLG_H
