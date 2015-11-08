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

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>
#include <math.h>

#include <xclass/OXClient.h>
#include <xclass/OXLabel.h>
#include <xclass/OResourcePool.h>

#include "OXScan.h"


#define S_MAX       10000    // lin mode
#define S_MAXDB     800      // log mode (note: 10 times the normal value)

#define SCAN_WIDTH  300


//----------------------------------------------------------------------

OXBandScope::OXBandScope(const OXWindow *p, int scanwidth,
                         unsigned int options) :
  OXFrame(p, 10, 10, options | OWN_BKGND) {

  _sfont = _client->GetFont("Helvetica -8");
  _nfont = _client->GetFont("Helvetica -10");

  _margins = OInsets(_sfont->XTextWidth("80") + 5, 10,
                     _nfont->TextHeight(),
                     _sfont->TextHeight() + _nfont->TextHeight() + 2);

  _pixw = scanwidth + _margins.l + _margins.r + 1;
  _pixh = 150 + _margins.t + _margins.b + 1;

  _pix = XCreatePixmap(GetDisplay(), _id, _pixw, _pixh,
                       _client->GetDisplayDepth());
  if (_pix == None)
    FatalError("OXBandScope: cannot create buffer pixmap");

  XGCValues gcv;
  _gc = new OXGC(GetDisplay(), _id, 0L, &gcv);
  _gc->SetForeground(_blackPixel);
  _gc->SetFont(_sfont->GetId());

  _lgc = new OXGC(GetDisplay(), _id, 0L, &gcv);
  _lgc->SetForeground(_blackPixel);
  _lgc->SetFillStyle(FillStippled);
  _lgc->SetStipple(_client->GetResourcePool()->GetCheckeredBitmap());

  _ngc = new OXGC(GetDisplay(), _id, 0L, &gcv);
  _ngc->SetForeground(_blackPixel);
  _ngc->SetFont(_nfont->GetId());

  SetBackgroundColor(_blackPixel);

  _style = SCOPE_BARS;
  _yMode = SCOPE_LOG;
  _maxY = S_MAXDB;

  _draw_pending = False;

  _dataLen = scanwidth; //_pixw - _margins.r - _margins.l - 1;

  _data = new int[_dataLen];
  Clear();  // calls Draw()

  _curX = -1;
  _fWidth = 0;
  _bdown = False;

  _curfreq = 0;
  _sfreq = 0;
  _freqstep = 1000;

  XGrabButton(GetDisplay(), Button1, AnyModifier, _id, False,
              ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
              GrabModeAsync, GrabModeAsync, None, None);

  AddInput(PointerMotionMask | EnterWindowMask | LeaveWindowMask);
}

OXBandScope::~OXBandScope() {
  XFreePixmap(GetDisplay(), _pix);
  delete[] _data;
  delete _gc;
  delete _lgc;
  delete _ngc;
  _client->FreeFont(_sfont);
  _client->FreeFont(_nfont);
}

ODimension OXBandScope::GetDefaultSize() const {
  return ODimension(_pixw + _insets.l + _insets.r,
                    _pixh + _insets.t + _insets.b);
}

void OXBandScope::SetScanPoints(int np) {
  _dataLen = np;
  delete[] _data;
  _data = new int[_dataLen];
  Clear();
}

void OXBandScope::SetFilterWidth(int fw) {
  _fWidth = fw;
  Draw();
}

void OXBandScope::Clear() {
  for (int i = 0; i < _dataLen; ++i) _data[i] = 0;
  Draw();
}

void OXBandScope::SetValue(int x, int y) {
  if (y > S_MAX) y = S_MAX;
  _data[x] = y;
  Draw();
}

void OXBandScope::SetMode(int mode) {
  _yMode = mode;
  if (_yMode == SCOPE_LIN) _maxY = S_MAX; else _maxY = S_MAXDB;
  Draw();
}

void OXBandScope::SetStyle(int style) {
  _style = style;
  Draw();
}

void OXBandScope::SetRange(long fmin, long df) {
  _sfreq = fmin;
  _freqstep = df;
  Draw();
}

void OXBandScope::SetTunedFreq(long freq) {
  _curfreq = freq;
  Draw();
}

