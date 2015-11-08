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

#ifndef __OXWINDOW_H
#define __OXWINDOW_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <xclass/utils.h>
#include <xclass/OXObject.h>
#include <xclass/OXClient.h>
#include <xclass/OString.h>


//----------------------------------------------------------------------
// X window base class

class OXWindow : public OXObject {
public:
  OXWindow() {}
  OXWindow(const OXWindow *p, int x, int y,
	   int w, int h, int border = 0,
	   int depth = CopyFromParent,
	   unsigned int clss = CopyFromParent,
	   Visual *visual = CopyFromParent,
	   unsigned long vmask = 0L,
	   XSetWindowAttributes *attr = NULL);
  OXWindow(OXClient *c, Window id, OXWindow *parent = NULL);

  virtual ~OXWindow();

  const class OXWindow *GetParent() const { return _parent; }
  const class OXWindow *GetTopLevel() const { return _toplevel; }

  virtual int HandleExpose(XExposeEvent *event) {
    if (event->count == 0) _client->NeedRedraw(this);
    return True;
  }
  virtual int HandleGraphicsExpose(XGraphicsExposeEvent *event) {
    if (event->count == 0) _client->NeedRedraw(this);
    return True;
  }
  virtual int HandleEvent(XEvent * /*event*/) { return False; }
  virtual int HandleMaskEvent(XEvent * /*event*/) { return False; }

  virtual void Move(int x, int y);
  virtual void Resize(int w, int h);
  virtual void MoveResize(int x, int y, int w, int h);
  virtual int  IsMapped();

  virtual void Reconfig() {}

  void NeedRedraw(int clr = True)
//       { _clearBgnd = clr; _client->NeedRedraw(this); }
       { _clearBgnd = clr || (_clearBgnd && _needRedraw);
         _client->NeedRedraw(this); }

  virtual void MapWindow()         { XMapWindow(GetDisplay(), _id); }
  virtual void MapSubwindows()     { XMapSubwindows(GetDisplay(), _id); }
  virtual void MapRaised()         { XMapRaised(GetDisplay(), _id); }
  virtual void UnmapWindow()       { XUnmapWindow(GetDisplay(), _id); }
  virtual void DestroyWindow()     { XDestroyWindow(GetDisplay(), _id); }
  virtual void DestroySubwindows() { XDestroySubwindows(GetDisplay(), _id); }
  virtual void RaiseWindow()       { XRaiseWindow(GetDisplay(), _id); }
  virtual void LowerWindow()       { XLowerWindow(GetDisplay(), _id); }

  virtual void ReparentWindow(const class OXWindow *p, int x = 0, int y = 0);

  virtual void SetBackgroundColor(unsigned long color)
       { XSetWindowBackground(GetDisplay(), _id, color); }
  virtual void SetBackgroundPixmap(Pixmap pixmap)
       { XSetWindowBackgroundPixmap(GetDisplay(), _id, pixmap); }

  void SelectInput(unsigned long emask)
       { XSelectInput(GetDisplay(), _id, emask); }

  void RequestFocus()
       { XSetInputFocus(GetDisplay(), _id, RevertToParent, CurrentTime); }

  // Drawing methods. Use these inlined functions as much as possible
  // instead of making direct Xlib calls.

  void ClearWindow()
       { XClearWindow(GetDisplay(), _id); }
  void ClearArea(int x, int y, int w, int h, int exposures = False)
       { XClearArea(GetDisplay(), _id, x, y, w, h, exposures); }

  void DrawPoint(GC gc, int x, int y) 
       { XDrawPoint(GetDisplay(), _id, gc, x, y); }
  void DrawPoints(GC gc, XPoint *points, int npoints, int mode)
       { XDrawPoints(GetDisplay(), _id, gc, points, npoints, mode); }

  void DrawLine(GC gc, int x1, int y1, int x2, int y2)
       { XDrawLine(GetDisplay(), _id, gc, x1, y1, x2, y2); }
  void DrawLines(GC gc, XPoint *points, int npoints, int mode)
       { XDrawLines(GetDisplay(), _id, gc, points, npoints, mode); }
  void DrawSegments(GC gc, XSegment *segments, int nsegments)
       { XDrawSegments(GetDisplay(), _id, gc, segments, nsegments); }

