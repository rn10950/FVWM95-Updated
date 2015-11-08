#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "OIrcMessage.h"
#include "IRCcodes.h"
#include "OIrc.h"


//#define DEBUG

//----------------------------------------------------------------------

OIrcMessage::OIrcMessage(int typ, int act, const char *raw_msg) :
  OMessage(typ, act) {

  rawmsg = raw_msg;

  prefix = NULL;
  command = NULL;
  for (argc = 0; argc < 16; ++argc) argv[argc] = NULL;

  // using strncpy for safety...

  strncpy(msg, raw_msg, IRC_MSG_LENGTH);
  msg[IRC_MSG_LENGTH-1] = '\0';

  char *p = msg;

  if (*p == ':') {
    prefix = ++p;
    while (*p && *p != ' ') ++p;
    if (*p) *p++ = '\0';
    while (*p && *p == ' ') ++p;
  }

  command = p;
  while (*p && *p != ' ') ++p;
  if (*p) *p++ = '\0';
  while (*p && *p == ' ') ++p;

  argc = 0;
  while (*p) {
    if (*p == ':') {
      argv[argc++] = ++p;
      break;
    } else {
      argv[argc++] = p;
      if (argc == IRC_MAX_ARGS) break;
      while (*p && *p != ' ') ++p;
      if (*p) *p++ = '\0';
      while (*p && *p == ' ') ++p;
    }
  }

  if (prefix) {
    char *p = strchr(prefix, '!');
    if (p) {
      nick = new char[p - prefix + 1];
      strncpy(nick, prefix, p - prefix);
      nick[p - prefix] = '\0';
      ircname = StrDup(p + 1);
    } else {
      nick = StrDup("");
      ircname = StrDup(prefix);
    }
  } else {
    nick = StrDup("");
    ircname = StrDup("");
  }

#ifdef DEBUG
  fprintf(stderr, "*** raw msg:  \"%s\"\n", rawmsg);
  fprintf(stderr, "+++ prefix:   \"%s\"\n", prefix);
  fprintf(stderr, "++++ nick:    \"%s\"\n", nick);
  fprintf(stderr, "++++ ircname: \"%s\"\n", ircname);
  fprintf(stderr, "+++ command:  \"%s\"\n", command);
  for (int i = 0; i < 16; ++i) {
    if (!argv[i]) break;
    fprintf(stderr, "+++ argv[%02d]: \"%s\"\n", i, argv[i]);
  }
  fprintf(stderr, "\n");
#endif
}

OIrcMessage::~OIrcMessage() {
  delete[] nick;
  delete[] ircname;
}


//----------------------------------------------------------------------

OIrc::OIrc() : OComponent() {
  _tcp = new OTcp();
  _tcp->Associate(this);
}

OIrc::~OIrc() {
  delete _tcp;
}

int OIrc::ProcessMessage(OMessage *msg) {

  switch (msg->type) {
    case INCOMING_TCP_MSG:
      {
        OTcpMessage *tcpmsg = (OTcpMessage *) msg;
        char *tcp_message = (char *) tcpmsg->string->GetString();

        // ignore empty messages
	if (strlen(tcp_message) == 0) break;

        OIrcMessage message(INCOMING_IRC_MSG, 0, tcp_message);
        SendMessage(_msgObject, &message);

      }
      break;

    case OUTGOING_TCP_MSG:
#ifdef DEBUG
      fprintf(stderr, "<<< \"%s\"\n",
                      ((OTcpMessage *)msg)->string->GetString());
#endif
      SendMessage(_tcp, msg);
      break;

    case INCOMING_IRC_MSG:
      FatalError("OIrc: INCOMING_IRC_MSG");
      break;

    case OUTGOING_IRC_MSG:
      FatalError("OIrc: OUTGOING_IRC_MSG");
      break;

    case BROKEN_PIPE:
#ifdef DEBUG
      fprintf(stderr, "Remote end closed connection.\n");
#endif
      SendMessage(_msgObject, msg);
      break;
  }

  return True;
}
