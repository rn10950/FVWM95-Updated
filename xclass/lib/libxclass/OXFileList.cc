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

#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dirent.h>
#include <unistd.h>
#include <grp.h>
#include <pwd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#include <xclass/utils.h>
#include <xclass/OMimeTypes.h>
#include <xclass/OXFrame.h>
#include <xclass/OXMsgBox.h>
#include <xclass/OXClient.h>
#include <xclass/OString.h>
#include <xclass/OLayout.h>
#include <xclass/OGC.h>
#include <xclass/OXFont.h>
#include <xclass/ODNDmanager.h>
#include <xclass/OResourcePool.h>
#include <xclass/OXFileList.h>

#include "icons/folder.s.xpm"
#include "icons/folder.t.xpm"
#include "icons/app.s.xpm"
#include "icons/app.t.xpm"
#include "icons/doc.s.xpm"
#include "icons/doc.t.xpm"
#include "icons/slink.s.xpm"
#include "icons/slink.t.xpm"
#include "icons/desktop.s.xpm"
#include "icons/desktop.t.xpm"
#include "icons/recycle-empty.s.xpm"
#include "icons/recycle-empty.t.xpm"
#include "icons/recycle-full.s.xpm"
#include "icons/recycle-full.t.xpm"


#define REFRESH_TIME   5000

Atom OXFileList::URI_list = None;


//---------------------------------------------------------------------------

OFileItem::OFileItem(const OXListView *p, int id,
      const OPicture *bpic, const OPicture *blpic,
      const OPicture *spic, const OPicture *slpic,
      std::vector <OString *>names, int type, int is_link,
      unsigned long fsize, time_t mtime, int ViewMode, int FullLine) :
  OListViewItem(p, id, bpic, spic, names, ViewMode, FullLine) {

    _lcurrent =
    _blpic = blpic;
    _slpic = slpic;
    _mtime = mtime;

    _viewMode = -1;
    SetViewMode(ViewMode);

    _type = type;
    _fsize = fsize;
    _is_link = is_link;

    _prev_state = _isSelected;
}

void OFileItem::SetViewMode(int ViewMode, int FullLine) {
  OListViewItem::SetViewMode(ViewMode, FullLine);

  if (ViewMode == LV_LARGE_ICONS)
    _lcurrent = _blpic;
  else
    _lcurrent = _slpic;

//  NeedRedraw();
}

void OFileItem::Draw(OXWindow *w, OPosition pos) {
  OListViewItem::Draw(w, pos);
  if (_lcurrent) {
    _lcurrent->Draw(_client->GetDisplay(), w->GetId(), _normGC->GetGC(),
                    pos.x + _iconPos.x, pos.y + _iconPos.y);
  }
}

