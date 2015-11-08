/**************************************************************************

    This file is part of xclass.
    Copyright (C) 2002-2003 Hector Peraza.

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

#ifndef __OIMAGE_H
#define __OIMAGE_H

#include <xclass/OBaseObject.h>
#include <xclass/utils.h>


//--------------------------------------------------------------------

// Abstract base class for images. OImage is similar to OPicture,
// but there are some differences:
//  - OImages are not cached by OXClient
//  - they can have more than one frame (animated images)
//  - they are not limited to a single file format
//  - they can use more sophisticated rescaling algorithms,
//    color dithering, double-buffering, etc.
// OImages and derivates are used by the OXHtml widget.

class OImage : public OBaseObject {
public:
  OImage(OXClient *c) { _client = c; _numFrames = 0; }
  virtual ~OImage() {}

  int NumFrames() { return _numFrames; }

  virtual bool IsAnimated() { return (_numFrames > 1); }
  virtual void NextFrame() = 0;     // advance to next frame
  
  virtual unsigned long GetAnimDelay() { return 0L; }

  virtual void Draw(Drawable d, GC gc, int src_x, int src_y,
                    int src_w, int src_h, int dst_x, int dst_y) = 0;

  virtual int GetWidth() { return 0; }
  virtual int GetHeight() { return 0; }

  virtual bool isTransparent() { return false; }

  Pixmap GetPicture() const { return _bufferPixmap; }

protected:
  Pixmap _bufferPixmap;
  Pixmap _bufferMask;
  int _numFrames;

  OXClient *_client;
};


#endif  // __OIMAGE_H
