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

#include <stdio.h>
#include <stdlib.h>

#include <X11/keysym.h>

#include <xclass/OXLabel.h>
#include <xclass/OXMsgBox.h>         // for ID_OK, ID_CANCEL
#include <xclass/OLayout.h>
#include <xclass/O2ColumnsLayout.h>
#include <xclass/OGC.h>
#include <xclass/OColor.h>
#include <xclass/OResourcePool.h>
#include <xclass/OXColorSelect.h>    // for OColorSelMessage
#include <xclass/OXColorDialog.h>


// TODO:
// - implement "custom" colors.
// - optimize the code, specially the one handling the colormap image
//   and dithering in pseudo-color modes; remove duplicated code.
// - improve the color allocation routine.
// - use a buffering pixmap for the colormap image.
// - clip user-entered RGB/HLS values to the allowed range.


#define CDLG_OK         100
#define CDLG_CANCEL     101
#define CDLG_ADD        102

#define CDLG_SPALETTE   200
#define CDLG_CPALETTE   201
#define CDLG_COLORPICK  202

#define CDLG_HTE        300
#define CDLG_LTE        301
#define CDLG_STE        302
#define CDLG_RTE        303
#define CDLG_GTE        304
#define CDLG_BTE        305


// "Basic" colors:

static unsigned char bcolor[48][3] = {
  { 0xff, 0x80, 0x80 }, { 0xff, 0xff, 0x80 },
  { 0x80, 0xff, 0x80 }, { 0x00, 0xff, 0x80 },
  { 0x80, 0xff, 0xff }, { 0x00, 0x80, 0xff },
  { 0xff, 0x80, 0xc0 }, { 0xff, 0x80, 0xff },

  { 0xff, 0x00, 0x00 }, { 0xff, 0xff, 0x00 },
  { 0x80, 0xff, 0x00 }, { 0x00, 0xff, 0x40 },
  { 0x00, 0xff, 0xff }, { 0x00, 0x80, 0xc0 },
  { 0x80, 0x80, 0xc0 }, { 0xff, 0x00, 0xff },

  { 0x80, 0x40, 0x40 }, { 0xff, 0x80, 0x40 },
  { 0x00, 0xff, 0x00 }, { 0x00, 0x80, 0x80 },
  { 0x00, 0x40, 0x80 }, { 0x80, 0x80, 0xff },
  { 0x80, 0x00, 0x40 }, { 0xff, 0x00, 0x80 },

  { 0x80, 0x00, 0x00 }, { 0xff, 0x80, 0x00 },
  { 0x00, 0x80, 0x00 }, { 0x00, 0x80, 0x40 },
  { 0x00, 0x00, 0xff }, { 0x00, 0x00, 0xa0 },
  { 0x80, 0x00, 0x80 }, { 0x80, 0x00, 0xff },

  { 0x40, 0x00, 0x00 }, { 0x80, 0x40, 0x00 },
  { 0x00, 0x40, 0x00 }, { 0x00, 0x40, 0x40 },
  { 0x00, 0x00, 0x80 }, { 0x00, 0x00, 0x40 },
  { 0x40, 0x00, 0x40 }, { 0x40, 0x00, 0x80 },

  { 0x00, 0x00, 0x00 }, { 0x80, 0x80, 0x00 },
  { 0x80, 0x80, 0x40 }, { 0x80, 0x80, 0x80 },
  { 0x40, 0x80, 0x80 }, { 0xc0, 0xc0, 0xc0 },
  { 0x40, 0x00, 0x40 }, { 0xff, 0xff, 0xff }
};


//----------------------------------------------------------------------

// OXColorPalette widget: this is just a grid of color cells of the
// specified size. Colors can be selectd by clicking on them or by 
// using the arrow keys.

OXColorPalette::OXColorPalette(const OXWindow *p, int cols, int rows, int id) :
  OXFrame(p, 10, 10, CHILD_FRAME), OXWidget(id, "OXColorPalette") {

    _cw = 20;
    _ch = 17;

    _rows = rows;
    _cols = cols;

    _cx = _cy = 0;

    _colors = new OColor[_rows * _cols];
    _pixels = new unsigned long[_rows * _cols];

    for (int i = 0; i < _rows * _cols; ++i) {
      _colors[i] = OColor(255, 255, 255);
      _pixels[i] = _client->GetColor(_colors[i]);
    }

    _drawGC = new OXGC(GetDisplay(), _id);

    XGrabButton(GetDisplay(), Button1, AnyModifier, _id, True,
                ButtonPressMask | ButtonReleaseMask | ButtonMotionMask,
                GrabModeAsync, GrabModeAsync, None, None);

    AddInput(FocusChangeMask);
}