void OFileItem::_SetDragPixmap(ODNDmanager *dndManager) {
  Pixmap pic, mask;
  XGCValues gcval;
  unsigned long gcmask;
  GC _picGC, _maskGC;
  int ix, iy, lx, ly;

  int w = _size.w;
  int h = _size.h;

  const OResourcePool *res = _client->GetResourcePool();

  if (_viewMode == LV_LARGE_ICONS) {
    ix = (w - _currentPic->GetWidth()) >> 1;
    iy = 0;
    lx = (w - _tw) >> 1;
    ly = h - (_th+1) - 2;
  } else {
    ix = 0;
    iy = (h - _currentPic->GetHeight()) >> 1;
    lx = _currentPic->GetWidth() + 2;
    ly = (h - (_th+1)) >> 1;
  }

  pic  = XCreatePixmap(_client->GetDisplay(), _client->GetRoot()->GetId(),
                       w, h, _client->GetDisplayDepth());
  mask = XCreatePixmap(_client->GetDisplay(), _client->GetRoot()->GetId(),
                       w, h, 1);

  gcmask = GCForeground | GCFont;
  gcval.font = _font->GetId();
  gcval.foreground = _client->GetResourcePool()->GetBlackColor(); //_whitePixel;
  _picGC = XCreateGC(_client->GetDisplay(), _client->GetRoot()->GetId(),
                     gcmask, &gcval);

  gcmask = GCForeground | GCBackground;
  gcval.foreground = 0;
  gcval.background = 0;
  _maskGC = XCreateGC(_client->GetDisplay(), mask, gcmask, &gcval);

  // Draw the pixmap...

  if (_currentPic)
    _currentPic->Draw(_client->GetDisplay(), pic, _picGC, ix, iy);

  if (_lcurrent)
    _lcurrent->Draw(_client->GetDisplay(), pic, _picGC, ix, iy);

  XDrawString(_client->GetDisplay(), pic, _picGC, lx, ly + _ta,
              _names[0]->GetString(), _names[0]->GetLength());


  // Now draw the mask: first clear the pixmap...
  XFillRectangle(_client->GetDisplay(), mask, _maskGC, 0, 0, w, h);

  // ...then draw the icon mask
  gcmask = GCForeground | GCFillStyle | GCStipple |
           GCClipMask | GCClipXOrigin | GCClipYOrigin |
           GCTileStipXOrigin | GCTileStipYOrigin;
  gcval.foreground = 1;
  gcval.fill_style = FillStippled;
  gcval.stipple = res->GetCheckeredBitmap();
  gcval.clip_mask = _currentPic->GetMask();
  gcval.clip_x_origin = ix;
  gcval.clip_y_origin = iy;
  gcval.ts_x_origin = ix;
  gcval.ts_y_origin = iy;
  XChangeGC(_client->GetDisplay(), _maskGC, gcmask, &gcval);

  XFillRectangle(_client->GetDisplay(), mask, _maskGC, ix, iy,
                 _currentPic->GetWidth(), _currentPic->GetHeight());

  if (_lcurrent) {
    gcmask = GCClipMask;
    gcval.clip_mask = _lcurrent->GetMask();
    XChangeGC(_client->GetDisplay(), _maskGC, gcmask, &gcval);

    XFillRectangle(_client->GetDisplay(), mask, _maskGC, ix, iy,
                   _lcurrent->GetWidth(), _lcurrent->GetHeight());
  }

  // ...then draw the label
  gcmask = GCFillStyle | GCClipMask | GCFont;
  gcval.fill_style = FillSolid;
  gcval.clip_mask = None;
  gcval.font = _font->GetId();
  XChangeGC(_client->GetDisplay(), _maskGC, gcmask, &gcval);

  XDrawString(_client->GetDisplay(), mask, _maskGC, lx, ly + _ta,
              _names[0]->GetString(), _names[0]->GetLength());

  XFreeGC(_client->GetDisplay(), _picGC);
  XFreeGC(_client->GetDisplay(), _maskGC);

//  ODNDmanager *dndManager = NULL;

//  if (_toplevel) dndManager = ((OXMainFrame *)_toplevel)->GetDNDmanager();

  if (dndManager) {
    dndManager->SetDragPixmap(pic, mask, w/2, 2+_currentPic->GetHeight()/2);
  } else {
    XFreePixmap(_client->GetDisplay(), pic);
    XFreePixmap(_client->GetDisplay(), mask);
  }
}

/*****************************************************
Atom OXFileItem::HandleDNDenter(Atom *typelist) {
  _prev_state = _isSelected;
  // we need to check our user/group id to see whether we
  // could really execute the file...
  if (S_ISDIR(_type) || 
      (_type & S_IXUSR) || (_type & S_IXGRP) || (_type & S_IXOTH)) {
    Activate(True);
    for (int i = 0; typelist[i] != None; ++i) {
      if (typelist[i] == OXFileList::URI_list)
        return typelist[i];
    }
  }
  return None;
}

int OXFileItem::HandleDNDleave() {
  Activate(_prev_state);
  return True;
}

int OXFileItem::HandleDNDdrop(ODNDdata *data) {
  Activate(_prev_state);
  return True;
}

Atom OXFileItem::HandleDNDposition(int x, int y, Atom action,
                                   int xroot, int yroot) {
  // directories usually would accept any of the standard xdnd actions...
  if (S_ISDIR(_type)) return action;

  // for executable files we should return XdndActionPrivate
  // but remember if the source requested XdndActionAsk in order
  // to give the user a choice using a context menu...
  if ((_type & S_IXUSR) || (_type & S_IXGRP) || (_type & S_IXOTH))
    return ODNDmanager::DNDactionPrivate;

  // otherwise refuse the drop
  return None;
}
************************************/

