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

#include <stdio.h>

#include <xclass/utils.h>

#include "TDList.h"
#include "OFile.h"


//----------------------------------------------------------------------

OFile::OFile() {
  _type = 0;
  _size = 0;
  _is_link = 0;
  _name = _user = _group = NULL;
}

OFile::OFile(const OFile *item) {
  _type = item->_type;
  _size = item->_size;
  _is_link = item->_is_link;

  _name = _user = _group = NULL;
  SetName(item->_name);
  SetUser(item->_user);
  SetGroup(item->_group);
}

OFile::~OFile() {
  if (_name) delete[] _name;
  if (_name) delete[] _user;
  if (_name) delete[] _group;
}

void OFile::SetName(char *name) {
  if (_name) delete[] _name;
  _name = NULL;
  if (name) _name = StrDup(name);
}

void OFile::SetUser(char *user) {
  if (_user) delete[] _user;
  _user = NULL;
  if (user) _user = StrDup(user);
}

void OFile::SetGroup(char *group) {
  if (_group) delete[] _group;
  _group = NULL;
  if (group) _group = StrDup(group);
}


//----------------------------------------------------------------------

ODir::ODir(char *path, TDDLList<OFile *> *list) {
  _path = NULL;
  _list = NULL;
  SetPath(path);
  SetList(list);
}

ODir::~ODir() {
  if (_path) delete[] _path;
  if (_list) delete _list;
}

void ODir::SetPath(char *path) {
  if (_path) delete[] _path;
  _path = NULL;
  if (path) _path = StrDup(path);
}

void ODir::SetList(TDDLList<OFile *> *list) {
  if (_list) delete _list;
  _list = list;
}
