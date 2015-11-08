/**************************************************************************

    This file is part of explorer95, a file manager for fvwm95.
    Copyright (C) 1996, 1997 David Barth, Hector Peraza.

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

#ifndef __OXOPTIONS_H
#define __OXOPTIONS_H


#include <xclass/OXClient.h>
#include <xclass/OXWindow.h>
#include <xclass/OXTransientFrame.h>
#include <xclass/OXTextButton.h>
#include <xclass/OXRadioButton.h>
#include <xclass/OXCheckButton.h>
#include <xclass/OXGroupFrame.h>
#include <xclass/OXLabel.h>
#include <xclass/OXListBox.h>
#include <xclass/OXIcon.h>
#include <xclass/OX3dLines.h>
#include <xclass/OString.h>
#include <xclass/OPicture.h>
#include <xclass/utils.h>


//---------------------------------------------------------------------

class OXOptionsDialog : public OXTransientFrame {
public:
  OXOptionsDialog(const OXWindow *p, const OXWindow *main,
                  unsigned long options = MAIN_FRAME | VERTICAL_FRAME);
  virtual ~OXOptionsDialog();

  virtual int ProcessMessage(OMessage *msg);

protected:
  void UpdateListBox();

  //=== dialog widgets and frames:

  //--- "OK Cancel Apply" buttons:
  OXButton *Ok, *Cancel, *Apply;
  OLayoutHints *bly, *bfly;

  OLayoutHints *Ltab;

  //--- Tab contents:
  //--- tab 1
  OXRadioButton *rad1, *rad2;
  OLayoutHints *Lr, *Lly, *Ily, *Gly;

  //--- tab 2
  OXCheckButton *chk1, *chk2, *chk3;
  OLayoutHints *L3, *L4, *L6;

  //--- tab 3
  OXTextButton *New, *Remove, *Edit;
  OXLabel *lblex1, *lblex2, *lblop1, *lblop2;
  OXGroupFrame *gf3;
  OXListBox *lbf;
  OXIcon *icf;
  OLayoutHints *lbly, *gfly, *lgf3, *lfh, *lex1,
               *nbly, *nbfly, *lshf, *lfd;

  int _NewBrowser, _DispFullPath;
};


#endif   // __OXOPTIONS_H
