/**************************************************************************

    This file is part of foxftp, a ftp client for fOX.
    Copyright (C) 1998, 1999, G. Matzka.

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

class OXStatusBar;
class OXSiteView;
class OXFtpManager;
class OXFtpConfig;
class OXToolBar;

#include <xclass/OXMainFrame.h>
#include <xclass/OXMdi.h>


#define FOXFTP_VERSION  "0.5.6"


#define M_FILE_NEW		1
#define M_FILE_OPEN		2
#define M_FILE_OPEN_WITH	3
#define M_FILE_QUICK		4
#define M_FILE_CONNECT		5
#define M_FILE_RECONNECT	6
#define M_FILE_DISCONNECT	7
#define M_FILE_CLOSE		8
#define M_FILE_CWD		9
#define M_FILE_RENAME		10
#define M_FILE_PROPS		11
#define M_FILE_DOWNLOAD		12
#define M_FILE_UPLOAD		13
#define M_FILE_EXIT		14

#define M_EDIT_DELETE		21
#define M_EDIT_COPY		22
#define M_EDIT_PASTE		23
#define M_EDIT_SELECT_ALL	24
#define M_EDIT_INVERT_SELECT	25

#define M_VIEW_TOOLBAR		31
#define M_VIEW_STATUSBAR	32
#define M_VIEW_SHOW_LABELS      33
#define M_VIEW_LARGE		34
#define M_VIEW_SMALL		35
#define M_VIEW_LIST		36
#define M_VIEW_DETAIL		37
#define M_VIEW_DOT_FILES	38
#define M_VIEW_STOP		39
#define M_VIEW_REFRESH		40
#define M_VIEW_ARRANGE		41

#define M_VIEW_ARRANGE_BYNAME	51
#define M_VIEW_ARRANGE_BYTYPE	52
#define M_VIEW_ARRANGE_BYSIZE	53
#define M_VIEW_ARRANGE_BYDATE	54

#define M_UP_ONE_LEVEL		61

#define M_OPTIONS_SESSION	71
#define M_OPTIONS_GLOBAL	72

#define M_WINDOW_CASCADE	81
#define M_WINDOW_HORIZONTALLY	82
#define M_WINDOW_VERTICALLY	83
#define M_WINDOW_ICONS          84

#define M_HELP_CONTENTS		91
#define M_HELP_SEARCH		92
#define M_HELP_ABOUT		93

#define MENU_DISABLED     (1<<0)
#define MENU_CHECKED      (1<<1)
#define MENU_RCHECKED     (1<<2)

//----- Popup stuff...
struct _popup {
  OXPopupMenu *ptr;
  struct {
    char *name;
    int  id, state;
    struct _popup *popup_ref;
  } popup[20];
};


//---------------------------------------------------------------------

class OXFtpMain : public OXMainFrame  {
public:
  OXFtpMain(const OXWindow *p, char *homepath, int w, int h);
  virtual ~OXFtpMain();

  virtual int ProcessMessage(OMessage *msg);
  virtual int HandleTimer(OTimer *t);

protected:
  void DoQuickConnect();
  void DoConnect(OSiteConfig *siteConfig);
  void DoReConnect();
  void DoDisConnect();
  void DoSetDirectory();
  void DoStartFtp(OXSiteView *from, OXSiteView *to);

  void SetViewByMode(int new_mode);
  void SetSortByMode(int new_mode);
  void SetViewById(int id, OXSiteView *site);
  void SetSortById(int id, OXSiteView *site);
  void SetControls();

  OXPopupMenu *_MakePopup(struct _popup *);

  char *_homepath;
  bool _showLabels;
  OTimer *_clean_timer;
  const OPicture *_bmpic;
  OGeneralConfig *_genConfig;
  OSiteConfig *_quickConfig;

  OXMdiMenuBar *_menuBar;
  OXMdiMainFrame *_mainFrame;

  OXHorizontal3dLine *_toolBarSep;
  OXToolBar *_toolBar;
  OXStatusBar *_statusBar;

  OXFtpManager *_manager;
  OXFtpConfig *_config;

  OLayoutHints *_l1, *_l2, *_l3, *_l4, *_l5;
  OXPopupMenu *_menuFile, *_menuEdit, *_menuView, *_menuSort;
  OXPopupMenu *_menuOptions, *_menuWindow, *_menuHelp;

  OXSiteView *_local, *_remote;
};