int OFileItem::Compare(const OListViewItem *item, int column) const {
  int type1, type2;
  OString *name1, *name2;

  switch (column) {
  case 1:
    type1 = GetFileType();
    type2 = ((OFileItem *)item)->GetFileType();

    //--- use posix macros

    if (S_ISDIR(type1)) type1 = 1;
    #if defined(S_IFLNK)
    else if ((type1 & S_IFMT) == S_IFLNK) type1 = 2;
    #endif
    #if defined(S_IFSOCK)
    else if ((type1 & S_IFMT) == S_IFSOCK) type1 = 3;
    #endif
    else if (S_ISFIFO(type1)) type1 = 4;
    else if (S_ISREG(type1) &&
            (type1 & S_IXUSR)) type1 = 5;
    else type1 = 6;
   
    if (S_ISDIR(type2)) type2 = 1;
    #if defined(S_IFLNK)
    else if ((type2 & S_IFMT) == S_IFLNK) type2 = 2;
    #endif
    #if defined(S_IFSOCK)
    else if ((type2 & S_IFMT) == S_IFSOCK) type2 = 3;
    #endif
    else if (S_ISFIFO(type2)) type2 = 4;
    else if (S_ISREG(type2) &&
            (type2 & S_IXUSR)) type2 = 5;
    else type2 = 6;

    if (type1 != type2) return (type1 - type2);
    column = 0;
    // fall thru
  default:
    name1 = GetName(column);
    name2 = item->GetName(column);

    if (!name1 || !name2) return 0;

    return strcmp(name1->GetString(), name2->GetString());

  case 3:
    return GetFileSize() - ((OFileItem *)item)->GetFileSize();

  case 6:
    return GetModificationTime() - ((OFileItem *)item)->GetModificationTime();

  } 
}


//---------------------------------------------------------------------------

