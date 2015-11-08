#ifndef __MENUDEF_H
#define __MENUDEF_H


//--------- menu states

#define MENU_DISABLED     (1<<0)
#define MENU_CHECKED      (1<<1)
#define MENU_RCHECKED     (1<<2)


//--------- menu entries id

#define M_FILE_OPEN        101
#define M_FILE_EXIT        102

#define M_EDIT_CUT         201
#define M_EDIT_COPY        202
#define M_EDIT_SELECTALL   203
#define M_EDIT_INVERTSEL   204

#define M_VIEW_LARGE_ICONS 301
#define M_VIEW_SMALL_ICONS 302
#define M_VIEW_LIST        303
#define M_VIEW_DETAILS     304

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
  { "&Open Containing folder", M_FILE_OPEN,  0,             NULL },
  { "",                        -1,           0,             NULL },
  { "E&xit",                   M_FILE_EXIT,  0,             NULL },
  { NULL,                      -1,           0,             NULL } } };

struct _popup edit_popup = {
  NULL, {
  { "Cu&t",              M_EDIT_CUT,         MENU_DISABLED, NULL },
  { "&Copy",             M_EDIT_COPY,        MENU_DISABLED, NULL },
  { "",                  -1,                 0,             NULL },
  { "Select &All",       M_EDIT_SELECTALL,   MENU_DISABLED, NULL },
  { "&Invert Selection", M_EDIT_INVERTSEL,   MENU_DISABLED, NULL },
  { NULL,                -1,                 0,             NULL } } };

struct _popup view_popup = {
  NULL, {
  { "Lar&ge Icons",      M_VIEW_LARGE_ICONS, 0,             NULL },
  { "S&mall Icons",      M_VIEW_SMALL_ICONS, 0,             NULL },
  { "&List",             M_VIEW_LIST,        0,             NULL },
  { "&Details",          M_VIEW_DETAILS,     MENU_RCHECKED, NULL },
  { NULL,                -1,                 0,             NULL } } };

struct _popup help_popup = {
  NULL, {
  { "&Contents...",      M_HELP_CONTENTS,    MENU_DISABLED, NULL },
  { "&Search...",        M_HELP_SEARCH,      MENU_DISABLED, NULL },
  { "",                  -1,                 0,             NULL },
  { "&About...",         M_HELP_ABOUT,       0,             NULL },
  { NULL,                -1,                 0,             NULL } } };


#endif  // __MENUDEF_H
