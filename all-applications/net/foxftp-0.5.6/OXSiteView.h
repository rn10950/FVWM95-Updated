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

#ifndef __OXSITEVIEW_H
#define __OXSITEVIEW_H

#include <xclass/OXFileList.h>
#include <xclass/OXMdi.h>

class OFileSystem;
class ODir;
class OFile;
class OGeneralConfig;
class OMimeTypes;
class OListTreeItem;
class OSiteConfig;
class OTimer;

class OXLogView;
class OXFtpContainer;
class OXPopupMenu;
class OXListTree;
class OXListView;
class OXVerticalResizer;
class OXHorizontalResizer;

template <class T> class TDDLList;


//----------------------------------------------------------------------

class OFtpItem : public OFileItem {
public:
  OFtpItem(const OXListView *p, int id, 
           const OPicture *bpic, const OPicture *blpic,
           const OPicture *spic, const OPicture *slpic,
           std::vector<OString *> names, OFile *felem, int ViewMode) :
    OFileItem(p, id, bpic, blpic, spic, slpic, names,
              felem->_type, felem->_is_link, felem->_size, felem->_time,
              ViewMode) {
      _felem = felem;
  }

  OFile *GetFileInfo() { return _felem; }
  const OPicture *GetFilePicture(int small) const 
        { return (small == 1) ? _smallPic : _bigPic; }
  int EditName(char *) {return false;}

protected:
  OFile *_felem;  // who deletes this?
//  OXEditLabel *_edit;
};


class OXSiteView : public OXMdiFrame {
public:
  OXSiteView(OXMdiMainFrame *p, OMimeTypes *mime_types,
             OFileSystem *base, int w, int h,
             unsigned int options = SUNKEN_FRAME,
             unsigned long back = _defaultFrameBackground);
  virtual ~OXSiteView();

  virtual int ProcessMessage(OMessage *msg);
  virtual int HandleButton(XButtonEvent *event);
  virtual int HandleTimer(OTimer *t);
  virtual int HandleFileEvent(OFileHandler *fh, unsigned int mask);
  virtual int CloseWindow();

  void SetGeneralConfig(OGeneralConfig *genConfig);
  void SetSiteConfig(OSiteConfig *siteConfig);
  OGeneralConfig *GetGeneralConfig() { return _genConfig; }
  OSiteConfig *GetSiteConfig() { return _siteConfig; }

  void SetViewMode(int mode);
  int GetViewMode() { return _viewMode; }
  void SetSortMode(int mode);
  int GetSortMode() { return _sortMode; }

  void ChangeDirectory(const char *path);
  void DisplayDirectory(int force = false);

  OHandle *OpenFile(const char *file, int mode);
  int CloseFile(OHandle *handle);
  int Mkdir(char *dir);

  void ShowProperties();
  int Connect();
  int Reconnect();
  int Disconnect(int force = false);
  bool IsConnected();
  void DoDelete();
  void SelectAll();
  void InvertSelection();

  TDDLList<ODir *> *CreateFileList();

  const char *GetConnectName() { return _connectName; }

protected:
  void SetSiteLock(bool lock);
  void SetAliveTimer(bool set);
  void SetRetryTimer(bool set);
  int Chdir(const char *path);
  void UpdateLists(ODir *delem);
  void UpdateTree(const char *path);
  void AddFile(OFile *item);
  void Delete(int &rtc, TDDLList<ODir *> *dlist, ODir *delem);
  OListTreeItem *FindItemInLt(const char *path);

  TDDLList<ODir *> *CreateFileListFromFc();
  TDDLList<ODir *> *CreateFileListFromLt();
  int CreateSubFolderList(char *abs_path, char *rel_path, 
                          TDDLList<ODir *> *list); 

  Cursor _waitCursor;
  OFileHandler *_dirhandler;
  OFileSystem *_base;
  OGeneralConfig *_genConfig;
  OTimer *_alive_timeout, *_retry_timeout;
  OSiteConfig *_siteConfig;
  const OPicture *_recyclepic;

  int _viewMode, _sortMode;
  int _retry_count, _sys_type;
  bool _lock;
  char *_connectName;
  TDDLList<ODir *> *_cache;

  OXLogView *_log;
  OXCompositeFrame *_hframe;
  OXPopupMenu *_selectMenu, *_unselectMenu, *_treeMenu;
  OXPopupMenu *_arrangeMenu, *_viewMenu;
  OXVerticalResizer *_vres;
  OXHorizontalResizer *_hres;
  OXListTree *_lt;
  OXFileList *_lv;
};

#endif  // __OXSITEVIEW_H
