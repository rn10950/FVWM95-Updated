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

#include "OXMailCheckPlugin.h"
#include "../../plugins.h"
#include "../../envvar.h"

#include "mini-mail.xpm"
#include "mini-newmail.xpm"
#include "mini-unreadmail.xpm"


OXMailCheckPlugin *CreatePlugin(const OXWindow *p, OComponent *c,
                                OIniFile *rcfile, int id) {
  return new OXMailCheckPlugin(p, c, rcfile, id);
}

        
OXMailCheckPlugin::OXMailCheckPlugin(const OXWindow *p, OComponent *c,
                                     OIniFile *rcfile, int id) :
  OXFrame(p, 16, 16, CHILD_FRAME) {

    _msgObject = c;
    _pluginID = id;

    _currentStatus = HAS_NO_MAIL;

    _mailFile = envDupExpand("/var/spool/mail/$USER");
    _mailSize = 0;

    _icon[HAS_NO_MAIL]     = NULL;
    _icon[HAS_MAIL]        = _client->GetPicture("mini-mail.xpm", mini_mail_xpm);
    _icon[HAS_NEW_MAIL]    = _client->GetPicture("mini-newmail.xpm", mini_newmail_xpm);
    _icon[HAS_UNREAD_MAIL] = _client->GetPicture("mini-unreadmail.xpm", mini_unreadmail_xpm);

    _msg[HAS_NO_MAIL]     = StrDup("No mail");
    _msg[HAS_MAIL]        = StrDup("You have mail");
    _msg[HAS_NEW_MAIL]    = StrDup("You have new mail");
    _msg[HAS_UNREAD_MAIL] = StrDup("You have unread mail");

    _actionClick = NULL;
    _actionDblClick = NULL;
    _actionNewMail = NULL;

    _updateInterval = 30;  // in seconds
    _visible = False;

    _ParseConfig(rcfile);

    // Do not call _CheckMail() here, but schedule instead an immediate
    // timer event...
    _timer = new OTimer(this, 1);

    AddInput(ButtonPressMask | ButtonReleaseMask);
}

OXMailCheckPlugin::~OXMailCheckPlugin() {
  delete _timer;
  delete[] _mailFile;
  if (_icon[HAS_NO_MAIL]) _client->FreePicture(_icon[HAS_NO_MAIL]);
  if (_icon[HAS_MAIL]) _client->FreePicture(_icon[HAS_MAIL]);
  if (_icon[HAS_NEW_MAIL]) _client->FreePicture(_icon[HAS_NEW_MAIL]);
  if (_icon[HAS_UNREAD_MAIL]) _client->FreePicture(_icon[HAS_UNREAD_MAIL]);
  delete[] _msg[HAS_NO_MAIL];
  delete[] _msg[HAS_MAIL];
  delete[] _msg[HAS_NEW_MAIL];
  delete[] _msg[HAS_UNREAD_MAIL];
  if (_actionClick) delete[] _actionClick;
  if (_actionDblClick) delete[] _actionDblClick;
  if (_actionNewMail) delete[] _actionNewMail;
}