OXColorPalette::~OXColorPalette() {
  for (int i = 0; i < _rows * _cols; ++i) {
    _client->FreeColor(_pixels[i]);
  }
  delete[] _colors;
  delete[] _pixels;
  delete _drawGC;
}

int OXColorPalette::HandleButton(XButtonEvent *event) {

  OXFrame::HandleButton(event);
  if (!IsEnabled()) return True;

  if (event->button != Button1) return False;

  if ((event->type == ButtonPress) && TakesFocus())
    RequestFocus();

  int cx = event->x / (_cw + 5);
  int cy = event->y / (_ch + 5);

  if (cx >= 0 && cx < _cols && cy >= 0 && cy < _rows) {

    _DrawFocusHilite(False);

    _cx = cx;
    _cy = cy;

    _DrawFocusHilite(HasFocus());

    OWidgetMessage msg(MSG_COLORSEL, MSG_CLICK, _widgetID);
    SendMessage(_msgObject, &msg);

  }

  return True;
}

int OXColorPalette::HandleMotion(XMotionEvent *event) {

  OXFrame::HandleMotion(event);
  if (!IsEnabled()) return True;

  int cx = event->x / (_cw + 5);
  int cy = event->y / (_ch + 5);

  if (cx >= 0 && cx < _cols && cy >= 0 && cy < _rows) {

    _DrawFocusHilite(False);

     _cx = cx;
     _cy = cy;

    _DrawFocusHilite(HasFocus());

  }

  return True;
}

int OXColorPalette::HandleKey(XKeyEvent *event) {

  if (event->type == KeyPress) {

    int cx = _cx;
    int cy = _cy;

    int keysym = XKeycodeToKeysym(GetDisplay(), event->keycode, 0);

    switch (keysym) {
      case XK_Left:
        if (cx > 0) --cx;
        break;

      case XK_Right:
        if (cx < _cols - 1) ++cx;
        break;

      case XK_Up:
        if (cy > 0) --cy;
        break;

      case XK_Down:
        if (cy < _rows - 1) ++cy;
        break;

      case XK_Home:
        cx = cy = 0;
        break;

      case XK_End:
        cx = _cols - 1;
        cy = _rows - 1;
        break;
    }
    
    if (cx != _cx || cy != _cy) {

      _DrawFocusHilite(False);

      _cx = cx;
      _cy = cy;

      _DrawFocusHilite(HasFocus());

      OWidgetMessage msg(MSG_COLORSEL, MSG_CLICK, _widgetID);
      SendMessage(_msgObject, &msg);

    }
  }

  return True;
}

void OXColorPalette::SetColors(OColor colors[]) {
  for (int i = 0; i < _rows * _cols; ++i) SetColor(i, colors[i]);
  NeedRedraw();
}

void OXColorPalette::SetColor(int ix, OColor color) {
  _colors[ix] = color;
  _client->FreeColor(_pixels[ix]);
  _pixels[ix] = _client->GetColor(_colors[ix]);
  NeedRedraw();
}

void OXColorPalette::SetCurrentCellColor(OColor color) {
  SetColor(_cy * _cols + _cx, color);
}

void OXColorPalette::SetCellSize(int w, int h) {
  _cw = w;
  _ch = h;
  NeedRedraw();
}

OColor OXColorPalette::GetCurrentColor() const {
  if (_cx >= 0 && _cy >= 0)
    return GetColorByIndex(_cy * _cols + _cx);
  else
    return OColor(0, 0, 0);
}

void OXColorPalette::_DoRedraw() {
  int i, j, k, x, y;

  k = 0;
  y = 2;
  for (i = 0; i < _rows; ++i) {
    x = 2;
    for (j = 0; j < _cols; ++j) {
      _Draw3dRectangle(SUNKEN_FRAME | DOUBLE_BORDER, x, y, _cw, _ch);
      _drawGC->SetForeground(_pixels[k++]);
      FillRectangle(_drawGC->GetGC(), x + 2, y + 2, _cw - 4, _ch - 4);
      x += _cw + 5;
    }
    y += _ch + 5;
  }

  _DrawFocusHilite(HasFocus());
}

void OXColorPalette::_GotFocus() {
  OXFrame::_GotFocus();
  AddInput(KeyPressMask | KeyReleaseMask);
  NeedRedraw(False);
}

void OXColorPalette::_LostFocus() {
  OXFrame::_LostFocus();
  RemoveInput(KeyPressMask | KeyReleaseMask);
  NeedRedraw(True);
}