void OXBandScope::ClearPixmap() {
  XFillRectangle(GetDisplay(), _pix, _blackGC, 0, 0, _pixw, _pixh);
  NeedRedraw();
}

static char *log_label[6] = { "0", "16", "32", "48", "64", "80" };
static char *lin_label[6] = { "0", ".2", ".4", ".6", ".8", "1" };

void OXBandScope::DrawGrid() {
  int  i, w, h, x, y;
  long freq;
  char *str;

  OFontMetrics fm;
  _sfont->GetFontMetrics(&fm);

  _gc->SetForeground(_client->GetColorByName("#008000"));
  _lgc->SetForeground(_client->GetColorByName("#008000"));

  w = _pixw - _margins.r - _margins.l;
  h = _pixh - _margins.b - _margins.t;

  XDrawRectangle(GetDisplay(), _pix, _gc->GetGC(),
                 _margins.l, _margins.t, w, h);

  // vertical lines (X)

  // round to nearest multiple of _freqstep * 50
  int tval = _freqstep * 50;
  freq = ((_sfreq + (tval / 2)) / tval) * tval;

  for ( ; ; freq += tval) {
    x = (freq - _sfreq) / _freqstep;

    if (x < 0) continue;
    if (x > w) break;

    _gc->SetForeground(_client->GetColorByName("#008000"));
    XDrawLine(GetDisplay(), _pix, _lgc->GetGC(),
              _margins.l + x, _margins.t,
              _margins.l + x, _margins.t + h);

    int  mhz, khz;
    char str[12];

    mhz = freq / 1000000L;
    khz = (freq / 1000L) % 1000L;

    sprintf(str, "%2d.%03d", mhz, khz);
    int tw = _nfont->XTextWidth(str);

    int tx = _margins.l + x - tw / 2;
    //_ngc->SetForeground(_client->GetColorByName("#00ff00"));
    _gc->SetForeground(_client->GetColorByName("#00c000"));
    XDrawString(GetDisplay(), _pix, _gc->GetGC(), //_ngc->GetGC(),
                tx, _margins.t + h + fm.ascent,
                str, strlen(str));


  }

  // horizontal lines (Y)

  for (i = 1; i < 5; ++i) {
    y = _margins.t + i * h / 5;
    XDrawLine(GetDisplay(), _pix, _lgc->GetGC(),
              _margins.l, y,
              _margins.l + w, y);
  }

  _gc->SetForeground(_client->GetColorByName("#00c000"));
  for (i = 0; i < 6; ++i) {
    y = _margins.t + i * h / 5;
    if (_yMode == SCOPE_LIN) str = lin_label[5-i]; else str = log_label[5-i];
    XDrawString(GetDisplay(), _pix, _gc->GetGC(),
                _margins.l - 2 - _sfont->XTextWidth(str), y + fm.ascent / 2 - 1,
                str, strlen(str));
  }
}

