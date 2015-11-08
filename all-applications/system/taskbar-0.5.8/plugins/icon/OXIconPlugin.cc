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

#include "OXIconPlugin.h"
#include "../../plugins.h"

#include "mini-question.xpm"


OXIconPlugin *CreatePlugin(const OXWindow *p, OComponent *c,
                           OIniFile *rcfile, int id) {
  return new OXIconPlugin(p, c, rcfile, id);
}

        
OXIconPlugin::OXIconPlugin(const OXWindow *p, OComponent *c,
                           OIniFile *rcfile, int id) : 
  OXFrame(p, 16, 16, CHILD_FRAME) {

    _msgObject = c;
    _pluginID = id;

    _icon = _client->GetPicture("mini-question.xpm", mini_question_xpm);

    _actionClick = StrDup("Nop");
    _actionDblClick = NULL;

    _tipMsg = NULL;

    _ParseConfig(rcfile);
    if (_tipMsg) SetTip(_tipMsg);

    AddInput(ButtonPressMask | ButtonReleaseMask);
}

OXIconPlugin::~OXIconPlugin() {
  if (_icon) _client->FreePicture(_icon);
  if (_actionClick) delete[] _actionClick;
  if (_actionDblClick) delete[] _actionDblClick;
  if (_tipMsg) delete[] _tipMsg;
}

void OXIconPlugin::_ParseConfig(OIniFile *rcfile) {
  char arg[256];

  if (rcfile->GetItem("icon", arg)) {
    if (_icon) _client->FreePicture(_icon);
    _icon = _client->GetPicture(arg);
  }

  if (rcfile->GetItem("fvwm_action_click", arg)) {
    if (_actionClick) delete[] _actionClick;
    _actionClick = StrDup(arg);
  }

  if (rcfile->GetItem("fvwm_action_doubleclick", arg)) {
    if (_actionDblClick) delete[] _actionDblClick;
    _actionDblClick = StrDup(arg);
  }

  if (rcfile->GetItem("tip", arg)) {
    if (_tipMsg) delete[] _tipMsg;
    _tipMsg = StrDup(arg);
  }
}


void OXIconPlugin::_DoRedraw() {
  int y;

  OXFrame::_DoRedraw();

  if (_icon) {
    y = (_h - _icon->GetHeight()) >> 1;
    _icon->Draw(GetDisplay(), _id, _blackGC, 1, y);
  }
}

ODimension OXIconPlugin::GetDefaultSize() const {
  int w, h;

  if (_icon) {
    w = _icon->GetWidth()+2;
    h = _icon->GetHeight();
  } else {
    w = 2;
    h = 16;
  }
  return ODimension(w, h);
}

int OXIconPlugin::HandleButton(XButtonEvent *event) {
  if ((event->type == ButtonPress) && _actionClick) {
    OPluginMessage msg(MSG_PLUGIN, PLUGIN_DOACTION, _pluginID,
                       _actionClick);
    SendMessage(_msgObject, &msg);
  }
  return True;
}

int OXIconPlugin::HandleDoubleClick(XButtonEvent *event) {
  if (_actionDblClick) {
    OPluginMessage msg(MSG_PLUGIN, PLUGIN_DOACTION, _pluginID,
                       _actionDblClick);
    SendMessage(_msgObject, &msg);
  }
  return True;
}
