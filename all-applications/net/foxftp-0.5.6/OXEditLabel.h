/**************************************************************************

    This file is part of foxftp, a ftp client for fOX.
    Copyright (C) 1998, 1999, G. Matzka.

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

#ifndef _OXEDITLABEL_H
#define _OXEDITLABEL_H

class ODimension;
class OString;

#include <X11/Xlib.h>

#include <xclass/OXFrame.h>
#include <xclass/OTextBuffer.h>

//-----------------------------------------------------------------

class OXEditLabel : public OXFrame {
protected:
  static Cursor _defaultCursor;
  static GC _defaultGC;
  static XFontStruct *_defaultFontStruct;
  GC _normGC;
  XFontStruct *_fontStruct;

public:
  OXEditLabel(const OXWindow *p, OString *text, int ID,
          GC norm = _defaultGC,
          XFontStruct *font = _defaultFontStruct,
          unsigned int options = CHILD_FRAME,
          unsigned long back = _defaultFrameBackground);
  virtual ~OXEditLabel();

  virtual int HandleButton(XButtonEvent *event);
  virtual int HandleDoubleClick(XButtonEvent *event);
  virtual int HandleMotion(XMotionEvent *event);
  virtual int HandleKey(XKeyEvent *event);

  virtual ODimension GetDefaultSize() const 
    {  return ODimension(_tw+_ml+_mr, _th+_mt+_mb+1); }
  const char *GetText() const { return _text->GetString(); }
  void SetText(char *new_text);
  void SetTextAlignment(const int tmode) { _alignment = tmode; } 
  void SetMargins(int left=0, int right=0, int top=0, int bottom=0)
    { _ml = left; _mr = right; _mt = top; _mb = bottom; }
  void SetGC(GC g){_normGC = g; _FontSizeUp();}
  void SetFont(XFontStruct *font){_fontStruct = font;_FontSizeUp();}

  void Edit(bool select);
  void UnEdit();

protected:
  void _FontSizeUp();
  void SetCursor(int newpos);
  virtual void _DoRedraw();

//  OString *_text;
  OTextBuffer *_text;
  int _ID, _alignment, _tw, _th, _ml, _mr, _mt, _mb;
  int _width_min;
  int _sel_start, _sel_end, _cursor_pos;
  bool _edit, _selection_on, _dragging;
};


#endif
