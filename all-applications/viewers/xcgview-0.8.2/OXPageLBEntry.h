/**************************************************************************

    This file is part of xcgview, a xclass port of Ghostview 1.5
    Copyright (C) 1998 Hector Peraza.

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

#ifndef __OXPAGELBENTRY_H
#define __OXPAGELBENTRY_H


#include <xclass/utils.h>
#include <xclass/OXListBox.h>


//*** Custom listbox entry object for page index list ***


#define PLBE_CHECKMARK   (1<<0)
#define PLBE_RADIOMARK   (1<<1)


//-------------------------------------------------------------

class OXPageLBEntry : public OXTextLBEntry {
public:
  OXPageLBEntry(const OXWindow *p, OString *s, int ID);

  virtual ODimension GetDefaultSize() const { return ODimension(_tw+20, _th+1); }
  void SetMark(int mark_type, int mode);
  int  GetFlags() const { return _flags; }

protected:
  virtual void _DoRedraw();

  void DrawCheckMark(GC gc);
  void DrawRCheckMark(GC gc);

  int _flags;
};

#endif  // __OXPAGELBENTRY_H