void OXBandScope::DrawData() {
  int i, y, y0, y1, w, h;

  _gc->SetForeground(_client->GetColorByName("#00ff00"));

  w = _pixw - _margins.r - _margins.l;
  h = _pixh - _margins.b - _margins.t;

  if (_style == SCOPE_LINE) {

    if (_yMode == SCOPE_LIN) {
      y0 = _data[0];
    } else {
      // note the 10 scaling factor in the 200.0 below, to ensure
      // adequate vertical resolution (S_MAXDB is also 10 times higher)
      y0 = (_data[0] > 0) ? (int) (200.0 * log10(_data[0])) : 0;
    }
    y0 = y0 * (h - 2) / _maxY;
    for (i = 1; i < _dataLen; ++i) {
      if (_yMode == SCOPE_LIN) {
        y1 = _data[i];
      } else {
        y1 = (_data[i] > 0) ? (int) (200.0 * log10(_data[i])) : 0;
      }
      y1 = y1 * (h - 2) / _maxY;
      XDrawLine(GetDisplay(), _pix, _gc->GetGC(),
                _margins.l + i, _margins.t + h - 1 - y0,
                _margins.l + i + 1, _margins.t + h - 1 - y1);
      y0 = y1;
    }

  } else {

    for (i = 0; i < _dataLen; ++i) {
      if (_yMode == SCOPE_LIN) {
        y = _data[i];
      } else {
        y = (_data[i] > 0) ? (int) (200.0 * log10(_data[i])) : 0;
      }
      y = y * (h - 2) / _maxY;
      XDrawLine(GetDisplay(), _pix, _gc->GetGC(),
                _margins.l + i + 1, _margins.t + h - 1,
                _margins.l + i + 1, _margins.t + h - 1 - y);
    }

  }

  // highlited area

  int cx = (_curfreq - _sfreq) / _freqstep;

  int xmin = cx - _fWidth / 2 - 1;
  int xmax = cx + _fWidth / 2;

  if (xmin < 0) xmin = 0;
  if (xmax > _dataLen) xmax = _dataLen;

  if ((xmax > 0) && (xmin < _dataLen-1)) {
    _gc->SetForeground(_client->GetColorByName("#80ff40"));

    if (_style == SCOPE_LINE) {

      if (_yMode == SCOPE_LIN) {
        y0 = _data[xmin];
      } else {
        y0 = (_data[xmin] > 0) ? (int) (200.0 * log10(_data[xmin])) : 0;
      }
      y0 = y0 * (h - 2) / _maxY;
      for (i = xmin + 1; i < xmax; ++i) {
        if (_yMode == SCOPE_LIN) {
          y1 = _data[i];
        } else {
          y1 = (_data[i] > 0) ? (int) (200.0 * log10(_data[i])) : 0;
        }
        y1 = y1 * (h - 2) / _maxY;
        XDrawLine(GetDisplay(), _pix, _gc->GetGC(),
                  _margins.l + i, _margins.t + h - 1 - y0,
                  _margins.l + i + 1, _margins.t + h - 1 - y1);
        y0 = y1;
      }

    } else {

      for (i = xmin; i < xmax; ++i) {
        if (_yMode == SCOPE_LIN) {
          y = _data[i];
        } else {
          y = (_data[i] > 0) ? (int) (200.0 * log10(_data[i])) : 0;
        }
        y = y * (h - 2) / _maxY;
        XDrawLine(GetDisplay(), _pix, _gc->GetGC(),
                  _margins.l + i + 1, _margins.t + h - 1,
                  _margins.l + i + 1, _margins.t + h - 1 - y);
      }
    }

  }
}

void OXBandScope::DrawArea() {
  int w, h;

  w = _pixw - _margins.r - _margins.l;
  h = _pixh - _margins.b - _margins.t;

  int cx = (_curfreq - _sfreq) / _freqstep;

  int xmin = cx - _fWidth / 2 - 1;
  int xmax = cx + _fWidth / 2;

  if (xmin < 0) xmin = 0;
  if (xmax > w-1) xmax = w-1;

  if ((xmax > 0) && (xmin < w-1)) {

    _gc->SetForeground(_client->GetColorByName("#402020"));
    XFillRectangle(GetDisplay(), _pix, _gc->GetGC(),
                   _margins.l + xmin + 1, _margins.t + 1,
                   xmax - xmin, h - 1);
  }
}

void OXBandScope::DrawCursor() {
  int w, h;

  w = _pixw - _margins.r - _margins.l;
  h = _pixh - _margins.b - _margins.t;

  int cx = (_curfreq - _sfreq) / _freqstep;

  if (cx > 0 && cx < _dataLen) {
    _ngc->SetForeground(_client->GetColorByName("#ff0000"));
    XDrawLine(GetDisplay(), _pix, _ngc->GetGC(),
              _margins.l + cx, _margins.t + 1,
              _margins.l + cx, _margins.t + h - 1);
  }

  if (_curX > 0) {
    _ngc->SetForeground(_client->GetColorByName("#0000ff"));
    XDrawLine(GetDisplay(), _pix, _ngc->GetGC(),
              _margins.l + _curX, _margins.t + 1,
              _margins.l + _curX, _margins.t + h - 1);

    int  mhz, khz, hz;
    char str[12];
    long freq = _sfreq + _curX * _freqstep;

    mhz = freq / 1000000L;
    khz = (freq / 1000L) % 1000L;
    hz = freq % 1000L;

    sprintf(str, "%2d.%03d.%03d", mhz, khz, hz);
    int tw = _nfont->XTextWidth(str);

    int tx = _margins.l + _curX - tw / 2;
    _ngc->SetForeground(_client->GetColorByName("#00ffff"));
    XDrawString(GetDisplay(), _pix, _ngc->GetGC(),
                tx, _margins.t - 2, str, strlen(str));
  }
}

