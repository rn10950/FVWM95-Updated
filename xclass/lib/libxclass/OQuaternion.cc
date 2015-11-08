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

#include <math.h>

#include <xclass/OQuaternion.h>
#include <xclass/OVector3D.h>


//----------------------------------------------------------------------

OQuaternion::OQuaternion() {
  x = 0.0f;
  y = 0.0f;
  z = 0.0f;
  w = 1.0f;
}

OQuaternion::OQuaternion(float x, float y, float z, float w) {
  this->x = x;
  this->y = y;
  this->z = z;
  this->w = w;
}

OQuaternion::OQuaternion(const OVector3D &v, float w) {
  v.getValues(&x, &y, &z);
  this->w = w;
}

void OQuaternion::Normalize() {
  float mag = (float) sqrt(x * x + y * y + z * z + w * w);
  x /= mag;
  y /= mag;
  z /= mag;
  w /= mag;
}

// Compute the equivalent single rotation, given two input rotations.

OQuaternion OQuaternion::operator*(const OQuaternion &q) const {
  static int count = 0;

  OVector3D t1(this->x, this->y, this->z);
  OVector3D qt1 = t1;
  t1.scale(q.w);

  OVector3D t2(q.x, q.y, q.z);
  OVector3D qt2 = t2;
  t2.scale(this->w);

  OVector3D t3 = OVector3D::cross(qt2, qt1);

  OVector3D tf = t1 + t2 + t3;

  float w = this->w * q.w - qt1 * qt2;

  OQuaternion Q(tf.getX(), tf.getY(), tf.getZ(), w);
  Q.Normalize();

  return Q;
}

// Given an axis and angle, compute quaternion.

OQuaternion OQuaternion::FromAxis(float a[3], float phi) {
  return FromAxis(a[0], a[1], a[2], phi);
}

OQuaternion OQuaternion::FromAxis(float x, float y, float z, float phi) {
  OVector3D v(x, y, z);
  v.normalize();
  v.scale((float) sin(phi / 2.0));

  return OQuaternion(v, (float) cos(phi / 2.0));
}

// Build a rotation matrix

void OQuaternion::RotMatrix(float m[4][4]) {
  m[0][0] = 1.0f - 2.0f * (y * y + z * z);
  m[0][1] = 2.0f * (x * y - z * w);
  m[0][2] = 2.0f * (z * x + y * w);
  m[0][3] = 0.0f;

  m[1][0] = 2.0f * (x * y + z * w);
  m[1][1] = 1.0f - 2.0f * (z * z + x * x);
  m[1][2] = 2.0f * (y * z - x * w);
  m[1][3] = 0.0f;

  m[2][0] = 2.0f * (z * x - y * w);
  m[2][1] = 2.0f * (y * z + x * w);
  m[2][2] = 1.0f - 2.0f * (y * y + x * x);
  m[2][3] = 0.0f;

  m[3][0] = 0.0f;
  m[3][1] = 0.0f;
  m[3][2] = 0.0f;
  m[3][3] = 1.0f;
}

// Perform a spherical linear interpolation (SLERP).
// t ranges from 0.0 (at p) to 1.0 (at q)

OQuaternion OQuaternion::Slerp(const OQuaternion &p, const OQuaternion &q,
                               const float t) {
  double fi, cosfi, sinfi;
  float  scale0, scale1, tx, ty, tz, tw;

  // use dot product to get the cos of the angle between the quaternions
  cosfi = p.x * q.x + p.y * q.y + p.z * q.z + p.w * q.w;

  // adjust signs if neccessary
  if (cosfi < 0.0) {
    cosfi = -cosfi;
    tx = -q.x;
    ty = -q.y;
    tz = -q.z;
    tw = -q.w;
  } else {
    tx = q.x;
    ty = q.y;
    tz = q.z;
    tw = q.w;
  }

  // calculate coefficients
  if ((1.0 - cosfi) > 0.0001f) {
    // standard case (slerp)
    fi = acos(cosfi);
    sinfi = sin(fi);
    scale0 = float(sin((1.0 - t) * fi) / sinfi);
    scale1 = float(sin(t * fi) / sinfi);
  } else {
    // quaternions are very close, we can do a linear interpolation
    scale0 = 1.0f - t;
    scale1 = t;
  }

  // calculate final values
  tx = scale0 * p.x + scale1 * tx;
  ty = scale0 * p.y + scale1 * ty;
  tz = scale0 * p.z + scale1 * tz;
  tw = scale0 * p.w + scale1 * tw;

  return OQuaternion(tx, ty, tz, tw);
}
