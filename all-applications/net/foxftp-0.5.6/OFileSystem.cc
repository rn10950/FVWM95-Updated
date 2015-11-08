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

#include <limits.h>

#include "OFileSystem.h"
#include "OXLogView.h"


//----------------------------------------------------------------------

OFileSystem::OFileSystem() {
  _log = NULL;
  _state = FS_STATE_NOT_READY;
  _pwd = new char[PATH_MAX];
}

OFileSystem::~OFileSystem() {
  if (_pwd) delete[] _pwd;
}

void OFileSystem::Log(const char *txt, const char *string) {

  if (_log == NULL) return;

  char buf[256];
  sprintf(buf, "%s%s", txt, string);
  if (strcmp(txt, ">> ") == 0)
    _log->AddLine(SENT_TYPE, buf);
  else
    _log->AddLine(RECEIVE_TYPE, buf);
}
