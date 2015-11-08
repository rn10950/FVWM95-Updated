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

#ifndef __OFTPFILEHANDLER_H
#define __OFTPFILEHANDLER_H

#include <xclass/OFileHandler.h>

#include "OFileSystem.h"


//----------------------------------------------------------------------

class OFtpFileHandler : public OFileHandler {
public:
  OFtpFileHandler(OXWindow *w, unsigned int event_mask,
                  OHandle *handle, ODir *ditem) :
    OFileHandler(w, handle->fd, event_mask) {
      _ditem = ditem; _handle = handle;
  }

  OHandle *GetHandle() { return _handle; }
  ODir *GetDir() { return _ditem; }

protected:
  ODir *_ditem;
  OHandle *_handle;
};


#endif  // __OFTPFILEHANDLER.H
