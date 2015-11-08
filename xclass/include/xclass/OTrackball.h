/**************************************************************************

    This is file is part of xclass.
    Copyright (C) 2000-2006, Hector Peraza.

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

#ifndef __OTRACKBALL_H
#define __OTRACKBALL_H

#include "OQuaternion.h"


//----------------------------------------------------------------------
// Virtual trackball object used by OXGLView

class OTrackball {
public:
  OTrackball();

  void SetRadius(float r);
  OQuaternion Track(float x1, float y1, float x2, float y2);

protected:
  float ProjectPoint(float x, float y);

  float r;
};


#endif  // __OTRACKBALL_H
