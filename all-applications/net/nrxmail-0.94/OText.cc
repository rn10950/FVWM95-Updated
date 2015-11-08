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

#include <X11/Xlib.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <xclass/utils.h>

#include "OText.h"

// Changes:
//
// - OLine now has the notion of capacity (_size), which is used to
//   improve performance by avoiding calling new/delete too often. When
//   deleting chars, the space is reallocated only when the size of the
//   string becomes less than the half of the current capacity, but never
//   less than 8 chars. When adding chars, new is not called on every
//   char, but instead the space is allocated in 20-char chunks.
//
// - OLine's buffer is never allowed to be NULL. For empty lines,
//   it will contain a valid buffer with an empty ("") string.


// TODO:
//
// - process special cases when adding/removing text


//----------------------------------------------------------------------

OLine::OLine() {
  _length = 0;
  _size = 8;
  _string = new char[8];
  _string[0] = '\0';
  _prev = _next = NULL;
}

OLine::OLine(OLine *line) {
  _length = line->GetLineLength();
  if (_length > 0) {
    _size = _length + 1;
    _string = new char[_size];
    strcpy(_string, line->GetString());
  } else {
    _size = 8;
    _string = new char[8];
    _string[0] = '\0';
  }
  _prev = _next = NULL;
}

OLine::OLine(const char *string) {
  if (string) {
    _length = strlen(string);
    _size = _length + 1;
    _string = new char[_size];
    strcpy(_string, string);
  } else {
    _length = 0;
    _size = 8;
    _string = new char[8];
    _string[0] = '\0';
  }
  _prev = _next = NULL;
}

OLine::~OLine() {
  delete[] _string;
}

void OLine::Clear() {
  if (_size > 8) {
    _size = 8;
    delete[] _string;
    _string = new char[8];
  }
  _string[0] = '\0';
  _length = 0;
}

void OLine::DelText(unsigned long pos, unsigned long length) {

  if ((_length == 0) || (pos >= _length) || (pos + length > _length))
    return;

  if (pos + length >= _length) {
    _string[pos] = '\0';
    _length = 0;
    return;
  }

  strcpy(_string + pos, _string + pos + length);
  _length = strlen(_string);
}

void OLine::InsText(unsigned long pos, const char *text) {
  int  n, tlen;
  char *p1, *p2;

  //if (pos < 0) pos = 0;
  if (pos > _length) pos = _length;

  tlen = strlen(text);
  n = _length - pos + 1;  // amount of chars to move, including trailing zero

  if (_length + tlen >= _size) {
    char *str = new char[_size = _length + tlen + 20];
    strcpy(str, _string);
    delete[] _string;
    _string = str;
  }

  p1 = _string + _length;
  p2 = p1 + tlen;

  // make space for new text
  while (n-- > 0) *p2-- = *p1--;

  p1 = _string + pos;
  if (pos + tlen >= _size) tlen = _size - pos - 1;

  while (tlen-- > 0) *p1++ = *text++;

  _string[_size-1] = '\0';
  _length = strlen(_string);
}

// ==!== check for possible mem leaks when using this

char *OLine::GetSubString(unsigned long pos, unsigned long length) {
  char *retstring;

  if ((pos >= _length) || (length > _length) || (pos + length > _length))
    return NULL;

  retstring = new char[length + 1];
  retstring[length] = '\0';
  strncpy(retstring, _string + pos, length);

  return retstring;
}

// this deletes 'backwards' (i.e. previous char) !!!

void OLine::DelChar(unsigned long pos) {
  if (pos > 0) DelText(pos - 1, 1);
}

void OLine::InsChar(unsigned long pos, char character) {

  if (pos > _length) return;

  char tmp[2] = { character, '\0' };
  InsText(pos, tmp);
}

int OLine::GetChar(unsigned long pos) {
  if ((_length <= 0) || (pos >= _length)) return -1;
  return _string[pos];
}


//----------------------------------------------------------------------

OText::OText() {
  _OText();
}

OText::OText(OText *text) {
  OPosition pos(0, 0), end;

  end.y = text->RowCount() - 1;
  end.x = text->GetLineLength(end.y) - 1;
  _OText();
  this->InsText(pos, text, pos, end);
}

OText::OText(const char *string) {
  OPosition pos(0, 0);

  _OText();
  this->InsText(pos, string);
}

OText::~OText() {
  Clear();
  delete first;
}

void OText::_OText() {
  first = new OLine();
  current = first;
  isSaved = True;
  colCount = 0;
  rowCount = 1;
  current_row = 0;
}

void OText::Clear() {
  OLine *travel = first->_next;
  OLine *toDelete;
  while (travel) {
    toDelete = travel;
    travel = travel->_next;
    delete toDelete;
  }
  first->Clear();
  current = first;
  current_row = 0;
  colCount = 0;
  rowCount = 1;
  isSaved = True;
}

