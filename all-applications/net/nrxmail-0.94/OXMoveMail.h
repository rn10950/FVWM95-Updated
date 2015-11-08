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

#ifndef __OXMOVEMAIL_H
#define __OXMOVEMAIL_H

#include <stdlib.h>
#include <stdio.h>

#include <xclass/OXTransientFrame.h>
#include <xclass/OXRadioButton.h>
#include <xclass/OXCheckButton.h>
#include <xclass/OXListBox.h>
#include <xclass/OString.h>


//----------------------------------------------------------------------

class OXMoveMail : public OXTransientFrame {
public:
  OXMoveMail(const OXWindow *p, const OXWindow *main, int w, int h,
             char **names, long *num, int *ret,
             unsigned long options = MAIN_FRAME | VERTICAL_FRAME);
  virtual ~OXMoveMail();

  virtual int ProcessMessage(OMessage *msg);

protected:
  int *ret; 
  long *num;
  OXCompositeFrame *f1;
  OXButton *MoveButton, *CancelButton;
  OXListBox *list; 
};


#endif  // __OXMOVEMAIL_H
