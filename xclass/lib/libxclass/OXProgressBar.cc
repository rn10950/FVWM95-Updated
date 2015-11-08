/**************************************************************************
 
    This file is part of xclass, a Win95-looking GUI toolkit.
    Copyright (C) 1996, 1997 David Barth, Hector Peraza.
 
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
 
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.
 
    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 
**************************************************************************/

#include <xclass/utils.h>
#include <xclass/OXFrame.h>
#include <xclass/OXProgressBar.h>
#include <xclass/OXFont.h>
#include <xclass/OResourcePool.h>


#define USE_FONT_BORDER

//---------------------------------------------------------------------

OXProgressBar::OXProgressBar(const OXWindow *p, int w, int h,
                             int mode, int ID,
                             unsigned int options, unsigned long back) :
  OXFrame(p, w, h, options | SUNKEN_FRAME, back) {

    _widgetID = ID;
    _widgetType = "OXProgressBar";
    _msgObject = p;

    _min = 0;
    _pos = 0;
    _laststop = -1;
    _max = 100;
    _mode = mode;
    _show = True;
    _fltxt = False;

#ifdef USE_FONT_BORDER
    _font = _client->GetFont("Helvetica -12 bold");
#else
    _font = GetResourcePool()->GetDefaultFont();
#endif

    XGCValues gcv;
    int mask = GCForeground | GCBackground;
 
    gcv.foreground = _defaultSelectedBackground;  
    gcv.background = _backPixel;
    _gc_marked = XCreateGC(GetDisplay(), _id, mask, &gcv);

#ifdef USE_FONT_BORDER
    mask = GCForeground | GCFont;
    gcv.foreground = _whitePixel;
    gcv.font = _font->GetId();
    _white_gc = XCreateGC(GetDisplay(), _id, mask, &gcv);
    gcv.foreground = _blackPixel;
    _black_gc = XCreateGC(GetDisplay(), _id, mask, &gcv);
#else
    mask = GCFont | GCPlaneMask | GCFunction;
    gcv.function = GXinvert;
    gcv.plane_mask = gcv.foreground ^ _backPixel;
    gcv.font = _font->GetId();
    _pro_gc = XCreateGC(GetDisplay(), _id, mask, &gcv);
#endif
}

OXProgressBar::~OXProgressBar() {
  XFreeGC(GetDisplay(), _gc_marked);
#ifdef USE_FONT_BORDER
  XFreeGC(GetDisplay(), _white_gc);
  XFreeGC(GetDisplay(), _black_gc);
  _client->FreeFont((OXFont *)_font);
#else
  XFreeGC(GetDisplay(), _pro_gc);
#endif
}

//---------------------------------------------------------------------

void OXProgressBar::SetColor(unsigned int color) {
  XSetForeground(GetDisplay(), _gc_marked, color);
#ifndef USE_FONT_BORDER
  XSetForeground(GetDisplay(), _pro_gc, color);
  XSetPlaneMask(GetDisplay(), _pro_gc, color ^ _backPixel);
#endif
  NeedRedraw();
}

void OXProgressBar::_Redraw() {
  if (_max == _min) return;

  double ds = ((double) (_w - 3.0) * (double) _pos) / (double) (_max - _min);
  int stop = (int) ds;

  // do we really need a Redraw ?
  if (_laststop == stop) return;

  NeedRedraw(False/*_laststop > stop*/);
  _laststop = stop;
}

void OXProgressBar::_DoRedraw() {

  if (_max == _min) return;

#if 1
  double ds = ((double) (_w - 3.0) * (double) _pos) / (double) (_max - _min);
  int stop = (int) ds;
#else
  int stop = ((_w - 3) * _pos) / (_max - _min);
#endif

  _laststop = stop;

  OXFrame::_DoRedraw();

  if (_bw < 2)
    DrawRectangle(_bckgndGC, 1, 1, _w - 3, _h - 3);
  if (_bw < 1)
    DrawRectangle(_bckgndGC, 0, 0, _w - 1, _h - 1);

  if (_mode == PROGBAR_MODE1) {
    int x, step = 9;
    for (x = 0; x < stop && x < _w; x += step+1) {
      if (x + 14 > _w) step = _w - x - 4;
      if (step <= 0) break;
      FillRectangle(_gc_marked, x + 2, 2, step, _h - 4);
      DrawLine(_bckgndGC, x + step + 2, 2, x + step + 2, _h - 3);
    }
    if (x < _w - 4)
      FillRectangle(_bckgndGC, x+2, 2, _w - x - 4, _h - 4);
  } else {
    if (stop > 1)
      FillRectangle(_gc_marked, 2, 2, stop-1, _h - 4);
    if (stop < _w - 4)
      FillRectangle(_bckgndGC, stop+2, 2, _w - stop - 4, _h - 4);
  }

  if (_show) {
    OFontMetrics fm;
    int x, y, len;

    _font->GetFontMetrics(&fm);
    float percent = ((((float)_pos - (float)_min) /
                      ((float)_max - (float)_min)) *100);
    char _perc[5];
    sprintf(_perc, "%.0f%%", percent);
    int _hloc = ((_h - (fm.ascent + fm.descent))/2);
    int _percSize = _font->TextWidth(_perc);
    if (_fltxt) {
      x = ((stop - _percSize)/2) +2;
      if (x < 2) x = 2;
    } else {
      x = ((_w - _percSize)/2);
    }
    y = _hloc + fm.ascent;
    len = strlen(_perc);
#ifdef USE_FONT_BORDER
    DrawString(_black_gc, x-1, y-1, _perc, len);
    DrawString(_black_gc, x,   y-1, _perc, len);
    DrawString(_black_gc, x+1, y-1, _perc, len);
    DrawString(_black_gc, x-1, y,   _perc, len);
    DrawString(_black_gc, x+1, y,   _perc, len);
    DrawString(_black_gc, x-1, y+1, _perc, len);
    DrawString(_black_gc, x,   y+1, _perc, len);
    DrawString(_black_gc, x+1, y+1, _perc, len);
    DrawString(_white_gc, x,   y,   _perc, len);
#else
    DrawString(_pro_gc, x, y, _perc, strlen(_perc));
#endif
  }
}
