/**************************************************************************

    This file is part of taskbar.
    Copyright (C) 2000, Hector Peraza.

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

#ifndef __OTRAYLAYOUT_H
#define __OTRAYLAYOUT_H

#include <xclass/OBaseObject.h>
#include <xclass/ODimension.h>
#include <xclass/OLayout.h>


//--------------------------------------------------------------------

class OTrayLayout : public OLayoutManager {
public:
  OTrayLayout(class OXCompositeFrame *main,
              int rowh = 22, int rows = 1, int sep = 1);

  virtual void Layout();
  virtual ODimension GetDefaultSize() const;

  int _sep; // interval between frames
  int _rows, _rowHeight;

protected:
  class OXCompositeFrame *_main; // container frame
  SListFrameElt **_list;         // list of frames to arrange
};

#endif  // __OTRAYLAYOUT_H
