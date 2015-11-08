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

#include <xclass/utils.h>

#include "URL.h"


// This is a simple class used to parse URLs
//
// It currently understands the standard URL specification, as well as
// a few "broken" formats used by some applications like netscape and
// some gnome/kde apps.
//
// Examples:
//   url:http://www.yahoo.com/search/google/index.html
//   http://www.yahoo.com/search/google/index.html
//   file:///home/documents/index.txt
//   file:/home/documents/index.txt
//   /home/documents/doc.txt

//#define DEBUG_URL


//-------------------------------------------------------------------

URL::URL(const char *urlspec) {
  char *p, *p1;
  const char *url;

  url = urlspec;
  if (strncasecmp(url, "url:", 4) == 0) url += 4;
  full_url = StrDup(url);

  p = strchr(full_url, '\n');
  if (p) *p = '\0';
  p = strchr(full_url, '\r');
  if (p) *p = '\0';

  p = strchr(full_url, ':');
  if (p) {
    scheme = new char[p - full_url + 1];
    strncpy(scheme, full_url, p - full_url);
    scheme[p - full_url] = '\0';
    if (strncmp(++p, "//", 2) == 0) {
      p += 2;
      p1 = strchr(p, '/');
      if (p1) {
        full_path = StrDup(p1);  // + 1
        host = new char[p1 - p + 1];
        strncpy(host, p, p1 - p);
        host[p1 - p] = '\0';
      } else {
        host = StrDup(p);
        full_path = StrDup("");
      }
    } else {
      host = StrDup("");
      full_path = StrDup(p);
    }
  } else {
    scheme = StrDup("");
    host = StrDup("");
    full_path = StrDup(full_url);
  }

  p = strrchr(full_path, '/');
  if (p) {
    filename = StrDup(p + 1);
    directory = new char[p - full_path + 1];
    strncpy(directory, full_path, p - full_path);
    directory[p - full_path] = '\0';
  } else {
    filename = StrDup(full_path);
    directory = StrDup("");
  }

#ifdef DEBUG_URL
  printf("url:    \"%s\"\n"
         "scheme: \"%s\"\n"
         "host:   \"%s\"\n"
         "path:   \"%s\"\n"
         "dir:    \"%s\"\n"
         "doc:    \"%s\"\n\n", full_url, scheme, host,
                               full_path, directory, filename);
#endif
}

URL::~URL() {
  delete[] full_url;
  delete[] scheme;
  delete[] host;
  delete[] full_path;
  delete[] directory;
  delete[] filename;
}
