/**************************************************************************

    This file is part of xclass, a Win95-looking GUI toolkit.
    Copyright (C) 1996, 1997 David Barth, Kevin Pearson, Hector Peraza.

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

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <assert.h>
#include <regex.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <xclass/utils.h>
#include <xclass/OIniFile.h>
#include <xclass/OMimeTypes.h>


#define MERGE_PATTERN


//----------------------------------------------------------------------

OMime::OMime() {
  type = pattern = action = icon = description = NULL;
  has_preg = false;
  next = NULL;
}

OMime::~OMime() {
  if (type) delete[] type;
  if (pattern) delete[] pattern;
  if (action) delete[] action;
  if (icon) delete[] icon;
  if (description) delete[] description;
  if (has_preg) regfree(&preg);
}


//----------------------------------------------------------------------

OMimeTypes::OMimeTypes(OXClient *client, const char *filename) {
  char line[1024];
  char mime[1024];
  char pattern[256];
  char icon[256];
  char action[256];
  char *description;
  OIniFile myfile(filename, INI_READ);

  list = new OMime;
  list->next = NULL;
  listptr = list;
  _clientX = client;
  strcpy(_filename, filename);

  while (myfile.GetNext(line)) {
    strcpy(mime, line);
    myfile.GetItem("pattern", pattern);
    myfile.GetItem("icon", icon);
    myfile.GetItem("action", action);
    description = myfile.GetItem("description", NULL);
#ifdef MERGE_PATTERN
    if (strlen(pattern) > 0) {
      // remove trailing spaces, if any (this should go to _CompilePattern)
      char *p = pattern + strlen(pattern) - 1;
      while (*p == ' ') --p;
      *(++p) = '\0';
    }
    AddType(description, mime, pattern, icon, action);
#else
    if (strchr(pattern, ' ') != NULL) {
      tmppattern = strtok(pattern, " ");
      while (tmppattern && (*tmppattern != ' ')) {
        AddType(description, mime, (const char *) tmppattern, icon, action);
        tmppattern = strtok(NULL, " ");
      }
    } else {
      AddType(description, mime, pattern, icon, action);
    }
#endif
    if (description) delete[] description;
  }
  _changed = False;
}

OMimeTypes::~OMimeTypes() {
  OMime *nextptr;

  if (_changed) SaveMimes();
  listptr = list->next;
  while (listptr) {
    nextptr = listptr->next;
    delete listptr;
    listptr = nextptr;
  }
  delete list;
}

OMime *OMimeTypes::_Find(const char *filename) {
  regmatch_t pmatch[1];
  size_t     nmatch = 1;

  listptr = list->next;
  if (filename) {
    while (listptr) {
      if (regexec(&listptr->preg, filename, nmatch, pmatch, 0) == 0) {
        if ((pmatch[0].rm_so == 0) && 
            (pmatch[0].rm_eo == strlen(filename))) {
          return listptr;
        }
      }
      listptr = listptr->next;
    }
  }
  return NULL;
}

const OPicture *OMimeTypes::GetIcon(OMime *mime, int small_icon) {
  const OPicture *pic;
  char tmp[256];

  if (small_icon) {
    if (strstr(mime->icon, ".xpm")) {
      return _clientX->GetPicture(mime->icon, 16, 16);
    } else {
      sprintf(tmp, "%s.t.xpm", mime->icon);
      pic = _clientX->GetPicture(tmp, 16, 16);
      if (pic) return pic;
      sprintf(tmp, "%s.xpm", mime->icon);
      return _clientX->GetPicture(tmp, 16, 16);
    }
  } else {
    if (strstr(mime->icon, ".xpm")) {
      return _clientX->GetPicture(mime->icon, 32, 32);
    } else {
      sprintf(tmp, "%s.s.xpm", mime->icon);
      pic = _clientX->GetPicture(tmp, 32, 32);
      if (pic) return pic;
      sprintf(tmp, "%s.xpm", mime->icon);
      return _clientX->GetPicture(tmp, 32, 32);
    }
  }
}

const OPicture *OMimeTypes::GetIcon(const char *filename, int small_icon) {
  if (_Find(filename))
    return GetIcon(listptr, small_icon);
  return NULL;
}

int OMimeTypes::GetAction(const char *filename, char *action) {
  *action = '\0';
  if (_Find(filename)) {
    strcpy(action, listptr->action);
    return (strlen(action) > 0);
  }
  return 0;
}

int OMimeTypes::GetType(const char *filename, char *type) {
  *type = '\0';
  if (_Find(filename)) {
    strcpy(type, listptr->type);
    return (strlen(type) > 0);
  }
  return 0;
}

char *OMimeTypes::GetDescription(const char *filename) {
  if (_Find(filename))
    return listptr->description;
  return NULL;
}

void OMimeTypes::PrintTypes() {

  listptr = list->next;

  while (listptr) {
    printf("Description: %s\n", listptr->description ? listptr->description : "");
    printf("Type: %s\n", listptr->type);
    printf("Pattern: %s\n", listptr->pattern);
    printf("Action: %s\n", listptr->action);
    printf("------------\n\n");
    listptr = listptr->next;
  }
}

void OMimeTypes::SaveMimes() {
  OMime    *tmpptr;
  OIniFile  myfile(_filename, INI_WRITE);
  
  tmpptr = listptr;
  listptr = list->next;
  while (listptr) {
    myfile.PutNext(listptr->type);
    if (listptr->description)
      myfile.PutItem("description", listptr->description);
    myfile.PutItem("pattern", listptr->pattern);
    myfile.PutItem("icon", listptr->icon);
    myfile.PutItem("action", listptr->action);
    myfile.PutNewLine();
    listptr = listptr->next;
  }
  listptr = tmpptr;
  _changed = False;
}

void OMimeTypes::_CompilePattern(const char *pattern, regex_t *preg) {
  char *expression;
  int  pos = 0;
  const char *ptr = NULL;

#ifdef MERGE_PATTERN
  int merge = (strchr(pattern, ' ') != NULL);
  ptr = pattern;
  pos = 0;
  expression = new char[strlen(pattern)*2 + 10];
  if (merge) expression[pos++] = '(';
  while (*ptr != '\0') {
    if (*ptr == '*')
      expression[pos++] = '.';
    else if (*ptr == '.')
      expression[pos++] = '\\';
    if (*ptr == ' ')
      expression[pos++] = '|';
    else
      expression[pos++] = *ptr;
    ptr++;
  }
  if (merge) expression[pos++] = ')';
  expression[pos] = '\0';
  regcomp(preg, expression, REG_EXTENDED);
  delete[] expression;
#else
  ptr = pattern;
  pos = 0;
  expression = new char[strlen(pattern)*2 + 1];
  while (*ptr != '\0') {
    if (*ptr == '*')
      expression[pos++] = '.';
    else if (*ptr == '.')
      expression[pos++] = '\\';
    expression[pos++] = *ptr;
    ptr++;
  }
  expression[pos] = '\0';
  regcomp(preg, expression, REG_EXTENDED);
  delete[] expression;
#endif
}

void OMimeTypes::AddType(char *description,
                   const char *type, 
		   const char *pattern,
		   const char *icon,
		   const char *action) {

  OMime *tmpmime = new OMime;

  listptr = list;
  while (listptr->next) listptr = listptr->next;

  listptr->next = tmpmime;
  listptr = tmpmime;

  listptr->type = StrDup(type);
  listptr->pattern = StrDup(pattern);
  _CompilePattern(pattern, &listptr->preg);
  listptr->has_preg = true;
  listptr->action = StrDup(action);
  listptr->icon = StrDup(icon);
  listptr->description = description ? StrDup(description) : NULL;
  listptr->next = NULL;
  _changed = True;
}

void OMimeTypes::Modify(OMime *mime, 
                        char *description,
                        char *type, 
                        char *pattern,
                        char *icon,
                        char *action) {

  if (description) {
    if (mime->description) delete[] mime->description;
    mime->description = StrDup(description);
  }

  if (type) {
    if (mime->type) delete[] mime->type;
    mime->type = StrDup(type);
  }

  if (action) {
    if (mime->action) delete[] mime->action;
    mime->action = StrDup(action);
  }

  if (icon) {
    if (mime->icon) delete[] mime->icon;
    mime->icon = StrDup(icon);
  }

  if (pattern) {
    if (mime->pattern) delete[] mime->pattern;
    mime->pattern = StrDup(pattern);
    regfree(&mime->preg);
    _CompilePattern(pattern, &mime->preg);
    mime->has_preg = true;
  }

  _changed = True;
}
