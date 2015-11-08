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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>

#include <xclass/utils.h>

#include "OFile.h"
#include "OFileSystem.h"
#include "OLocal.h"


//----------------------------------------------------------------------

OLocal::OLocal() : OFileSystem() {
  _state = FS_STATE_READY;
  _dp = NULL;
  _dpath = NULL;
  _pwd[0] = '\0';
  _last_errno = 0;
}

OLocal::~OLocal() {
  if (_dpath) delete[] _dpath;
}

int OLocal::SysType(char *buf, int max) {
  _last_errno = 0;
  strcpy(buf, "UNIX");
  return true;
}

int OLocal::Pwd(char *pwd, int max) {
  _last_errno = 0;
  if (strlen(_pwd) != 0) {
    strcpy(pwd, _pwd);
    return true;
  }
  if (getcwd(pwd, max) == NULL) {
    _last_errno = errno;
    return false;
  }
  strcpy(_pwd, pwd);

  return true;
}

int OLocal::Chdir(const char *cwd) {
  char buf[PATH_MAX];

  const char *p1;
  struct passwd *pwd;
  if (cwd[0] == '~') {
    if (cwd[1] != '/' && cwd[1] != '\0') {
      // a user
      p1 = strchr(cwd, '/');
      if (p1) { 
        strncpy(buf, &cwd[1], p1-cwd-1);
        buf[p1-cwd-1] = '\0';
      } else {
        strcpy(buf, &cwd[1]);
      }
      pwd = getpwnam(buf);
      if (!pwd) return false;
      strcpy(buf, pwd->pw_dir);
      if (p1) strcat(buf, p1);
    } else {
      p1 = getenv("HOME");
      if (p1) {
        strcpy(buf, p1);
        strcat(buf, &cwd[1]);
      } else {
        return false;
      }
    }
  } else {
    strcpy(buf, cwd);
  }

  _last_errno = 0;
  _pwd[0] = '\0';

  if (chdir(buf) != 0) return false;

  return true;
}

int OLocal::CDUp() {
  _last_errno = 0;
  if (chdir("..") != 0) {
    _last_errno = errno;
    return false;
  }
  return true;
}

int OLocal::Mkdir(const char *path) {
  _last_errno = 0;
  if (mkdir(path, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH ) != 0) {
    _last_errno = errno;
    return false;
  }
  return true;
}

int OLocal::Rmdir(const char *path) {
  _last_errno = 0;
  if (rmdir(path) != 0) {
    _last_errno = errno;
    return false;
  }
  return true;
}

int OLocal::Size(const char *path, int *size, char mode) {
  struct stat statbuf;

  _last_errno = 0;
  *size = 0;
  if (stat(path, &statbuf) != 0) {
    _last_errno = errno;
    return false;
  }
  *size = statbuf.st_size;
  return true;
}

int OLocal::ModDate(const char *path, char *dt, int max) {
  struct stat statbuf;

  _last_errno = 0;
  dt[0] = '\0';
  if (stat(path, &statbuf) != 0) {
    _last_errno = errno;
    return false;
  }
  strcpy(dt, ctime(&statbuf.st_mtime));
  return true;
}

int OLocal::Rename(const char *src, const char *dst) {
  _last_errno = 0;
  if (rename(src, dst) != 0) {
    _last_errno = errno;
    return false;
  }
  return true;
}

int OLocal::Delete(const char *fnm) {
  _last_errno = 0;
  if (unlink(fnm) != 0) {
    _last_errno = errno;
    return false;
  }
  return true;
}

char *OLocal::LastResponse() {
  if (_last_errno != 0)
    return strerror(_last_errno);
  return "<no error>";
}

OHandle *OLocal::OpenDir(const char *path, const char *args, int mode) {

  if ((_dp = opendir(path)) == NULL) return false;

  _dpath = StrDup(path);

  OHandle *tmp = new OHandle;
  memset(tmp, '\0', sizeof(OHandle));
#ifdef __sun__
  tmp->fd = _dp->dd_fd;
#else
  tmp->fd = dirfd(_dp);
#endif
  tmp->mode = mode;
  tmp->buf = NULL;

  return tmp;
}

int OLocal::GetFile(OHandle *handle, OFile *&elem) {
  char buf[PATH_MAX];
  struct dirent *dent;
  struct stat sbuf;
  struct passwd *pwd;
  struct group *grp;

  elem = NULL;
  dent = readdir(_dp);
  if (dent) {
    elem = new OFile();
    elem->SetName(dent->d_name);
    elem->_type = 00644;
    sprintf(buf, "%s/%s", _dpath, dent->d_name);
    if (lstat(buf, &sbuf) == 0) {
      if (S_ISLNK(sbuf.st_mode)) {
        elem->_is_link = 1;
        stat(buf, &sbuf);
      } else {
        elem->_is_link = 0;
      }
      elem->_type = sbuf.st_mode;
      elem->_size = sbuf.st_size;
      elem->_time = sbuf.st_mtime;
      pwd = getpwuid(sbuf.st_uid);
      if (pwd) {
        elem->SetUser(pwd->pw_name);
      } else {
        sprintf(buf, "%d", sbuf.st_uid);
        elem->SetUser(buf);
      }
      grp = getgrgid(sbuf.st_gid);
      if (grp) {
        elem->SetGroup(grp->gr_name);
      } else {
        sprintf(buf, "%d", sbuf.st_gid);
        elem->SetGroup(buf);
      }
    }
    return true;
  }
  return false;
}

OHandle *OLocal::OpenFile(const char *file, int mode) {
  int fd;

  if ((mode & (MODE_TRANS_GET | MODE_TRANS_PUT)) == 0) return NULL;

  if (mode & MODE_TRANS_GET) {
    fd = open(file, O_RDONLY);
  } else {
    fd = creat(file, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
  }

  if (fd < 0) return NULL;

  OHandle *tmp = new OHandle;
  memset(tmp, '\0', sizeof(OHandle));
  tmp->fd = fd;
  tmp->mode = mode;
  tmp->buf = NULL;

  return tmp;
}

int OLocal::Close(OHandle *handle) {
  if (handle->mode & (MODE_DIR_LIST | MODE_DIR_NLST)) {
    if (_dpath) {
      delete[] _dpath;
      _dpath = NULL;
    }

    if (_dp) {
      closedir(_dp);
      _dp = NULL;
    }
  } else {
    close(handle->fd);
  }

  if (handle->buf) delete[] handle->buf;
  delete handle;

  return true;
}
