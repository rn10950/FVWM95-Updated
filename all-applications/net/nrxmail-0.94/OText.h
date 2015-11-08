/**************************************************************************

    This file is part of NRX mail, a mail client using the XClass95 toolkit.
    Copyright (C) 1998 by Harald Radke.                 

    This application is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This application is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

**************************************************************************/

#ifndef __OTEXT_H
#define __OTEXT_H

#include <X11/Xlib.h>

#include <xclass/ODimension.h>

class OText;


//----------------------------------------------------------------------

class OLine {
public:
  OLine();
  OLine(OLine *line);
  OLine(const char *string);
  ~OLine();

  void Clear();
  unsigned long GetLineLength() const { return _length; }
  void DelText(unsigned long pos, unsigned long length);
  void InsText(unsigned long pos, const char *text);
  char *GetSubString(unsigned long pos, unsigned long length);
  const char *GetString() const { return _string; }

  void DelChar(unsigned long pos);
  void InsChar(unsigned long pos, char character);
  int  GetChar(unsigned long pos);

  friend class OText;

protected:
  char *_string;
  unsigned long _length, _size;
  OLine *_prev, *_next;
};


//----------------------------------------------------------------------

class OText {
public:
  OText();
  OText(OText *text);
  OText(const char *string);
  ~OText();

  void Clear();
  bool Load(const char *fn, long startpos = 0, long length = -1);
  bool Save(const char *fn);
  bool Append(const char *fn);
  bool IsSaved() const { return isSaved; }
  bool DelChar(OPosition pos);
  bool InsChar(OPosition pos, char c);
  int  GetChar(OPosition pos);

  bool DelText(OPosition start, OPosition end);
  bool InsText(OPosition pos, const char *buf);
  bool InsText(OPosition ins_pos, OText *src,
               OPosition start_src, OPosition end_src);
  bool AddText(OText *text);

  bool DelLine(unsigned long pos);
  const char *GetLine(OPosition pos);
  const char *GetSubString(OPosition pos, unsigned long length);
  bool BreakLine(OPosition pos);
  bool InsLine(unsigned long row, char *string);

  long RowCount() const { return rowCount; }
  long ColCount() const { return colCount; }
  long GetLineLength(long row);
  long GetLongestLine() const { return longestLine; }

  bool Search(OPosition *foundPos, OPosition star,
              char *searchString, bool direction, bool caseSensitive);
  bool Replace(OPosition pos, char *oldText, char *newText,
               bool direction, bool caseSensitive);

protected:
  void _OText();
  bool SetCurrentRow(long row);
  void LongestLine();
  long UpSearchBM(char *actualLine, char *searchPattern, long len, bool cS);
  long DownSearchBM(char *actualLine, char *searchPattern, long len, bool cS);
  bool CharEqual(char first, char second, bool cS);

  bool isSaved;
  OLine *first, *current;
  long current_row, rowCount, colCount, longestLine;
};


#endif  // __OTEXT_H
