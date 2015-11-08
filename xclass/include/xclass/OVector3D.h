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

#ifndef __OVECTOR3D_H
#define __OVECTOR3D_H


//----------------------------------------------------------------------
// This is a partial implementation of a 3D vector object. It has the
// methods needed by OQuaternion, OTrackball and OXGLView

class OVector3D {
public:
  OVector3D();
  OVector3D(float x, float y, float z);

  void  zero();
  static float dot(const OVector3D &v1, const OVector3D &v2);
  static OVector3D cross(const OVector3D &v1, const OVector3D &v2);
  float length();
  void  scale(float div);
  void  normalize();

  void  getValues(float *x, float *y, float *z) const;

  float getX() const { return x; }
  float getY() const { return y; }
  float getZ() const { return z; }

  OVector3D& operator=(const OVector3D &v);

  OVector3D  operator+(const OVector3D &v) const;
  OVector3D& operator+=(const OVector3D &v);
  OVector3D  operator-() const;          // negation
  OVector3D  operator-(const OVector3D &v) const;
  OVector3D& operator-=(const OVector3D &v);

  float      operator*(const OVector3D &v) const;  // dot product

protected:
  float x, y, z;
};


#endif  // __OVECTOR3D_H