OXFileList::OXFileList(const OXWindow *p, int id,
                       OMimeTypes *mime_types, char *filter,
                       int w, int h, unsigned int options) : 
  OXListView(p, w, h, id, options) {

    _widgetType = "OXFileList";
    _msgType = MSG_LISTVIEW;

    if (URI_list == None) {
      URI_list = XInternAtom(GetDisplay(), "text/uri-list", False);
    }

    MimeTypesList = mime_types;

    _sortType = SORT_BY_NAME;
    _foldersFirst = False;
    _autoRefresh = True;
    _refresh = new OTimer(this, REFRESH_TIME);

    // try loading user-defined pixmaps first...

    _folder_s  = _client->GetPicture("folder.s.xpm");
    _folder_t  = _client->GetPicture("folder.t.xpm");
    _app_s     = _client->GetPicture("app.s.xpm");
    _app_t     = _client->GetPicture("app.t.xpm");
    _doc_s     = _client->GetPicture("doc.s.xpm");
    _doc_t     = _client->GetPicture("doc.t.xpm");
    _slink_s   = _client->GetPicture("slink.s.xpm");
    _slink_t   = _client->GetPicture("slink.t.xpm");
    _desktop_s = _client->GetPicture("desktop.s.xpm");
    _desktop_t = _client->GetPicture("desktop.t.xpm");
    _rbempty_s = _client->GetPicture("recycle-empty.s.xpm");
    _rbempty_t = _client->GetPicture("recycle-empty.t.xpm");
    _rbfull_s  = _client->GetPicture("recycle-full.s.xpm");
    _rbfull_t  = _client->GetPicture("recycle-full.t.xpm");

    // use xclass default pixmaps for the missing ones...

    if (!_folder_s)
      _folder_s  = _client->GetPicture("folder.s.xpm", XCP_folder_s_xpm);
    if (!_folder_t)
      _folder_t  = _client->GetPicture("folder.t.xpm", XCP_folder_t_xpm);
    if (!_app_s)
      _app_s     = _client->GetPicture("app.s.xpm", XCP_app_s_xpm);
    if (!_app_t)
      _app_t     = _client->GetPicture("app.t.xpm", XCP_app_t_xpm);
    if (!_doc_s)
      _doc_s     = _client->GetPicture("doc.s.xpm", XCP_doc_s_xpm);
    if (!_doc_t)
      _doc_t     = _client->GetPicture("doc.t.xpm", XCP_doc_t_xpm);
    if (!_slink_s)
      _slink_s   = _client->GetPicture("slink.s.xpm", XCP_slink_s_xpm);
    if (!_slink_t)
      _slink_t   = _client->GetPicture("slink.t.xpm", XCP_slink_t_xpm);
    if (!_desktop_s)
      _desktop_s = _client->GetPicture("desktop.s.xpm", XCP_desktop_s_xpm);
    if (!_desktop_t)
      _desktop_t = _client->GetPicture("desktop.t.xpm", XCP_desktop_t_xpm);
    if (!_rbempty_s)
      _rbempty_s = _client->GetPicture("recycle-empty.s.xpm", XCP_recycle_empty_s_xpm);
    if (!_rbempty_t)
      _rbempty_t = _client->GetPicture("recycle-empty.t.xpm", XCP_recycle_empty_t_xpm);
    if (!_rbfull_s)
      _rbfull_s  = _client->GetPicture("recycle-full.s.xpm", XCP_recycle_full_s_xpm);
    if (!_rbfull_t)
      _rbfull_t  = _client->GetPicture("recycle-full.t.xpm", XCP_recycle_full_t_xpm);

    if (!_folder_s  || !_folder_t ||
        !_app_s     || !_app_t    ||
        !_doc_s     || !_doc_t    ||
        !_slink_s   || !_slink_t  ||
        !_desktop_s || !_desktop_t ||
        !_rbempty_s || !_rbempty_t ||
        !_rbfull_s  || !_rbfull_t)
      FatalError("OXFileList: Missing required pixmap(s).");

    _compileFilter(filter);

    const char *uroot = _client->GetResourcePool()->GetUserRoot();
    if (uroot) {
      _desktopPath = new char[strlen(uroot)+strlen("/desktop")+1];
      sprintf(_desktopPath, "%s/desktop", uroot);
      _recyclePath = new char[strlen(uroot)+strlen("/recycle")+1];
      sprintf(_recyclePath, "%s/recycle", uroot);
    } else {
      _desktopPath = NULL;
      _recyclePath = NULL;
    }

    AddColumn(new OString("Name"), 0, TEXT_LEFT);
    AddColumn(new OString("Attributes"), 2);
    AddColumn(new OString("Last Modified"), 6);
    AddColumn(new OString("Size"), 3, TEXT_RIGHT);
    AddColumn(new OString("Owner"), 4);
    AddColumn(new OString("Group"), 5);
    AddColumn(new OString(""), 1000);

    _bdown = False;

    _showDotFiles = True;

    _dragging = False;
    _dragOver = NULL;

    if (_toplevel) _dndManager = ((OXMainFrame *)_toplevel)->GetDNDmanager();
}

OXFileList::~OXFileList() {
  if (_refresh) delete _refresh;
  _client->FreePicture(_folder_s);
  _client->FreePicture(_folder_t);
  _client->FreePicture(_app_s);
  _client->FreePicture(_app_t);
  _client->FreePicture(_doc_s);
  _client->FreePicture(_doc_t);
  _client->FreePicture(_slink_s);
  _client->FreePicture(_slink_t);
  _client->FreePicture(_desktop_s);
  _client->FreePicture(_desktop_t);
  _client->FreePicture(_rbempty_s);
  _client->FreePicture(_rbempty_t);
  _client->FreePicture(_rbfull_s);
  _client->FreePicture(_rbfull_t);
  if (_recyclePath) delete[] _recyclePath;
  if (_desktopPath) delete[] _desktopPath;
}

