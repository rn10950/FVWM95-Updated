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

#include <X11/Xlib.h>

#include <xclass/OColor.h>
#include <xclass/OXImageView.h>


//---------------------------------------------------------------------

ORGBImage::ORGBImage(int w, int h) {
  width = w;
  height = h;
  r = new unsigned char[w * h];
  g = new unsigned char[w * h];
  b = new unsigned char[w * h];
}

ORGBImage::~ORGBImage() {
  delete[] r;
  delete[] g;
  delete[] b;
}

//---------------------------------------------------------------------

OXImageView::OXImageView(const OXWindow *p) : 
  OXView(p, 10, 10, -1) {
    _image = 0;
    _cimage = 0;
    _ncolors = 0;
    _ditherMode = DITHER_FLOYD_STEINBERG;
    _virtualSize = ODimension(0, 0);

    XSetWindowAttributes wa;
    unsigned long mask = CWWinGravity | CWBitGravity;
    wa.bit_gravity = NorthWestGravity;
    wa.win_gravity = NorthWestGravity;
    XChangeWindowAttributes(GetDisplay(), _canvas->GetId(), mask, &wa);
}

OXImageView::~OXImageView() {
  _FreeImage();
  _FreeColors();
}

void OXImageView::Clear() {
  SetImage(0);
  OXView::Clear();
}

void OXImageView::SetImage(ORGBImage *i) {
  _cimage = i;
  if (_cimage) {
    _AllocImage();
    _CreateImage();
    _virtualSize = ODimension(_image->width, _image->height);
  } else {
    _FreeImage();
    _virtualSize = ODimension(0, 0);
  }
  Layout();
  NeedRedraw(ORectangle(_visibleStart, _canvas->GetSize()));
}

int OXImageView::DrawRegion(OPosition coord, ODimension size, int clear) {
  OXView::DrawRegion(coord, size, clear);

  if (_image) {
    XPutImage(GetDisplay(), _canvas->GetId(), _blackGC, _image, 
              coord.x, coord.y,
              ToPhysical(coord).x, ToPhysical(coord).y,
              size.w, size.h);
  }

  return True;
}

void OXImageView::_AllocImage() {

  if (_image && (_image->width == _cimage->GetWidth()) &&
                (_image->height == _cimage->GetHeight())) return;

  _FreeImage();

  int depth = _client->GetDisplayDepth();
  int bitmap_pad;

  if (depth <= 8)
    bitmap_pad = 8;
  else if (depth <= 16)
    bitmap_pad = 16;
  else
    bitmap_pad = 32;

  _image = XCreateImage(GetDisplay(),
                        DefaultVisual(GetDisplay(),
                        _client->GetScreenNumber()),
                        depth, ZPixmap, 0, NULL,
                        _cimage->GetWidth(), _cimage->GetHeight(),
                        bitmap_pad, 0);

  _image->data = (char *) calloc(1, _image->bytes_per_line *
                                    _image->height);
}

void OXImageView::_FreeImage() {
  if (_image) {
    XDestroyImage(_image);
    _image = 0;
  }
}

// Allocate a color cube.
// This routine is called in PseudoColor modes only.

void OXImageView::_AllocColors() {
  // Try to allocate first a palette of 64 colors.

  XColor color;
  int i;

  for (i = 0; i < 64; ++i) {
    int cc[4] = { 0, 21845, 43691, 65535 };
    color.pixel = 0;
    color.red   = cc[i & 0x3];
    color.green = cc[(i >> 2) & 0x3];
    color.blue  = cc[(i >> 4) & 0x3];
    if (XAllocColor(GetDisplay(), _client->GetDefaultColormap(), &color) == 0)
      break;
    colormap[i][0] = color.red / 256;
    colormap[i][1] = color.green / 256;
    colormap[i][2] = color.blue / 256;
    pixel[i] = color.pixel;
  }

  _ncolors = i;
  if (_ncolors == 64) return;  // success

  // Failed, try a simpler 27-color.

  _FreeColors();

  for (i = 0; i < 27; ++i) {
    int cc[3] = { 0, 32768, 65535 };
    color.pixel = 0;
    color.red   = cc[i % 3];
    color.green = cc[(i / 3) % 3];
    color.blue  = cc[(i / 9) % 3];
    if (XAllocColor(GetDisplay(), _client->GetDefaultColormap(), &color) == 0)
      break;
    colormap[i][0] = color.red / 256;
    colormap[i][1] = color.green / 256;
    colormap[i][2] = color.blue / 256;
    pixel[i] = color.pixel;
  }

  _ncolors = i;
  if (_ncolors == 27) return;  // success

  // Failed, try then a much simpler 8-color.

  _FreeColors();

  for (i = 0; i < 8; ++i) {
    color.pixel = 0;
    color.red   = (i & 1) * 65535;
    color.green = ((i >> 1) & 1) * 65535;
    color.blue  = ((i >> 2) & 1) * 65535;
    if (XAllocColor(GetDisplay(), _client->GetDefaultColormap(), &color) == 0)
      break;
    colormap[i][0] = color.red / 256;
    colormap[i][1] = color.green / 256;
    colormap[i][2] = color.blue / 256;
    pixel[i] = color.pixel;
  }

  _ncolors = i;
  if (_ncolors == 8) return;  // success

  // Failed, try to get at least 8 closest colors...
  // (TODO: search for closest colors in the colormap, right now we just
  // get as many as exact colors we can for the 8-color palette)

  _FreeColors();

  for (i = 0; i < 8; ++i) {
    color.pixel = 0;
    color.red   = (i & 1) * 65535;
    color.green = ((i >> 1) & 1) * 65535;
    color.blue  = ((i >> 2) & 1) * 65535;
    if (XAllocColor(GetDisplay(), _client->GetDefaultColormap(), &color) != 0) {
      colormap[_ncolors][0] = color.red / 256;
      colormap[_ncolors][1] = color.green / 256;
      colormap[_ncolors][2] = color.blue / 256;
      pixel[_ncolors++] = color.pixel;
    }
  }

  // continue with what we got...
}

