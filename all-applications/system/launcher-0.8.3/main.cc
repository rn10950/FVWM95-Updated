/**************************************************************************

    This file is part of xclauncher.
    Copyright (C) 1998-2002 Hector Peraza.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

**************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/stat.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <vector>

#include <xclass/utils.h>
#include <xclass/OXCanvas.h>
#include <xclass/OXMsgBox.h>
#include <xclass/OIniFile.h>
#include <xclass/OResourcePool.h>
#include <xclass/OExec.h>

#include "OXLaunchButton.h"
#include "OXEditDlg.h"
#include "OXRenameDlg.h"

#include "main.h"

#include "default.xpm"

#define MAIN_WIDTH          60
#define MAIN_HEIGHT        500

#define M_BUTTON_EDIT       11
#define M_BUTTON_REMOVE     12

#define M_GROUP_NEW         21
#define M_GROUP_ADDBUTTON   22
#define M_GROUP_REMOVE      23
#define M_GROUP_RENAME      24
#define M_GROUP_SHOWLABELS  25
#define M_GROUP_RENAMEWIN   26
#define M_GROUP_BGNDCOLOR   27
#define M_GROUP_ABOUT       28


//----------------------------------------------------------------------

SGroup::SGroup(const char *n, OXCompositeFrame *c, int gid) {
  name = StrDup(n);
  f = c;
  id = gid;
}

SGroup::~SGroup() {
  delete[] name;
}

SButton::SButton(const char *n, const char *i, const char *cmd,
                 const char *sdir, int bid, SGroup *g, OXLaunchButton *b) {
  name = StrDup(n);
  icon = StrDup(i);
  command = StrDup(cmd);
  start_dir = StrDup(sdir);
  id = bid;
  group = g;
  button = b;
}

SButton::~SButton() {
  delete[] name;
  delete[] icon;
  delete[] command;
  delete[] start_dir;
}


//----------------------------------------------------------------------

// We need this container frame in order to capture mouse clicks
// inside the shutter canvases.

OXButtonContainer::OXButtonContainer(const OXWindow *p, int ID) :
  OXVerticalFrame(p) {

  _widgetID = ID;

  XGrabButton(GetDisplay(), Button3, AnyModifier, _id, False,
              ButtonPressMask | ButtonReleaseMask,
              GrabModeAsync, GrabModeAsync, None, None);
}

int OXButtonContainer::HandleButton(XButtonEvent *event) {
  OXFrame *f = GetFrameFromPoint(event->x, event->y);

  if (f != this) return f->HandleButton(event);

  if (event->button == Button3) {
    if (event->type == ButtonRelease) {
      if (event->x >= 0 && event->x < _w &&
          event->y >= 0 && event->y < _h) {
        OContainerMessage msg(MSG_CONTAINER, MSG_CLICK, _widgetID,
                              event->button, 0, 0,
                              event->x_root, event->y_root);
        SendMessage(_msgObject, &msg);
      }
    }
    return True;
  } 

  return False;
}

//----------------------------------------------------------------------

int main(int argc, char *argv[]) {
  int i;

  OXClient *clientX = new OXClient(argc, argv);

  OXMain *mainWindow = new OXMain(clientX->GetRoot(), argc, argv);
  mainWindow->MapWindow();

  clientX->Run();

  return 0;
}

OXMain::OXMain(const OXWindow *p, int argc, char *argv[]) :
  OXMainFrame(p, MAIN_WIDTH, MAIN_HEIGHT) {

  //--- create button context menu

  _buttonMenu = new OXPopupMenu(_client->GetRoot());
  _buttonMenu->AddEntry(new OHotString("&Edit..."), M_BUTTON_EDIT);
  _buttonMenu->AddSeparator();
  _buttonMenu->AddEntry(new OHotString("&Remove"), M_BUTTON_REMOVE);
  _buttonMenu->Associate(this);

  //--- create shutter context menu

  _launcherMenu = new OXPopupMenu(_client->GetRoot());
  _launcherMenu->AddEntry(new OHotString("Add &button..."), M_GROUP_ADDBUTTON);
  _launcherMenu->AddSeparator();
  _launcherMenu->AddEntry(new OHotString("&Add group..."), M_GROUP_NEW);
  _launcherMenu->AddEntry(new OHotString("&Rename group..."), M_GROUP_RENAME);
  _launcherMenu->AddEntry(new OHotString("Re&move group"), M_GROUP_REMOVE);
  _launcherMenu->AddSeparator();
  _launcherMenu->AddEntry(new OHotString("&Show button labels"), M_GROUP_SHOWLABELS);
  _launcherMenu->AddSeparator();
  _launcherMenu->AddEntry(new OHotString("Rename &window..."), M_GROUP_RENAMEWIN);
  _launcherMenu->Associate(this);

  //--- initialize variables

  _winName = StrDup("Launcher");

  _winx = _winy = -10000;
  _winw = MAIN_WIDTH;
  _winh = MAIN_HEIGHT;

  _showLabels = True;

  _groups.clear();
  _buttons.clear();

  char *sdir = getenv("HOME");
  _defaultDir = StrDup(sdir ? sdir : ".");

  //--- layout hints object for buttons

  _l1 = new OLayoutHints(LHINTS_TOP | LHINTS_CENTER_X, 2, 2, 5, 0);

  _defaultpic = _client->GetPicture("default.xpm", default_xpm);
  if (!_defaultpic) FatalError("Failed to load default pixmap");

  //--- add the shutter widget

  _shutter = new OXShutter(this, 100);

  _l2 = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y);
  AddFrame(_shutter, _l2);

  //--- load options buttons from ini file

  ReadIniFile();

  //--- if no groups/buttons loaded, create a default group.
  //    we need at least an empty shutter tab that allows us
  //    to capture mouse events.

  if (_groups.size() == 0) {
    GetGroup("Default");
    _launcherMenu->DisableEntry(M_GROUP_REMOVE);
  }

  //--- check/uncheck menu entries according to current options

  if (_showLabels)
    _launcherMenu->CheckEntry(M_GROUP_SHOWLABELS);
  else
    _launcherMenu->UnCheckEntry(M_GROUP_SHOWLABELS);

  //--- set the window names

  SetWindowName(_winName);
  SetIconName(_winName);
  SetClassHints("XCLASS", "xclauncher");

  //--- set WM decoration hints: close and minimize buttons only

  SetMWMHints(MWM_DECOR_ALL | MWM_DECOR_RESIZEH | MWM_DECOR_MAXIMIZE |
                              MWM_DECOR_MENU,
              MWM_FUNC_ALL | MWM_FUNC_RESIZE | MWM_FUNC_MAXIMIZE,
              MWM_INPUT_MODELESS);

  //--- parse the geometry string, if one was specified on the command
  //    line, overriding the .ini file settings

  for (int i = 1; argv[i]; ++i) {
    if ((strcmp(argv[i], "-geometry") == 0) ||
        (strcmp(argv[i], "--geometry") == 0) ||
        (strcmp(argv[i], "-g") == 0)) {
      if (argv[i+1]) ParseGeometry(argv[i+1]);
      break;
    }
  }

  //--- move the window to its last-run position

  if (_winx > -10000 && _winy > -10000) {
    Move(_winx, _winy);
    SetWMPosition(_winx, _winy);
  }

  //--- map windows, initialize layout, etc.

  MapSubwindows();
  Resize(_winw, _winh);
  Layout();
}

OXMain::~OXMain() {
  int i;

  //--- save options and buttons

  SaveIniFile();

  delete _l1;
  delete _l2;

  delete[] _winName;
  delete[] _defaultDir;

  //--- delete buttons and groups

  for (i = 0; i < _buttons.size(); ++i) delete _buttons[i];
  for (i = 0; i < _groups.size(); ++i) delete _groups[i];

  delete _buttonMenu;
  delete _launcherMenu;
}

// Parse the window geometry string and setup variables accordingly.

void OXMain::ParseGeometry(const char *gstr) {
  int flags, x, y;
  unsigned int w, h;

  flags = XParseGeometry(gstr, &x, &y, &w, &h);

  if (flags & HeightValue) _winh = h;

  if (flags & WidthValue) _winw = w;

  if (flags & XValue) {
    _winx = x;
    if (flags & XNegative) _winx += _client->GetDisplayWidth() - _winw;
  }

  if (flags & YValue) {
    _winy = y;
    if (flags & YNegative) _winy += _client->GetDisplayHeight() - _winh;
  }

  //--- set the WM gravity hint accordingly

  switch (flags & (XValue | XNegative | YValue | YNegative)) {
  case (XValue | XNegative):
  case (XValue | XNegative | YValue):
    SetWMGravity(NorthEastGravity);
    break;

  case (YValue | YNegative):
  case (YValue | YNegative | XValue):
    SetWMGravity(SouthWestGravity);
    break;

  case (XValue | XNegative | YValue | YNegative):
    SetWMGravity(SouthEastGravity);
    break;

  default:
    SetWMGravity(NorthWestGravity);
    break;
  }
}

// Find a group (shutter tab) with the given name.

SGroup *OXMain::FindGroup(const char *name) {
  int i;

  for (i = 0; i < _groups.size(); ++i)
    if (strcmp(_groups[i]->name, name) == 0) return _groups[i];

  return NULL;
}

// Similar to FindGroup, but creates a new group if no one was
// found with the given name. The 'after' argument, if specified,
// has meaning only when a new group is created: the new group will
// be inserted after the specified by 'after'.

SGroup *OXMain::GetGroup(const char *name, SGroup *after) {
  OXShutterItem *item;
  OXCompositeFrame *container;
  static int gid = 1000;

  for (int i = 0; i < _groups.size(); ++i)
    if (strcmp(_groups[i]->name, name) == 0) return _groups[i];

  //--- no group with such name yet, create a new one...

  item = new OXShutterItem(_shutter, new OHotString(name), gid);
#if 0
  container = (OXCompositeFrame *) item->GetContainer();
#else
  //--- replace shutter's container with our special one

  container = new OXButtonContainer(item->GetCanvas()->GetViewPort(), gid);
  container->Associate(this);
  item->SetContainer(container);
#endif

  SGroup *group = new SGroup(name, container, gid++);

  if (after) {
    _shutter->InsertItem(item, after->id);
    for (int i = 0; i < _groups.size(); ++i) {
      if (_groups[i] == after) {
        _groups.insert(_groups.begin() + i + 1, group);
        break;
      }
    }
  } else {
    _shutter->AddItem(item);
    _groups.push_back(group);
  }

  return group;
}

// Create and add a button to the specified group

OXButton *OXMain::AddButton(SButton *b, const char *grp) {
  SGroup *group;
  OXCompositeFrame *container;
  const OPicture *buttonpic;
  OXLaunchButton *button;
  static int bid = 5000;

  group = GetGroup(grp);
  container = group->f;

  b->group = group;
  b->id = bid++;

  buttonpic = _client->GetPicture(b->icon);
  if (!buttonpic) {
    Debug(DBG_WARN, "Missing pixmap \"%s\", using default.\n", b->icon);
    buttonpic = _defaultpic;
  }

  button = new OXLaunchButton(container, buttonpic,
                              new OHotString(b->name), b->id);
  container->AddFrame(button, _l1);
  button->Associate(this);
#if 0
  button->TakeFocus(False);
#endif
  button->SetTip(b->name);
  button->SetLabelColor(_whitePixel);
  button->ShowLabel(_showLabels);
  b->button = button;

  _buttons.push_back(b);

  return button;
}

// Delete the button with the specified id.

void OXMain::RemoveButton(int bid) {
  SButton *b;
  SGroup *group;
  int i;

  for (i = 0; i < _buttons.size(); ++i) {
    if (_buttons[i]->id == bid) {
      b = _buttons[i];
      _buttons.erase(_buttons.begin() + i);
      group = b->group;
      group->f->RemoveFrame(b->button);   // remove the frame from its parent
      b->button->DestroyWindow();         // destroy the window
      delete b->button;                   // delete the frame object
      delete b;                           // and delete the button structure
      _shutter->Layout();
      return;
    }
  }
}

// Show or hide button labels

void OXMain::DoShowLabels(int onoff) {
  int i;

  if (_showLabels != onoff) {
    _showLabels = onoff;

    for (i = 0; i < _buttons.size(); ++i)
      _buttons[i]->button->ShowLabel(_showLabels);

    _shutter->Layout();

    if (_showLabels)
      _launcherMenu->CheckEntry(M_GROUP_SHOWLABELS);
    else
      _launcherMenu->UnCheckEntry(M_GROUP_SHOWLABELS);
  }
}

// Load buttons and options from ini file

void OXMain::ReadIniFile() {
  char *inipath, line[1024], arg[256];

  inipath = GetResourcePool()->FindIniFile("xclauncher.rc", INI_READ);
  if (!inipath) return;

  OIniFile ini(inipath, INI_READ);

  while (ini.GetNext(line)) {

    if (strcasecmp(line, "defaults") == 0) {
      if (ini.GetItem("window size", arg)) {
        if (sscanf(arg, "%d x %d", &_winw, &_winh) == 2) {
          if (_winw < 10 || _winh > 32000 || _winw < 10 || _winh > 32000) {
            _winw = 600;
            _winh = 400;
          }
        } else {
          _winw = 600;
          _winh = 400;
        }
      }
      if (ini.GetItem("window pos", arg)) {
        if (sscanf(arg, "%d, %d", &_winx, &_winy) != 2) {
          _winx = _winy = -1;
        } else {
          SetWMGravity(StaticGravity);
        }
      }
      if (ini.GetItem("window name", arg)) {
        delete[] _winName;
        _winName = StrDup(arg);
      }
      _showLabels = ini.GetBool("show labels", _showLabels);

    } else if (strcasecmp(line, "groups") == 0) {
      int  i;
      char tmp[50];

      for (i = 0; ; ++i) {
        sprintf(tmp, "group %d", i + 1);
        if (ini.GetItem(tmp, arg)) {
          GetGroup(arg);
        } else {
          break;
        }
      }

    //--- anything else means a button

    } else {
      char name[256], group[256], icon[256], cmd[256], sdir[PATH_MAX];

      if (ini.GetItem("name", name) &&
          ini.GetItem("group", group) &&
          ini.GetItem("icon", icon) &&
          ini.GetItem("command", cmd)) {
        if (!ini.GetItem("start dir", sdir)) strcpy(sdir, _defaultDir);
        SButton *b = new SButton(name, icon, cmd, sdir, 0, NULL, NULL);
        AddButton(b, group);
      }
    }
  }

  delete[] inipath;
}

// Save options and buttons

void OXMain::SaveIniFile() {
  int i;
  char *inipath, tmp[256];
  Window destw;

  inipath = GetResourcePool()->FindIniFile("xclauncher.rc", INI_WRITE);
  if (!inipath) return;

  OIniFile ini(inipath, INI_WRITE);

  ini.PutNext("defaults");
  ini.PutItem("window name", _winName);
  sprintf(tmp, "%d x %d", _w, _h);
  ini.PutItem("window size", tmp);
#if 0
  XTranslateCoordinates(GetDisplay(), _id, _client->GetRoot()->GetId(),
                        0, 0, &_winx, &_winy, &destw);
  sprintf(tmp, "%d, %d", _winx, _winy);
#else
  sprintf(tmp, "%d, %d", _x, _y);
#endif
  ini.PutItem("window pos", tmp);
  ini.PutBool("show labels", _showLabels);
  ini.PutNewLine();

  ini.PutNext("groups");
  for (i = 0; i < _groups.size(); ++i) {
    sprintf(tmp, "group %d", i + 1);
    ini.PutItem(tmp, _groups[i]->name);
  }
  ini.PutNewLine();

  for (i = 0; i < _buttons.size(); ++i) {
    ini.PutNext(_buttons[i]->name);
    ini.PutItem("name", _buttons[i]->name);
    ini.PutItem("group", _buttons[i]->group->name);
    ini.PutItem("icon", _buttons[i]->icon);
    ini.PutItem("command", _buttons[i]->command);
    ini.PutItem("start dir", _buttons[i]->start_dir);
    ini.PutNewLine();
  }

  delete[] inipath;
}

int OXMain::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;
  int i, menu_id;

  switch (msg->type) {
    case MSG_BUTTON:
      if (msg->action == MSG_CLICK) {
        for (i = 0; i < _buttons.size(); ++i) {
          if (_buttons[i]->id == wmsg->id) {
            DoExecCommand(_buttons[i]->command, _buttons[i]->start_dir);
            break;
          }
        }

      } else if (msg->action == MSG_SELECT) {
        OContainerMessage *cmsg = (OContainerMessage *) msg;
        menu_id = _buttonMenu->PopupMenu(cmsg->xroot, cmsg->yroot);
        switch (menu_id) {
          case M_BUTTON_EDIT:
            for (i = 0; i < _buttons.size(); ++i) {
              if (_buttons[i]->id == wmsg->id) {
                DoEditButton(_buttons[i]);
                break;
              }
            }
            break;

          case M_BUTTON_REMOVE:
            DoRemoveButton(wmsg->id);
            break;
        }
      }
      break;

    case MSG_CONTAINER:
      if (msg->action == MSG_CLICK) {
        OContainerMessage *cmsg = (OContainerMessage *) msg;
        if (cmsg->button == Button3) {
          menu_id = _launcherMenu->PopupMenu(cmsg->xroot, cmsg->yroot);
          switch (menu_id) {
            case M_GROUP_ADDBUTTON:
              DoAddButton();
              break;

            case M_GROUP_NEW:
              DoAddGroup();
              break;

            case M_GROUP_RENAME:
              DoRenameGroup();
              break;

            case M_GROUP_REMOVE:
              DoRemoveGroup();
              break;

            case M_GROUP_RENAMEWIN:
              DoRenameWindow();
              break;

            case M_GROUP_SHOWLABELS:
              DoShowLabels(!_showLabels);
              break;
          }
        }
      }
      break;

    case MSG_SHUTTER:
      break;

    default:
      break;
  }

  return True;
}

// Execute command

void OXMain::DoExecCommand(const char *cmd, const char *sdir) {
  int  i, argc;
  char *command, *argptr, *argv[PATH_MAX];

  //--- break the command line into argv components

  command = StrDup(cmd);
  argc = 0;
  argptr = strtok(command, " ");
  while (argptr) {
    argv[argc++] = StrDup(argptr);
    argptr = strtok(NULL, " ");
  }
  argv[argc] = NULL;

  //--- execute the command

  OExec exec(_client, argv[0], argv, sdir, False, True);

  for (i = 0; i < argc; i++) delete[] argv[i];
  delete[] command;
}

void OXMain::DoEditButton(SButton *b) {
  int retc;

  new OXEditButtonDialog(_client->GetRoot(), this, b, &retc);

  if (retc == ID_OK) {
    b->button->SetLabel(new OHotString(b->name));
    const OPicture *pic = _client->GetPicture(b->icon);
    if (!pic) {
      Debug(DBG_WARN, "Missing pixmap \"%s\", using default.\n", pic);
      pic = _defaultpic;
    }
    b->button->SetNormalPic(pic);
    b->button->SetOnPic(pic);
    b->button->SetDisabledPic(pic);
    _shutter->Layout();
    SaveIniFile();
  }
}

void OXMain::DoRemoveButton(int bid) {
  RemoveButton(bid);
  SaveIniFile();
}

void OXMain::DoAddButton() {
  SButton *b;
  int retc;
  OXShutterItem *si;
  OXTextButton *shutterBtn;

  b = new SButton("", "app.s.xpm", "", _defaultDir, 0, NULL, NULL);

  new OXEditButtonDialog(_client->GetRoot(), this, b, &retc);

  if (retc == ID_OK) {
    si = _shutter->GetCurrentTab();
    if (si) {
      shutterBtn = (OXTextButton *) si->GetButton();
      OXButton *btn = AddButton(b, shutterBtn->GetText()->GetString());
      btn->MapWindow();
      _shutter->Layout();
      SaveIniFile();
    }
  } else {
    delete b;
  }
}

void OXMain::DoAddGroup() {
  int retc;
  OString name("");

  new OXRenameDlg(_client->GetRoot(), this,
                  new OString("New group"),
                  new OString("Enter a name for the new group"),
                  new OString("Group name:"),
                  &name, &retc);

  if (retc == ID_OK) {
#if 0
    // no need to check for duplicate names, since
    // GetGroup() will take care if that
    GetGroup(name.GetString());
    _shutter->MapSubwindows();
    _shutter->Layout();
    _launcherMenu->EnableEntry(M_GROUP_REMOVE);
    SaveIniFile();
#else
    SGroup *grp = FindGroup(name.GetString());
    if (grp) {
      _shutter->Select(grp->id);
    } else {
      OXShutterItem *si = _shutter->GetCurrentTab();
      for (int i = 0; i < _groups.size(); ++i) {
        if (_groups[i]->f == si->GetContainer()) {
          grp = _groups[i];
          break;
        }
      }
      grp = GetGroup(name.GetString(), grp);
      _shutter->MapSubwindows();
      _shutter->Layout();
      _launcherMenu->EnableEntry(M_GROUP_REMOVE);
      _shutter->Select(grp->id);
      SaveIniFile();
    }
#endif
  }
}

void OXMain::DoRenameGroup() {
  OXShutterItem *si;
  OXTextButton *shutterBtn;
  int retc;

  si = _shutter->GetCurrentTab();
  if (si) {
    shutterBtn = (OXTextButton *) si->GetButton();
    OString name(shutterBtn->GetText()->GetString());

    new OXRenameDlg(_client->GetRoot(), this,
                    new OString("Rename"),
                    new OString("Enter new group name"),
                    new OString("New name:"),
                    &name, &retc);

    if (retc == ID_OK) {
      shutterBtn->SetText(new OString(&name));
      _shutter->Layout();
      SaveIniFile();
    }
  }
}

void OXMain::DoRemoveGroup() {
  OXShutterItem *si;
  OXTextButton *shutterBtn;
  SGroup *grp;
  int i, retc, empty;

  si = _shutter->GetCurrentTab();
  if (si) {
    shutterBtn = (OXTextButton *) si->GetButton();
    grp = GetGroup(shutterBtn->GetText()->GetString());

    empty = True;

    for (i = 0; i < _buttons.size(); ++i) {
      if (_buttons[i]->group == grp) {
        empty = False;
        break;
      }
    }

    if (!empty) {
      new OXMsgBox(_client->GetRoot(), this,
                   new OString("Remove group"),
                   new OString("The current group is not empty.\n"
                   "Are you sure you want to delete it?"),
                   MB_ICONSTOP, ID_YES | ID_NO, &retc);

      if (retc == ID_YES) empty = True;
    }

    if (empty) {

      for (i = 0; i < _buttons.size(); ++i) {
        if (_buttons[i]->group == grp) {
          RemoveButton(_buttons[i]->id);
          --i;   // since an element was removed from _buttons
        }
      }

      for (i = 0; i < _groups.size(); ++i) {
        if (_groups[i] == grp) {
          _groups.erase(_groups.begin() + i);
          break;
        }
      }

      SetFocusOwner(NULL);
      _shutter->RemoveItem(grp->id);
      SetFocusOwner(_shutter);
      delete grp;

      if (_groups.size() == 0) {
        GetGroup("Default");
        _launcherMenu->DisableEntry(M_GROUP_REMOVE);
      }

      SaveIniFile();
    }

  }
}

void OXMain::DoRenameWindow() {
  int retc;

  OString name(_winName);

  new OXRenameDlg(_client->GetRoot(), this,
                  new OString("Rename"),
                  new OString("Enter new top-level window name"),
                  new OString("New name:"),
                  &name, &retc);

  if (retc == ID_OK) {
    delete[] _winName;
    _winName = StrDup(name.GetString());
    SetWindowName(_winName);
    SetIconName(_winName);
    SaveIniFile();
  }
}
