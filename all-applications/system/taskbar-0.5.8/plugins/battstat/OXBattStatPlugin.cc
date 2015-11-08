#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <sys/stat.h>

#include "OXBattStatPlugin.h"
#include "../../plugins.h"

#include "apm-loading.xpm"
#include "apm-online.xpm"
#include "apm-alert.xpm"
#include "apm-empty.xpm"
#include "apm-half.xpm"
#include "apm-full.xpm"
#include "apm-unknown.xpm"



OXBattStatPlugin *CreatePlugin(const OXWindow *p, OComponent *c,
                               OIniFile *rcfile, int id) {
  return new OXBattStatPlugin(p, c, rcfile, id);
}

OXBattStatPlugin::OXBattStatPlugin(const OXWindow *p, OComponent *c,
                                   OIniFile *rcfile, int id) :
  OXFrame(p, 16, 16, CHILD_FRAME) {

    _msgObject = c;
    _pluginID = id;

    _acStatus = _battStatus = _battPercent = -1;
    _currentStatus = APM_UNKNOWN;
    _apmAvailable = True;

    _icon[APM_LOADING] = _client->GetPicture("apm-loading.xpm", apm_loading_xpm);
    _icon[APM_ONLINE]  = _client->GetPicture("apm-online.xpm",  apm_online_xpm);
    _icon[APM_ALERT]   = _client->GetPicture("apm-alert.xpm",   apm_alert_xpm);
    _icon[APM_EMPTY]   = _client->GetPicture("apm-empty.xpm",   apm_empty_xpm);
    _icon[APM_HALF]    = _client->GetPicture("apm-half.xpm",    apm_half_xpm);
    _icon[APM_FULL]    = _client->GetPicture("apm-full.xpm",    apm_full_xpm);
    _icon[APM_UNKNOWN] = _client->GetPicture("apm-unknown.xpm", apm_unknown_xpm);

    _actionClick = NULL;
    _actionDblClick = NULL;
    _actionAlert = NULL;

    _updateInterval = 30;  // in seconds
    _fd = -1;

    _ParseConfig(rcfile);

    // Do not call _CheckBattStatus() here, but schedule instead an
    // immediate timer event...
    _timer = new OTimer(this, 1);

    AddInput(ButtonPressMask | ButtonReleaseMask);
}

OXBattStatPlugin::~OXBattStatPlugin() {
  int i;

  if (_timer) delete _timer;
  if (_fd > 0) close(_fd);

  for (i = 0; i < 7; ++i) {
    _client->FreePicture(_icon[i]);
  }

  if (_actionClick) delete[] _actionClick;
  if (_actionDblClick) delete[] _actionDblClick;
  if (_actionAlert) delete[] _actionAlert;
}


void OXBattStatPlugin::_UpdateStatus() {

  if (!_apmAvailable) {

    _currentStatus = APM_UNKNOWN;

  } else if (_acStatus == 0x01) {   // online

    _currentStatus = (_battStatus == 0x03) ? APM_LOADING : APM_ONLINE;

  } else {  // offline

    switch (_battStatus) {
      case 0:  _currentStatus = APM_FULL; break;
      case 1:  _currentStatus = APM_HALF; break;
      case 2:  _currentStatus = APM_ALERT; break;
      default: _currentStatus = APM_EMPTY; break;
    }

  }

}

// Read information from OS about battery status. This implementation uses
// Linux's /proc/apm. APM support must be enabled in the kernel.

