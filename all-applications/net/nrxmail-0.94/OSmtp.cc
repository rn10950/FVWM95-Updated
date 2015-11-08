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

#include <pwd.h>
#include <unistd.h>
#include <string.h>

#include "OSmtp.h"


//----------------------------------------------------------------------

int OSmtp::SetupSmtp(char *smtp, int port) {
  socket_nr = socket(PF_INET, SOCK_STREAM, 0);
  if (socket < 0) {
    printf("ERROR !!!! Socket not set up !");
    return SOCKET_FAILED;
  }
  smtpServer = gethostbyname(smtp);	// server name als char *
  if (smtpServer == NULL) {
    printf("ERROR !!!! Unknown Host %s\n", smtp);
    return UNKNOWN_HOST;
  }

  server.sin_family = AF_INET;
  server.sin_port = htons(port);
  server.sin_addr = *(struct in_addr *) smtpServer->h_addr;

  if (connect(socket_nr, (struct sockaddr *) &server, sizeof(server)) < 0) {
    printf("ERROR !!!! Connection not successful !");
    return CONNECT_FAILED;
  }

  return CONNECT_OK;
}

int OSmtp::SendMail(FILE *f, char *identity, char *returnpath, char *to,
		    char *reply, char *cc) {
  char *temp;
  temp = new char[256];
  temp[255] = '\0';
  char *mail_string = "MAIL FROM:";
  char *rcpt_string = "RCPT TO:";
  char *data_string = "DATA";
  long count = 0;
  char *sender;
  char *travel = NULL;
  char *travel2 = NULL;

  if (SendLine(NULL, 220) != 0) {
    printf("ERROR !!!! No SMTP server at local machine ???\n");
    return NO_SERVER;
  }
//  recv(socket_nr, (void*) temp, 256, 0);
  delete[] temp;

  if (SendLine("HELO localhost", 250) != 0) {
    printf("ERROR !!!! SMTP ERROR !halo\n");
    return SMTP_ERROR;
  }

  temp = new char[strlen(mail_string) + 2 + strlen(returnpath) + 1];
  sprintf(temp, "%s<%s>", mail_string, returnpath);
  if (SendLine(temp, 250) != 0) {
    printf("ERROR !!!! SMTP ERROR !a\n");
    return SMTP_ERROR;
  }
  delete[] temp;

  travel = to;
  travel2 = travel;
  travel--;
  while ((travel = strchr(travel2, ',')) != NULL) {
    temp = new char[strlen(rcpt_string) + (travel - travel2) + 1];
    strcpy(temp, rcpt_string);
    strncpy(temp + strlen(rcpt_string), travel2, travel - travel2);
    temp[strlen(rcpt_string) + travel - travel2] = '\0';
    if (SendLine(temp, 250) != 0) {
      printf("ERROR !!!! SMTP ERROR ! to: %s<", temp);
      return SMTP_ERROR;
    }
    delete[] temp;
    travel2 = travel + 1;
    if (*travel2 == ' ') travel2++;
    travel = travel2;
  }
  temp = new char[strlen(rcpt_string) + strlen(travel2) + 1];
  sprintf(temp, "%s%s", rcpt_string, travel2);
  if (SendLine(temp, 250) != 0) {
    printf("ERROR !!!! SMTP ERROR !to: %s<", travel2);
    return SMTP_ERROR;
  }
  delete[] temp;

  if ((cc != NULL) && (strlen(cc) > 0)) {
    travel = cc;
    travel2 = travel;
    travel--;
    while ((travel = strchr(travel2, ',')) != NULL) {
      temp = new char[strlen(rcpt_string) + (travel - travel2) + 1];
      strcpy(temp, rcpt_string);
      strncpy(temp + strlen(rcpt_string), travel2, travel - travel2);
      temp[strlen(rcpt_string) + travel - travel2] = '\0';
      if (SendLine(temp, 250) != 0) {
	printf("ERROR !!!! SMTP ERROR !cc %s<", travel2);
	return SMTP_ERROR;
      }
      delete[] temp;
      travel2 = travel + 1;
      if (*travel2 == ' ')
	travel2++;
      travel = travel2;
    }
    temp = new char[strlen(rcpt_string) + strlen(travel2) + 1];
    sprintf(temp, "%s%s", rcpt_string, travel2);
    if (SendLine(temp, 250) != 0) {
      printf("ERROR !!!! SMTP ERROR !cc %s<", travel2);
      return SMTP_ERROR;
    }
    delete[] temp;
  }

  if (SendLine(data_string, 354) != 0) {
    printf("ERROR !!!! SMTP ERROR 2!");
    return SMTP_ERROR;
  }
  SendFile(f);

  return SEND_OK;
}


int OSmtp::CloseConnect() {

  SendLine(".\r\n", 0);
  SendLine("QUIT\r\n", 0);

  close(socket_nr);

  return CONNECT_OK;
}


int OSmtp::SendLine(char *string, int number) {
  int  k;
  char buffer[1024];
  char num[4];
  int  a;
  char *linebuffer;

  if (string != NULL) {
    linebuffer = new char[strlen(string) + 2 + 1];
    sprintf(linebuffer, "%s\r\n", string);
    send(socket_nr, (void *) linebuffer, strlen(linebuffer), 0);
    delete[] linebuffer;
  }
  if (number != 999) RecvMsg(buffer);

  for (a = 0; a < 3; a++) num[a] = buffer[a];
  num[4] = 0;

  if (atoi(num) != number) return -1;

  return 0;
}

void OSmtp::RecvMsg(char *buffer) {
  recv(socket_nr, (void *) buffer, 1024, 0);
}

void OSmtp::SendFile(FILE *f) {
  char line[1024];
  char *eof;

  fseek(f, 0, SEEK_SET);
  while ((eof = fgets(line, 1024, f)) != NULL) {
    if (line[strlen(line) - 1] == '\n')
      line[strlen(line) - 1] = '\0';
    SendLine(line);
  }
}
