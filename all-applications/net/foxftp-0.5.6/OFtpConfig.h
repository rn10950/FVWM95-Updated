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

#ifndef __OFTPCONFIG_H
#define __OFTPCONFIG_H


//----------------------------------------------------------------------

class OGeneralConfig {
public:
  OGeneralConfig();
  ~OGeneralConfig();

  void SetProxyHost(char *proxy);
  void SetAnonymousPasswd(char *anonymous_pw);
  void SetDownloadDir(char *download_dir);
  void SetCacheDir(char *cache_dir);

  char *_proxy, *_anonymous_pw, *_download_dir, *_cache_dir;
  int _proxyport, _recvbuffer, _sendbuffer;
  int _transfer_type, _exist_mode, _proxy_mode;
  int _site_retry, _retry_delay;
  bool _disconnect_close, _close_disconnect;
};


//----------------------------------------------------------------------

class OSiteConfig {
public:
  OSiteConfig();
  OSiteConfig(const OSiteConfig *info);
  ~OSiteConfig();

  void SetSite(char *site);
  void SetLogin(char *login);
  void SetPasswd(char *passwd);
  void SetInitialPath(char *ipath);
  char *_site;
  char *_login;
  char *_passwd;
  char *_ipath;
  int _port, _uploadmode, _keepalive;
  bool _usepassive, _useproxy, _useanonymous, _dolog, _showdot;

  char *Encrypt(char *scanstr, char *key);
  char *Decrypt(char *scanstr, char *key);

private:
  char KeyToChar(char *key);
};

#endif  // __OFTPCONFIG_H
