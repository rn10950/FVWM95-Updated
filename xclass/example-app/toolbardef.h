#ifndef __TOOLBARDEF_H
#define __TOOLBARDEF_H

#include <xclass/OXToolBar.h>

//---------- toolbar button definitions

SToolBarData tb_data[] = {
  { "tb-open.xpm",  tb_open_xpm,  "Open File", BUTTON_NORMAL, M_FILE_OPEN,  NULL },
  { "tb-save.xpm",  tb_save_xpm,  "Save File", BUTTON_NORMAL, M_FILE_SAVE,  NULL },
  { "tb-print.xpm", tb_print_xpm, "Print",     BUTTON_NORMAL, M_FILE_PRINT, NULL },
  { "",             NULL,         0,           0,             -1,           NULL },
  { NULL,           NULL,         NULL,        0,             0,            NULL }
};

#endif  // __TOOLBARDEF_H
