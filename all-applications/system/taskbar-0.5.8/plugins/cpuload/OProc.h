/***************************************************************************

   Based on kcpuload's proc.h (C) 1999 by Markus Gustavsson, GPL.

 ***************************************************************************/

#ifndef __OPROC_H
#define __OPROC_H

class OProc {
public:
  OProc();
  ~OProc();

  void ReadLoad(int *ld, int *ld0, int *ld1);
  int  CheckSMP() const { return _smp; }

private:
  int _smp;

  int UT,  ST,  NT,  IT;
  int UT0, ST0, NT0, IT0;
  int UT1, ST1, NT1, IT1;
};

#endif  // __OPROC_H
