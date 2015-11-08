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

#include <xclass/OVector3D.h>


//----------------------------------------------------------------------

OVector3D::OVector3D() {
  x = 0.0f;
  y = 0.0f;
  z = 0.0f;
}

OVector3D::OVector3D(float x, float y, float z) {
  this->x = x;
  this->y = y;
  this->z = z;
}

OVector3D& OVector3D::operator=(const OVector3D &v) {
  x = v.x;
  y = v.y;
  z = v.z;
  return *this;
}

OVector3D OVector3D::operator+(const OVector3D &v) const {
  return OVector3D(x + v.x, y + v.y, z + v.z);
}

OVector3D& OVector3D::operator+=(const OVector3D &v) {
  x += v.x;
  y += v.y;
  z += v.z;
  return *this;
}

OVector3D OVector3D::operator-(const OVector3D &v) const {
  return OVector3D(x - v.x, y - v.y, z - v.z);
}

OVector3D& OVector3D::operator-=(const OVector3D &v) {
  x -= v.x;
  y -= v.y;
  z -= v.z;
  return *this;
}  

OVector3D OVector3D::operator-() const {
  return OVector3D(-x, -y, -z);
}

float OVector3D::operator*(const OVector3D &v) const {
  return x * v.x + y * v.y + z * v.z;
}

float OVector3D::dot(const OVector3D &v1, const OVector3D &v2) {
  return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

OVector3D OVector3D::cross(const OVector3D &v1, const OVector3D &v2) {
  float x, y, z;

  x = v1.y * v2.z - v1.z * v2.y;
  y = v1.z * v2.x - v1.x * v2.z;
  z = v1.x * v2.y - v1.y * v2.x;

  return OVector3D(x, y, z);
}

float OVector3D::length() {
  return (float) sqrt(x * x + y * y + z * z);
}

void OVector3D::scale(float div) {
  x *= div;
  y *= div;
  z *= div;
}

void OVector3D::normalize() {
  float len = length();
  if (len > 0.0f) scale(1.0f / length());
}

void OVector3D::getValues(float *x, float *y, float *z) const {
  *x = this->x;
  *y = this->y;
  *z = this->z;
}
