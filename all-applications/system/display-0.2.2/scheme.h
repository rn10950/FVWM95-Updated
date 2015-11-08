#ifndef __SCHEME_H
#define __SCHEME_H

#include <xclass/OBaseObject.h>

class Scheme : public OBaseObject {
public:
  Scheme(char *name = 0);
  Scheme(Scheme *scheme, char *name = 0);
  virtual ~Scheme();
  
  void SetName(const char *name);
  char *GetName() const { return name; }

  void SetDefaultFont(const char *str);
  char *GetDefaultFont() const { return default_font; }

  void SetMenuFont(const char *str);
  char *GetMenuFont() const { return menu_font; }

  void SetMenuHiFont(const char *str);
  char *GetMenuHiFont() const { return menu_hi_font; }

  void SetDocFixedFont(const char *str);
  char *GetDocFixedFont() const { return doc_fixed_font; }

  void SetDocPropFont(const char *str);
  char *GetDocPropFont() const { return doc_prop_font; }

  void SetIconFont(const char *str);
  char *GetIconFont() const { return icon_font; }

  void SetStatusFont(const char *str);
  char *GetStatusFont() const { return status_font; }

  void SetFrameBgColor(const char *str);
  char *GetFrameBgColor() const { return frame_bg_color; }

  void SetFrameFgColor(const char *str);
  char *GetFrameFgColor() const { return frame_fg_color; }

  void SetDocBgColor(const char *str);
  char *GetDocBgColor() const { return doc_bg_color; }

  void SetDocFgColor(const char *str);
  char *GetDocFgColor() const { return doc_fg_color; }

  void SetSelBgColor(const char *str);
  char *GetSelBgColor() const { return sel_bg_color; }

  void SetSelFgColor(const char *str);
  char *GetSelFgColor() const { return sel_fg_color; }

  void SetTipBgColor(const char *str);
  char *GetTipBgColor() const { return tip_bg_color; }

  void SetTipFgColor(const char *str);
  char *GetTipFgColor() const { return tip_fg_color; }
  
  void SetDesktopBgColor(const char *str);
  char *GetDesktopBgColor() const { return desktop_bg_color; }
  
  void SetDesktopFgColor(const char *str);
  char *GetDesktopFgColor() const { return desktop_fg_color; }
  
  void SetFrameBgPixmap(const char *str);
  char *GetFrameBgPixmap() const { return frame_bg_pixmap; }

  void SetDocBgPixmap(const char *str);
  char *GetDocBgPixmap() const { return doc_bg_pixmap; }

  void SetDesktopBgPixmap(const char *str);
  char *GetDesktopBgPixmap() const { return desktop_bg_pixmap; }

  void PrintValues();

protected:
  char *name;

  char *default_font;
  char *menu_font;
  char *menu_hi_font;
  char *doc_fixed_font;
  char *doc_prop_font;
  char *icon_font;
  char *status_font;

  char *frame_bg_color;
  char *frame_fg_color;
  char *doc_bg_color;
  char *doc_fg_color;
  char *sel_bg_color;
  char *sel_fg_color;
  char *tip_bg_color;
  char *tip_fg_color;
  char *desktop_bg_color;
  char *desktop_fg_color;

  char *frame_bg_pixmap;
  char *doc_bg_pixmap;
  char *desktop_bg_pixmap;
};


#endif  // __SCHEME_H
