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

#include <unistd.h>

#include <xclass/utils.h>

#include "OMailList.h"


//----------------------------------------------------------------------

OMailList::OMailList() {
  folder = NULL;
  mails = NULL;
  numberOfMails = 0;
}

OMailList::OMailList(const char *folderName) {
  folder = StrDup(folderName);
  mails = NULL;
  numberOfMails = 0;
  Reload();
}

OMailList::~OMailList() {
  Clear();
  if (folder) delete[] folder;
}

void OMailList::Clear() {
  OMail *travel, *toDelete;

  travel = toDelete = mails;
  while (travel != NULL) {
    travel = travel->next;
    delete toDelete;
    toDelete = travel;
  }

  mails = NULL;
  numberOfMails = 0;
}

bool OMailList::Reload() {
  FILE *f;
  char line[1024];
  char eof;

  Clear();
  if (!InitMails()) return False;
  return True;
}

bool OMailList::InitMails() {
  OMail *travel;
  FILE *f;
  bool flag = False;
  char *eof;
  char line[1024];
  unsigned long count = 0;
  long start, body, end;

  if ((f = fopen(folder, "r")) == NULL) return False;

  if (fgets(line, 2, f) == NULL) return True;

  fseek(f, 0, SEEK_SET);
  travel = mails;

  while (((eof = fgets(line, 1024, f)) != NULL)) {
    body = start = end = 0;
    numberOfMails++;
    start = ftell(f) - strlen(line);
    while (((eof = fgets(line, 1024, f)) != NULL) && (strlen(line) > 1));
    body = ftell(f);
    while ((!flag) && ((eof = fgets(line, 1024, f)) != NULL)) {
      count += strlen(line);
      if (line[strlen(line) - 1] == '\n') count = 0;
      if ((strncmp(line, "From ", 5) == 0) && (count == 0)) {
	end = ftell(f) - strlen(line) - 2;
	fseek(f, ftell(f) - strlen(line), SEEK_SET);
	flag = True;
      }
    }
    flag = False;
    if (eof == NULL) end = ftell(f);
    if (travel == NULL) {
      mails = new OMail(this);
      travel = mails;
    } else {
      travel->next = new OMail(this);
      travel->next->prev = travel;
      travel = travel->next;
    }
    travel->InitMail(folder, start, body, end);
  }
  fclose(f);

  return True;
}

OMail *OMailList::GetMail(unsigned long mailNumber) {
  OMail *travel = mails;

  if (numberOfMails < mailNumber) return NULL;

  for (unsigned long i = 0; i < mailNumber; i++)
    travel = travel->next;

  return travel;
}

void OMailList::EraseMail(char *id) {
  OMail *travel = mails;
  FILE *f, *g;
  char line[1024];
  char *eof;

  while ((travel != NULL) && strcmp(travel->_message_id, id))
    travel = travel->next;

  if (travel == NULL) return;

  f = fopen(travel->_folder, "r+");   // ==!==
  g = fopen("/tmp/mailtest", "w+");  // ==!==

  while (ftell(f) < travel->_headerStart) {
    eof = fgets(line, 1024, f);
    fputs(line, g);
  }
  fseek(f, travel->_bodyEnd + 2, SEEK_SET);
  while ((eof = fgets(line, 1024, f)) != NULL) fputs(line, g);
  fclose(f);

  f = fopen(folder, "w");  // ==!==
  fseek(g, 0, SEEK_SET);
  while ((eof = fgets(line, 1024, g)) != NULL) fputs(line, f);
  fclose(f);

  fclose(g);

  numberOfMails--;

  if (travel->prev != NULL)
    travel->prev->next = travel->next;
  else
    mails = travel->next;

  if (travel->next != NULL)
    travel->next->prev = travel->prev;

  delete travel;

  Reload();
}

void OMailList::MoveMail(char *id, char *newfolder) {
  OMail *travel = mails;
  FILE *f, *g;
  long counter = 0;
  char line[1024];
  char *eof;

  while ((travel != NULL) && strcmp(travel->_message_id, id))
    travel = travel->next;

  if (travel == NULL) return;

  f = fopen(travel->_folder, "r");  // ==!==
  g = fopen(newfolder, "a");       // ==!==
  counter = travel->_headerStart;
  fseek(f, travel->_headerStart, SEEK_SET);
  while ((counter < travel->_bodyEnd)
	 && ((eof = fgets(line, 1024, f)) != NULL)) {
    counter += strlen(eof);
    fputs(line, g);
  }
  fputc('\n', g);
  fclose(f);
  fclose(g);

  EraseMail(id);
}

void OMailList::Sort(int mode) {
  switch (mode) {
    case SORT_FROM:
      break;

    case SORT_DATE:
      break;

    case SORT_SUBJECT:
      break;
  }
}
