/**************************************************************************

    This file is part of OnyX, a visual development environment
    using the xclass toolkit. Copyright (C) 1997, 1998 Frank Hall.

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

#ifndef __ONYXDRAW_H
#define __ONYXDRAW_H

#include <xclass/OXWindow.h>
#include <xclass/OXFrame.h>
#include <xclass/OXFont.h>
#include <xclass/OGC.h>

#include "OnyxBase.h"


//----------------------------------------------------------------------

class RedirectedDrawFrame : public OXFrame {
public:
  RedirectedDrawFrame(const OXWindow *p, int w, int h, int ID);
  virtual ~RedirectedDrawFrame();

  virtual int HandleButton(XButtonEvent *event);
  virtual int HandleMotion(XMotionEvent *event);
  virtual int HandleKey(XKeyEvent *event);

  virtual void SetColor(int r, int g, int b);
  virtual void SetColor(const char *color);
  virtual void SetFont(const char *font);
  virtual void ClearArea(int x, int y, int width, int height);
  virtual void DrawPoint(int x, int y);
  virtual void DrawLine(int x1, int y1, int x2, int y2);
  virtual void DrawRectangle(int x, int y, int width, int height);
  virtual void FillRectangle(int x, int y, int width, int height);
  virtual void FillPolygon(int *points, int npoints, int shape, int mode);
  virtual void DrawArc(int x, int y, int w, int h, int angle1, int angle2);
  virtual void FillArc(int x, int y, int w, int h, int angle1, int angle2);
  virtual void DrawString(int x, int y, const char *string);
  virtual void CopyArea(int srcx, int srcy, int w, int h, int dstx, int dsty);
  virtual int  StringWidth(const char *str);
  virtual int  StringHeight(const char *str);
  virtual int  StringAscent(const char *str);
  
  virtual void DoubleBuffer(int onoff);
  virtual void Redraw();

private:
  virtual void _Resized();
  virtual void _DoRedraw();
  
  virtual void CreatePixmap();

  int InternalID;
  OXGC *DrawGC;
  const OXFont *DrawFont;
  OFontMetrics FontMetrics;
  unsigned long CurrentColor;
  
  Pixmap Buffer;
};


//----------------------------------------------------------------------

class OnyxDraw : public OnyxObject {
public:
  OnyxDraw();
  virtual ~OnyxDraw();

  virtual int  Create();
  virtual void Move(int newX, int newY);
  virtual void Resize(int newWidth, int newHeight);
  virtual void MoveResize(int newX, int newY, int newWidth, int newHeight);
  virtual void Update();

  virtual void SetColor(int r, int g, int b);
  virtual void SetColor(const char *color);
  virtual void SetFont(const char *font);
  virtual void ClearArea(int x, int y, int width, int height);
  virtual void DrawPoint(int x, int y);
  virtual void DrawLine(int x1, int y1, int x2, int y2);
  virtual void DrawRectangle(int x, int y, int width, int height);
  virtual void FillRectangle(int x, int y, int width, int height);
  virtual void FillPolygon(int *points, int npoints, int shape, int mode);
  virtual void DrawArc(int x, int y, int w, int h, int angle1, int angle2);
  virtual void FillArc(int x, int y, int w, int h, int angle1, int angle2);
  virtual void DrawString(int x, int y, const char *string);
  virtual void CopyArea(int srcx, int srcy, int w, int h, int dstx, int dsty);
  virtual int  StringWidth(const char *str);
  virtual int  StringHeight(const char *str);
  virtual int  StringAscent(const char *str);

  int DoubleBuffer;

protected:
  RedirectedDrawFrame *InternalDrawFrame;
  int DrawFrameExists;
};


#endif  // __ONYXDRAW_H
