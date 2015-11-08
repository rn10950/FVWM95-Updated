/* OFtp.cc is a quick C++ hack based on ftplib.c,			   */
/* http://www.cnj.digex.net/~pfau/ftplib/ 				   */
/* it might not work on Windows and VMS though.(som code deleted)          */
/* Copyright (C) 1998, 1999 Per Borgentun, e4borgen@etek.chalmers.se	   */

/* This library is free software; you can redistribute it and/or	   */
/* modify it under the terms of the GNU Library General Public		   */
/* License.								   */


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

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <ctype.h>
#include <time.h>
#include <sys/stat.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "OFile.h"
#include "OFtp.h"


// TODO:
// - get rid of perror messages (calling app should use a MsgBox)
// - use OTimer's for timeouts
// - replace select calls with OFileHandler's
// - handle "broken pipe" cases


//----------------------------------------------------------------------

OFtp::OFtp():OFileSystem() {
  ctrl = NULL;
  _pwd[0] = 0;
}

OFtp::~OFtp() {
  if (ctrl) {
    close(ctrl->handle.fd);
    delete[] ctrl->handle.buf;
    delete ctrl;
    ctrl = NULL;
  }
}

// OFtp::Connect - connect to remote server  
// Return 1 if connected, 0 if not.
// In the last case errno contains the error code.

int OFtp::Connect(const char *host) {
  int sControl;
  int on = 1;
  struct sockaddr_in sin;
  struct hostent *phe;
  struct servent *pse;
  char *lhost;
  char *pnum;

  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  lhost = strdup(host);
  pnum = strchr(lhost, ':');
  if (pnum == NULL) {
    if ((pse = getservbyname("ftp", "tcp")) == NULL) {
      perror("getservbyname");
      return 0;
    }
    sin.sin_port = pse->s_port;
  } else {
    *pnum++ = '\0';
    if (isdigit(*pnum))
      sin.sin_port = htons(atoi(pnum));
    else {
      pse = getservbyname(pnum, "tcp");
      sin.sin_port = pse->s_port;
    }
  }
  if ((sin.sin_addr.s_addr = inet_addr(lhost)) == INADDR_NONE) {
    if ((phe = gethostbyname(lhost)) == NULL) {
      perror("gethostbyname");
      return 0;
    }
    memcpy((char *) &sin.sin_addr, phe->h_addr, phe->h_length);
  }
  free(lhost);
  sControl = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sControl == -1) {
    perror("socket");
    return 0;
  }
  if (setsockopt(sControl, SOL_SOCKET, SO_REUSEADDR,
		 SETSOCKOPT_OPTVAL_TYPE & on, sizeof(on)) == -1) {
    perror("setsockopt");
    close(sControl);
    return 0;
  }
  signal(SIGALRM, _Timeout);
  alarm(30);
  if (connect(sControl, (struct sockaddr *) &sin, sizeof(sin)) == -1) {
    alarm(0);
    perror("connect");
    close(sControl);
    return 0;
  }

  ctrl = new ONetBuf;
  memset(ctrl, '\0', sizeof(ONetBuf));
  ctrl->handle.fd = sControl;
  ctrl->handle.mode = MODE_CONN_PORT | MODE_TRANS_ASCII;
  ctrl->handle.buf = new char[FTP_BUFSIZ];
  if (_readresp('2') == 0) {
    close(sControl);
    delete[] ctrl->handle.buf;
    delete ctrl;
    ctrl = NULL;
    return 0;
  }
  _state = FS_STATE_READY;

  return 1;
}

// OFtp::Quit - disconnect from remote
// Return 1 if successful, 0 otherwise

int OFtp::Quit() {
  if (ctrl == NULL) return 0;
  _SendCmd("QUIT", '2');
  close(ctrl->handle.fd);
  delete[] ctrl->handle.buf;
  delete ctrl;
  ctrl = NULL;
  _state = FS_STATE_NOT_READY;
  return 1;
}

// OFtp::Login - log in to remote server
// Return 1 if logged in, 0 otherwise

int OFtp::Login(const char *user, const char *pass) {
  char tempbuf[128];
  sprintf(tempbuf, "USER %s", user);
  if (_SendCmd(tempbuf, '3') == 0) {
    if (ctrl->response[0] == '2') return 1;
    return 0;
  }
  sprintf(tempbuf, "PASS %s", pass);
  return _SendCmd(tempbuf, '2');
}

