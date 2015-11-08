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

#ifndef __OXSECRETTEXTENTRY_H
#define __OXSECRETTEXTENTRY_H

#include <xclass/OXTextEntry.h>


//----------------------------------------------------------------------

class OXSecretTextEntry : public OXTextEntry {
public:
  OXSecretTextEntry(const OXWindow *p, OTextBuffer *text = NULL, int ID = -1,
              unsigned int option = SUNKEN_FRAME | DOUBLE_BORDER,
              unsigned long back = _defaultDocumentBackground);

protected:
  virtual int  _TextWidth(int begin, int length);
  virtual void _DrawString(GC gc, int x, int y, int begin, int length);
  virtual void _DrawImageString(GC gc, int x, int y, int begin, int length);

  char _password_char_width;
};


#endif  // __OXSECRETTEXTENTRY_H
