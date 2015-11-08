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

#ifndef __OXMAILLIST_H
#define __OXMAILLIST_H

#include <xclass/OPicture.h>
#include <xclass/OXListView.h>

#include "OFolder.h"


#define XC_MAIL	        91
#define MAIL_SELECTED   1

#define NEW_MAIL	1
#define UNREAD_MAIL	2
#define READ_MAIL	3


//----------------------------------------------------------------------

class OXMailList : public OXListView {
public:
  OXMailList(const OXWindow *p, int w, int h, OFolder *folder = NULL);
  virtual ~OXMailList();

  void ReloadList();
  void LoadList(OFolder *);
  OMail *GetCurrentMail();
  int GetCurrentMailNum();

protected:
  int _currentMail;
  OFolder *_folder;
};

#endif  // __OXMAILLIST_H
