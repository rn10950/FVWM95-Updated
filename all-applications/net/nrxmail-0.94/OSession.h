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

#ifndef __OSESSION_H
#define __OSESSION_H

#include <stdio.h>
#include <stdlib.h>

#include <X11/Xlib.h>

#include "OFolder.h"

#define NRX_VERSION  "0.94dev"


//----------------------------------------------------------------------

class OSession {
public:
  OSession();
  OSession(char *inifile);
  ~OSession();

  bool Save();
  bool AddFolder(const char *name, int mode = 0,
                 const char *linkedFolder = NULL);
  bool DeleteFolder(const char *name);
  bool ChangeFolder(const char *name, const char *newname,
                    int mode = 0, const char *linkedFolder = NULL); 
  OFolder *GetFolder(const char *name);
  OFolder *GetFolder(long num);
  char **GetFolderNames();
  bool FolderNameExists(const char *name);
  void SetIdentity(const char *name);
  void SetReturnPath(const char *returnpath);
  void SetSmtpServer(const char *smtpserver);
  void SetSmtpPort(unsigned int smtpport);
  void SetSignatureFile(const char *signaturefile);
  void SetPrintCommand(const char *printcommand);
  void SetPrinter(const char *printer);
  void SetInBox(const char *inbox);
  OFolder *GetLastFolder();
  const char *GetClientVersion() { return _clientVersion; }
  char *GetIdentity() { return _identity; }
  char *GetReturnPath() { return _returnpath; }
  char *GetSmtpServer() { return _smtpserver; }
  unsigned int GetSmtpPort() { return _smtpport; }
  char *GetSignatureFile() { return _signaturefile; }
  char *GetPrintCommand() { return _printcommand; }
  char *GetPrinter() { return _printer; }
  char *GetFolderPath() { return _folderpath; }
  char *GetInBox() { return _inbox; }
  char *GetInitFile() { return _initfile; }
  char *AbsolutePath(const char *name);
  int GetFolderNumber(const char *name);
  void ClearFolder(const char *name);
  char *itoa(int num);
  
protected:
  bool ToFolderList(const char *name, int mode = 0,
                    const char *linkedFolder = NULL);

  OFolder *_folders;
  const char *_clientVersion;
  char *_identity;
  char *_returnpath;
  char *_smtpserver;
  unsigned int _smtpport;
  char *_signaturefile;
  char *_printcommand;
  char *_printer;
  char *_folderpath;
  char *_inbox;
  char *_initfile;
};

#endif  // __OSESSION_H
