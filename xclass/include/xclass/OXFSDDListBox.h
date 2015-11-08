/**************************************************************************

    This file is part of xclass, a Win95-looking GUI toolkit.
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

#ifndef __OXFSDDLISTBOX_H
#define __OXFSDDLISTBOX_H

#include <X11/Xlib.h>

#include <xclass/OXClient.h>
#include <xclass/OXCompositeFrame.h>
#include <xclass/OXWindow.h>
#include <xclass/OXWidget.h>
#include <xclass/OXDDListBox.h>
#include <xclass/OSelectedPicture.h>
#include <xclass/OString.h>


//----- This is temp here...

struct _lbc {
  char *name;
  char *path;
  char *pixmap;
  int  ID, indent, flags;
};


class OXFont;
class OXGC;

//----------------------------------------------------------------------

class OXTreeLBEntry : public OXLBEntry {
protected:
  static unsigned long _selPixel;
  static OXGC *_defaultGC;
  static const OXFont *_defaultFont;
  static int _init;

public:
  OXTreeLBEntry(const OXWindow *p,
            OString *text, const OPicture *pic, int ID, OString *path = NULL,
            unsigned int options = HORIZONTAL_FRAME,
            unsigned long back = _defaultDocumentBackground);
  virtual ~OXTreeLBEntry();

  const OString *GetText() const { return _text; }
  const OPicture *GetPicture() const { return _pic; }
  const OString *GetPath() const { return _path; }

  virtual ODimension GetDefaultSize() const;

  virtual void Activate(int a);
  virtual void UpdateEntry(OXLBEntry *e);

protected:
  virtual void _DoRedraw();

  OXGC *_normGC;
  const OXFont *_font;
  OString *_text, *_path;
  const OPicture *_pic;
  OSelectedPicture *_selpic;
  int _active, _tw, _th, _ta;
};


class OXFileSystemDDListBox : public OXDDListBox {
public:
  OXFileSystemDDListBox(const OXWindow *p, int ID,
              unsigned int options = HORIZONTAL_FRAME |
                                     SUNKEN_FRAME | DOUBLE_BORDER,
              unsigned long back = _defaultDocumentBackground);

  virtual void UpdateContents(char *path);

};


#endif  // __OXFSDDLISTBOX_H