void OXImageView::_FreeColors() {
  if (_ncolors > 0) {
    XFreeColors(GetDisplay(), _client->GetDefaultColormap(),
                pixel, _ncolors, 0);
    _ncolors = 0;
  }
}

void OXImageView::_CreateImage() {
  int r, g, b, ppos;

  Visual *visual = DefaultVisual(GetDisplay(), _client->GetScreenNumber());
  //int ncolors = XDisplayCells(GetDisplay(), _client->GetScreenNumber());

  if (visual->c_class == TrueColor || visual->c_class == DirectColor) {

    int rmask = visual->red_mask, rmax = rmask, rshift = 0;
    if (rmax != 0) while ((rmax & 1) == 0) { rmax >>= 1; ++rshift; }

    int gmask = visual->green_mask, gmax = gmask, gshift = 0;
    if (gmax != 0) while ((gmax & 1) == 0) { gmax >>= 1; ++gshift; }

    int bmask = visual->blue_mask, bmax = bmask, bshift = 0;
    if (bmax != 0) while ((bmax & 1) == 0) { bmax >>= 1; ++bshift; }

    ppos = 0;

    for (int y = 0; y < _image->height; ++y) {
      for (int x = 0; x < _image->width; ++x) {
        r = _cimage->r[ppos] * rmax / 255;
        g = _cimage->g[ppos] * gmax / 255;
        b = _cimage->b[ppos] * bmax / 255;
        XPutPixel(_image, x, y, (r << rshift) | (g << gshift) | (b << bshift));
        ++ppos;
      }
    }
  } else {
    _DitherImage();
  }
}

// Create a dithered version of the image for display modes with reduced
// number of colors. This routine is called in PseudoColor modes only.

void OXImageView::_DitherImage() {

  if (_ncolors == 0) _AllocColors();

  switch (_ditherMode) {
    case DITHER_CLOSEST_COLOR:
      _DitherClosestColor();
      break;

    default:
    case DITHER_FLOYD_STEINBERG:
      _DitherFloydSteinberg();
      break;

    case DITHER_BAYER:
      _DitherBayer();
      break;
  }
}

// Dither with Floyd-Steinberg error diffusion algorithm.

void OXImageView::_DitherFloydSteinberg() {
  XColor *line;
  struct __rgb { int r, g, b; } *ed, ef;
  int  ppos, x, y, c, nc, v, e[4];
  long dist, sdist;

  int iw = _image->width;
  int ih = _image->height;

  line = new XColor[iw];
  ed = new struct __rgb[iw];

  for (x = 0; x < iw; ++x) {
    ed[x].r = ed[x].g = ed[x].b = 0;
  }

  ppos = 0;

  for (y = 0; y < ih; ++y) {
    for (x = 0; x < iw; ++x) {
      line[x].red   = _cimage->r[ppos];
      line[x].green = _cimage->g[ppos];
      line[x].blue  = _cimage->b[ppos];
      ++ppos;
    }

    ef.r = ef.g = ef.b = 0;        // no forward error for first pixel

    for (x = 0; x < iw; ++x) {

      // add errors from previous line

      v = line[x].red + ed[x].r;
      if (v < 0) v = 0; else if (v > 255) v = 255;
      line[x].red = v;

      v = line[x].green + ed[x].g;
      if (v < 0) v = 0; else if (v > 255) v = 255;
      line[x].green = v;

      v = line[x].blue + ed[x].b;
      if (v < 0) v = 0; else if (v > 255) v = 255;
      line[x].blue = v;

    }

    for (x = 0; x < iw; ++x) {

      // add forward errors

      v = line[x].red + ef.r;
      if (v < 0) v = 0; else if (v > 255) v = 255;
      line[x].red = v;

      v = line[x].green + ef.g;
      if (v < 0) v = 0; else if (v > 255) v = 255;
      line[x].green = v;

      v = line[x].blue + ef.b;
      if (v < 0) v = 0; else if (v > 255) v = 255;
      line[x].blue = v;

      // find the nearest color in colormap[]

      sdist = 255L * 255L * 255L;
      for (c = 0; c < _ncolors; ++c) {

        int dr = line[x].red   - colormap[c][0];
        int dg = line[x].green - colormap[c][1];
        int db = line[x].blue  - colormap[c][2];

        dist = dr * dr + dg * dg + db * db;
        if (dist < sdist) {
          nc = c;
          sdist = dist;
        }
      }

      XPutPixel(_image, x, y, pixel[nc]);

#define FILTER(v) \
      e[0] = (7 * v) >> 4; \
      e[1] = v >> 4;       \
      e[2] = (5 * v) >> 4; \
      e[3] = (3 * v) >> 4;

      v = line[x].red - colormap[nc][0];
      FILTER(v)

      ef.r = e[0];
      if (x < iw-1) ed[x+1].r = e[1];
      if (x == 0) ed[x].r = e[2]; else ed[x].r += e[2];
      if (x > 0) ed[x-1].r += e[3];

      v = line[x].green - colormap[nc][1];
      FILTER(v)

      ef.g = e[0];
      if (x < iw-1) ed[x+1].g = e[1];
      if (x == 0) ed[x].g = e[2]; else ed[x].g += e[2];
      if (x > 0) ed[x-1].g += e[3];

      v = line[x].blue - colormap[nc][2];
      FILTER(v)

      ef.b = e[0];
      if (x < iw-1) ed[x+1].b = e[1];
      if (x == 0) ed[x].b = e[2]; else ed[x].b += e[2];
      if (x > 0) ed[x-1].b += e[3];

    }
  }

  delete[] line;
  delete[] ed;
}

