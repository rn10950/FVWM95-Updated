#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <xclass/utils.h>
#include <xclass/OXFrame.h>
#include <xclass/OXFont.h>

#include "OXTButton.h"

#include <fvwm/fvwmlib.h>


//----------------------------------------------------------------------

OXGC *OXTButton::_gc = NULL;

OXTButton::OXTButton(const OXWindow *parent, int ID,
                     const OXFont *norm, const OXFont *bold,
                     char *title, const OPicture *pic) :
  OXButton(parent, ID) {

    TakeFocus(False);

    if (title)
      _title = StrDup(title);
    else
      _title = NULL;

    _p = pic;

    _type = BUTTON_STAYDOWN;
    _autoTip = True;

    _normFont = norm;
    _hiFont = bold;

    _winFlags = _desk = 0;

    if (!_gc) {
      XGCValues gcval;
      unsigned long gcm;

      gcm = GCForeground | GCBackground | GCFont | GCGraphicsExposures;
      gcval.foreground = _blackPixel;
      gcval.background = _backPixel;
      gcval.font = _normFont->GetId();
      gcval.graphics_exposures = False;

      _gc = new OXGC(GetDisplay(), _id, gcm, &gcval);
    }

    XGrabButton(GetDisplay(), AnyButton, AnyModifier, _id, False,
                ButtonPressMask | ButtonReleaseMask |
                EnterWindowMask | LeaveWindowMask,
                GrabModeAsync, GrabModeAsync, None, None);
}

OXTButton::~OXTButton() {
  if (_title) delete[] _title;
}

ODimension OXTButton::GetDefaultSize() const {
  OFontMetrics fm;
  ODimension size;

  _normFont->GetFontMetrics(&fm);
  size.h = fm.ascent + fm.descent + 7;
  size.w = 6;
  if (_title) size.w += _normFont->XTextWidth(_title) + 3;
  if (_p) size.w += _p->GetWidth() + 2;

  return size;
}

void OXTButton::DrawBorder() {
  int x = 0, y = 0, w = _w, h = _h;

  ClearArea(x, y, w, h, False);

  switch (_state) {
  case BUTTON_UP:
    DrawLine(_hilightGC, x,     y,     x+w-2, y);
    DrawLine(_hilightGC, x,     y,     x,     y+h-2);

    DrawLine(_shadowGC,  x+1,   y+h-2, x+w-2, y+h-2);
    DrawLine(_shadowGC,  x+w-2, y+h-2, x+w-2, y+1);
    DrawLine(_blackGC,            x,     y+h-1, x+w-1, y+h-1);
    DrawLine(_blackGC,            x+w-1, y+h-1, x+w-1, y);
    break;

  case BUTTON_ENGAGED:
    FillRectangle(_hibckgndGC->GetGC(), x+2, y+2, w-4, h-4);
    DrawLine(_hilightGC, x+2, y+2, x+w-3, y+2);
  case BUTTON_DOWN:
    DrawLine(_blackGC,            x, y, x+w-1, y);
    DrawLine(_blackGC,            x, y, x,     y+h-1);
    
    DrawLine(_shadowGC,  x+1,   y+1,   x+w-3, y+1);   
    DrawLine(_shadowGC,  x+1,   y+1,   x+1,   y+h-3);   
    DrawLine(_hilightGC, x+1,   y+h-1, x+w-1, y+h-1);
    DrawLine(_hilightGC, x+w-1, y+h-1, x+w-1, y+1);
    break;
  }
}

// This is needed because our taskbar button must behave a little
// differently than a normal OXButton...

int OXTButton::HandleButton(XButtonEvent *event) {
  
  if (!IsEnabled()) return True;

  switch(_type) {
  default:
  case BUTTON_ONOFF:
  case BUTTON_NORMAL:
    return OXButton::HandleButton(event);

  case BUTTON_STAYDOWN:
    OXFrame::HandleButton(event);
    if (event->type == ButtonPress) {
      if (TakesFocus()) RequestFocus();
      _click = True;
      _prevstate = _state;
      if (_state == BUTTON_UP) SetState(BUTTON_DOWN);
      OTButtonMessage message(MSG_TBUTTON, MSG_CLICK, _widgetID, -1);
      SendMessage(_msgObject, &message);
    } else {
      _click = False;
      if (_state == BUTTON_DOWN || _state == BUTTON_ENGAGED) {
        OTButtonMessage message(MSG_TBUTTON, MSG_CLICK, _widgetID,
                                event->button);
        SendMessage(_msgObject, &message);
      }
    }
    break;
  }

  return True;
}

void OXTButton::_DoRedraw() {
  static char *t3p = "...";
  int state, len, x3p, w3p, newx;
  const OXFont *font;
  OFontMetrics fm;
    
  int x = 0, y = 0;

  OXFrame::_DoRedraw();

  if (_state != BUTTON_UP) { x++; y++; }

  if (_state == BUTTON_ENGAGED)
    font = _hiFont;
  else
    font = _normFont;

  font->GetFontMetrics(&fm);
  _gc->SetFont(font->GetId());

  newx = 4;

  w3p = font->XTextWidth(t3p, 3);

  if (_p && (newx + _p->GetWidth() + w3p + 3 < _w)) {
    _p->Draw(GetDisplay(), _id, _gc->GetGC(),
             x + 3, y + ((_h - _p->GetHeight()) >> 1));
    newx += _p->GetWidth()+2;
  }

  if (!_title) return;

  len = strlen(_title);

  if (font->XTextWidth(_title, len) > _w-newx-3) {
    while ((x3p = font->XTextWidth(_title, len) + newx) > _w-w3p-3)
      if (len-- <= 0) break;
    DrawString(_gc->GetGC(), x+newx, y+fm.ascent+3, _title, len);
    DrawString(_gc->GetGC(), x + x3p, y+fm.ascent+3, t3p, 3);
    if (_autoTip) SetTip(_title);
  } else {
    DrawString(_gc->GetGC(), x+newx, y+fm.ascent+3, _title, len);
    if (_autoTip) RemoveTip();
  }
}


void OXTButton::SetName(char *title) {
  if (_title) delete[] _title;
  if (title)
    _title = StrDup(title);
  else
    _title = NULL;
  NeedRedraw();
}

void OXTButton::SetIcon(const OPicture *p) {
  _p = p;
  NeedRedraw();
}
