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

#ifndef __O2COLUMNSLAYOUT_H
#define __O2COLUMNSLAYOUT_H

#include <xclass/OXCompositeFrame.h>
#include <xclass/OLayout.h>


class O2ColumnsLayout : public OLayoutManager {
public:
  O2ColumnsLayout(OXCompositeFrame *main, int hsep = 0, int vsep = 0);

  virtual void Layout();
  virtual ODimension GetDefaultSize() const;

protected:
  int _hsep, _vsep;
  class OXCompositeFrame *_main;
  SListFrameElt **_list;
};

#endif  // __O2COLUMNSLAYOUT_H
