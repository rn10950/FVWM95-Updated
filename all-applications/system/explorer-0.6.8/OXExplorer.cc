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

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <time.h>
#include <grp.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <dirent.h>

#include <xclass/utils.h>
#include <xclass/version.h>
#include <xclass/OIniFile.h>
#include <xclass/OMimeTypes.h>
#include <xclass/OString.h>
#include <xclass/OPicture.h>
#include <xclass/OResourcePool.h>
#include <xclass/OXSList.h>
#include <xclass/OXClient.h>
#include <xclass/OXWindow.h>
#include <xclass/OXMainFrame.h>
#include <xclass/OXMenu.h>
#include <xclass/OXMsgBox.h>
#include <xclass/OXResizer.h>
#include <xclass/OXFileDialog.h>
#include <xclass/OXPropertiesDialog.h>
#include <xclass/OXAboutDialog.h>
#include <xclass/ODNDmanager.h>
#include <xclass/OExec.h>

#include "URL.h"
#include "OXOptions.h"
#include "OXOpenWith.h"
#include "ORecycledFiles.h"
#include "OXCopyBox.h"
#include "OXExplorer.h"


//-------------------------------------------------------------------

extern char *AppName, *AppPath;

extern OMimeTypes *MimeTypeList;

int DispFullPath = False;
int NewBrowser   = False;

struct _default_icon default_icon[] = {
  { "folder",   NULL, NULL },
  { "app",      NULL, NULL },
  { "doc",      NULL, NULL },
  { "slink",    NULL, NULL },
  { NULL,       NULL, NULL }
};

SToolBarData tb_data[] = {
  { "tb-uplevel.xpm",  NULL, "Up one level",        BUTTON_NORMAL,  99, NULL  },
  { "",                NULL, "",                    0,             -10, NULL },
  { "tb-mapntw.xpm",   NULL, "Mount file system",   BUTTON_NORMAL, -10, NULL },
  { "tb-dscntw.xpm",   NULL, "Unmount file system", BUTTON_NORMAL, -10, NULL },
  { "",                NULL, "",                    0,             -10, NULL },
  { "tb-cut.xpm",      NULL, "Cut",                 BUTTON_NORMAL, -10, NULL },
  { "tb-copy.xpm",     NULL, "Copy",                BUTTON_NORMAL, -10, NULL },
  { "tb-paste.xpm",    NULL, "Paste",               BUTTON_NORMAL, -10, NULL },
  { "",                NULL, "",                    0,             -10, NULL },
  { "tb-undo.xpm",     NULL, "Undo",                BUTTON_NORMAL, -10, NULL },
  { "",                NULL, "",                    0,             -10, NULL },
  { "tb-delete.xpm",   NULL, "Delete",              BUTTON_NORMAL, M_FILE_DELETE,     NULL },
  { "tb-prop.xpm",     NULL, "Properties",          BUTTON_NORMAL, M_FILE_PROPS,      NULL },
  { "",                NULL, "",                    0,             -10, NULL },
  { "tb-bigicons.xpm", NULL, "Large icons",         BUTTON_STAYDOWN, M_VIEW_LARGEICONS, NULL },
  { "tb-smicons.xpm",  NULL, "Small icons",         BUTTON_STAYDOWN, M_VIEW_SMALLICONS, NULL },
  { "tb-list.xpm",     NULL, "List",                BUTTON_STAYDOWN, M_VIEW_LIST,       NULL },
  { "tb-details.xpm",  NULL, "Details",             BUTTON_STAYDOWN, M_VIEW_DETAILS,    NULL },
  { NULL,              NULL, NULL,                  0,             -10, NULL }
};


//----------------------------------------------------------------------

static void remove_suffix(char *name, char *suffix = NULL) {
  char *np = name + strlen(name);
  char *sp = suffix + strlen(suffix);

  while (np > name && sp > suffix) if (*--np != *--sp) return;

  if (np > name) *np = '\0';
}

void strip_trailing_slashes(char *path) {
  int last = strlen(path) - 1;
  while (last > 0 && path[last] == '/')	path[last--] = '\0';
}

char *basename(char *name, char *suffix = NULL) {
  strip_trailing_slashes(name);
  char *base = strrchr(name, '/');
  if (base) base++; else base = name;
  if (suffix) remove_suffix(base, suffix);

  return base;
}

char *dirname(char *name) {
  strip_trailing_slashes(name);
  char *line = strrchr(name, '/');
  if (line) {
    while (line > name && *line == '/') --line;
    line[1] = 0;
  } else {
    name[0] = '.';
    name[1] = 0;
  }

  return name;
}



//----------------------------------------------------------------------

