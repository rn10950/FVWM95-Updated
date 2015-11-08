/**************************************************************************

    This file is part of a xclass desktop manager.
    Copyright (C) 1996-2000 David Barth, Hector Peraza.

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

#ifndef __OXDESKTOPCONTAINER_H
#define __OXDESKTOPCONTAINER_H

#include <X11/Xlib.h>

#include <xclass/OXCompositeFrame.h>
#include <xclass/OXMenu.h>
#include <xclass/OXListView.h>
#include <xclass/OString.h>
#include <xclass/OPicture.h>
#include <xclass/OMimeTypes.h>
#include <xclass/ORectangle.h>


//----- sort types

#define SORT_BY_NAME  0
#define SORT_BY_TYPE  1
#define SORT_BY_SIZE  2
#define SORT_BY_DATE  3


class OXFont;
class OXGC;
class OTimer;

extern OXPopupMenu *objectMenu;

class OXDesktopIcon;
class OXDesktopMain;
class ORecycledFiles;
class URL;
class ODNDmanager;

//-------------------------------------------------------------------

class OXDesktopContainer : public OXCompositeFrame {
protected:
  static OXGC *_lineGC;

public:
  OXDesktopContainer(const OXWindow *p, OXDesktopMain *main,
                     ODNDmanager *dnd);
  virtual ~OXDesktopContainer();

  virtual OXDesktopIcon *AddIcon(const OPicture *pic, const OPicture *lpic,
                                 OString *name, int type, unsigned long size);
  virtual OXDesktopIcon *NewIcon(int x, int y, URL *url, int ftype, int action);

  virtual int CreateObject(int x, int y, int objtype);  ////

  virtual int HandleButton(XButtonEvent *event);
  virtual int HandleDoubleClick(XButtonEvent *event);
  virtual int HandleMotion(XMotionEvent *event);
  virtual int HandleTimer(OTimer *t);

  virtual OXFrame *GetFrameFromPoint(int x, int y);

  virtual Atom HandleDNDenter(Atom *typelist);
  virtual int  HandleDNDleave();
  virtual Atom HandleDNDposition(int x, int y, Atom action, int xr, int yr);
  virtual int  HandleDNDdrop(ODNDdata *DNDdata);
  virtual int  HandleDNDfinished();

  void Init();
  void Save();

  void Sort(int sortType);
  void ChangeDirectory(const char *path);
  void DisplayDirectory();
  void RemoveAllIcons();
  void UnselectAll();
  void ArrangeIcons();
  
  virtual const OXDesktopIcon *GetNextSelected(void **current);
  virtual int NumItems() const { return _total; }
  virtual int NumSelected() const { return _selected; }

  void SelectInRectangle(ORectangle rect, int add = False);
  void DeleteSelectedFiles();
  void Rename(OXDesktopIcon *f);
  void DoAction(OXDesktopIcon *f);

private:
  void GetIconPics(const OPicture **pic, const OPicture **lpic,
                   char *fname, int ftype, int is_link);
  SListFrameElt **_doSort(SListFrameElt **head, int n);
  int _compare(SListFrameElt *p1, SListFrameElt *p2);
  int _isRecycleBin(const char *path);
  int _isEmptyDir(const char *path);
  void CreateIcons();

protected:
  OMimeTypes *MimeTypesList;

  OTimer *_refresh;
  OXDesktopMain *_desktopMain;
  OXDesktopIcon *_last_active;
  ODNDmanager *_dndManager;
  OXPopupMenu *_popup, *_subpopup, *_dndpopup;
  OLayoutHints *_fl;

  int _xp, _yp, _x0, _xf, _y0, _yf, _dndx, _dndy;
  int _total, _selected, _dragging, _sortType;
  unsigned long _st_mtime, _rb_mtime;
  char *_inifile;

  const OPicture *_folder, *_app, *_doc, *_slink, *_rbempty, *_rbfull;
  ORecycledFiles *_recycled;
};


#endif  // __OXDESKTOPCONTAINER_H
