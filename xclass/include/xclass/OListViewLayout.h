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

#ifndef __OLISTVIEWLAYOUT_H
#define __OLISTVIEWLAYOUT_H

#include <xclass/OLayout.h>


//----------------------------------------------------------------------
// The following Layout objects are used by the OXListView widget

class OTileLayout : public OLayoutManager {
public:
  OTileLayout(OXCompositeFrame *main, int sep = 0);

  virtual void Layout();
  virtual ODimension GetDefaultSize() const;

protected:
  int _sep;
  class OXCompositeFrame *_main;
  SListFrameElt **_list;
};

class OListLayout : public OTileLayout {
public:
  OListLayout(OXCompositeFrame *main, int sep = 0) :
    OTileLayout(main, sep) {}

  virtual void Layout();
  virtual ODimension GetDefaultSize() const;
};

class OListDetailsLayout : public OTileLayout {
public:
  OListDetailsLayout(class OXCompositeFrame *main, int sep = 0) :
    OTileLayout(main, sep) {}
  
  virtual void Layout();
  virtual ODimension GetDefaultSize() const;
};

#endif  // __OLISTVIEWLAYOUT_H