void OXMailCheckPlugin::_ParseConfig(OIniFile *rcfile) {
  char arg[256];

  if (rcfile->GetItem("mailbox", arg)) {
    delete[] _mailFile;
    _mailFile = envDupExpand(arg);
  }

  if (rcfile->GetItem("update_interval", arg)) {
    _updateInterval = atoi(arg);
    if (_updateInterval < 1) _updateInterval = 1;
  }

  // actions

  if (rcfile->GetItem("fvwm_action_click", arg)) {
    if (_actionClick) delete[] _actionClick;
    _actionClick = StrDup(arg);
  }

  if (rcfile->GetItem("fvwm_action_doubleclick", arg)) {
    if (_actionDblClick) delete[] _actionDblClick;
    _actionDblClick = StrDup(arg);
  }

  if (rcfile->GetItem("fvwm_action_new_mail", arg)) {
    if (_actionNewMail) delete[] _actionNewMail;
    _actionNewMail = StrDup(arg);
  }

  // icons

  if (rcfile->GetItem("no_mail_icon", arg)) {
    if (_icon[HAS_NO_MAIL]) _client->FreePicture(_icon[HAS_NO_MAIL]);
    if (strcasecmp(arg, "None") == 0)
      _icon[HAS_NO_MAIL] = NULL;
    else
      _icon[HAS_NO_MAIL] = _client->GetPicture(arg);
  }

  if (rcfile->GetItem("has_mail_icon", arg)) {
    if (_icon[HAS_MAIL]) _client->FreePicture(_icon[HAS_MAIL]);
    if (strcasecmp(arg, "None") == 0)
      _icon[HAS_MAIL] = NULL;
    else
      _icon[HAS_MAIL] = _client->GetPicture(arg);
  }

  if (rcfile->GetItem("new_mail_icon", arg)) {
    if (_icon[HAS_NEW_MAIL]) _client->FreePicture(_icon[HAS_NEW_MAIL]);
    if (strcasecmp(arg, "None") == 0)
      _icon[HAS_NEW_MAIL] = NULL;
    else
      _icon[HAS_NEW_MAIL] = _client->GetPicture(arg);
  }

  if (rcfile->GetItem("unread_mail_icon", arg)) {
    if (_icon[HAS_UNREAD_MAIL]) _client->FreePicture(_icon[HAS_UNREAD_MAIL]);
    if (strcasecmp(arg, "None") == 0)
      _icon[HAS_UNREAD_MAIL] = NULL;
    else
      _icon[HAS_UNREAD_MAIL] = _client->GetPicture(arg);
  }

  // tooltips

  if (rcfile->GetItem("no_mail_tip", arg)) {
    if (_msg[HAS_NO_MAIL]) delete[] _msg[HAS_NO_MAIL];
    _msg[HAS_NO_MAIL] = StrDup(arg);
  }

  if (rcfile->GetItem("has_mail_tip", arg)) {
    if (_msg[HAS_MAIL]) delete[] _msg[HAS_MAIL];
    _msg[HAS_MAIL] = StrDup(arg);
  }

  if (rcfile->GetItem("new_mail_tip", arg)) {
    if (_msg[HAS_NEW_MAIL]) delete[] _msg[HAS_NEW_MAIL];
    _msg[HAS_NEW_MAIL] = StrDup(arg);
  }

  if (rcfile->GetItem("unread_mail_tip", arg)) {
    if (_msg[HAS_UNREAD_MAIL]) delete[] _msg[HAS_UNREAD_MAIL];
    _msg[HAS_UNREAD_MAIL] = StrDup(arg);
  }

}

void OXMailCheckPlugin::_DoRedraw() {
  int y;

  OXFrame::_DoRedraw();

  if (_icon[_currentStatus]) {
    y = (_h - _icon[_currentStatus]->GetHeight()) >> 1;
    _icon[_currentStatus]->Draw(GetDisplay(), _id, _blackGC, 1, y);
  }
}

ODimension OXMailCheckPlugin::GetDefaultSize() const {
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

int OXMailCheckPlugin::HandleTimer(OTimer *t) {

  if (t != _timer) return False;

  delete _timer;
  _timer = new OTimer(this, _updateInterval * 1000);

  if (_CheckMail()) {

    // request taskbar to hide us if we have no icon defined for
    // for a given status

    if (_icon[_currentStatus]) {
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

int OXMailCheckPlugin::HandleButton(XButtonEvent *event) {
  if ((event->type == ButtonPress) && _actionClick) {
    OPluginMessage msg(MSG_PLUGIN, PLUGIN_DOACTION, _pluginID,
                       _actionClick);
    SendMessage(_msgObject, &msg);
  }
  return True;
}

int OXMailCheckPlugin::HandleDoubleClick(XButtonEvent *event) {
  if (_actionDblClick) {
    OPluginMessage msg(MSG_PLUGIN, PLUGIN_DOACTION, _pluginID,
                       _actionDblClick);
    SendMessage(_msgObject, &msg);
  }
  return True;
}

int OXMailCheckPlugin::_CheckMail() {
  char   newStatus;
  off_t  newSize;
  struct stat st;
  int    fd;

  fd = open(_mailFile, O_RDONLY, 0);
  if (fd < 0) {
    newStatus = HAS_NO_MAIL;
    newSize = 0;
  } else {
    fstat(fd, &st);
    close(fd);
    newSize = st.st_size;
    if (newSize > 0) {
      newStatus = HAS_MAIL;
      if (st.st_mtime >= st.st_atime) {
        newStatus = HAS_UNREAD_MAIL;
        if (newSize > _mailSize) {
          newStatus = HAS_NEW_MAIL;
        }
      }
    }
  }

  _mailSize = newSize;

  if (newStatus != _currentStatus) {
    _currentStatus = newStatus;
    RemoveTip();
    if (_msg[_currentStatus]) SetTip(_msg[_currentStatus]);
    return True;
  }
  return False;
}
