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

#ifndef __OFILESYSTEM_H
#define __OFILESYSTEM_H

#ifndef false
#define false 0
#endif
#ifndef true
#define true 1
#endif
 
#define FS_STATE_NOT_READY  1
#define FS_STATE_READY      2
 
class OFile;
class OXLogView;

struct OHandle {
public:
  int fd;
  int mode;
  char *buf;
};

#define MODE_DIR_LIST		(1 << 0)
#define MODE_DIR_NLST		(1 << 1)
#define MODE_CONN_PASSIVE	(1 << 2)
#define MODE_CONN_PORT		(1 << 3)
#define MODE_TRANS_ASCII	(1 << 4)
#define MODE_TRANS_IMAGE	(1 << 5)
#define MODE_TRANS_GET		(1 << 6)
#define MODE_TRANS_PUT		(1 << 7)

class OFileSystem {
public:
  OFileSystem();
  virtual ~OFileSystem();

  int  GetState() { return _state; }
  void SetLogObject(OXLogView *log) { _log = log; }

  virtual int NeedConnect() { return false; }
  virtual int Noop() { return true; }

  virtual int Connect(const char *host) { return true; }
  virtual int Quit() { return true; }
  virtual int Login(const char *user, const char *pass) { return true; }
  virtual int Site(const char *cmd) { return true; }
  virtual int SysType(char *buf, int max) = 0;
  virtual int Mkdir(const char *path) = 0;
  virtual int Chdir(const char *path) = 0;
  virtual int CDUp() = 0;
  virtual int Rmdir(const char *path) = 0;
  virtual int Pwd(char *path, int max) = 0;
  virtual int Size(const char *path, int *size, char mode) = 0;
  virtual int ModDate(const char *path, char *dt, int max) = 0;
  virtual int Rename(const char *src, const char *dst) = 0;
  virtual int Delete(const char *fnm) = 0;
  virtual char *LastResponse() = 0;

  virtual OHandle *OpenDir(const char *path, const char *args, int mode) = 0;
  virtual OHandle *OpenFile(const char *file, int mode) = 0;
  virtual int GetFile(OHandle *handle, OFile *&elem) = 0;
  virtual int Close(OHandle *handle) = 0;

protected:
  void Log(const char *txt, const char *string);
 
  char *_pwd;
  int _state;
  OXLogView *_log;
};

#endif  // __OFILESYSTEM_H
