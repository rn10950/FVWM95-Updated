/**************************************************************************

    This file is part of xclass, a Win95-looking GUI toolkit.
    Copyright (C) 1996, 1997 Kevin Pearson, Hector Peraza.

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

#ifndef __OINIFILE_H
#define __OINIFILE_H

#include <stdio.h>
#include <limits.h>
#include <xclass/OBaseObject.h>

#define INI_MAX_LINE_LEN    (PATH_MAX+256)
#define INI_READ            100
#define INI_WRITE           101
#define INI_APPEND          102


//----------------------------------------------------------------------

class OIniFile : public OBaseObject {
public:
  OIniFile(const char *filename, int type);
  ~OIniFile() { if (ifp) fclose(ifp); }

  void  Rewind() { if (ifp) fseek(ifp, offset = 0, SEEK_SET); }

  int   GetNext(char *);
  char *GetItem(const char *, char *);
  bool  GetBool(const char *, bool _default = false);

  int   PutNext(const char *);
  int   PutItem(const char *, const char *);
  int   PutBool(const char *, bool);

  int   PutNewLine();

private:
  FILE  *ifp;
  long   offset;
};

#endif  // __OINIFILE_H
