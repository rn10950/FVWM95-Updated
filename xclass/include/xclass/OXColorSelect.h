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

#ifndef __OXCOLORSELECT_H
#define __OXCOLORSELECT_H

#include <xclass/OXCompositeFrame.h>
#include <xclass/OXTextButton.h>
#include <xclass/OColor.h>
#include <xclass/OMessage.h>


class OXGC;


//----------------------------------------------------------------------

class OColorSelMessage : public OWidgetMessage {
public:
  OColorSelMessage(int typ, int act, int id, int b = 0,
                   OColor c = OColor(0, 0, 0)) :
    OWidgetMessage(typ, act, id) { button = b; color = c; }

public:
  int button;
  OColor color;
};

class OXColorFrame : public OXFrame {
public:
  OXColorFrame(const OXWindow *p, OColor c, int n);
  virtual ~OXColorFrame();

  virtual int  HandleButton(XButtonEvent *event);
  virtual void DrawBorder();

  void SetActive(bool in) { _active = in; NeedRedraw(); }
  OColor GetColor() const { return _color; }

protected:
  unsigned long _pixel;
  int id;
  bool _active;
  GC grayGC;
  OColor _color;
};

//----------------------------------------------------------------------

class OX16ColorSelector : public OXCompositeFrame {
public:
  OX16ColorSelector(const OXWindow *p);

  virtual int ProcessMessage(OMessage *msg);

  void SetActive(int newat);
  int  GetActive() { return active; }

protected:
  int active;

  OXColorFrame *ce[16];
};

//----------------------------------------------------------------------

class OXColorPopup : public OXCompositeFrame {
public:
  OXColorPopup(const OXWindow *p, OColor color, int dlg = True);

  virtual int HandleButton(XButtonEvent *event);
  virtual int ProcessMessage(OMessage *msg);

  void PlacePopup(int x, int y, int w, int h);
  void EndPopup();

protected:
  int active, _launchDialog;
  OColor _currentColor;
};

//----------------------------------------------------------------------

class OXColorSelect : public OXButton {
public:
  OXColorSelect(const OXWindow *p, OColor color, int id, int dlg = True);
  virtual ~OXColorSelect();

  virtual int HandleButton(XButtonEvent *event);
  virtual int ProcessMessage(OMessage *msg);

  void SetColor(unsigned long c);
  void SetColor(OColor color);
  OColor GetColor() const { return _color; }
  
  virtual ODimension GetDefaultSize() const { return ODimension(43, 21); }
            
protected:
  virtual void _DoRedraw();
  
  void _DrawTriangle(GC gc, int x, int y);

  int _dlg;
  unsigned long _pixel;
  OColor _color;
  OXGC *_drawGC;
};


#endif  // __OXCOLORSELECT_H
