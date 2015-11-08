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

#ifndef __OMAILLIST_H
#define __OMAILLIST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "OMail.h"

#define SORT_DATE	1
#define SORT_FROM	2
#define SORT_SUBJECT	3


//----------------------------------------------------------------------

class OMailList {
public:
  OMailList(const char *folderName);
  OMailList();
  ~OMailList();

  void Clear();
  bool Reload();
  OMail *GetMail(unsigned long num);
  OMail *GetFirstMail() const { return mails; }
  OMail *GetNextMail(OMail *mail) { return mail->next; }
  void EraseMail(char *id);
  void MoveMail(char *id, char *newfolder);
  char *GetFolder() const { return folder; }
  unsigned long GetNumberOfMails() const { return numberOfMails; }
  void Sort(int mode);

protected:
  bool InitMails();

  char *folder;
  OMail *mails;
  unsigned long numberOfMails;
};


#endif  // __OMAILLIST_H
