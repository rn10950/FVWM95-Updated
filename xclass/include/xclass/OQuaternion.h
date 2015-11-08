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

#ifndef __OQUATERNION_H
#define __OQUATERNION_H

class OVector3D;


//----------------------------------------------------------------------
// This is partial implementation of a quaternion object. It has only
// the methods needed by OXGLView to compute rotations.

class OQuaternion {
public:
  OQuaternion();
  OQuaternion(float x, float y, float z, float w);
  OQuaternion(const OVector3D &v, float w);

  void Normalize();

  OQuaternion operator*(const OQuaternion &q) const;

  static OQuaternion FromAxis(float a[3], float phi);
  static OQuaternion FromAxis(float x, float y, float z, float phi);
  static OQuaternion Slerp(const OQuaternion &p, const OQuaternion &q, const float t);

  void RotMatrix(float m[4][4]);

protected:
  float x, y, z, w;
};


#endif  // __OQUATERNION_H