int OXBattStatPlugin::_GetBattStatus() {
  char buffer[100];

  if (!_apmAvailable) return False;

  if (_fd < 0) {
    _fd = open(PROC_APM_FILE, O_RDONLY);
    if (_fd < 0) {
      _apmAvailable = False;
      return False;
    }
  } else {
    lseek(_fd, 0L, SEEK_SET);
  }

  // information about the layout and meaning of bits 
  // can be found in the apm_bios.c source file in the kernel.

  read(_fd, buffer, 100);

  _acStatus = _battStatus = _battPercent = 0;

  // skip driver version 
  // skip apm-bios version 
  // skip apm-bios flags

  int arg = sscanf(buffer, "%*s %*s %*s %x %x %*s %ld",
		   &_acStatus, &_battStatus, &_battPercent);
  if (arg < 3) {
    // fprintf(stderr, "%s: bad format or invalid version\n", PROC_APM_FILE);
    return False;
  }

  return True;
}

void OXBattStatPlugin::_UpdateTip() {
  char buffer[100];

  if (!_apmAvailable) {
    SetTip("No APM available");
    return;
  }

  if (_acStatus == 0) {
    sprintf(buffer, "Battery level: %ld%%", _battPercent);
  } else {
    if (_battStatus == 0x03) {
      sprintf(buffer, "Online and charging: %ld%%", _battPercent);
    } else {
      sprintf(buffer, "Online");
    }
  }

  SetTip(buffer);
}

void OXBattStatPlugin::_ParseConfig(OIniFile *rcfile) {
  char arg[256];

  if (rcfile->GetItem("fvwm_action_click", arg)) {
    if (_actionClick) delete[] _actionClick;
    _actionClick = StrDup(arg);
  }

  if (rcfile->GetItem("fvwm_action_doubleclick", arg)) {
    if (_actionDblClick) delete[] _actionDblClick;
    _actionDblClick = StrDup(arg);
  }

  if (rcfile->GetItem("fvwm_action_alert", arg)) {
    if (_actionAlert) delete[] _actionAlert;
    _actionAlert = StrDup(arg);
  }

  if (rcfile->GetItem("update_interval", arg)) {
    _updateInterval = atoi(arg);
    if (_updateInterval < 1) _updateInterval = 1;
  }
}

void OXBattStatPlugin::_DoRedraw() {
  int y;

  OXFrame::_DoRedraw();

  if (_icon[_currentStatus]) {
    y = (_h - _icon[_currentStatus]->GetHeight()) / 2;
    _icon[_currentStatus]->Draw(GetDisplay(), _id, _blackGC, 1, y);
  }
}

ODimension OXBattStatPlugin::GetDefaultSize() const {
  int w, h;

  if (_icon[_currentStatus]) {
    w = _icon[_currentStatus]->GetWidth()+2;
    h = _icon[_currentStatus]->GetHeight();
  } else {
    w = 16 +2;
    h = 16;
  }
  return ODimension(w, h);
}

int OXBattStatPlugin::HandleTimer(OTimer *t) {

  if (t == _timer) {

    if (_GetBattStatus()) {
      int oldStatus = _currentStatus;
      if ((_currentStatus == APM_ALERT) && _actionAlert) {
        OPluginMessage msg(MSG_PLUGIN, PLUGIN_DOACTION, _pluginID,
                           _actionAlert);
        SendMessage(_msgObject, &msg);
      }
      if (oldStatus != _currentStatus) NeedRedraw();
    }

    _UpdateTip();

    delete _timer;
    if (_apmAvailable)
      _timer = new OTimer(this, _updateInterval * 1000);
    else
      _timer = NULL;

    return True;
  }

  return False;
}

int OXBattStatPlugin::HandleButton(XButtonEvent *event) {
  if ((event->type == ButtonPress) && _actionClick) {
    OPluginMessage msg(MSG_PLUGIN, PLUGIN_DOACTION, _pluginID,
                       _actionClick);
    SendMessage(_msgObject, &msg);
  }
  return True;
}

int OXBattStatPlugin::HandleDoubleClick(XButtonEvent *event) {
  if (_actionDblClick) {
    OPluginMessage msg(MSG_PLUGIN, PLUGIN_DOACTION, _pluginID,
                       _actionDblClick);
    SendMessage(_msgObject, &msg);
  }
  return True;
}
