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

#ifndef __OXICON_H
#define __OXICON_H

#include <X11/Xlib.h>

#include <xclass/OXFrame.h>
#include <xclass/ODimension.h>
#include <xclass/OPicture.h>


//----------------------------------------------------------------------

class OXIcon : public OXFrame {
public:
  OXIcon(const OXWindow *p, const OPicture *pic, int w = 10, int h = 10,
         unsigned int options = CHILD_FRAME,
         unsigned long back = _defaultFrameBackground) :
    OXFrame(p, w, h, options, back) { _pic = pic; }

  const OPicture *GetPicture() const { return _pic; }
  virtual void SetPicture(const OPicture *pic);

  virtual ODimension GetDefaultSize() const;

protected:
  virtual void _DoRedraw();

  const OPicture *_pic;
};


#endif  // __OXICON_H
