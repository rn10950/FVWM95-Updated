/**************************************************************************

    This file is part of a xclass desktop manager.
    Copyright (C) 1996-2000 David Barth, Hector Peraza.

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
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <assert.h>

#include <X11/Xatom.h>

#include <xclass/utils.h>
#include <xclass/OXFrame.h>
#include <xclass/OXClient.h>
#include <xclass/OString.h>
#include <xclass/OXListView.h>
#include <xclass/OResourcePool.h>
#include <xclass/OGC.h>
#include <xclass/OXFont.h>
#include <xclass/ODNDmanager.h>

#include "URL.h"
#include "OXDesktopIcon.h"
#include "OXDesktopContainer.h"
#include "OXEditLabel.h"

#include <X11/cursorfont.h>
#include <X11/extensions/shape.h>


Cursor OXDesktopIcon::_defaultCursor = None;
unsigned int OXDesktopIcon::_selPixel;
OXGC *OXDesktopIcon::_defaultGC;
const OXFont *OXDesktopIcon::_defaultFont = NULL;

extern Atom URI_list;

#define SHAPED_LABEL


//---------------------------------------------------------------------------

OXDesktopIcon::OXDesktopIcon(const OXWindow *p, const OPicture *pic,
                             const OPicture *lpic, OString *text,
                             int type, unsigned long size,
                             unsigned int options, unsigned long back) :
  OXFrame(p, 32, 32, options, back) {

  if (!_defaultFont) {
    XGCValues gval;
    unsigned long gmask;

    _defaultFont = GetResourcePool()->GetIconFont();
    _selPixel = GetResourcePool()->GetSelectedFgndColor();

    gmask = GCForeground | GCBackground | GCFont |
            GCFillStyle | GCGraphicsExposures;
    gval.fill_style = FillSolid;
    gval.graphics_exposures = False;
    gval.font = _defaultFont->GetId();
    gval.background = GetResourcePool()->GetFrameBgndColor();
    gval.foreground = GetResourcePool()->GetFrameFgndColor();
    _defaultGC = new OXGC(GetDisplay(), _id, gmask, &gval);

    _defaultCursor = XCreateFontCursor(GetDisplay(), XC_top_left_arrow);
  }

  _pic  = pic;
  _lpic = lpic;
  _name = text;
  _selpic = NULL;
  _type = type;
  _size = size;
  _is_link = (_lpic != NULL);

  _active = _last_state = False;
  _bdown = False;
  _button = 0;
  _dragging = False;

  _normGC = _defaultGC;
  _font = _defaultFont;

  OFontMetrics fm;

  _tw = _font->TextWidth(_name->GetString(), _name->GetLength());
  _th = _font->TextHeight();
  _font->GetFontMetrics(&fm);
  _ta = fm.ascent;

  XSetWindowAttributes wattr;
  unsigned long mask;

  bx = by = -1;

  mask = CWOverrideRedirect | CWBackPixmap;
  wattr.background_pixmap = ParentRelative;
  wattr.override_redirect = True;
    
  XChangeWindowAttributes(GetDisplay(), _id, mask, &wattr);

  Resize(GetDefaultSize());

  AddInput(ButtonPressMask | ButtonReleaseMask | PointerMotionMask);

  XDefineCursor(GetDisplay(), _id, _defaultCursor);

  // Setup the XDND manager. Each icon requires its own separate ODNDmanager
  // object.

  static Atom dndTypeList[2];

  dndTypeList[0] = URI_list;
  dndTypeList[1] = NULL;

  // This is weird: we're casting OXDesktopIcon, an OXFrame derivate, to an
  // OXMainFrame object. OXDesktopIcon objects behave much like a top-level
  // window, they are direct descendants of the root window of the display
  // and therefore need to have an ODNDmanager object of their own attached
  // to them, otherwise they would remain invisible to other dnd
  // applications. Fortunatelly, ODNDmanager internally it does not care
  // whether the passed argument is really an OXMainFrame or not, so the
  // cast below is safe.

  _dndManager = new ODNDmanager(_client, (OXMainFrame *) this, dndTypeList);
}

OXDesktopIcon::~OXDesktopIcon() {
  if (_name) delete _name;
  if (_selpic) delete _selpic;
  if (_normGC != _defaultGC) delete _normGC;
  if (_font != _defaultFont) _client->FreeFont((OXFont *) _font);
}

void OXDesktopIcon::SetName(OString *name) {
  if (_name) delete _name;
  _name = name;
  if (_name) _tw = _font->TextWidth(_name->GetString(), _name->GetLength());
  Layout();
  NeedRedraw();
}

void OXDesktopIcon::Activate(int a) {
  if (_active == a) return;
  _active = a;
  
  if (_active) {
    _selpic = new OSelectedPicture(_client, _pic);
  } else {
    if (_selpic) delete _selpic;
    _selpic = NULL;
  }

#ifdef SHAPED_LABEL
  Layout();
#endif
  _DoRedraw();
}

void OXDesktopIcon::Layout() {
  XRectangle rect;

  _ix = (_w - _pic->GetWidth()) >> 1;
  _iy = 0;
  _lx = (_w - _tw) >> 1;
  _ly = _h - (_th+1) - 2;

  OXFrame::Layout();
  
  if (_pic->GetMask() != None) {
    XShapeCombineMask(GetDisplay(), _id, ShapeBounding,
                      _ix, _iy, _pic->GetMask(), ShapeSet);
  } else {
    rect.x = 0;
    rect.y = 0;
    rect.width  = _pic->GetWidth();
    rect.height = _pic->GetHeight();
    XShapeCombineRectangles(GetDisplay(), _id, ShapeBounding,
                            _ix, _iy, &rect, 1, ShapeSet, Unsorted);
  }

  if (_lpic && _lpic->GetMask() != None) {
    XShapeCombineMask(GetDisplay(), _id, ShapeBounding,
                      _ix, _iy, _lpic->GetMask(), ShapeUnion);
  }

  rect.x = 0;
  rect.y = 0;
  rect.width  = _tw;
  rect.height = _th+1;
#ifdef SHAPED_LABEL
  if (_active) {
    XShapeCombineRectangles(GetDisplay(), _id, ShapeBounding,
                            _lx, _ly, &rect, 1, ShapeUnion, Unsorted);
  } else {
    Pixmap lmask;
    XGCValues gcval;
    unsigned long gcmask;
    GC _maskGC;
    OFontMetrics fm;

    lmask = XCreatePixmap(GetDisplay(), _id, rect.width, rect.height, 1);

    gcmask = GCForeground | GCBackground | GCFillStyle | GCFont;
    gcval.foreground = 0;
    gcval.background = 0;
    gcval.fill_style = FillSolid;
    gcval.font = _font->GetId();
    _maskGC = XCreateGC(GetDisplay(), lmask, gcmask, &gcval);

    XFillRectangle(GetDisplay(), lmask, _maskGC,
                   0, 0, rect.width, rect.height);

    XSetForeground(GetDisplay(), _maskGC, 1);

    XDrawString(GetDisplay(), lmask, _maskGC, 0, _ta,
                _name->GetString(), _name->GetLength());

    XFreeGC(GetDisplay(), _maskGC);

    XShapeCombineMask(GetDisplay(), _id, ShapeBounding,
                      _lx, _ly, lmask, ShapeUnion);

    XFreePixmap(GetDisplay(), lmask);
  }
#else
  XShapeCombineRectangles(GetDisplay(), _id, ShapeBounding,
                          _lx, _ly, &rect, 1, ShapeUnion, Unsorted);
#endif
}

void OXDesktopIcon::_DoRedraw() {

  if (_active) {
    if (_selpic) _selpic->Draw(GetDisplay(), _id, _normGC->GetGC(), _ix, _iy);
    _normGC->SetForeground(_defaultSelectedBackground);
    FillRectangle(_normGC->GetGC(), _lx, _ly, _tw, _th+1);
    _normGC->SetForeground(_selPixel);
  } else {
    _pic->Draw(GetDisplay(), _id, _normGC->GetGC(), _ix, _iy);
//    _normGC->SetForeground(GetResourcePool()->GetDocumentBgndColor());
    _normGC->SetForeground(_client->GetColorByName("turquoise4"));
    FillRectangle(_normGC->GetGC(), _lx, _ly, _tw, _th+1);
//    _normGC->SetForeground(GetResourcePool()->GetDocumentFgndColor());
    _normGC->SetForeground(_whitePixel);
  }

  if (_lpic) _lpic->Draw(GetDisplay(), _id, _normGC->GetGC(), _ix, _iy);

  _name->Draw(GetDisplay(), _id, _normGC->GetGC(), _lx, _ly + _ta);
  _normGC->SetForeground(GetResourcePool()->GetDocumentFgndColor());
}

ODimension OXDesktopIcon::GetDefaultSize() const {
  ODimension size;

  size.w = max(_pic->GetWidth(), _tw);
  size.h = _pic->GetHeight() + _th+1 + 6;

  return size;
}

bool OXDesktopIcon::IsInsideLabel(OPosition pos) const {
  return ((pos.x >= _lx) && (pos.x <= _lx + _tw) &&
          (pos.y >= _ly) && (pos.y <= _ly + _th));
}

bool OXDesktopIcon::IsInsideIcon(OPosition pos) const {
  return ((pos.x >= _ix) && (pos.x <= _ix + _pic->GetWidth()) &&
          (pos.y >= _iy) && (pos.y <= _iy + _pic->GetHeight()));
}


int OXDesktopIcon::HandleButton(XButtonEvent *event) {

  event->window = _fw->GetId();
  event->subwindow = _id;

#if 0
  if ((event->type == ButtonPress) && _active) {
    if ((event->x >= _lx) && (event->x <= _lx + _tw) &&
        (event->y >= _ly) && (event->y <= _ly + _th)) {
      EditLabel();
      return True;
    }
  }
#endif

  if (!_dragging) _fw->HandleButton(event);

  if (event->type == ButtonPress) {
    _bdown = True;
    _button = event->button;
    bx = _x;
    by = _y;
    x0 = event->x;
    y0 = event->y;
  } else {  // ButtonRelease
    if (_dragging) _dndManager->Drop();
    _bdown = False;
    _button = 0;
    _dragging = False;
    if ((bx != _x) || (by != _y)) {
//      PlaceIcon();
    }
  }

  return True;
}

int OXDesktopIcon::HandleDoubleClick(XButtonEvent *event) {

  event->window = _fw->GetId();
  event->subwindow = _id;

  _fw->HandleDoubleClick(event);

  return True;
}

int OXDesktopIcon::HandleMotion(XMotionEvent *event) {

  bx = event->x_root;
  by = event->y_root;

  if (!_dragging) {
    if (_bdown && ((abs(event->x - x0) > 3) || (abs(event->y - y0) > 3))) {
      if (_dndManager) {
        _SetDragPixmap();
        _dragging = _dndManager->StartDrag(this, event->x_root, event->y_root, _id);
      }
    }
  }

  if (_dragging) {
    int action;

    if (event->state & Button3Mask) {
      action = ODNDmanager::DNDactionAsk;
    } else {
      if (event->state & ControlMask) {
        action = ODNDmanager::DNDactionCopy;
      } else {
        action = ODNDmanager::DNDactionMove;
      }
    }
    _dndManager->Drag(event->x_root, event->y_root, action, event->time);
  }

  return True;
}  

void OXDesktopIcon::InitPos() {
  if (bx != -1) Move(bx, by);
  MapWindow();
  LowerWindow();
}

void OXDesktopIcon::PlaceIcon(int x, int y) {
  Move((x|1)-_w/2, (y|1)-(2+_pic->GetHeight()/2));
  ((OXDesktopContainer *)_fw)->Save();
}

OString *OXDesktopIcon::EditLabel() {

  OXEditLabel *edl = new OXEditLabel(_client->GetRoot(), _name);
  edl->Show(_x + _lx - 1, _y + _ly - 3, _tw, edl->GetDefaultHeight());

  _client->WaitForUnmap(edl);

  OString *str = new OString(edl->GetString());

  delete edl;
  return str;
}

void OXDesktopIcon::_SetDragPixmap() {
  Pixmap pic, mask;
  XGCValues gcval;
  unsigned long gcmask;
  GC _picGC, _maskGC;

  const OResourcePool *res = _client->GetResourcePool();

  pic  = XCreatePixmap(GetDisplay(), _id, _w, _h, _client->GetDisplayDepth());
  mask = XCreatePixmap(GetDisplay(), _id, _w, _h, 1);

  gcmask = GCForeground | GCFont;
  gcval.font = _font->GetId();
  gcval.foreground = _blackPixel; //_whitePixel;
  _picGC = XCreateGC(GetDisplay(), _id, gcmask, &gcval);

  gcmask = GCForeground | GCBackground;
  gcval.foreground = 0;
  gcval.background = 0;
  _maskGC = XCreateGC(GetDisplay(), mask, gcmask, &gcval);

  // Draw the pixmap...

  if (_pic)
    _pic->Draw(GetDisplay(), pic, _picGC, _ix, _iy);

  if (_lpic)
    _lpic->Draw(GetDisplay(), pic, _picGC, _ix, _iy);

  XDrawString(GetDisplay(), pic, _picGC, _lx, _ly + _ta,
              _name->GetString(), _name->GetLength());


  // Now draw the mask: first clear the pixmap...
  XFillRectangle(GetDisplay(), mask, _maskGC, 0, 0, _w, _h);

  // ...then draw the icon mask
  gcmask = GCForeground | GCFillStyle | GCStipple |
           GCClipMask | GCClipXOrigin | GCClipYOrigin |
           GCTileStipXOrigin | GCTileStipYOrigin;
  gcval.foreground = 1;
  gcval.fill_style = FillStippled;
  gcval.stipple = res->GetCheckeredBitmap();
  gcval.clip_mask = _pic->GetMask();
  gcval.clip_x_origin = _ix;
  gcval.clip_y_origin = _iy;
  gcval.ts_x_origin = _ix;
  gcval.ts_y_origin = _iy;
  XChangeGC(GetDisplay(), _maskGC, gcmask, &gcval);

  XFillRectangle(GetDisplay(), mask, _maskGC, _ix, _iy,
                 _pic->GetWidth(), _pic->GetHeight());

  if (_lpic) {
    gcmask = GCClipMask;
    gcval.clip_mask = _lpic->GetMask();
    XChangeGC(GetDisplay(), _maskGC, gcmask, &gcval);

    XFillRectangle(GetDisplay(), mask, _maskGC, _ix, _iy,
                   _lpic->GetWidth(), _lpic->GetHeight());
  }

  // ...then draw the label
  gcmask = GCFillStyle | GCClipMask | GCFont;
  gcval.fill_style = FillSolid;
  gcval.clip_mask = None;
  gcval.font = _font->GetId();
  XChangeGC(GetDisplay(), _maskGC, gcmask, &gcval);

  XDrawString(GetDisplay(), mask, _maskGC, _lx, _ly + _ta,
              _name->GetString(), _name->GetLength());

  XFreeGC(GetDisplay(), _picGC);
  XFreeGC(GetDisplay(), _maskGC);

  if (_dndManager) {
    _dndManager->SetDragPixmap(pic, mask, _w/2, 2+_pic->GetHeight()/2);
  } else {
    XFreePixmap(GetDisplay(), pic);
    XFreePixmap(GetDisplay(), mask);
  }
}

//----------------------------------------------------------------------

// The following routines are required by the DND manager.

int OXDesktopIcon::HandleClientMessage(XClientMessageEvent *event) {
  if (_dndManager) {
    if (_dndManager->HandleClientMessage(event)) return True;
  }
  return OXFrame::HandleClientMessage(event);
}

int OXDesktopIcon::HandleSelection(XSelectionEvent *event) {
  if (event->selection == ODNDmanager::DNDselection) {
    if (_dndManager)
      return _dndManager->HandleSelection(event);
  }
  return OXFrame::HandleSelection(event);
}

int OXDesktopIcon::HandleSelectionRequest(XSelectionRequestEvent *event) {
  if (event->selection == ODNDmanager::DNDselection) {
    if (_dndManager)
      return _dndManager->HandleSelectionRequest(event);
  }
  return OXFrame::HandleSelectionRequest(event);
}

//----------------------------------------------------------------------

Atom OXDesktopIcon::HandleDNDenter(Atom *typelist) {
  _last_state = _active;
  Activate(True);

  for (int i = 0; typelist[i] != NULL; ++i)
    if (typelist[i] == URI_list) return typelist[i];

  return None;
}

int OXDesktopIcon::HandleDNDleave() {
  Activate(_last_state);
  return True;
}

Atom OXDesktopIcon::HandleDNDposition(int x, int y, Atom action,
                                      int xroot, int yroot) {
//printf("OXDesktopIcon: dnd position (%#x)\n", action);
  // _dndAction = action;

  // directories usually would accept any of the standard xdnd actions...
  if (S_ISDIR(_type)) return action;

  // for executable files we should return XdndActionPrivate
  // but remember if the source requested XdndActionAsk in order
  // to give the user a choice using a context menu...
  if ((_type & S_IXUSR) || (_type & S_IXGRP) || (_type & S_IXOTH))
    return ODNDmanager::DNDactionPrivate;

  // otherwise refuse the drop
  return None;
}

int OXDesktopIcon::HandleDNDdrop(ODNDdata *data) {
printf("OXDesktopIcon: dnd drop (%s)\n", (char *) data->data);
  Activate(_last_state);
  if (_dndManager->GetTarget() == _id) {
    PlaceIcon();
  } else {
    // do action here...
    char *str = (char *) data->data;
    if (str) {
      URL url(str);

      // fork() -> exec(_name->GetString(), url.full_path)

printf("exec %s %s\n", _name->GetString(), url.full_path);

// if we're a folder we should move the file instead

    }
  }
  return True;
}

int OXDesktopIcon::HandleDNDfinished() {
printf("OXDesktopIcon: dnd finished\n");
  Activate(_last_state);
  return True;
}

ODNDdata *OXDesktopIcon::GetDNDdata(Atom dataType) {
  static ODNDdata data;
  static char str[1024];

  if (dataType == URI_list) {
    char cwd[PATH_MAX];

    sprintf(str, "file://localhost/%s/%s\r\n", getcwd(cwd, PATH_MAX), _name->GetString());
    data.data = (void *) str;
    data.dataLength = strlen(str);
    data.dataType = dataType;

    return &data;

  } else {

    return NULL;

  }
}
