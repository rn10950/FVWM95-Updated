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

#ifndef __OXPROGRESSBAR_H
#define __OXPROGRESSBAR_H

#include <xclass/OXFrame.h>
#include <xclass/OXWidget.h>

#define PROGBAR_MODE1	1
#define PROGBAR_MODE2	2

class OXFont;


//----------------------------------------------------------------------

class OXProgressBar : public OXFrame, public OXWidget {
public:
  OXProgressBar(const OXWindow *p, int w, int h,
                int mode = PROGBAR_MODE1, int id = -1,
                unsigned int options = SUNKEN_FRAME,
              unsigned long back = _defaultFrameBackground);
  virtual ~OXProgressBar();

  int  GetPosition() { return _pos; }
  void SetRange(int min, int max) { _min = min; _max = max; _Redraw(); }
  void SetPosition(int pos) { if (_pos != pos) { _pos = pos; _Redraw(); } }
  void ShowPercentage(bool show) { _show = show; NeedRedraw(); }
  void SetColor(unsigned int color);
  void SetFloatingText(bool fltxt) { _fltxt = fltxt; NeedRedraw(); }

protected:
  virtual void _DoRedraw();

  void _Redraw();

  int _laststop, _pos, _min, _max, _mode;
  bool _show, _fltxt;
  GC _gc_marked, _pro_gc, _white_gc, _black_gc;
  const OXFont *_font;
};


#endif  // __OXPROGRESSBAR_H
