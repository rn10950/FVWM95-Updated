#include <xclass/utils.h>

#include "scheme.h"

Scheme::Scheme(char *str) {
  if (str)
    name = StrDup(str);
  else
    name = NULL;

  default_font = NULL;
  menu_font = NULL;
  menu_hi_font = NULL;
  doc_fixed_font = NULL;
  doc_prop_font = NULL;
  icon_font = NULL;
  status_font = NULL;

  frame_bg_color = NULL;
  frame_fg_color = NULL;
  doc_bg_color = NULL;
  doc_fg_color = NULL;
  sel_bg_color = NULL;
  sel_fg_color = NULL;
  tip_bg_color = NULL;
  tip_fg_color = NULL;
  desktop_bg_color = NULL;
  desktop_fg_color = NULL;

  frame_bg_pixmap = NULL;
  doc_bg_pixmap = NULL;
  desktop_bg_pixmap = NULL;
}

Scheme::Scheme(Scheme *scheme, char *nam) {
  name = StrDup(nam);

  default_font = StrDup(scheme->default_font);
  menu_font = StrDup(scheme->menu_font);
  menu_hi_font = StrDup(scheme->menu_hi_font);
  doc_fixed_font = StrDup(scheme->doc_fixed_font);
  doc_prop_font = StrDup(scheme->doc_prop_font);
  icon_font = StrDup(scheme->icon_font);
  status_font = StrDup(scheme->status_font);

  frame_bg_color = StrDup(scheme->frame_bg_color);
  frame_fg_color = StrDup(scheme->frame_fg_color);
  doc_bg_color = StrDup(scheme->doc_bg_color);
  doc_fg_color = StrDup(scheme->doc_fg_color);
  sel_bg_color = StrDup(scheme->sel_bg_color);
  sel_fg_color = StrDup(scheme->sel_fg_color);
  tip_bg_color = StrDup(scheme->tip_bg_color);
  tip_fg_color = StrDup(scheme->tip_fg_color);
  desktop_bg_color = StrDup(scheme->desktop_bg_color);
  desktop_fg_color = StrDup(scheme->desktop_fg_color);

  frame_bg_pixmap = StrDup(scheme->frame_bg_pixmap);
  doc_bg_pixmap = StrDup(scheme->doc_bg_pixmap);
  desktop_bg_pixmap = StrDup(scheme->desktop_bg_pixmap);
}

Scheme::~Scheme() {
  if (name) delete name;

  if (default_font) delete default_font;
  if (menu_font) delete menu_font;
  if (menu_hi_font) delete menu_hi_font;
  if (doc_fixed_font) delete doc_fixed_font;
  if (doc_prop_font) delete doc_prop_font;
  if (icon_font) delete icon_font;
  if (status_font) delete status_font;

  if (frame_bg_color) delete frame_bg_color;
  if (frame_fg_color) delete frame_fg_color;
  if (doc_bg_color) delete doc_bg_color;
  if (doc_fg_color) delete doc_fg_color;
  if (sel_bg_color) delete sel_bg_color;
  if (sel_fg_color) delete sel_fg_color;
  if (tip_bg_color) delete tip_bg_color;
  if (tip_fg_color) delete tip_fg_color;
  if (desktop_bg_color) delete desktop_bg_color;

  if (frame_bg_pixmap) delete frame_bg_pixmap;
  if (doc_bg_pixmap) delete doc_bg_pixmap;
}
  
void Scheme::SetName(const char *str) {
  if (str) {
    if (name) delete[] name;
    name = StrDup(str);
  }
}

void Scheme::SetDefaultFont(const char *str) {
  if (str) {
    if (default_font) delete default_font;
    default_font = StrDup(str);
  }
}

void Scheme::SetMenuFont(const char *str) {
  if (str) {
    if (menu_font) delete menu_font;
    menu_font = StrDup(str);
  }
}

void Scheme::SetMenuHiFont(const char *str) {
  if (str) {
    if (menu_hi_font) delete menu_hi_font;
    menu_hi_font = StrDup(str);
  }
}

void Scheme::SetDocFixedFont(const char *str) {
  if (str) {
    if (doc_fixed_font) delete doc_fixed_font;
    doc_fixed_font = StrDup(str);
  }
}

void Scheme::SetDocPropFont(const char *str) {
  if (str) {
    if (doc_prop_font) delete doc_prop_font;
    doc_prop_font = StrDup(str);
  }
}

