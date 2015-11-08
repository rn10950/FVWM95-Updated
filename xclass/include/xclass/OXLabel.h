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

#ifndef __OXLABEL_H
#define __OXLABEL_H

#include <X11/Xlib.h>

#include <xclass/utils.h>
#include <xclass/OXFrame.h>
#include <xclass/ODimension.h>
#include <xclass/OString.h>

class OXGC;
class OTextLayout;
class OXFont;


#define LABEL_NORMAL  0
#define LABEL_RAISED  1
#define LABEL_SUNKEN  2
#define LABEL_SHADOW  3


//----------------------------------------------------------------------

class OXLabel : public OXFrame {
protected:
  static const OXFont *_defaultFont;
  static const OXGC *_defaultGC;
  static int _init;

public:
  OXLabel(const OXWindow *p, OString *text, OXFont *font = NULL,
          unsigned int options = CHILD_FRAME,
          unsigned long back = _defaultFrameBackground);
  OXLabel(const OXWindow *p, char *text, OXFont *font = NULL,
          unsigned int options = CHILD_FRAME,
          unsigned long back = _defaultFrameBackground);
  virtual ~OXLabel();

  virtual ODimension GetDefaultSize() const 
    { return ODimension(_tw+_ml+_mr, _th+_mt+_mb+1); }

  void SetText(OString *new_text);
  void SetFont(const OXFont *font);
  void SetTextAlignment(int tmode) { _align = tmode; } 
  void SetTextJustify(int j) { _justify = j; Layout(); }
  void SetTextFlags(int tflags) { _textFlags = tflags; Layout(); }
  void SetMargins(int left=0, int right=0, int top=0, int bottom=0)
    { _ml = left; _mr = right; _mt = top; _mb = bottom; }
  void SetWrapLength(int wl) { _wrapLength = wl; Layout(); }
  void SetTextColor(unsigned int color);
  void Set3DStyle(int style);

  const OString *GetText() const { return _text; }
  int GetTextAlignment() const { return _align; }
  int GetTextJustify() const { return _justify; }
  int GetTextFlags() const { return _textFlags; }
  int GetWrapLength() const { return _wrapLength; }
  int Get3DStyle() const { return _3dStyle; }
  
  int GetLeftMargin() const { return _ml; }
  int GetRightMargin() const { return _mr; }
  int GetTopMargin() const { return _mt; }
  int GetBottomMargin() const { return _mb; }

  virtual void Layout();

protected:
  virtual void _DoRedraw();
  void _DrawText(int x, int y);

  OString *_text;
  int _align, _3dStyle, _textFlags, _justify, _wrapLength;
  int _tw, _th, _ml, _mr, _mt, _mb;
  OXGC *_normGC;
  const OXFont *_font;
  OTextLayout *_tl;
};


#endif  // __OXLABEL_H
