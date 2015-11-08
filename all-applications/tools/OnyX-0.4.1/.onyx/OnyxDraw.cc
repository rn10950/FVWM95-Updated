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

#include <X11/X.h>
#include <X11/extensions/shape.h>

#include "OnyxDraw.h"

extern MessageCenter *CentralMessageCenter;


//----------------------------------------------------------------------

RedirectedDrawFrame::RedirectedDrawFrame(const OXWindow *p,
    int w, int h, int ID) : OXFrame(p, w, h) {
  InternalID = ID;

  XGCValues gcval;
  unsigned long gcmask;

  AddInput(StructureNotifyMask | ButtonPressMask | ButtonReleaseMask |
           PointerMotionMask | KeyPressMask | KeyReleaseMask);

  CurrentColor = _client->GetColor(_backPixel);

  DrawFont = _client->GetFont("Helvetica -12");
  DrawFont->GetFontMetrics(&FontMetrics);

  gcmask = GCForeground | GCBackground | GCFont | GCGraphicsExposures;
  gcval.foreground = CurrentColor;
  gcval.background = CurrentColor;
  gcval.font = DrawFont->GetId();
  gcval.graphics_exposures = True;

  DrawGC = new OXGC(GetDisplay(), _id, gcmask, &gcval);

  Buffer = None;
}

int RedirectedDrawFrame::HandleButton(XButtonEvent *event) {
  OnyxMessage omsg;

  OXFrame::HandleButton(event);

  omsg.onyx_type = ContainerType;
  omsg.id = InternalID;

  omsg.button = event->button;
  omsg.x = event->x;
  omsg.y = event->y;
  omsg.x_root = event->x_root;
  omsg.y_root = event->y_root;

  if (event->type == ButtonPress) {
    omsg.action = ButtonPressed;
    CentralMessageCenter->ProcessMessage(&omsg);
  } else {
    omsg.action = ButtonReleased;
    CentralMessageCenter->ProcessMessage(&omsg);
  }

  return True;
}

int RedirectedDrawFrame::HandleMotion(XMotionEvent *event) {
  OnyxMessage omsg;

  OXFrame::HandleMotion(event);

  omsg.onyx_type = ContainerType;
  omsg.id = InternalID;
  omsg.action = PointerMoved;
  omsg.x = event->x;
  omsg.y = event->y;
  omsg.x_root = event->x_root;
  omsg.y_root = event->y_root;

  return CentralMessageCenter->ProcessMessage(&omsg);
}

int RedirectedDrawFrame::HandleKey(XKeyEvent *event) {
  OnyxMessage omsg;

  OXFrame::HandleKey(event);

  omsg.onyx_type = ContainerType;
  omsg.id = InternalID;

  omsg.keysym = XKeycodeToKeysym(GetDisplay(), event->keycode, 0);

  if (event->type == KeyPress) {
    omsg.action = KeyPressed;
    CentralMessageCenter->ProcessMessage(&omsg);
  } else {
    omsg.action = KeyReleased;
    CentralMessageCenter->ProcessMessage(&omsg);
  }

  return True;
}

void RedirectedDrawFrame::DoubleBuffer(int onoff) {
  if (onoff) {
    if (Buffer == None) CreatePixmap();
  } else {
    if (Buffer != None) XFreePixmap(GetDisplay(), Buffer);
    Buffer = None;
    NeedRedraw();
  }
}

void RedirectedDrawFrame::_Resized() {
  if (Buffer != None) {
    XFreePixmap(GetDisplay(), Buffer);
    CreatePixmap();
  }
}

void RedirectedDrawFrame::CreatePixmap() {
  Buffer = XCreatePixmap(GetDisplay(), _id, _w, _h,
                         _client->GetDisplayDepth());
  unsigned long old_fg = DrawGC->GetForeground();
  DrawGC->SetForeground(_backPixel);
  XFillRectangle(GetDisplay(), Buffer, DrawGC->GetGC(), 0, 0, _w, _h);
  DrawGC->SetForeground(old_fg);

  Redraw();
}

void RedirectedDrawFrame::Redraw() {
  OnyxMessage omsg;

  omsg.onyx_type = ContainerType;
  omsg.id = InternalID;
  omsg.action = Paint;

  CentralMessageCenter->ProcessMessage(&omsg);

  if (Buffer != None) NeedRedraw();
}

void RedirectedDrawFrame::_DoRedraw() {
  if (Buffer != None) {
    OXFrame::DrawBorder();
    OXFrame::CopyArea(Buffer, _id, DrawGC->GetGC(), _insets.l, _insets.t,
                      _w - _insets.l - _insets.r, _h - _insets.t - _insets.b,
                      _insets.l, _insets.t);
  } else {
#if 0
    OXFrame::DrawBorder();
#else
    OXFrame::_DoRedraw();
#endif

    Redraw();
  }
}

