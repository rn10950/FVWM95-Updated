#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
//#include <sys/types.h>
//#include <arpa/inet.h>
//#include <netdb.h>
#include <errno.h>

#include "OTcp.h"
#include "OTcpMessage.h"


//----------------------------------------------------------------------

OTcp::OTcp() : OComponent() {
  _rem_bytes = 0;
  _msgObject = NULL;
  _holdon = False;
  _fd = -1;
}

OTcp::~OTcp() {
  Close();
}

int OTcp::BuildAddress(const char *server, int port) {
  char buf[1024];
 
  _fd = 0;

  if (!server) {
    host.sin_addr.s_addr = INADDR_ANY;
    host.sin_family = AF_INET;
//    gethostname(buf, sizeof(buf));
//    server = buf;
  } else {
    if ((host.sin_addr.s_addr = inet_addr(server)) != -1) {
      host.sin_family = AF_INET;
    } else {
      if ((hp = gethostbyname(server)) != NULL) {
        bzero((char *) &host, sizeof(host));
       	bcopy(hp->h_addr, (char *) &host.sin_addr, hp->h_length);
        host.sin_family = hp->h_addrtype;
      } else {
        _fd = -2;
      }
    }
  }
  
  if (!_fd) {
    host.sin_port = (unsigned short) htons(port);
    if ((_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      _fd = -3;  // address error
    }
  }

  return _fd;
}

int OTcp::Bind(int port) {

  _fd = BuildAddress(0, port);

  // ioctl SIOCGIFCONF to find out all the hostnames on this system
  // used in bind http://www.isc.org/bind.html

  if (_fd > 0) {
    if (bind(_fd, (struct sockaddr *) &host, sizeof(host)) < 0) {
      close(_fd);
      _fd = -4;
      return _fd;
    }
    fcntl(_fd, F_SETFL, FNDELAY);
    fcntl(_fd, F_SETLK, F_UNLCK);
  }

  return _fd;
}

int OTcp::Connect(const char *server, int port, int async) {
  int fd = BuildAddress(server, port);
  if (fd > 0) fd = _Connect(fd, async);
  return fd;
}

int OTcp::Connect(unsigned long server, int port, int async) {
  host.sin_addr.s_addr = htonl(server);
  host.sin_family = AF_INET;
  host.sin_port = (unsigned short) htons(port);

  if ((_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    _fd = -3;  // address error
    return _fd;
  }

  return _Connect(_fd, async);
}

int OTcp::_Connect(int fd, int async) {
  int status, curState, origState;

  _fd = fd;
  _rem_bytes = 0;

  if (_fd > 0) {

    // Set the close-on-exec flag so that the socket will not get
    // inherited by child processes.
    fcntl(_fd, F_SETFD, FD_CLOEXEC);

    //if (connect(_fd, (struct sockaddr *) &host, sizeof(host)) < 0) {
    //  close(_fd);
    //  _fd = -4;
    //}


    // Attempt to connect. The connect may fail at present with an
    // EINPROGRESS but at a later time it will complete. The caller
    // should set up a OFileHandler on the socket in order to be
    // notified when the connect completes.

    if (async) {
#ifdef USE_FIONBIO
      curState = 1;
      status = ioctl(_fd, FIONBIO, &curState);
#else
      origState = fcntl(_fd, F_GETFL);
      curState = origState | O_NONBLOCK;
      status = fcntl(_fd, F_SETFL, curState);
#endif            
    } else {
      status = 0;
    }

    if (status > -1) {
      status = connect(_fd, (struct sockaddr *) &host, sizeof(host));
      if (status < 0) {
        if (errno == EINPROGRESS) {
//          asyncConnect = 1;
          status = 0;
        } else {
          close(_fd);
          _fd = -4;
        }
      }
    }

  }

  if (_fd > 0) {
    fcntl(_fd, F_SETFL, FNDELAY);
    fcntl(_fd, F_SETLK, F_UNLCK);
  }

  return _fd;
}

int OTcp::GetLocalAddress(unsigned long *hostr, unsigned short *portr) {
  int length = sizeof(host);

  if (getsockname(_fd, (struct sockaddr *) &host, (socklen_t *) &length))
    return False;

  *hostr = host.sin_addr.s_addr;
  *portr = host.sin_port;

  return True;
}

int OTcp::Listen(unsigned long *hostr, unsigned short *portr, OTcp *serv) {
  unsigned long th;
  unsigned short tp;

  if (Bind(0) < 0) return False;

  if (serv) {
    if (!serv->GetLocalAddress(hostr, &tp)) return False;
  }
  if (!GetLocalAddress(&th, portr)) return False;
  if (listen(_fd, 4) < 0) return False;

  return True;
}

int OTcp::Accept(int sockfd) {
  int len = sizeof(struct sockaddr_in);

  _fd = accept(sockfd, (struct sockaddr *) &host, (socklen_t *) &len);
  if (_fd < 0) return _fd;
  fcntl(_fd, F_SETFD, FD_CLOEXEC);

  return _fd;
}

int OTcp::Send(const char *message) {
  return(send(_fd, message, strlen(message), 0));
}

int OTcp::BinarySend(const char *buf, unsigned long size) {
  return(send(_fd, buf, size, 0));
}

int OTcp::Receive() {
  int  i;
  char *msg;

  if (_holdon) return 0;

  _blength = recv(_fd, &_buffer[_rem_bytes],
                  TCP_BUFFER_LENGTH - _rem_bytes, 0);

  if (_blength > 0) {
    for (i = 0, msg = _buffer; i < _blength + _rem_bytes; i++) {
      if (_buffer[i] == '\n' || _buffer[i] == '\r') {
        _buffer[i] = 0;
	if (_msgObject && *msg) {
          _holdon = True;
	  OTcpMessage message(INCOMING_TCP_MSG, 0, msg);
          SendMessage(_msgObject, &message);
          _holdon = False;
          if (_fd < 0) return 0;  // can happen due to reentrancy
	}
        msg = _buffer + i + 1;
      }
    }

    _rem_bytes += _buffer + _blength - msg;
    for (i = 0; i < _rem_bytes; i++)
      _buffer[i] = msg[i];

    _buffer[_rem_bytes] = 0;
    if (_rem_bytes) {
      Receive();
    }

    return 1;
  } else {
    if (_blength == 0) {
      _holdon = True;
      OTcpMessage message(BROKEN_PIPE, 0);
      SendMessage(_msgObject, &message);
      _holdon = False;
    }
    return 0;
  }
}

int OTcp::BinaryReceive(char *buf, unsigned long size) {
  _blength = recv(_fd, buf, size, 0);
  return _blength;
}

int OTcp::ProcessMessage(OMessage *msg) {
  if (msg->type == OUTGOING_TCP_MSG) {
    OTcpMessage *tcpmsg = (OTcpMessage *) msg;
    if (Send((char *) tcpmsg->string->GetString()) < 0) {
      if (errno == EPIPE) {
        // hmmm... a message back
        OTcpMessage message(BROKEN_PIPE, 0);
        SendMessage(_msgObject, &message);
      }
    }
    return True;
  }
  fprintf(stderr, "Bad message received (OTcp)\n");
  return False;
}
