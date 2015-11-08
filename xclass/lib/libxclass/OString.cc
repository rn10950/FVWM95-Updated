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
#include <xclass/OString.h>


//----------------------------------------------------------------------
       
void OString::Append(const char *s, int len) {
  if (len < 0) len = strlen(s);
  char *str = new char[(_len+=len)+1];
  if (_string) {
    strcpy(str, _string);
    strncat(str, s, len);
    delete[] _string;
  } else {
    strncpy(str, s, len);
  }
  str[_len] = '\0';
  _string = str;
}

void OString::Append(const OString *s) {
  char *str = new char[(_len+=s->GetLength())+1];
  if (_string) {
    strcpy(str, _string);
    strcat(str, s->GetString());
    delete[] _string;
  } else {
    strcpy(str, s->GetString());
  }
  _string = str;
}

void OString::Prepend(const char *s) {
  char *str = new char[(_len+=strlen(s))+1];
  strcpy(str, s);
  if (_string) {
    strcat(str, _string);
    delete[] _string;
  }
  _string = str;
}

void OString::Prepend(const OString *s) {
  char *str = new char[(_len+=s->GetLength())+1];
  strcpy(str, s->GetString());
  if (_string) {
    strcat(str, _string);
    delete[] _string;
  }
  _string = str;
}

void OString::Draw(Display *dpy, XID id, GC gc, int x, int y, int len) {
  XDrawString(dpy, id, gc, x, y, _string, len < 0 ? _len : len);
}


//-----------------------------------------------------------------  
     
OHotString::OHotString(const char *s) : OString(s) {
  char *p;

  _lastGC = None;
  _off1 = _off2 = 0;

  _hotchar = '\0';
  _hotpos = -1;    // No hotkey defaults the offset to -1

  for (p=_string; *p; ++p) {
    if (*p == '&') {
      if (p[1] == '&') { // escaped & ?
        // copy the string down over it   
        for (char *tmp=p; *tmp; tmp++) tmp[0] = tmp[1];
        continue; // and skip to the key char
      }
      // hot key marker - calculate the offset value 
      _hotpos = (p - _string);
      _hotchar = p[1];
      for (; *p; p++) p[0] = p[1];  // copy down
      break;                        // allow only one hotkey per item
    }
  }
  _len = strlen(_string);  // recompute length
}

void OHotString::Draw(Display *dpy, XID id, GC gc, int x, int y, int len) {

  XDrawString(dpy, id, gc, x, y, _string, len < 0 ? _len : len);

  if ((_hotpos >= 0) && (_hotpos < (len < 0) ? _len : len)) {
    if (_lastGC != gc) {
      XGCValues gcval;
      XFontStruct *font;

      XGetGCValues(dpy, gc, GCFont, &gcval);
      font = XQueryFont(dpy, gcval.font);

      _off1 = XTextWidth(font, _string, _hotpos);
      _off2 = XTextWidth(font, _string, _hotpos+1) - 1;
      _lastGC = gc;
      XFreeFontInfo(NULL, font, 1);
    }
    XDrawLine(dpy, id, gc, x+_off1, y+1, x+_off2, y+1);
  }
}
