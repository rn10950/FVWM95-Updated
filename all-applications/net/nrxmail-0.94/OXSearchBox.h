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

#ifndef __OXSEARCHBOX_H
#define __OXSEARCHBOX_H

#include <stdlib.h>
#include <stdio.h>

#include <xclass/OXTransientFrame.h>
#include <xclass/OXRadioButton.h>
#include <xclass/OXCheckButton.h>
#include <xclass/OXIcon.h>
#include <xclass/OXLabel.h>
#include <xclass/OXGroupFrame.h>
#include <xclass/OString.h>
#include <xclass/OXTextEntry.h>


//----------------------------------------------------------------------

typedef struct search_type {
  bool direction;
  bool caseSensitive;
  char *buffer;
} search_struct;

class OXSearchBox : public OXTransientFrame {
public:
  OXSearchBox(const OXWindow *p, const OXWindow *main, int w, int h,
              search_struct *sstruct, int *ret_val,
              unsigned long options = MAIN_FRAME | VERTICAL_FRAME);
  virtual ~OXSearchBox();
  
  virtual int ProcessMessage(OMessage *msg);

protected:
  OXCompositeFrame *f1, *f2, *f3, *f4, *f5;
  OXButton *SearchButton, *CancelButton;
  OXCheckButton *case_check;
  OLayoutHints *L1, *L2, *L3, *L4, *L5, *L6, *L21, *L9, *L10;
  OXTextEntry  *search;
  OTextBuffer  *tbsearch;
  OXLabel  *lsearch;
  search_struct *s;
  int *ret;
  const OPicture *searchPicture;
  OXIcon *searchIcon;
};


#endif  // __OXSEARCHBOX_H
