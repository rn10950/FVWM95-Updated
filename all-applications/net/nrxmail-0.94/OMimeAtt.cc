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

#include "OMimeAtt.h"


//----------------------------------------------------------------------

OMimeAtt::OMimeAtt(FILE **f, char *mailfile, char *boundary, long border) {
  OPosition pos(0, 0);
  SParm *travel;
  const char *templine, *eof, *first, *second;
  char line[1024];

  char *normboundary = new char[strlen(boundary) + 4];
  sprintf(normboundary, "--%s\n", boundary);

  char *endboundary = new char[strlen(boundary) + 6];
  sprintf(endboundary, "--%s--\n", boundary);

  _isKnown = False;
  _isMultipart = False;
  _encoding = BIT7;
  _parameters = NULL;
  _name = NULL;
  _type = NULL;
  _next = NULL;

  _mailfile = StrDup(mailfile);

  _start = ftell(*f);

  // Kram auslesen
  OText *contHeader = new OText();
  while (((eof = fgets(line, 1024, *f)) != NULL) && (strlen(line) > 1));
  _contStart = ftell(*f);
  contHeader->Load(_mailfile, _start, _contStart - _start);
  UnFold(contHeader);
  while ((ftell(*f) < border) && ((eof = fgets(line, 1024, *f)) != NULL)
	 && (strcmp(line, normboundary))) {
    if (!strcmp(line, endboundary)) break;
    while (line[strlen(line) - 1] != '\n') {
      eof = fgets(line, 1024, *f);
      if (eof == NULL) break;
    }
  }
  if (eof == NULL) {
    _end = _start;
    printf("Mime allocation error!\n");
    return;
  }
  _end = ftell(*f) - strlen(line) - 1;

  fseek(*f, _end + 1, SEEK_SET);

  for (pos.y = 0; pos.y < contHeader->RowCount(); pos.y++) {
    templine = contHeader->GetLine(pos);
    if (templine && !(strncasecmp(templine, "Content-type: ", 14))) {
      second = strchr(templine, ';');
      if (second == NULL)
	second = templine + strlen(templine);
      first = templine + 14;
      _type = new char[second - first + 1];
      strncpy(_type, first, second - first);
      _type[second - first] = '\0';
      SetParameters(templine);
      travel = _parameters;
      while (travel != NULL) {
	if (!strcmp(travel->identifier, "name")) {
	  _name = StrDup(travel->value);
	  break;
	}
	travel = travel->next;
      }
    }
    if (_type == NULL) {
      _type = StrDup("text/plain");
    }
    if (_name == NULL) {
      if (!strcasecmp(_type, "message/rfc822")) {
	_name = StrDup("Mail");
      } else {
	_name = StrDup("????");
      }
    }
    if ((templine != NULL)
	&& !(strncasecmp(templine, "Content-Transfer-Encoding: ", 27))) {
      first = templine + 27;
      if (!(strncasecmp(first, "base64", 6)))
	_encoding = BASE64;
      else if (!(strncasecmp(first, "7bit", 4)))
	_encoding = BIT7;
      else if (!(strncasecmp(first, "8bit", 4)))
	_encoding = BIT8;
      else if (!(strncasecmp(first, "quoted-printable", 16)))
	_encoding = QUOTED_PRINTABLE;
      else if (!(strncasecmp(first, "quoted printable", 16)))
	_encoding = QUOTED_PRINTABLE;
    }
  }
}

OMimeAtt::~OMimeAtt() {
  delete[] _mailfile;
  delete[] _name;
  delete[] _type;

  SParm *travel = _parameters;
  SParm *toDelete;
  while (travel != NULL) {
    toDelete = travel;
    delete[] travel->identifier;
    delete[] travel->value;
    travel = travel->next;
    delete toDelete;
  }
}

void OMimeAtt::SetParameters(const char *line) {
  SParm *travel, *delptr;
  const char *first, *second;

  first = strchr(line, ';');
  if (first == NULL) return;

#if 1
  second = first + 1;
  while (*second == ' ') ++second;
  if (!*second) return;
#endif

  _parameters = new SParm;
  travel = _parameters;
  travel->next = NULL;

  while (first != NULL) {
    first++;
    if (*first == ' ') first++;
    second = strchr(first, '=');
    travel->identifier = new char[second - first + 1];
    strncpy(travel->identifier, first, second - first);
    travel->identifier[second - first] = '\0';
    first = second + 1;
    if (*first == '"') {
      first++;
      second = strchr(first, '"');
    } else {
      second = strchr(first, ';');
    }
    if (second == NULL)
      second = line + strlen(line) - 1;
    travel->value = new char[second - first + 1];
    strncpy(travel->value, first, second - first);
    travel->value[second - first] = '\0';
    travel->next = new SParm;
    delptr = travel;
    travel = travel->next;
    travel->next = NULL;
    first = strchr(second, ';');
  }
  delete travel;
  delptr->next = NULL;
}