void OXColorPalette::_DrawFocusHilite(int onoff) {
  if (_cx >= 0 && _cy >= 0) {
    GC gc = onoff ? GetResourcePool()->GetFocusHiliteGC()->GetGC() :
                    _bckgndGC;
    DrawRectangle(gc, _cx * (_cw + 5) + 0, _cy * (_ch + 5) + 0,
                  _cw + 3, _ch + 3);
  }
}

//----------------------------------------------------------------------

// OXColorPick is an HLS color picker widget. It consists of two 
// elements: a color map window from where the user can select the
// hue and saturation level of a color, and a slider to select
// color's lightness.

#define CLICK_NONE  0
#define CLICK_HS    1
#define CLICK_L     2

OXColorPick::OXColorPick(const OXWindow *p, int w, int h, int id) :
  OXFrame(p, w, h, CHILD_FRAME), OXWidget(id, "OXColorPick") {

  _colormapRect = ORectangle(1, 1, w - 33 - 2, h - 2);
  _sliderRect = ORectangle(w - 18 - 2, 1, 10, h - 2);

  _ncolors = 0;

  _CreateImages();

  _cx = 0;
  _cy = 0;
  _cz = _Limage->height / 2;

  _click = CLICK_NONE;

  _UpdateCurrentColor();
  _InitImages();

  XGrabButton(GetDisplay(), Button1, AnyModifier, _id, True,
              ButtonPressMask | ButtonReleaseMask | ButtonMotionMask,
              GrabModeAsync, GrabModeAsync, None, None);
}

OXColorPick::~OXColorPick() {
  XDestroyImage(_HSimage);
  XDestroyImage(_Limage);
  _FreeColors();
}

int OXColorPick::HandleButton(XButtonEvent *event) {

  OXFrame::HandleButton(event);
  if (!IsEnabled()) return True;

  if (event->button != Button1) return False;

  if (event->type == ButtonPress) {
    if (_colormapRect.contains(event->x, event->y)) {

      _click = CLICK_HS;
      _SetHScursor(event->x - _colormapRect.x, event->y - _colormapRect.y);

    } else if (event->x > _sliderRect.x) {

      _click = CLICK_L;
      _SetLcursor(event->y - _sliderRect.y);

    } else {  // ButtonRelease

      _click = CLICK_NONE;

    }
  }

  _UpdateCurrentColor();
  if (_click == CLICK_HS) _SetSliderColor();

  OWidgetMessage msg(MSG_COLORSEL, MSG_CLICK, _widgetID);
  SendMessage(_msgObject, &msg);

  return True;
}

int OXColorPick::HandleMotion(XMotionEvent *event) {

  OXFrame::HandleMotion(event);
  if (!IsEnabled()) return True;

  if (_click == CLICK_HS) {

    _SetHScursor(event->x - _colormapRect.x, event->y - _colormapRect.y);

  } else if (_click == CLICK_L) {

    _SetLcursor(event->y - _sliderRect.y);

  } else {

    return True;

  }

  _UpdateCurrentColor();
  if (_click == CLICK_HS) _SetSliderColor();

  OWidgetMessage msg(MSG_COLORSEL, MSG_CLICK, _widgetID);
  SendMessage(_msgObject, &msg);

  return True;
}

void OXColorPick::_CreateImages() {
  int depth = _client->GetDisplayDepth();
  int bitmap_pad;

  if (depth <= 8)
    bitmap_pad = 8;
  else if (depth <= 16)
    bitmap_pad = 16;
  else
    bitmap_pad = 32;

  _HSimage = XCreateImage(GetDisplay(),
                          DefaultVisual(GetDisplay(),
                          _client->GetScreenNumber()),
                          depth, ZPixmap, 0, NULL,
                          _colormapRect.w, _colormapRect.h,
                          bitmap_pad, 0);

  _HSimage->data = (char *) calloc(1, _HSimage->bytes_per_line *
                                      _HSimage->height);

  _Limage = XCreateImage(GetDisplay(),
                         DefaultVisual(GetDisplay(),
                         _client->GetScreenNumber()),
                         depth, ZPixmap, 0, NULL,
                         _sliderRect.w, _sliderRect.h,
                         bitmap_pad, 0);

  _Limage->data = (char *) calloc(1, _Limage->bytes_per_line *
                                     _Limage->height);
}

// Create a dithered version of the color map and lightness images for
// display modes with reduced number of colors. The Floyd-Steinberg error
// diffusion dithering algorithm is used.

// This routine is called in PseudoColor modes only.