void OXBandScope::DrawStatus() {
  char str[100];

  OFontMetrics fm;
  _nfont->GetFontMetrics(&fm);

  _ngc->SetForeground(_client->GetColorByName("#00ffff"));

  int y = _pixh - 2 - fm.descent;

  if (_yMode == SCOPE_LIN) strcpy(str, "Lin"); else strcpy(str, "Log");
  XDrawString(GetDisplay(), _pix, _ngc->GetGC(),
              5, y, str, strlen(str));

  sprintf(str, "%ld kHz", _freqstep * _dataLen / 1000);
  XDrawString(GetDisplay(), _pix, _ngc->GetGC(),
              _pixw - _nfont->XTextWidth(str) - 5, y, str, strlen(str));
}

void OXBandScope::Draw() {
  _draw_pending = True;
  NeedRedraw();
}

void OXBandScope::DoDraw() {
  ClearPixmap();
  DrawArea();
  DrawGrid();
  DrawData();
  DrawCursor();
  DrawStatus();
  //NeedRedraw(False);
  _draw_pending = False;
}

int OXBandScope::HandleButton(XButtonEvent *event) {
  if (event->type == ButtonPress) {
    _bdown = True;
    if ((event->x > _insets.l + _margins.l) &&
        (event->x < _w - _insets.r - _margins.r) &&
        (event->y > _insets.t + _margins.t) &&
        (event->y < _h - _insets.b - _margins.b)) {
      _curX = event->x - _insets.l - _margins.l;
      OWidgetMessage msg(MSG_SCOPE, MSG_CLICK, _curX);
      SendMessage(&msg);
    }
  } else {
    _bdown = False;
  }
  return True;
}

int OXBandScope::HandleMotion(XMotionEvent *event) {
  if ((event->x > _insets.l + _margins.l) &&
      (event->x < _w - _insets.r - _margins.r) &&
      (event->y > _insets.t + _margins.t) &&
      (event->y < _h - _insets.b - _margins.b)) {
    _curX = event->x - _insets.l - _margins.l;
    Draw();
    if (_bdown) {
      OWidgetMessage msg(MSG_SCOPE, MSG_CLICK, _curX);
      SendMessage(&msg);
    }
  } else {
    if (_curX != -1) {
      _curX = -1;
      Draw();
    }
  }
  return True;
}

int OXBandScope::HandleCrossing(XCrossingEvent *event) {
  if (event->type == LeaveNotify) {
    if (_curX != -1) {
      _curX = -1;
      Draw();
    }
  }
  return True;
}

void OXBandScope::_DoRedraw() {
  DrawBorder(); //OXFrame::_DoRedraw();
  if (_draw_pending) DoDraw();
  CopyArea(_pix, _id, _gc->GetGC(), 0, 0, _pixw, _pixh,
           _insets.l, _insets.t);
}

//----------------------------------------------------------------------

