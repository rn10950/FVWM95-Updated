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

#include "OMail.h"
#include "OMimeAtt.h"


//----------------------------------------------------------------------

void UnFold(OText *field) {
  OPosition pos;
  OPosition pos2, pos3;
  bool flag = False;
  const char *line;

//printf("Unfold:\nEntry:\n");
//for (pos.y = 0; pos.y < field->RowCount(); pos.y++)
//printf("  %s\n", field->GetLine(pos));

  for (pos.y = 0; pos.y < field->RowCount(); pos.y++) {
    flag = False;

    for (pos.x = 0; pos.x < field->GetLineLength(pos.y); pos.x++) {
      switch (field->GetChar(pos)) {
      case ' ':
	if (flag) {   // remove leading spaces and tabs?
          pos2.x = pos.x + 1;
          pos2.y = pos.y;
	  field->DelChar(pos2);
	  pos.x--;
	} else {
	  flag = True;
	}
	break;

#if 0
      case '\t':
	pos2.y = pos.y;
	pos2.x = pos.x + 7;
	field->DelText(pos, pos2);
	if (!flag) {
	  flag = True;
	  field->InsChar(pos, ' ');
	} else {
	  pos.x--;
	}
	break;
#else
      case '\t':  // replace tabs with spaces, remove leading tabs
        pos2.y = pos.y;
        pos2.x = pos.x + 1;
	field->DelChar(pos2);
	if (!flag) {
	  flag = True;
	  field->InsChar(pos, ' ');
	} else {
	  pos.x--;
	}
	break;
#endif

      default:
	flag = False;
	break;
      }
    }
  }

  pos.x = 0;
  for (pos.y = 1; pos.y < field->RowCount(); pos.y++) {
    if (field->GetLineLength(pos.y) == 0) {
      field->DelLine(pos.y);
      pos.y--;
    } else if (field->GetChar(pos) == ' ') {
      line = field->GetLine(pos);
      pos2.y = pos.y - 1;
      pos2.x = field->GetLineLength(pos2.y);
      field->InsText(pos2, line);
      field->DelLine(pos.y);
      pos.y--;
    }
  }

//printf("Exit:\n");
//for (pos.y = 0; pos.y < field->RowCount(); pos.y++)
//printf("  %s\n", field->GetLine(pos));
//printf("\n\n");
//fflush(stdout);

}


//----------------------------------------------------------------------

OMail::OMail(OMailList *list) {
  _mailList = list;
  _headerStart = _bodyStart = _bodyEnd = 0;
  _subject = _date = _folder = _message_id = NULL;
  _sender = _from = _to = _cc = _replyTo = NULL;
  _isMultipart = _isMime = False;
  _mimes = NULL;
  _mimeBoundary = NULL;
  next = NULL;
  prev = NULL;
}

OMail::~OMail() {
  OMimeAtt *travel, *toDelete;

  delete[] _subject;
  delete[] _date;
  delete[] _folder;
  delete[] _message_id;
  delete _from;
  delete _to;
  delete _cc;
  delete _replyTo;
  delete[] _mimeBoundary;

  travel = toDelete = _mimes;
  while (travel != NULL) {
    travel = travel->_next;
    delete toDelete;
    toDelete = travel;
  }
}

OText *OMail::GetMessage() {
  OText *text = NULL;

  if (!(_isMime && _isMultipart)) {
    text = new OText();
    text->Load(_folder, _bodyStart, _bodyEnd - _bodyStart + 1);
  } else {
    text = new OText();
    text->Load(_folder, _mimes->GetContentsStart(),
	       _mimes->GetEnd() - _mimes->GetContentsStart());
  }
  return text;
}

OText *OMail::GetMessagePlusHeader() {
  SAddress *travel;
  OPosition pos;

  OText *body = GetMessage();
  if (body == NULL) return NULL;

  pos.x = pos.y = 0;
  body->InsLine(pos.y, "Date: ");
  pos.x = 6;
  body->InsText(pos, GetDate());
  pos.y++;
  body->InsLine(pos.y, "From: ");
  pos.x = 6;
  body->InsText(pos, GetFrom()->rawadress);
  pos.y++;

  travel = GetTo();
  if (travel != NULL) {
    body->InsLine(pos.y, "To: ");
    pos.x = 4;
    body->InsText(pos, travel->rawadress);
    travel = travel->next;
    pos.y++;
    while (travel != NULL) {
      body->InsLine(pos.y, "    ");  // \t ??? (comma in between?)
      body->InsText(pos, travel->rawadress);
      travel = travel->next;
      pos.y++;
    }
  }
  travel = GetCC();
  if (travel != NULL) {
    body->InsLine(pos.y, "CC: ");
    pos.x = 4;
    body->InsText(pos, travel->rawadress);
    travel = travel->next;
    pos.y++;
    while (travel != NULL) {
      body->InsLine(pos.y, "    ");  // \t ???
      body->InsText(pos, travel->rawadress);
      travel = travel->next;
      pos.y++;
    }
  }
  pos.x = 0;
  body->InsLine(pos.y, "Subject: ");
  pos.x = 9;
  body->InsText(pos, GetSubject());
  pos.x = body->GetLineLength(pos.y);
  body->BreakLine(pos);
  body->BreakLine(pos);

  return body;
}