void Scheme::SetIconFont(const char *str) {
  if (str) {
    if (icon_font) delete icon_font;
    icon_font = StrDup(str);
  }
}

void Scheme::SetStatusFont(const char *str) {
  if (str) {
    if (status_font) delete status_font;
    status_font = StrDup(str);
  }
}

void Scheme::SetFrameBgColor(const char *str) {
  if (str) {
    if (frame_bg_color) delete frame_bg_color;
    frame_bg_color = StrDup(str);
  }
}

void Scheme::SetFrameFgColor(const char *str) {
  if (str) {
    if (frame_fg_color) delete frame_fg_color;
    frame_fg_color = StrDup(str);
  }
}

void Scheme::SetDocBgColor(const char *str) {
  if (str) {
    if (doc_bg_color) delete doc_bg_color;
    doc_bg_color = StrDup(str);
  }
}

void Scheme::SetDocFgColor(const char *str) {
  if (str) {
    if (doc_fg_color) delete doc_fg_color;
    doc_fg_color = StrDup(str);
  }
}

void Scheme::SetSelBgColor(const char *str) {
  if (str) {
    if (sel_bg_color) delete sel_bg_color;
    sel_bg_color = StrDup(str);
  }
}

void Scheme::SetSelFgColor(const char *str) {
  if (str) {
    if (sel_fg_color) delete sel_fg_color;
    sel_fg_color = StrDup(str);
  }
}

void Scheme::SetTipBgColor(const char *str) {
  if (str) {
    if (tip_bg_color) delete tip_bg_color;
    tip_bg_color = StrDup(str);
  }
}

void Scheme::SetTipFgColor(const char *str) {
  if (str) {
    if (tip_fg_color) delete tip_fg_color;
    tip_fg_color = StrDup(str);
  }
}

void Scheme::SetDesktopBgColor(const char *str) {
  if (str) {
    if (desktop_bg_color) delete desktop_bg_color;
    desktop_bg_color = StrDup(str);
  }
}

void Scheme::SetDesktopFgColor(const char *str) {
  if (str) {
    if (desktop_fg_color) delete desktop_fg_color;
    desktop_fg_color = StrDup(str);
  }
}

void Scheme::SetFrameBgPixmap(const char *str) {
  if (str) {
    if (frame_bg_pixmap) delete frame_bg_pixmap;
    frame_bg_pixmap = StrDup(str);
  }
}

void Scheme::SetDocBgPixmap(const char *str) {
  if (str) {
    if (doc_bg_pixmap) delete doc_bg_pixmap;
    doc_bg_pixmap = StrDup(str);
  }
}

void Scheme::SetDesktopBgPixmap(const char *str) {
  if (str) {
    if (desktop_bg_pixmap) delete desktop_bg_pixmap;
    desktop_bg_pixmap = StrDup(str);
  }
}

void Scheme::PrintValues() {
  printf("Default font:        \"%s\"\n", default_font);
  printf("Menu font:           \"%s\"\n", menu_font);
  printf("Mehu hilite font:    \"%s\"\n", menu_hi_font);
  printf("Doc fixed font:      \"%s\"\n", doc_fixed_font);
  printf("Doc prop font:       \"%s\"\n", doc_prop_font);
  printf("Icon font:           \"%s\"\n", icon_font);
  printf("Status font:         \"%s\"\n", status_font);
  printf("\n");
  printf("Frame bgnd color:    \"%s\"\n", frame_bg_color);
  printf("Frame fgnd color:    \"%s\"\n", frame_fg_color);
  printf("Doc bgnd color:      \"%s\"\n", doc_bg_color);
  printf("Doc fgnd color:      \"%s\"\n", doc_fg_color);
  printf("Sel bgnd color:      \"%s\"\n", sel_bg_color);
  printf("Sel fgnd color:      \"%s\"\n", sel_fg_color);
  printf("Tip bgnd color:      \"%s\"\n", tip_bg_color);
  printf("Tip fgnd color:      \"%s\"\n", tip_fg_color);
  printf("Desktop bgnd color:  \"%s\"\n", desktop_bg_color);
  printf("\n");
  printf("Frame bgnd pixmap:   \"%s\"\n", frame_bg_pixmap);
  printf("Doc bgnd pixmap:     \"%s\"\n", doc_bg_pixmap);
  printf("Desktop bgnd pixmap: \"%s\"\n", desktop_bg_pixmap);
}
