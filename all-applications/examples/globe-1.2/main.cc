/**************************************************************************

    This file is part of xcglobe, an example xclass program.
    Copyright (C) 1996, 1997 David Barth, Hector Peraza.

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

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <xclass/OXClient.h>
#include <xclass/OXWindow.h>
#include <xclass/OXMainFrame.h>
#include <xclass/OXStatusBar.h>
#include <xclass/OXCanvas.h>
#include <xclass/OXLabel.h>
#include <xclass/OXMenu.h>
#include <xclass/OXMsgBox.h>
#include <xclass/OString.h>
#include <xclass/utils.h>

#include <X11/keysym.h>

#include "globe.data"


#define RAD(x) ((double)x * M_PI / 180.0)
#define DEG(x) ((double)x * 180.0 / M_PI)


#define M_FILE_EXIT        101

#define M_VIEW_GRID        201
#define M_VIEW_ZOOMIN      202
#define M_VIEW_ZOOMOUT     203

#define M_HELP_CONTENTS    301
#define M_HELP_ABOUT       302


//---------------------------------------------------------------------

class OXGlobeFrame : public OXFrame {
public:
  OXGlobeFrame(const OXWindow *p, int w, int h,
               unsigned int options = 0,
               unsigned long back = _defaultFrameBackground);
  ~OXGlobeFrame();

  virtual ODimension GetDefaultSize() const { return ODimension(2*R, 2*R); }

  void SetRadius(int r);
  int  GetRadius() { return R; }
  void ShowGrid(int state);
  void SetViewPoint(int fi, int th);

private:
  void DrawGlobe();
  void DrawSpace();
  void Convert();
  void DrawSegment(GC gc);
  void DrawGrid();

protected:
  virtual void _DoRedraw();
  GC _mapGC, _gridGC, _spcGC, _globeGC;

  int R, Fi, Th, Wfi, Wth, X, Y, Xold, Yold, GridStep;

  double CosFi,  SinFi,    // Fi is longitude
         CosTh,  SinTh,    // Th is latitude
         CosWth, SinWth;

  int _showGrid;
};

class OXAppMainFrame : public OXMainFrame {
public:
  OXAppMainFrame(const OXWindow *p, int w, int h);
  virtual ~OXAppMainFrame();

  virtual int ProcessMessage(OMessage *msg);
  virtual int HandleKey(XKeyEvent * event);

  void UpdateStatus();

  void MoveNorth();
  void MoveSouth();
  void MoveEast();
  void MoveWest();

  void ZoomIn();
  void ZoomOut();

  void ToggleGrid();

protected:
  OXStatusBar *_statusBar;
  OXCanvas *_canvas;
  OXGlobeFrame *_globe;

  OLayoutHints *_menuBarLayout, *_menuBarItemLayout;

  OXMenuBar *_menuBar;
  OXPopupMenu *_menuFile, *_menuView, *_menuHelp;

  int GridState, _wfi, _wth, _step;
};


//---------------------------------------------------------------------

OXClient *clientX;
OXAppMainFrame *mainWindow;

main() {

  clientX = new OXClient;

  mainWindow = new OXAppMainFrame(clientX->GetRoot(), 400, 200);
  mainWindow->MapWindow();

  clientX->Run();
}

OXAppMainFrame::OXAppMainFrame(const OXWindow *p, int w, int h) :
  OXMainFrame(p, w, h) {

  _menuBarLayout = new OLayoutHints(LHINTS_TOP | LHINTS_LEFT | LHINTS_EXPAND_X, 0, 0, 1, 1);
  _menuBarItemLayout = new OLayoutHints(LHINTS_TOP | LHINTS_LEFT, 0, 4, 0, 0);

  _menuFile = new OXPopupMenu(_client->GetRoot());
  _menuFile->AddEntry(new OHotString("E&xit"), M_FILE_EXIT);

  _menuView = new OXPopupMenu(_client->GetRoot());
  _menuView->AddEntry(new OHotString("&Grid"), M_VIEW_GRID);
  _menuView->AddSeparator();
  _menuView->AddEntry(new OHotString("Zoom &In"), M_VIEW_ZOOMIN);
  _menuView->AddEntry(new OHotString("Zoom &Out"), M_VIEW_ZOOMOUT);

  GridState = False;

  _menuHelp = new OXPopupMenu(_client->GetRoot());
  _menuHelp->AddEntry(new OHotString("&Contents"), M_HELP_CONTENTS);
  _menuHelp->AddSeparator();
  _menuHelp->AddEntry(new OHotString("&About..."), M_HELP_ABOUT);

  _menuFile->Associate(this);
  _menuView->Associate(this);
  _menuHelp->Associate(this);

  _menuBar = new OXMenuBar(this, 1, 1, HORIZONTAL_FRAME);
  _menuBar->AddPopup(new OHotString("&File"), _menuFile, _menuBarItemLayout);
  _menuBar->AddPopup(new OHotString("&View"), _menuView, _menuBarItemLayout);
  _menuBar->AddPopup(new OHotString("&Help"), _menuHelp, _menuBarItemLayout);

  AddFrame(_menuBar, _menuBarLayout);

  _canvas = new OXCanvas(this, 304, 304);
  _globe = new OXGlobeFrame(_canvas->GetViewPort(), 10, 10, 
                            HORIZONTAL_FRAME | OWN_BKGND,
                            _whitePixel);
  _canvas->SetContainer(_globe);

  AddFrame(_canvas, new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y,
                                     0, 0, 1, 1));

  _statusBar = new OXStatusBar(this);
  AddFrame(_statusBar, new OLayoutHints(LHINTS_BOTTOM | LHINTS_EXPAND_X,
                                        0, 0, 2, 0));

  _statusBar->SetText(0, new OString("Ready."));

  _globe->SetViewPoint(_wfi = 0, _wth = 0);
  _step = 5;

  UpdateStatus();

  SetWindowName("Globe");
  SetIconName("globe");
  SetClassHints("XCGlobe", "XCGlobe");
  MapSubwindows();

  Resize(GetDefaultSize());
// Resize(400, 200);

  AddInput(KeyPressMask | KeyReleaseMask);
}

OXAppMainFrame::~OXAppMainFrame() {
  delete _menuBarLayout;
  delete _menuBarItemLayout;

  delete _menuFile;
  delete _menuView;
  delete _menuHelp;
}

int OXAppMainFrame::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;

  switch (msg->action) {

    case MSG_CLICK:
      switch(msg->type) {

        case MSG_MENU:
          switch(wmsg->id) {

            case M_FILE_EXIT:
              CloseWindow();
              break;

            case M_VIEW_GRID:
              ToggleGrid();
              break;

            case M_VIEW_ZOOMIN:
              ZoomIn();
              break;

            case M_VIEW_ZOOMOUT:
              ZoomOut();
              break;

            case M_HELP_CONTENTS:
            case M_HELP_ABOUT:
              break;

            default:
              break;
          } // switch(id)
          break;

        default:
          break;

      } // switch(type)
      break;

    default:
      break;

  } // switch(action)

  return True;
}

int OXAppMainFrame::HandleKey(XKeyEvent *event) {
  char tmp[10];
  KeySym keysym;
  XComposeStatus compose = { NULL, 0 };

  if (OXMainFrame::HandleKey(event)) return True;

  if (event->type == KeyPress) {
    XLookupString(event, tmp, sizeof(tmp)-1, &keysym, &compose);
    switch (keysym) {
      case XK_Left:  MoveWest();  break;
      case XK_Right: MoveEast();  break;
      case XK_Up:    MoveNorth(); break;
      case XK_Down:  MoveSouth(); break;
      default:
        if (tmp[0] == 'i' || tmp[0] == 'I')
          ZoomIn();
        else if (tmp[0] == 'o' || tmp[0] == 'O')
          ZoomOut();
        else if (tmp[0] == 'g' || tmp[0] == 'G')
          ToggleGrid();
        else if (tmp[0] == 'q' || tmp[0] == 'Q')
          CloseWindow();
        break;        
    }
  }

  return True;
}

void OXAppMainFrame::MoveNorth() {
  if (_wth <= 90-_step)
    _wth += _step;
  else
    _wth = 90;

  _globe->SetViewPoint(_wfi, _wth);
  UpdateStatus();
}

void OXAppMainFrame::MoveSouth() {
  if (_wth >= _step-90)
    _wth -= _step;
  else
    _wth = -90;

  _globe->SetViewPoint(_wfi, _wth);
  UpdateStatus();
}

void OXAppMainFrame::MoveEast() {
  _wfi += _step;
  _wfi %= 360;

  _globe->SetViewPoint(_wfi, _wth);
  UpdateStatus();
}

void OXAppMainFrame::MoveWest() {
  _wfi -= _step;
  _wfi %= 360;

  _globe->SetViewPoint(_wfi, _wth);
  UpdateStatus();
}

void OXAppMainFrame::ZoomIn() {
  int r = _globe->GetRadius();
  if (r < 1000) r += 50;
  _globe->SetRadius(r);
  _canvas->Layout();
}

void OXAppMainFrame::ZoomOut() {
  int r = _globe->GetRadius();
  if (r > 50) r -= 50;
  _globe->SetRadius(r);
  _canvas->Layout();
}

void OXAppMainFrame::ToggleGrid() {
  GridState = !GridState;
  if (GridState)
    _menuView->CheckEntry(M_VIEW_GRID);
  else
    _menuView->UnCheckEntry(M_VIEW_GRID);
  _globe->ShowGrid(GridState);
}

void OXAppMainFrame::UpdateStatus() {
  int  fi, th;
  char *ew, *ns, tmp[100];

  if (_wth >= 0) th = _wth, ns = "N"; else th = -_wth, ns = "S";
  fi = _wfi % 360;
  if (fi > 180) fi -= 360;
  if (fi < -180) fi += 360;
  if (fi >= 0) ew = "E"; else fi = -fi, ew = "W";
  sprintf(tmp, "View Point: %d %s, %d %s", th, ns, fi, ew);

  _statusBar->SetText(0, new OString(tmp));
}

//---------------------------------------------------------------------

OXGlobeFrame::OXGlobeFrame(const OXWindow *p, int w, int h, 
                           unsigned int options, unsigned long back) :
  OXFrame(p, w, h, options | OWN_BKGND, _whitePixel) {

  XGCValues gval;
  unsigned long gmask;

  R = 150;
  GridStep = 15;

  Wfi = Wth = 0;

  _showGrid = False;

  gmask = GCForeground | GCGraphicsExposures;
  gval.graphics_exposures = False;
  gval.foreground = _client->GetColorByName("#000080");
  _spcGC = XCreateGC(GetDisplay(), _client->GetRoot()->GetId(), gmask, &gval);

  gmask |= GCFillStyle;
  gval.fill_style = FillSolid;
  gval.foreground = _whitePixel;
  _globeGC = XCreateGC(GetDisplay(), _client->GetRoot()->GetId(), gmask, &gval);

  gval.foreground = _client->GetColorByName("#008000");
  _mapGC = XCreateGC(GetDisplay(), _client->GetRoot()->GetId(), gmask, &gval);

  gmask |= GCLineWidth | GCLineStyle;
  gval.foreground = _client->GetColorByName("#8080ff");
  gval.line_width = 0;
  gval.line_style = LineOnOffDash;
  _gridGC = XCreateGC(GetDisplay(), _client->GetRoot()->GetId(), gmask, &gval);
  XSetDashes(GetDisplay(), _gridGC, 0, "\x1\x1", 2);

}

OXGlobeFrame::~OXGlobeFrame() {
  XFreeGC(GetDisplay(), _mapGC);
  XFreeGC(GetDisplay(), _gridGC);
  XFreeGC(GetDisplay(), _globeGC);
  XFreeGC(GetDisplay(), _spcGC);
}

void OXGlobeFrame::_DoRedraw() {
  //OXFrame::_DoRedraw();

  DrawGlobe();
}

void OXGlobeFrame::DrawSpace() {
  int i;

  for (i=0; i<R; i += 2) {
    X = (int) (R - (sqrt( (double)R*R - (double)(R-i)*(R-i) )) - 0.5);
    XDrawLine(GetDisplay(), _id, _spcGC, 0,     i,     X,   i);
    XDrawLine(GetDisplay(), _id, _spcGC, 2*R-X, i,     2*R, i);
    XDrawLine(GetDisplay(), _id, _spcGC, 0,     2*R-i, X,   2*R-i);
    XDrawLine(GetDisplay(), _id, _spcGC, 2*R-X, 2*R-i, 2*R, 2*R-i);
  }
}

void OXGlobeFrame::DrawGlobe() {
  int i;

  DrawSpace();

  CosWth = cos(RAD(Wth));
  SinWth = sin(RAD(Wth));

  i = 0;
  Fi = globe_data[i++];
  Th = globe_data[i++];
  Convert();
  Xold = X;
  Yold = Y;

  //--- plot
  while (i < NPOINTS) {
    Fi = globe_data[i++];
    Th = globe_data[i++];
    if ((Fi != 0) || (Th != 0)) {
      Convert();
      DrawSegment(_mapGC);
      Xold = X;
      Yold = Y;
      continue;
    }
    if (i == NPOINTS) break;
    Fi = globe_data[i++];
    Th = globe_data[i++];
    Convert();
    Xold = X;
    Yold = Y;
  }

  if (_showGrid) DrawGrid();
}

void OXGlobeFrame::Convert() {
  int dFi = Fi - Wfi;

  CosFi = cos(RAD(dFi));
  SinFi = sin(RAD(dFi));
  CosTh = cos(RAD(Th));
  SinTh = sin(RAD(Th));

  X = (int) (CosTh * SinFi * (double) R);
  Y = (int) ((CosFi * CosTh * SinWth - SinTh * CosWth) * (double) R);
}

void OXGlobeFrame::DrawSegment(GC gc) {
  if (CosFi * CosTh * CosWth + SinTh * SinWth > 0.)
    XDrawLine(GetDisplay(), _id, gc, Xold+R, Yold+R, X+R, Y+R);
}

void OXGlobeFrame::ShowGrid(int state) {
  if (state != _showGrid) {
    _showGrid = state;
    //XClearWindow(GetDisplay(), _id);
    XFillArc(GetDisplay(), _id, _globeGC, 0, 0, R+R, R+R, 0, 360*64);
    _client->NeedRedraw(this);
  }
}

void OXGlobeFrame::DrawGrid() {
  int temp;

  //--- Parallels
  temp = Wfi;
  Wfi = 0;
  for (Th=-90+GridStep; Th<90; Th+=GridStep) {
    Fi = -180;
    Convert();
    Xold = X;
    Yold = Y;
    while (Fi < 180) {
      Fi += 6;
      Convert();
      DrawSegment(_gridGC);
      Xold = X;
      Yold = Y;
    }
  }
  Wfi = temp;

  //--- Meridians
  for (Fi=-180; Fi<180; Fi+=GridStep) {
    Th = -84;
    Convert();
    Xold = X;
    Yold = Y;
    while (Th < 84) {
      Th += 6;
      Convert();
      DrawSegment(_gridGC);
      Xold = X;
      Yold = Y;
    }
  }
}

void OXGlobeFrame::SetViewPoint(int fi, int th) {
  if (th < -90) th = -90;
  if (th >  90) th = 90;
  Wth = th;
  Wfi = fi % 360;
  //XClearWindow(GetDisplay(), _id);
  XFillArc(GetDisplay(), _id, _globeGC, 0, 0, R+R, R+R, 0, 360*64);
  _client->NeedRedraw(this);
}

void OXGlobeFrame::SetRadius(int r) {
  R = r;
  //Resize(2*R, 2*R);
  /**/ XClearWindow(GetDisplay(), _id);
  _client->NeedRedraw(this);
}