// OFtp::Noop - send a NOOP command
// Return 1 if command successful, 0 otherwise

int OFtp::Noop() {
  return _SendCmd("NOOP", '2');
}

// OFtp::Site - send a SITE command
// Return 1 if command successful, 0 otherwise

int OFtp::Site(const char *cmd) {
  char buf[256];
  sprintf(buf, "SITE %s", cmd);
  return _SendCmd(buf, '2');
}

// OFtp::SysType - send a SYST command
// Fills in the user buffer with the remote system type.  If more
// information from the response is required, the user can parse
// it out of the response buffer returned by OFtp::LastResponse().
// Return 1 if command successful, 0 otherwise

int OFtp::SysType(char *buf, int max) {
  int l = max;
  char *b = buf;
  char *s;

  if (_SendCmd("SYST", '2') == 0) return 0;
  s = &ctrl->response[4];
  while ((--l) && (*s != ' ')) *b++ = *s++;
  *b++ = '\0';

  return 1;
}

// OFtp::Mkdir - create a directory at server
// Return 1 if successful, 0 otherwise

int OFtp::Mkdir(const char *path) {
  char buf[256];
  sprintf(buf, "MKD %s", path);
  return _SendCmd(buf, '2');
}

// OFtp::Chdir - change path at remote
// Return 1 if successful, 0 otherwise

int OFtp::Chdir(const char *path) {
  char buf[256];
  sprintf(buf, "CWD %s", path);
  _pwd[0] = 0;
  return _SendCmd(buf, '2');
}

// OFtp::CDUp - move to parent directory at remote
// Return 1 if successful, 0 otherwise

int OFtp::CDUp() {
  _pwd[0] = 0;
  return _SendCmd("CDUP", '2');
}

// OFtp::Rmdir - remove directory at remote
// Return 1 if successful, 0 otherwise

int OFtp::Rmdir(const char *path) {
  char buf[256];
  sprintf(buf, "RMD %s", path);
  return _SendCmd(buf, '2');
}

// OFtp::Pwd - get working directory at remote
// Return 1 if successful, 0 otherwise

int OFtp::Pwd(char *path, int max) {
  int l = max;
  char *b = path;
  char *s;
  if (strlen(_pwd) != 0) {
    strcpy(path, _pwd);
    return 1;
  }
  if (_SendCmd("PWD", '2') == 0) return 0;
  s = strchr(ctrl->response, '"');
  if (s == NULL) return 0;
  s++;
  while ((--l) && (*s) && (*s != '"')) *b++ = *s++;
  *b++ = '\0';
  strcpy(_pwd, path);
  return 1;
}

// OFtp::Size - determine the size of a remote file
// Return 1 if successful, 0 otherwise

int OFtp::Size(const char *path, int *size, char mode) {
  char cmd[256];
  int resp, sz, rv = 1;

  if (_SetType(mode)) return 0;
  sprintf(cmd, "SIZE %s", path);
  if (_SendCmd(cmd, '2') == 0) {
    rv = 0;
  } else {
    if (sscanf(ctrl->response, "%d %d", &resp, &sz) == 2)
      *size = sz;
    else
      rv = 0;
  }
  return rv;
}

// OFtp::ModDate - determine the modification date of a remote file
// Return 1 if successful, 0 otherwise

int OFtp::ModDate(const char *path, char *dt, int max) {
  char buf[256];
  int rv = 1;

  sprintf(buf, "MDTM %s", path);
  if (_SendCmd(buf, '2') == 0)
    rv = 0;
  else
    strncpy(dt, &ctrl->response[4], max);
  return rv;
}

// OFtp::Rename - rename a file at remote
// Return 1 if successful, 0 otherwise

int OFtp::Rename(const char *src, const char *dst) {
  char cmd[256];
  sprintf(cmd, "RNFR %s", src);
  if (_SendCmd(cmd, '3') == 0) return 0;
  sprintf(cmd, "RNTO %s", dst);
  return _SendCmd(cmd, '2');
}

// OFtp::Delete - delete a file at remote
// Return 1 if successful, 0 otherwise

int OFtp::Delete(const char *fnm) {
  char cmd[256];
  sprintf(cmd, "DELE %s", fnm);
  return _SendCmd(cmd, '2');
}

