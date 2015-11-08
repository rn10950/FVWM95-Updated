#ifndef __OXMAILCHECKPLUGIN_H
#define __OXMAILCHECKPLUGIN_H

#include <xclass/OXClient.h>
#include <xclass/OXFrame.h>
#include <xclass/OXSList.h>
#include <xclass/OLayout.h>
#include <xclass/OIniFile.h>


#define HAS_NO_MAIL      0
#define HAS_MAIL         1
#define HAS_NEW_MAIL     2
#define HAS_UNREAD_MAIL  3


class OXMailCheckPlugin;
class OPicture;

extern "C" {
  OXMailCheckPlugin *CreatePlugin(const OXWindow *p, OComponent *c,
                                  OIniFile *rcfile, int id);
}

class OXMailCheckPlugin : public OXFrame {
public:
  OXMailCheckPlugin(const OXWindow *p, OComponent *c,
                    OIniFile *rcfile, int id);
  virtual ~OXMailCheckPlugin();
  
  virtual ODimension GetDefaultSize() const;

  virtual int HandleTimer(OTimer *t);
  virtual int HandleButton(XButtonEvent *event);
  virtual int HandleDoubleClick(XButtonEvent *event);

protected:
  virtual void _DoRedraw();

  void _ParseConfig(OIniFile *rcfile);
  int  _CheckMail();

  const OPicture *_icon[4];
  OTimer *_timer;
  char *_mailFile, *_msg[4],
       *_actionClick, *_actionDblClick, *_actionNewMail;
  long _mailSize;
  int _currentStatus, _updateInterval, _visible;
  
  int _pluginID;
  OComponent *_msgObject;
};

#endif  // __OXMAILCHECKPLUGIN_H
