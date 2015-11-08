/**************************************************************************

    This file is part of xclass' desktop manager.
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

#ifndef __URL_H
#define __URL_H

#include <xclass/OBaseObject.h>


//-------------------------------------------------------------------

class URL : public OBaseObject {
public:
  URL(const char *url);
  virtual ~URL();
  
  char *full_url;
  char *full_path;
  char *scheme;
  char *host;
  char *directory;
  char *filename;
};


#endif  // __URL_H