OHandle *OFtp::OpenDir(const char *path, const char *args, int mode) {
  char cmd[256];
  OHandle *handle;

  mode = (mode & ~MODE_TRANS_PUT) | MODE_TRANS_GET;

  if (_SetType(mode) == 0) return NULL;

  if (_OpenPort(&handle, mode) == 0) return NULL;

  sprintf(cmd, "%s %s%s%s",
	  ((mode & MODE_DIR_NLST) == MODE_DIR_NLST) ? "NLST" : "LIST",
	  (args) ? args : "", (args && *args) ? " " : "", (path) ? path : ".");
  if (_SendCmd(cmd, '1') == 0) {
    _ClosePort(handle);
    return NULL;
  }

  if (mode & MODE_CONN_PORT) {
    if (_AcceptPort(handle) == 0) {
      _ClosePort(handle);
      return NULL;
    }
  }

  _readline(handle, ctrl->response, 256);
  return handle;
}

static char *months[] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

int OFtp::GetFile(OHandle *handle, OFile * &elem) {
  char *cptr, buf[256];
  int i, n = 00400;

  elem = NULL;
  do {
    if (_readline(handle, buf, 256) == 0) return 0;
    cptr = strtok(buf, " \r\n");
  } while (cptr == NULL);

  elem = new OFile();

  switch (*cptr++) {
    case 'p':
    case 'P':
      elem->_type |= S_IFIFO;
      break;

    case 'c':
    case 'C':
      elem->_type |= S_IFCHR;
      break;

    case 'd':
    case 'D':
      elem->_type |= S_IFDIR;
      break;

    case 'b':
    case 'B':
      elem->_type |= S_IFBLK;
      break;

    case 'l':
    case 'L':
      elem->_is_link = 1;
      elem->_type |= S_IFLNK;
      break;

    default:
      elem->_type |= S_IFREG;
      break;
  }

  for (i = 1; i < 10; i++, cptr++) {
    if (*cptr != '-') elem->_type |= n;
    n >>= 1;
  }

  strtok(NULL, " \r\n");
  elem->SetUser(strtok(NULL, " \r\n"));
  elem->SetGroup(strtok(NULL, " \r\n"));
  elem->_size = atoi(strtok(NULL, " \r\n"));

  cptr = strtok(NULL, " \r\n");
  int stime = time(NULL);
  struct tm *local = localtime((const long *) &stime);
  for (int j = 0; j < 12; j++) {
    if (strncmp(cptr, months[j], 3) == 0) {
      local->tm_mon = j + 1;
      break;
    }
  }
  cptr += 4;
  local->tm_mday = atoi(cptr);
  cptr += 3;
  if (atoi(cptr) > 1900) {
    cptr += 3;
    local->tm_year = atoi(cptr);
    cptr += 3;
  } else {
    local->tm_hour = atoi(cptr);
    cptr += 3;
    local->tm_min = atoi(cptr);
    cptr += 3;
  }
  elem->_time = mktime(local);

  if (elem->_is_link) {
    char *p = strstr(cptr, " -> ");
    if (p) *p = '\0';
  }
  elem->SetName(strtok(cptr, "\r\n"));

  return 1;
}

OHandle *OFtp::OpenFile(const char *file, int mode) {
  char cmd[256];
  OHandle *handle;

  if (mode & (MODE_TRANS_GET | MODE_TRANS_PUT) == 0) return NULL;

  if (_SetType(mode) == 0) return NULL;

  if (_OpenPort(&handle, mode) == 0) return NULL;

  sprintf(cmd, "%s %s",
	  ((mode & MODE_TRANS_GET) == MODE_TRANS_GET) ? "RETR" : "STOR",
	  file);

  if (_SendCmd(cmd, '1') == 0) {
    _ClosePort(handle);
    return NULL;
  }

  if (mode & MODE_CONN_PORT) {
    if (_AcceptPort(handle) == 0) {
      _ClosePort(handle);
      _readresp('2');
      return NULL;
    }
  }

  return handle;
}

int OFtp::Close(OHandle *handle) {
  if (handle) {
    _ClosePort(handle);
    _readresp('2');
  }
  return 1;
}

// OFtp::LastResponse - last response from remote or error message

char *OFtp::LastResponse() {
  if (ctrl) return ctrl->response;
  return NULL;
}

// OFtp::_SetType - set the new transfer type
// Return 1 if successful, 0 otherwise