OXExplorer::OXExplorer(const OXWindow *p, const char *startDir, int mainMode) :
  OXMainFrame(p, 400, 200) {
  char tmp[BUFSIZ];

  _ww = 600;
  _wh = 360;

  _dndTypeList = new Atom[2];

  _dndTypeList[0] = XInternAtom(GetDisplay(), "text/uri-list", False);
  _dndTypeList[1] = None;

  _dndManager = new ODNDmanager(_client, this, _dndTypeList);

  _showToolBar = False;
  _showStatusBar = True;
  _viewMode = M_VIEW_LARGEICONS;
  _sortMode = M_VIEW_ARRANGE_BYNAME;
  _mainMode = mainMode;

  //-------------- load default icons (shouldn't we #include them instead?)

  for (int i=0; default_icon[i].picname_prefix != NULL; ++i) {
    char *name;

    name = new char[20];
    sprintf(name, "%s.s.xpm", default_icon[i].picname_prefix);
    if ((default_icon[i].icon[0] = _client->GetPicture(name)) == NULL)
      FatalError("Pixmap not found: %s", name);

    name = new char[20];
    sprintf(name, "%s.t.xpm", default_icon[i].picname_prefix);
    if ((default_icon[i].icon[1] = _client->GetPicture(name)) == NULL)
      FatalError("Pixmap not found: %s", name);
  }

  //-------------- create menu bar and popup menus

  _menuBarLayout = new OLayoutHints(LHINTS_TOP | LHINTS_LEFT | LHINTS_EXPAND_X, 0, 0, 1, 1);
  _menuBarItemLayout = new OLayoutHints(LHINTS_TOP | LHINTS_LEFT, 0, 4, 0, 0);

  _sendToMenu = new OXPopupMenu(_client->GetRoot());
  _sendToMenu->AddEntry(new OHotString("Fax Recipient"), 21, _client->GetPicture("fax.t.xpm"));
  _sendToMenu->AddEntry(new OHotString("Mail Recipient"), 22, _client->GetPicture("mail.t.xpm"));
  _sendToMenu->AddEntry(new OHotString("My Briefcase"), 23, _client->GetPicture("briefcase.t.xpm"));

  _contextMenu = new OXPopupMenu(_client->GetRoot());
  _contextMenu->AddEntry(new OHotString("&Open"), M_FILE_OPEN);
  _contextMenu->AddEntry(new OHotString("&Explore"), M_FILE_EXPLORE);
  _contextMenu->AddEntry(new OHotString("&Find..."), M_FILE_FIND);
  _contextMenu->AddSeparator();
  _contextMenu->AddPopup(new OHotString("Se&nd To"), _sendToMenu);
  _contextMenu->AddSeparator();
  _contextMenu->AddEntry(new OHotString("Cu&t"), M_EDIT_CUT);
  _contextMenu->AddEntry(new OHotString("&Copy"), M_EDIT_COPY);
  _contextMenu->AddEntry(new OHotString("&Paste"), M_EDIT_PASTE);
  _contextMenu->AddSeparator();
  _contextMenu->AddEntry(new OHotString("Create &Soft Link"), M_FILE_NEWSHORTCUT);
  _contextMenu->AddEntry(new OHotString("&Delete"), M_FILE_DELETE);
  _contextMenu->AddEntry(new OHotString("Rena&me"), M_FILE_RENAME);
  _contextMenu->AddSeparator();
  _contextMenu->AddEntry(new OHotString("P&roperties..."), M_FILE_PROPS);
  _contextMenu->SetDefaultEntry(M_FILE_OPEN);

  _sendToMenu->Associate(this);
  _contextMenu->Associate(this);

  _newMenu = new OXPopupMenu(_client->GetRoot());
  _newMenu->AddEntry(new OHotString("&Folder"), M_FILE_NEWFOLDER);
  _newMenu->AddEntry(new OHotString("&Soft Link"), M_FILE_NEWSHORTCUT);
  _newMenu->AddSeparator();
  _newMenu->AddEntry(new OHotString("Bitmap Image"), 113);
  _newMenu->AddEntry(new OHotString("Text Document"), 113);
  _newMenu->AddEntry(new OHotString("Briefcase"), 113);

  _menuFile = new OXPopupMenu(_client->GetRoot());
  _menuFile->AddEntry(new OHotString("&Open"), M_FILE_OPEN);
  _menuFile->AddEntry(new OHotString("&Explore"), M_FILE_EXPLORE);
  _menuFile->AddEntry(new OHotString("&Find..."), M_FILE_FIND);
  _menuFile->AddSeparator();
  _menuFile->AddPopup(new OHotString("Se&nd To"), _sendToMenu);
  _menuFile->AddSeparator();
  _menuFile->AddPopup(new OHotString("Ne&w"), _newMenu);
  _menuFile->AddSeparator();
  _menuFile->AddEntry(new OHotString("Create &Soft Link"), M_FILE_NEWSHORTCUT);
  _menuFile->AddEntry(new OHotString("&Delete"), M_FILE_DELETE);
  _menuFile->AddEntry(new OHotString("Rena&me"), M_FILE_RENAME);
  _menuFile->AddEntry(new OHotString("P&roperties..."), M_FILE_PROPS);
  _menuFile->AddSeparator();
  _menuFile->AddEntry(new OHotString("&Close"), M_FILE_CLOSE);
  _menuFile->SetDefaultEntry(M_FILE_OPEN);

  _newMenu->DisableEntry(M_FILE_NEWSHORTCUT);
  _menuFile->DisableEntry(M_FILE_NEWSHORTCUT);
  _menuFile->DisableEntry(M_FILE_DELETE);
  _menuFile->DisableEntry(M_FILE_RENAME);
  _menuFile->DisableEntry(M_FILE_PROPS);

  _menuEdit = new OXPopupMenu(_client->GetRoot());
  _menuEdit->AddEntry(new OHotString("&Undo"), M_EDIT_UNDO);
  _menuEdit->AddSeparator();
  _menuEdit->AddEntry(new OHotString("Cu&t"), M_EDIT_CUT);
  _menuEdit->AddEntry(new OHotString("&Copy"), M_EDIT_COPY);
  _menuEdit->AddEntry(new OHotString("&Paste"), M_EDIT_PASTE);
  _menuEdit->AddEntry(new OHotString("Paste &Soft Link"), M_EDIT_PASTESHORTCUT);
  _menuEdit->AddSeparator();
  _menuEdit->AddEntry(new OHotString("Select &All"), M_EDIT_SELECTALL);
  _menuEdit->AddEntry(new OHotString("&Invert Selection"), M_EDIT_INVSELECTION);
//_menuEdit->SetDefaultEntry(M_EDIT_INVSELECTION);
  _menuEdit->DisableEntry(M_EDIT_UNDO);
  _menuEdit->DisableEntry(M_EDIT_CUT);
  _menuEdit->DisableEntry(M_EDIT_COPY);
  _menuEdit->DisableEntry(M_EDIT_PASTE);
  _menuEdit->DisableEntry(M_EDIT_PASTESHORTCUT);

  _sortMenu = new OXPopupMenu(_client->GetRoot());
  _sortMenu->AddEntry(new OHotString("By &Name"), M_VIEW_ARRANGE_BYNAME);
  _sortMenu->AddEntry(new OHotString("By &Type"), M_VIEW_ARRANGE_BYTYPE);
  _sortMenu->AddEntry(new OHotString("By &Size"), M_VIEW_ARRANGE_BYSIZE);
  _sortMenu->AddEntry(new OHotString("By &Date"), M_VIEW_ARRANGE_BYDATE);
  _sortMenu->AddSeparator();
  _sortMenu->AddEntry(new OHotString("&Auto Arrange"), M_VIEW_ARRANGE_AUTO);
  _sortMenu->CheckEntry(M_VIEW_ARRANGE_AUTO);

  _menuView = new OXPopupMenu(_client->GetRoot());
  _menuView->AddEntry(new OHotString("&Toolbar"), M_VIEW_TOOLBAR);
  _menuView->AddEntry(new OHotString("Status &Bar"), M_VIEW_STATUS);
  _menuView->AddSeparator();
  _menuView->AddEntry(new OHotString("Lar&ge Icons"), M_VIEW_LARGEICONS);
  _menuView->AddEntry(new OHotString("S&mall Icons"), M_VIEW_SMALLICONS);
  _menuView->AddEntry(new OHotString("&List"), M_VIEW_LIST);
  _menuView->AddEntry(new OHotString("&Details"), M_VIEW_DETAILS);
  _menuView->AddSeparator();
  _menuView->AddPopup(new OHotString("Arrange &Icons"), _sortMenu);
  _menuView->AddEntry(new OHotString("Lin&e up Icons"), M_VIEW_LINEUP);
  _menuView->AddSeparator();
  _menuView->AddEntry(new OHotString("&Refresh"), M_VIEW_REFRESH);
  _menuView->AddEntry(new OHotString("&Options..."), M_VIEW_OPTIONS);
  _menuView->CheckEntry(M_VIEW_TOOLBAR);
  _menuView->CheckEntry(M_VIEW_STATUS);

  if (_mainMode == EXPLORER_MODE) {
    _findMenu = new OXPopupMenu(_client->GetRoot());
    _findMenu->AddEntry(new OHotString("&Files or Folders..."), 1001);
    _findMenu->AddEntry(new OHotString("&Computer..."), 1002);

    _menuTools = new OXPopupMenu(_client->GetRoot());
    _menuTools->AddPopup(new OHotString("&Find"), _findMenu);
    _menuTools->AddSeparator();
    _menuTools->AddEntry(new OHotString("&Mount file system..."), 1003);
    _menuTools->AddEntry(new OHotString("&Dismount file system..."), 1004);
    _menuTools->AddSeparator();
    _menuTools->AddEntry(new OHotString("&Go to..."), 1005);
  }

  _menuHelp = new OXPopupMenu(_client->GetRoot());
  _menuHelp->AddEntry(new OHotString("&Contents"), M_HELP_CONTENTS);
  _menuHelp->AddEntry(new OHotString("&Search..."), M_HELP_SEARCH);
  _menuHelp->AddSeparator();
  _menuHelp->AddEntry(new OHotString("&About"), M_HELP_ABOUT);

  _menuFile->Associate(this);
  _menuEdit->Associate(this);
  _menuView->Associate(this);
  _menuHelp->Associate(this);
  _sortMenu->Associate(this);

  _menuBar = new OXMenuBar(this, 1, 1, HORIZONTAL_FRAME);
  _menuBar->AddPopup(new OHotString("&File"), _menuFile, _menuBarItemLayout);
  _menuBar->AddPopup(new OHotString("&Edit"), _menuEdit, _menuBarItemLayout);
  _menuBar->AddPopup(new OHotString("&View"), _menuView, _menuBarItemLayout);
  if (_mainMode == EXPLORER_MODE)
    _menuBar->AddPopup(new OHotString("&Tools"), _menuTools, _menuBarItemLayout);
  _menuBar->AddPopup(new OHotString("&Help"), _menuHelp, _menuBarItemLayout);

  AddFrame(_menuBar, _menuBarLayout);

  _dndMenu = new OXPopupMenu(_client->GetRoot());
  _dndMenu->AddEntry(new OHotString("&Move here"), M_DND_MOVE);
  _dndMenu->AddEntry(new OHotString("&Copy here"), M_DND_COPY);
  _dndMenu->AddEntry(new OHotString("Create &soft link here"), M_DND_SYMLINK);
  _dndMenu->AddSeparator();
  _dndMenu->AddEntry(new OHotString("Cancel"), -1);
  _dndMenu->SetDefaultEntry(M_DND_MOVE);
  _dndMenu->Associate(NULL);

  //-------------- tool bar and separator

  _toolBarSep = new OXHorizontal3dLine(this);

  _toolBar = new OXToolBar(this);

  _ddlb = new OXFileSystemDDListBox(_toolBar, 1010);
  _ddlb->SetTip("Go To Folder");

  _toolBar->AddFrame(_ddlb, new OLayoutHints(LHINTS_LEFT | LHINTS_EXPAND_Y,
                                             0, 8, 0, 0));
  _ddlb->Resize(150, _ddlb->GetDefaultHeight());
  _ddlb->Associate(this);

  _toolBar->AddButtons(tb_data);

  AddFrame(_toolBarSep, new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X));
  AddFrame(_toolBar, new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X,
                                      0, 0, 2, 2));

  //-------------- panels

  _hf = new OXHorizontalFrame(this, 10, 10);
  _v2 = new OXVerticalFrame(_hf, 10, 10);

  if (_mainMode == EXPLORER_MODE) {
    _v1 = new OXVerticalFrame(_hf, 10, 10, FIXED_WIDTH);

    _treeHdr = new OXCompositeFrame(_v1, 10, 10, SUNKEN_FRAME);
    _listHdr = new OXCompositeFrame(_v2, 10, 10, SUNKEN_FRAME);

    _lbl1 = new OXLabel(_treeHdr, new OString("All Folders"));
    _lbl2 = new OXLabel(_listHdr, new OString("Contents of \".\""));

    _treeHdr->AddFrame(_lbl1, new OLayoutHints(LHINTS_LEFT | LHINTS_CENTER_Y,
                                               3, 0, 0, 0));
    _listHdr->AddFrame(_lbl2, new OLayoutHints(LHINTS_LEFT | LHINTS_CENTER_Y,
                                               3, 0, 0, 0));

    _v1->AddFrame(_treeHdr, new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X,
                                             0, 0, 1, 2));
    _v2->AddFrame(_listHdr, new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X,
                                             0, 0, 1, 2));

    _v1->Resize(_treeHdr->GetDefaultWidth()+100, _v1->GetDefaultHeight());

    _hf->AddFrame(_v1, new OLayoutHints(LHINTS_LEFT | LHINTS_EXPAND_Y,
                                      0, 0/*2*/, 0, 0));
    OXVerticalResizer *res = new OXVerticalResizer(_hf, 2, 2);
    _hf->AddFrame(res, new OLayoutHints(LHINTS_EXPAND_Y));
    res->SetPrev(_v1);
    res->SetNext(_v2);
  }

  _hf->AddFrame(_v2, new OLayoutHints(LHINTS_RIGHT |
                                      LHINTS_EXPAND_X | LHINTS_EXPAND_Y));

  //-------------- tree

  if (_mainMode == EXPLORER_MODE) {

    _dt = new OXDirectoryTree(_v1, 10, 10, -1, SUNKEN_FRAME | DOUBLE_BORDER);
    _dt->Associate(this);

    //_dt->SetScrollMode(SB_ACCELERATED);
    //_dt->SetScrollDelay(10, 10);

    _v1->AddFrame(_dt, new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y));
  }

  //-------------- files

  _fileWindow = new OXFileList(_v2, -1, MimeTypeList, "*", 520, 250);
  _fileWindow->Associate(this);

  _v2->AddFrame(_fileWindow, new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y));

  AddFrame(_hf, new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y,
 		 	                   0, 0, 0/*1*/, 0));

  //-------------- status bar

  _statusBar = new OXStatusBar(this);
  AddFrame(_statusBar, new OLayoutHints(LHINTS_BOTTOM | LHINTS_EXPAND_X,
		 		        0, 0, 3, 0));

  _statusBar->AddLabel(180);
  _statusBar->SetWidth(0, 180);

  //--------------

  strcpy(_startDir, startDir);

  _rcfilename = "explorerrc";

  char recyclerc[PATH_MAX];
  char recyclebin[PATH_MAX];

  sprintf(recyclerc, "%s/etc/recycle.files",
                     _client->GetResourcePool()->GetUserRoot());

  sprintf(recyclebin, "%s/recycle",
                      _client->GetResourcePool()->GetUserRoot());

  _recycled = new ORecycledFiles(recyclebin, recyclerc);

  SetWindowName(getcwd(tmp, BUFSIZ));
  SetIconName("Explorer");
  SetClassHints("Explorer", "Explorer");

  SetMWMHints(MWM_DECOR_ALL, MWM_FUNC_ALL, MWM_INPUT_MODELESS);

  ReadIniFile();

  MapSubwindows();
  Resize(_ww, _wh);
  Layout();

  if (_mainMode == EXPLORER_MODE) {
    SetFocusOwner(_dt);
  } else {
    SetFocusOwner(_fileWindow);
  }

  MapWindow();
}

