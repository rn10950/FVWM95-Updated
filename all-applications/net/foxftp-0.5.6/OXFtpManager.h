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

#ifndef __OXFTPMANAGER_H
#define __OXFTPMANAGER_H

class OBookmark;
class OBookmarkList;
class OIniFile;
class OLayoutHints;
class OListTreeItem;
class OString;

class OXHorizontal3dLine;
class OXListTree;
class OXListView;
class OXTextEntry;
class OXSecretTextEntry;
class OXCheckButton;
class OXTextEdit;

template <class T> class TDDLList;

#include <xclass/OXTransientFrame.h>
#include <xclass/OXTab.h>
#include <xclass/OXGroupFrame.h>
#include <xclass/OXDDListBox.h>
#include <xclass/OXTextButton.h>
#include <xclass/OXLabel.h>


//----------------------------------------------------------------------

class OBookmark {
public:
  OBookmark(const char *name, const char *description, OSiteConfig *info);
  OBookmark(const char *name, OString *description, OSiteConfig *info);
  OBookmark(const OBookmark &item);
  virtual ~OBookmark();

  void SetName(const char *name);
  void SetDescription(OString *);
  void SetDescription(const char *);
  void SetSiteConfig(OSiteConfig *);
  char *GetName() { return _name; }
  OString *GetDescription() { return _description; }
  OSiteConfig *GetSiteConfig() { return _info; }

protected:
  char *_name;
  OString *_description;
  OSiteConfig *_info;
};


class OBookmarkList {
public:
  OBookmarkList(char *path, TDDLList<OBookmark *> *list);
  virtual ~OBookmarkList();

  void SetPath(char *path);
  void SetList(TDDLList<OBookmark *> *list);
  char *GetPath() { return _path; }
  TDDLList<OBookmark *> *GetList() { return _list; }

protected:
  char *_path;
  TDDLList<OBookmark *> *_list;
};


class OXBookmarkProp : public OXTransientFrame {
public:
  OXBookmarkProp(const OXWindow *p, const OXWindow *main, int w, int h,
                OBookmark *bmark, int *rtc,
                unsigned long options = MAIN_FRAME | VERTICAL_FRAME);
  virtual ~OXBookmarkProp();

  virtual int ProcessMessage(OMessage *);

protected:
  OBookmark *_bmark;
  int *_rtc;

  OXTab *_tab;
  OXCompositeFrame *_tabFrame1, *_tabFrame2, *_tabFrame3;
  OXCompositeFrame *_bframe, *_mframe, *_mframe2;
  OXTextButton *_tbOk, *_tbCancel;
  OXLabel *_lname, *_ldes, *_lhost, *_lport, *_linitd;
  OXTextEntry *_tename, *_tehost, *_teport, *_teipath, *_tealive;
  OXTextEdit *_teddes;
  OXGroupFrame *_gblogin;
  OXLabel *_llogin, *_lpasswd, *_laccount, *_lupload;
  OXTextEntry *_telogin, *_tepasswd, *_teaccount;
  OXCheckButton *_cbanonymous, *_cball, *_cbpassive, *_cbproxy, *_cblog, *_cbalive;
  OXDDListBox *_lbupload;
  OLayoutHints *_l1, *_l2, *_l3, *_l4, *_l41, *_l42, *_l5;
};

class OBookmarkItem : public OListViewItem {
public:
  OBookmarkItem(const OXListView *p, int id,
                std::vector <OString *> names, OBookmark *bmark,
                const OPicture *bpic, const OPicture *spic) :
    OListViewItem(p, id, bpic, spic, names, LV_SMALL_ICONS) {
      _bmark = bmark;
  }
  virtual ~OBookmarkItem() {}

  OBookmark *GetBookmark() { return _bmark; }

protected:
  OBookmark *_bmark;
};


//----------------------------------------------------------------------

class OXFtpManager : public OXTransientFrame {
public:
  OXFtpManager(const OXWindow *p, const OXWindow *main, char *homepath,
               int w, int h,
               unsigned long options = MAIN_FRAME | VERTICAL_FRAME);
  virtual ~OXFtpManager();
  
  void OpenWindow();

  virtual int CloseWindow();
  virtual int ProcessMessage(OMessage *);

  void DoSaveBookmarks();
  void DoNewBookmark();
  void DoEditBookmark();
  void DoCopyBookmark();
  void DoDeleteBookmark();
  void DoNewFolder();
  void DoRenameFolder();
  void DoDeleteFolder();
  void DoConnect();

protected:
  void SetControls();
  OBookmarkList *FindBookmarkList(char *path);
  void UpdateBookmarkList(OBookmarkList *blist);
  int ReadFile(char *file);
  int ReadSubFolders(char *name, OListTreeItem *parent, OIniFile *ini);
  int ReadFolder(char *name, char *folder, OIniFile *ini);
  int ReadBookmark(char *name, OBookmark *bmark, OIniFile *ini);
  int WriteFile(char *file);
  int WriteSubFolders(char *name, OListTreeItem *parent,
                      int &folders, int &books, OIniFile *ini);
  int WriteFolder(char *name, OListTreeItem *blist,
                  int &folders, int &books, OIniFile *ini);
  int WriteBookmark(OBookmark *bmark, OIniFile *ini);

  char *_currentFile;
  TDDLList<OBookmarkList *> *_bookmarks;
  OBookmarkList *_currentBmList;

  OXHorizontal3dLine *_toolBarSep;
  OXCompositeFrame *_frame1, *_frame2, *_frame3, *_frame4, *_fbutton;
  OXTextButton *_bsave, *_bconnect, *_bclose;
  OXTextButton *_bnewbm, *_beditbm, *_bcopybm, *_bdeletebm;
  OXTextButton *_bnewfd, *_brenamefd, *_bdeletefd;
  OXGroupFrame *_gfdetails;
  OXLabel *_lhost, *_ldetails;
  OLayoutHints *_l1, *_l2, *_l3, *_l4, *_l5, *_l6, *_l7a, *_l7b, *_l8;

  OXListView *_lv;
  OXListTree *_lt;
  const OXWindow *_mainWindow;
};

#endif
