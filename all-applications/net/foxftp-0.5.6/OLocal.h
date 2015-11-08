/**************************************************************************

    This file is part of foxftp, a ftp client for fOX.
    Copyright (C) 1998, 1999, G. Matzka.

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

#ifndef __OLOCAL_H
#define __OLOCAL_H

#include <dirent.h>

#include "OFileSystem.h"

class OFileElem;


//----------------------------------------------------------------------

class OLocal : public OFileSystem {
public:
  OLocal();
  virtual ~OLocal();

  virtual int SysType(char *buf, int max);
  virtual int Mkdir(const char *path);
  virtual int Chdir(const char *path);
  virtual int CDUp();
  virtual int Rmdir(const char *path);
  virtual int Pwd(char *path, int max);
  virtual int Size(const char *path, int *size, char mode);
  virtual int ModDate(const char *path, char *dt, int max);
  virtual int Rename(const char *src, const char *dst);
  virtual int Delete(const char *fnm);
  virtual char *LastResponse();

  virtual OHandle *OpenDir(const char *path, const char *args, int mode);
  virtual OHandle *OpenFile(const char *file, int mode);
  virtual int GetFile(OHandle *handle, OFile *&elem);
  virtual int Close(OHandle *handle);

protected:
  DIR *_dp;
  char *_dpath;
  int _last_errno;
};

#endif  // __OLOCAL_H
