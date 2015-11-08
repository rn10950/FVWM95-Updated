/**************************************************************************

    This is a program intended for testing xclass xdnd events.
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
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <X11/X.h>
#include <X11/Xatom.h>

#include <xclass/utils.h>
#include <xclass/OXClient.h>
#include <xclass/OXWindow.h>
#include <xclass/OXMainFrame.h>
#include <xclass/OXTextButton.h>
#include <xclass/OXIcon.h>
#include <xclass/OXLabel.h>
#include <xclass/OXFont.h>
#include <xclass/OTimer.h>
#include <xclass/OString.h>
#include <xclass/ODNDmanager.h>
#include <xclass/OXMsgBox.h>
#include <xclass/OSelectedPicture.h>
#include <xclass/OResourcePool.h>

static Atom URI_list = None;
static Atom TextPlain = None;


//---------------------------------------------------------------------

class OXDNDicon : public OXIcon {
public:
  OXDNDicon(const OXWindow *p, const OPicture *pic) :
    OXIcon(p, pic, 32, 32) { _active = False; _selpic = NULL; }
  virtual ~OXDNDicon() { if (_selpic) delete _selpic; }

  void Activate(int a) {
    if (_active == a) return;
    _active = a;
  
    if (_active) {
      _selpic = new OSelectedPicture(_client, _pic);
    } else {
      if (_selpic) delete _selpic;
      _selpic = NULL;
    }

    _DoRedraw();
  }

  virtual Atom HandleDNDenter(Atom *typelist) {
    Activate(True);
    for (int i = 0; typelist[i]; ++i)
      if (typelist[i] == URI_list) return URI_list;
    return None;
  }

  virtual int HandleDNDleave() {
    Activate(False);
    return True;
  }

  virtual Atom HandleDNDposition(int x, int y, Atom action, int xr, int yr) {
    if (action == ODNDmanager::DNDactionCopy) return action;
    return None;
  }

  virtual ODNDdata *GetDNDdata(Atom dataType) {
    if (dataType == URI_list) {
      sprintf(str, "file:/tmp/%s\r\n", _pic->GetName());
      data.data = (void *) str;
      data.dataLength = strlen((char *) data.data);
      data.dataType = dataType;
    } else if (dataType == TextPlain) {
      sprintf(str, "My icon name is \"%s\"", _pic->GetName());
      data.data = (void *) str;
      data.dataLength = strlen((char *) data.data);
      data.dataType = dataType;
    } else {
      data.data = NULL;
      data.dataLength = 0;
      data.dataType = None;
    }
    return &data;
  }

  virtual int HandleDNDdrop(ODNDdata *data) {
    char tmp[1000];

    Activate(False);

    sprintf(tmp, "Received DND data:\nLenght = %d\nType = %#d\n\"%s\"",
            data->dataLength, data->dataType, data->data);
    new OXMsgBox(_client->GetRoot(), _toplevel,
                 new OString("OXDNDIcon"), new OString(tmp),
                 MB_ICONASTERISK, ID_OK);

    return True;
  }

protected:
  virtual void _DoRedraw() {
    OXIcon::_DoRedraw();
    if (_active)
      if (_selpic) _selpic->Draw(GetDisplay(), _id, _bckgndGC, 0, 0);
  }

  int _active;
  OSelectedPicture *_selpic;
  ODNDdata data;
  char str[256];
};

class OXMain : public OXMainFrame {
public:
  OXMain(const OXWindow *p, int w, int h);
  virtual ~OXMain();

  virtual int HandleButton(XButtonEvent *event); 
  virtual int HandleMotion(XMotionEvent *event);

  virtual int ProcessMessage(OMessage *msg);

  virtual Atom HandleDNDposition(int x, int y, Atom action, int xr, int yr);
  virtual Atom HandleDNDenter(Atom *typelist);
  virtual int  HandleDNDleave();
  virtual int  HandleDNDdrop(ODNDdata *);

protected:
  void _SetDragPixmap(OXIcon *icon);
  int _GrabRoot();

  OXButton *_close;
  OXDNDicon *_icon[2];
  OXLabel *_status;
  OLayoutHints *_lh, *_ll, *_lb;

  int x0, y0, _bdown, _dragging;

  Atom *_dndTypeList;
};


//---------------------------------------------------------------------

main(int argc, char *argv[]) {
  OXClient *clientX = new OXClient(argc, argv);

  OXMain *mainWindow = new OXMain(clientX->GetRoot(), 400, 200);
  mainWindow->MapWindow();

  clientX->Run();

  return 0;
}


//---------------------------------------------------------------------

OXMain::OXMain(const OXWindow *p, int w, int h) :
  OXMainFrame(p, w, h) {

  _lh = new OLayoutHints(LHINTS_CENTER_X | LHINTS_TOP, 100, 100, 10, 10);
  _ll = new OLayoutHints(LHINTS_CENTER_X | LHINTS_TOP);
  _lb = new OLayoutHints(LHINTS_CENTER_X | LHINTS_BOTTOM, 0, 0, 10, 10);

  _icon[0] = new OXDNDicon(this, _client->GetPicture("folder.s.xpm"));
  AddFrame(_icon[0], _lh);

  _icon[1] = new OXDNDicon(this, _client->GetPicture("doc.s.xpm"));
  AddFrame(_icon[1], _lh);

  _status = new OXLabel(this, new OString("Ready!\n"));
  AddFrame(_status, _lh);

  _close = new OXTextButton(this, new OString("Close"), 101);
  AddFrame(_close, _lb);

  _dndTypeList = new Atom[3];

  _dndTypeList[0] = TextPlain = XInternAtom(GetDisplay(), "text/plain", False);
  _dndTypeList[1] = URI_list  = XInternAtom(GetDisplay(), "text/uri-list", False);
  _dndTypeList[2] = NULL;

  _dndManager = new ODNDmanager(_client, this, _dndTypeList);
  _bdown = False;
  _dragging = False;

  SetWindowName("XDND test");

  AddInput(ButtonPressMask | ButtonReleaseMask | PointerMotionMask);

  _icon[0]->SetDNDTarget(True);
  _icon[0]->SetDNDSource(True);

  _icon[1]->SetDNDTarget(False);
  _icon[1]->SetDNDSource(True);

  SetDNDTarget(True);

  MapSubwindows();
  Resize(GetDefaultSize());
}

OXMain::~OXMain() {
  delete _lh;
  delete _ll;
  delete _lb;
  delete[] _dndTypeList;
}

void OXMain::_SetDragPixmap(OXIcon *icon) {
  Pixmap pic, mask;
  XGCValues gcval;
  unsigned long gcmask;
  GC _picGC, _maskGC;
  int w, h, ix, iy, lx, ly, _tw, _th, _ta;
  OFontMetrics fm;

  OString *_name = new OString((icon == _icon[0]) ? "Home" : "/tmp/doc.s.xpm");

  const OResourcePool *res = _client->GetResourcePool();
  const OXFont *_font = res->GetIconFont();

  _tw = _font->TextWidth(_name->GetString(), _name->GetLength());
  _th = _font->TextHeight();
  _font->GetFontMetrics(&fm);
  _ta = fm.ascent;

  w = max(icon->GetWidth(), _tw);
  h = icon->GetHeight() + _th+1 + 6;

  ix = (w - icon->GetWidth()) >> 1;
  iy = 0;
  lx = (w - _tw) >> 1;
  ly = h - (_th+1) - 2;

  pic  = XCreatePixmap(GetDisplay(), _id, w, h, _client->GetDisplayDepth());
  mask = XCreatePixmap(GetDisplay(), _id, w, h, 1);

  gcmask = GCForeground | GCFont;
  gcval.font = _font->GetId();
  gcval.foreground = _blackPixel; //_whitePixel;
  _picGC = XCreateGC(GetDisplay(), _id, gcmask, &gcval);

  gcmask = GCForeground | GCBackground;
  gcval.foreground = 0;
  gcval.background = 0;
  _maskGC = XCreateGC(GetDisplay(), mask, gcmask, &gcval);

  // Draw the pixmap...

  if (icon->GetPicture())
    icon->GetPicture()->Draw(GetDisplay(), pic, _picGC, ix, iy);

  XDrawString(GetDisplay(), pic, _picGC, lx, ly + _ta,
              _name->GetString(), _name->GetLength());


  // Now draw the mask: first clear the pixmap...
  XFillRectangle(GetDisplay(), mask, _maskGC, 0, 0, w, h);

  // ...then draw the icon mask
  gcmask = GCForeground | GCFillStyle | GCStipple |
           GCClipMask | GCClipXOrigin | GCClipYOrigin |
           GCTileStipXOrigin | GCTileStipYOrigin;
  gcval.foreground = 1;
  gcval.fill_style = FillStippled;
  gcval.stipple = res->GetCheckeredBitmap();
  gcval.clip_mask = icon->GetPicture() ? icon->GetPicture()->GetMask() : None;
  gcval.clip_x_origin = ix;
  gcval.clip_y_origin = iy;
  gcval.ts_x_origin = ix;
  gcval.ts_y_origin = iy;
  XChangeGC(GetDisplay(), _maskGC, gcmask, &gcval);

  XFillRectangle(GetDisplay(), mask, _maskGC, ix, iy,
                 icon->GetWidth(), icon->GetHeight());

  // ...then draw the label
  gcmask = GCFillStyle | GCClipMask | GCFont;
  gcval.fill_style = FillSolid;
  gcval.clip_mask = None;
  gcval.font = _font->GetId();
  XChangeGC(GetDisplay(), _maskGC, gcmask, &gcval);

  XDrawString(GetDisplay(), mask, _maskGC, lx, ly + _ta,
              _name->GetString(), _name->GetLength());

  XFreeGC(GetDisplay(), _picGC);
  XFreeGC(GetDisplay(), _maskGC);

  if (_dndManager) {
    _dndManager->SetDragPixmap(pic, mask, w/2, 2+icon->GetHeight()/2);
  } else {
    XFreePixmap(GetDisplay(), pic);
    XFreePixmap(GetDisplay(), mask);
  }
}


//---------------------------------------------------------------------

int OXMain::ProcessMessage(OMessage *msg) {
  if ((msg->type == MSG_BUTTON) &&
      (msg->action == MSG_CLICK) &&
      (((OWidgetMessage *)msg)->id == 101)) CloseWindow();

  return True;
}

int OXMain::HandleButton(XButtonEvent *event) {

  if (event->type == ButtonPress) {
    x0 = event->x;
    y0 = event->y;
    _bdown = True;

  } else {  // ButtonRelease
    if (_dragging) _dndManager->Drop();
    _bdown = False;
    _dragging = False;

  }
  return True;
}

int OXMain::HandleMotion(XMotionEvent *event) {
  if (!_dragging) {
    if (_bdown && ((abs(event->x - x0) > 3) || (abs(event->y - y0) > 3))) {
      OXFrame *f = GetFrameFromPoint(event->x, event->y);
      if (_dndManager && ((f == _icon[0]) || (f == _icon[1]))) {
        _SetDragPixmap((OXIcon *) f);
        _dragging = _dndManager->StartDrag(f, event->x_root, event->y_root);
        if (!_dragging) {
          new OXMsgBox(_client->GetRoot(), _toplevel,
                       new OString("OXMain"),
                       new OString("Failed to acquire ownership of XdndSelection"),
                       MB_ICONASTERISK, ID_OK);
        }
      }
    }
  }
  if (_dragging) {
    _dndManager->Drag(event->x_root, event->y_root,
                      ODNDmanager::DNDactionCopy, event->time);
  }
  return True;
}


//---------------------------------------------------------------------

Atom OXMain::HandleDNDenter(Atom *typelist) {
  char tmp[256];

  OString *s = new OString("DND enter\n");
  for (int i = 0; typelist[i] != None; ++i) {
    if (typelist[i] == TextPlain) {
      sprintf(tmp, "accepting data type %#d", typelist[i]);
      s->Append(tmp);
      _status->SetText(s);
      Layout();
      return TextPlain;
    }
  }
  s->Append("No suitable data type, refusing drop.");
  _status->SetText(s);
  Layout();
  return None;
}

int OXMain::HandleDNDleave() {
  _status->SetText(new OString("DND leave."));
  Layout();
  return True;
}

int OXMain::HandleDNDdrop(ODNDdata *data) {
  char tmp[1000];

  _status->SetText(new OString("Drop event received"));
  Layout();

  sprintf(tmp, "Received DND data:\nLenght = %d\nType = %#d\n\"%s\"",
          data->dataLength, data->dataType, data->data);
  new OXMsgBox(_client->GetRoot(), _toplevel,
               new OString("OXMain"), new OString(tmp),
               MB_ICONASTERISK, ID_OK);

  return True;
}

Atom OXMain::HandleDNDposition(int x, int y, Atom action, int xr, int yr) {
  char tmp[256];

  sprintf(tmp, "Accepting action %#d", action);
  _status->SetText(new OString(tmp));
  Layout();

  return action;
}