OXExplorer::~OXExplorer() {
  SaveIniFile();

  // Need to delete OLayoutHints and OXPopupMenu objects only,
  // OXCompositeFrame takes care of the rest...
 
  delete _menuBarLayout;
  delete _menuBarItemLayout;
 
  delete _menuFile;
  delete _menuEdit;
  delete _menuView;
  delete _menuHelp;
  delete _newMenu;
  delete _sortMenu;

  delete _contextMenu;
  delete _sendToMenu;

  delete _recycled;
}

int OXExplorer::HandleMapNotify(XMapEvent *event) {

  DoChangeDirectory(_startDir);
  UpdateListBox();
  UpdateTree();

  return True;
}


//--- Load explorer settings

void OXExplorer::ReadIniFile() {
  char *inipath, line[1024], arg[256];
  int  ViewMode, SortMode;

  inipath = _client->GetResourcePool()->FindIniFile(_rcfilename, INI_READ);
  if (!inipath) inipath = _rcfilename;

  OIniFile rcfile(inipath, INI_READ);

  while(rcfile.GetNext(line)) {
    if (strcasecmp(line, "defaults") == 0) {

      DispFullPath   = rcfile.GetBool("DisplayFullPath");
      NewBrowser     = rcfile.GetBool("NewBrowser");
      _showToolBar   = rcfile.GetBool("ShowToolBar");
      _showStatusBar = rcfile.GetBool("ShowStatusBar");

      if (rcfile.GetItem("ViewMode", arg)) {
        if (strcasecmp(arg, "largeicons") == 0) ViewMode = M_VIEW_LARGEICONS;
        else if (strcasecmp(arg, "smallicons") == 0) ViewMode = M_VIEW_SMALLICONS;
        else if (strcasecmp(arg, "list") == 0) ViewMode = M_VIEW_LIST;
        else if (strcasecmp(arg, "details") == 0) ViewMode = M_VIEW_DETAILS;
      }

      if (rcfile.GetItem("SortOrder", arg)) {
        if (strcasecmp(arg, "byname") == 0) SortMode = M_VIEW_ARRANGE_BYNAME;
        else if (strcasecmp(arg, "bytype") == 0) SortMode = M_VIEW_ARRANGE_BYTYPE;
        else if (strcasecmp(arg, "bydate") == 0) SortMode = M_VIEW_ARRANGE_BYDATE;
        else if (strcasecmp(arg, "bysize") == 0) SortMode = M_VIEW_ARRANGE_BYSIZE;
      }

      if (rcfile.GetItem("WindowSize", arg)) {
        int w, h;

        if (sscanf(arg, "%d x %d", &w, &h) == 2) {
          if (w > 10 && w < 10000 && h > 10 && h < 10000) {
            _ww = w;
            _wh = h;
          }
        }
      }
    }
  }

  //--- init view mode...

  SetViewMode(ViewMode, True);

  //--- init sort mode...

  SetSortMode(SortMode);

  //--- toolbar and status bar...

  if (!_showToolBar) {
    _menuView->UnCheckEntry(M_VIEW_TOOLBAR);
    HideFrame(_toolBar);
    HideFrame(_toolBarSep);
  }

  if (!_showStatusBar) {
    _menuView->UnCheckEntry(M_VIEW_STATUS);
    HideFrame(_statusBar);
  }

  if (inipath != _rcfilename) delete[] inipath;
}


