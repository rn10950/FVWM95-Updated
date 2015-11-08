/**************************************************************************

    This file is part of xclass, a Win95-looking GUI toolkit.
    Copyright (C) 1996, 1997 David Barth, Hector Peraza.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

**************************************************************************/

#include <stdio.h>
#include <unistd.h>

#include <xclass/utils.h>
#include <xclass/OFileHandler.h>


int OFileHandler::id_count = 0;

OFileHandler::OFileHandler(OComponent *c, int fd, unsigned int event_mask) {

  if (c != NULL) {

    _c = c;
    _client = c->_client;
    _id = ++id_count;

    _fd = fd;
    _eventMask = event_mask;

    _client->RegisterFileEvent(this);

  } else {
    FatalError("Attempt to attach file event to a non-existing object");
  }
}

OFileHandler::~OFileHandler() {
  _client->UnregisterFileEvent(this);
}