// Dither using Bayer (matrix) algorithm.

static int pattern[8][8] = {
  {  0, 32,  8, 40,  2, 34, 10, 42 },   // 8x8 Bayer ordered dithering
  { 48, 16, 56, 24, 50, 18, 58, 26 },   // pattern.  Each input pixel
  { 12, 44,  4, 36, 14, 46,  6, 38 },   // is scaled to the 0..63 range
  { 60, 28, 52, 20, 62, 30, 54, 22 },   // before looking in this table
  {  3, 35, 11, 43,  1, 33,  9, 41 },   // to determine the action.
  { 51, 19, 59, 27, 49, 17, 57, 25 },
  { 15, 47,  7, 39, 13, 45,  5, 37 },
  { 63, 31, 55, 23, 61, 29, 53, 21 }
};

void OXImageView::_DitherBayer() {
  int  x, y, c, nc, v, ppos;
  long dist, sdist;                   // Used for least-squares match.

  int iw = _image->width;
  int ih = _image->height;

  XColor color;

  ppos = 0;

  for (y = 0; y < ih; ++y) {
    for (x = 0; x < iw; ++x) {

      //v = (_cimage->r[ppos] >> 1) + (pattern[x & 7][y & 7] << 1);
      v = (_cimage->r[ppos] + ((1 + pattern[x & 7][y & 7]) << 2)) >> 1;
      if (v < 0) v = 0;
      if (v > 255) v = 255;
      color.red = v;

      //v = (_cimage->g[ppos] >> 1) + (pattern[x & 7][y & 7] << 1);
      v = (_cimage->g[ppos] + ((1 + pattern[x & 7][y & 7]) << 2)) >> 1;
      if (v < 0) v = 0;
      if (v > 255) v = 255;
      color.green = v;

      //v = (_cimage->b[ppos] >> 1) + (pattern[x & 7][y & 7] << 1);
      v = (_cimage->b[ppos] + ((1 + pattern[x & 7][y & 7]) << 2)) >> 1;
      if (v < 0) v = 0;
      if (v > 255) v = 255;
      color.blue = v;

      // find the nearest color in colormap[]

      sdist = 255L * 255L * 255L + 1L;
      for (c = 0; c < _ncolors; ++c) {

        int dr = color.red   - colormap[c][0];
        int dg = color.green - colormap[c][1];
        int db = color.blue  - colormap[c][2];

        dist = dr * dr + dg * dg + db * db;
        if (dist < sdist) {
          nc = c;
          sdist = dist;
        }
      }

      // Nearest color found; plot it.

      XPutPixel(_image, x, y, pixel[nc]);

      ++ppos;
    }
  }
}

// No dithering. Use closest color

void OXImageView::_DitherClosestColor() {
  int  x, y, c, nc, ppos;
  long dist, sdist;                   // Used for least-squares match.

  int iw = _image->width;
  int ih = _image->height;

  ppos = 0;

  for (y = 0; y < ih; ++y) {
    for (x = 0; x < iw; ++x) {

      // find the nearest color in colormap[]

      sdist = 255L * 255L * 255L + 1L;
      for (c = 0; c < _ncolors; ++c) {

        int dr = _cimage->r[ppos] - colormap[c][0];
        int dg = _cimage->g[ppos] - colormap[c][1];
        int db = _cimage->b[ppos] - colormap[c][2];

        dist = dr * dr + dg * dg + db * db;
        if (dist < sdist) {
          nc = c;
          sdist = dist;
        }
      }

      // Nearest color found; plot it.

      XPutPixel(_image, x, y, pixel[nc]);

      ++ppos;
    }
  }
}