int OFtp::_SetType(int mode) {
  char cmd[256];

  int mode1 = ctrl->handle.mode & (MODE_TRANS_ASCII | MODE_TRANS_IMAGE);
  int mode2 = mode & (MODE_TRANS_ASCII | MODE_TRANS_IMAGE);
  if (mode1 != mode2) {
    sprintf(cmd, "TYPE %c", (mode2 & MODE_TRANS_ASCII) ? 'A' : 'I');
    if (_SendCmd(cmd, '2') == 0) return 0;
    ctrl->handle.mode &= ~mode1;
    ctrl->handle.mode |= mode2;
  }
  return 1;
}

// OFtp::_OpenPort - set up data connection
// Return 1 if successful, 0 otherwise

int OFtp::_OpenPort(OHandle **nData, int mode) {
  int sData;
  union {
    struct sockaddr sa;
    struct sockaddr_in in;
  } sin;
  struct linger lng = { 0, 0 };
  socklen_t l;
  int on = 1;
  unsigned int v[6];
  char *cp, buf[256];

  *nData = NULL;
  if (ctrl == NULL) return 0;

  if ((mode & (MODE_TRANS_GET | MODE_TRANS_PUT)) == 0) {
    sprintf(ctrl->response, "Invalid direction\n");
    return 0;
  }
  if ((mode & (MODE_TRANS_ASCII | MODE_TRANS_IMAGE)) == 0) {
    sprintf(ctrl->response, "Invalid mode\n");
    return 0;
  }
  if ((mode & (MODE_CONN_PASSIVE | MODE_CONN_PORT)) == 0) {
    sprintf(ctrl->response, "Invalid connection mode\n");
    return 0;
  }

  l = sizeof(sin);
  if (mode & MODE_CONN_PASSIVE) {
    memset(&sin, 0, l);
    sin.in.sin_family = AF_INET;

    if (_SendCmd("PASV", '2') == 0) return -1;
    cp = strchr(ctrl->response, '(');
    if (cp == NULL) return -1;
    cp++;
    sscanf(cp, "%u,%u,%u,%u,%u,%u",
	   &v[2], &v[3], &v[4], &v[5], &v[0], &v[1]);
    sin.sa.sa_data[2] = v[2];
    sin.sa.sa_data[3] = v[3];
    sin.sa.sa_data[4] = v[4];
    sin.sa.sa_data[5] = v[5];
    sin.sa.sa_data[0] = v[0];
    sin.sa.sa_data[1] = v[1];
  } else {
    if (getsockname(ctrl->handle.fd, &sin.sa, &l) < 0) {
      perror("getsockname");
      return 0;
    }
  }
  sData = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sData == -1) {
    perror("socket");
    return 0;
  }
  if (setsockopt(sData, SOL_SOCKET, SO_REUSEADDR,
		 SETSOCKOPT_OPTVAL_TYPE & on, sizeof(on)) == -1) {
    perror("setsockopt");
    close(sData);
    return 0;
  }
  if (setsockopt(sData, SOL_SOCKET, SO_LINGER,
		 SETSOCKOPT_OPTVAL_TYPE & lng, sizeof(lng)) == -1) {
    perror("setsockopt");
    close(sData);
    return 0;
  }

  if (mode & MODE_CONN_PASSIVE) {
    if (connect(sData, &sin.sa, sizeof(sin.sa)) == -1) {
      perror("connect");
      close(sData);
      return 0;
    }
  } else {
    sin.in.sin_port = 0;
    if (bind(sData, &sin.sa, sizeof(sin)) == -1) {
      perror("bind");
      close(sData);
      return 0;
    }
    if (listen(sData, 1) < 0) {
      perror("listen");
      close(sData);
      return 0;
    }
    if (getsockname(sData, &sin.sa, &l) < 0) return 0;
    sprintf(buf, "PORT %d,%d,%d,%d,%d,%d",
	    (unsigned char) sin.sa.sa_data[2],
	    (unsigned char) sin.sa.sa_data[3],
	    (unsigned char) sin.sa.sa_data[4],
	    (unsigned char) sin.sa.sa_data[5],
	    (unsigned char) sin.sa.sa_data[0],
	    (unsigned char) sin.sa.sa_data[1]);

    if (_SendCmd(buf, '2') == 0) {
      close(sData);
      return 0;
    }
  }

  OHandle *tmp = new OHandle;
  memset(tmp, '\0', sizeof(OHandle));
  tmp->fd = sData;
  tmp->mode = mode;
  tmp->buf = new char[FTP_BUFSIZ];
  *nData = tmp;

  return 1;
}

