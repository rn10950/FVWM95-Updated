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

#ifndef __OXFOLDERPROP_H
#define __OXFOLDERPROP_H

#include <stdlib.h>
#include <stdio.h>

#include <xclass/OXTransientFrame.h>
#include <xclass/OXRadioButton.h>
#include <xclass/OXCheckButton.h>
#include <xclass/OXIcon.h>
#include <xclass/OXLabel.h>
#include <xclass/OXGroupFrame.h>
#include <xclass/OX3dLines.h>
#include <xclass/OString.h>

#include "OSession.h"
#include "OFolder.h"

//----------------------------------------------------------------------

class OXFolderProp : public OXTransientFrame {
public:
  OXFolderProp(const OXWindow *p, const OXWindow *main, int w, int h,
               int *ret_val, OSession *session, OFolder *folder = NULL, 
               unsigned long options = MAIN_FRAME | VERTICAL_FRAME);
  virtual ~OXFolderProp();
  
  virtual int ProcessMessage(OMessage *msg);

protected:
  void SetLinkPath();
  bool CmpName(char *name, char **namelist);

  int *ret;
  bool hasChanged;
  int _mode;

  OSession *_session;
  OFolder *_folder;

  OXLabel *filelabel, *namelabel;
  OXHorizontal3dLine *line;
  OXCompositeFrame *f1, *f3, *f4, *f5, *f6, *f7, *bf1, *bf2;
  OXButton *OkButton, *CancelButton, *BrowseButton;
  OXRadioButton *regFolder, *linkToFolder;
  OXGroupFrame *ftype, *info;
  OXTextEntry  *name, *linkpath;
  OTextBuffer  *tname, *tlinkpath;
};


#endif  // __OXFOLDERPROP_H
