#ifndef __TOOLBARDEF_H
#define __TOOLBARDEF_H

#include <xclass/OXToolBar.h>

//---------- toolbar button definitions

#define _USE_FLAT_BUTTONS_

#ifdef _USE_FLAT_BUTTONS_
SToolBarDataEx tb_data[] = {
  { "tb-open.xpm",        tb_open_xpm,
    "tb-open.xpm",        tb_open_xpm,
    "tb-open.xpm",        tb_open_xpm,
       "", "Open File",        BUTTON_NORMAL, M_FILE_OPEN,        NULL },
  { "tb-save.xpm",        tb_save_xpm,
    "tb-save.xpm",        tb_save_xpm,
    "tb-save.xpm",        tb_save_xpm,
       "", "Save File",        BUTTON_NORMAL, M_FILE_SAVE,        NULL },
  { "tb-print.xpm",       tb_print_xpm,
    "tb-print.xpm",       tb_print_xpm,
    "tb-print.xpm",       tb_print_xpm,
       "", "Print",            BUTTON_NORMAL, M_FILE_PRINT,       NULL },
  { "",                   NULL,
    "",                   NULL,
    "",                   NULL,
       "", 0,                  0,             -1,                 NULL },
  { "tb-reset.xpm",       tb_reset_xpm,
    "tb-reset.xpm",       tb_reset_xpm,
    "tb-reset.xpm",       tb_reset_xpm,
       "", "Reset View",       BUTTON_NORMAL, M_VIEW_RESET,       NULL },
  { "",                   NULL,
    "",                   NULL,
    "",                   NULL,
       "", 0,                  0,             -1,                 NULL },
  { "tb-leftside.xpm",    tb_leftside_xpm,
    "tb-leftside.xpm",    tb_leftside_xpm,
    "tb-leftside.xpm",    tb_leftside_xpm,
       "", "View Left Side",   BUTTON_NORMAL, M_VIEW_LEFTSIDE,    NULL },
  { "tb-rightside.xpm",   tb_rightside_xpm,
    "tb-rightside.xpm",   tb_rightside_xpm,
    "tb-rightside.xpm",   tb_rightside_xpm,
       "", "View Right Side",  BUTTON_NORMAL, M_VIEW_RIGHTSIDE,   NULL },
  { "tb-topside.xpm",     tb_topside_xpm,
    "tb-topside.xpm",     tb_topside_xpm,
    "tb-topside.xpm",     tb_topside_xpm,
       "", "View Top Side",    BUTTON_NORMAL, M_VIEW_TOPSIDE,     NULL },
  { "tb-bottomside.xpm",  tb_bottomside_xpm,
    "tb-bottomside.xpm",  tb_bottomside_xpm,
    "tb-bottomside.xpm",  tb_bottomside_xpm,
       "", "View Bottom Side", BUTTON_NORMAL, M_VIEW_BOTTOMSIDE,  NULL },
  { "tb-frontside.xpm",   tb_frontside_xpm,
    "tb-frontside.xpm",   tb_frontside_xpm,
    "tb-frontside.xpm",   tb_frontside_xpm,
       "", "View Front Side",  BUTTON_NORMAL, M_VIEW_FRONTSIDE,   NULL },
  { "tb-rearside.xpm",    tb_rearside_xpm,
    "tb-rearside.xpm",    tb_rearside_xpm,
    "tb-rearside.xpm",    tb_rearside_xpm,
       "", "View Rear Side",   BUTTON_NORMAL, M_VIEW_REARSIDE,    NULL },
  { "tb-perspective.xpm", tb_perspective_xpm,
    "tb-perspective.xpm", tb_perspective_xpm,
    "tb-perspective.xpm", tb_perspective_xpm,
       "", "Perspective View", BUTTON_NORMAL, M_VIEW_PERSPECTIVE, NULL },
  { "",                   NULL,
    "",                   NULL,
    "",                   NULL,
       "", 0,                  0,             -1,                 NULL },
  { "tb-wireframe.xpm",   tb_wireframe_xpm,
    "tb-wireframe.xpm",   tb_wireframe_xpm,
    "tb-wireframe.xpm",   tb_wireframe_xpm,
       "", "Toggle Wireframe Mode", BUTTON_ONOFF, M_VIEW_WIREFRAME, NULL },
  { NULL,                 NULL,
    NULL,                 NULL,
    NULL,                 NULL,
       "", 0,                  0,             0,                  NULL }
};
#else
SToolBarData tb_data[] = {
  { "tb-open.xpm",        tb_open_xpm,        "Open File",        BUTTON_NORMAL, M_FILE_OPEN,        NULL },
  { "tb-save.xpm",        tb_save_xpm,        "Save File",        BUTTON_NORMAL, M_FILE_SAVE,        NULL },
  { "tb-print.xpm",       tb_print_xpm,       "Print",            BUTTON_NORMAL, M_FILE_PRINT,       NULL },
  { "",                   NULL,               0,                  0,             -1,                 NULL },
  { "tb-reset.xpm",       tb_reset_xpm,       "Reset View",       BUTTON_NORMAL, M_VIEW_RESET,       NULL },
  { "",                   NULL,               0,                  0,             -1,                 NULL },
  { "tb-leftside.xpm",    tb_leftside_xpm,    "View Left Side",   BUTTON_NORMAL, M_VIEW_LEFTSIDE,    NULL },
  { "tb-rightside.xpm",   tb_rightside_xpm,   "View Right Side",  BUTTON_NORMAL, M_VIEW_RIGHTSIDE,   NULL },
  { "tb-topside.xpm",     tb_topside_xpm,     "View Top Side",    BUTTON_NORMAL, M_VIEW_TOPSIDE,     NULL },
  { "tb-bottomside.xpm",  tb_bottomside_xpm,  "View Bottom Side", BUTTON_NORMAL, M_VIEW_BOTTOMSIDE,  NULL },
  { "tb-frontside.xpm",   tb_frontside_xpm,   "View Front Side",  BUTTON_NORMAL, M_VIEW_FRONTSIDE,   NULL },
  { "tb-rearside.xpm",    tb_rearside_xpm,    "View Rear Side",   BUTTON_NORMAL, M_VIEW_REARSIDE,    NULL },
  { "tb-perspective.xpm", tb_perspective_xpm, "Perspective View", BUTTON_NORMAL, M_VIEW_PERSPECTIVE, NULL },
  { "",                   NULL,               0,                  0,             -1,                 NULL },
  { "tb-wireframe.xpm",   tb_wireframe_xpm,   "Toggle Wireframe Mode", BUTTON_ONOFF, M_VIEW_WIREFRAME, NULL },
  { NULL,                 NULL,               NULL,               0,             0,                  NULL }
};
#endif

#endif  // __TOOLBARDEF_H
