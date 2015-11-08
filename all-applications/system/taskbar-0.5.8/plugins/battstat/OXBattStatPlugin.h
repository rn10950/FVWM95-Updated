#ifndef __OXBATTSTATPLUGIN_H
#define __OXBATTSTATPLUGIN_H

#include <xclass/OXClient.h>
#include <xclass/OXFrame.h>
#include <xclass/OIniFile.h>
#include <xclass/OPicture.h>


#define PROC_APM_FILE  "/proc/apm"

#define APM_UNKNOWN  0
#define APM_ALERT    1
#define APM_EMPTY    2
#define APM_HALF     3
#define APM_FULL     4
#define APM_ONLINE   5
#define APM_LOADING  6


class OXBattStatPlugin;

extern "C" {
  OXBattStatPlugin *CreatePlugin(const OXWindow *p, OComponent *c,
                                 OIniFile *rcfile, int id);
}

class OXBattStatPlugin : public OXFrame {
public:
  OXBattStatPlugin(const OXWindow *p, OComponent *c,
                   OIniFile *rcfile, int id);
  virtual ~OXBattStatPlugin();
  
  virtual ODimension GetDefaultSize() const;

  virtual int HandleTimer(OTimer *t);
  virtual int HandleButton(XButtonEvent *event);
  virtual int HandleDoubleClick(XButtonEvent *event);

protected:
  virtual void _DoRedraw();

  void _ParseConfig(OIniFile *rcfile);
  int  _GetBattStatus();
  void _UpdateStatus();
  void _UpdateTip();

  const OPicture *_icon[7];
  OTimer *_timer;
  int _acStatus, _battStatus, _battPercent, _currentStatus;
  int _fd, _apmAvailable, _updateInterval, _visible;
  
  int _pluginID;
  OComponent *_msgObject;
  
  char *_actionClick, *_actionDblClick, *_actionAlert;
};

#endif  // __OXBATTSTATPLUGIN_H
