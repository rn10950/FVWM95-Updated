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
#include <sys/types.h>
#include <sys/stat.h>

#include <dirent.h>

#include <xclass/utils.h>
#include <xclass/OMimeTypes.h>
#include <xclass/OString.h>
#include <xclass/OPicture.h>
#include <xclass/OResourcePool.h>
#include <xclass/OXClient.h>
#include <xclass/OXMsgBox.h>
#include <xclass/ODNDmanager.h>

#include "URL.h"
#include "OXDirectoryTree.h"
#include "OXExplorer.h"


extern struct _default_icon default_icon[];


//----------------------------------------------------------------------

OXDirectoryTree::OXDirectoryTree(const OXWindow *p, int w, int h, int ID,
                                 unsigned long options) :
  OXListTree(p, w, h, ID, options) {

//  _dndTypeList = new Atom[2];

//  _dndTypeList[0] = XInternAtom(GetDisplay(), "text/uri-list", False);
//  _dndTypeList[1] = NULL;

//  _dndManager = new ODNDmanager(_client, this, _dndTypeList);

#ifdef IDLE_UPDATE
    _idle = NULL;
#endif
}

OXDirectoryTree::~OXDirectoryTree() {
  if (_idle) delete _idle;
}


//--- Update the filesystem list tree contents

void OXDirectoryTree::UpdateTree() {
#ifdef IDLE_UPDATE
  _currentDir[0] = '/';
  getcwd(&_currentDir[1], PATH_MAX);

  _dir = "/";

  _i = FindChildByName(NULL, _dir);
  if (!_i) {
    // make entry for root dir
    _i = AddItem(NULL, _dir,
                 _client->GetPicture("fdisk.t.xpm"),
                 _client->GetPicture("fdisk.t.xpm"));
  }
  OpenNode(_i);
  if (strcmp(&_currentDir[1], "/") == 0) HighlightItem(_i, True);

  DefineCursor(GetResourcePool()->GetWaitCursor());
  XFlush(GetDisplay());

  _dir = _currentDir;
  _p = strchr(_dir + 1, '/');
  if (_p) *_p++ = '\0';
  _tmp[0] = '\0';

  _idle = new OIdleHandler(this);
#else
  OListTreeItem *i;
  char *p, *dir = "/";
  char wd[PATH_MAX+1];
  char tmp[PATH_MAX];

  wd[0] = '/';
  getcwd(&wd[1], PATH_MAX);

  i = FindChildByName(NULL, dir);
  if (!i) {
    // make entry for root dir
    i = AddItem(NULL, dir,
                _client->GetPicture("fdisk.t.xpm"),
                _client->GetPicture("fdisk.t.xpm"));
  }
  OpenNode(i);
  HighlightItem(i, True);

  DefineCursor(GetResourcePool()->GetWaitCursor());
  XFlush(GetDisplay());

  dir = wd;
  p = strchr(dir + 1, '/');
  if (p) *p++ = '\0';
  tmp[0] = '\0';
  while (dir && *dir) {
    if (tmp[strlen(tmp)-1] != '/') strcat(tmp, "/");
    strcat(tmp, dir);
    ReadDir(tmp, i);
    dir = p;
    if (p) p = strchr(p, '/');
    if (p) *p++ = '\0';
    if (dir) {
      i = FindChildByName(i, dir);
      if (i) {
        if (p) OpenNode(i);
        //HighlightItem(i);
        //SortChildren(i);
      }
    }
  }
  if (i) HighlightItem(i, True);
  SortChildren(NULL);
  DefineCursor(None);
  XFlush(GetDisplay());
#endif
}

int OXDirectoryTree::HandleIdleEvent(OIdleHandler *ih) {
#ifdef IDLE_UPDATE

  if (ih != _idle) return False;

  delete _idle;
  _idle = NULL;

  if (_tmp[strlen(_tmp)-1] != '/') strcat(_tmp, "/");
  strcat(_tmp, _dir);
  ReadDir(_tmp, _i);
  _dir = _p;
  if (_p) _p = strchr(_p, '/');
  if (_p) *_p++ = '\0';
  if (_dir) {
    _i = FindChildByName(_i, _dir);
    if (_i) {
      if (_p) OpenNode(_i);
      //HighlightItem(_i, True);
      //SortChildren(_i);
    }
  }

  if (_dir && *_dir) {
    _idle = new OIdleHandler(this);
  } else {
    if (_i) HighlightItem(_i, True);
    SortChildren(NULL);
    DefineCursor(None);
    XFlush(GetDisplay());
  }

#endif
  return True;
}

void OXDirectoryTree::ReadDir(char *cdir, OListTreeItem *parent) {
  DIR *dirp;
  struct stat sbuf;
  struct dirent *dp;
  char *name;
  char filename[PATH_MAX];
  OListTreeItem *current;
  const OPicture *pic1 = NULL, *pic2 = NULL;

  //DeleteChildren(parent);

//printf("ReadDir: entering %s...\n", cdir);

  if ((dirp = opendir(cdir)) == NULL) return;

  while ((dp = readdir(dirp)) != NULL) {
    name = dp->d_name;
    if (strcmp (name, ".") && strcmp (name, "..")) {
      sprintf(filename, "%s/%s", cdir, name);
      if ((stat(filename, &sbuf) == 0) || (lstat(filename, &sbuf) == 0)) {
        if (S_ISDIR(sbuf.st_mode)) {
          current = FindChildByName(parent, name);
          if (!current) {
            // here we should test for the full path name!  ==!==
            if (strcmp(name, ".desktop") == 0) {
              pic1 = _client->GetPicture("desktop.t.xpm");
              pic2 = _client->GetPicture("desktop.t.xpm");
            // and here as well... ==!==
            } else if (strcmp(name, ".recycle") == 0) {
              // here we should check for empty recycle bin! ==!==
              pic1 = _client->GetPicture("recycle-empty.t.xpm");
              pic2 = _client->GetPicture("recycle-empty.t.xpm");
            } else {
              pic1 = NULL;
              pic2 = NULL;
            }
            current = AddItem(parent, name, pic1, pic2);
          }
          ReadSubDirs(filename, current);
          SortChildren(current);
        }
      } else {
        fprintf(stderr, "lstat: \"%s\": ", name);
        perror("");
      }
    }
  }

  SortChildren(parent);
  closedir(dirp);
}

void OXDirectoryTree::ReadSubDirs(char *cdir, OListTreeItem *parent) {
  DIR *dirp;
  struct stat sbuf;
  struct dirent *dp;
  char *name, filename[PATH_MAX];
  OListTreeItem *current;

//printf("ReadSubDirs: entering %s...\n", cdir);
////  DeleteChildren(parent);

  if ((dirp = opendir(cdir)) == NULL) return;

  while ((dp = readdir(dirp)) != NULL) {
    name = dp->d_name;
    if (strcmp (name, ".") && strcmp (name, "..")) {
      sprintf(filename, "%s/%s", cdir, name);
      if (lstat(filename, &sbuf) == 0) {
        if (S_ISDIR(sbuf.st_mode)) {
          current = FindChildByName(parent, name);
          if (!current) {
            AddItem(parent, name);
            // break;
          }
        }
      } else {
        fprintf(stderr, "lstat: \"%s\" -- ", name);
        perror("stat");
      }
    }
  }

  closedir(dirp);
}
