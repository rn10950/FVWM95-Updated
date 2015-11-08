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

#ifndef __OTEXTBUFFER_H
#define __OTEXTBUFFER_H

#include <stdlib.h>
#include <string.h>

#include <X11/Xlib.h>

#include <xclass/utils.h>
#include <xclass/OBaseObject.h>
#include <xclass/OString.h>

#undef NULL
#define NULL 0


//----------------------------------------------------------------------

class OTextBuffer : public OBaseObject {
public:
  OTextBuffer()
    { _buffer = NULL; _bfrlen = 0; _incr = 0; }
  OTextBuffer(unsigned length, int incr = 0)
    { _buffer = new char[_bfrlen = length]; *_buffer = '\0'; _incr = incr; }
  ~OTextBuffer()
    { if (_buffer) delete[] _buffer; }

  unsigned GetTextLength() const { return strlen(_buffer); }
  unsigned GetBufferLength() const { return _bfrlen; }

  const char *GetString() const { return _buffer; }
  OString *GetOString() const { return new OString(_buffer); }
  char GetChar(int pos);

  void AddText(int pos, const char *text);
  void AddText(int pos, const char *text, int length);
  void AddChar(int pos, const char character);

  void RemoveText(int pos, int length);
  void RemoveChar(int pos);

  void Clear() { if (_buffer) *_buffer = '\0'; }

  void Append(const char *s);

  void SetIncrement(int i = 0) { _incr = i; }

protected:
  void _Grow(int newsz);

  char *_buffer;
  unsigned _bfrlen;
  int _incr;
};


#endif  // __OTEXTBUFFER_H
