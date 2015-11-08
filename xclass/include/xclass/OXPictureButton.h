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

#ifndef __OXPICTUREBUTTON_H
#define __OXPICTUREBUTTON_H

#include <X11/Xlib.h>

#include <xclass/utils.h>
#include <xclass/OPicture.h>
#include <xclass/OXButton.h>


//----------------------------------------------------------------------

class OXPictureButton : public OXButton {
public:
  OXPictureButton(const OXWindow *p, const OPicture *pic, int ID,
                  unsigned int option = RAISED_FRAME | DOUBLE_BORDER);

  virtual ODimension GetDefaultSize() const {
    return ODimension(_tw + (_bw << 1) + _bw + 1,
                      _th + (_bw << 1) + _bw);
  }

  void SetPicture(const OPicture *pic) { if (pic) _pic = pic; }
  void SetDisabledPicture(const OPicture *pic) { _disabledPic = pic; }
  
  const OPicture *GetPicture() const { return _pic; }
  const OPicture *GetDisabledPicture() const { return _disabledPic; }

protected:
  virtual void _DoRedraw();

  const OPicture *_pic, *_disabledPic;
};


#endif  // __OXPICTUREBUTTON_H