void RedirectedDrawFrame::SetColor(const char *color) {
  _client->FreeColor(CurrentColor);
  CurrentColor = _client->GetColorByName(color);
  DrawGC->SetForeground(CurrentColor);
  DrawGC->SetBackground(CurrentColor);
}

void RedirectedDrawFrame::SetColor(int r, int g, int b) {
  char tmp[20];

  sprintf(tmp, "#%02x%02x%02x", r, g, b);
  SetColor(tmp);
}

void RedirectedDrawFrame::SetFont(const char *font) {
  _client->FreeFont((OXFont *) DrawFont);
  DrawFont = _client->GetFont(font);
  DrawFont->GetFontMetrics(&FontMetrics);
  DrawGC->SetFont(DrawFont->GetId());
}

void RedirectedDrawFrame::ClearArea(int x, int y, int w, int h) {
  if (Buffer != None) {
    unsigned long old_fg = DrawGC->GetForeground();
    DrawGC->SetForeground(_backPixel);
    XFillRectangle(GetDisplay(), Buffer, DrawGC->GetGC(), x, y, w, h);
    DrawGC->SetForeground(old_fg);
    NeedRedraw();
  } else {
    OXFrame::ClearArea(x, y, w, h);
  }
  
}

void RedirectedDrawFrame::DrawPoint(int x, int y) {
  if (Buffer != None) {
    XDrawPoint(GetDisplay(), Buffer, DrawGC->GetGC(), x, y);
    NeedRedraw();
  } else {
    OXFrame::DrawPoint(DrawGC->GetGC(), x, y);
  }
}

void RedirectedDrawFrame::DrawLine(int x1, int y1, int x2, int y2) {
  if (Buffer != None) {
    XDrawLine(GetDisplay(), Buffer, DrawGC->GetGC(), x1, y1, x2, y2);
    NeedRedraw();
  } else {
    OXFrame::DrawLine(DrawGC->GetGC(), x1, y1, x2, y2);
  }
}

void RedirectedDrawFrame::DrawRectangle(int x, int y, int width, int height) {
  if (Buffer != None) {
    XDrawRectangle(GetDisplay(), Buffer, DrawGC->GetGC(), x, y, width, height);
    NeedRedraw();
  } else {
    OXFrame::DrawRectangle(DrawGC->GetGC(), x, y, width, height);
  }
}

void RedirectedDrawFrame::FillRectangle(int x, int y, int width, int height) {
  if (Buffer != None) {
    XFillRectangle(GetDisplay(), Buffer, DrawGC->GetGC(), x, y, width, height);
    NeedRedraw();
  } else {
    OXFrame::FillRectangle(DrawGC->GetGC(), x, y, width, height);
  }
}

void RedirectedDrawFrame::FillPolygon(int *points, int npoints, int shape, int mode) {
  XPoint *p;
  int i;

  if (npoints >= 2) {
    p = new XPoint[npoints];
    for (i = 0; i < npoints; ++i) {
      p[i].x = points[2 * i];
      p[i].y = points[2 * i + 1];
    }
    if (Buffer != None) {
      XFillPolygon(GetDisplay(), Buffer, DrawGC->GetGC(), p, npoints, shape, mode);
      NeedRedraw();
    } else {
      OXFrame::FillPolygon(DrawGC->GetGC(), p, npoints, shape, mode);
    }
    delete[] p;
  }
}

void RedirectedDrawFrame::DrawArc(int x, int y, int w, int h, int angle1, int angle2) {
  if (Buffer != None) {
    XDrawArc(GetDisplay(), Buffer, DrawGC->GetGC(), x, y, w, h, angle1, angle2);
    NeedRedraw();
  } else {
    OXFrame::DrawArc(DrawGC->GetGC(), x, y, w, h, angle1, angle2);
  }
}

void RedirectedDrawFrame::FillArc(int x, int y, int w, int h, int angle1, int angle2) {
  if (Buffer != None) {
    XFillArc(GetDisplay(), Buffer, DrawGC->GetGC(), x, y, w, h, angle1, angle2);
    NeedRedraw();
  } else {
    OXFrame::FillArc(DrawGC->GetGC(), x, y, w, h, angle1, angle2);
  }
}

void RedirectedDrawFrame::DrawString(int x, int y, const char *string) {
  if (Buffer != None) {
    XDrawString(GetDisplay(), Buffer, DrawGC->GetGC(), x, y, string, strlen(string));
    NeedRedraw();
  } else {
    OXFrame::DrawString(DrawGC->GetGC(), x, y, string, strlen(string));
  }
}

void RedirectedDrawFrame::CopyArea(int srcx, int srcy, int w, int h,
                                   int dstx, int dsty) {
  if (Buffer != None) {
    XCopyArea(GetDisplay(), Buffer, Buffer, DrawGC->GetGC(),
              srcx, srcy, w, h, dstx, dsty);
    NeedRedraw();
  } else {
    OXFrame::CopyWindowArea(DrawGC->GetGC(), srcx, srcy, w, h, dstx, dsty);
  }
}