OText *OMail::GetMessageSrc() {
  OText *text = new OText();
  text->Load(_folder, _headerStart, _bodyEnd - _headerStart);
  return text;
}

void OMail::SaveMail(char *file) {
  FILE *f, *g;
  char *eof, line[1024];
  unsigned long count = _bodyEnd - _headerStart;

  f = fopen(_folder, "r");  // ==!== check for failures!
  g = fopen(file, "a");    // ==!==
  fseek(f, _headerStart, SEEK_SET);
  while (count > 0) {
    eof = fgets(line, 1024, f);
    fputs(line, g);
    count = count - strlen(line);
  }
  fputc('\n', g);
  fclose(f);
  fclose(g);
}

void OMail::InitMail(char *mailfolder, long start, long body, long end) {
  OText *header;
  OPosition pos;
  pos.y = pos.x = 0;

  _folder = StrDup(mailfolder);

  _headerStart = start;
  _bodyStart = body;
  _bodyEnd = end;
  header = new OText();
  header->Load(mailfolder, start, body - 2 - start);
  UnFold(header);
  ParseFields(header);
  delete header;

  if (IsMime() && _isMultipart) ParseMime();
}

void OMail::ParseMime() {
  OMimeAtt *travel;
  int count = 0;
  FILE *f;
  char line[1024];
  char *eof;

  char *boundary = new char[strlen(_mimeBoundary) + 4];
  sprintf(boundary, "--%s\n", _mimeBoundary);

  char *endboundary = new char[strlen(_mimeBoundary) + 6];
  sprintf(endboundary, "--%s--\n", _mimeBoundary);

  f = fopen(_folder, "r");  // ==!==
  fseek(f, _bodyStart, SEEK_SET);

  while ((ftell(f) < _bodyEnd) && ((eof = fgets(line, 1024, f)) != NULL)
	 && (strcmp(line, boundary))) {
    while ((line[strlen(line) - 1] != '\n')
	   && ((eof = fgets(line, 1024, f)) != NULL));
  }

  if (eof == NULL) {
    delete _mimes;
    delete[] boundary;
    delete[] endboundary;
    _mimes = NULL;
    fclose(f);
    printf("MIME read error!\n");  // ==!==
    return;
  }

  _mimes = new OMimeAtt(&f, _folder, _mimeBoundary, _bodyEnd);

  count++;
  travel = _mimes;
  while (((eof = fgets(line, 1024, f)) != NULL)
	 && strcmp(line, endboundary) && (ftell(f) < _bodyEnd)) {
    travel->_next = new OMimeAtt(&f, _folder, _mimeBoundary, _bodyEnd);
    count++;
    travel = travel->_next;
  }

  fclose(f);

  delete[] boundary;
  delete[] endboundary;
}

void OMail::ParseFields(OText *fields) {
  OPosition pos;
  const char *line;
  const char *boundString;
  const char *first = NULL;
  const char *second = NULL;
  bool mimeis = False;
  _isMultipart = _isMime = False;
  pos.x = pos.y = 0;

  for (pos.y; pos.y < fields->RowCount(); pos.y++) {
    line = fields->GetLine(pos);

    if (!(strncasecmp(line, "Sender:", 7)))
      _sender = SetAdresses("Sender:", line);

    else if (!(strncasecmp(line, "From:", 5)))
      _from = SetAdresses("From:", line);

    else if (!(strncasecmp(line, "To:", 3)))
      _to = SetAdresses("To:", line);

    else if (!(strncasecmp(line, "Subject:", 8)))
      _subject = SetSubject(line);

    else if (!(strncasecmp(line, "Message-id:", 11)))
      _message_id = SetMessageID(line);

    else if (!(strncasecmp(line, "Date:", 5)))
      _date = SetDate(line);

    else if (!(strncasecmp(line, "CC:", 3)))
      _cc = SetAdresses("CC:", line);

    else if (!(strncasecmp(line, "Reply-To:", 9)))
      _replyTo = SetAdresses("Reply-To:", line);

    else if (!(strncasecmp(line, "MIME-Version:", 13)))
      mimeis = _isMime = True;

    if (mimeis) {
      if (!(strncasecmp(line, "Content-Type: multipart", 23))) {
	mimeis = False;
	_isMultipart = True;
	boundString = strstr(line, "Boundary=");
	if (boundString == NULL)
	  boundString = strstr(line, "BOUNDARY=");
	if (boundString == NULL)
	  boundString = strstr(line, "boundary=");
	if (boundString != NULL) {
	  first = strchr(boundString, '"');
	  if (first != NULL) {
	    first++;
	    second = strchr(first, '"');
	    _mimeBoundary = new char[second - first + 1];
	    strncpy(_mimeBoundary, first, second - first);
	    _mimeBoundary[second - first] = '\0';
	  } else {
	    _mimeBoundary = new char[strlen(line) - strlen(boundString) +9+1];
	    strcpy(_mimeBoundary, (boundString + 9));
	  }
	}
      }
    }
  }

  if (_from == NULL) _from = _sender;

  if (_subject == NULL) _subject = StrDup("");
}