  void DrawRectangle(GC gc, int x, int y, int width, int height)
       { XDrawRectangle(GetDisplay(), _id, gc, x, y, width, height); }
  void DrawRectangles(GC gc, XRectangle rectangles[], int nrectangles)
       { XDrawRectangles(GetDisplay(), _id, gc, rectangles, nrectangles); }

  void FillRectangle(GC gc, int x, int y, int width, int height)
       { XFillRectangle(GetDisplay(), _id, gc, x, y, width, height); }
  void FillRectangles(GC gc, XRectangle rectangles[], int nrectangles)
       { XFillRectangles(GetDisplay(), _id, gc, rectangles, nrectangles); }

  void FillPolygon(GC gc, XPoint *points, int npoints, int shape, int mode)
       { XFillPolygon(GetDisplay(), _id, gc, points, npoints, shape, mode); }

  void DrawArc(GC gc, int x, int y, int w, int h, int angle1, int angle2)
       { XDrawArc(GetDisplay(), _id, gc, x, y, w, h, angle1, angle2); }
  void DrawArcs(GC gc, XArc *arcs, int narcs)
       { XDrawArcs(GetDisplay(), _id, gc, arcs, narcs); }

  void FillArc(GC gc, int x, int y, int w, int h, int angle1, int angle2)
       { XFillArc(GetDisplay(), _id, gc, x, y, w, h, angle1, angle2); }
  void FillArcs(GC gc, XArc *arcs, int narcs)
       { XFillArcs(GetDisplay(), _id, gc, arcs, narcs); }

  void DrawString(GC gc, int x, int y, const char *string, int length)
       { XDrawString(GetDisplay(), _id, gc, x, y, string, length); }
  void DrawString(GC gc, int x, int y, OString *s)
       { XDrawString(GetDisplay(), _id, gc, x, y,
                     s->GetString(), s->GetLength()); }
  void DrawString16(GC gc, int x, int y, XChar2b *string, int length)
       { XDrawString16(GetDisplay(), _id, gc, x, y, string, length); }

  void DrawImageString(GC gc, int x, int y, char *string, int length)
       { XDrawImageString(GetDisplay(), _id, gc, x, y, string, length); }
  void DrawImageString(GC gc, int x, int y, OString *s)
       { XDrawImageString(GetDisplay(), _id, gc, x, y,
                          s->GetString(), s->GetLength()); }
  void DrawImageString16(GC gc, int x, int y, XChar2b *string, int length)
       { XDrawString16(GetDisplay(), _id, gc, x, y, string, length); }

  void DrawText(GC gc, int x, int y, XTextItem *items, int nitems)
       { XDrawText(GetDisplay(), _id, gc, x, y, items, nitems); }
  void DrawText16(GC gc, int x, int y, XTextItem16 *items, int nitems)
       { XDrawText16(GetDisplay(), _id, gc, x, y, items, nitems); }

  void CopyArea(Drawable src, Drawable dst, GC gc,
                int srcx, int srcy, int w, int h, int dstx, int dsty)
       { XCopyArea(GetDisplay(), src, dst, gc, srcx, srcy, w, h, dstx, dsty); }
  void CopyWindowArea(GC gc, int sx, int sy, int w, int h, int dx, int dy)
       { XCopyArea(GetDisplay(), _id, _id, gc, sx, sy, w, h, dx, dy); }

  void DefineCursor(Cursor cursor)
       { XDefineCursor(GetDisplay(), _id, cursor); XSync(GetDisplay(), False); }
  void UndefineCursor()
       { XUndefineCursor(GetDisplay(), _id); }

  friend class OXClient;
  friend class OTimer;
  friend class OFileHandler;
  friend class OIdleHandler;

protected:
  virtual void _DoRedraw() {}

  const OXWindow *_parent, *_toplevel;
  int _needRedraw, _clearBgnd, _windowExists;
};


#endif  // __OXWINDOW_H
