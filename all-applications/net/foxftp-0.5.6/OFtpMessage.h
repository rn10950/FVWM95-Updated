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

#ifndef __OFTPMESSAGE_H
#define __OFTPMESSAGE_H

#include <xclass/utils.h>
#include <xclass/OMessage.h>

#include "OFtpConfig.h"


class ODir;
class OSiteConfig;
class OXSiteView;

template <class T> class TDDLList;


#define SITE_MESSAGE	101
#define SITE_CONNECT	 1
#define FTP_TRANSFER	102
#define FTP_START	 1
#define CLIPBD_MESSAGE	103
#define CLIPBD_COPY	 1
#define CLIPBD_PASTE	 2
#define STATUS_MESSAGE	104
#define	STATUS_CHANGED	 1


//----------------------------------------------------------------------

class OTransferMessage : public OMessage {
public:
  OTransferMessage(int typ, int act, OXSiteView *de, OXSiteView *a) :
    OMessage(typ, act) {
      from = de;
      to = a;
  }
  OXSiteView *from, *to;
};

class OSiteMessage : public OMessage {
public:
  OSiteMessage(int typ, int act, OSiteConfig *inf) :
    OMessage(typ, act) {
      info = new OSiteConfig(inf);
  }
  virtual ~OSiteMessage() { if (info) delete info; }

  OSiteConfig *info;
};
 
class OClipboardMessage : public OMessage {
public:
  OClipboardMessage(int typ, int act,
                    OXSiteView *de, TDDLList<ODir *> *lst) :
    OMessage(typ, act) {
      from = de;
      list = lst;
  }
  virtual ~OClipboardMessage() { if (list) delete list; }

  OXSiteView *from;
  TDDLList<ODir *> *list;
};

class OStatusMessage : public OMessage {
public:
  OStatusMessage(int typ, int act, const char *s) :
    OMessage(typ, act) {
      str = StrDup(s);
  }
  void SetText(const char *s) {
    if (str) delete[] str;
    str = StrDup(s);
  }
  virtual ~OStatusMessage() { if (str) delete str; }

  char *str;
};


#endif  // __OFTPMESSAGE_H
