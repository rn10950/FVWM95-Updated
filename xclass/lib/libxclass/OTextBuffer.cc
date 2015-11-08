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
#include <xclass/OTextBuffer.h>


//----------------------------------------------------------------------

void OTextBuffer::AddChar(int pos, const char character) {
  char tmpbuffer[2] = { character, '\0' };
  AddText(pos, tmpbuffer);
}


void OTextBuffer::AddText(int pos, const char *text) {
  AddText(pos, text, strlen(text));
}


void OTextBuffer::AddText(int pos, const char *text, int length) {
  int  n, slen, tlen;
  char *p1, *p2;

  slen = strlen(_buffer);

  if (pos < 0) pos = 0;
  if (pos > slen) pos = slen;

  tlen = length; //strlen(text);
  //n = slen - pos + 2;  // amount of chars to move, including trailing zero
  n = slen - pos + 1;  // amount of chars to move, including trailing zero

  p1 = _buffer + slen;
  p2 = p1 + tlen;

  if (slen + tlen >= _bfrlen) {
    if (_incr > 0) {
      _Grow(slen + tlen + 1);

      p1 = _buffer + slen;
      p2 = p1 + tlen;

    } else {
      int ex;

      ex = slen + tlen - _bfrlen + 1;
      p1 -= ex;
      p2 -= ex;
      n -= ex;
    }
  }

  // make space for new text
  while (n-- > 0) *p2-- = *p1--;

  p1 = _buffer + pos;
  if (pos + tlen >= _bfrlen) tlen = _bfrlen - pos - 1;

  while (tlen-- > 0) *p1++ = *text++;

  _buffer[_bfrlen-1] = 0;
}


void OTextBuffer::Append(const char *s) {
  int len = strlen(s);

  int newlen = len+1;
  if (_buffer) newlen += strlen(_buffer);
  if (newlen > _bfrlen) {
    if (_incr > 0) {
      _Grow(newlen);
      strcat(_buffer, s);
    } else if (_bfrlen > strlen(_buffer)) {
      strncat(_buffer, s, _bfrlen - strlen(_buffer));
    }
  } else {
    strcat(_buffer, s);
  }
}

//----------------------------------------------------------------------

void OTextBuffer::RemoveChar(int pos) {
  RemoveText(pos, 1);
}


void OTextBuffer::RemoveText(int pos, int length) {
  int  slen;
  char *p1, *p2;

  slen = strlen(_buffer);

  if (length <= 0) return;
  if ((pos >= _bfrlen) || (pos >= slen)) return;

  if (pos + length >= _bfrlen) {
    _buffer[pos] = 0;
    return;
  }

  p1 = _buffer + pos;
  p2 = p1 + length;

  do { *p1++ = *p2; } while (*p2++); // include trailing zero!
}


//----------------------------------------------------------------------

char OTextBuffer::GetChar(int pos) {
  int slen = strlen(_buffer);
  if (pos > slen)
    return '\0';
  else
    return _buffer[pos]; 
}

void OTextBuffer::_Grow(int newsz) {
  if (newsz > _bfrlen) {
    char *str = new char[_bfrlen = newsz + _incr];
    str[0] = '\0';
    if (_buffer) { strcpy(str, _buffer); delete[] _buffer; }
    _buffer = str;
  }
}
