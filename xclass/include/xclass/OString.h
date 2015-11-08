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

#ifndef __OSTRING_H
#define __OSTRING_H

#include <stdlib.h>
#include <string.h>

#include <X11/Xlib.h>

#include <xclass/utils.h>
#include <xclass/OBaseObject.h>
 
#undef NULL
#define NULL 0


//----------------------------------------------------------------------

class OString : public OBaseObject {
public:
  OString() { _len = 0; _string = NULL; }
  OString(const char *s) {
    _string = new char[(_len=strlen(s))+1];
    strcpy(_string, s);
  }
  OString(const OString *s) {
    _string = new char[(_len=s->GetLength())+1];
    strcpy(_string, s->GetString());
  }
  virtual ~OString() {
    if (_string) delete[] _string;
  }

  int  GetLength() const { return _len; }
  void SetLength(int len) { _len = len; } // use with care!
  void ResetLength() { if (_string) _len=strlen(_string); else _len=0; }
  const char *GetString() const { return _string; }

  void Clear() { _len=0; if (_string) *_string='\0'; }

  void Append(const char *s, int len = -1);
  void Append(const OString *s);

  void Prepend(const char *s);
  void Prepend(const OString *s);

  virtual int GetHotChar() const { return -1; }
  virtual int GetHotIndex() const { return -1; }

  virtual void Draw(Display *dpy, XID id, GC gc, int x, int y, int len = -1);

protected:
  char *_string;
  int _len;
};

class OHotString : public OString {
public:
  OHotString(const char *s);

  virtual int GetHotChar() const { return _hotchar; }
  virtual int GetHotIndex() const { return _hotpos; }
  virtual void Draw(Display *dpy, XID id, GC gc, int x, int y, int len = -1);

protected:
  char _hotchar;
  int _hotpos;
  // some _off1, _off2 caching because OHotString::Draw is slow, 
  // specially noticeable in slow machines/connections.
  GC  _lastGC;
  int _off1, _off2;
};

#endif  // __OSTRING_H
