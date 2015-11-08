/**************************************************************************

    This file is part of xclass.
    Copyright (C) 2001, Hector Peraza.

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

#ifndef __OEXEC_H
#define __OEXEC_H

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>

#include <xclass/utils.h>
#include <xclass/OComponent.h>
#include <xclass/OIdleHandler.h>
#include <xclass/OMessage.h>
#include <xclass/OXSList.h>


//----------------------------------------------------------------------

#define OExecMessage OWidgetMessage

class OExec : public OComponent {
protected:
  static ODList *_childList;
  static void CatchExit(int signo);

public:
  OExec(OXClient *c, const char *prog, char *argv[], const char *sdir = NULL,
        int pipe_io = True, int persistent = False);
  virtual ~OExec();

  virtual int HandleIdleEvent(OIdleHandler *);

  int Read(char *buf, int len);
  int ReadError(char *buf, int len);
  int Write(char *buf, int len);
  
  int GetInputFd() const { return _input_fd; }
  int GetOutputFd() const { return _output_fd; }
  int GetErrorFd() const { return _error_fd; }
  
  int Kill(int signal = SIGTERM);
  int IsRunning();
  int Wait(int options = 0);
  int GetStatus() const { return _status; }
  int GetExitCode();
  
  const char *GetProgramName() const { return _prog; }
  
protected:
  int _Exited(int status);

  int _input_fd, _output_fd, _error_fd;
  int _pid, _status, _persistent;
  
  OIdleHandler *_idle;
  char *_prog;
};


#endif  // __OEXEC_H
