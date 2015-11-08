/***************************************************************************/
/*									   */
/* ftplib.c - callable ftp access routines				   */
/* Copyright (C) 1996, 1997, 1998 Thomas Pfau, pfau@cnj.digex.net	   */
/*	73 Catherine Street, South Bound Brook, NJ, 08880		   */
/*									   */
/* This library is free software; you can redistribute it and/or	   */
/* modify it under the terms of the GNU Library General Public		   */
/* License as published by the Free Software Foundation; either		   */
/* version 2 of the License, or (at your option) any later version.	   */
/* 									   */
/* This library is distributed in the hope that it will be useful,	   */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of	   */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU	   */
/* Library General Public License for more details.			   */
/* 									   */
/* You should have received a copy of the GNU Library General Public	   */
/* License along with this progam; if not, write to the			   */
/* Free Software Foundation, Inc., 59 Temple Place - Suite 330,		   */
/* Boston, MA 02111-1307, USA.						   */
/* 									   */
/***************************************************************************/

#ifndef __OFTP_H
#define __OFTP_H

#include "OFileSystem.h"

#define SETSOCKOPT_OPTVAL_TYPE (const char *)

#define FTP_BUFSIZ	8192
#define ACCEPT_TIMEOUT	30


//----------------------------------------------------------------------

class OFtp : public OFileSystem {
public:
  OFtp();
  virtual ~OFtp();

  virtual int NeedConnect() { return true; }
  virtual int Noop();

  virtual int Connect(const char *host);
  virtual int Quit();
  virtual int Login(const char *user, const char *pass);
  virtual int Site(const char *cmd);
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
  struct ONetBuf {
    int cavail, cleft;
    OHandle handle;
    char response[256];
  };

  static void _Timeout(int);

  int _SetType(int mode);

  int _OpenPort(OHandle **nData, int mode);
  int _AcceptPort(OHandle *nData);
  int _ClosePort(OHandle *nData);

  int _SendCmd(const char *cmd, char expresp);

  int _readline(OHandle *nData, char *buf, int max);
  int _readresp(char c);

  ONetBuf *ctrl;
};

#endif  // __OFTP_H
