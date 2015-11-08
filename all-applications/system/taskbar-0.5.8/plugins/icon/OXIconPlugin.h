#ifndef __OXICONPLUGIN_H
#define __OXICONPLUGIN_H

#include <xclass/OXClient.h>
#include <xclass/OXFrame.h>


class OXIconPlugin;
class OPicture;
class OIniFile;

extern "C" {
  OXIconPlugin *CreatePlugin(const OXWindow *p, OComponent *c,
                             OIniFile *rcfile, int id);
}

class OXIconPlugin : public OXFrame {
public:
  OXIconPlugin(const OXWindow *p, OComponent *c, OIniFile *rcfile, int id);
  virtual ~OXIconPlugin();
  
  virtual ODimension GetDefaultSize() const;

  virtual int HandleButton(XButtonEvent *event);
  virtual int HandleDoubleClick(XButtonEvent *event);

protected:
  virtual void _DoRedraw();
  
  void _ParseConfig(OIniFile *rcfile);

  const OPicture *_icon;
  char *_actionClick, *_actionDblClick, *_tipMsg;

  int _pluginID;
  OComponent *_msgObject;
};

#endif  // __OXICONPLUGIN_H
