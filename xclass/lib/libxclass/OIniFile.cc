/**************************************************************************

    This file is part of xclass, a Win95-looking GUI toolkit.
    Copyright (C) 1996, 1997 David Barth, Kevin Pearson, Hector Peraza.

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

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <xclass/utils.h>
#include <xclass/OIniFile.h>


OIniFile::OIniFile(const char *filename, int type) { 

  if (type == INI_READ)
    ifp = fopen(filename, "r");
  else if (type == INI_WRITE)
    ifp = fopen(filename, "w");
  else if (type == INI_APPEND)
    ifp = fopen(filename, "a");

  offset = 0;
}

char *OIniFile::GetItem(const char *item, char *value) {

  char  line[INI_MAX_LINE_LEN];
  char *tmp;

  if (value) *value = '\0';

  if (!ifp) return NULL;

  fseek(ifp, offset, SEEK_SET);
  while (!(feof(ifp))) {
    if (fgets(line, INI_MAX_LINE_LEN, ifp) != NULL) {
      line[strlen(line) - 1] = '\0';
      if ((strchr(line, '[') != 0) && (strchr(line, ']') != 0)) {
	return NULL;
      }
      if (strstr(line, item) == line) {
	strtok(line, "=");
        tmp = strtok(NULL, "=");
        while (tmp && (*tmp == ' ')) tmp++;
        if (tmp) {
          if (!value) value = new char[strlen(tmp)+1];
          strcpy(value, tmp);
          return value;
	} else {
	  return NULL;
	}
      }
    }
  }
  return NULL;
}

bool OIniFile::GetBool(const char *item, bool _default) {
  char line[INI_MAX_LINE_LEN];
  char *tmp;

  if (!ifp) return false;

  fseek(ifp, offset, SEEK_SET);
  while (!(feof(ifp))) {
    if (fgets(line, INI_MAX_LINE_LEN, ifp) != NULL) {
      line[strlen(line) - 1] = '\0';
      if ((strchr(line, '[') != 0) && (strchr(line, ']') != 0)) {
        return _default;
      }
      if (strstr(line, item) == line) {
        strtok(line, "=");
        tmp = strtok(NULL, "=");
        while (tmp && (*tmp == ' ')) tmp++;
        if (tmp) {
          if (strncasecmp(tmp, "true", 4) == 0) return true;
          if (strncasecmp(tmp, "yes", 3)  == 0) return true;
        }
        return false;
      }
    }
  }
  return _default;
}

int OIniFile::GetNext(char *type) {

  char  line[INI_MAX_LINE_LEN];
  char *start, *end;

  if (!ifp) return 0;

  fseek(ifp, offset, SEEK_SET);
  while (!(feof(ifp))) {
    if (fgets(line, INI_MAX_LINE_LEN, ifp) != NULL) {
      line[strlen(line) - 1] = '\0';
      if (((start = strchr(line, '[')) != NULL) && 
          ((end = strchr(start, ']')) != NULL)) {
        *end = '\0';
	strcpy(type, ++start);
	offset = ftell(ifp);
 	return 1;
      }
    }
  }
  return 0;
}

int OIniFile::PutItem(const char *item, const char *value) {
  if (ifp) {
    return fprintf(ifp, "%s = %s\n", item, value);
  }
  return 0;
}

int OIniFile::PutBool(const char *item, bool value) {
  if (ifp) return fprintf(ifp, "%s = %s\n", item, value ? "true" : "false");
  return 0;
}

int OIniFile::PutNext(const char *type) {
  if (ifp) return fprintf(ifp, "[%s]\n", type);
  return 0;
}

int OIniFile::PutNewLine() {
  if (ifp) return fprintf(ifp, "\n");
  return 0;
}
