#ifndef __OXLOCKFILEPLUGIN_H
#define __OXLOCKFILEPLUGIN_H

#include <xclass/OXClient.h>
#include <xclass/OXFrame.h>


class OXLockfilePlugin;
class OPicture;
class OIniFile;

extern "C" {
  OXLockfilePlugin *CreatePlugin(const OXWindow *p, OComponent *c,
                                 OIniFile *rcfile, int id);
}

class OXLockfilePlugin : public OXFrame {
public:
  OXLockfilePlugin(const OXWindow *p, OComponent *c,
                   OIniFile *rcfile, int id);
  virtual ~OXLockfilePlugin();
  
  virtual ODimension GetDefaultSize() const;

  virtual int HandleTimer(OTimer *t);
  virtual int HandleButton(XButtonEvent *event);
  virtual int HandleDoubleClick(XButtonEvent *event);

protected:
  virtual void _DoRedraw();
  
  void _ParseConfig(OIniFile *rcfile);
  int  _CheckFile();

  const OPicture *_iconOn, *_iconOff;
  OTimer *_timer;
  char *_lockfile, *_actionClick, *_actionDblClick,
       *_tipOnMsg, *_tipOffMsg;
  int _status, _visible;

  int _pluginID;
  OComponent *_msgObject;
};

#endif  // __OXLOCKFILEPLUGIN_H
