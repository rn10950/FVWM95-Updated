#ifndef __MENUDEF_H
#define __MENUDEF_H


//--------- menu states

#define MENU_DISABLED     (1<<0)
#define MENU_CHECKED      (1<<1)
#define MENU_RCHECKED     (1<<2)


//--------- menu entries id

#define M_FILE_EXIT        101

#define M_VIEW_FOG         301
#define M_VIEW_TEXTURE     302

#define M_HELP_KEYS        401
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
  { "E&xit",           M_FILE_EXIT,       0,             NULL },
  { NULL,              -1,                0,             NULL } } };

struct _popup view_popup = {
  NULL, {
  { "&Fog\tf",         M_VIEW_FOG,        MENU_CHECKED,  NULL },
  { "&Textures\tt",    M_VIEW_TEXTURE,    MENU_CHECKED,  NULL },
  { NULL,              -1,                0,             NULL } } };

struct _popup help_popup = {
  NULL, {
  { "&Keys\th",        M_HELP_KEYS,       MENU_CHECKED,  NULL },
  { "",                -1,                0,             NULL },
  { "&About...",       M_HELP_ABOUT,      0,             NULL },
  { NULL,              -1,                0,             NULL } } };


#endif  // __MENUDEF_H
