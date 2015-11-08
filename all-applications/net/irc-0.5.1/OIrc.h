#ifndef __OIRC_H
#define __OIRC_H

#include <stdlib.h>
#include <stdio.h>

#include <xclass/OFileHandler.h>

#include "OTcp.h"


//---------------------------------------------------------------------

class OIrc : public OComponent {
public:
  OIrc();
  virtual ~OIrc();

  virtual int ProcessMessage(OMessage *msg);

  int  Connect(char *server, int port) 
       const { return _tcp->Connect(server, port, True); }
  void Close() const { _tcp->Close(); }
  int  GetFD() const { return _tcp->GetFD(); }

  int Receive() { return _tcp->Receive(); }
  OTcp *GetOTcp() { return _tcp; }

protected:
  OTcp *_tcp;
};

#endif  // __OIRC_H
