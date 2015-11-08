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

#ifndef __OXWIDGET_H
#define __OXWIDGET_H

#include <X11/Xlib.h>

#include <xclass/utils.h>
#include <xclass/OComponent.h>
#include <xclass/OMessageCodes.h>


#ifndef interface
#define interface class
#endif


//--- Text justification modes

#define TEXT_LEFT         (1<<0)
#define TEXT_RIGHT        (1<<1)
#define TEXT_CENTER_X     (1<<2)

#define TEXT_TOP          (1<<3)
#define TEXT_BOTTOM       (1<<4)
#define TEXT_CENTER_Y     (1<<5)


//----------------------------------------------------------------------

interface OXWidget {
public:
  OXWidget() {}
  OXWidget(int ID, char *type) { 
    _widgetID = ID;
    _widgetType = type;
    _widgetFlags = 0;
    _msgType = MSG_EMPTY;
  }

  int  WidgetID() const { return _widgetID; }
  char *WidgetType() const { return _widgetType; }

protected:
  int _SetFlags(int flags) { return _widgetFlags |= flags; }
  int _ClearFlags(int flags) { return _widgetFlags &= ~flags; }

  int _widgetID;
  int _widgetFlags;
  int _msgType;
  char *_widgetType;
};


#endif  // __OXWIDGET_H
