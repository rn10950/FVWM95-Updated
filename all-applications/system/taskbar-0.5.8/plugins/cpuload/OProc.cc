/***************************************************************************

   Based on kcpuload's proc.cpp (C) 1999 by Markus Gustavsson, GPL.

 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/Xlib.h>

#include "OProc.h"

OProc::OProc() {
  FILE *f;

  UT  = ST  = NT  = IT  = 0;
  UT0 = ST0 = NT0 = IT0 = 0;
  UT1 = ST1 = NT1 = IT1 = 0;

  f = fopen("/proc/stat", "r");
  if (f) {
    int  cUT, cNT, cST, cIT;
    char tag[33];

    fscanf(f, "%32s %d %d %d %d", tag, &cUT, &cNT, &cST, &cIT);
    fscanf(f, "%32s %d %d %d %d", tag, &cUT, &cNT, &cST, &cIT);
    fclose(f);
    _smp = (strcmp(tag, "cpu0") == 0);
  } else {
    _smp = False;
  }
}

void OProc::ReadLoad(int *ld, int *ld0, int *ld1) {
  FILE *f;
  char tag[33];
  int  cUT,  cST,  cNT,  cIT;
  int  cUT1, cST1, cNT1, cIT1;
  int  cUT0, cST0, cNT0, cIT0;
  int  tot;

  *ld = *ld0 = *ld1 = 0;

  f = fopen("/proc/stat", "r");
  if (!f) return;

  if (_smp) {

    fscanf(f, "%32s %d %d %d %d", tag, &cUT,  &cNT,  &cST,  &cIT);
    fscanf(f, "%32s %d %d %d %d", tag, &cUT0, &cNT0, &cST0, &cIT0);
    fscanf(f, "%32s %d %d %d %d", tag, &cUT1, &cNT1, &cST1, &cIT1);

    tot = (cUT - UT) + (cST - ST) + (cNT - NT) + (cIT - IT);

    if (tot > 0) 
      *ld = ((100 * (cUT - UT)) / tot) 
          + ((100 * (cST - ST)) / tot)
          + ((100 * (cNT - NT)) / tot);
    else
      *ld = 0;

    UT = cUT;
    ST = cST;
    NT = cNT;
    IT = cIT;

    tot = (cUT0 - UT0) + (cST0 - ST0) + (cNT0 - NT0) + (cIT0 - IT0);

    if (tot > 0)
      *ld0 = ((100 * (cUT0 - UT0)) / tot)
           + ((100 * (cST0 - ST0)) / tot)
           + ((100 * (cNT0 - NT0)) / tot);
    else
      *ld0 = 0;

    UT0 = cUT0;
    ST0 = cST0;
    NT0 = cNT0;
    IT0 = cIT0;

    tot = (cUT1 - UT1) + (cST1 - ST1) + (cNT1 - NT1) + (cIT1 - IT1);

    if (tot > 0)
      *ld1 = ((100 * (cUT1 - UT1)) / tot)
           + ((100 * (cST1 - ST1)) / tot)
           + ((100 * (cNT1 - NT1)) / tot);
    else
      *ld1 = 0;

    UT1 = cUT1;
    ST1 = cST1;
    NT1 = cNT1;
    IT1 = cIT1;

  } else {

    fscanf(f, "%32s %d %d %d %d", tag, &cUT, &cNT, &cST, &cIT);

    tot = (cUT - UT) + (cST - ST) + (cNT - NT) + (cIT - IT);

    if (tot > 0)
      *ld = ((100 * (cUT - UT)) / tot)
          + ((100 * (cST - ST)) / tot)
          + ((100 * (cNT - NT)) / tot);
    else
      *ld = 0;

    UT = cUT;
    ST = cST;
    NT = cNT;
    IT = cIT;
  }

  fclose(f);
}
