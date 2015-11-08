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

#ifndef __OFOLDER_H
#define __OFOLDER_H

#include <stdio.h>
#include <stdlib.h>

#include <X11/Xlib.h>

#include "OMailList.h"

#define NAME_EXISTS     1
#define PATH_EXISTS     2

#define IS_SYSTEM       (1<<1)
#define IS_LINK         (1<<2)


class OSession;
class OXMail;


//----------------------------------------------------------------------

class OFolder : public OMailList {
public:
  OFolder(const char *newname, const char *folderpath, int newmode,
          const char *newlinkpath); 
  ~OFolder();

  bool  IsSystem() { return (mode & IS_SYSTEM); }
  bool  IsLink() { return (mode & IS_LINK); }
  const char *GetName() const { return name; }
  void  SetName(const char *newname);
  const char *GetLinkPath() const { return linkpath; }
  void  SetLinkPath(const char *newlinkpath);
  int   GetMode() { return mode; }

  friend class OSession;
  friend class OXMail;

protected:
  char *path; 
  char *name; 
  char *linkpath;
  int  mode;

  OFolder *prev, *next;   
};


#endif  // __OFOLDER_H