//--- Save current explorer settings

void OXExplorer::SaveIniFile() {
  char *arg, *inipath, tmp[256];

  inipath = _client->GetResourcePool()->FindIniFile(_rcfilename, INI_WRITE);
  if (!inipath) inipath = _rcfilename;

  OIniFile rcfile(inipath, INI_WRITE);

  rcfile.PutNext("defaults");

  rcfile.PutBool("DisplayFullPath", DispFullPath);
  rcfile.PutBool("NewBrowser", NewBrowser);
  rcfile.PutBool("ShowToolBar", _toolBar->IsVisible());
  rcfile.PutBool("ShowStatusBar", _statusBar->IsVisible());

  switch (_viewMode) {
    default:
    case M_VIEW_LARGEICONS: arg = "LargeIcons"; break;
    case M_VIEW_SMALLICONS: arg = "SmallIcons"; break;
    case M_VIEW_LIST:       arg = "List";       break;
    case M_VIEW_DETAILS:    arg = "Details";    break;
  }
  rcfile.PutItem("ViewMode", arg);

  switch (_sortMode) {
    default:
    case M_VIEW_ARRANGE_BYNAME: arg = "ByName"; break;
    case M_VIEW_ARRANGE_BYTYPE: arg = "ByType"; break;
    case M_VIEW_ARRANGE_BYSIZE: arg = "BySize"; break;
    case M_VIEW_ARRANGE_BYDATE: arg = "ByDate"; break;
  }
  rcfile.PutItem("SortOrder", arg);

  sprintf(tmp, "%d x %d", _w, _h);
  rcfile.PutItem("WindowSize", tmp);

  if (inipath != _rcfilename) delete[] inipath;
}


//--- Display the number of objects in the file container in status bar

void OXExplorer::DisplayTotal(int total, int selected) {
  char tmp[256], *fmt;

  if (selected)
    fmt = "%d object%s, %d selected.";
  else
    fmt = "%d object%s.";

  sprintf(tmp, fmt, total, (total == 1) ? "" : "s", selected);
  _statusBar->SetText(0, new OString(tmp));
}


//--- Set file window's view mode and update menu and toolbar buttons
//    accordingly.

