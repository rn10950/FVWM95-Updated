/**************************************************************************

    This file is part of xclass, a Win95-looking GUI toolkit.
    Copyright (C) 1996, 1997 David Barth, Hector Peraza.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

**************************************************************************/

#ifndef __OPICTURE_H
#define __OPICTURE_H

#include <limits.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/xpm.h>

#include <xclass/OBaseObject.h>
#include <xclass/OHashTable.h>
#include <xclass/OXClient.h>


//----------------------------------------------------------------------

class OPicture : public OBaseObject {
public:
  int GetWidth() const { return _attributes.width; }
  int GetHeight() const { return _attributes.height; }
  Pixmap GetPicture() const { return _pic; }
  Pixmap GetMask() const { return _mask; }
  const char *GetName() const { return _name; }

  virtual void Draw(Display *dpy, XID id, GC gc, int x, int y) const;
  virtual void DrawMask(Display *dpy, XID id, GC gc, int x, int y) const;

  friend class OPicturePool;

protected:
  OPicture(const char *name, int scaled = False) { 
    _name = name; 
    _scaled = scaled;
  }

  const char *_name;
  int _count;           // usage count
  int _scaled;
  XpmAttributes _attributes;
  Pixmap _pic;
  Pixmap _mask;
  OHashEntry *_hash;    // needed to remove the pic from the hash table
};

class OPicturePool : public OBaseObject {
public:
  OPicturePool(const OXClient *client, const OResourcePool *rp,
               const char *path);
  ~OPicturePool();

  const OPicture *GetPicture(const char *name, char **data);
  const OPicture *GetPicture(const char *name, char **data,
                                               int new_width, int new_height);
  const OPicture *GetPicture(const char *name);
  const OPicture *GetPicture(const char *name, int new_width, int new_height);
  const OPicture *GetPicture(const OPicture *pic);
  void FreePicture(const OPicture *pic);

  void SetPath(const char *path);
  void AddPath(const char *path);
  const char *GetPath() const { return _path; }

  friend class OXClient;

private:
  int LocatePicture(const char *name, char *path, char *buffer);
  const char *GetUid(const char *string);

protected:
//  void _LoadSymbols();
  static bool _inited;
  static XpmColorSymbol _colors[6];
  const OXClient *_client;
  char *_path;
  OStringHashTable *_ntable;
  OArrayHashTable *_ptable;
  Colormap _defaultColormap;
};


#endif  // __OPICTURE_H
