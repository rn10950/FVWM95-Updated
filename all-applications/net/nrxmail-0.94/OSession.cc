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

#include <limits.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/stat.h>
#include <netdb.h>
#include <string.h>

#include <xclass/utils.h>
#include <xclass/OIniFile.h>

#include "OSession.h"


//----------------------------------------------------------------------

OSession::OSession() {
  int  pos;
  char *host, *buffer, tmp[256];

  _clientVersion = NRX_VERSION;
  _folders = NULL;

  _folderpath = new char[strlen(getenv("HOME")) + strlen("/.nrxmail/") + 1];
  sprintf(_folderpath, "%s%s", getenv("HOME"), "/.nrxmail/");

  _initfile = new char[strlen(_folderpath) + strlen("nrxmail.ini") + 1];
  sprintf(_initfile, "%s%s", _folderpath, "nrxmail.ini");

  mkdir(_folderpath, S_IRWXU);	// ==!== check for the return value of this!

  gethostname(tmp, 256);
  struct hostent *hostname = gethostbyname(tmp);
  host = hostname->h_name;

  struct passwd *user = NULL;
  int user_id;
  user_id = (int) getuid();
  user = getpwuid(user_id);
  buffer = strchr(user->pw_gecos, (int) ',');

  if (buffer != NULL)
    pos = strlen(user->pw_gecos) - strlen(buffer);
  else
    pos = strlen(user->pw_gecos);

  if (pos > 0) {
    buffer = new char[pos];
    buffer[pos - 1] = '\0';
    strncpy(buffer, user->pw_gecos, pos);
  } else {
    buffer = new char[1];
    buffer[0] = '\0';
  }
  _identity = buffer;

  _returnpath = new char[strlen(user->pw_name) + strlen(host) + 2];
  sprintf(_returnpath, "%s@%s", user->pw_name, host);

  _signaturefile = new char[strlen(getenv("HOME")) + strlen("/.signature") + 1];
  sprintf(_signaturefile, "%s/.signature", getenv("HOME"));

  _smtpserver = StrDup("localhost");

  _smtpport = 25;

  _inbox = new char[strlen("/var/spool/mail/") + strlen(user->pw_name) + 1];
  sprintf(_inbox, "/var/spool/mail/%s", user->pw_name);

  _printer = StrDup("lp");
  _printcommand = StrDup("lpr");

  AddFolder("Inbox", IS_SYSTEM | IS_LINK, _inbox);
  AddFolder("Unsent", IS_SYSTEM);
  AddFolder("Sentmail", IS_SYSTEM);
  AddFolder("Trash", IS_SYSTEM);
}

OSession::OSession(char *inidir) {
  _clientVersion = NRX_VERSION;
  _folders = NULL;

  _folderpath = new char[strlen(getenv("HOME")) + strlen("/.nrxmail/") + 1];
  sprintf(_folderpath, "%s%s", getenv("HOME"), "/.nrxmail/");

  _initfile = new char[strlen(_folderpath) + strlen("/nrxmail.ini") + 1];
  sprintf(_initfile, "%s%s", _folderpath, "/nrxmail.ini");

  char line[INI_MAX_LINE_LEN];
  OIniFile *ini = new OIniFile(_initfile, INI_READ);

  do {
    ini->GetNext(line);
  } while (strcmp(line, "Configuration"));

  if (ini->GetItem("Identity", line)) {
    _identity = StrDup(line);
  } else {
    _identity = StrDup("");
  }

  ini->GetItem("Return-Path", line);
  _returnpath = StrDup(line);

  ini->GetItem("Inbox", line);
  _inbox = StrDup(line);

  ini->GetItem("SMTP-Server", line);
  _smtpserver = StrDup(line);

  ini->GetItem("SMTP-Port", line);
  _smtpport = atoi(line);

  ini->GetItem("Signature-File", line);
  _signaturefile = StrDup(line);

  ini->GetItem("Print-Command", line);
  _printcommand = StrDup(line);

  ini->GetItem("Printer", line);
  _printer = StrDup(line);

  char *name, *linkedFolder;
  int mode;

  while (ini->GetNext(line)) {
    name = new char[strlen(strchr(line, '/'))];  // ==!==
    strcpy(name, strchr(line, '/') + 1);
    ini->GetItem("Mode", line);
    mode = atoi(line);
    if (mode & IS_LINK) {
      ini->GetItem("Linked-Folder", line);
      linkedFolder = line;
    } else {
      linkedFolder = NULL;
    }
    ToFolderList(name, mode, linkedFolder);
    delete[] name;
    mode = 0;
  }

  delete ini;
}

