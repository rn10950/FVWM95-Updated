#ifndef __OXCHANNELLIST_H
#define __OXCHANNELLIST_H

#include <xclass/OXTransientFrame.h>
#include <xclass/OXListView.h>

class OXIrc;
class OXButton;


//----------------------------------------------------------------------

class OXChannelList : public OXTransientFrame {
public:
  OXChannelList(const OXWindow *p, const OXWindow *main,
                OXIrc *irc, int w, int h);
  virtual ~OXChannelList();

  virtual int ProcessMessage(OMessage *msg);
  virtual int CloseWindow();

  void ClearList();
  void AddChannel(const char *name, int nusers, const char *title);
  
protected:
  OXIrc *_irc;
  OXListView *_listView;
  OXButton *_filt, *_cl, *_join;
};


#endif  // __OXCHANNELLIST_H
