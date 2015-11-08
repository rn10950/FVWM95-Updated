/**************************************************************************

    This file is part of rx320, a control program for the Ten-Tec RX320
    receiver. Copyright (C) 2000-2004, Hector Peraza. Portions
    Copyright (C) 2000, A. Maitland Bottoms.

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

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <math.h>
#include <time.h>

#include "ORX320.h"


SFilter Filters[] = {
  { 8000, 33 }, { 6000,  0 }, { 5700,  1 }, { 5400,  2 },
  { 5100,  3 }, { 4800,  4 }, { 4500,  5 }, { 4200,  6 },
  { 3900,  7 }, { 3600,  8 }, { 3300,  9 }, { 3000, 10 },
  { 2850, 11 }, { 2700, 12 }, { 2550, 13 }, { 2400, 14 },
  { 2250, 15 }, { 2100, 16 }, { 1950, 17 }, { 1800, 18 },
  { 1650, 19 }, { 1500, 20 }, { 1350, 21 }, { 1200, 22 },
  { 1050, 23 }, {  900, 24 }, {  750, 25 }, {  675, 26 },
  {  600, 27 }, {  525, 28 }, {  450, 29 }, {  375, 30 },
  {  330, 31 }, {  300, 32 }, {    0, 32 }
};


//----------------------------------------------------------------------

ORX320::ORX320(OXClient *c, const char *dev) : OComponent() {
  _client = c;
  _msgObject = NULL;

  Mcor = 0;

  _fd = -1;
  _fh = NULL;
  _ix = 0;
  _count = 0;

  _compress = False;
  _queue.clear();
  _tqueue = NULL;

  _tsignal = NULL;
  _rate = 0;

  _muted = True;
  _mode = RX320_AM;
  _spkvol = _linevol = 32;
  _filter = &Filters[0];
  _agc = RX320_AGC_MEDIUM;
  _freq = 930000;  // Hz
  _pbt = 0;
  _cwo = 1000;

  strcpy(_errmsg, "");

  if (dev) {
    OpenSerial(dev);
    Mute(True);
    Reset();
  }
}

ORX320::~ORX320() {
  CloseSerial();
  if (_tsignal) delete _tsignal;
  if (_tqueue) delete _tqueue;
  for (int i = 0; i < _queue.size(); ++i) delete[] _queue[i].cmd;
  _queue.clear();
}

//----------------------------------------------------------------------

int ORX320::SetSerial(const char *dev) {
  int retc;

  CloseSerial();
  if (dev) {
    retc = OpenSerial(dev);
    if (retc) {
      Mute(True);
      Reset();
    }
  }

  return retc;
}

int ORX320::HandleTimer(OTimer *t) {
  if (t == _tsignal) {
    char scmd[2];
    scmd[0] = 'X';
    scmd[1] = 0x0D;
    SendCommand(scmd, 2);
    delete _tsignal;
    _tsignal = new OTimer(this, _rate);
    return True;
  } else if (t == _tqueue) {
    delete _tqueue;
    _tqueue = NULL;
    if (_queue.size() > 0) {
      _tqueue = new OTimer(this, _queue[0].len * 10);
      if (_fd >= 0) write(_fd, _queue[0].cmd, _queue[0].len);
      delete[] _queue[0].cmd;
      _queue.erase(_queue.begin());
    }
  }
  return False;
}

int ORX320::HandleFileEvent(OFileHandler *f, unsigned int evmask) {

  if (evmask & XCM_READABLE) {
    char c;

    read(_fd, &c, 1);

    if (_count == 0) {
      _ix = 1;
      if (c == 'X') {
        _count = 3;
        _inbuf[0] = c;
      } else if (c == 'Z') {
        _count = 1;
        _inbuf[0] = c;
      } else if (c == 'V') {
        _count = 6;
        _inbuf[0] = c;
      } else if (c == 'D') {
        _count = 10;
        _inbuf[0] = c;
      } else if (c == ' ') {
        _count = 0;
      } else {
        //must be some RS232 noise... (e.g. unplugging the connector)
        //sprintf(_errmsg, "ORX320: unexpected response %02x (%c)\n", (unsigned) c, c);
      }
    } else {
      _inbuf[_ix++] = c;
      if (--_count == 0) {
        c = _inbuf[0];
        if (c == 'X') {
          unsigned int val, lo, hi;
          hi = _inbuf[1]; hi &= 0xFF;
          lo = _inbuf[2]; lo &= 0xFF;
          val = (hi << 8) | lo;
          val &= 0xFFFF;
          OWidgetMessage msg(MSG_RX320, MSG_SIGNAL, val);
          SendMessage(_msgObject, &msg);
        } else if (c == 'Z') {
          OWidgetMessage msg(MSG_RX320, MSG_ERROR);
          SendMessage(_msgObject, &msg);
        } else if (strncmp(_inbuf, "VER", 3) == 0) {
          OWidgetMessage msg(MSG_RX320, MSG_VERSION, 103);
          SendMessage(_msgObject, &msg);
        } else if (strncmp(_inbuf, "DSP START", 9) == 0) {
          Reset();
          OWidgetMessage msg(MSG_RX320, MSG_POWERON, 1);
          SendMessage(_msgObject, &msg);
        }
      }
    }

  }

  return True;
}

int ORX320::Reset() {
  SetMode(_mode);
  SetFilter(_filter->filter);
  SetVolume(RX320_SPEAKER, _spkvol);
  SetVolume(RX320_LINE, _linevol);
  SetFrequency(_freq);
  return 0;
}

int ORX320::Mute(int onoff) {
  _muted = onoff;
  SetVolume(RX320_SPEAKER, _spkvol);
  SetVolume(RX320_LINE, _linevol);
  return 0;
}


// MODE: AM USB LSB CW

int ORX320::SetMode(int mode) {
  char mcmd[3];

  _mode = mode;

  mcmd[0] = 'M';
  mcmd[2] = 0x0D;

  switch (mode) {
    case RX320_USB:
      Mcor = 1;
      mcmd[1] = '1';
      break;

    case RX320_LSB:
      Mcor = -1;
      mcmd[1] = '2';
      break;

    case RX320_CW:
      Mcor = -1;
      mcmd[1] = '3';
      break;

    default:
    case RX320_AM:
      Mcor = 0;
      mcmd[1] = '0';
      break;
  }

  SendCommand(mcmd, 3);
  return SetFrequency(_freq);
}

// FILTER: (34 of them)

int ORX320::SetFilter(int filt) {
  char fcmd[3];
  SFilter *fe;

  if (filt > 34) {
    // Allow selection by Hz as well as number
    for (fe = Filters; fe->bandwidth > 0; fe++)
      if (filt >= fe->bandwidth) break;
    filt = fe->filter;
  }
  fcmd[0] = 'W';
  fcmd[1] = filt;
  fcmd[2] = 0x0D;

  for (fe = Filters; filt != fe->filter; fe++);
  _filter = fe;

  SendCommand(fcmd, 3);
  return SetFrequency(_freq);
}


// VOLUME: (speaker, line-out, both) 0-63
// NOTE: actually mutes the receiver if _muted is True

int ORX320::SetVolume(int output, int vol) {
  char vcmd[4];

  switch (output) {
    case RX320_SPEAKER:
      _spkvol = vol;
      vcmd[0] = 'V';
      break;

    case RX320_LINE:
      _linevol = vol;
      vcmd[0] = 'A';
      break;

    default:
    case RX320_BOTH:
      _spkvol = _linevol = vol;
      vcmd[0] = 'C';
      break;
  }
  vcmd[1] = 0;
  vcmd[3] = 0x0D;
  if ((vol >= 0) && (vol < 64)) {
    vcmd[2] = _muted ? 63 : vol;
    return SendCommand(vcmd, 4);
  }

  return 0;
}

int ORX320::GetVolume(int output) const {

  switch (output) {
    case RX320_SPEAKER:
        return _spkvol;

    case RX320_LINE:
        return _linevol;

    default:
        return 0;
  }
}


// AGC: SLOW MEDIUM FAST

int ORX320::SetAGC(int agc) {
  char agccmd[3];

  _agc = agc;

  agccmd[0] = 'G';
  switch (agc) {
    case RX320_AGC_SLOW:
      agccmd[1] = '1';
      break;

    default:
    case RX320_AGC_MEDIUM:
      agccmd[1] = '2';
      break;

    case RX320_AGC_FAST:
      agccmd[1] = '3';
      break;

  }
  agccmd[2] = 0x0D;

  return SendCommand(agccmd, 3);
}

int ORX320::SetCWO(int cwo) {
  _cwo = cwo;
  if (_mode == RX320_CW)
    return SetFrequency(_freq);
  else
    return 0;
}

int ORX320::SetPBT(int pbt) {
  _pbt = pbt;
  return SetFrequency(_freq);
}

// FREQUENCY: COARSE -- FINE -- BFO

int ORX320::SetFrequency(long freq) {
  char fcmd[8];
  long AdjTfreq;		// Adjusted Tuned Frequency
  float Fcor, Cbfo;
  int Ctf;			// Coarse tuning factor
  int Ftf;			// Fine Tuning Factor
  int Btf;			// BFO Tuning factor

  _freq = freq;

  if (_mode == RX320_CW) {
    Fcor = 0.0;
    Cbfo = (float) _cwo;
  } else {
    Fcor = ((float) _filter->bandwidth / 2.0) + 200.0;
    Cbfo = 0.0;
  }

  AdjTfreq = freq - 1250 + (int) (Mcor * (Fcor + _pbt));
  Ctf = (int) AdjTfreq / 2500 + 18000;
  Ftf = (int) ((float) (AdjTfreq % 2500) * 5.46);
  Btf = (int) ((Fcor + (float) _pbt + Cbfo + 8000.0) * 2.73);

  fcmd[0] = 'N';
  fcmd[1] = 0xFF & (Ctf >> 8);
  fcmd[2] = 0xFF & Ctf;
  fcmd[3] = 0XFF & (Ftf >> 8);
  fcmd[4] = 0xFF & Ftf;
  fcmd[5] = 0xFF & (Btf >> 8);
  fcmd[6] = 0xFF & Btf;
  fcmd[7] = 0x0D;

  return SendCommand(fcmd, 8);
}

// SIGNAL STRENGTH

int ORX320::GetSignal() {
  char scmd[2];
  unsigned char response[5];
  unsigned int hi = 0, lo = 0, i;

  if (_fd < 0) return -1;

  scmd[0] = 'X';
  scmd[1] = 0x0D;
  SendCommand(scmd, 2, True);

  for (i = 0; i < 4; ++i) {
    read(_fd, &response[i], 1);
  }

  response[4] = 0;
  if (response[0] == 'X') {
    hi = response[1];
    lo = response[2];
  }

  return (256 * hi + lo);
}

void ORX320::RequestSignal(int rate) {
  _rate = rate;
  if (_rate == 0) {
    if (_tsignal) delete _tsignal;
    _tsignal = NULL;
  } else if (_tsignal == NULL) {
    _tsignal = new OTimer(this, _rate);
  }
}

// FIRMWARE REVISION

char *ORX320::GetFirmwareVersion() {
  char vcmd[2];
  static char vers[80];

  vcmd[0] = '?';
  vcmd[1] = 0x0D;
  SendCommand(vcmd, 2, True);
  GetResponse(vers, 80, 0x0D);

  return vers;
}

void ORX320::CompressEvents(int onoff) {
  if (onoff) {
    _compress = True;
  } else {
    // flush the queue before turning off _compress
    for (int i = 0; i < _queue.size(); ++i) {
      if (_fd >= 0) write(_fd, _queue[i].cmd, _queue[i].len);
      delete[] _queue[i].cmd;
    }
    _queue.clear();
    _compress = False;
    if (_tqueue) delete _tqueue;
    _tqueue = 0;
  }
}

// Send command

int ORX320::SendCommand(char *cmd, int len, int now) {
  if (_fd < 0) return -1;
  if (now || !_compress) {
    write(_fd, cmd, len);
    return 0;
  } else {
    // special case: mode (M) or filter (W) commands are always followed by
    // a tunning (N) command, arrange thing so they are sent together. This
    // minimizes clicking noises when operating the BW slider and prevents
    // the N command replacing an N queued *before* the M or W.
    if ((cmd[0] == 'N') && (_queue.size() > 0)) {
      int last = _queue.size() - 1;
      if (_queue[last].cmd[0] == 'M' || _queue[last].cmd[0] == 'W') {
        char *oldcmd = _queue[last].cmd;
        _queue[last].cmd = new char[_queue[last].len + len];
        memcpy(_queue[last].cmd, oldcmd, _queue[last].len);
        memcpy(_queue[last].cmd + _queue[last].len, cmd, len);
        _queue[last].len += len;
        delete[] oldcmd;
        return 0;  // we *know* _tqueue already exists
      }
    }
    for (int i = 0; i < _queue.size(); ++i) {
      if (_queue[i].cmd[0] == cmd[0]) {
        memcpy(_queue[i].cmd, cmd, len);
        _queue[i].len = len;   // this is not redundant: we need it
                               // as a consequence of the above
        return 0;
      }
    }
    _qelem q;
    q.cmd = new char[q.len = len];
    memcpy(q.cmd, cmd, len);
    _queue.push_back(q);
    if (!_tqueue) _tqueue = new OTimer(this, len * 10);
    return 0;
  }
}

int ORX320::GetResponse(char *buf, int n, char term) {
  int i;

  if (_fd < 0) return 0;
  for (i = 0; i < n; i++) {
    read(_fd, &buf[i], 1);
    if (buf[i] == term) break;
  }
  buf[i] = 0;
  return i;
}

// open serial port, return 0 on success or -1 on error;
// use GetLastError() to get a descriptive error message string

int ORX320::OpenSerial(const char *dev) {
  struct termios ts;
  int off = 0;
  int modemlines;

  _fd = open(dev, O_RDWR | O_NOCTTY | O_NONBLOCK, 0);
  if (_fd < 0) {
    sprintf(_errmsg, "Can't open %s: %s", dev, strerror(errno));
    return -1;
  }
  if (!isatty(_fd)) {
    sprintf(_errmsg, "%s is not a tty", dev);
    close(_fd);
    _fd = -1;
    return -1;
  }
  if (tcgetattr(_fd, &ts) == -1) {
    sprintf(_errmsg, "failed to read tty settings from %s", dev);
    close(_fd);
    _fd = -1;
    return -1;
  }
  cfmakeraw(&ts);
  ts.c_iflag = IGNBRK;	/* | IGNCR; */
  ts.c_oflag = 0;
  ts.c_cflag = CS8 | CREAD | CLOCAL;
  ts.c_lflag = 0;
  ts.c_cc[VMIN] = 1;
  ts.c_cc[VTIME] = 0;
  if (cfsetospeed(&ts, B1200) == -1) {
    sprintf(_errmsg, "failed to set output speed on %s: %s",
            dev, strerror(errno));
    close(_fd);
    _fd = -1;
    return -1;
  }
  if (cfsetispeed(&ts, B1200) == -1) {
    sprintf(_errmsg, "failed to set input speed on %s: %s",
            dev, strerror(errno));
    close(_fd);
    _fd = -1;
    return -1;
  }
  if (tcsetattr(_fd, TCSAFLUSH, &ts) == -1) {
    sprintf(_errmsg, "failed to set line attributes on %s: %s",
            dev, strerror(errno));
    close(_fd);
    _fd = -1;
    return -1;
  }

  // set the line back to blocking mode after setting CLOCAL.
  if (ioctl(_fd, FIONBIO, &off) < 0) {
    sprintf(_errmsg, "failed to set blocking mode on %s: %s",
            dev, strerror(errno));
    close(_fd);
    _fd = -1;
    return -1;
  }
  modemlines = TIOCM_RTS;
  if (ioctl(_fd, TIOCMBIC, &modemlines)) {
    sprintf(_errmsg, "failed to clear RTS line on %s: %s",
            dev, strerror(errno));
    close(_fd);
    _fd = -1;
    return -1;
  }

  _fh = new OFileHandler(this, _fd, XCM_READABLE);

  strcpy(_errmsg, "");
  return 0;
}

int ORX320::CloseSerial() {
  if (_fd >= 0) close(_fd);
  _fd = -1;
  if (_fh) delete _fh;
  _fh = NULL;
  return 0;
}