void OXExplorer::SetViewMode(int new_mode, int force) {
  int i, lv, bnum;

  if (force || (_viewMode != new_mode)) {

    switch (new_mode) {
      default: if (!force) return; else new_mode = M_VIEW_LARGEICONS;
      case M_VIEW_LARGEICONS: bnum = 14; lv = LV_LARGE_ICONS; break;
      case M_VIEW_SMALLICONS: bnum = 15; lv = LV_SMALL_ICONS; break;
      case M_VIEW_LIST:       bnum = 16; lv = LV_LIST;        break;
      case M_VIEW_DETAILS:    bnum = 17; lv = LV_DETAILS;     break;
    }

    _viewMode = new_mode;
    _menuView->RCheckEntry(_viewMode, M_VIEW_LARGEICONS, M_VIEW_DETAILS);

    for (i = 14; i <= 17; ++i)
      tb_data[i].button->SetState((i == bnum) ? BUTTON_ENGAGED : BUTTON_UP);

    _fileWindow->SetViewMode(lv);

  }
}


//--- Set file sort mode and update menu accordingly.

void OXExplorer::SetSortMode(int new_mode) {
  int smode;

  switch (new_mode) {
    default: new_mode = M_VIEW_ARRANGE_BYNAME;
    case M_VIEW_ARRANGE_BYNAME: smode = SORT_BY_NAME; break;
    case M_VIEW_ARRANGE_BYTYPE: smode = SORT_BY_TYPE; break;
    case M_VIEW_ARRANGE_BYDATE: smode = SORT_BY_DATE; break;
    case M_VIEW_ARRANGE_BYSIZE: smode = SORT_BY_SIZE; break;
  }

  _sortMode = new_mode;
  _sortMenu->RCheckEntry(_sortMode, M_VIEW_ARRANGE_BYNAME, M_VIEW_ARRANGE_BYDATE);
  _fileWindow->Sort(smode);
}


//--- Set the top-level window title to the current path

void OXExplorer::SetTitle() {
  char *p = _currentDir;
     
  if (!DispFullPath) {
    p = strrchr(_currentDir, '/');
    if (p) p = *(++p) ? p : _currentDir;
  }

  SetWindowName(p);
}


//--- Update the filesystem drop-down listbox contents

void OXExplorer::UpdateListBox() {
  char path[PATH_MAX];

  // Use what getcwd returns because the name of the directory
  // may have changed.

  _ddlb->UpdateContents(getcwd(path, PATH_MAX));

  strcpy(_currentDir, path);
  SetTitle();
}


//--- Update the filesystem list tree contents

void OXExplorer::UpdateTree() {
  if (_mainMode == EXPLORER_MODE) {
    _dt->UpdateTree();
  }
}


//----------------------------------------------------------------------

int OXExplorer::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;
  OItemViewMessage *vmsg;

  switch (msg->type) {

    // process menu and buttons together
    case MSG_MENU:
    case MSG_BUTTON:
      switch (msg->action) {

        case MSG_CLICK:
          switch (wmsg->id) {

            case M_FILE_OPEN:
              if (_fileWindow->NumSelected() == 1) {
                const OFileItem *f;
                std::vector<OItem *> items;

                items = _fileWindow->GetSelectedItems();
                f = (OFileItem *) items[0];
                DoOpen(f);
              }
              break;

            case M_FILE_OPENWITH:
              if (_fileWindow->NumSelected() == 1) {
                const OFileItem *f;
                std::vector<OItem *> items;

                items = _fileWindow->GetSelectedItems();
                f = (OFileItem *) items[0];
                DoOpenWith(f);
              }
              break;

            case M_FILE_EXPLORE:
              if (_fileWindow->NumSelected() == 1) {
                const OFileItem *f;
                std::vector<OItem *> items;

                items = _fileWindow->GetSelectedItems();
                f = (OFileItem *) items[0];
                DoExplore(f);
              }
              break;

            case M_FILE_PROPS:
              if (_fileWindow->NumSelected() == 1) {
                const OFileItem *f;
                std::vector<OItem *> items;

                items = _fileWindow->GetSelectedItems();
                f = (OFileItem *) items[0];
                new OXPropertiesDialog(_client->GetRoot(), this,
                                       new OString(f->GetName()));
              } else {
                // group file properties: chmod only?
                XBell(GetDisplay(), 0);
              }
              break;

            case M_FILE_DELETE:
              DeleteFiles();
              break;

            case M_FILE_CLOSE:
              CloseWindow();
              break;

            case M_EDIT_SELECTALL:
              _fileWindow->SelectAll();
              break;

            case M_EDIT_INVSELECTION:
              _fileWindow->InvertSelection();
              break;

            case M_VIEW_TOOLBAR:
              DoToggleToolBar();
              break;

            case M_VIEW_STATUS:
              DoToggleStatusBar();
              break;

            case M_VIEW_OPTIONS:
              new OXOptionsDialog(_client->GetRoot(), this);
              break;

            case M_VIEW_LARGEICONS:
            case M_VIEW_SMALLICONS:
            case M_VIEW_LIST:
            case M_VIEW_DETAILS:
              SetViewMode(wmsg->id);
              break;

            case M_VIEW_ARRANGE_BYNAME:
            case M_VIEW_ARRANGE_BYTYPE:
            case M_VIEW_ARRANGE_BYSIZE:
            case M_VIEW_ARRANGE_BYDATE:
              SetSortMode(wmsg->id);
              break;

            case M_VIEW_REFRESH:
              _fileWindow->DisplayDirectory();
              UpdateListBox();
UpdateTree();
              break;

            case M_HELP_ABOUT:
              About();
              break;

            case 99:
              DoChangeDirectory("..");
              UpdateListBox();
UpdateTree();
              break;

            default:
              XBell(GetDisplay(), 0);
              break;

          } // switch (msg->_parm1)
          break;

        default:
          break;
      }
      break;

    case MSG_DDLISTBOX:
      if ((msg->action == MSG_CLICK) && (wmsg->id == 1010)) {
        OXTreeLBEntry *e = (OXTreeLBEntry *) _ddlb->GetSelectedEntry();
        if (e) {
          DoChangeDirectory(e->GetPath()->GetString());
          UpdateListBox();
UpdateTree();
        }
      }
      break;

    case MSG_LISTVIEW:
      vmsg = (OItemViewMessage *) msg;
      switch (msg->action) {
        case MSG_CLICK:
          if (_fileWindow->NumSelected() == 1) {
            const OFileItem *f;
            std::vector<OItem *> items;

            items = _fileWindow->GetSelectedItems();
            f = (OFileItem *) items[0];
            SetupContextMenu(CONTEXT_MENU_FILE, f->GetFileType());
          } else {
            SetupContextMenu(CONTEXT_MENU_NONE, 0);
          }
          if (vmsg->button == Button3) {
            if (!_fileWindow->IsDragging())
              _contextMenu->PlaceMenu(vmsg->pos.x, vmsg->pos.y, True, True);
          }
          break;

        case MSG_DBLCLICK:
          if (vmsg->button == Button1) {
            if (_fileWindow->NumSelected() == 1) {
              const OFileItem *f;
              std::vector<OItem *> items;

              items = _fileWindow->GetSelectedItems();
              f = (OFileItem *) items[0];
              DoOpen(f);
            }
          }
          break;

        case MSG_SELCHANGED:
          if (_fileWindow->NumSelected() == 0) {
            SetupContextMenu(CONTEXT_MENU_NONE, 0);
          }
          DisplayTotal(_fileWindow->GetNumberOfItems(),
                       _fileWindow->NumSelected());
          break;

        case MSG_DROP:
          {
          ODNDmessage *dndmsg = (ODNDmessage *) msg;
          char *data = (char *) dndmsg->data->data;

          // TODO:
          // - put all this in a separate routine
          // - check for special cases: drop on same icon, drop on
          //   same dir, dest is recycle bin, etc.

          if (data) {
            char from[PATH_MAX], to[PATH_MAX], cwd[PATH_MAX], *p;

            getcwd(cwd, PATH_MAX);

            URL url(data);

            strcpy(from, url.full_path);

            if (dndmsg->dragOver) {
              int type = dndmsg->dragOver->GetFileType();
              if (S_ISDIR(type)) {
                // drop on a directory
                strcpy(to, cwd);
                strcat(to, "/");
                strcat(to, dndmsg->dragOver->GetName()->GetString());
                strcat(to, "/");
                strcat(to, url.filename);
              } else {
                if ((type & S_IXUSR) || (type & S_IXGRP) || (type & S_IXOTH)) {
                  // drop on an executable
                  strcpy(to, cwd);
                  strcat(to, "/");
                  strcat(to, dndmsg->dragOver->GetName()->GetString());
                  char *argv[3];
                  argv[0] = to;
                  argv[1] = url.full_path;
                  argv[2] = NULL;
                  OExec exec(_client, to, argv, NULL, False, True);
                  return True;
                } else {
                  // drop on a regular document
                  return True;
                }
              }
            } else {
              // drop on empty space
              strcpy(to, cwd);
              strcat(to, "/");
              strcat(to, url.filename);  // basename(url.full_path);
            }

            if (strcmp(from, to) != 0) {
              int action = dndmsg->dndAction;

              if (action == ODNDmanager::DNDactionAsk) {
                int choice = _dndMenu->PopupMenu(dndmsg->pos.x, dndmsg->pos.y);
                switch (choice) {
                  case M_DND_MOVE:
                    action = ODNDmanager::DNDactionMove;
                    break;

                  case M_DND_COPY:
                    action = ODNDmanager::DNDactionCopy;
                    break;

                  case M_DND_SYMLINK:
                    action = ODNDmanager::DNDactionLink;
                    break;

                  default:
                    return True;
                }
              }

              //printf("action %d from \"%s\" to \"%s\"\n", action, from, to);

              if (action == ODNDmanager::DNDactionCopy) {
                CopyFile(from, to);
              } else if (action == ODNDmanager::DNDactionMove) {
                MoveFile(from, to);
              } else if (action == ODNDmanager::DNDactionLink) {
                SymlinkFile(from, to);
              } else {
                // unknown action...
              }
            }
            _fileWindow->DisplayDirectory(True);
          }
          }

        default:
          break;
      }
      break;

//    case MSG_DIRCHANGED:
//      UpdateListBox();
//      break;

    case MSG_LISTTREE:
      switch (msg->action) {
        case MSG_DBLCLICK:
          break;

        case MSG_CLICK:
          { OListTreeItem *h;
            if ((h = _dt->GetSelected()) != NULL) {
              char *p, tmp[PATH_MAX];
              _dt->GetPathnameFromItem(h, tmp);
              p = tmp;
              while (*p && *(p+1) == '/') ++p;
              DefineCursor(GetResourcePool()->GetWaitCursor());
              XFlush(GetDisplay());
              _dt->ReadDir(p, h);
              if (strcmp(p, _currentDir) != 0) {
                DoChangeDirectory(p);
                UpdateListBox();
              }
              DefineCursor(None);
            }
          }
          break;
      }
      break;

    default:
      break;

  } // switch (type)

  return True;
}


