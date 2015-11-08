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

#include <string.h>
#include <stdlib.h>

#include <vector>

#include "OFreqRecord.h"
#include "ORX320.h"


/*

                Clifton Turner's *.320 Frequency File Format
                --------------------------------------------

The record structure is as follows: 

"80m PSK-31",3.533000,USB,3300,3,100,0,"","","0000","2400","Notes",0,0

1. Name            String, up to 20 char in length, enclosed in quotes 

2. Frequency       Numeric, real. Frequency in MHz. (You don't need to add
                   trailing zeros. ScanR8 and Scan320 do, RX320 doesn't.
                   Both styles work just fine in any of these programs.)

3. Mode            AM, USB, LSB, or CW. This field is not enclosed in
                   quotes. An entry in this field is required.

4. Filter          Numeric, integer. Filter bandwidth in Hz 

5. AGC             Numeric, integer. 1 = slow, 2 = medium, 3 = fast 

6. Tuning step     Numeric, integer. Tuning step in Hz 

7. PBT offset.     Numeric, integer. Passband tuning value in Hz 

8. Location        String, up to 15 char in length, enclosed in quotes 

9. Language        String, up to 10 char in length, enclosed in quotes 

10. Start time     String: 4 digits, enclosed in quotes. This, and the end
                   time field, may be left blank (just the quotes) but, if
                   you include it, the format must be a 4-digit integer
                   between 0000 and 2400.

11. End time       Same as above 

12. Notes          String, up to 64 char in length, enclosed in quotes. 

13. Offset         Numeric, integer. Unused in Scan320. 

14. Lockout        Numeric, integer. ScanR8 writes a non-zero value to this
                   field to indicate that the record is locked out during
                   file scanning.

You may leave any of the string fields blank. If you do, however, make sure
that you leave the empty quotes.


               Ten-Tec *.cdf (exported) Frequency File Format
               ----------------------------------------------

The record structure is as follows: 

BBC World Service, 9.915000,AM,6 KHz,UK,English

1. Name            String, up to ?? chars in length.

2. Frequency       Numeric, real. Frequency in MHz (%9.6f format, may
                   contain leading spaces).

3. Mode            String: AM, USB, LSB, or CW.

4. Filter          Numeric, integer or float. Filter bandwidth in Hz,
                   if integer (unless followed by "kHz"); KHz if float.

5. Location        String, up to ?? char in length.

6. Language        String, up to ?? char in length.

Strings may be enclosed in quotes. Some of the string fields may be left
blank.

*/


//----------------------------------------------------------------------

OFreqRecord::OFreqRecord(char *str, int fmt) {
  char *s;

  _p = str;

  if (fmt == FORMAT_CLIFTON_320) {

    strcpy(name, GetQuotedString(20));

    freq = GetDouble();

    s = GetString();
    if (strcasecmp(s, "AM") == 0)
      mode = RX320_AM;
    else if (strcasecmp(s, "USB") == 0)
      mode = RX320_USB;
    else if (strcasecmp(s, "LSB") == 0)
      mode = RX320_LSB;
    else if (strcasecmp(s, "CW") == 0)
      mode = RX320_CW;
    else
      mode = RX320_AM;

    filter_bw = GetInteger();
    agc = GetInteger();
    tuning_step = GetInteger();
    pbt_offset = GetInteger();

    strcpy(location, GetQuotedString(15));
    strcpy(language, GetQuotedString(10));

    strcpy(start_time, GetQuotedString(4));
    strcpy(end_time, GetQuotedString(4));

    strcpy(notes, GetQuotedString(64));

    offset = GetInteger();
    lockout = GetInteger();

  } else if (fmt == FORMAT_TENTEC_CDF) {

    strcpy(name, *_p == '"' ? GetQuotedString(20) : GetString(20));

    freq = GetDouble();

    s = GetString();
    if (strcasecmp(s, "AM") == 0)
      mode = RX320_AM;
    else if (strcasecmp(s, "USB") == 0)
      mode = RX320_USB;
    else if (strcasecmp(s, "LSB") == 0)
      mode = RX320_LSB;
    else if (strcasecmp(s, "CW") == 0)
      mode = RX320_CW;
    else
      mode = RX320_AM;

#if 1
    double bw = GetDouble();
    filter_bw = (bw < 10.0) ? (int) (bw * 1000.0 + 0.5) : (int) (bw + 0.5);
#else
    filter_bw = GetInteger();
#endif

    agc = RX320_AGC_MEDIUM;
    tuning_step = 1000;
    pbt_offset = 0;

    strcpy(location, GetString(15));
    strcpy(language, GetString(10));

    strcpy(start_time, "0000");
    strcpy(end_time, "0000");

    strcpy(notes, "");

    offset = 0;
    lockout = 0;

  } else {

    // undefined format

    strcpy(name, "");
    freq = 0.030000;
    mode = RX320_AM;
    filter_bw = 8000;
    agc = RX320_AGC_MEDIUM;
    tuning_step = 1000;
    pbt_offset = 0;
    strcpy(location, "");
    strcpy(language, "");
    strcpy(start_time, "0000");
    strcpy(end_time, "0000");
    strcpy(notes, "");
    offset = 0;
    lockout = 0;

  }
}

