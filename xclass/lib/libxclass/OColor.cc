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

#include <X11/X.h>

#include <xclass/utils.h>
#include <xclass/OXClient.h>
#include <xclass/OColor.h>

#define COLOR_MAX  255
#define HLS_MAX    255


//----------------------------------------------------------------------

OColor::OColor() {
  SetRGB(0, 0, 0);
}

OColor::OColor(int r, int g, int b) {
  SetRGB(r, g, b);
}

OColor::~OColor() {
}

void OColor::SetRGB(int r, int g, int b) {
  RGB2HLS(_r = r, _g = g, _b = b, &_h, &_l, &_s);
}

void OColor::SetHLS(int h, int l, int s) {
  HLS2RGB(_h = h, _l = l, _s = s, &_r, &_g, &_b);
}

// Return a screen pixel value that matches current RGB values.
// The pixel must be freed with OXClient::FreeColor() when it is
// no longer needed by the application.

unsigned long OColor::GetDisplayPixel(OXClient *c) {
  XColor color;

  color.pixel = 0;
  color.red   = _r * 256;
  color.green = _g * 256;
  color.blue  = _b * 256;

  if (!XAllocColor(c->GetDisplay(), c->GetDefaultColormap(), &color)) {
    Debug(DBG_WARN, "OColor: Couldn't retrieve color #%02x%02x%02x\n",
          _r, _g, _b);
    // force allocation of pixel 0
    XQueryColor(c->GetDisplay(), c->GetDefaultColormap(), &color);
    if (!XAllocColor(c->GetDisplay(), c->GetDefaultColormap(), &color)) {
      Debug(DBG_WARN, "OColor: failed to allocate pixel 0!!!\n");
    }
  }

  return color.pixel;
}

// Set current RGB/HLS values from screen pixel.

void OColor::SetColor(OXClient *c, unsigned long col) {
  XColor color;

  color.pixel = col;
  XQueryColor(c->GetDisplay(), c->GetDefaultColormap(), &color);

  SetRGB(color.red / 256, color.green / 256, color.blue / 256);
}

//----------------------------------------------------------------------

void OColor::RGB2HLS(int r, int g, int b, int *h, int *l, int *s) {
  double _R, _G, _B, _H, _L, _S, D, Cmax, Cmin;

  _R = r / (double) COLOR_MAX;
  _G = g / (double) COLOR_MAX;
  _B = b / (double) COLOR_MAX;

  Cmax = max(_R, max(_G, _B));
  Cmin = min(_R, min(_G, _B));

  // calculate luminosity
  _L = (Cmax + Cmin) / 2.0;

  if (Cmax == Cmin) {  // it's grey
    _H = 0.0;  // it's actually undefined
    _S = 0.0;
  } else {
    D = Cmax - Cmin;

    // calculate saturation
    if (_L < 0.5) {
      _S = D / (Cmax + Cmin);
    } else {
      _S = D / (2.0 - Cmax - Cmin);
    }

    // calculate hue
    if (_R == Cmax) {
      _H = (_G - _B) / D;
    } else if (_G == Cmax) {
      _H = 2.0 + (_B - _R) / D;
    } else {
      _H = 4.0 + (_R - _G) / D;
    }

    _H /= 6.0;
    if (_H < 0.0) _H += 1.0;
  }

  *h = (int) (_H * HLS_MAX);
  *l = (int) (_L * HLS_MAX);
  *s = (int) (_S * HLS_MAX);
}

static int Hue2Color(double hue, double m1, double m2) {
  double v;

  if (hue < 0.0) {
    hue += 1.0;
  } else if (hue > 1.0) {
    hue -= 1.0;
  }

  if (hue * 6.0 < 1.0) {
    v = m1 + (m2 - m1) * hue * 6.0;
  } else if (hue * 2.0 < 1.0) {
    v = m2;
  } else if (hue * 3.0 < 2.0) {
    v = m1 + (m2 - m1) * (0.666666667 - hue) * 6;
  } else {
    v = m1;
  }

  return (int) (v * COLOR_MAX);
}

void OColor::HLS2RGB(int h, int l, int s, int *r, int *g, int *b) {
  double M1, M2, _H, _L, _S;

  _H = h / (double) HLS_MAX;
  _L = l / (double) HLS_MAX;
  _S = s / (double) HLS_MAX;

  if (_S == 0.0) {
    *r = *g = *b = (int) (_L * COLOR_MAX);
  } else {
    if (_L <= 0.5) {
      M2 = _L * (1.0 + _S);
    } else {
      M2 = _L + _S - _L * _S;
    }
    M1 = 2.0 * _L - M2;

    *r = Hue2Color(_H + 0.333333333, M1, M2);
    *g = Hue2Color(_H, M1, M2);
    *b = Hue2Color(_H - 0.333333333, M1, M2);
  }
}
