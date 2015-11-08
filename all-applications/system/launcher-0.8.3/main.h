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

#ifndef __MAIN_H
#define __MAIN_H

#include <xclass/utils.h>
#include <xclass/OXClient.h>
#include <xclass/OXMainFrame.h>
#include <xclass/OXPictureButton.h>
#include <xclass/OXIcon.h>
#include <xclass/OXLabel.h>
#include <xclass/OXShutter.h>
#include <xclass/OXMenu.h>

#define XCLAUNCHER_VERSION  "0.8.3"

class OXLaunchButton;


//----------------------------------------------------------------------

struct SGroup {
  SGroup(const char *n, OXCompositeFrame *c, int gid);
  ~SGroup();

  char *name;
  OXCompositeFrame *f;
  int id;
};

struct SButton {
  SButton(const char *n, const char *i, const char *cmd, const char *sdir,
          int bid, SGroup *g, OXLaunchButton *b);
  ~SButton();

  char *name, *icon, *command, *start_dir;
  int  id;
  OXLaunchButton *button;
  SGroup *group;
};

//----------------------------------------------------------------------

class OXButtonContainer : public OXVerticalFrame {
public:
  OXButtonContainer(const OXWindow *p, int ID);
  
  virtual int HandleButton(XButtonEvent *event);

protected:
  int _widgetID;
};


//----------------------------------------------------------------------

class OXMain : public OXMainFrame {
public:
  OXMain(const OXWindow *p, int argc, char *argv[]);
  virtual ~OXMain();

  virtual int ProcessMessage(OMessage *msg);

  //--- menu and button actions:

  void DoShowLabels(int onoff);
  void DoExecCommand(const char *cmd, const char *sdir);
  void DoEditButton(SButton *b);
  void DoAddButton();
  void DoRemoveButton(int bid);
  void DoAddGroup();
  void DoRenameGroup();
  void DoRemoveGroup();
  void DoRenameWindow();

protected:
  void ParseGeometry(const char *gstr);

  void ReadIniFile();
  void SaveIniFile();

  SGroup *FindGroup(const char *name);
  SGroup *GetGroup(const char *name, SGroup *after = NULL);
  OXButton *AddButton(SButton *b, const char *group);
  void RemoveButton(int bid);

  OXPopupMenu *_buttonMenu, *_launcherMenu;

  OXShutter *_shutter;
  OLayoutHints *_l1, *_l2;
  const OPicture *_defaultpic;

  char *_winName, *_defaultDir;
  int _winx, _winy, _winw, _winh;
  int _showLabels;

  std::vector<SGroup *> _groups;
  std::vector<SButton *> _buttons;
};


#endif  // __MAIN_H
