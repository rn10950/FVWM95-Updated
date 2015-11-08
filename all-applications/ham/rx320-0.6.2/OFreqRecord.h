/**************************************************************************

    This file is part of rx320, a control program for the Ten-Tec RX320
    receiver. Copyright (C) 2000-2004, Hector Peraza.

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

#ifndef __OFREQRECORD_H
#define __OFREQRECORD_H

#include <xclass/OBaseObject.h>


#define FORMAT_CLIFTON_320  1
#define FORMAT_TENTEC_CDF   2



//----------------------------------------------------------------------

class OFreqRecord : public OBaseObject {
public:
  OFreqRecord(OFreqRecord *frec);
  OFreqRecord(char *str, int fmt = FORMAT_CLIFTON_320);

  char *RecordString(char *dst = NULL, int fmt = FORMAT_CLIFTON_320);

public:
  char name[21];
  double freq;
  int  mode, filter_bw, agc, tuning_step, pbt_offset;
  char location[16];
  char language[11];
  char start_time[5], end_time[5];
  char notes[65];
  int  offset, lockout;
  
protected:
  int GetInteger();
  double GetDouble();
  char *GetString(int maxlen = -1);
  char *GetQuotedString(int maxlen = -1);

  char *_p;
};


#endif  // __OFREQRECORD_H
