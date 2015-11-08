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

#ifndef __OMIMEATT_H
#define __OMIMEATT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "OMail.h"

#define ASCII	1
#define BIT7	2
#define BIT8	3
#define BASE64  4
#define QUOTED_PRINTABLE  5


//----------------------------------------------------------------------

class OMimeAtt {
public:
  OMimeAtt(FILE **f, char *mailfile, char *boundary, long border);
  ~OMimeAtt();

  char *GetMailFile() const { return _mailfile; }
  char *GetName() const { return _name; }
  char *GetType() const { return _type; }
  int  GetEncoding() const { return _encoding; }
  SParm *GetParamters() const { return _parameters; }
  bool IsMultipart() const { return _isMultipart; }
  long GetStart() const { return _start; }
  long GetContentsStart() const { return _contStart; }
  long GetEnd() const { return _end; }
  OMimeAtt *GetNext() const { return _next; }

  friend class OMail;

  bool IsKnown() const { return _isKnown; }
  void SetKnown() { _isKnown = True; }
  bool Save(char *filename);

protected:
  bool DecodeBase64(FILE *g);
  bool DecodeQuotedPrintable(FILE *g);
  bool Decode7Bit(FILE *g);
  bool Decode8Bit(FILE *g);
  void SetParameters(const char *line);

  bool _isKnown, _isMultipart;
  long _start, _end, _contStart;
  SParm *_parameters;
  char *_name, *_type, *_mailfile;
  int  _encoding;

  OMimeAtt *_next;
};


#endif  // __OMIMEATT_H
