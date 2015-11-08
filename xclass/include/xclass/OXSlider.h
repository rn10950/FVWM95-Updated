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

#ifndef __OXSLIDER_H
#define __OXSLIDER_H

#include <xclass/OXFrame.h>
#include <xclass/OXWidget.h>
#include <xclass/ODimension.h>
#include <xclass/OMessage.h>


//--- sizes for vert. and horz. sliders

#define SLIDER_WIDTH  24
#define SLIDER_HEIGTH SLIDER_WIDTH

//--- slider types

#define SLIDER_1	0x1
#define SLIDER_2	0x2

//--- scaling of slider

#define SCALE_NO	0x10
#define SCALE_NONE	SCALE_NO
#define SCALE_DOWNRIGHT	0x20
#define SCALE_BOTH	0x40


//----------------------------------------------------------------------

class OSliderMessage : public OWidgetMessage {
public:
  OSliderMessage(int typ, int act, int wid, int p) :
    OWidgetMessage(typ, act, wid) { pos = p; }

  int pos;
};


//----------------------------------------------------------------------
// Abstract base class for sliders

class OXSlider : public OXFrame, public OXWidget {
public:
  OXSlider(const OXWindow *p, int w, int h, int type, int ID = -1,
              unsigned int options = CHILD_FRAME,
              unsigned long back = _defaultFrameBackground);
  virtual ~OXSlider() {}

  virtual int  HandleButton(XButtonEvent *event) = 0;
  virtual int  HandleMotion(XMotionEvent *event) = 0;
  
  virtual void SetScale(int scale) { _scale = scale; }
  virtual void SetRange(int min, int max) { _vmin = min; _vmax = max; }
  virtual void SetPosition(int pos) { _pos = pos; _client->NeedRedraw(this); }
  virtual int  GetPosition() { return _pos; }
  virtual void MapSubwindows() { OXWindow::MapSubwindows(); }

protected:
  int _dragging, _pos, _relpos, _vmin, _vmax, _type, _scale;
  const OPicture *_sliderpic;
};

class OXVSlider : public OXSlider {
public:
  OXVSlider(const OXWindow *p, int h, int type, int ID = -1,
               unsigned int options = VERTICAL_FRAME,
               unsigned long back = _defaultFrameBackground);
  virtual ~OXVSlider();
      
  virtual int HandleButton(XButtonEvent *event);
  virtual int HandleMotion(XMotionEvent *event);
  virtual ODimension GetDefaultSize() const 
                     { return ODimension(SLIDER_WIDTH, _h); }

protected:
  virtual void _DoRedraw();

  int _yp;
};

class OXHSlider : public OXSlider {
public:
  OXHSlider(const OXWindow *p, int w, int type, int ID = -1,
                unsigned int options = HORIZONTAL_FRAME,  
                unsigned long back = _defaultFrameBackground);
  virtual ~OXHSlider();

  virtual int HandleButton(XButtonEvent *event);
  virtual int HandleMotion(XMotionEvent *event);
  virtual ODimension GetDefaultSize() const
                     { return ODimension(_w, SLIDER_HEIGTH); }

protected:
  virtual void _DoRedraw();

  int _xp;
};


#endif  // __OXSLIDER_H
