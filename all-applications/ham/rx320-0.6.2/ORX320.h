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

#ifndef __ORX320_H
#define __ORX320_H

#include <vector>

#include <xclass/OComponent.h>
#include <xclass/OFileHandler.h>
#include <xclass/OTimer.h>
#include <xclass/OMessage.h>


#define RX320_USB         1
#define RX320_LSB         2
#define RX320_CW          3
#define RX320_AM          4

#define RX320_SPEAKER     1
#define RX320_LINE        2
#define RX320_BOTH        3

#define RX320_AGC_SLOW    1
#define RX320_AGC_MEDIUM  2
#define RX320_AGC_FAST    3


#define MSG_RX320         (MSG_USERMSG + 320)
#define MSG_SIGNAL             1
#define MSG_POWERON            2
#define MSG_VERSION            3
#define MSG_ERROR              4


struct SFilter {
  int bandwidth;
  int filter;
};


//----------------------------------------------------------------------

class ORX320 : public OComponent {
public:
  ORX320(OXClient *c, const char *dev = NULL);
  virtual ~ORX320();

  virtual int HandleFileEvent(OFileHandler *f, unsigned int evmask);
  virtual int HandleTimer(OTimer *t);

public:
  int SetSerial(const char *dev);

  int Reset();
  
  int Mute(int onoff);

  int SetMode(int mode);
  int GetMode() const { return _mode; }
  
  int SetFilter(int filt);
  SFilter *GetFilter() const { return _filter; }
  
  int SetVolume(int output, int vol);
  int GetVolume(int output) const;
  
  int SetAGC(int agc);
  int GetAGC() const { return _agc; }
  
  int SetCWO(int cwo);
  int GetCWO() const { return _cwo; }
  
  int SetPBT(int pbt);
  int GetPBT() const { return _pbt; }
  
  int SetFrequency(long freq);  // in Hz
  long GetFrequency() const { return _freq; }
  
  int GetSignal();
  void RequestSignal(int rate);
  char *GetFirmwareVersion();

  void CompressEvents(int onoff);
  
  const char *GetLastError() const { return _errmsg; }
  
protected:
  int OpenSerial(const char *dev);
  int CloseSerial();
  int SendCommand(char *cmd, int len, int now = False);
  int GetResponse(char *buf, int n, char term);

  int _fd, _ix, _count, _muted, _compress;
  int _mode, _cwo, _pbt, _agc, _spkvol, _rate, _linevol;
  long _freq;
  SFilter *_filter;
  int Mcor;
  OFileHandler *_fh;
  char _inbuf[20], _errmsg[1024];
  OTimer *_tsignal, *_tqueue;

  struct _qelem { char *cmd; int len; };
  std::vector<_qelem> _queue;
};


#endif  // __ORX320_H