bool OMimeAtt::Save(char *filename) {
  FILE *g = fopen(filename, "w");  // ==!== fopen might fail!
  bool ret;

  switch (_encoding) {
  case BASE64:
    ret = DecodeBase64(g);
    break;

  case BIT7:
    ret = Decode7Bit(g);
    break;

  case BIT8:
    ret = Decode8Bit(g);
    break;

  case QUOTED_PRINTABLE:
    ret = DecodeQuotedPrintable(g);
    break;

  default:
    ret = False;
  }
  fclose(g);

  return ret;
}

bool OMimeAtt::Decode7Bit(FILE *g) {
  FILE *f;
  char *eof;
  char line[1025];

  f = fopen(_mailfile, "r");  // ==!== fopen might fail!
  fseek(f, _contStart, SEEK_SET);
  fseek(g, 0, SEEK_SET);

  while ((ftell(f) <= _end) && ((eof = fgets(line, 1024, f)) != NULL)) {
    for (int i = 0; i < strlen(line); i++)
      line[i] = line[i] & 0x7F;
    fputs(line, g);
  }

  fclose(f);

  if (eof != NULL)
    return True;
  else
    return False;
}

bool OMimeAtt::Decode8Bit(FILE *g) {
  FILE *f;
  char *eof;
  char line[1025];

  f = fopen(_mailfile, "r"); // ==!==
  fseek(f, _contStart, SEEK_SET);
  fseek(g, 0, SEEK_SET);

  while ((ftell(f) <= _end) && ((eof = fgets(line, 1024, f)) != NULL))
    fputs(line, g);

  fclose(f);

  if (eof != NULL)
    return True;
  else
    return False;
}

bool OMimeAtt::DecodeQuotedPrintable(FILE *g) {
  FILE *f;
  int  c, eof = false;

  f = fopen(_mailfile, "r"); // ==!==
  fseek(f, _contStart, SEEK_SET);
  fseek(g, 0, SEEK_SET);

  while (ftell(f) <= _end) {
    c = fgetc(f);
    if (c == EOF) {
      eof = true;
      break;
    }
    if (c == '=') {
      int c1, c2;
      c1 = fgetc(f);
      if (c1 == '\n') {
        c = c1;
      } else {
        if (c1 >= 'A') c1 -= 'A', c1 += 10; else c1 -= '0';
        c2 = fgetc(stdin);
        if (c2 >= 'A') c2 -= 'A', c2 += 10; else c2 -= '0';
        c = c1 * 16 + c2;
      }
    }
    fputc(c, g);
  }

  fclose(f);

  return !eof;
}

bool OMimeAtt::DecodeBase64(FILE *g) {
  FILE *f;
  unsigned char value[4], newVal[3];
  unsigned char tempVal;
  unsigned char writeLine[57];
  char readLine[78];
  char *eof;
  int count, count2;

  f = fopen(_mailfile, "r");  // ==!==
  fseek(f, _contStart, SEEK_SET);
  fseek(g, 0, SEEK_SET);

  count2 = 0;
  while ((ftell(f) <= _end) && ((eof = fgets(readLine, 78, f)) != NULL)
	 && (strlen(eof) <= 77)) {

    for (int i = 0; i < 57; i++)
      writeLine[i] = 0;

    count = 0;
    for (int i = 0; i < (strlen(readLine) - 1) / 4; i++) {
      for (int j = 0; j < 4; j++) {
	if ((readLine[i*4 + j] >= 65) && (readLine[i*4 + j] <= 90)) {
	  value[j] = readLine[i*4 + j] - 65;
	} else if ((readLine[i*4 + j] >= 97) && (readLine[i*4 + j] <= 122)) {
	  value[j] = readLine[i*4 + j] - 71;
	} else if ((readLine[i*4 + j] >= 48) && (readLine[i*4 + j] <= 57)) {
	  value[j] = readLine[i*4 + j] + 4;
	} else if (readLine[i*4 + j] == 43) {
	  value[j] = readLine[i*4 + j] + 19;
	} else if (readLine[i*4 + j] == 47) {
	  value[j] = readLine[i*4 + j] + 16;
	} else if (readLine[i*4 + j] == '=') {
	  value[j] = 0;
	} else {
	  fclose(f);
	  return False;
	}
      }

      newVal[0] = value[0] << 2;
      tempVal = value[1] >> 4;
      newVal[0] = newVal[0] | tempVal;
      newVal[1] = value[1] << 4;
      tempVal = value[2] >> 2;
      newVal[1] = newVal[1] | tempVal;
      newVal[2] = value[2] << 6;
      newVal[2] = newVal[2] | value[3];

      writeLine[count] = newVal[0];
      count++;
      writeLine[count] = newVal[1];
      count++;
      writeLine[count] = newVal[2];
      count++;
    }
    fwrite(writeLine, sizeof(unsigned char), count, g);
  }
  fclose(f);

  return True;
}
