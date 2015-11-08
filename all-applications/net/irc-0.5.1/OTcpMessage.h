#ifndef __OTCPMESSAGE_H
#define __OTCPMESSAGE_H

#include <xclass/OString.h>
#include <xclass/OMessage.h>


#define INCOMING_TCP_MSG	(MSG_USERMSG+20)
#define OUTGOING_TCP_MSG	(MSG_USERMSG+21)
#define BROKEN_PIPE		(MSG_USERMSG+25)


//-------------------------------------------------------------------------

class OTcpMessage : public OMessage {
public:
  OTcpMessage(int typ, int act, const char *str = "") :
    OMessage(typ, act) {
      string = new OString(str);
  }
  virtual ~OTcpMessage() { delete string; }
	
  OString *string;
};


#endif  // __OTCPMESSAGE_H