void OXColorPick::_AllocColors() {

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

void OXColorPick::_FreeColors() {
  if (_ncolors > 0) {
    XFreeColors(GetDisplay(), _client->GetDefaultColormap(),
                pixel, _ncolors, 0);
    _ncolors = 0;
  }
}

#define IMG_HS  1
#define IMG_L   2

#define WIDTH 200  // max width

void OXColorPick::_CreateDitheredImage(XImage *image, int which) {
  XColor line[WIDTH];
  struct { int r, g, b; } ed[WIDTH], ef;
  int  x, y, c, nc, v, e[4];
  long dist, sdist;

  int iw = image->width;
  int ih = image->height;

  for (x = 0; x < iw; ++x) {
    ed[x].r = ed[x].g = ed[x].b = 0;
  }

  if (_ncolors == 0) _AllocColors();

  OColor color;

  for (y = 0; y < ih; ++y) {

    if (which == IMG_HS) {

      for (x = 0; x < iw; ++x) {
        int h, l, s;

        h = x * 255 / iw;
        l = 128;
        s = (ih - y) * 255 / ih;

        color.SetHLS(h, l, s);

        line[x].red   = color.GetR();
        line[x].green = color.GetG();
        line[x].blue  = color.GetB();
      }

    } else if (which == IMG_L) {

      int l = (ih - y) * 255 / ih;

      color.SetHLS(_currentColor.GetH(), l, _currentColor.GetS());

      for (int x = 0; x < iw; ++x) {
        line[x].red   = color.GetR();
        line[x].green = color.GetG();
        line[x].blue  = color.GetB();
      }

    } else {

      return;

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

      XPutPixel(image, x, y, pixel[nc]);

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
}

void OXColorPick::_InitImages() {

  // initialize _HSimage

  Visual *visual = DefaultVisual(GetDisplay(), _client->GetScreenNumber());
  //int ncolors = XDisplayCells(GetDisplay(), _client->GetScreenNumber());

  if (visual->c_class == TrueColor || visual->c_class == DirectColor) {
    OColor color;

    int rmask = visual->red_mask, rmax = rmask, rshift = 0;
    if (rmax != 0) while ((rmax & 1) == 0) { rmax >>= 1; ++rshift; }

    int gmask = visual->green_mask, gmax = gmask, gshift = 0;
    if (gmax != 0) while ((gmax & 1) == 0) { gmax >>= 1; ++gshift; }

    int bmask = visual->blue_mask, bmax = bmask, bshift = 0;
    if (bmax != 0) while ((bmax & 1) == 0) { bmax >>= 1; ++bshift; }

    for (int y = 0; y < _HSimage->height; ++y) {
      for (int x = 0; x < _HSimage->width; ++x) {
        int h, l, s;

        h = x * 255 / _HSimage->width;
        l = 128;
        s = (_HSimage->height - y) * 255 / _HSimage->height;

        int r, g, b;

        OColor::HLS2RGB(h, l, s, &r, &g, &b);

        r = r * rmax / 255;
        g = g * gmax / 255;
        b = b * bmax / 255;
        XPutPixel(_HSimage, x, y, (r << rshift) | (g << gshift) | (b << bshift));
      }
    }
  } else {
    _CreateDitheredImage(_HSimage, IMG_HS);
  }

  // initialize _Limage

  _SetSliderColor();
}

void OXColorPick::_SetSliderColor() {

  Visual *visual = DefaultVisual(GetDisplay(), _client->GetScreenNumber());
  //int ncolors = XDisplayCells(GetDisplay(), _client->GetScreenNumber());

  if (visual->c_class == TrueColor || visual->c_class == DirectColor) {
    OColor color;

    int rmask = visual->red_mask, rmax = rmask, rshift = 0;
    if (rmax != 0) while ((rmax & 1) == 0) { rmax >>= 1; ++rshift; }

    int gmask = visual->green_mask, gmax = gmask, gshift = 0;
    if (gmax != 0) while ((gmax & 1) == 0) { gmax >>= 1; ++gshift; }

    int bmask = visual->blue_mask, bmax = bmask, bshift = 0;
    if (bmax != 0) while ((bmax & 1) == 0) { bmax >>= 1; ++bshift; }

    OColor c;
    for (int y = 0; y < _Limage->height; ++y) {

      int l = (_Limage->height - y) * 255 / _Limage->height;

      int r, g, b;

      OColor::HLS2RGB(_currentColor.GetH(), l, _currentColor.GetS(), &r, &g, &b);

      r = r * rmax / 255;
      g = g * gmax / 255;
      b = b * bmax / 255;
      unsigned long pixel = (r << rshift) | (g << gshift) | (b << bshift);

      for (int x = 0; x < _Limage->width; ++x) {
        XPutPixel(_Limage, x, y, pixel);
      }
    }
  } else {
    _CreateDitheredImage(_Limage, IMG_L);
  }

  NeedRedraw();
}

void OXColorPick::SetColor(OColor color) {

  _currentColor = color;

  _SetHScursor(_currentColor.GetH() * (_HSimage->width - 1) / 255,
               (255 - _currentColor.GetS()) * (_HSimage->height - 1) / 255);

  _SetLcursor((255 - _currentColor.GetL()) * (_Limage->height - 1) / 255);

  _SetSliderColor();
}

void OXColorPick::_UpdateCurrentColor() {

  int h = _cx * 255 / (_HSimage->width - 1);
  int l = (_Limage->height - _cz - 1) * 255 / (_Limage->height - 1);
  int s = (_HSimage->height - _cy - 1) * 255 / (_HSimage->height - 1);

  _currentColor.SetHLS(h, l, s);
}

void OXColorPick::_DoRedraw() {
  OXFrame::DrawBorder();

  _Draw3dRectangle(SUNKEN_FRAME, _colormapRect.x - 1, _colormapRect.y - 1,
                                 _colormapRect.w + 2, _colormapRect.h + 2);
  XPutImage(GetDisplay(), _id, _bckgndGC, _HSimage, 0, 0,
            _colormapRect.x, _colormapRect.y,
            _HSimage->width, _HSimage->height);

  _Draw3dRectangle(SUNKEN_FRAME, _sliderRect.x - 1, _sliderRect.y - 1,
                                 _sliderRect.w + 2, _sliderRect.h + 2);
  XPutImage(GetDisplay(), _id, _bckgndGC, _Limage, 0, 0,
            _sliderRect.x, _sliderRect.y,
            _Limage->width, _Limage->height);

  _DrawHScursor(True);
  _DrawLcursor(True);
}

void OXColorPick::_SetHScursor(int x, int y) {

  _DrawHScursor(False);

  _cx = x;
  _cy = y;

  if (_cx < 0)
    _cx = 0;
  else if (_cx >= _HSimage->width)
    _cx = _HSimage->width - 1;

  if (_cy < 0)
    _cy = 0;
  else if (_cy >= _HSimage->height)
    _cy = _HSimage->height - 1;

  _DrawHScursor(True);

}

void OXColorPick::_SetLcursor(int z) {

  _DrawLcursor(False);

  _cz = z - _sliderRect.y + 1;

  if (_cz < 0)
    _cz = 0;
  else if (_cz >= _Limage->height)
    _cz = _Limage->height - 1;

  _DrawLcursor(True);

}

void OXColorPick::_DrawHScursor(int onoff) {
  if (onoff) {
    int x, y;
    XRectangle rect;

    x = _cx + _colormapRect.x;
    y = _cy + _colormapRect.y;

    rect.x = _colormapRect.x;
    rect.y = _colormapRect.x;
    rect.width = _colormapRect.w;
    rect.height = _colormapRect.h;
    XSetClipRectangles(GetDisplay(), _blackGC, 0, 0, &rect, 1, Unsorted);

    FillRectangle(_blackGC, x - 9, y - 1, 5, 3);
    FillRectangle(_blackGC, x - 1, y - 9, 3, 5);
    FillRectangle(_blackGC, x + 5, y - 1, 5, 3);
    FillRectangle(_blackGC, x - 1, y + 5, 3, 5);

    XSetClipMask(GetDisplay(), _blackGC, None);
  } else {
    int x, y, w, h;

    x = _cx - 9; w = 19;
    y = _cy - 9; h = 19;

    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }

    if (x + w > _HSimage->width) w = _HSimage->width - x;
    if (y + h > _HSimage->width) h = _HSimage->height - y;

    XPutImage(GetDisplay(), _id, _bckgndGC, _HSimage, x, y,
              _colormapRect.x + x, _colormapRect.y + y, w, h);
  }
}

void OXColorPick::_DrawLcursor(int onoff) {
  int l = _sliderRect.x + _sliderRect.w + 3;
  int r = l + 5;
  int t = _cz - 5 + _sliderRect.y;
  int b = t + 10;

  XPoint points[3];

  int m = (t + b) >> 1;

  points[0].x = r;
  points[0].y = t;
  points[1].x = r;
  points[1].y = b;
  points[2].x = l;
  points[2].y = m;

  GC gc = (onoff) ? _blackGC : _bckgndGC;

  XFillPolygon(GetDisplay(), _id, gc, points, 3, Convex, CoordModeOrigin);
}

//----------------------------------------------------------------------

// Finally the color selection dialog...

#define C_X  175  // Win95: 177
#define C_Y  180  // Win95: 189

OXColorDialog::OXColorDialog(const OXWindow *p, const OXWindow *m,
                             int *retc, OColor *color) :
  OXTransientFrame(p, m, 200, 150, HORIZONTAL_FRAME) {

    _retc = retc;
    _retColor = color;

    if (_retc) *_retc = ID_CANCEL;

    OXVerticalFrame *vf1 = new OXVerticalFrame(this);
    OXVerticalFrame *vf2 = new OXVerticalFrame(this);

    AddFrame(vf1, new OLayoutHints(LHINTS_LEFT | LHINTS_EXPAND_Y));
    AddFrame(vf2, new OLayoutHints(LHINTS_LEFT | LHINTS_EXPAND_Y));

    //----------------------------------------- Left panel

    // basic colors

    vf1->AddFrame(new OXLabel(vf1, new OHotString("&Basic colors:")),
                  new OLayoutHints(LHINTS_NORMAL, 5, 0, 7, 2));

    _palette = new OXColorPalette(vf1, 8, 6, CDLG_SPALETTE);
    vf1->AddFrame(_palette, new OLayoutHints(LHINTS_NORMAL, 5, 5, 0, 0));
    _palette->Associate(this);

    for (int i = 0; i < 48; ++i)
      _palette->SetColor(i, OColor(bcolor[i][0], bcolor[i][1], bcolor[i][2]));

    // custom colors

    vf1->AddFrame(new OXLabel(vf1, new OHotString("&Custom colors:")),
                  new OLayoutHints(LHINTS_NORMAL, 5, 0, 15, 2));

    _cpalette = new OXColorPalette(vf1, 8, 3, CDLG_CPALETTE);
    vf1->AddFrame(_cpalette, new OLayoutHints(LHINTS_NORMAL, 5, 5, 0, 0));
    _cpalette->Associate(this);
    LoadCustomColors();

    // button frame

    OXHorizontalFrame *hf = new OXHorizontalFrame(vf1, 10, 10, FIXED_WIDTH);
    vf1->AddFrame(hf, new OLayoutHints(LHINTS_BOTTOM | LHINTS_LEFT, 5, 5, 10, 5));

    OXTextButton *ok = new OXTextButton(hf, new OHotString("OK"), CDLG_OK);
    OXTextButton *cancel = new OXTextButton(hf, new OHotString("Cancel"), CDLG_CANCEL);

    hf->AddFrame(ok, new OLayoutHints(LHINTS_BOTTOM | LHINTS_EXPAND_X, 0, 5, 0, 0));
    hf->AddFrame(cancel, new OLayoutHints(LHINTS_BOTTOM | LHINTS_EXPAND_X));

    int w = ok->GetDefaultWidth();
    w = max(w, cancel->GetDefaultWidth());
    hf->Resize(2 * (w + 20), hf->GetDefaultHeight());

    ok->Associate(this);
    cancel->Associate(this);

    //----------------------------------------- Right panel

    // colormap frame

    _colors = new OXColorPick(vf2, C_X + 33, C_Y, CDLG_COLORPICK);
    vf2->AddFrame(_colors, new OLayoutHints(LHINTS_LEFT | LHINTS_TOP, 5, 5, 15, 5));
    _colors->Associate(this);

    if (color) _colors->SetColor(*color);

    // color sample frame

    OXHorizontalFrame *hf3 = new OXHorizontalFrame(vf2);
    vf2->AddFrame(hf3, new OLayoutHints(LHINTS_LEFT | LHINTS_TOP, 5, 5, 2, 5));

    OXVerticalFrame *vf3 = new OXVerticalFrame(hf3);
    hf3->AddFrame(vf3, new OLayoutHints(LHINTS_LEFT | LHINTS_TOP));

    _sample = new OXFrame(vf3, 60, 42, SUNKEN_FRAME | OWN_BKGND);
    vf3->AddFrame(_sample, new OLayoutHints(LHINTS_LEFT | LHINTS_TOP));

    vf3->AddFrame(new OXLabel(vf3, new OString("Color")),
                  new OLayoutHints(LHINTS_CENTER_X | LHINTS_TOP, 0, 0, 2, 0));

    if (color)
      _currentColor = _client->GetColor(*color);
    else
      _currentColor = _client->GetColorByName("red");

    _sample->SetBackgroundColor(_currentColor);

    OXCompositeFrame *cf = new OXCompositeFrame(hf3);
    hf3->AddFrame(cf, new OLayoutHints(LHINTS_LEFT | LHINTS_TOP, 10, 0, 0, 0));
    cf->SetLayoutManager(new O2ColumnsLayout(cf, 4, 1));

    cf->AddFrame(new OXLabel(cf, new OHotString("Hu&e:")), NULL);
    cf->AddFrame(_hte = new OXTextEntry(cf, NULL, CDLG_HTE), NULL);
    _hte->Resize(35, _hte->GetDefaultHeight());
    cf->AddFrame(new OXLabel(cf, new OHotString("&Sat:")), NULL);
    cf->AddFrame(_ste = new OXTextEntry(cf, NULL, CDLG_STE), NULL);
    _ste->Resize(35, _ste->GetDefaultHeight());
    cf->AddFrame(new OXLabel(cf, new OHotString("&Lum:")), NULL);
    cf->AddFrame(_lte = new OXTextEntry(cf, NULL, CDLG_LTE), NULL);
    _lte->Resize(35, _lte->GetDefaultHeight());

    cf = new OXCompositeFrame(hf3);
    hf3->AddFrame(cf, new OLayoutHints(LHINTS_LEFT | LHINTS_TOP, 5, 0, 0, 0));
    cf->SetLayoutManager(new O2ColumnsLayout(cf, 4, 1));

    cf->AddFrame(new OXLabel(cf, new OHotString("&Red:")), NULL);
    cf->AddFrame(_rte = new OXTextEntry(cf, NULL, CDLG_RTE), NULL);
    _rte->Resize(35, _rte->GetDefaultHeight());
    cf->AddFrame(new OXLabel(cf, new OHotString("&Green:")), NULL);
    cf->AddFrame(_gte = new OXTextEntry(cf, NULL, CDLG_GTE), NULL);
    _gte->Resize(35, _gte->GetDefaultHeight());
    cf->AddFrame(new OXLabel(cf, new OHotString("Bl&ue:")), NULL);
    cf->AddFrame(_bte = new OXTextEntry(cf, NULL, CDLG_BTE), NULL);
    _bte->Resize(35, _bte->GetDefaultHeight());

    _hte->Associate(this);
    _lte->Associate(this);
    _ste->Associate(this);
    _rte->Associate(this);
    _gte->Associate(this);
    _bte->Associate(this);

    if (color) {
      UpdateRGBentries(color);
      UpdateHLSentries(color);
    }

    OXTextButton *add = new OXTextButton(vf2, new OHotString("&Add to Custom Colors"),
                                         CDLG_ADD);
    vf2->AddFrame(add, new OLayoutHints(LHINTS_BOTTOM | LHINTS_EXPAND_X,
                                        5, 5, 0, 5));
    add->Associate(this);

    SetDefaultAcceptButton(ok);
    SetDefaultCancelButton(cancel);

    SetFocusOwner(_palette);

    MapSubwindows();
    Resize(GetDefaultSize());

    //---- position relative to the parent's window

    CenterOnParent();

    //---- make the message box non-resizable

    SetWMSize(_w, _h);
    SetWMSizeHints(_w, _h, _w, _h, 0, 0);

    SetWindowName("Select color");
    SetIconName("Select color");
    SetClassHints("XCLASS", "dialog");

    SetMWMHints(MWM_DECOR_ALL | MWM_DECOR_RESIZEH | MWM_DECOR_MAXIMIZE |
                                MWM_DECOR_MINIMIZE | MWM_DECOR_MENU,
                MWM_FUNC_ALL | MWM_FUNC_RESIZE | MWM_FUNC_MAXIMIZE |
                               MWM_FUNC_MINIMIZE,
                MWM_INPUT_MODELESS);


    MapWindow();
    _client->WaitFor(this);
}

OXColorDialog::~OXColorDialog() {
  _client->FreeColor(_currentColor);
}

void OXColorDialog::UpdateRGBentries(OColor *c) {
  char tmp[20];

  sprintf(tmp, "%d", c->GetR());
  _rte->Clear();
  _rte->AddText(0, tmp);

  sprintf(tmp, "%d", c->GetG());
  _gte->Clear();
  _gte->AddText(0, tmp);

  sprintf(tmp, "%d", c->GetB());
  _bte->Clear();
  _bte->AddText(0, tmp);
}

void OXColorDialog::UpdateHLSentries(OColor *c) {
  char tmp[20];

  sprintf(tmp, "%d", c->GetH());
  _hte->Clear();
  _hte->AddText(0, tmp);

  sprintf(tmp, "%d", c->GetL());
  _lte->Clear();
  _lte->AddText(0, tmp);

  sprintf(tmp, "%d", c->GetS());
  _ste->Clear();
  _ste->AddText(0, tmp);
}

int OXColorDialog::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;
  unsigned long color;
  OColor c;

  switch (msg->type) {
    case MSG_BUTTON:
      switch (msg->action) {
        case MSG_CLICK:
          switch (wmsg->id) {
            case CDLG_ADD:
              c.SetColor(_client, _currentColor);
              _cpalette->SetCurrentCellColor(c);
              break;

            case CDLG_OK:
              if (_retc) *_retc = ID_OK;
              if (_retColor) {
                _retColor->SetRGB(atoi(_rte->GetString()),
                                  atoi(_gte->GetString()),
                                  atoi(_bte->GetString()));
              }
              //SaveCustomColors();
            case CDLG_CANCEL:
              SaveCustomColors();
              CloseWindow();
              break;
          }
          break;
      }
      break;

    case MSG_COLORSEL:
      switch (msg->action) {
        case MSG_CLICK:
          switch (wmsg->id) {
            case CDLG_SPALETTE:
              c = _palette->GetCurrentColor();
              color = _client->GetColor(c);
              _sample->SetBackgroundColor(color);
              _sample->NeedRedraw(True);
              _client->FreeColor(_currentColor);
              _currentColor = color;
              _colors->SetColor(c);
              UpdateRGBentries(&c);
              UpdateHLSentries(&c);
              break;

            case CDLG_CPALETTE:
              c = _cpalette->GetCurrentColor();
              color = _client->GetColor(c);
              _sample->SetBackgroundColor(color);
              _sample->NeedRedraw(True);
              _client->FreeColor(_currentColor);
              _currentColor = color;
              _colors->SetColor(c);
              UpdateRGBentries(&c);
              UpdateHLSentries(&c);
              break;

            case CDLG_COLORPICK:
              c = _colors->GetCurrentColor();
              color = _client->GetColor(c);
              _sample->SetBackgroundColor(color);
              _sample->NeedRedraw(True);
              _client->FreeColor(_currentColor);
              _currentColor = color;
              UpdateRGBentries(&c);
              UpdateHLSentries(&c);
              break;

          }
          break;
      }
      break;

    case MSG_TEXTENTRY:
      switch (msg->action) {
        case MSG_TEXTCHANGED:
          switch (wmsg->id) {
            case CDLG_HTE:
            case CDLG_LTE:
            case CDLG_STE:
              c.SetHLS(atoi(_hte->GetString()),
                       atoi(_lte->GetString()),
                       atoi(_ste->GetString()));
              color = _client->GetColor(c);
              _sample->SetBackgroundColor(color);
              _sample->NeedRedraw(True);
              _client->FreeColor(_currentColor);
              _currentColor = color;
              _colors->SetColor(c);
              UpdateRGBentries(&c);
              break;

            case CDLG_RTE:
            case CDLG_GTE:
            case CDLG_BTE:
              c.SetRGB(atoi(_rte->GetString()),
                       atoi(_gte->GetString()),
                       atoi(_bte->GetString()));
              color = _client->GetColor(c);
              _sample->SetBackgroundColor(color);
              _sample->NeedRedraw(True);
              _client->FreeColor(_currentColor);
              _currentColor = color;
              _colors->SetColor(c);
              UpdateHLSentries(&c);
              break;

          }
          break;
      }
      break;
  }

  return True;
}

void OXColorDialog::LoadCustomColors() {
  char *inipath, line[1024], arg[PATH_MAX];

  inipath = GetResourcePool()->FindIniFile("colordlg", INI_READ);
  if (!inipath) return;

  OIniFile ini(inipath, INI_READ);

  while (ini.GetNext(line)) {
    if (strcasecmp(line, "custom colors") == 0) {
      char tmp[50];

      for (int i = 0; i < 8 * 3; ++i) {
        sprintf(tmp, "color %d", i+1);
        if (ini.GetItem(tmp, arg)) {
          int r, g, b;

          if (sscanf(arg, "%d,%d,%d", &r, &g, &b) == 3) {
            if (r >= 0 && r <= 255 &&
                g >= 0 && g <= 255 &&
                b >= 0 && b <= 255)
              _cpalette->SetColor(i, OColor(r, g, b));
          }
        }
      }
    }
  }

  delete[] inipath;
}

void OXColorDialog::SaveCustomColors() {
  char *inipath, tmp[20], arg[100];

  inipath = GetResourcePool()->FindIniFile("colordlg", INI_WRITE);
  if (!inipath) return;

  OIniFile ini(inipath, INI_WRITE);

  ini.PutNext("custom colors");
  for (int i = 0; i < 8 * 3; ++i) {
    OColor color = _cpalette->GetColorByIndex(i);
    sprintf(tmp, "color %d", i+1);
    sprintf(arg, "%d,%d,%d",
                 color.GetR(),
                 color.GetG(),
                 color.GetB());
    ini.PutItem(tmp, arg);
  }
  ini.PutNewLine();

  delete[] inipath;
}