// OFtp::_AcceptPort - accept connection from server
// Return 1 if successful, 0 otherwise

int OFtp::_AcceptPort(OHandle *nData) {
  int sData;
  struct sockaddr addr;
  socklen_t l;
  int i;
  struct timeval tv;
  fd_set mask;
  int rv;

  FD_ZERO(&mask);
  FD_SET(ctrl->handle.fd, &mask);
  FD_SET(nData->fd, &mask);
  tv.tv_usec = 0;
  tv.tv_sec = ACCEPT_TIMEOUT;
  i = ctrl->handle.fd;
  if (i < nData->fd) i = nData->fd;
  i = select(i + 1, &mask, NULL, NULL, &tv);
  if (i == -1) {
    strcpy(ctrl->response, strerror(errno));
    close(nData->fd);
    nData->fd = 0;
    rv = 0;
  } else if (i == 0) {
    strcpy(ctrl->response, "timed out waiting for connection");
    close(nData->fd);
    nData->fd = 0;
    rv = 0;
  } else {
    if (FD_ISSET(nData->fd, &mask)) {
      l = sizeof(addr);
      sData = accept(nData->fd, &addr, &l);
      i = errno;
      close(nData->fd);
      if (sData > 0) {
	rv = 1;
	nData->fd = sData;
      } else {
	strcpy(ctrl->response, strerror(i));
	nData->fd = 0;
	rv = 0;
      }
    } else if (FD_ISSET(ctrl->handle.fd, &mask)) {
      close(nData->fd);
      nData->fd = 0;
      _readresp('2');
      rv = 0;
    }
  }
  return rv;
}

// _ClosePort - close a data connection

int OFtp::_ClosePort(OHandle *nData) {
  shutdown(nData->fd, 2);
  close(nData->fd);
  if (nData->buf) delete[] nData->buf;
  delete nData;
  //return(_readresp('2'));
  return 1;
}

// OFtp::_readline - read a line of text
// Return -1 on error or bytecount

int OFtp::_readline(OHandle *handle, char *buf, int max) {
  int i, pos = 0;
  int rtc = 1;

  do {
    i = read(handle->fd, &buf[pos], 1);
    if (i < 0) {
      rtc = 0;
      break;
    }
    if (i == 0) {
      buf[pos] = '\0';
      rtc = 0;
      break;
    }
    if (buf[pos] == '\n') {
      buf[pos] = '\0';
      break;
    }
    pos++;
  } while (pos < max);

  return rtc;
}

// OFtp::_SendCmd - send a command and wait for expected response
// Return 1 if proper response received, 0 otherwise

void OFtp::_Timeout(int) {
}

int OFtp::_SendCmd(const char *cmd, char expresp) {
  char buf[256];

  if (ctrl == NULL) return 0;

  if (strncmp(cmd, "PASS", 4) != 0)
    Log(">> ", cmd);
  else
    Log(">> ", "<password>");

  sprintf(buf, "%s\r\n", cmd);
  signal(SIGALRM, _Timeout);
  alarm(30);
  if (write(ctrl->handle.fd, buf, strlen(buf)) <= 0) {
    alarm(0);
    perror("write");
    return 0;
  }

  return _readresp(expresp);
}

// OFtp::_readresp - read a response from the server
// Return 0 if first char doesn't match
// Return 1 if first char matches

int OFtp::_readresp(char c) {
  char match[5];

  signal(SIGALRM, _Timeout);
  alarm(30);
  if (_readline(&ctrl->handle, ctrl->response, 256) == -1) {
    alarm(0);
    perror("Control socket read failed");
    return 0;
  } else {
    Log("<< ", ctrl->response);
    if (ctrl->response[3] == '-') {
      strncpy(match, ctrl->response, 3);
      match[3] = ' ';
      match[4] = '\0';
      do {
	if (_readline(&ctrl->handle, ctrl->response, 256) == -1) {
	  alarm(0);
	  perror("Control socket read failed");
	  return 0;
	}
	Log("<< ", ctrl->response);
      } while (strncmp(ctrl->response, match, 4));
      alarm(0);
    }
  }

  if (ctrl->response[0] == '2') Log("", "");

  return (ctrl->response[0] == c);
}
