/**************************************************************************

    This file is part of foxftp, a ftp client for fOX.
    Copyright (C) 1998, 1999, G. Matzka.

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

#ifndef __OFILE_H
#define __OFILE_H

#include <sys/types.h>

template <class T> class TDDLList;

class OFile {
public:
  OFile();
  OFile(const OFile *);
  virtual ~OFile();

  char *_name;
  char *_user;
  char *_group;
  int _type;
  time_t _time;
  int _is_link;
  long _size;

  void SetName(char *name);
  void SetUser(char *user);
  void SetGroup(char *group);
};

class ODir {
public:
  ODir(char *path, TDDLList<OFile *> *list);
  virtual ~ODir();
 
  void SetPath(char *path);
  void SetList(TDDLList<OFile *> *list);
  char *GetPath() { return _path; }
  TDDLList<OFile *> *GetList() { return _list; }
 
protected:
  char *_path;
  TDDLList<OFile *> *_list;
};

#endif  // __OFILE_H
