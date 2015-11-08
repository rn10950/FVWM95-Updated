/**************************************************************************

    This file is part of xclass.
    Copyright (C) 2000-2004, Hector Peraza.

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

#ifndef __OXIMAGEVIEW_H
#define __OXIMAGEVIEW_H

#include <xclass/utils.h>
#include <xclass/OXClient.h>
#include <xclass/OXView.h>


#define DITHER_CLOSEST_COLOR     0
#define DITHER_FLOYD_STEINBERG   1
#define DITHER_BAYER             2


//---------------------------------------------------------------------

class ORGBImage : public OBaseObject {
public:
  ORGBImage(int w, int h);
  virtual ~ORGBImage();

  int GetWidth() const { return width; }
  int GetHeight() const { return height; }

public:
  unsigned char *r, *g, *b;

protected:
  int width, height;
};

class OXImageView : public OXView {
public:
  OXImageView(const OXWindow *p);
  virtual ~OXImageView();

  virtual void Clear();
  void SetImage(ORGBImage *i);

  void SetDitheringMode(int mode);

  virtual int DrawRegion(OPosition coord, ODimension size, int clear = True);
    
protected:
  void _AllocColors();
  void _FreeColors();
  void _CreateImage();
  void _DitherImage();

  void _DitherClosestColor();
  void _DitherFloydSteinberg();
  void _DitherBayer();

  void _AllocImage();
  void _FreeImage();

  XImage *_image;
  ORGBImage *_cimage;
  
  int _ncolors;
  int _ditherMode;
  
private:
  int colormap[64][3];
  unsigned long pixel[64];
};


#endif  // __OXIMAGEVIEW_H
