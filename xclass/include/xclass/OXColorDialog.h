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

#ifndef __OXCOLORDIALOG_H
#define __OXCOLORDIALOG_H

#include <xclass/OXTransientFrame.h>
#include <xclass/OXWidget.h>
#include <xclass/OXTextButton.h>
#include <xclass/OXTextEntry.h>
#include <xclass/ORectangle.h>
#include <xclass/OColor.h>

class OXGC;


//----------------------------------------------------------------------

class OXColorPalette : public OXFrame, public OXWidget {
public:
  OXColorPalette(const OXWindow *p, int cols, int rows, int id = -1);
  virtual ~OXColorPalette();

  virtual int HandleButton(XButtonEvent *event);
  virtual int HandleMotion(XMotionEvent *event);
  virtual int HandleKey(XKeyEvent *event);
  
  virtual ODimension GetDefaultSize() const
       { return ODimension((_cw + 5) * _cols, (_ch + 5) * _rows); }

  void SetColors(OColor colors[]);
  void SetColor(int ix, OColor color);
  void SetCurrentCellColor(OColor color);
  
  void SetCellSize(int w = 20, int h = 17);
  
  OColor GetColorByIndex(int ix) const { return _colors[ix]; }
  OColor GetCurrentColor() const;

protected:
  virtual void _DoRedraw();
  virtual void _GotFocus();
  virtual void _LostFocus();
  
  void _DrawFocusHilite(int onoff);

  int _cx, _cy;             // coordinates of current selected color cell
  int _cw, _ch;             // color cell width and height
  int _rows, _cols;
  OColor *_colors;
  unsigned long *_pixels;
  
  OXGC *_drawGC;
};

class OXColorPick : public OXFrame, public OXWidget {
public:
  OXColorPick(const OXWindow *p, int w, int h, int id = -1);
  virtual ~OXColorPick();

  virtual int HandleButton(XButtonEvent *event);
  virtual int HandleMotion(XMotionEvent *event);
  
  void SetColor(OColor color);

  OColor GetCurrentColor() const { return _currentColor; }

protected:
  virtual void _DoRedraw();
  
  void _DrawHScursor(int onoff);
  void _DrawLcursor(int onoff);
  void _SetHScursor(int x, int y);
  void _SetLcursor(int z);

  void _CreateImages();
  void _InitImages();
  void _SetSliderColor();
  void _UpdateCurrentColor();
  
  void _AllocColors();
  void _FreeColors();
  void _CreateDitheredImage(XImage *image, int which);

  XImage *_HSimage, *_Limage;
  int _ncolors, _click, _cx, _cy, _cz;
  OColor _currentColor;
  ORectangle _colormapRect, _sliderRect;
  
private:
  int colormap[64][3];
  unsigned long pixel[64];
};

//----------------------------------------------------------------------

class OXColorDialog : public OXTransientFrame {
public:
  OXColorDialog(const OXWindow *p, const OXWindow *m, int *retc = NULL,
                OColor *color = NULL);
  virtual ~OXColorDialog();
  
  virtual int ProcessMessage(OMessage *msg);

protected:
  void LoadCustomColors();
  void SaveCustomColors();
  void UpdateRGBentries(OColor *c);
  void UpdateHLSentries(OColor *c);

  unsigned long _currentColor;
  
  OXColorPalette *_palette, *_cpalette;
  OXColorPick *_colors;
  OXFrame *_sample;
  OXTextEntry *_rte, *_gte, *_bte, *_hte, *_lte, *_ste;
  
  int *_retc;
  OColor *_retColor;
};


#endif  // __OXCOLORDIALOG_H
