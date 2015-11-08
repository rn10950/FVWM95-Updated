/**************************************************************************

   This file is part of cdplay, a cdplayer for fvwm95.
   Copyright (C) 1997 Rodolphe Suescun.

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

#include <xclass/OXClient.h>
#include <xclass/OXFont.h>
#include <xclass/OXWindow.h>
#include <xclass/OXFrame.h>
#include <xclass/OXButton.h>
#include <xclass/OXMenu.h>
#include <xclass/OXMsgBox.h>
#include <xclass/OXSlider.h>
#include <xclass/OX3dLines.h>
#include <xclass/OString.h>
#include <xclass/OXToolBar.h>

#include "OMedium.h"
#include "playlist.h"

#define M_MODIF_SELECT     1
#define M_EXIT             2

#define M_VIEW_TOOLBAR     3
#define M_VIEW_INFO        4
#define M_VIEW_STATUSBAR   5
#define M_TRACK_ELAPSED    6
#define M_TRACK_REMAINING  7
#define M_DISK_REMAINING   8
#define M_VOL_CONTROL      9

#define M_PREFERENCES      10

#define M_HELP_CONTENTS    11
#define M_HELP_SEARCH      12
#define M_HELP_ABOUT       13

#define M_RANDOM           14
#define M_CONTINUE         15
#define M_INTRO            16

#define CDPLAY		   20
#define CDPAUSE		   21
#define CDSTOP		   22
#define CDEJECT		   23
#define CDPREVIOUS	   24
#define CDBACKWARD	   25
#define CDFORWARD	   26
#define CDNEXT		   27


//---------------------------------------------------------------------

class OXCDPlayerMainFrame : public OXMainFrame {
public:
  OXCDPlayerMainFrame(const OXWindow *p, int w, int h);
  virtual ~OXCDPlayerMainFrame();

  void SetViewMode(int mode);
  void SetPlayMode(int mode);

  virtual int ProcessMessage(OMessage *msg);

  int HandleTimer(OTimer *t);

protected:
  void RedrawDisplay();
  int  RedrawStatusBar();

  int blink, ViewMode;

  const OXFont *font;

  OMedium *omedium;

  OLayoutHints *_menuBarLayout, *_menuBarItemLayout,
               *lsblayout, *rsblayout, *sllayout;

  OXCompositeFrame *_status, *_main, *_display, *_buttons;

  OXHorizontal3dLine *_toolBarSep;
  OXToolBar *_toolBar, *_buttons1, *_buttons2;

  OXMenuBar *_menuBar;
  OXPopupMenu *_menuDisk, *_menuDisplay, *_menuOptions, *_menuHelp;

  OXLabel *TimeLabel;
  OXHSlider *_volume;
  OTimer *timer;

  OXCompositeFrame *_lstat, *_rstat;
  OXLabel *_lstatus, *_rstatus;
};
