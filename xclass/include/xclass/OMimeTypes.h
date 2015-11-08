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

#ifndef __OMIMETYPES_H
#define __OMIMETYPES_H

#include <limits.h>
#include <regex.h>

#include <xclass/OBaseObject.h>
#include <xclass/OPicture.h>
#include <xclass/OXClient.h>


//----------------------------------------------------------------------

class OMime : public OBaseObject {
public:
  OMime();
  virtual ~OMime();

public:
  char *type;
  char *pattern;
  char *action;
  char *icon;
  char *description;
  regex_t preg;
  int  has_preg;
  OMime *next;
};

class OMimeTypes : public OBaseObject {
public:
  OMimeTypes(OXClient *, const char *);
  ~OMimeTypes();

  void SaveMimes();
  int  HasChanged() { return _changed; }
  void AddType(char *, const char *, const char *, const char *, const char *);
  void Modify(OMime *, char *, char *, char *, char *, char *);

  void PrintTypes();
  int  GetAction(const char *, char *);
  int  GetType(const char *, char *);
  char *GetDescription(const char *);
  void DeleteType(const char *, const char *, const char *, const char *);
  const OPicture *GetIcon(const char *, int);
  const OPicture *GetIcon(OMime *, int);

  // if arg is NULL, go to first
  OMime *NextMime(OMime *prev) { return (prev) ? prev->next : list->next; }

protected:
  OMime *_Find(const char *);
  void _CompilePattern(const char *, regex_t *);
  
  OMime *list, *listptr;
  OXClient *_clientX;
  char _filename[PATH_MAX];
  int  _changed;
};


#endif  // __OMIMETYPES_H