//--- Setup right-click context menu according to object type

void OXExplorer::SetupContextMenu(int mode, int ftype) {
  _contextMenu->RemoveAllEntries();
  _menuFile->RemoveAllEntries();

  if (mode == CONTEXT_MENU_FILE) {

    if (S_ISDIR(ftype)) {
      _contextMenu->AddEntry(new OHotString("&Open"), M_FILE_OPEN);
      _contextMenu->AddEntry(new OHotString("&Explore"), M_FILE_EXPLORE);
      _contextMenu->AddEntry(new OHotString("&Find..."), M_FILE_FIND);
      _contextMenu->SetDefaultEntry(M_FILE_OPEN);

      _menuFile->AddEntry(new OHotString("&Open"), M_FILE_OPEN);
      _menuFile->AddEntry(new OHotString("&Explore"), M_FILE_EXPLORE);
      _menuFile->AddEntry(new OHotString("&Find..."), M_FILE_FIND);
      _menuFile->SetDefaultEntry(M_FILE_OPEN);
    } else {
      _contextMenu->AddEntry(new OHotString("Op&en with..."), M_FILE_OPENWITH);
      _contextMenu->SetDefaultEntry(M_FILE_OPENWITH);

      _menuFile->AddEntry(new OHotString("Op&en with..."), M_FILE_OPENWITH);
      _menuFile->SetDefaultEntry(M_FILE_OPENWITH);
    }

    _contextMenu->AddSeparator();
    _contextMenu->AddPopup(new OHotString("Se&nd To"), _sendToMenu);
    _contextMenu->AddSeparator();
    _contextMenu->AddEntry(new OHotString("Cu&t"), M_EDIT_CUT);
    _contextMenu->AddEntry(new OHotString("&Copy"), M_EDIT_COPY);
    _contextMenu->AddEntry(new OHotString("&Paste"), M_EDIT_PASTE);
    _contextMenu->AddSeparator();

    _menuFile->AddSeparator();
    _menuFile->AddPopup(new OHotString("Se&nd To"), _sendToMenu);
    _menuFile->AddSeparator();

  } else if (mode == CONTEXT_MENU_RECYCLE) {

/*
                 File menu                        Context menu

  no file        Empty Recycle &Bin               &View           >
  selected       ------------------               -----------------
                 (Create &Shortcut)               Arrange &Icons  >
                 (&Delete)                        Line &Up Icons
                 (Rena&me)                        -----------------
                 (P&roperties)                    Re&fesh
                 ------------------               -----------------
                 &Close                           (&Paste)
                                                  (Paste &Shortcut)
                                                  -----------------
                                                  P&roperties

  file           R&estore                         R&estore
  selected       ------------------               -----------------
                 Empty Recycle &Bin               Cu&t
                 ------------------               -----------------
                 (Create &Shortcut)               &Delete
                 &Delete                          -----------------
                 (Rena&me)                        >P&roperties<      (default)
                 P&roperties
                 ------------------
                 &Close


                                                  >&Open<
                                                  &Explore
                                                  Empty Recycle &Bin
                                                  ------------------
                                                  Create &Shortcut
                                                  -----------------
                                                  P&roperties
*/


  } else if (mode == CONTEXT_MENU_NONE) {

    _contextMenu->AddPopup(new OHotString("Ne&w"), _newMenu);
    _contextMenu->AddSeparator();

  }

  _contextMenu->AddEntry(new OHotString("Create &Soft Link"), M_FILE_NEWSHORTCUT);
  _contextMenu->AddEntry(new OHotString("&Delete"), M_FILE_DELETE);
  _contextMenu->AddEntry(new OHotString("Rena&me"), M_FILE_RENAME);
  _contextMenu->AddSeparator();
  _contextMenu->AddEntry(new OHotString("P&roperties..."), M_FILE_PROPS);

  _menuFile->AddPopup(new OHotString("Ne&w"), _newMenu);
  _menuFile->AddSeparator();
  _menuFile->AddEntry(new OHotString("Create &Soft Link"), M_FILE_NEWSHORTCUT);
  _menuFile->AddEntry(new OHotString("&Delete"), M_FILE_DELETE);
  _menuFile->AddEntry(new OHotString("Rena&me"), M_FILE_RENAME);
  _menuFile->AddEntry(new OHotString("P&roperties..."), M_FILE_PROPS);
  _menuFile->AddSeparator();
  _menuFile->AddEntry(new OHotString("&Close"), M_FILE_CLOSE);

  if (mode != CONTEXT_MENU_FILE) {

    _menuFile->DisableEntry(M_FILE_NEWSHORTCUT);
    _menuFile->DisableEntry(M_FILE_DELETE);
    _menuFile->DisableEntry(M_FILE_RENAME);
    _menuFile->DisableEntry(M_FILE_PROPS);

    _contextMenu->DisableEntry(M_FILE_NEWSHORTCUT);
    _contextMenu->DisableEntry(M_FILE_DELETE);
    _contextMenu->DisableEntry(M_FILE_RENAME);
    _contextMenu->DisableEntry(M_FILE_PROPS);

  }
}


