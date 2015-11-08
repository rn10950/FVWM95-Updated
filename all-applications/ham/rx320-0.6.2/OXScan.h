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

#ifndef __OXSCAN_H
#define __OXSCAN_H

#include <xclass/utils.h>
#include <xclass/OXTransientFrame.h>
#include <xclass/ODimension.h>
#include <xclass/OXFont.h>
#include <xclass/OXTextButton.h>
#include <xclass/OGC.h>
#include <xclass/OTimer.h>

#include "ORX320.h"
#include "main.h"


#define SCOPE_LIN   0
#define SCOPE_LOG   1

#define SCOPE_LINE  0
#define SCOPE_BARS  1

#define MSG_SCOPE   (MSG_USERMSG+2320)


//----------------------------------------------------------------------

class OXBandScope : public OXFrame {
public:
  OXBandScope(const OXWindow *p, int scanwidth,
              unsigned int options = CHILD_FRAME);
  virtual ~OXBandScope();
  
  virtual int HandleButton(XButtonEvent *event);
  virtual int HandleMotion(XMotionEvent *event);
  virtual int HandleCrossing(XCrossingEvent *event);

  void SetMode(int mode);
  void SetStyle(int style);

  void Clear();
  void SetScanPoints(int np);
  void SetFilterWidth(int fw);
  void SetValue(int x, int y);
  void SetRange(long fmin, long fstep);
  void SetTunedFreq(long freq);
  
  virtual ODimension GetDefaultSize() const;
  
protected:
  virtual void _DoRedraw();

  void Draw();
  void DoDraw();
  void ClearPixmap();
  void DrawGrid();
  void DrawData();
  void DrawArea();
  void DrawCursor();
  void DrawStatus();
  
  OXGC *_gc, *_lgc, *_ngc;
  Pixmap _pix;
  int _pixw, _pixh;
  
  int _dataLen, *_data, _maxY, _curX, _fWidth;
  int _yMode, _style, _bdown, _draw_pending;
  OInsets _margins;
  OXFont *_sfont, *_nfont;
  long _curfreq, _sfreq, _freqstep;
};


class OXScanWindow : public OXTransientFrame {
public:
  OXScanWindow(const OXWindow *p, OXMain *m, SScanSettings *settings);
  virtual ~OXScanWindow();

  virtual int HandleTimer(OTimer *t);

  virtual int ProcessMessage(OMessage *msg);
  virtual int CloseWindow();

  void UpdateDisplay();

  void InitScan();
  void AddPoint(int y);
  long GetStartFreq();
  long GetNextFreq();

  void SetRange(long range);

protected:
  OXMain *_rxmain;
  OXBandScope *_plot;
  OXTextButton *_start, *_stop, *_slin, *_slog;
  SScanSettings *_settings;
  
  long _sfreq, _freq, _freqstep;
  int  _curX;
};


#endif  // __OXSCAN_H