OSession::~OSession() {
  delete[] _identity;
  delete[] _returnpath;
  delete[] _smtpserver;
  delete[] _signaturefile;
  delete[] _printcommand;
  delete[] _printer;
  delete[] _folderpath;
  delete[] _inbox;
  delete[] _initfile;

  OFolder *travel = _folders;
  while (travel != NULL) {
    _folders = travel->next;
    delete travel;
    travel = _folders;
  }
}

bool OSession::Save() {
  OIniFile *ini = new OIniFile(_initfile, INI_WRITE);

  ini->PutNext("Configuration");
  ini->PutItem("Identity", _identity);
  ini->PutItem("Return-Path", _returnpath);
  ini->PutItem("Inbox", _inbox);
  ini->PutItem("SMTP-Server", _smtpserver);
  ini->PutItem("SMTP-Port", itoa(_smtpport));
  ini->PutItem("Signature-File", _signaturefile);
  ini->PutItem("Printer", _printer);
  ini->PutItem("Print-Command", _printcommand);

  ini->PutNewLine();

  OFolder *travelfolder = _folders;
  char *buffer;
  while (travelfolder != NULL) {
    buffer = new char[strlen("Folder/") + strlen(travelfolder->GetName()) + 1];
    sprintf(buffer, "Folder/%s", travelfolder->GetName());
    ini->PutNext(buffer);
    ini->PutItem("Mode", itoa(travelfolder->mode));
    if (travelfolder->IsLink())
      ini->PutItem("Linked-Folder", (char *) travelfolder->GetLinkPath());
    ini->PutNewLine();
    travelfolder = travelfolder->next;
    delete[] buffer;
  }
  delete ini;
}

void OSession::ClearFolder(const char *name) {
  FILE *s, *d;
  char line[1025];
  char *spath;
  char *dpath;
  long count = 0;

  spath = AbsolutePath(name);
  if (strcmp(name, "Trash")) {
    dpath = AbsolutePath("Trash");
    s = fopen(spath, "r");   // ==!==
    d = fopen(dpath, "a");
    while (fgets(line, 1024, s) != NULL)
      fputs(line, d);
    fclose(s);
    fclose(d);
    delete dpath;
  }
  s = fopen(spath, "w");  // ==!==
  fclose(s);
  delete spath;
  OFolder *travel = GetFolder(name);
  travel->Reload();
}

char *OSession::AbsolutePath(const char *name) {
  char *abspath = new char[strlen(name) + strlen(_folderpath) + 2];
  sprintf(abspath, "%s%s", _folderpath, name);
  return abspath;
}

bool OSession::FolderNameExists(const char *name) {
  return (GetFolder(name) != NULL);
}

int OSession::GetFolderNumber(const char *name) {
  int count = 0;
  OFolder *travel = _folders;
  while ((travel != NULL) && strcmp(travel->GetName(), name)) {
    count++;
    travel = travel->next;
  }
  if (travel == NULL) return -1;
  return count;
}

char **OSession::GetFolderNames() {
  char **names = NULL;
  int count = 0;

  OFolder *travel = _folders;
  while (travel != NULL) {
    count++;
    travel = travel->next;
  }
  travel = _folders;
  names = new char *[count + 1];
  names[count] = NULL;
  count = 0;
  while (travel != NULL) {
    names[count] = StrDup(travel->GetName());
    count++;
    travel = travel->next;
  }
  return names;
}

bool OSession::AddFolder(const char *name, int mode,
                         const char *linkedFolder) {
  if (mode & IS_LINK)
    symlink(linkedFolder, AbsolutePath(name));

  if (!ToFolderList(name, mode, linkedFolder))
    return False;

  Save();

  return True;
}