int RedirectedDrawFrame::StringWidth(const char *str) {
  return DrawFont->XTextWidth(str);
}

int RedirectedDrawFrame::StringHeight(const char *str) {
  return DrawFont->TextHeight();
}

int RedirectedDrawFrame::StringAscent(const char *str) {
  return FontMetrics.ascent;
}

RedirectedDrawFrame::~RedirectedDrawFrame() {
  delete DrawGC;
  _client->FreeColor(CurrentColor);
  _client->FreeFont((OXFont *) DrawFont);
  if (Buffer != None) XFreePixmap(GetDisplay(), Buffer);
}

//----------------------------------------------------------------------

OnyxDraw::OnyxDraw() : OnyxObject("OnyxDraw") {
  DrawFrameExists = 0;
  Width = 32;
  Height = 32;
  X = 0;
  Y = 15;
  DoubleBuffer = 0;
}

int OnyxDraw::Create() {
  OnyxObject::Create();
  if (!DrawFrameExists) {
    InternalDrawFrame = new RedirectedDrawFrame(Parent, Width, Height, ID);
    InternalDrawFrame->SetBorderStyle(BorderStyle);
    InternalDrawFrame->DoubleBuffer(DoubleBuffer);
    InternalDrawFrame->Move(X, Y);
    DrawFrameExists++;
  }
  return DrawFrameExists;
}

void OnyxDraw::Move(int newX, int newY) {
  OnyxObject::Move(newX, newY);
  InternalDrawFrame->Move(newX, newY);
}

void OnyxDraw::Resize(int newWidth, int newHeight) {
  OnyxObject::Resize(newWidth, newHeight);
  InternalDrawFrame->Resize(newWidth, newHeight);
}

void OnyxDraw::MoveResize(int newX, int newY, int newWidth, int newHeight) {
  OnyxObject::MoveResize(newX, newY, newWidth, newHeight);
  InternalDrawFrame->MoveResize(newX, newY, newWidth, newHeight);
}

void OnyxDraw::Update() {
  OnyxObject::Update();
  InternalDrawFrame->SetBorderStyle(BorderStyle);
  InternalDrawFrame->DoubleBuffer(DoubleBuffer);
  InternalDrawFrame->Redraw();
}

void OnyxDraw::SetColor(int r, int g, int b) {
  InternalDrawFrame->SetColor(r, g, b);
}

void OnyxDraw::SetColor(const char *color) {
  InternalDrawFrame->SetColor(color);
}

void OnyxDraw::SetFont(const char *font) {
  InternalDrawFrame->SetFont(font);
}

void OnyxDraw::ClearArea(int x, int y, int width, int height) {
  InternalDrawFrame->ClearArea(x, y, width, height);
}

void OnyxDraw::DrawPoint(int x, int y) {
  InternalDrawFrame->DrawPoint(x, y);
}

void OnyxDraw::DrawLine(int x1, int y1, int x2, int y2) {
  InternalDrawFrame->DrawLine(x1, y1, x2, y2);
}

void OnyxDraw::DrawRectangle(int x, int y, int width, int height) {
  InternalDrawFrame->DrawRectangle(x, y, width, height);
}

void OnyxDraw::FillRectangle(int x, int y, int width, int height) {
  InternalDrawFrame->FillRectangle(x, y, width, height);
}

void OnyxDraw::FillPolygon(int *points, int npoints, int shape, int mode) {
  InternalDrawFrame->FillPolygon(points, npoints, shape, mode);
}

void OnyxDraw::DrawArc(int x, int y, int w, int h, int angle1, int angle2) {
  InternalDrawFrame->DrawArc(x, y, w, h, angle1, angle2);
}

void OnyxDraw::FillArc(int x, int y, int w, int h, int angle1, int angle2) {
  InternalDrawFrame->FillArc(x, y, w, h, angle1, angle2);
}

void OnyxDraw::DrawString(int x, int y, const char *string) {
  InternalDrawFrame->DrawString(x, y, string);
}

void OnyxDraw::CopyArea(int srcx, int srcy, int w, int h, int dstx, int dsty) {
  InternalDrawFrame->CopyArea(srcx, srcy, w, h, dstx, dsty);
}

int OnyxDraw::StringWidth(const char *str) {
  return InternalDrawFrame->StringWidth(str);
}

int OnyxDraw::StringHeight(const char *str) {
  return InternalDrawFrame->StringHeight(str);
}

int OnyxDraw::StringAscent(const char *str) {
  return InternalDrawFrame->StringAscent(str);
}

OnyxDraw::~OnyxDraw() {
  InternalDrawFrame->DestroyWindow();
  delete InternalDrawFrame;
}