void OXFileList::AutoRefresh(int onoff) {
  _autoRefresh = onoff;
  if (_autoRefresh) {
    if (!_refresh) _refresh = new OTimer(this, REFRESH_TIME);
  } else {
    if (_refresh) delete _refresh;
    _refresh = NULL;
  }
}

void OXFileList::SetFileFilter(const char *filter, int update) {
  _compileFilter(filter);
  if (update) DisplayDirectory();
}

void OXFileList::ShowDotFiles(int onoff) {
  if (_showDotFiles != onoff) {
    _showDotFiles = onoff;
    DisplayDirectory();
  }
}

int OXFileList::HandleTimer(OTimer *t) {
  struct stat sbuf;

  if (t != _refresh) return OXListView::HandleTimer(t);

  if (stat(".", &sbuf) == 0)
    if (_st_mtime != sbuf.st_mtime) DisplayDirectory();

  delete _refresh;
  _refresh = new OTimer(this, REFRESH_TIME);

  return True;
}

void OXFileList::Sort(int sortType) {
  int col;

  _sortType = sortType;

  switch (_sortType) {
    default:
    case SORT_BY_NAME: col = 0; break;
    case SORT_BY_TYPE: col = 1; break;
    case SORT_BY_SIZE: col = 3; break;
    case SORT_BY_DATE: col = 6; break;
  }

  SortColumn(col, LV_ASCENDING);
}


//---- Determine the file picture for the given file type.

void OXFileList::GetFilePictures(const OPicture **pic,
             const OPicture **lpic, int file_type, int is_link, 
             char *name, int small) {
  char temp[BUFSIZ];

#if 0
  *pic = MimeTypesList->GetIcon(name, small);
  if (*pic == NULL) {
    *pic = small ? _doc_t : _doc_s;
    if (S_ISREG(file_type) &&
               (file_type) & S_IXUSR) {
      if (small) {
        sprintf(temp, "%s.t.xpm", name);
        *pic = _client->GetPicture(temp);
        if (*pic == NULL) *pic = _app_t;
      } else {
        sprintf(temp, "%s.s.xpm", name);
        *pic = _client->GetPicture(temp);
        if (*pic == NULL) *pic = _app_s;
      }
    }
    if (S_ISDIR(file_type))
      *pic = small ? _folder_t : _folder_s;
  }
#else
  if (S_ISDIR(file_type)) {
    *pic = small ? _folder_t : _folder_s;
  } else {
    *pic = MimeTypesList->GetIcon(name, small);
    if (!*pic) {
      *pic = small ? _doc_t : _doc_s;
      if (S_ISREG(file_type) &&
                 (file_type) & S_IXUSR) {
        if (small) {
          sprintf(temp, "%s.t.xpm", name);
          *pic = _client->GetPicture(temp);
          if (!*pic) *pic = _app_t;
        } else {
          sprintf(temp, "%s.s.xpm", name);
          *pic = _client->GetPicture(temp);
          if (!*pic) *pic = _app_s;
        }
      }
    }
  }
#endif

  if (is_link)
    *lpic = small ? _slink_t : _slink_s;
  else
    *lpic = NULL;
}


//---- Change current directory

int OXFileList::ChangeDirectory(const char *path) {
  if (chdir(path)) return errno;
  DisplayDirectory();
  return 0;
}


//---- Display the contents of the current directory in the container.
//     This can be used to refresh the contents of the window.
//     If check is True, the directory will be re-read only if it
//     was modified since the last call.

void OXFileList::DisplayDirectory(int check) {
  struct stat sbuf;

  if (check)
    if (stat(".", &sbuf) == 0)
      if (_st_mtime == sbuf.st_mtime) return;

  Clear();
  _CreateFileList();
  Sort(_sortType);

  //---- notify application about the changes
//  OContainerMessage message(_msgType, MSG_SELCHANGED, -1, 0,
//                            _total, _selected);
//  SendMessage(_msgObject, &message);

  Layout();
}


