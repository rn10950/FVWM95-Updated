/**************************************************************************

    This file is part of explorer95, a file manager for fvwm95.
    Copyright (C) 1996, 1997 David Barth, Hector Peraza.

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

#ifndef __OXEXPLORER_H
#define __OXEXPLORER_H

#include <X11/cursorfont.h>

#include <xclass/OXClient.h>
#include <xclass/OXWindow.h>
#include <xclass/OXMainFrame.h>
#include <xclass/OXToolBar.h>
#include <xclass/OXStatusBar.h>
#include <xclass/OX3dLines.h>
#include <xclass/OXMenu.h>
#include <xclass/OXLabel.h>
#include <xclass/OString.h>
#include <xclass/OPicture.h>
#include <xclass/OXFileList.h>
#include <xclass/OXFSDDListBox.h>
#include <xclass/utils.h>

#include "ORecycledFiles.h"
#include "OXDirectoryTree.h"


#define EXPLORER_VERSION        "0.6.8"


//----- Operation modes:

#define EXPLORER_MODE   0
#define FILE_MGR_MODE   1


//----- Menu IDs

#define M_FILE_OPEN             101
#define M_FILE_OPENWITH         102
#define M_FILE_EXPLORE          103
#define M_FILE_FIND             104
#define M_FILE_NEW		105
#define M_FILE_NEWFOLDER	    111
#define M_FILE_NEWSHORTCUT	    112
#define M_FILE_DELETE		106
#define M_FILE_RENAME		107
#define M_FILE_PROPS		108
#define M_FILE_CLOSE		109

#define M_EDIT_UNDO		201
#define M_EDIT_CUT		202
#define M_EDIT_COPY		203
#define M_EDIT_PASTE		204
#define M_EDIT_PASTESHORTCUT	205
#define M_EDIT_SELECTALL	206
#define M_EDIT_INVSELECTION	207

#define M_VIEW_TOOLBAR		301
#define M_VIEW_STATUS		302
#define M_VIEW_LARGEICONS	303
#define M_VIEW_SMALLICONS	304
#define M_VIEW_LIST		305
#define M_VIEW_DETAILS		306
#define M_VIEW_ARRANGE_BYNAME	    311
#define M_VIEW_ARRANGE_BYTYPE	    312
#define M_VIEW_ARRANGE_BYSIZE	    313
#define M_VIEW_ARRANGE_BYDATE	    314
#define M_VIEW_ARRANGE_AUTO	    315
#define M_VIEW_LINEUP           307
#define M_VIEW_REFRESH          308
#define M_VIEW_OPTIONS          309

#define M_HELP_CONTENTS		401
#define M_HELP_SEARCH		402
#define M_HELP_ABOUT		403

#define M_DND_MOVE              501
#define M_DND_COPY              502
#define M_DND_SYMLINK           503


//----- Struct and #defines for default icons

// These are just indexes to default_icon[], see main.cc

#define ICON_FOLDER      0
#define ICON_APP         1
#define ICON_DOC         2
#define ICON_LINK        3

#define ICON_BIG         0
#define ICON_SMALL       1

struct _default_icon {
  char  *picname_prefix;
  const OPicture *icon[2];
};


//----- Context menu modes

#define CONTEXT_MENU_NONE     0
#define CONTEXT_MENU_FILE     1
#define CONTEXT_MENU_DND      2
#define CONTEXT_MENU_RECYCLE  3


//---------------------------------------------------------------------

class OXExplorer : public OXMainFrame {
public:
  OXExplorer(const OXWindow *p, const char *startDir, int mode);
  virtual ~OXExplorer();

  virtual int HandleMapNotify(XMapEvent *event);
  virtual int ProcessMessage(OMessage *msg);

  void ReadIniFile();
  void SaveIniFile();
  void DisplayTotal(int total, int selected);
  void SetViewMode(int new_mode, int force = False);
  void SetSortMode(int new_mode);
  void SetTitle();
  void UpdateListBox();
  void UpdateTree();
  void SetupContextMenu(int mode, int ftype);
  void DoOpen(const OFileItem *f);
  void DoOpenWith(const OFileItem *f);
  void DoExplore(const OFileItem *f);
  void DoChangeDirectory(const char *path);
  void CopyFile(const char *from, const char *to);
  void MoveFile(const char *from, const char *to);
  void SymlinkFile(const char *from, const char *to);
  void DeleteFiles();
  void DoToggleToolBar();
  void DoToggleStatusBar();
  void About();

  void DisplayDir() { _fileWindow->DisplayDirectory(); }

  OXFileList *GetFileWindow() const { return _fileWindow; }
  
  int GetMainMode() const { return _mainMode; }

protected:
  OXHorizontal3dLine *_toolBarSep;
  OXToolBar *_toolBar;
  OXFileSystemDDListBox *_ddlb;
  OXStatusBar *_statusBar;
  OXVerticalFrame *_v1, *_v2;
  OXLabel *_lbl1, *_lbl2;
  OXHorizontalFrame *_hf;
  OXCompositeFrame *_treeHdr, *_listHdr;
  OXFileList *_fileWindow;
  OXDirectoryTree *_dt;

  OLayoutHints *_menuBarLayout, *_menuBarItemLayout;

  OXMenuBar *_menuBar;
  OXPopupMenu *_menuFile, *_menuEdit, *_menuView, *_menuTools,
              *_menuHelp;
  OXPopupMenu *_newMenu, *_sortMenu, *_findMenu;
  OXPopupMenu *_contextMenu, *_sendToMenu, *_dndMenu;

  char *_rcfilename;
  char _currentDir[PATH_MAX];
  char _startDir[PATH_MAX];
  
  ORecycledFiles *_recycled;
  
  int _ww, _wh;
  int _showStatusBar, _showToolBar, _viewMode, _sortMode, _mainMode;

  Atom *_dndTypeList;
};


#endif  // __OXEXPLORER_H
