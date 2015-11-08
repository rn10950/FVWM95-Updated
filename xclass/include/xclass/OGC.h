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

#ifndef __OGC_H
#define __OGC_H


#include <xclass/OBaseObject.h>
#include <xclass/OXObject.h>


//----------------------------------------------------------------------

class OXGC : public OXObject {
public: 
  OXGC(Display *dpy, Drawable d, unsigned long valuemask = 0L,
       XGCValues *values = NULL) {
    if (values)
      memcpy(&_values, values, sizeof(XGCValues));
    else
      memset(&_values, 0, sizeof(XGCValues));
    _gc = XCreateGC(_dpy = dpy, _d = d, _mask = valuemask, &_values);
  }
  OXGC(const OXGC *g) {
    memcpy(&_values, &g->_values, sizeof(XGCValues));
    _gc = XCreateGC(_dpy = g->_dpy, _d = g->_d, _mask = g->_mask, &_values); 
  }
  virtual ~OXGC() { XFreeGC(_dpy, _gc); }

  GC GetGC() const { return _gc; }

  void SetAttributes(unsigned long valuemask, XGCValues *values)
    { XChangeGC(_dpy, _gc, valuemask, values); }
  void SetFunction(int v) { _values.function = v; XChangeGC(_dpy, _gc, GCFunction, &_values); }
  void SetPlaneMask(unsigned long v) { _values.plane_mask = v; XChangeGC(_dpy, _gc, GCPlaneMask, &_values); }
  void SetBackground(unsigned long v) { _values.background = v; XChangeGC(_dpy, _gc, GCBackground, &_values); }
  void SetForeground(unsigned long v) { _values.foreground = v; XChangeGC(_dpy, _gc, GCForeground, &_values); }
  void SetLineWidth(int v) { _values.line_width = v; XChangeGC(_dpy, _gc, GCLineWidth, &_values); }
  void SetLineStyle(int v) { _values.line_style = v; XChangeGC(_dpy, _gc, GCLineStyle, &_values); }
  void SetCapStyle(int v) { _values.cap_style = v; XChangeGC(_dpy, _gc, GCCapStyle, &_values); }
  void SetJoinStyle(int v) { _values.join_style = v; XChangeGC(_dpy, _gc, GCJoinStyle, &_values); }
  void SetFillStyle(int v) { _values.fill_style = v; XChangeGC(_dpy, _gc, GCFillStyle, &_values); }
  void SetFillRule(int v) { _values.fill_rule = v; XChangeGC(_dpy, _gc, GCFillRule, &_values); }
  void SetTile(Pixmap v) { _values.tile = v; XChangeGC(_dpy, _gc, GCTile, &_values); }
  void SetStipple(Pixmap v) { _values.stipple = v; XChangeGC(_dpy, _gc, GCStipple, &_values); }
  void SetTileStipXOrigin(int v) { _values.ts_x_origin = v; XChangeGC(_dpy, _gc, GCTileStipXOrigin, &_values); }
  void SetTileStipYOrigin(int v) { _values.ts_y_origin = v; XChangeGC(_dpy, _gc, GCTileStipYOrigin, &_values); }
  void SetFont(Font v) { _values.font = v; XChangeGC(_dpy, _gc, GCFont, &_values); }
  void SetSubwindowMode(int v) { _values.subwindow_mode = v; XChangeGC(_dpy, _gc, GCSubwindowMode, &_values); }
  void SetGraphicsExposures(Bool v) { _values.graphics_exposures = v; XChangeGC(_dpy, _gc, GCGraphicsExposures, &_values); }
  void SetClipXOrigin(int v) { _values.clip_x_origin = v; XChangeGC(_dpy, _gc, GCClipXOrigin, &_values); }
  void SetClipYOrigin(int v) { _values.clip_y_origin = v; XChangeGC(_dpy, _gc, GCClipYOrigin, &_values); }
  void SetClipMask(Pixmap v) { _values.clip_mask = v; XChangeGC(_dpy, _gc, GCClipMask, &_values); }
  void SetDashOffset(int v) { _values.dash_offset = v; XChangeGC(_dpy, _gc, GCDashOffset, &_values); }
  void SetDashList(char v) { _values.dashes = v; XChangeGC(_dpy, _gc, GCDashList, &_values); }
  void SetArcMode(int v) { _values.arc_mode = v; XChangeGC(_dpy, _gc, GCArcMode, &_values); }

  const XGCValues *GetAttributes() const { return &_values; }
  unsigned long GetValueMask() const { return _mask; }

  int GetFunction() const { return _values.function; }
  unsigned long GetForeground() const { return _values.foreground; }
  unsigned long GetBackground() const { return _values.background; }
  int GetLineWidth() const { return _values.line_width; }
  int GetLineStyle() const { return _values.line_style; }
  Font GetFont() const { return _values.font; }
  int GetCapStyle() const { return _values.cap_style; }
  int GetJoinStyle() const { return _values.join_style; }
  int GetFillStyle() const { return _values.fill_style; }
  int GetFillRule() const { return _values.fill_rule; }

  friend class OGCPool;

protected:
  unsigned long _mask;
  XGCValues _values;
  int _depth;
  GC _gc;
  Display *_dpy;
  Drawable _d;
};

class OGCPool : public OBaseObject {
public:
  OGCPool(const OXClient *client);
  ~OGCPool();

  const OXGC *GetGC(Drawable d, unsigned long mask, XGCValues *values,
                    int rw = False);
  void  FreeGC(const OXGC *gc);
  void  FreeGC(const GC gc);

  friend class OXClient;

private:
  int  MatchGC(const OXGC *gc, unsigned long mask, XGCValues *values);
  void UpdateGC(OXGC *gc, unsigned long mask, XGCValues *values);

protected:
  const OXClient *_client;
  typedef struct _SListGCElt {
    class OXGC *gc;
    int count, rwflag;
    struct _SListGCElt *next;
  } SListGCElt;
  SListGCElt *_gclist;
};


#endif  // __OGC_H
