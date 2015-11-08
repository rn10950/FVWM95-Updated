/**************************************************************************

    This file is part of xclass.
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

#ifndef __OPACKEDMATRIXLAYOUT_H
#define __OPACKEDMATRIXLAYOUT_H

#include <xclass/OLayout.h>


class OPackedMatrixLayout : public OLayoutManager {
public:
  OPackedMatrixLayout(class OXCompositeFrame *main, int r, int c,
                      int s = 0, int h = 0);

  virtual void Layout();
  virtual ODimension GetDefaultSize() const;

  int sep; // interval between frames
  int hints;
  int rows, columns;

protected:
  class OXCompositeFrame *_main; // container frame
  SListFrameElt **_list;         // list of frames to arrange
};


#endif  // __OPACKEDMATRIXLAYOUT_H