bool OText::Load(const char *fn, long startpos, long length) {
  bool isFirst = True;
  bool finished = False;
  long n, count, charcount;
  FILE *fp;
  int  i, cnt;
  char buf[8000], *p;
  OLine *travel, *temp;
  travel = first;

  if ((fp = fopen(fn, "r")) == NULL) return False;
  i = 0;
  fseek(fp, startpos, SEEK_SET);
  charcount = 0;
  while (fgets(buf, 8000, fp)) {
    if ((length != -1) && (charcount + strlen(buf) > length)) {
      count = length - charcount;
      finished = True;
    } else {
      count = 8000;
    }
    charcount += strlen(buf);
    if (buf[strlen(buf)-1] == '\n') buf[strlen(buf)-1] = '\0';
    temp = new OLine(buf);
    if (isFirst) {
      delete first;
      first = temp;
      current = temp;
      travel = first;
      isFirst = False;
    } else {
      travel->_next = temp;
      temp->_prev = travel;
      travel = travel->_next;
    }
    ++i;
    if (finished) break;
  }
  fclose(fp);

  rowCount = i;
  if (rowCount == 0) rowCount++;
  isSaved = True;
  LongestLine();

  return True;
}

bool OText::Save(const char *fn) {
  OLine *travel;
  FILE *fp;

  if ((fp = fopen(fn, "w")) == NULL) return False;

  for (travel = first; travel != NULL; travel = travel->_next) {
    if (fprintf(fp, "%s\n", travel->_string) != travel->_length + 1) {
      fclose(fp);
      return False;
    }
  }
  isSaved = True;
  fclose(fp);

  return True;
}

bool OText::Append(const char *fn) {
  OLine *travel;
  FILE *fp;

  if ((fp = fopen(fn, "a")) == NULL) return False;

  for (travel = first; travel != NULL; travel = travel->_next) {
    if (fprintf(fp, "%s\n", travel->_string) != travel->_length + 1) {
      fclose(fp);
      return False;
    }
  }
  isSaved = True;
  fclose(fp);

  return True;
}

bool OText::DelChar(OPosition pos) {
  if (pos.y >= rowCount) return False;
  SetCurrentRow(pos.y);
  current->DelChar(pos.x);
  isSaved = False;
  LongestLine();
  return True;
}

bool OText::InsChar(OPosition pos, char c) {
  if (pos.y >= rowCount) return False;
  SetCurrentRow(pos.y);
  current->InsChar(pos.x, c);
  isSaved = False;
  LongestLine();
  return True;
}

int OText::GetChar(OPosition pos) {
  if (pos.y >= rowCount) return -1;
  SetCurrentRow(pos.y);
  return current->GetChar(pos.x);
}

bool OText::DelText(OPosition start, OPosition end) {
  char *tempbuffer;

  if ((start.y >= rowCount) || (end.y >= rowCount)) return False;

  SetCurrentRow(start.y);
  if (start.y == end.y) {
    current->DelText(start.x, end.x - start.x + 1);
    return True;
  }
  current->DelText(start.x, current->_length - start.x);
  SetCurrentRow(current_row + 1);

  for (long i = start.y + 1; i < end.y; i++) DelLine(current_row);

  tempbuffer = current->GetSubString(end.x + 1, current->_length - end.x - 1);
  DelLine(current_row);
  SetCurrentRow(start.y);
  if (tempbuffer) {
    current->InsText(current->GetLineLength(), tempbuffer);
    delete[] tempbuffer;
  } else {
    if (current->_next) {
      current->InsText(current->_length, current->_next->_string);
      DelLine(current_row + 1);
      SetCurrentRow(start.y);
    }
  }
  isSaved = False;
  LongestLine();
}

