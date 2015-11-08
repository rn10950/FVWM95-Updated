/**************************************************************************

    This file is part of xclass.
    Copyright (C) 1996-2000 Hector Peraza.

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

#ifndef __OXFILELIST_H
#define __OXFILELIST_H

#include <vector>

#include <regex.h>
#include <X11/Xlib.h>

#include <xclass/OXCompositeFrame.h>
#include <xclass/OXWidget.h>
#include <xclass/OXIcon.h>
#include <xclass/OString.h>
#include <xclass/OPicture.h>
#include <xclass/OMimeTypes.h>
#include <xclass/OXListView.h>
#include <xclass/OMessage.h>


//----- sort types

#define SORT_BY_NAME  0
#define SORT_BY_TYPE  1
#define SORT_BY_SIZE  2
#define SORT_BY_DATE  3


//----------------------------------------------------------------------

class ODNDmanager;
class OFileItem;

class ODNDmessage : public OWidgetMessage {
public:
  ODNDmessage(int typ, int act, int id, ODNDdata *d, Atom a,
              OPosition p, OFileItem *i) :
    OWidgetMessage(typ, act, id) {
      data = d; dndAction = a; pos = p; dragOver = i;
  }

public:
  ODNDdata *data;
  Atom dndAction;
  OFileItem *dragOver;
  OPosition pos;
};


//----------------------------------------------------------------------

class OFileItem : public OListViewItem {
public:
  OFileItem(const OXListView *p, int id,
            const OPicture *bpic, const OPicture *blpic,
            const OPicture *spic, const OPicture *slpic,
            std::vector<OString *> names, int type, int is_link, 
            unsigned long fsize, time_t mtime,
            int ViewMode, int FullLine = False);

  virtual void SetViewMode(int ViewMode, int fullLine = False);

//  int IsActive() const { return _active; }
  int IsSymLink() const { return _is_link; }
  int GetFileType() const { return _type; }
  int GetFileSize() const { return _fsize; }
  time_t GetModificationTime() const { return _mtime; }

//  void DoEdit() { _label->DoEdit(); }

  void _SetDragPixmap(ODNDmanager *dndManager);

  virtual void Draw(OXWindow *w, OPosition pos);

  virtual int Compare(const OListViewItem *item, int column) const;

protected:
  const OPicture *_blpic, *_slpic, *_lcurrent;
  int _type, _is_link, _prev_state;
  unsigned long _fsize;
  time_t _mtime;

  int *_ctw;
};


//----------------------------------------------------------------------

class OXFileList : public OXListView {
public:
  static Atom URI_list;

public:
  OXFileList(const OXWindow *p, int id, OMimeTypes *mime_types,
             char *filter, int w, int h,
             unsigned int options = SUNKEN_FRAME | DOUBLE_BORDER | OWN_BKGND);
  virtual ~OXFileList();

  virtual int HandleButton(XButtonEvent *event);
  virtual int HandleMotion(XMotionEvent *event);

  virtual int HandleTimer(OTimer *t);

  void Sort(int sortType);
  void AutoRefresh(int onoff = True);

  int  ChangeDirectory(const char *path);
  void DisplayDirectory(int check = False);
  void SetFileFilter(const char *filter, int update = True);
  void ShowDotFiles(int onoff = True);

  void GetFilePictures(const OPicture **pic, const OPicture **lpic, 
                       int file_type, int is_link, char *ext, int small);

  virtual Atom HandleDNDenter(Atom *typelist);
  virtual int  HandleDNDleave();
  virtual Atom HandleDNDposition(int x, int y, Atom action, int xr, int yr);
  virtual int  HandleDNDdrop(ODNDdata *DNDdata);
  virtual int  HandleDNDfinished();
  virtual ODNDdata *GetDNDdata(Atom dataType);

  char *AttributeString(int type, int is_link);
  char *SizeString(unsigned long size);
  char *Owner(int uid);
  char *Group(int gid);
  char *TimeString(time_t tm);

  int FileMatch(const char *filename);
  int IsEmptyDir(const char *dir);
  
  int IsDragging() const { return _dragging; }

protected:
  void _CreateFileList();
  void _compileFilter(const char *filter);

  int _sortType, _foldersFirst, _showDotFiles, _bdown;
  regex_t _filter;
  OTimer *_refresh;
  unsigned long _st_mtime;
  OMimeTypes *MimeTypesList;
  const OPicture *_folder_t,  *_folder_s;
  const OPicture *_app_t,     *_app_s;
  const OPicture *_doc_t,     *_doc_s;
  const OPicture *_slink_t,   *_slink_s;
  const OPicture *_desktop_t, *_desktop_s;
  const OPicture *_rbempty_t, *_rbempty_s;
  const OPicture *_rbfull_t,  *_rbfull_s;
  ODNDmanager *_dndManager;
  char *_recyclePath, *_desktopPath;

  int _autoRefresh, _dragging, _dragOverPrevState;
  OFileItem *_dragOver;
  OPosition _dropPos;
};


#endif  // __OXFILELIST_H
