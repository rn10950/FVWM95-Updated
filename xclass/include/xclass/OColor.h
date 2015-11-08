/**************************************************************************

    This file is part of xclass.
    Copyright (C) 2000, 2001, Hector Peraza.

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

#ifndef __OCOLOR_H
#define __OCOLOR_H

#include <xclass/OBaseObject.h>

class OXClient;


class OColor : public OBaseObject {
public:
  OColor();
  OColor(int r, int g, int b);
  virtual ~OColor();
  
  static void HLS2RGB(int h, int l, int s, int *r, int *g, int *b);
  static void RGB2HLS(int r, int g, int b, int *h, int *l, int *s);

  int GetR() const { return _r; }
  int GetG() const { return _g; }
  int GetB() const { return _b; }

  int GetH() const { return _h; }
  int GetL() const { return _l; }
  int GetS() const { return _s; }
  
  void SetRGB(int r, int g, int b);
  void SetHLS(int h, int l, int s);
  
  unsigned long GetDisplayPixel(OXClient *c);
  void SetColor(OXClient *c, unsigned long color);
  
protected:
  int _r, _g, _b, _h, _l, _s;
};

#endif  // __OXCOLOR_H