bool OText::InsText(OPosition ins_pos, OText *src, OPosition
		    start_src, OPosition end_src) {
  OPosition pos;
  unsigned long len;
  const char *lineString;
  const char *restString;
  OLine *following;

  if (ins_pos.y > rowCount)	// opsie, wrong coords ??
    return False;

  if (ins_pos.y == rowCount) {	// for appending text
    pos.y = rowCount - 1;
    pos.x = GetLineLength(pos.y);
    BreakLine(pos);		// current row is set by this
  } else {
    SetCurrentRow(ins_pos.y);	// otherwise going to the desired row
  }

  // preparing first line to be inserted
  restString = current->GetSubString(ins_pos.x, current->_length - ins_pos.x);
  current->DelText(ins_pos.x, current->_length - ins_pos.x);
  following = current->_next;

  // inserting first line
  if (start_src.y == end_src.y) {
    len = end_src.x - start_src.x + 1;
  } else {
    len = src->GetLineLength(start_src.y) - start_src.x;
  }
  if (len > 0) {
    lineString = src->GetSubString(start_src, len);
    current->InsText(ins_pos.x, lineString);
    delete[] lineString;
  } else {
    BreakLine(ins_pos);
  }

  // [...] inserting possible lines
  pos.y = start_src.y + 1;
  pos.x = 0;
  for (pos.y; pos.y < end_src.y; pos.y++) {
    lineString = src->GetLine(pos);
    current->_next = new OLine(lineString);
    current->_next->_prev = current;
    current = current->_next;
    rowCount++;
    current_row++;
  }

  // last line of inserted text is as special as first line
  if (start_src.y != end_src.y) {
    pos.y = end_src.y;
    pos.x = 0;
    lineString = src->GetSubString(pos, end_src.x + 1);
    if (lineString) {
      current->_next = new OLine(lineString);
      current->_next->_prev = current;
      current = current->_next;
      rowCount++;
      current_row++;
      delete[] lineString;
    } else {
      pos.y = current_row;
      pos.x = 0;
      BreakLine(pos);
    }
  }

  // ok, now we have to add the rest of the first destination line
  if (restString) {
    current->InsText(current->_length, restString);
    delete[] restString;
  }

  // now re linking the rest of the origin text
  current->_next = following;
  if (current->_next)
    current->_next->_prev = current;

  LongestLine();
  isSaved = False;

  return True;
}

bool OText::AddText(OText *text) {
  OPosition end, start_src, end_src;

  end.y = rowCount;
  end.x = 0;
  start_src.x = start_src.y = 0;
  end_src.y = text->RowCount() - 1;
  end_src.x = text->GetLineLength(end_src.y) - 1;
  isSaved = False;

  return InsText(end, text, start_src, end_src);
}


bool OText::InsLine(unsigned long pos, char *string) {
  OLine *following, *previous, *newline;

  if (pos > rowCount) return False;

  if (pos < rowCount)
    SetCurrentRow(pos);
  else
    SetCurrentRow(rowCount - 1);

  previous = current->_prev;
  newline = new OLine(string);
  newline->_prev = previous;

  if (previous)
    previous->_next = newline;
  else
    first = newline;

  newline->_next = current;
  current->_prev = newline;

  rowCount++;
  current_row++;

  LongestLine();
  isSaved = False;

  return True;
}

bool OText::InsText(OPosition pos, const char *buffer) {

  if (pos.y > rowCount) return False;

  if (pos.y == rowCount) {
    SetCurrentRow(rowCount - 1);
    current->_next = new OLine(buffer);
    current->_next->_prev = current;
    rowCount++;
  } else {
    SetCurrentRow(pos.y);
    current->InsText(pos.x, buffer);
  }
  LongestLine();
  isSaved = False;

  return True;
}

bool OText::DelLine(unsigned long pos) {

  if ((!SetCurrentRow(pos)) || (rowCount == 1)) return False;

  OLine *travel = current;
  if (travel == first) {
    first = first->_next;
    first->_prev == NULL;
  } else {
    travel->_prev->_next = travel->_next;
    if (travel->_next) {
      travel->_next->_prev = travel->_prev;
      current = current->_next;
    } else {
      current = current->_prev;
      current_row--;
    }
  }
  delete travel;
  rowCount--;
  isSaved = False;
  LongestLine();
}

const char *OText::GetLine(OPosition pos) {
  if (pos.y >= rowCount) return NULL;
  SetCurrentRow(pos.y);
  return current->GetString();
}

const char *OText::GetSubString(OPosition pos, unsigned long length) {
  if (pos.y >= rowCount) return NULL;
  SetCurrentRow(pos.y);
  return current->GetSubString(pos.x, length);
}

bool OText::BreakLine(OPosition pos) {
  OLine *temp;
  char *tempbuffer;

  if (SetCurrentRow(pos.y) == False) return False;
  if ((pos.x < 0) || (pos.x > current->_length)) return False;

  if (pos.x < current->_length) {
    tempbuffer = current->GetSubString(pos.x, current->_length - pos.x);
    temp = new OLine(tempbuffer);
    current->DelText(pos.x, current->_length - pos.x);
    delete[] tempbuffer;
  } else {
    temp = new OLine();
  }

  temp->_prev = current;
  temp->_next = current->_next;
  current->_next = temp;
  if (temp->_next) temp->_next->_prev = temp;
  isSaved = False;

  rowCount++;
  current_row++;

  current = current->_next;
  LongestLine();

  return True;
}

long OText::GetLineLength(long row) {
  if (row >= rowCount) return -1;
  SetCurrentRow(row);
  return current->GetLineLength();
}

