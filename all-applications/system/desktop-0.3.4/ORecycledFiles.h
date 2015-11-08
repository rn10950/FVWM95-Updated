//-*-C++-*-
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

#ifndef __ORECYCLED_FILES_H
#define __ORECYCLED_FILES_H

#include <limits.h>

class File {
public:
  char name[PATH_MAX];
  char storedname[PATH_MAX];
  char date[PATH_MAX];
  char location[PATH_MAX];
  File *next;
};

class ORecycledFiles {
public:
  ORecycledFiles(const char *, const char *);
  ~ORecycledFiles();

  void SaveRecycledrc();
  const char *GetRecycleBin() const { return _recyclebin; }
  int  HasChanged() const { return _changed; };
  void AddFile(const char *, const char *, const char *, const char *);
  void PrintFiles();
  void DeleteFile(const char *, const char *, const char *, const char *);
  void EmptyRecycleBin();

protected:
  File *list, *listptr;
  char _filename[PATH_MAX];
  char _recyclebin[PATH_MAX];
  int  _changed;
};


#endif  // __ORECYCLED_FILES_H