//--- Change directory

void OXExplorer::DoChangeDirectory(const char *path) {

  _fileWindow->ChangeDirectory(path);

  if (_mainMode == EXPLORER_MODE) {
    char cwd[PATH_MAX], hdr[PATH_MAX];

    getcwd(cwd, PATH_MAX);
    sprintf(hdr, "Contents of \"%s\"", cwd);
    _lbl2->SetText(new OString(hdr));
    _listHdr->Layout();  // so the label gets resized accordingly...
  }
}


//--- Execute the "open" action for the specified object

void OXExplorer::DoOpen(const OFileItem *f) {
  int   pid, ftype;
  char  action[PATH_MAX];
  char  filename[PATH_MAX];
  char  path[PATH_MAX]; 
  char *argv[PATH_MAX]; 
  int   argc = 0;
  char *argptr;

  // get file type
  ftype = f->GetFileType();

  // is a directory?
  if (S_ISDIR(ftype)) { 
    if ((_mainMode == FILE_MGR_MODE) && NewBrowser) {
      char *argv[3];
      argv[0] = AppPath;
      argv[1] = (char *) f->GetName()->GetString();
      argv[2] = NULL;
      OExec exec(_client, AppPath, argv, NULL, False, True);
    } else {
      DoChangeDirectory(f->GetName()->GetString());
      UpdateListBox();
    }

  // execute the action specified in MimeTypeList, if any.
  } else if (MimeTypeList->GetAction(f->GetName()->GetString(), action)) {
    getcwd(path, PATH_MAX);
    sprintf(filename, "%s/%s", path, f->GetName()->GetString());
    argptr = strtok(action, " ");
    while (argptr != NULL) {
      if (strcmp(argptr, "%s") == 0)
        argv[argc++] = StrDup(filename);
      else
        argv[argc++] = StrDup(argptr);
      argptr = strtok(NULL, " ");
    }
    argv[argc] = NULL;

    OExec exec(_client, argv[0], argv, NULL, False, True);

    for (int i = 0; i < argc; i++) delete[] argv[i];

  // is an exec file?
  } else if (S_ISREG(ftype) && (ftype & S_IXUSR)) {

    getcwd(path, PATH_MAX);
    sprintf(filename, "%s/%s", path, f->GetName()->GetString());
    char *argv[2];
    argv[0] = (char *) f->GetName()->GetString();
    argv[1] = NULL;

    OExec exec(_client, filename, argv, NULL, False, True);

  }
}


//--- Execute the "open with" action for the specified object

void OXExplorer::DoOpenWith(const OFileItem *f) {
  int  pid;
  char path[PATH_MAX], filename[PATH_MAX];

  OString progname("");

  new OXOpenWithDialog(_client->GetRoot(), this, f->GetName(), &progname);
  if (progname.GetLength() == 0) return;  // user cancelled dialog

  // execute the program
  getcwd(path, PATH_MAX);
  sprintf(filename, "%s/%s", path, f->GetName()->GetString());

  char *argv[3];
  argv[0] = (char *) progname.GetString();
  argv[1] = filename;
  argv[2] = NULL;

  OExec exec(_client, progname.GetString(), argv, NULL, False, True);
}


//--- Execute the "explore" action for the specified folder

void OXExplorer::DoExplore(const OFileItem *f) {
  int pid;

  // must be a directory!
  if (S_ISDIR(f->GetFileType())) { 
    pid = fork();
    if (pid == 0) {
      char FullPath[PATH_MAX];

      if (!realpath(AppPath, FullPath)) strcpy(FullPath, AppPath);
      chdir(f->GetName()->GetString());

      execlp(FullPath, FullPath, NULL);

      // if we are here then execlp failed!
      fprintf(stderr, "Cannot spawn \"%s\": execlp failed!\n", AppPath);
      exit(1);
    }
  }
}


//--- Copy a single file

void OXExplorer::CopyFile(const char *from, const char *to) {
  new OXCopyBox(_client->GetRoot(), this, from, to);
}


//--- Move a single file