OXScanWindow::OXScanWindow(const OXWindow *p, OXMain *m, SScanSettings *s) :
  OXTransientFrame(p, NULL, 10, 10) {

  _rxmain = m;
  _settings = s;

  unsigned int opt = GetOptions();
  opt = (opt & ~VERTICAL_FRAME) | HORIZONTAL_FRAME;
  ChangeOptions(opt);  

  OLayoutHints *lv = new OLayoutHints(LHINTS_LEFT | LHINTS_EXPAND_Y, 5, 5, 5, 5);
  OLayoutHints *lh1 = new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X);
  OLayoutHints *lh2 = new OLayoutHints(LHINTS_BOTTOM | LHINTS_EXPAND_X);

  // ---- start/stop and scale buttons

  OXHorizontalFrame *hf = new OXHorizontalFrame(this, 1, 1);
  AddFrame(hf, new OLayoutHints(LHINTS_EXPAND_X | LHINTS_TOP));

  OXVerticalFrame *vef = new OXVerticalFrame(hf, 10, 10, FIXED_WIDTH);
  hf->AddFrame(vef, lv);

  OXLabel *_al = new OXLabel(vef, new OString("Scan"));
  _al->SetFont(_client->GetFont("Helvetica -12 bold"));
  _al->Set3DStyle(LABEL_SUNKEN);
  _start = new OXTextButton(vef, new OHotString("&Start"), 801);
  _start->SetFont(_client->GetFont("Lucida -10"));
  _start->TakeFocus(False);
  _start->Associate(this);
  _stop = new OXTextButton(vef, new OHotString("S&top"), 802);
  _stop->SetFont(_client->GetFont("Lucida -10"));
  _stop->TakeFocus(False);
  _stop->Associate(this);

  vef->AddFrame(_al, lh1);
  vef->AddFrame(_start, lh1);
  vef->AddFrame(_stop, lh1);

  int width = _start->GetDefaultWidth();
  vef->Resize(width + 25, vef->GetDefaultHeight());

  _al = new OXLabel(vef, new OString("Scale"));
  _al->SetFont(_client->GetFont("Helvetica -12 bold"));
  _al->Set3DStyle(LABEL_SUNKEN);
  _slin = new OXTextButton(vef, new OHotString("&Lin"), 803);
  _slin->SetFont(_client->GetFont("Lucida -10"));
  _slin->TakeFocus(False);
  _slin->Associate(this);
  _slog = new OXTextButton(vef, new OHotString("L&og"), 804);
  _slog->SetFont(_client->GetFont("Lucida -10"));
  _slog->TakeFocus(False);
  _slog->Associate(this);

  vef->AddFrame(_al, lh1);
  vef->AddFrame(_slin, lh1);
  vef->AddFrame(_slog, lh1);

  OXTextButton *b;
  _al = new OXLabel(vef, new OString("Style"));
  _al->SetFont(_client->GetFont("Helvetica -12 bold"));
  _al->Set3DStyle(LABEL_SUNKEN);
  vef->AddFrame(_al, lh1);
  b = new OXTextButton(vef, new OHotString("Li&ne"), 805);
  b->SetFont(_client->GetFont("Lucida -10"));
  b->TakeFocus(False);
  b->Associate(this);
  vef->AddFrame(b, lh1);
  b = new OXTextButton(vef, new OHotString("&Bars"), 806);
  b->SetFont(_client->GetFont("Lucida -10"));
  b->TakeFocus(False);
  b->Associate(this);
  vef->AddFrame(b, lh1);

  vef->Resize(width + 25, vef->GetDefaultHeight());

  // ---- band scope

  _plot = new OXBandScope(this, SCAN_WIDTH, SUNKEN_FRAME | DOUBLE_BORDER);
  _plot->Associate(this);
  AddFrame(_plot, new OLayoutHints(LHINTS_EXPAND_Y, 0, 0, 1, 0));

  // ---- range buttons

  hf = new OXHorizontalFrame(this, 1, 1);
  AddFrame(hf, new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y));

  vef = new OXVerticalFrame(hf, 10, 10, FIXED_WIDTH);
  hf->AddFrame(vef, lv);

  _al = new OXLabel(vef, new OString("Range"));
  _al->SetFont(_client->GetFont("Helvetica -12 bold"));
  _al->Set3DStyle(LABEL_SUNKEN);
  vef->AddFrame(_al, lh1);
  b = new OXTextButton(vef, new OHotString("3 kHz"), 1001);
  b->SetFont(_client->GetFont("Lucida -10"));
  b->TakeFocus(False);
  b->Associate(this);
  vef->AddFrame(b, lh1);
  b = new OXTextButton(vef, new OHotString("30 kHz"), 1002);
  b->SetFont(_client->GetFont("Lucida -10"));
  b->TakeFocus(False);
  b->Associate(this);
  vef->AddFrame(b, lh1);
  b = new OXTextButton(vef, new OHotString("60 kHz"), 1003);
  b->SetFont(_client->GetFont("Lucida -10"));
  b->TakeFocus(False);
  b->Associate(this);
  vef->AddFrame(b, lh1);
  b = new OXTextButton(vef, new OHotString("150 kHz"), 1004);
  b->SetFont(_client->GetFont("Lucida -10"));
  b->TakeFocus(False);
  b->Associate(this);
  vef->AddFrame(b, lh1);
  b = new OXTextButton(vef, new OHotString("300 kHz"), 1005);
  b->SetFont(_client->GetFont("Lucida -10"));
  b->TakeFocus(False);
  b->Associate(this);
  vef->AddFrame(b, lh1);
  b = new OXTextButton(vef, new OHotString("600 kHz"), 1006);
  b->SetFont(_client->GetFont("Lucida -10"));
  b->TakeFocus(False);
  b->Associate(this);
  vef->AddFrame(b, lh1);
  b = new OXTextButton(vef, new OHotString("1.5 MHz"), 1007);
  b->SetFont(_client->GetFont("Lucida -10"));
  b->TakeFocus(False);
  b->Associate(this);
  vef->AddFrame(b, lh1);

  b = new OXTextButton(vef, new OHotString("&Close"), 100);
  b->SetFont(_client->GetFont("Lucida -10"));
  b->TakeFocus(False);
  b->Associate(this);
  vef->AddFrame(b, lh2);

  vef->Resize(width + 25, vef->GetDefaultHeight());

  _freqstep = _settings->range / SCAN_WIDTH;
  _plot->SetMode(_settings->ymode);
  _plot->SetStyle(_settings->style);

  InitScan();
 
  MapSubwindows();
  Resize(GetDefaultSize());

  SetWindowName("RX320 Scan");
  SetIconName("RX320 Scan");

  SetWMSizeHints(_w, _h, _w, _h, 0, 0);

  MapWindow();
}

