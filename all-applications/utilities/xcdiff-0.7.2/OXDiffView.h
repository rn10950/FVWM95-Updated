/**************************************************************************
 
    This file is part of xcdiff, a front-end to the diff command.              
    Copyright (C) 1998-2002 Matzka Gerald, Hector Peraza.            
 
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

#ifndef __OXDIFFVIEW_H
#define __OXDIFFVIEW_H


#include <xclass/OXCompositeFrame.h>
#include <xclass/OXLabel.h>
#include <xclass/OXScrollBar.h>
#include <xclass/OXFont.h>


#define DISPLAY_NORMAL	0x01
#define DISPLAY_CHANGE	0x02
#define DISPLAY_INSERTL	0x04
#define DISPLAY_INSERTR 0x08
#define DISPLAY_DELETE	0x10

#define MAXLINESIZE 300
#define MAXLINES    5000
#define MARGIN      5

class ODiffColors;


//----------------------------------------------------------------------

class OXDiffFrame : public OXFrame {
protected:
  OXFont *_font;
  ODiffColors *_colors;
  unsigned long _normal_fg,  _normal_bg,
                _changed_fg, _changed_bg,
                _added_fg,   _added_bg,
                _deleted_fg, _deleted_bg;
  GC _gc, _gc_change, _gc_ladd, _gc_radd;
  int _numbering;
  int _marked_start, _marked_end;

public:
  OXDiffFrame(OXWindow *parent, int w, int h, unsigned int options,
              unsigned long back = _defaultFrameBackground);
  virtual ~OXDiffFrame();

  int  LoadFile(char *fname);
  int  InsLine(int, char*, int);
  int  AppendLine(char*, int);
  int  SetLineType(int, int);
  void Clear();
  int  GetLines() const { return nlines; }
  int  GetVisibleLines() const { return (_h / _th); }
  void SetTopLine(int new_top);
  int  GetTopLine() const { return top; }
  void SetLineNumOn(int on);
  void CalcMarkRegion(int start, int end, int &realstart, int &realend);
  void MarkRegion(int start, int end);
  void PrintText();
  void CenterDiff();

  virtual int HandleExpose(XExposeEvent *event) {
    DrawRegion(event->x, event->y, event->width, event->height);
    return True;
  }
  virtual int HandleGraphicsExpose(XGraphicsExposeEvent *event) {
    DrawRegion(event->x, event->y, event->width, event->height);
    return True;
  }
  
  OXFont *GetFont() const { return _font; }
  void SetFont(OXFont *f);
  
  ODiffColors *GetColors() const { return _colors; }
  void SetColors(ODiffColors *colors);
  
protected:
  void DrawRegion(int x, int y, int w, int h);
  void ScrollWindow(int new_top);
  
  char *chars[MAXLINES];   // lines of text
  int lnnum[MAXLINES];
  u_char lntype[MAXLINES];     // length of each line
  int _th, nlines, top;
};


//----------------------------------------------------------------------

class OXDiffView : public OXCompositeFrame {
public:
  OXDiffView(OXWindow *parent, int w, int h, unsigned int options,
             unsigned int back = _defaultFrameBackground);
  virtual ~OXDiffView();

  int  LoadFile(char *fname);
  void Clear();

  void CenterDiff();
  void AdjustScrollBar();

  virtual void SetLineNumOn(int on) { _textCanvas->SetLineNumOn(on); }
  virtual int  ProcessMessage(OMessage *msg);
  virtual void Layout();

  virtual ODimension GetDefaultSize() const { return ODimension(_w, _h); }

  OXDiffFrame *GetTextFrame() const { return _textCanvas; }

protected:
  OXDiffFrame  *_textCanvas;
  OXVScrollBar *_vsb;
};


#endif  // __OXDIFFVIEW_H
