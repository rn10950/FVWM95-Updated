#ifndef __OTCP_H
#define __OTCP_H

/*
 * I've (mike) made some changes to this class so that it can be included
 * in the libfOX. I've bumped the OIrcMessage up a level and created a
 * OTcpMessage. It contains the string1 of OIrcMessage.
 * I've also added a bind command to begin coding the server side in.
 * To accomodate these changes I moved the host and hp structs to the
 * class scope. They are now setup using the BuildAddress function,
 * which is called by both Connect and Bind.
 */

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#ifdef __FreeBSD__
#include <netinet/in.h>
#endif
#include <sys/socket.h>
#include <unistd.h>
#include <xclass/OComponent.h>

//#define TCP_BUFFER_LENGTH 65535
#define TCP_BUFFER_LENGTH 16383

//----------------------------------------------------------------------

class OTcp : public OComponent {
public:
  OTcp();
  virtual ~OTcp();

  int  Bind(int port);
  int  Connect(const char *server, int port, int async);
  int  Connect(unsigned long server, int port, int async);
  int  Listen(unsigned long *hostr, unsigned short *portr, OTcp *serv = NULL);
  int  Accept(int sockfd);
  void Close() { if (_fd > 0) { close(_fd); _fd = -1; } }
  int  Send(const char *message);
  int  BinarySend(const char *buf, unsigned long size);
  int  Receive();
  int  BinaryReceive(char *buf, unsigned long size);
  int  GetLocalAddress(unsigned long *hostr, unsigned short *portr);
  int  GetFD() { return _fd; }
  char *GetAddress() const { return inet_ntoa(host.sin_addr); }
  int  GetPort() const { return ntohs(host.sin_port); }

  virtual int ProcessMessage(OMessage *msg);

protected:

  // Builds a sockaddr_in and hostent.
  // Also creates a socket and returns the file descriptor.
  int BuildAddress(const char *server, int port);

  int _Connect(int fd, int async);

  char *_server;
  int _port;
  int _fd;
  struct sockaddr_in host;
  struct hostent *hp;

  char _buffer[TCP_BUFFER_LENGTH+1];
  int _blength, _rem_bytes, _holdon;
};


#endif  // __OTCP_H