bool OText::SetCurrentRow(long row) {
  int count;

  if ((row < 0) || (row >= rowCount)) return False;

  if (row > current_row) {
    for (count = current_row; count < row; count++) {
      if (current->_next == NULL) {
	current_row = count;
	return False;
      }
      current = current->_next;
    }
  } else {
    if (current_row == row) return True;
    for (count = current_row; count > row; count--) {
      if (current->_prev == NULL) {
	current_row = count;
	return False;
      }
      current = current->_prev;
    }
  }
  current_row = row;

  return True;
}

bool OText::Search(OPosition *foundPos, OPosition start,
                   char *searchString, bool direction, bool caseSensitive) {

  if (SetCurrentRow(start.y) == False) return False;

  if (direction) {

    while (1) {
      foundPos->x = DownSearchBM(current->_string + start.x,
				 searchString, current->_length - start.x,
				 caseSensitive);
      if (foundPos->x != -1) {
	foundPos->x += start.x;
	foundPos->y = current_row;
	return True;
      }
      if (SetCurrentRow(current_row + 1) == False) break;
      start.x = 0;
    }

  } else {

    while (1) {
      foundPos->x = UpSearchBM(current->_string, searchString, start.x,
			       caseSensitive);
      if (foundPos->x != -1) {
	foundPos->y = current_row;
	return True;
      }
      if (SetCurrentRow(current_row - 1) == False) break;
      start.x = current->_length;
    }

  }

  return False;
}

long OText::DownSearchBM(char *actualLine, char *searchPattern,
                         long len, bool cS) {
  char d[95];
  long i, j, k, m, n;

  m = strlen(searchPattern);
  n = len;

  if ((len <= 0) || (m > n)) return -1;

  for (i = 0; i < 95; i++) d[i] = m;

  for (i = 0; i < m - 1; i++) {
    d[searchPattern[i] - 32] = m - i - 1;
    if (cS == False) {
      if ((searchPattern[i] >= 65) && (searchPattern[i] <= 90))
	d[searchPattern[i]] == m - i - 1;
      else if ((searchPattern[i] >= 97) && (searchPattern[i] <= 122))
	d[searchPattern[i] - 64] = m - i - 1;
    }
  }
  i = m;

  do {
    j = m;
    k = i;
    do {
      k--;
      j--;
    } while ((j >= 0) && CharEqual(searchPattern[j], actualLine[k], cS));
    i = i + d[actualLine[i - 1] - 32];
  } while ((j >= 0) && (i <= n));

  if (j < 0)
    return i - m - d[searchPattern[m - 1] - 32];
  else
    return -1;
}

long OText::UpSearchBM(char *actualLine, char *searchPattern,
                       long len, bool cS) {
  int d[95];
  long i, j, k, m, n;

  m = strlen(searchPattern);
  n = len;

  if ((len <= 0) || (m > n)) return -1;

  for (i = 0; i < 95; i++) d[i] = m;

  for (i = m - 1; i > 0; i--) {
    d[searchPattern[i] - 32] = i;
    if (cS == False) {
      if ((searchPattern[i] >= 65) && (searchPattern[i] <= 90))
	d[searchPattern[i]] = i;
      else if ((searchPattern[i] >= 97) && (searchPattern[i] <= 122))
	d[searchPattern[i] - 64] = i;
    }
  }
  i = n - m - 1;

  do {
    j = -1;
    k = i;
    do {
      k++;
      j++;
    } while ((j < m) && CharEqual(searchPattern[j], actualLine[k], cS));
    i = i - d[actualLine[i + 1] - 32];
  } while ((j < m) && (i >= 0));

  if (j >= m)
    return i + m + d[searchPattern[1] - 32];
  else
    return -1;
}

bool OText::CharEqual(char first, char second, bool cS) {

  if (first == second) return True;

  if (cS == False)
    if (tolower(first) == tolower(second)) return True;

  return False;
}

bool OText::Replace(OPosition pos, char *oldText, char *newText, bool
		    direction, bool caseSensitive) {
  OPosition delEnd;

  if (SetCurrentRow(pos.y) == False) return False;

  if (direction) {
    if (DownSearchBM(current->_string + pos.x, oldText,
                     current->_length - pos.x, caseSensitive) == False)
      return False;
  } else {
    if (UpSearchBM(current->_string + pos.x, oldText,
                   current->_length - pos.x, caseSensitive) == False)
      return False;
    pos.x = pos.x - strlen(oldText) - 1;
  }

  delEnd.y = pos.y;
  delEnd.x = pos.x + strlen(oldText) - 1;
  DelText(pos, delEnd);
  InsText(pos, newText);

  return True;
}

void OText::LongestLine() {
  OLine *travel;
  long line_count = 0;

  colCount = 0;

  for (travel = first; travel != NULL; travel = travel->_next) {
    if (travel->_length > colCount) {
      colCount = travel->_length;
      longestLine = line_count;
    }
    line_count++;
  }
}