OXScanWindow::~OXScanWindow() {
  _rxmain->CloseScan();
}

int OXScanWindow::CloseWindow() {
  return OXTransientFrame::CloseWindow();
}

int OXScanWindow::HandleTimer(OTimer *t) {
  return False;
}

int OXScanWindow::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;

  switch (msg->type) {
    case MSG_BUTTON:
      switch (msg->action) {
        case MSG_CLICK:
          switch (wmsg->id) {
            case 100:
              CloseWindow();
              break;

            case 801:
              _rxmain->StartScan();
              break;

            case 802:
              _rxmain->StopScan();
              break;

            case 803:
              _plot->SetMode(_settings->ymode = SCOPE_LIN);
              break;

            case 804:
              _plot->SetMode(_settings->ymode = SCOPE_LOG);
              break;

            case 805:
              _plot->SetStyle(_settings->style = SCOPE_LINE);
              break;

            case 806:
              _plot->SetStyle(_settings->style = SCOPE_BARS);
              break;

            case 1001: SetRange(3000); break;
            case 1002: SetRange(30000); break;
            case 1003: SetRange(60000); break;
            case 1004: SetRange(150000); break;
            case 1005: SetRange(300000); break;
            case 1006: SetRange(600000); break;
            case 1007: SetRange(1500000); break;

          }
          break;
      }
      break;

    case MSG_SCOPE:
      switch (msg->action) {
        case MSG_CLICK:
          _rxmain->TuneTo(_sfreq + wmsg->id * _freqstep);
          break;
      }
      break;

  }

  return False;
}

void OXScanWindow::SetRange(long range) {
  _settings->range = range;
  _freqstep = range / SCAN_WIDTH;
  InitScan();
}

void OXScanWindow::UpdateDisplay() {
  _plot->SetTunedFreq(_rxmain->GetVFOA().freq);
  _plot->SetFilterWidth(_rxmain->GetRX()->GetFilter()->bandwidth / _freqstep);
}

void OXScanWindow::InitScan() {
  _curX = 0;
  _plot->Clear();
  _sfreq = _rxmain->GetVFOA().freq - SCAN_WIDTH * _freqstep / 2;
  _plot->SetRange(_sfreq, _freqstep);
  _plot->SetTunedFreq(_rxmain->GetVFOA().freq);
  _plot->SetFilterWidth(_rxmain->GetRX()->GetFilter()->bandwidth / _freqstep);
  _freq = _sfreq;
}

long OXScanWindow::GetStartFreq() {
  return _sfreq;
}

long OXScanWindow::GetNextFreq() {
  if (_curX++ >= SCAN_WIDTH) return -1;
  _freq += _freqstep;
  return _freq;
}

void OXScanWindow::AddPoint(int y) {
  _plot->SetValue(_curX, y);
}
