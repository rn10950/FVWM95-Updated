/**************************************************************************

    This file is part of xcpaint, a pixmap editor for xclass
    Copyright (C) 1997, Hector Peraza.

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

#include <xclass/OXWindow.h>
#include <xclass/OXTransientFrame.h>
#include <xclass/O2ColumnsLayout.h>
#include <xclass/OXTextButton.h>
#include <xclass/OXRadioButton.h>
#include <xclass/OXTextEntry.h>
#include <xclass/OXLabel.h>
#include <xclass/OXMsgBox.h>
#include <xclass/OString.h>


#define DIALOG_RESIZE     0
#define DIALOG_RESCALE    1
#define DIALOG_NEW_IMAGE  2


//---------------------------------------------------------------------

class OXSizeDialog : public OXTransientFrame {
public:
  OXSizeDialog(const OXWindow *p, const OXWindow *main, int w, int h,
               unsigned long options = MAIN_FRAME | VERTICAL_FRAME,
               int dlg_type = DIALOG_RESIZE,
               int *retw = NULL, int *reth = NULL);
  virtual ~OXSizeDialog();

  virtual int CloseWindow();
  virtual int ProcessMessage(OMessage *msg);

protected:
  OXCompositeFrame *_f1, *_f2;
  OLayoutHints *_l1, *_l2, *_l3;
  OXButton *_okButton, *_cancelButton;
  OXRadioButton *_r1, *_r5, *_r10;
  OXTextEntry *_widthTE, *_heightTE;
  OXLabel *_widthLabel, *_heightLabel, *_zoomLabel;

  int _dlgType, *ret_width, *ret_height;
};


//---------------------------------------------------------------------

class OXTextDialog : public OXTransientFrame {
public:
  OXTextDialog(const OXWindow *p, const OXWindow *main, char *rtxt = NULL, 
               unsigned long options = MAIN_FRAME | VERTICAL_FRAME);
  virtual ~OXTextDialog();

  virtual int CloseWindow();
  virtual int ProcessMessage(OMessage *msg);

protected:
  OXCompositeFrame *_f1, *_f2;
  OLayoutHints *_l1, *_l2, *_l3;
  OXButton *_okButton, *_cancelButton;
  OXTextEntry *_textTE;
  OXLabel *_textLabel;

  char *ret_text;
};


#endif  // __OXDIALOGS_H