bool OSession::ToFolderList(const char *name, int mode,
                            const char *linkedFolder) {
  OFolder *travel = _folders;

  if (_folders == NULL) {
    _folders = new OFolder(name, AbsolutePath(name), mode, linkedFolder);
  } else {
    while (travel->next != NULL) {
      if (!strcmp(travel->GetName(), name)) return False;
      travel = travel->next;
    }
    travel->next = new OFolder(name, AbsolutePath(name), mode, linkedFolder);
    travel->next->prev = travel;
  }
  return True;
}

bool OSession::DeleteFolder(const char *name) {
  OFolder *folder = GetFolder(name);

  if (folder == NULL) return False;

  unlink(AbsolutePath(folder->GetName()));

  if (folder->IsSystem()) return False;

  if (folder->prev != NULL) folder->prev->next = folder->next;
  if (folder->next != NULL) folder->next->prev = folder->prev;

  delete folder;

  Save();
}

OFolder *OSession::GetLastFolder() {
  OFolder *travel = _folders;

  if (_folders == NULL) return NULL;
  while (travel->next != NULL) travel = travel->next;

  return travel;
}

OFolder *OSession::GetFolder(const char *name) {
  OFolder *travel = _folders;

  while ((travel != NULL) && strcmp(travel->GetName(), name))
    travel = travel->next;

  return travel;
}

OFolder *OSession::GetFolder(long num) {
  OFolder *travel = _folders;
  long count = 0;

  while ((travel != NULL) && (count < num)) {
    travel = travel->next;
    count++;
  }

  return travel;
}

bool OSession::ChangeFolder(const char *name, const char *newname,
                            int mode, const char *linkedFolder) {
  char *oldname;
  OFolder *folder = GetFolder(name);

  if (folder == NULL) return False;

  oldname = StrDup(folder->GetName());
  if (strcmp(name, newname)) {
    if (folder->IsSystem()) {
      return False;
    } else {
      if (access(AbsolutePath(folder->GetName()), R_OK | W_OK) == 0) {
        rename(AbsolutePath(folder->GetName()), AbsolutePath(newname));
      }
      folder->SetName(newname);
    }
  }
  if (mode & IS_LINK) {
    if (folder->IsLink()) unlink(AbsolutePath(oldname));
    folder->mode |= IS_LINK;
    folder->SetLinkPath(linkedFolder);
    symlink(folder->GetLinkPath(), AbsolutePath(folder->GetName()));
  } else {
    if (folder->IsLink()) unlink(AbsolutePath(oldname));
    folder->mode &= ~IS_LINK;
    folder->SetLinkPath(NULL);
  }
  delete[] oldname;

  Save();

  delete[] folder->folder;
  folder->folder = new char[strlen(_folderpath) + strlen(folder->GetName()) + 1];
  sprintf(folder->folder, "%s%s", _folderpath, folder->GetName());

  folder->Reload();
}

void OSession::SetIdentity(const char *name) {
  delete[] _identity;
  _identity = StrDup(name);
}

void OSession::SetReturnPath(const char *returnpath) {
  delete[] _returnpath;
  _returnpath = StrDup(returnpath);
}

void OSession::SetSmtpServer(const char *smtpserver) {
  delete[] _smtpserver;
  _smtpserver = StrDup(smtpserver);
}

void OSession::SetSmtpPort(unsigned int smtpport) {
  _smtpport = smtpport;
}

void OSession::SetSignatureFile(const char *signaturefile) {
  delete[] _signaturefile;
  _signaturefile = StrDup(signaturefile);
}

void OSession::SetPrintCommand(const char *printcommand) {
  delete[] _printcommand;
  _printcommand = StrDup(printcommand);
}

void OSession::SetPrinter(const char *printer) {
  delete[] _printer;
  _printer = StrDup(printer);
}

void OSession::SetInBox(const char *inbox) {
  delete[] _inbox;
  _inbox = StrDup(inbox);
}

char *OSession::itoa(int number) {
  static char numString[40];
  long divider = 10;
  long count = 1;
  int i;

  while (number / divider) {
    count++;
    divider = divider * 10;
  }
  numString[count] = '\0';
  divider = 1;
  for (int x = 0; x < count - 1; x++)
    divider = divider * 10;
  for (i = 0; i < count - 1; i++) {
    numString[i] = '0' + number / divider;
    number = number - (number / divider) * divider;
    divider = divider / 10;
  }
  numString[i] = '0' + number % 10;

  return numString;
}
