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

#ifndef __OSMTP_H
#define __OSMTP_H

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define	CONNECT_OK	0
#define SOCKET_FAILED 	1
#define BIND_FAILED	2
#define UNKNOWN_HOST	3
#define CONNECT_FAILED	4
#define NO_SERVER	5
#define SMTP_ERROR	6
#define SEND_OK		7


//----------------------------------------------------------------------

class OSmtp {
public:
  OSmtp() { smtpServer = NULL; }
  ~OSmtp() {}

  int SetupSmtp(char *server, int port = 25);
  int CloseConnect();
  int SendMail(FILE *f, char *identity, char *returnpath, 
               char *to, char *reply = NULL, char *cc = NULL);
protected:
  int socket_nr;
  struct sockaddr_in client;
  struct sockaddr_in server;
  struct hostent *smtpServer;  

  int  SendLine(char *line, int number = 999);
  void RecvMsg(char *buffer);
  void SendFile(FILE *f);
};


#endif  // __OSMTP_H