void OXExplorer::MoveFile(const char *from, const char *to) {
  // need to check for the existence of the destination
  // first, since rename replaces without asking
  int retc = ID_YES;
  if (access(to, F_OK) == 0) {
    new OXMsgBox(_client->GetRoot(), this,
                 new OString("Move"),
                 new OString("A file with the same name already "
                             "exists. Overwrite?"),
                 MB_ICONQUESTION, ID_YES | ID_NO, &retc);
    if (retc != ID_YES) return;
  }
  if (rename(from, to) == 0) return;
  if (errno == EXDEV) {
    new OXCopyBox(_client->GetRoot(), this, from, to);
    // multi-threading would really help here...
    // if (unlink(from) == 0) return;
    // retc = errno;
    // unlink(to);
    // new OXMsgBox(...Cannot move/remove source file... strerr(retc)...);
    // return;
  } else {
    char tmp[PATH_MAX*3];
    sprintf(tmp, "Cannot move file \"%s\"\n"
                 "to \"%s\":\n%s.", from, to, strerror(errno));
    new OXMsgBox(_client->GetRoot(), this, new OString("Error"),
                 new OString(tmp), MB_ICONSTOP, ID_OK);
  }
}


//--- Create a softlink to a file

void OXExplorer::SymlinkFile(const char *from, const char *to) {
#if 0
  // check first for the existence of the destination
  int retc = ID_YES;
  if (access(to, F_OK) == 0) {
    new OXMsgBox(_client->GetRoot(), this,
                 new OString("Move"),
                 new OString("A file with the same name already "
                             "exists. Overwrite?"),
                 MB_ICONQUESTION, ID_YES | ID_NO, &retc);
    if (retc != ID_YES) return;
  }
#endif
  if (symlink(from, to) == 0) return;

  char tmp[PATH_MAX*3];
  sprintf(tmp, "Cannot move file \"%s\"\n"
               "to \"%s\":\n%s.", from, to, strerror(errno));
  new OXMsgBox(_client->GetRoot(), this, new OString("Error"),
               new OString(tmp), MB_ICONSTOP, ID_OK);
}


//--- Delete selected files

void OXExplorer::DeleteFiles() {
  const OFileItem *f;  
  struct stat  inode;
  const  char *basename;
  char         dirname[PATH_MAX];
  char         newfilename[PATH_MAX];
  char         filename[PATH_MAX];
  char         recycle[PATH_MAX];
  char         recfilename[PATH_MAX];
  int          index = 0;
  time_t t;
  int    sel, retval;
  const  OPicture *pic;
  char  *title, prompt[256];
  std::vector<OItem *> items;

  if ((sel = _fileWindow->NumSelected()) == 0) return;

  items = _fileWindow->GetSelectedItems();
  f = (OFileItem *) items[0];

  if (sel == 1) {
    pic = _client->GetPicture("srecycle.xpm");
    sprintf(prompt, "Are you sure you want to send \"%s\" to the Recycle Bin?",
            f->GetName()->GetString());
    title = "Confirm File Delete";
  } else {
    pic = _client->GetPicture("mrecycle.xpm");
    sprintf(prompt, "Are you sure you want to send these %d items to the Recycle Bin?",
            sel);
    title = "Confirm Multiple File Delete";
  }

  new OXMsgBox(_client->GetRoot(), this,
               new OString(title), new OString(prompt), pic,
               ID_YES | ID_NO, &retval);

  if (retval != ID_YES) return;

  getcwd(dirname, PATH_MAX);
  strcpy(recycle, _recycled->GetRecycleBin());

  for (int i = 0; i < items.size(); ++i) {
    f = (OFileItem *) items[i];
    basename = f->GetName()->GetString();
    if ((strcmp(basename, ".") != 0) && (strcmp(basename, "..") != 0)) {
      sprintf(filename, "%s/%s", dirname, basename);
      sprintf(recfilename, "%s/%s", recycle, basename);
      sprintf(newfilename, "%s", basename);
      index = 0;
      while (stat(recfilename, &inode) == 0) {
        sprintf(newfilename, "%s.%d", basename, index++);
        sprintf(recfilename, "%s/%s", recycle, newfilename);
      }
      time(&t);
      _recycled->AddFile(basename, newfilename,
                         ctime((const time_t *) &t), filename);

      // warning! possible cross-device rename attempt!

      if (rename(filename, recfilename) != 0) {
        sprintf(prompt, "Cannot delete file \"%s\": %s.",
                basename, strerror(errno));
        new OXMsgBox(_client->GetRoot(), this,
                     new OString("Error"), new OString(prompt),
                     MB_ICONSTOP, ID_OK);
        
      }
    }
  }

  _fileWindow->DisplayDirectory();
}

void OXExplorer::DoToggleToolBar() {
  if (_toolBar->IsVisible()) {
    HideFrame(_toolBar);
    HideFrame(_toolBarSep);
    _menuView->UnCheckEntry(M_VIEW_TOOLBAR);
  } else {
    ShowFrame(_toolBarSep);
    ShowFrame(_toolBar);
    _menuView->CheckEntry(M_VIEW_TOOLBAR);
  }
}

void OXExplorer::DoToggleStatusBar() {
  if (_statusBar->IsVisible()) {
    HideFrame(_statusBar);
    _menuView->UnCheckEntry(M_VIEW_STATUS);
  } else {
    ShowFrame(_statusBar);
    _menuView->CheckEntry(M_VIEW_STATUS);
  }
}

void OXExplorer::About() {
  OAboutInfo info;
  
  info.wname = "About xclass Explorer";
  info.title = "Explorer version "EXPLORER_VERSION"\n"
               "A Windows95-like file manager fox X windows\n"
               "Compiled with xclass version "XCLASS_VERSION;
  info.copyright = "Copyright © 1998-2001, Héctor Peraza.";
  info.text = "This program is free software; you can redistribute it "
              "and/or modify it under the terms of the GNU "
              "General Public License.\n\n"
              "http://xclass.sourceforge.net\n"
              "http://www.foxproject.org";

  new OXAboutDialog(_client->GetRoot(), this, &info);
}

/*

Object-dependent actions:

  Context menu
  File menu
  Drop action
  Properties
  Open (double-click) action  (same as default (bold) menu entry)
  
Object types:

  Regular folders (directories)
  Special folders (recycle bin, desktop)
  Executable files
  Documents of known type (associated to a mime action)
  Documents of unknown type

Properties dialog (multiple files and/or dirs):

  46 Files, 8 Folders
  -------------------
  Type:     All of type File Folder (or Multiple Types)
  Location: All in C:\Windows (or Various Folders --from Find dialog)
  Size:     4.67MB (4,897,852 bytes), 4,988,928 bytes used.
  --------------------
  Attributes:
    -as usual- partly set attribute bits have ligth-grayed bgnd w/ #808080
               check mark.

  4 Files, 0 Folders

Properties dialog (recycle bin):

  "Global" Tab:

  ( ) &Configure drives independently
  (*) &Use one setting for all drives ---group frame----+
   [ ] Do not remove files to the &Recycle Bin.\n       |
       Remove files immediately when deleted            |
                                                        |
     --V------------                                    |
     | | | | | | | |                                    |
           10%                                          |
                                                        |
   &Maximum size of Recycle Bin (percent of each drive) |
  ------------------------------------------------------+
  [x] &Display delete confirmation dialog box.

*/
