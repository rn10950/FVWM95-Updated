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
#include <string.h>
#include <ctype.h>

#include <xclass/utils.h>

#include "OFtpConfig.h"


//----------------------------------------------------------------------

void trim(char *str) {
  int len = strlen(str);
  int st = 0;
  while (st < len && isspace(str[st])) st++;
  while (st < len && isspace(str[len - 1])) len--;
  strncpy(str, &str[st], len - st);
  str[len - st] = '\0';
}


//----------------------------------------------------------------------

OGeneralConfig::OGeneralConfig() {
  _proxy = _anonymous_pw = _download_dir = _cache_dir = NULL;
  _proxyport = 21;
  _recvbuffer = _sendbuffer = 1024;
  _disconnect_close = _close_disconnect = true;
  _transfer_type = _exist_mode = _proxy_mode = 1;
  _site_retry = 0;
  _retry_delay = 10;
}

OGeneralConfig::~OGeneralConfig() {
  if (_proxy) delete[] _proxy;
  if (_anonymous_pw) delete[] _anonymous_pw;
  if (_download_dir) delete[] _download_dir;
  if (_cache_dir) delete[] _cache_dir;
}

void OGeneralConfig::SetProxyHost(char *proxy) {
  if (_proxy) delete[] _proxy;
  _proxy = NULL;
  if (proxy) {
    _proxy = StrDup(proxy);
    trim(_proxy);
  }
}

void OGeneralConfig::SetAnonymousPasswd(char *anonymous_pw) {
  if (_anonymous_pw) delete[] _anonymous_pw;
  _anonymous_pw = NULL;
  if (anonymous_pw) {
    _anonymous_pw = StrDup(anonymous_pw);
    trim(_anonymous_pw);
  }
}

void OGeneralConfig::SetDownloadDir(char *download_dir) {
  if (_download_dir) delete[] _download_dir;
  _download_dir = NULL;
  if (download_dir) {
    _download_dir = StrDup(download_dir);
    trim(_download_dir);
  }
}

void OGeneralConfig::SetCacheDir(char *cache_dir) {
  if (_cache_dir) delete[] _cache_dir;
  _cache_dir = NULL;
  if (cache_dir) {
    _cache_dir = StrDup(cache_dir);
    trim(_cache_dir);
  }
}


//----------------------------------------------------------------------

OSiteConfig::OSiteConfig() {
  _site = _login = _passwd = _ipath = NULL;
  _port = 21;
  _uploadmode = 1;
  _keepalive = 60;
  _useproxy = _useanonymous = _usepassive = _showdot = false; 
  _dolog = true;
}

OSiteConfig::OSiteConfig(const OSiteConfig *info) {
  _site = _login = _passwd = _ipath = NULL;
  SetSite(info->_site);
  SetLogin(info->_login);
  SetPasswd(info->_passwd);
  SetInitialPath(info->_ipath);
  _port = info->_port;
  _useproxy = info->_useproxy;
  _useanonymous = info->_useanonymous;
  _uploadmode = info->_uploadmode;
  _usepassive = info->_usepassive;
  _keepalive = info->_keepalive;
  _dolog = info->_dolog;
  _showdot = info->_showdot;
}

OSiteConfig::~OSiteConfig() {
  if (_site) delete[] _site;
  if (_login) delete[] _login;
  if (_passwd) delete[] _passwd;
  if (_ipath) delete[] _ipath;
}

void OSiteConfig::SetSite(char *site) {
  if (_site) delete[] _site;
  _site = NULL;
  if (site) {
    _site = StrDup(site);
    trim(_site);
  }
}

void OSiteConfig::SetLogin(char *login) {
  if (_login) delete[] _login;
  _login = NULL;
  if (login) {
    _login = StrDup(login);
    trim(_login);
  }
}

void OSiteConfig::SetPasswd(char *passwd) {
  if (_passwd) delete[] _passwd;
  _passwd = NULL;
  if (passwd) _passwd = StrDup(passwd);
}

void OSiteConfig::SetInitialPath(char *ipath) {
  if (_ipath) delete[] _ipath;
  _ipath = NULL;
  if (ipath) {
    _ipath = StrDup(ipath);
    trim(_ipath);
  }
}

char OSiteConfig::KeyToChar(char *key){
  char c = '\0';
  while (*key) c ^= *key++;
  c &= 0x7f;
  return c;
}

char *OSiteConfig::Encrypt(char *scanstr, char *key) {
  char k, c;
  char *es, *cstr;

  k = KeyToChar(key);
  cstr = es = new char[strlen(scanstr) * 2 + 1];
  while (*scanstr) {
    c = *scanstr++ ^ k;
    if (iscntrl(c)) {
      *cstr++ = '^';
      if (c == '\177')
        *cstr++ = '?';
      else
        *cstr++ = c + 'A';
    } else if (c == '^') {
      *cstr++ = '^';
      *cstr++ = '~';
    } else {
      *cstr++ = c;
    }
  }
  *cstr = '\0';
  return es;
}

char *OSiteConfig::Decrypt(char *scanstr, char *key) {
  char k, c;
  char *ds, *cstr;

  k = KeyToChar(key);
  cstr = ds = new char[strlen(scanstr) + 1];
  while (*scanstr) {
    c = *scanstr++;
    if (c != '^') {
      *cstr++ = c ^ k;
    } else {
      c = *scanstr++;
      if (c == '?') {
        *cstr = '\177';
      } else if (c == '~') {
        *cstr = '^';
      } else {
        *cstr = c - 'A';
      }
      *cstr++ ^= k;
    }
  }
  *cstr = '\0';
  return ds;
}
