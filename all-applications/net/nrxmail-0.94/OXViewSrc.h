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

#ifndef __OXVIEWSRC_H
#define __OXVIEWSRC_H

#include <stdlib.h>
#include <stdio.h>

#include <xclass/OXTransientFrame.h>
#include <xclass/OXRadioButton.h>
#include <xclass/OXCheckButton.h>
#include <xclass/OXIcon.h>
#include <xclass/OXGroupFrame.h>
#include <xclass/OXMsgBox.h>
#include <xclass/OString.h>
#include <xclass/OXMenu.h>
#include <xclass/OX3dLines.h>
#include <xclass/OXTextEdit.h>

#include "OSession.h"


#define MVS_FILE_SAVE             1
#define MVS_FILE_PRINT            2
#define MVS_FILE_EXIT             3

#define MVS_EDIT_COPY             4
#define MVS_EDIT_SELECTALL        5
#define MVS_EDIT_SEARCH           6
#define MVS_EDIT_SEARCHAGAIN      7

#define MVS_HELP_CONTENTS         8
#define MVS_HELP_SEARCH           9
#define MVS_HELP_ABOUT           10

//----------------------------------------------------------------------

class OXViewSrc : public OXTransientFrame {
public:
  OXViewSrc(const OXWindow *p, const OXWindow *main, int w, int h,
            char *printercommand, char *printer, OText *text,
            unsigned long options = MAIN_FRAME | VERTICAL_FRAME);
  virtual ~OXViewSrc();

  virtual int ProcessMessage(OMessage *msg);

protected:
  char *_printercommand, *_printer;

  OXTextEdit *tv;
  OLayoutHints *_menuBarItemLayout, *_menuBarLayout;
  OXMenuBar *_menuBar;
  OXPopupMenu *_menuFile, *_menuEdit, *_menuHelp;

  void SaveSource();
  void PrintSource();
  void InitMenu(); 
};


#endif  // __OXVIEWSRC_H
