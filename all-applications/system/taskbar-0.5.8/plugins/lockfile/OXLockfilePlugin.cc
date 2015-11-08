#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <xclass/OXClient.h>
#include <xclass/OPicture.h>
#include <xclass/OXFrame.h>
#include <xclass/OTimer.h>
#include <xclass/OXFont.h>
#include <xclass/OGC.h>
#include <xclass/OIniFile.h>

#include "OXLockfilePlugin.h"
#include "../../plugins.h"
#include "../../envvar.h"


#define OFF     0
#define ON      1
#define UNKNOWN 2

OXLockfilePlugin *CreatePlugin(const OXWindow *p, OComponent *c,
                               OIniFile *rcfile, int id) {
  return new OXLockfilePlugin(p, c, rcfile, id);
}

        
OXLockfilePlugin::OXLockfilePlugin(const OXWindow *p, OComponent *c,
                                   OIniFile *rcfile, int id) : 
  OXFrame(p, 16, 16, CHILD_FRAME) {

    _msgObject = c;
    _pluginID = id;

    _iconOn = NULL;
    _iconOff = NULL;

    _actionClick = StrDup("Nop");
    _actionDblClick = NULL;

    _tipOnMsg = NULL;
    _tipOffMsg = NULL;

    _lockfile = NULL;
    _status = UNKNOWN;

    _visible = False;

    _ParseConfig(rcfile);

    // do not call _CheckFile() here, but rather schedule an immediate
    // timer event...
    _timer = new OTimer(this, 1);

    AddInput(ButtonPressMask | ButtonReleaseMask);
}

OXLockfilePlugin::~OXLockfilePlugin() {
  if (_iconOn)  _client->FreePicture(_iconOn);
  if (_iconOff) _client->FreePicture(_iconOff);
  if (_actionClick)    delete[] _actionClick;
  if (_actionDblClick) delete[] _actionDblClick;
  if (_tipOnMsg)       delete[] _tipOnMsg;
  if (_tipOffMsg)      delete[] _tipOffMsg;
}

void OXLockfilePlugin::_ParseConfig(OIniFile *rcfile) {
  char arg[256];

  if (rcfile->GetItem("icon", arg)) {
    if (_iconOn) _client->FreePicture(_iconOn);
    _iconOn = _client->GetPicture(arg);
  }

  if (rcfile->GetItem("icon_on", arg)) {
    if (_iconOn) _client->FreePicture(_iconOn);
    _iconOn = _client->GetPicture(arg);
  }

  if (rcfile->GetItem("icon_off", arg)) {
    if (_iconOff) _client->FreePicture(_iconOff);
    _iconOff = _client->GetPicture(arg);
  }

  if (rcfile->GetItem("tip", arg)) {
    if (_tipOnMsg) delete[] _tipOnMsg;
    _tipOnMsg = StrDup(arg);
  }

  if (rcfile->GetItem("tip_on", arg)) {
    if (_tipOnMsg) delete[] _tipOnMsg;
    _tipOnMsg = StrDup(arg);
  }

  if (rcfile->GetItem("tip_off", arg)) {
    if (_tipOffMsg) delete[] _tipOffMsg;
    _tipOffMsg = StrDup(arg);
  }

  if (rcfile->GetItem("fvwm_action_click", arg)) {
    if (_actionClick) delete[] _actionClick;
    _actionClick = StrDup(arg);
  }

  if (rcfile->GetItem("fvwm_action_doubleclick", arg)) {
    if (_actionDblClick) delete[] _actionDblClick;
    _actionDblClick = StrDup(arg);
  }

  if (rcfile->GetItem("lockfile", arg)) {
    if (_lockfile) delete[] _lockfile;
    _lockfile = envDupExpand(arg);
  }

}


void OXLockfilePlugin::_DoRedraw() {
  const OPicture *icon = (_status == ON) ? _iconOn : _iconOff;

  OXFrame::_DoRedraw();

  if (icon) {
    int y = (_h - icon->GetHeight()) >> 1;
    icon->Draw(GetDisplay(), _id, _blackGC, 1, y);
  }
}

ODimension OXLockfilePlugin::GetDefaultSize() const {
  int w, h;
  const OPicture *icon = (_status == ON) ? _iconOn : _iconOff;

  if (icon) {
    w = icon->GetWidth()+2;
    h = icon->GetHeight();
  } else {
    w = 1;
    h = 16;
  }
  return ODimension(w, h);
}

int OXLockfilePlugin::HandleTimer(OTimer *t) {

  if (t != _timer) return False;

  delete _timer;
  _timer = new OTimer(this, 15000);

  if (_CheckFile()) {
    const OPicture *icon = (_status == ON) ? _iconOn : _iconOff;

    if (icon) {
      if (!_visible) {
        _visible = True;
        OPluginMessage msg(MSG_PLUGIN, PLUGIN_SHOW, _pluginID);
        SendMessage(_msgObject, &msg);
      } else {
        OPluginMessage msg(MSG_PLUGIN, PLUGIN_CHANGED, _pluginID);
        SendMessage(_msgObject, &msg);
      }
      NeedRedraw();
    } else {
      if (_visible) {
        _visible = False;
        OPluginMessage msg(MSG_PLUGIN, PLUGIN_HIDE, _pluginID);
        SendMessage(_msgObject, &msg);
      }
    }

  }

  return True;
}

int OXLockfilePlugin::HandleButton(XButtonEvent *event) {
  if ((event->type == ButtonPress) && _actionClick) {
    OPluginMessage msg(MSG_PLUGIN, PLUGIN_DOACTION, _pluginID,
                       _actionClick);
    SendMessage(_msgObject, &msg);
  }
  return True;
}

int OXLockfilePlugin::HandleDoubleClick(XButtonEvent *event) {
  if (_actionDblClick) {
    OPluginMessage msg(MSG_PLUGIN, PLUGIN_DOACTION, _pluginID,
                       _actionDblClick);
    SendMessage(_msgObject, &msg);
  }
  return True;
}

int OXLockfilePlugin::_CheckFile() {
  int newStatus;

  if (_lockfile) {
    newStatus = (access(_lockfile, F_OK) == 0) ? ON : OFF;
  } else {
    newStatus = OFF;
  }

  if (newStatus != _status) {
    _status = newStatus;
    RemoveTip();
    if (_status == ON) {
      if (_tipOnMsg) SetTip(_tipOnMsg);
    } else {
      if (_tipOffMsg) SetTip(_tipOffMsg);
    }
    return True;
  }

  return False;
}
