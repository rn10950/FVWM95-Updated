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

#define M_EDIT_CUT         201
#define M_EDIT_COPY        202
#define M_EDIT_PASTE       203

#define M_VIEW_TOOLBAR     301
#define M_VIEW_STATUSBAR   302
#define M_VIEW_OPTIONS     303

#define M_VIEW_LEFTSIDE    310
#define M_VIEW_RIGHTSIDE   311
#define M_VIEW_TOPSIDE     312
#define M_VIEW_BOTTOMSIDE  313
#define M_VIEW_FRONTSIDE   314
#define M_VIEW_REARSIDE    315
#define M_VIEW_PERSPECTIVE 316
#define M_VIEW_RESET       320
#define M_VIEW_WIREFRAME   330

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
  { "&Open...",        M_FILE_OPEN,       0,             NULL },
  { "&Save",           M_FILE_SAVE,       MENU_DISABLED, NULL },
  { "Save &as...",     M_FILE_SAVEAS,     MENU_DISABLED, NULL },
  { "",                -1,                0,             NULL },
  { "&Print...",       M_FILE_PRINT,      MENU_DISABLED, NULL },
  { "P&rint setup...", M_FILE_PRINTSETUP, MENU_DISABLED, NULL },
  { "",                -1,                0,             NULL },
  { "E&xit",           M_FILE_EXIT,       0,             NULL },
  { NULL,              -1,                0,             NULL } } };

struct _popup edit_popup = {
  NULL, {
  { "Cu&t",            M_EDIT_CUT,        MENU_DISABLED, NULL },
  { "&Copy",           M_EDIT_COPY,       MENU_DISABLED, NULL },
  { "&Paste",          M_EDIT_PASTE,      MENU_DISABLED, NULL },
  { NULL,              -1,                0,             NULL } } };

struct _popup view_popup = {
  NULL, {
  { "&Toolbar",        M_VIEW_TOOLBAR,    MENU_CHECKED,  NULL },
  { "Status &Bar",     M_VIEW_STATUSBAR,  MENU_CHECKED,  NULL },
  { "",                -1,                0,             NULL },
  { "&Options...",     M_VIEW_OPTIONS,    MENU_DISABLED, NULL },
  { NULL,              -1,                0,             NULL } } };

struct _popup help_popup = {
  NULL, {
  { "&Contents...",    M_HELP_CONTENTS,   MENU_DISABLED, NULL },
  { "&Search...",      M_HELP_SEARCH,     MENU_DISABLED, NULL },
  { "",                -1,                0,             NULL },
  { "&About...",       M_HELP_ABOUT,      0,             NULL },
  { NULL,              -1,                0,             NULL } } };


#endif  // __MENUDEF_H
