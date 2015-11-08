/**************************************************************************

    This file is part of rx320, a control program for the Ten-Tec RX320
    receiver. Copyright (C) 2000-2004, Hector Peraza.

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
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>
#include <math.h>

#include <xclass/OXClient.h>
#include <xclass/OResourcePool.h>

#include "OXSlideRuler.h"


//----------------------------------------------------------------------

OXSlideRuler::OXSlideRuler(const OXWindow *p, OXMain *m, int w, int h,
                           unsigned int options) :
  OXFrame(p, w, h, options | OWN_BKGND) {

  _rxmain = m;
  _sfont = _client->GetFont("Helvetica -10");

  _pix = None;

  CreatePixmap();

  XGCValues gcv;
  _gc = new OXGC(GetDisplay(), _id, 0L, &gcv);
  _gc->SetForeground(_blackPixel);
  _gc->SetFont(_sfont->GetId());

  SetBackgroundColor(_blackPixel);

  _draw_pending = True;

  _bdown = False;

  _freq = 0;
  _tickval = 10;

  XGrabButton(GetDisplay(), Button1, AnyModifier, _id, False,
              ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
              GrabModeAsync, GrabModeAsync, None, None);
}

OXSlideRuler::~OXSlideRuler() {
  XFreePixmap(GetDisplay(), _pix);
  delete _gc;
  _client->FreeFont(_sfont);
}

ODimension OXSlideRuler::GetDefaultSize() const {
  return ODimension(_w, _sfont->TextHeight() + 9 + _insets.t + _insets.b);
}

void OXSlideRuler::CreatePixmap() {
  if (_pix != None) XFreePixmap(GetDisplay(), _pix);
  _pixw = _w - _insets.l - _insets.r;
  _pixh = _h - _insets.t - _insets.b;
  _pix = XCreatePixmap(GetDisplay(), _id, _pixw, _pixh,
                       _client->GetDisplayDepth());
  if (_pix == None)
    FatalError("OXSlideRuler: cannot create buffer pixmap");
}

void OXSlideRuler::SetFreq(long freq) {
  _freq = freq;
  Draw();
}

void OXSlideRuler::SetStep(long fstep) {
  if (fstep < 100)
    _tickval = 10;
  else if (fstep < 1000)
    _tickval = 100;
  else
    _tickval = 1000;
  Draw();
}

void OXSlideRuler::ClearPixmap() {
  XFillRectangle(GetDisplay(), _pix, _blackGC, 0, 0, _pixw, _pixh);
}

void OXSlideRuler::DrawTicks() {
  int  i, tsize;
  long f, freq;

  // round to nearest _tickval
  freq = ((_freq + (_tickval / 2)) / _tickval) * _tickval;

  f = freq - (_pixw / 2) * _tickval; 

  _gc->SetForeground(_client->GetColorByName("#00c0c0"));
  //_gc->SetForeground(_client->GetColorByName("#00c000"));
  for (i = 0; i < _pixw; ++i) {
    if ((f >= 0) && (f <= 30000000)) {
      tsize = 0;
      if ((f % (_tickval * 2L)) == 0L) tsize = 3;
      if ((f % (_tickval * 10L)) == 0L) tsize = 5;
      if ((f % (_tickval * 50L)) == 0L) tsize = 8;
      if ((f % (_tickval * 100L)) == 0L) tsize = 10;
      if (tsize > 0) {
        XDrawLine(GetDisplay(), _pix, _gc->GetGC(),
                  i, _pixh - tsize - 2, i, _pixh - 2);
      }
    }
    f += _tickval;
  }

  // the -40..+40 trick below is to allow partial drawing of
  // the labels at the edges

  f = freq - ((_pixw + 80) / 2) * _tickval; 

  _gc->SetForeground(_client->GetColorByName("#ffff00"));
  for (i = -40; i < _pixw + 40; ++i) {
    if ((f >= 0) && (f <= 30000000)) {
      if ((f % (_tickval * 100L)) == 0L) {
        int mhz, khz;
        char str[12];

        mhz = f / 1000000L;
        khz = (f / 1000L) % 1000L;

        if (_tickval < 100)
          sprintf(str, "%2d.%03d", mhz, khz);
        else if (_tickval < 1000)
          sprintf(str, "%2d.%02d", mhz, khz / 10);
        else
          sprintf(str, "%2d.%1d", mhz, khz / 100);

        int tw = _sfont->XTextWidth(str);

        XDrawString(GetDisplay(), _pix, _gc->GetGC(),
                    i - tw / 2, _pixh - 13,
                    str, strlen(str));
      }
    }
    f += _tickval;
  }
}

void OXSlideRuler::DrawBands() {
  long freq, fmin, fmax;
  int  i, xmin, xmax;

  SBand *bands = _rxmain->GetBandInfo();

  // round to nearest _tickval
  freq = ((_freq + (_tickval / 2)) / _tickval) * _tickval;

  fmin = freq - (_pixw / 2) * _tickval; 
  fmax = freq + (_pixw / 2) * _tickval + 1; 

  // since bands may overlap, we'll draw the bands first,
  // then the labels to avoid one band erasing the other's label.

  _gc->SetForeground(_client->GetColorByName("#0000c0"));
  for (i = 0; bands[i].name != 0; ++i) {
    if ((bands[i].lofreq < fmax) && (bands[i].hifreq > fmin)) {
      xmin = (bands[i].lofreq - freq) / _tickval + _pixw / 2;
      xmax = (bands[i].hifreq - freq) / _tickval + _pixw / 2;

      if (xmin < 0) xmin = 0;
      if (xmin > _pixw) continue;
      if (xmax < 0) continue;
      if (xmax > _pixw) xmax = _pixw;

      XFillRectangle(GetDisplay(), _pix, _gc->GetGC(),
                     xmin, _pixh - 10, xmax - xmin, 5);
    }
  }

  _gc->SetForeground(_client->GetColorByName("#00c0c0"));
  for (i = 0; bands[i].name != 0; ++i) {
    if ((bands[i].lofreq < fmax) && (bands[i].hifreq > fmin)) {
      xmin = (bands[i].lofreq - freq) / _tickval + _pixw / 2;
      xmax = (bands[i].hifreq - freq) / _tickval + _pixw / 2;

      int tw = _sfont->XTextWidth(bands[i].name);
      int xl = (xmax + xmin - tw) / 2;

      if (((xl + tw/2) > 0) || ((xl - tw/2) < _pixw-1)) {
        XDrawString(GetDisplay(), _pix, _gc->GetGC(),
                    xl, _pixh - 7, bands[i].name, strlen(bands[i].name));
      }
    }
  }
}

void OXSlideRuler::DrawCursor() {
  int cx = _pixw / 2;
  _gc->SetForeground(_client->GetColorByName("#ff0000"));
  XDrawLine(GetDisplay(), _pix, _gc->GetGC(), cx, 0, cx, _pixh);
}

void OXSlideRuler::Draw() {
  _draw_pending = True;
  NeedRedraw();
}

void OXSlideRuler::DoDraw() {
  ClearPixmap();
  DrawBands();
  DrawTicks();
  DrawCursor();
  //NeedRedraw(False);
  _draw_pending = False;
}

int OXSlideRuler::HandleButton(XButtonEvent *event) {
  if (event->type == ButtonPress) {
    _bdown = True;
    _lastx = event->x;
  } else {
    _bdown = False;
  }
  return True;
}

int OXSlideRuler::HandleMotion(XMotionEvent *event) {
  int  dx;
  long freq, df;

  if (_bdown) {
    dx = _lastx - event->x;
    _lastx = event->x;

    // round to nearest _tickval
    freq = ((_freq + (_tickval / 2)) / _tickval) * _tickval;

    df = dx * _tickval;
    _freq = freq + df;

    _rxmain->TuneTo(_freq, dx);

    //Draw();  // this will be called back by TuneTo()
  }

  return True;
}

void OXSlideRuler::_DoRedraw() {
  DrawBorder(); //OXFrame::_DoRedraw();
  if ((_pixw != _w - _insets.l - _insets.r) ||
      (_pixh = _h - _insets.t - _insets.b)) {
    CreatePixmap();
    Draw();
  }
  if (_draw_pending) DoDraw();
  CopyArea(_pix, _id, _gc->GetGC(), 0, 0, _pixw, _pixh,
           _insets.l, _insets.t);
}