//---- This function creates the file list from the current directory

void OXFileList::_CreateFileList() {
  DIR *dirp;
  struct stat sbuf, lbuf;
  struct dirent *dp;
  int type, is_link, uid, gid;
  bool in_userRoot;
  unsigned long size;
  char *name, filename[PATH_MAX];
  const OPicture *pic, *lpic, *spic, *slpic;

  if (stat(".", &sbuf) == 0) _st_mtime = sbuf.st_mtime;

  if ((dirp = opendir(".")) == NULL) return;

  getcwd(filename, PATH_MAX);
  in_userRoot = (strcmp(filename, _client->GetResourcePool()->GetUserRoot()) == 0);

  int i = 0;
  std::vector<OString *> names;

  while ((dp = readdir(dirp)) != NULL) {
    name = dp->d_name;
    if (strcmp(name, ".") && strcmp(name, "..")) {
      if ((name[0] == '.') && !_showDotFiles) continue;
      strcpy(filename, name);
      type = 0;
      size = 0;
      is_link = False;
//      if (lstat(name, &sbuf) == 0) {
      if ((stat(name, &sbuf) == 0) || (lstat(name, &sbuf) == 0)) {
        if (!S_ISDIR(sbuf.st_mode) &&
          FileMatch(name) == 0) continue;
        if (lstat(name, &lbuf) != 0) continue;
        is_link = S_ISLNK(lbuf.st_mode);
        type = lbuf.st_mode;
        size = lbuf.st_size;
        uid = lbuf.st_uid;
        gid = lbuf.st_gid;
        if (is_link) {
          type = sbuf.st_mode;
          size = sbuf.st_size;
        }
      } else {
        char msg[256];

        sprintf(msg, "Can't read file attributes of \"%s\": %s.",
                     name, strerror(errno));
        new OXMsgBox(_client->GetRoot(), _toplevel,
                     new OString("Error"), new OString(msg),
                     MB_ICONSTOP, ID_OK);
      }

      pic = spic = NULL;

      if (in_userRoot && _desktopPath && _recyclePath) {
        if (strcmp(filename, "desktop") == 0) {
          pic  = _desktop_s;
          spic = _desktop_t;
        } else if (strcmp(filename, "recycle") == 0) {
          if (IsEmptyDir(filename)) {
            pic  = _rbempty_s;
            spic = _rbempty_t;
          } else {
            pic  = _rbfull_s;
            spic = _rbfull_t;
          }
        }
        lpic  = is_link ? _slink_s : NULL;
        slpic = is_link ? _slink_s : NULL;
      }

      if (!pic || !spic) {
        GetFilePictures(&pic, &lpic, type, is_link, filename, False);
        GetFilePictures(&spic, &slpic, type, is_link, filename, True);
      }

      names.push_back(new OString(filename));
      names.push_back(new OString("")); // this is a dummy column
                                        // used to sort by type
      names.push_back(new OString(AttributeString(type, is_link)));
      names.push_back(new OString(SizeString(size)));
      names.push_back(new OString(Owner(uid)));
      names.push_back(new OString(Group(gid)));
      names.push_back(new OString(TimeString(sbuf.st_mtime)));
      AddItem(new OFileItem(this, i++, pic, lpic, spic, slpic,
                            names, type, is_link, size, sbuf.st_mtime,
                            _viewMode));
      names.clear();

      //_total++;
    }
  }

  closedir(dirp);
}

void OXFileList::_compileFilter(const char *filter) {
  char  expression[PATH_MAX];
  int   pos = 0;
  const char *ptr = NULL;

  if (!filter) filter = "*";

  ptr = filter;
  while (*ptr != '\0') {
    if (*ptr == '*' || *ptr == '?')
      expression[pos++] = '.';
    else if (*ptr == '.')
      expression[pos++] = '\\';
    if (*ptr != '?') expression[pos++] = *ptr;
    ptr++;
  }
  expression[pos] = '\0';
  regcomp(&_filter, expression, REG_EXTENDED);
}

