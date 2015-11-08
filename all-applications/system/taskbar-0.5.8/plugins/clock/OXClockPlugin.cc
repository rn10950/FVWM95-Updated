#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <xclass/OXClient.h>
#include <xclass/OXFrame.h>
#include <xclass/OTimer.h>
#include <xclass/OXFont.h>
#include <xclass/OGC.h>

#include "OXClockPlugin.h"
#include "../../plugins.h"


OXClockPlugin *CreatePlugin(const OXWindow *p, OComponent *c,
                            OIniFile *rcfile, int id) {
  return new OXClockPlugin(p, c, rcfile, id);
}

// Maybe we shoud add support for alarms?
        
OXClockPlugin::OXClockPlugin(const OXWindow *p, OComponent *c,
                             OIniFile *rcfile, int id) :
  OXFrame(p, 16, 16, CHILD_FRAME) {
    XGCValues gcval;
    unsigned long gcm;

    _msgObject = c;
    _pluginID = id;

    _font = _client->GetFont("fixed");

    gcm = GCForeground | GCBackground | GCFont | GCGraphicsExposures;
    gcval.foreground = _blackPixel;
    gcval.background = _backPixel;
    gcval.font = _font->GetId();
    gcval.graphics_exposures = False;
    _gc = new OXGC(GetDisplay(), _id, gcm, &gcval);

    _timeFmt = StrDup("%H:%M");
    _dateFmt = StrDup("%A, %B %d, %Y");

    _actionClick = NULL;
    _actionDblClick = NULL;

    _ParseConfig(rcfile);

    _SetTimeStr();
    _SetDateStr();

    SetTip(_dateStr);

    _timer = new OTimer(this, 30000);

    AddInput(ButtonPressMask | ButtonReleaseMask);
}

OXClockPlugin::~OXClockPlugin() {
  delete _timer;
  delete _gc;
  delete[] _dateFmt;
  delete[] _timeFmt;
  if (_actionClick) delete[] _actionClick;
  if (_actionDblClick) delete[] _actionDblClick;
  _client->FreeFont((OXFont *) _font);
}

void OXClockPlugin::_ParseConfig(OIniFile *rcfile) {
  char arg[256];

  if (rcfile->GetItem("date_format", arg)) {
    delete[] _dateFmt;
    _dateFmt = StrDup(arg);
  }

  if (rcfile->GetItem("time_format", arg)) {
    delete[] _timeFmt;
    _timeFmt = StrDup(arg);
  }

  if (rcfile->GetItem("fvwm_action_click", arg)) {
    if (_actionClick) delete[] _actionClick;
    _actionClick = StrDup(arg);
  }

  if (rcfile->GetItem("fvwm_action_doubleclick", arg)) {
    if (_actionDblClick) delete[] _actionDblClick;
    _actionDblClick = StrDup(arg);
  }

}

void OXClockPlugin::_DoRedraw() {
  int y;
  OFontMetrics fm;

  OXFrame::_DoRedraw();

  _font->GetFontMetrics(&fm);

  y = (_h - fm.linespace) >> 1;
  DrawString(_gc->GetGC(), 2, y + fm.ascent, _timeStr, strlen(_timeStr));
}

ODimension OXClockPlugin::GetDefaultSize() const {
  return ODimension(_font->XTextWidth(_timeStr)+4, _font->TextHeight());
}

int OXClockPlugin::HandleTimer(OTimer *t) {

  if (t != _timer) return False;

  delete _timer;
  _timer = new OTimer(this, 30000);

  if (_SetTimeStr()) NeedRedraw();

  if (_SetDateStr()) {
    RemoveTip();
    SetTip(_dateStr);
  }

  return True;
}

int OXClockPlugin::HandleButton(XButtonEvent *event) {
  if ((event->type == ButtonPress) && _actionClick) {
    OPluginMessage msg(MSG_PLUGIN, PLUGIN_DOACTION, _pluginID,
                       _actionClick);
    SendMessage(_msgObject, &msg);
  }
  return True;
}

int OXClockPlugin::HandleDoubleClick(XButtonEvent *event) {
  if (_actionDblClick) {
    OPluginMessage msg(MSG_PLUGIN, PLUGIN_DOACTION, _pluginID,
                       _actionDblClick);
    SendMessage(_msgObject, &msg);
  }
  return True;
}

int OXClockPlugin::_SetTimeStr() {
  struct tm *tms;
  time_t tmt;
  char newTime[100];
  int changed;

  time(&tmt);
  tms = localtime(&tmt);
  strftime(newTime, 100, _timeFmt, tms);
  if (newTime[0] == '0') newTime[0] = ' ';

  changed = strcmp(_timeStr, newTime);
  strcpy(_timeStr, newTime);

  return changed;
}

int OXClockPlugin::_SetDateStr() {
  struct tm *tms;
  time_t tmt;
  char newDate[100];
  int changed;

  time(&tmt);
  tms = localtime(&tmt);
  strftime(newDate, 100, _dateFmt, tms);

  changed = strcmp(_dateStr, newDate);
  strcpy(_dateStr, newDate);

  return changed;
}
