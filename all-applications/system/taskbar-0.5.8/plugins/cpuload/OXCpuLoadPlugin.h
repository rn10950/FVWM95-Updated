#ifndef __OXCPULOADPLUGIN_H
#define __OXCPULOADPLUGIN_H

#include <xclass/OXClient.h>
#include <xclass/OXFrame.h>


class OXCpuLoadPlugin;
class OIniFile;
class OProc;
class OTimer;
class OXPopupMenu;


extern "C" {
  OXCpuLoadPlugin *CreatePlugin(const OXWindow *p, OComponent *c,
                                OIniFile *rcfile, int id);
}

class OXCpuLoadPlugin : public OXFrame {
public:
  OXCpuLoadPlugin(const OXWindow *p, OComponent *c, OIniFile *rcfile, int id);
  virtual ~OXCpuLoadPlugin();

  virtual ODimension GetDefaultSize() const;

  virtual int HandleTimer(OTimer *t);
  virtual int HandleButton(XButtonEvent *event);
  virtual int ProcessMessage(OMessage *msg);

protected:
  virtual void _DoRedraw();
  
  void ToggleSMP();
  void ToggleGrid();
  void ToggleLabels();
  void Clear();
  void Unload();
  
  void _ParseConfig(OIniFile *rcfile);

  int _pluginID;
  OComponent *_msgObject;

  const OXFont *_font;
  OXGC *_gc;
  OXPopupMenu *_menu, *_dmenu, *_umenu, *_c0menu, *_c1menu;
  OTimer *_timer;
  unsigned long _pixel0, _pixel1;
  int _color0, _color1, _grid, _labels, _style, _upd;
  int _hasSMP, _SMPmode;
  int *_hbfr0, *_hbfr1, _hlen;
  OProc *_proc;
};

#endif  // __OXCPULOADPLUGIN_H
