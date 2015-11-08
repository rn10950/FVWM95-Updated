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

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>

#include <xclass/OExec.h>


ODList *OExec::_childList = NULL;

// TODO:
// - signal handler can be called when application exits or is stopped,
//   currently we assume only the first case. Need to implement the second.


//----------------------------------------------------------------------

// pipe_io - set this to True if you want to redirect the program's I/O
// persistent - set this to True if you do not want the application to
//    be killed when the corresponding OExec object is deleted (children
//    will survive parent's death).

OExec::OExec(OXClient *client, const char *prog, char *argv[],
             const char *start_dir, int pipe_io, int persistent) :
  OComponent(client) {

  int fd0[2], fd1[2], fd2[2];

  _prog = StrDup(prog);

  if (!_childList) {
    _childList = new ODList("Application Children");
    if (!_childList) FatalError("OExec: could not create children list");
    signal(SIGCHLD, OExec::CatchExit);
  }

  _msgObject = NULL;

  if (pipe_io) {
    pipe(fd0);
    pipe(fd1);
    pipe(fd2);
  }

  sigset_t smask;
  sigemptyset(&smask);
  sigaddset(&smask, SIGCHLD);
  sigprocmask(SIG_BLOCK, &smask, NULL);

  _pid = fork();
  if (_pid == 0) {
    if (pipe_io) {
      dup2(fd0[0], fileno(stdin));
      close(fd0[1]);
      dup2(fd1[1], fileno(stdout));
      close(fd1[0]);
      dup2(fd2[1], fileno(stderr));
      close(fd2[0]);
    }
    if (start_dir && *start_dir) chdir(start_dir);
    execvp(prog, argv);
    fprintf(stderr, "OExec: failed to execute %s\n", prog);
    exit(_status = 255);
  } else {
    if (pipe_io) {
      _input_fd = fd0[1];  // we write to the application's input
      close(fd0[0]);
      _output_fd = fd1[0]; // likewise, we read from application's output
      close(fd1[1]);
      _error_fd = fd2[0];
      close(fd2[1]);
    } else {
      _input_fd = -1;
      _output_fd = -1;
      _error_fd = -1;
    }
  }

  _childList->Add(_pid, (XPointer) this);
  sigprocmask(SIG_UNBLOCK, &smask, NULL);

  _status = 0;
  _persistent = persistent;
  _idle = NULL;
}

OExec::~OExec() {
  _childList->Remove(_pid);  // this must be done first!
  if (_idle) delete _idle;
  if (_input_fd  >= 0) close(_input_fd);
  if (_output_fd >= 0) close(_output_fd);
  if (_error_fd  >= 0) close(_error_fd);
  if (!_persistent) {
    Kill();
    //if (_pid > 0) waitpid(_pid, &_status, WNOHANG);
  }
  delete[] _prog;
}

int OExec::HandleIdleEvent(OIdleHandler *idle) {
  if (idle != _idle) return False;

  OExecMessage msg(MSG_EXEC, MSG_APPEXITED, _pid);
  SendMessage(_msgObject, &msg);

  delete _idle;
  _idle = NULL;

  //_pid = -1;

  return True;
}

int OExec::Read(char *buf, int len) {
  return read(_output_fd, buf, len);
}

int OExec::ReadError(char *buf, int len) {
  return read(_error_fd, buf, len);
}

int OExec::Write(char *buf, int len) {
  return write(_input_fd, buf, len);
}

int OExec::Kill(int signal) {
  if (_pid > 0) return kill(_pid, signal);
  errno = ESRCH;
  return -1;
}

int OExec::Wait(int options) {
#ifdef HAVE_WAITPID  
  return waitpid(_pid, &_status, options);
#else
  return wait4(_pid, &_status, options, NULL);
#endif
}

int OExec::IsRunning() {
  if (_pid < 0) return False;
  int retc = Wait(WNOHANG);
  if (retc == 0) return True;
  if (retc == _pid) _pid = -1;
  return False;
}

int OExec::GetExitCode() {
  if (WIFEXITED(_status)) return WEXITSTATUS(_status);
  return -1;
}

void OExec::CatchExit(int signo) {
  OXSNode *e;
  int pid, status;

  signal(SIGCHLD, OExec::CatchExit);

  pid = wait(&status);
  e = _childList->GetNode(pid);
  if (e) {
    OExec *exec = (OExec *) e->data;
    exec->_Exited(status);
  }
}

int OExec::_Exited(int status) {
  _status = status;

  // Do not send the MSG_APPEXITED at this point, since we're still
  // inside the signal handler and the application might generate
  // Xlib requests in response to the message that would conflict with
  // interrupted Xlib calls. Instead, schedule an idle event and then
  // send the message when it is safe to do so.

  if (_idle) delete _idle;
  _idle = _client ? new OIdleHandler(this) : NULL;

  return 0;
}
