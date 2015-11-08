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

#ifndef __OXDIRECTORYTREE_H
#define __OXDIRECTORYTREE_H

#include <X11/cursorfont.h>

#include <xclass/OXClient.h>
#include <xclass/OString.h>
#include <xclass/OPicture.h>
#include <xclass/OXListTree.h>
#include <xclass/OIdleHandler.h>
#include <xclass/utils.h>


#define IDLE_UPDATE


//---------------------------------------------------------------------

class OXDirectoryTree : public OXListTree {
public:
  OXDirectoryTree(const OXWindow *p, int w, int h, int ID,
                  unsigned long options);
  virtual ~OXDirectoryTree();

  virtual int HandleIdleEvent(OIdleHandler *i);

//  void SetViewMode(int new_mode, int force = False);
//  void SetSortMode(int new_mode);

  void UpdateTree();
  void DoChangeDirectory(const char *path);

  void ReadDir(char *cdir, OListTreeItem *parent);

protected:
  void ReadSubDirs(char *cdir, OListTreeItem *parent);

  char _currentDir[PATH_MAX];
//  Atom *_dndTypeList;
#ifdef IDLE_UPDATE
  OIdleHandler *_idle;
  char *_p, *_dir;
  char _tmp[PATH_MAX];
  OListTreeItem *_i;
#endif
};


#endif  // __OXDIRECTORYTREE_H
