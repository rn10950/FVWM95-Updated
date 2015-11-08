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

#ifndef __MENUDEF_H
#define __MENUDEF_H


//--------- menu states

#define MENU_DISABLED     (1<<0)
#define MENU_CHECKED      (1<<1)
#define MENU_RCHECKED     (1<<2)


//--------- menu entries id

#define M_FILE_OPEN        101
#define M_FILE_SAVE        102
#define M_FILE_SAVEAS      103
#define M_FILE_PRINT       104
#define M_FILE_PRINTSETUP  105
#define M_FILE_EXIT        106

#define M_EDIT_ADD         201
#define M_EDIT_CHANGE      202
#define M_EDIT_DELETE      203
#define M_EDIT_TUNE        204

#define M_VIEW_TOOLBAR     301
#define M_VIEW_STATUSBAR   302
#define M_VIEW_OPTIONS     303

#define M_HELP_CONTENTS    401
#define M_HELP_SEARCH      402
#define M_HELP_ABOUT       403


//--------- menu definitions

struct _popup {
  OXPopupMenu *ptr;
  struct {
    char *name;
    int  id, state;
    struct _popup *popup_ref;
  } popup[20];
};

struct _popup file_popup = {
  NULL, {
  { "&Open...",          M_FILE_OPEN,       0,             NULL },
  { "&Save",             M_FILE_SAVE,       MENU_DISABLED, NULL },
  { "Save &as...",       M_FILE_SAVEAS,     MENU_DISABLED, NULL },
  { "",                  -1,                0,             NULL },
  { "&Print...",         M_FILE_PRINT,      MENU_DISABLED, NULL },
  { "P&rint setup...",   M_FILE_PRINTSETUP, MENU_DISABLED, NULL },
  { "",                  -1,                0,             NULL },
  { "&Close",            M_FILE_EXIT,       0,             NULL },
  { NULL,                -1,                0,             NULL } } };

struct _popup edit_popup = {
  NULL, {
  { "&Change entry...",    M_EDIT_CHANGE,     MENU_DISABLED, NULL },
  { "Add &new station...", M_EDIT_ADD,        0,             NULL },
  { "&Tune to station",    M_EDIT_TUNE,       MENU_DISABLED, NULL },
  { "",                    -1,                0,             NULL },
  { "&Delete selected station(s)",  M_EDIT_DELETE,  MENU_DISABLED, NULL },
  { NULL,                -1,                0,             NULL } } };

struct _popup view_popup = {
  NULL, {
  { "&Toolbar",          M_VIEW_TOOLBAR,    MENU_CHECKED,  NULL },
  { "Status &Bar",       M_VIEW_STATUSBAR,  MENU_CHECKED,  NULL },
  { "",                  -1,                0,             NULL },
  { "&Options...",       M_VIEW_OPTIONS,    MENU_DISABLED, NULL },
  { NULL,                -1,                0,             NULL } } };

struct _popup help_popup = {
  NULL, {
  { "&Contents...",      M_HELP_CONTENTS,   0,             NULL },
  { "&Search...",        M_HELP_SEARCH,     MENU_DISABLED, NULL },
  { "",                  -1,                0,             NULL },
  { "&About...",         M_HELP_ABOUT,      0,             NULL },
  { NULL,                -1,                0,             NULL } } };


#endif  // __MENUDEF_H
