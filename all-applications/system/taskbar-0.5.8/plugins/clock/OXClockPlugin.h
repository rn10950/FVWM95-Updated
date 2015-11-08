#ifndef __OXCLOCKPLUGIN_H
#define __OXCLOCKPLUGIN_H

#include <xclass/OXClient.h>
#include <xclass/OXFrame.h>
#include <xclass/OXSList.h>
#include <xclass/OLayout.h>
#include <xclass/OIniFile.h>


class OXClockPlugin;

extern "C" {
  OXClockPlugin *CreatePlugin(const OXWindow *p, OComponent *c,
                              OIniFile *rcfile, int id);
}

class OXClockPlugin : public OXFrame {
public:
  OXClockPlugin(const OXWindow *p, OComponent *c, OIniFile *rcfile, int id);
  virtual ~OXClockPlugin();
  
  virtual ODimension GetDefaultSize() const;

  virtual int HandleTimer(OTimer *t);
  virtual int HandleButton(XButtonEvent *event);
  virtual int HandleDoubleClick(XButtonEvent *event);
    
protected:
  virtual void _DoRedraw();

  void _ParseConfig(OIniFile *rcfile);
  int  _SetDateStr();
  int  _SetTimeStr();

  const OXFont *_font;
  OXGC *_gc;
  OTimer *_timer;
  char _dateStr[100], _timeStr[100];
  char *_dateFmt, *_timeFmt, *_actionClick, *_actionDblClick;

  int _pluginID;
  OComponent *_msgObject;
};

#endif  // __OXCLOCKPLUGIN_H
