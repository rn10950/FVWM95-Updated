/**************************************************************************

    This file is part of Xclass95, a Win95-looking GUI toolkit.
    Copyright (C) 1996, 1997 David Barth, Hector Peraza.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

**************************************************************************/

#ifndef __OIRCMESSAGE_H
#define __OIRCMESSAGE_H

#include <xclass/OString.h>
#include <xclass/OMessage.h>

#include "OTcpMessage.h"

#define IRC_MSG_LENGTH	512	// as per RFC1459
#define IRC_MAX_ARGS	16	// 15 according to RFC1459


#define IRC_NICKLIST		(MSG_USERMSG+10)

#define INCOMING_IRC_MSG	(MSG_USERMSG+22)
#define OUTGOING_IRC_MSG	(MSG_USERMSG+23)
#define INCOMING_CTCP_MSG	(MSG_USERMSG+24)
#define OUTGOING_CTCP_MSG	(MSG_USERMSG+25)

#define PRIVMSG		0
#define NOTICE		1
#define DCC		2
#define CTCP            3


//----------------------------------------------------------------------

class OIrcMessage : public OMessage {
public:
  OIrcMessage(int typ, int act, const char *raw_msg);
  virtual ~OIrcMessage();

public:
  int  argc;
  char *prefix, *command, *argv[IRC_MAX_ARGS];
  const char *rawmsg;
  char *nick, *ircname;

protected:
  char msg[IRC_MSG_LENGTH];
};


#endif  // __OIRCMESSAGE_H
