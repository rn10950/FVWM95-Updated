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

#ifndef __OMAIL_H
#define __OMAIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "OText.h"


//----------------------------------------------------------------------

struct SAddress {
  char *name;
  char *adress;
  char *rawadress;
  SAddress *next;
};

struct SParm {
  char *identifier;
  char *value;
  SParm *next;
};


void UnFold(OText *field); 

class OMail;
class OMailList;
class OMimeAtt;
class OXMailList;
class OXHeaderView;
class OXBodyView;


//----------------------------------------------------------------------

class OMail {
public:
  OMail(OMailList *mail);
  ~OMail();

  void InitMail(char *folder, long start, long body, long end);
  char *GetSubject() const { return _subject; }
  char *GetDate() const { return _date; }
  char *GetFolder() const { return _folder; }
  bool IsMime() const { return _isMime; }
  bool IsMultipart() const { return _isMultipart; }
  SAddress *GetFrom() const { return _from; }
  SAddress *GetTo() const { return _to; }
  SAddress *GetCC() const { return _cc; }
  SAddress *GetReplyTo() const { return _replyTo; }
  OMimeAtt *GetMime() const { return _mimes; }
  OText *GetMessage();
  OText *GetMessagePlusHeader();
  OText *GetMessageSrc();
  void SaveMail(char *folder);

  friend class OMailList;
  friend class OXMailList;
  friend class OXHeaderView;
  friend class OXBodyView;

  OMailList *GetMailList() const { return _mailList; }
  char *GetMessageID() const { return _message_id; }

protected:
  void ParseFields(OText *fields);
  void ParseMime();
  void SetAdressFields(const char *line, char sep,
                       char **first, char **second);
  char *SetSubject(const char *line);
  char *SetDate(const char *line);
  char *SetMessageID(const char *line);
  SAddress *SetAdresses(const char *field, const char *line);

  bool _isMultipart, _isMime;
  OMailList *_mailList;
  long _headerStart, _bodyStart, _bodyEnd;
  char *_subject, *_date, *_folder, *_message_id;
  SAddress *_sender, *_from, *_to;
  SAddress *_cc; // **< what about BCC ?? >**
  SAddress *_replyTo;

  OMimeAtt *_mimes;
  char *_mimeBoundary;

  OMail *next, *prev;
};


#endif  // __OMAIL_H
