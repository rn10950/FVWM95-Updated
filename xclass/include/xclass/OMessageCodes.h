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

#ifndef __OMESSAGECODES_H
#define __OMESSAGECODES_H


//        name            type     action
//      --------          ----     ------

//--- The null message codes

#define MSG_EMPTY           0
#define MSG_NULL                      0

//--- Widget messages and actions (actions may be shared by several
//    message classes)

#define MSG_MENU            1
#define MSG_CLICK                     1
#define MSG_DBLCLICK                  2
#define MSG_SELECT                    3

#define MSG_BUTTON          2
#define MSG_CHECKBUTTON     3
#define MSG_RADIOBUTTON     4

#define MSG_LISTBOX         5
#define MSG_DDLISTBOX       6
#define MSG_COMBOBOX        7
#define MSG_FOCUSLOST                 4

#define MSG_HSCROLL         8
#define MSG_VSCROLL         9
#define MSG_LINEUP                    5
#define MSG_LINEDOWN                  6
#define MSG_PAGEUP                    7
#define MSG_PAGEDOWN                  8
#define MSG_SLIDERTRACK               9
#define MSG_SLIDERPOS                10

#define MSG_TEXTENTRY      10
#define MSG_TEXTCHANGED               1

#define MSG_TEXTEDIT       11
#define MSG_CURSORPOS                12

#define MSG_TAB            12
#define MSG_TABCHANGED                1

#define MSG_CONTAINER      13
#define MSG_ITEMVIEW       14
#define MSG_LISTVIEW       15
#define MSG_LISTTREE       16
#define MSG_SELCHANGED                3
#define MSG_SELCLEARED                4
#define MSG_DIRCHANGED                5
#define MSG_DROP                     11

#define MSG_SPINNER        17
#define MSG_VALUECHANGED              1

#define MSG_HSLIDER        18
#define MSG_VSLIDER        19

#define MSG_HRESIZER       20
#define MSG_VRESIZER       21

#define MSG_EXEC           22
#define MSG_APPSTOPPED               10
#define MSG_APPEXITED                11
#define MSG_APPFAILED                12

#define MSG_COLORSEL       23

#define MSG_SHUTTER        24

#define MSG_MDI            25

#define MSG_DOCKABLE       26
#define MSG_SHOW                     10
#define MSG_HIDE                     11
#define MSG_DOCK                     12
#define MSG_UNDOCK                   13
#define MSG_CLOSE                    14

#define MSG_HTML           27

#define MSG_HELP           28


#define MSG_MSGMAX         28
#define MSG_USERMSG       100


#endif  // __OMESSAGECODES_H