OFreqRecord::OFreqRecord(OFreqRecord *frec) {

  strcpy(name, frec->name);
  freq = frec->freq;;
  mode = frec->mode;
  filter_bw = frec->filter_bw;
  agc = frec->agc;
  tuning_step = frec->tuning_step;
  pbt_offset = frec->pbt_offset;
  strcpy(location, frec->location);
  strcpy(language, frec->language);
  strcpy(start_time, frec->start_time);
  strcpy(end_time, frec->end_time);
  strcpy(notes, frec->notes);
  offset = frec->offset;
  lockout = frec->lockout;

}

int OFreqRecord::GetInteger() {
  if (_p) {
    int i = atoi(_p);
    _p = strchr(_p, ',');
    if (_p) ++_p;
    return i;
  } else {
    return 0;
  }
}

double OFreqRecord::GetDouble() {
  if (_p) {
    double d = atof(_p);
    _p = strchr(_p, ',');
    if (_p) ++_p;
    return d;
  } else {
    return 0.0;
  }
}

char *OFreqRecord::GetString(int maxlen) {
  static char tmp[256];

  if (maxlen < 0 || maxlen > 255) maxlen = 255;

  if (_p) {
    char *s = strchr(_p, ',');
    maxlen = min(maxlen, s ? s - _p : strlen(_p));
    strncpy(tmp, _p, maxlen);
    tmp[maxlen] = '\0';
    _p = s;
    if (_p) ++_p;
  } else {
    tmp[0] = '\0';
  }

  return tmp;
}

char *OFreqRecord::GetQuotedString(int maxlen) {
  static char tmp[256];

  if (maxlen < 0 || maxlen > 255) maxlen = 255;

  if (_p) {
    while (*_p && *_p != '"') ++_p;
    if (*_p) ++_p;
    char *s = _p;
    while (*_p && *_p != '"') ++_p;

    maxlen = min(maxlen, _p - s);
    strncpy(tmp, s, maxlen);
    tmp[maxlen] = '\0';

    _p = strchr(_p, ',');
    if (_p) ++_p;
  } else {
    tmp[0] = '\0';
  }

  return tmp;
}

char *OFreqRecord::RecordString(char *dst, int fmt) {
  static char tmp[256];
  char *cmode;

  if (!dst) dst = tmp;

  switch (mode) {
    default:
    case RX320_AM:  cmode = "AM";  break;
    case RX320_LSB: cmode = "LSB"; break;
    case RX320_USB: cmode = "USB"; break;
    case RX320_CW:  cmode = "CW";  break;
  }

  if (fmt == FORMAT_TENTEC_CDF) {
    sprintf(dst, "%s,%9.6f,%s,%d,%s,%s",
            name, freq, cmode, filter_bw, location, language);
  } else {
    // default to *.320 format
    sprintf(dst, "\"%s\",%g,%s,%d,%d,%d,%d,\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",%d,%d",
            name, freq, cmode, filter_bw, agc, tuning_step, pbt_offset,
            location, language, start_time, end_time, notes, offset, lockout);
  }

  return dst;
}
