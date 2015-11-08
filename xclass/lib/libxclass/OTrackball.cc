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

#include <xclass/OTrackball.h>
#include <xclass/OVector3D.h>
#include <xclass/OQuaternion.h>

// This code is based on Gavin Bell's algorithm.
// See also the August '88 issue of Siggraph's "Computer Graphics,"
// pp. 121-129.

//---------------------------------------------------------------------

OTrackball::OTrackball() {
  r = 0.8f;
}

void OTrackball::SetRadius(float r) {
  this->r = r;
}

// Track-ball simulation. The points represent the coordinates of
// the last and current positions of the mouse, scaled so they range
// from -1.0 ... 1.0. Project the points onto the virtual trackball,
// then figure out the axis of rotation, which is the cross product
// of P1 P2 and O P1 (where O is the center of the ball, 0,0,0).
//
// Note:  This is a deformed trackball - is a trackball in the center,
// but is deformed into a hyperbolic sheet of rotation away from the
// center.  This particular function was chosen after trying out
// several variations.
//
// The return value is a quaternion representing the resulting rotation.

OQuaternion OTrackball::Track(float x1, float y1, float x2, float y2) {
  float phi;  // how much to rotate about axis
  float t;

  if (x1 == x2 && y1 == y2) {
    // Zero rotation
    return OQuaternion(0.0f, 0.0f, 0.0f, 1.0f);
  }

  // figure out z-coordinates for projection of P1 and P2 to
  // deformed sphere

  OVector3D p1(x1, y1, ProjectPoint(x1, y1));
  OVector3D p2(x2, y2, ProjectPoint(x2, y2));

  // obtain the axis of rotation as the cross product of P1 and P2

  OVector3D a = OVector3D::cross(p2, p1);

  // figure out how much to rotate around that axis.

  t = (p1 - p2).length() / (2.0f * r);

  // avoid problems with out-of-control values...

  if (t > 1.0f) t = 1.0f; else if (t < -1.0f) t = -1.0f;
  phi = (float) (2.0 * asin(t));

  return OQuaternion::FromAxis(a.getX(), a.getY(), a.getZ(), phi);
}

// Project an x,y pair onto the trackball sphere, or a hyperbolic sheet
// if we are away from the center of the sphere.

float OTrackball::ProjectPoint(float x, float y) {
  float d, z;

  d = (float) sqrt(x * x + y * y);
  if (d < r * 0.7071068f) {
    // inside sphere
    z = (float) sqrt(r * r - d * d);
  } else {
    // on hyperbola
    z = r * r / (d + d);
  }
  return z;
}
