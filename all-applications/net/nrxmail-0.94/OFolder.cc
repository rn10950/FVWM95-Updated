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

#include <string.h>
#include <unistd.h>

#include <xclass/utils.h>

#include "OFolder.h"


//----------------------------------------------------------------------

OFolder::OFolder(const char *newname, const char *folderpath, int newmode,
                 const char *newlinkpath) : OMailList(folderpath) {

  name = StrDup(newname);

  next = prev = NULL;
  mode = newmode;

  linkpath = (mode & IS_LINK) ? StrDup(newlinkpath) : (char *) NULL;
}

OFolder::~OFolder() {
  delete[] name;
  if (linkpath) delete[] linkpath;
}

void OFolder::SetName(const char *newname) {
  if (newname) {
    delete[] name;
    name = StrDup(newname);
  }
}

void OFolder::SetLinkPath(const char *newlinkpath) {
  if (linkpath) delete[] linkpath;
  linkpath = newlinkpath ? StrDup(newlinkpath) : (char *) NULL;
}