int OXFileList::FileMatch(const char *filename) {
  regmatch_t pmatch[1];
  size_t     nmatch = 1;

  if (filename) {
    if (regexec(&_filter, filename, nmatch, pmatch, 0) == 0) {
      if ((pmatch[0].rm_so == 0) &&
          (pmatch[0].rm_eo == strlen(filename))) {
        return 1;
      }
    }
  }
  return 0;
}

int OXFileList::IsEmptyDir(const char *dir) {
  DIR *d;
  struct dirent *dp;

  d = opendir(dir);
  if (!d) return True;

  while ((dp = readdir(d)) != NULL) {
    if (strcmp(dp->d_name, ".") && strcmp(dp->d_name, "..")) {
      closedir(d);
      return False;
    }
  }

  closedir(d);
  return True;
}

char *OXFileList::AttributeString(int type, int is_link) {
  static char tmp[12];

  sprintf(tmp, "%c%c%c%c%c%c%c%c%c%c",
                (is_link ?
                 'l' : 
                 (S_ISREG(type) ?
                  '-' : 
                  (S_ISDIR(type) ?
                   'd' : 
                    (S_ISCHR(type) ?
                     'c' : 
                     (S_ISBLK(type) ?
                      'b' : 
                      (S_ISFIFO(type) ?
                       'p' : 
                       (S_ISSOCK(type) ?
                        's' : '?' ))))))),
                 ((type & S_IRUSR) ? 'r' : '-'),
                 ((type & S_IWUSR) ? 'w' : '-'),
                 ((type & S_ISUID) ? 's' : ((type & S_IXUSR) ? 'x' : '-')),
                 ((type & S_IRGRP) ? 'r' : '-'),
                 ((type & S_IWGRP) ? 'w' : '-'),
                 ((type & S_ISGID) ? 's' : ((type & S_IXGRP) ? 'x' : '-')),
                 ((type & S_IROTH) ? 'r' : '-'),
                 ((type & S_IWOTH) ? 'w' : '-'),
                 ((type & S_ISVTX) ? 't' : ((type & S_IXOTH) ? 'x' : '-')));

  return tmp;
}

char *OXFileList::SizeString(unsigned long size) {
  static char tmp[30];
  unsigned long fsize = size;

  if (fsize > 1024) {
    fsize /= 1024;
    if (fsize > 1024) {
      // 3.7MB is more informative than just 3MB
      sprintf(tmp, "%ld.%ldM", fsize/1024, (fsize%1024)/103);
    } else {
      sprintf(tmp, "%ld.%ldK", size/1024, (size%1024)/103);
    }
  } else {
    sprintf(tmp, "%ld", size);
  }

  return tmp;
}

char *OXFileList::Owner(int uid) {
  struct passwd *pwd;
  static char tmp[256];

  pwd = getpwuid(uid);
  if (pwd)
    strcpy(tmp, pwd->pw_name);
  else
    sprintf(tmp, "%d", uid);

  return tmp;
}

char *OXFileList::Group(int gid) {
  struct group *grp;
  static char tmp[256];

  grp = getgrgid(gid);
  if (grp)
    strcpy(tmp, grp->gr_name);
  else
    sprintf(tmp, "%d", gid);

  return tmp;
}

char *OXFileList::TimeString(time_t tm) {
  static char tmp[256];
  struct tm *tms;

  tms = localtime(&tm);
  strftime(tmp, 255, "%b %d, %Y  %R", tms);

  return tmp;
}

//----------------------------------------------------------------------

// DnD operations...

Atom OXFileList::HandleDNDenter(Atom *typelist) {
  int i;

  for (i = 0; typelist[i] != None; ++i) {
    if (typelist[i] == URI_list) return URI_list;
  }
  return None;

}

int OXFileList::HandleDNDleave() {
  if (_dragOver) {
    SelectItem(_dragOver, _dragOverPrevState);
    _dragOver = NULL;
  }
  return True;
}

