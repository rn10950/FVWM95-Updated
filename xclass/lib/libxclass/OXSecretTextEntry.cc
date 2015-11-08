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

#include <xclass/utils.h>
#include <xclass/OXClient.h>
#include <xclass/OXFont.h>
#include <xclass/OXSecretTextEntry.h>
#include <xclass/OString.h>

#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>


#define PASSWORD_CHAR "*"

//-----------------------------------------------------------------

OXSecretTextEntry::OXSecretTextEntry(const OXWindow *p, OTextBuffer *text,
                       int ID, unsigned int options, unsigned long back)
  : OXTextEntry(p, text, ID, options, back ) {
  int tw;

  _widgetType = "OXSecretTextEntry";

  _password_char_width = _font->TextWidth(PASSWORD_CHAR, 1);
  tw = _TextWidth(0, _text->GetTextLength());
  Resize(tw + 8, _th + 7);
}

int OXSecretTextEntry::_TextWidth(int begin, int length) {
  return (length * _password_char_width);
}

void OXSecretTextEntry::_DrawString(GC gc, int x, int y,
                                    int begin, int length) {
  char tmptext[length + 1];

  for (int i = 0; i < length; i++) tmptext[i] = *PASSWORD_CHAR;
  tmptext[length] = 0;
  DrawString(gc, x, y, tmptext, length);
}

void OXSecretTextEntry::_DrawImageString(GC gc, int x, int y,
                                         int begin, int length) {
  char tmptext[length + 1];

  for (int i = 0; i < length; i++) tmptext[i] = *PASSWORD_CHAR;
  tmptext[length] = 0;
  DrawImageString(gc, x, y, tmptext, length);
}