char *OMail::SetSubject(const char *line) {
  const char *p = line + strlen("Subject:");
  while (*p == ' ') ++p;
  return StrDup(p);
}

char *OMail::SetMessageID(const char *line) {
  const char *p = line + strlen("Message-id:");
  while (*p == ' ') ++p;
  return StrDup(p);
}

char *OMail::SetDate(const char *line) {
  const char *start = line;
  int count = 0;
  char wd[12], dd[12], mm[12], yy[12];
  wd[0] = dd[0] = mm[0] = yy[0] = '\0';

  sscanf(line, "Date: %10s %10s %10s %10s", wd, dd, mm, yy);
  if (strchr(yy, ':')) strcpy(yy, "");
  char *retval = new char[strlen(wd) + strlen(dd) 
                        + strlen(mm) + strlen(yy) + 6];
  sprintf(retval, "%s %2s %s %s", wd, dd, mm, yy);

  return retval;
}

// ==!== this segfaults when line is just "Cc: "

SAddress *OMail::SetAdresses(const char *field, const char *line) {
  SAddress *retval, *travel, *tv2;
  const char *strtravel, *strend;
  const char *strname;
  int i = 0;
  char *lineText;
  OPosition pos;

  OText *tempFields = new OText();

  bool flag = True;
  strtravel = line + strlen(field);
  if (*strtravel == ' ') strtravel++;
  pos.x = pos.y = 0;

  while (flag) {
    while ((*strtravel == ' ') || (*strtravel == ',')) strtravel++;
    if (*strtravel == '"')
      strname = strchr(strtravel + 1, '"');
    else
      strname = strtravel;
    strend = strchr(strname, ',');
    if (strend == NULL) {
      strend = line + strlen(line);
      flag = False;
    }
    lineText = new char[strend - strtravel + 1];
    strncpy(lineText, strtravel, strend - strtravel);
    lineText[strend - strtravel] = '\0';
    tempFields->InsText(pos, lineText);
    delete[] lineText;
    pos.y++;
    strtravel = strend;
  }
  pos.y = 0;

  retval = new SAddress;
  retval->next = NULL;
  retval->adress = NULL;
  retval->name = NULL;
  retval->rawadress = NULL;
  travel = retval;

  for (pos.y; pos.y < tempFields->RowCount(); pos.y++) {
    const char *line = tempFields->GetLine(pos);
    travel->rawadress = StrDup(line);
    if (strchr(line, '<') != NULL) {
      SetAdressFields(line, '<', &travel->name, &travel->adress);
    } else {
      if (strchr(line, '(') != NULL) {
        SetAdressFields(line, '(', &travel->adress, &travel->name);
      } else {
        travel->name = NULL;
        travel->adress = StrDup(line);
      }
    }
    travel->next = new SAddress;
    tv2 = travel;
    travel = travel->next;
    travel->name = NULL;
    travel->next = NULL;
    travel->adress = NULL;
    travel->rawadress = NULL;
  }
  delete tv2->next;
  tv2->next = NULL;

  return retval;
}

void OMail::SetAdressFields(const char *line, char indicator,
                            char **one, char **two) {
  const char *travel = strchr(line, indicator);

  while (*line == ' ') line++;
  *one = new char[travel - line + 1];
  strncpy(*one, line, travel - line);
  (*one)[travel - line] = '\0';
  travel++;
  while (*travel == ' ') travel++;
  *two = new char[line + strlen(line) - travel];
  strncpy(*two, travel, line + strlen(line) - travel - 1);
  (*two)[line + strlen(line) - travel - 1] = '\0';
}