Atom OXFileList::HandleDNDposition(int x, int y, Atom action,
                                   int xroot, int yroot) {
  int nx = 0, ny = 0;

  _dropPos = OPosition(xroot, yroot);

  ((OXMainFrame *)_toplevel)->TranslateCoordinates(_canvas, x, y, &nx, &ny);

  OFileItem *fi = (OFileItem *) GetItemByPos(ToVirtual(OPosition(nx, ny)));

  if (fi != _dragOver) {
    // leaving _dragOver
    if (_dragOver)
      SelectItem(_dragOver, _dragOverPrevState);

    _dragOver = fi;
    // entering new _dragOver
    if (_dragOver) {
      _dragOverPrevState = fi->IsSelected();
      int type = _dragOver->GetFileType();

#if 0
      // we need to check our user/group id to see whether we
      // could really execute the file...
      if (S_ISDIR(type) || 
          (type & S_IXUSR) || (type & S_IXGRP) || (type & S_IXOTH)) {
        _dragOver->Select(True);
        return action;
      }
#else
      SelectItem(_dragOver, True);

      // directories usually would accept any of the standard xdnd actions...
      if (S_ISDIR(type)) return action;

      // for executable files we should return XdndActionPrivate
      // but remember if the source requested XdndActionAsk in order
      // to give the user a choice using a context menu...
      if ((type & S_IXUSR) || (type & S_IXGRP) || (type & S_IXOTH))
        return ODNDmanager::DNDactionPrivate;

      // otherwise refuse the drop
      return None;
#endif
    }
  }

  return action;
}

int OXFileList::HandleDNDdrop(ODNDdata *data) {
  if (_dragOver)
    SelectItem(_dragOver, _dragOverPrevState);

  ODNDmessage msg(_msgType, MSG_DROP, _id, data, data->action,
                  _dropPos, _dragOver);
  SendMessage(_msgObject, &msg);

  _dragOver = NULL;

  return True;
}

int OXFileList::HandleDNDfinished() {
  DisplayDirectory(True);
  return True;
}

int OXFileList::HandleButton(XButtonEvent *event) {

  OXListView::HandleButton(event);

  if ((event->button == Button4) || (event->button == Button5)) return True;

  if (event->type == ButtonPress) {
    //_xp = event->x;
    //_yp = event->y;
    _bdown = True;

  } else {  // ButtonRelease
    if (_dragging) _dndManager->Drop();
    _bdown = False;
    _dragging = False;

  }
  return True;
}

int OXFileList::HandleMotion(XMotionEvent *event) {

  OXListView::HandleMotion(event);

  int _xp = _selAnchor.x;
  int _yp = _selAnchor.y;

  if (_dragSelecting) return True;

  if (!_dragging) {
    if (_bdown && ((abs(event->x - _xp) > 3) || (abs(event->y - _yp) > 3))) {
      if (_dndManager && _anchorItem) {
        ((OFileItem *)_anchorItem)->_SetDragPixmap(_dndManager);
        _dragging = _dndManager->StartDrag(this, event->x_root, event->y_root, _id);
      }
    }
  }
  if (_dragging) {
    int action;

    if (event->state & Button3Mask) {
      action = ODNDmanager::DNDactionAsk;
    } else {
      if (event->state & ControlMask) {
        action = ODNDmanager::DNDactionCopy;
      } else {
        action = ODNDmanager::DNDactionMove;
      }
    }
    _dndManager->Drag(event->x_root, event->y_root, action, event->time);
  }
  return True;
}

ODNDdata *OXFileList::GetDNDdata(Atom dataType) {
  static ODNDdata data;
  static char str[1024];

  if (_anchorItem && dataType == URI_list) {
    char cwd[PATH_MAX];

    sprintf(str, "file://localhost/%s/%s\r\n",
                 getcwd(cwd, PATH_MAX),
                 ((OFileItem *)_anchorItem)->GetName(0)->GetString());
    data.data = (void *) str;
    data.dataLength = strlen(str);
    data.dataType = dataType;

    return &data;

  } else {

    return NULL;

  }
}
