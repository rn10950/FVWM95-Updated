/**************************************************************************

    This file is part of explorer95, a file manager for fvwm95.
    Copyright (C) 1996, 1997 David Barth, Hector Peraza.

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
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <assert.h>

#include <X11/Xlib.h>
#include <xclass/OIniFile.h>

#include "ORecycledFiles.h"


//----------------------------------------------------------------------

ORecycledFiles::ORecycledFiles(const char *bin, const char *filename) {
  char name[PATH_MAX];
  char storedname[PATH_MAX];
  char date[PATH_MAX];
  char location[PATH_MAX];
  char line[PATH_MAX];
  char *tmpext;
  struct stat inode;
  mode_t permissions = S_IRUSR | S_IWUSR | S_IXUSR;

  OIniFile myfile(filename, INI_READ);

  list = new File;
  listptr = list;
  strcpy(_recyclebin, bin);
  strcpy(_filename, filename);

  if (stat(bin, &inode) != 0)
    mkdir(bin, permissions);

  while (myfile.GetNext(line)) {
    strcpy(name, line);
    myfile.GetItem("storedname", storedname);
    myfile.GetItem("date", date);
    myfile.GetItem("location", location);
    AddFile(name, storedname, date, location);
  }
  _changed = False;
}

ORecycledFiles::~ORecycledFiles() {
  File *nextptr;

  if (_changed) SaveRecycledrc();
  listptr = list->next;
  while (listptr != NULL) {
    nextptr = listptr->next;
    delete listptr;
    listptr = nextptr;
  }
}

void ORecycledFiles::SaveRecycledrc() {
  File *lptr;

  OIniFile myfile(_filename, INI_WRITE);

  lptr = list->next;
  while (lptr != NULL) {
    myfile.PutNext(lptr->name);
    myfile.PutItem("storedname", lptr->storedname);
    myfile.PutItem("date", lptr->date);
    myfile.PutItem("location", lptr->location);
    myfile.PutNewLine();
    lptr = lptr->next;
  }
}

void ORecycledFiles::PrintFiles() {
  File *lptr;

  lptr = list->next;

  while (lptr != NULL) {
    printf("Name: %s\n", lptr->name);
    printf("Stored Name: %s\n", lptr->storedname);
    printf("Date: %s\n", lptr->date);
    printf("Location: %s\n", lptr->location);
    printf("------------\n\n");
    lptr = lptr->next;
  }
}

void ORecycledFiles::AddFile(const char *name, const char *storedname, 
                             const char *date, const char *location) {
  File *tmpfile;

  tmpfile = new File;
  listptr->next = tmpfile;
  listptr = tmpfile;
  strcpy(listptr->name, name);
  strcpy(listptr->storedname, storedname);
  strcpy(listptr->date, date);
  strcpy(listptr->location, location);
  listptr->next = NULL;
  _changed = True;
}

void ORecycledFiles::DeleteFile(const char *name, const char *storedname, 
                                const char *date, const char *location) {
  File *lptr, *prev;

  prev = list;
  lptr = list->next;

  while (lptr != NULL) {
    if ((strcmp(name, lptr->name) == 0) &&
        (strcmp(storedname, lptr->storedname) == 0) &&
        (strcmp(date, lptr->date) == 0) &&
        (strcmp(location, lptr->location) == 0)) {
      if (listptr == lptr) {
        listptr = prev;
      }
      prev->next = lptr->next;
      delete lptr;
      _changed = True;
      break;
    }
    prev = lptr;
    lptr = lptr->next;
  }
}

void ORecycledFiles::EmptyRecycleBin() {
  File *nextptr;

  listptr = list->next;
  while (listptr != NULL) {
    nextptr = listptr->next;
    delete listptr;
    listptr = nextptr;
  }
  list->next = NULL;
  listptr = list;
  _changed = True;
}
