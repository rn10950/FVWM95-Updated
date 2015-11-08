#ifndef __OCOLORTABLE_H
#define __OCOLORTABLE_H

#include <X11/X.h>
#include <X11/xpm.h>

//---- max display depth to use an array for color table

#define MAX_DEPTH 8


class OColorTable;
class OXPaintCanvas;


//------------------------------------------------------------------------

class OColorInfo {
public:
  OColorInfo(Pixel pix);

  char *symbol;         // symbol used to write pixmap to file
  char *s_name;         // symbolic name of color
  char *m_name;         // monochrome display color
  char *g4_name;        // g4 scale display color
  char *g_name;         // g6 scale display color
  char *c_name;         // color display name
  Pixel pixel;          // pixel of color in colormap
  int  in_xpm;          // tmp index of color in Xpm colormap

  friend class OColorTable;
  friend class OXPaletteFrame;  // por ahora...

protected:
  // the 2 following fields are useless if OColorTable
  // is an array, i.e., for depth < MAX_DEPTH

  OColorInfo *_left;    // B-tree left part
  OColorInfo *_right;   // B-tree right part
};

//------------------------------------------------------------------------

class OColorTable {
public:
  OColorTable(Display *dpy, unsigned int screen, 
                            unsigned int depth, Colormap colormap);
  ~OColorTable();

  void ResetColorsUse();
  OColorInfo *GetColor(Pixel pixel);
  void AddColor(OColorInfo *color);
  void UpdateColor(Pixel pixel, char *symbol, char *sname,
                                char *mname,  char *g4name,
                                char *gname,  char *cname);
  int UseColorInTable(Pixel pixel, char *symbol, char *sname,
                                    char *mname,  char *g4name,
                                    char *gname, char *cname);

  int IsBTree() const { return (_depth > MAX_DEPTH); }
  const OColorInfo **GetTable() const { return (const OColorInfo **) _table; }

protected:
  void _ResetColorUse(OColorInfo *root);
  OColorInfo *_GetColorWithPixel(OColorInfo *root, Pixel pix);
  OColorInfo *_AddColor(OColorInfo *root, OColorInfo *color);
  void _DeleteTree(OColorInfo *root);
  
  char *_NewString(char *str);

  OColorInfo **_table;

  Display *_dpy;
  unsigned int _screen, _depth;
  Colormap _colormap;
};


#endif  // __OCOLORTABLE_H
