#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/xpm.h>

#include "OXPaintCanvas.h"
#include "OColorTable.h"


//--- picked up from rgb.c of Xpm lib

char *rgb_fname = "RGBF";
//char *rgb_fname = "/usr/X11/lib/X11/rgb.txt";

#ifndef UNUSE_XPM

// rgb values and ascii names (from rgb text file)
typedef struct {
   int  r, g, b;  // rgb values, range of 0-65535
   char *name;    // color mnemonic of rgb value
} RgbName;

#define MAX_RGBNAMES 1024

#endif // UNUSE_XPM


RgbName rgb_table[MAX_RGBNAMES];

extern "C" int xpmReadRgbNames(const char *, RgbName *); 
extern "C" char *xpmGetRgbName(RgbName *, int, int, int, int);

//extern int xpmReadRgbNames(rgb_fname, rgb_table);

int max_ncolors = -1;


//---------------------------------------------------------------------

OColorInfo::OColorInfo(Pixel pix) {
  symbol  =
  s_name  =
  m_name  =
  g4_name =
  g_name  =
  c_name  = NULL;

  pixel   = pix;

  _left   =
  _right  = NULL;
}

//---------------------------------------------------------------------

OColorTable::OColorTable(Display *dpy, unsigned int screen,
                                       unsigned int depth,
                                       Colormap colormap) {
  int i;

  _dpy = dpy;
  _screen = screen;
  _depth = depth;
  _colormap = colormap;

  if (max_ncolors == -1)
    max_ncolors = xpmReadRgbNames(rgb_fname, rgb_table);

  // allocate max colors + 1 colorTable entries because 0 is transparent

  if (IsBTree()) {
    _table = new OColorInfo*;
    *_table = NULL;
  } else {
    _table = (OColorInfo **) new OColorInfo* [(1 << _depth) + 1];
    for (i = 0; i < (1 << _depth) + 1; ++i) _table[i] = NULL;
  }

}

OColorTable::~OColorTable() {
  int i;

  if (IsBTree()) {
    // recursive delete B-tree
    _DeleteTree(*_table);
    delete _table;
  } else {
    // delete array
    for (i = 0; i < (1 << _depth) + 1; i++)
      if (_table[i]) {
        if (_table[i]->symbol)  delete[] _table[i]->symbol;
        if (_table[i]->s_name)  delete[] _table[i]->s_name;
        if (_table[i]->m_name)  delete[] _table[i]->m_name;
        if (_table[i]->g4_name) delete[] _table[i]->g4_name;
        if (_table[i]->g_name)  delete[] _table[i]->g_name;
        if (_table[i]->c_name)  delete[] _table[i]->c_name;
        delete _table[i];
      }
    delete[] _table;
  }
}

void OColorTable::_DeleteTree(OColorInfo *root) {
  if (root) {
    _DeleteTree(root->_left);
    _DeleteTree(root->_right);
    if (root->symbol)  delete[] root->symbol;
    if (root->s_name)  delete[] root->s_name;
    if (root->m_name)  delete[] root->m_name;
    if (root->g4_name) delete[] root->g4_name;
    if (root->g_name)  delete[] root->g_name;
    if (root->c_name)  delete[] root->c_name;
    delete root;
  }
}

void OColorTable::_ResetColorUse(OColorInfo *root) {
  if (root) {
    root->in_xpm = -1;
    _ResetColorUse(root->_left);
    _ResetColorUse(root->_right);
  }
}

void OColorTable::ResetColorsUse() {
  int i;

  if (IsBTree()) {
    // color table is a B-tree
    if (*_table) _ResetColorUse(*_table);
  } else {
    // color table is an array
    for (i = 0; i < (1 << _depth) + 1; i++)
      if (_table[i]) _table[i]->in_xpm = -1;
  }
} 

OColorInfo *OColorTable::_GetColorWithPixel(OColorInfo *root, Pixel pix) {
  if (root) {
    if (root->pixel == pix)
      return root;
    else if (pix < root->pixel)
      return _GetColorWithPixel(root->_left, pix);
    else
      return _GetColorWithPixel(root->_right, pix);
  } else {
    return root;
  } 
}   

OColorInfo *OColorTable::_AddColor(OColorInfo *root, OColorInfo *color) {
  if (root == NULL) {
    return color;
  } else {
    if (color->pixel < root->pixel)
      root->_left = _AddColor(root->_left, color);
    else
      root->_right = _AddColor(root->_right, color);
      
    return root;
  }
}  

OColorInfo *OColorTable::GetColor(Pixel pixel) {

  if (IsBTree()) {
    // _colorTable is a B-tree, find proper entry recursively
    return _GetColorWithPixel(*_table, pixel);
  } else {
    // _colorTable is an array, pixel is the entry index
    // index in colorTable is 0 for transparent and pixel+1 for others
    int index = (pixel == TRANSPARENT(_dpy, _screen) ? 0 : pixel + 1);
  
    return _table[index];
  }
}

void OColorTable::AddColor(OColorInfo *color) {
  int index;

  if (IsBTree()) {
    // color table is a B-tree, find proper entry recursively
    *_table = _AddColor(*_table, color);
  } else {
    // color table is an array, pixel is the entry index
    // index in color table is 0 for transparent and pixel+1 for others
    index = color->pixel == TRANSPARENT(_dpy, _screen) ? 0 : color->pixel + 1;
    _table[index] = color;
  }
}

void OColorTable::UpdateColor(Pixel pixel, char *symbol, char *sname,
                                           char *mname,  char *g4name,
                                           char *gname,  char *cname) {

  // names are not used as is, instead memory is malloc'ed to fit in
      
  OColorInfo *color = GetColor(pixel);

  if (color == NULL) return; // inexistent color in Table
  
  // Update the color info in ColorTable
  if ((symbol) && ((!color->symbol) || (strcmp(symbol, color->symbol)))) {
    if (color->symbol) delete[] color->symbol;
    color->symbol = _NewString(symbol);
  }
     
  if ((sname) && ((!color->s_name) || (strcmp(sname, color->s_name)))) {
    if (color->s_name) delete[] color->s_name;
    color->s_name = _NewString(sname);
  }
      
  if ((mname) && ((!color->m_name) || (strcmp(mname, color->m_name)))) {
    if (color->m_name) delete[] color->m_name;
    color->m_name = _NewString(mname);
  }

  if ((g4name) && ((!color->g4_name) || (strcmp(g4name, color->g4_name)))) {
    if (color->g4_name) delete[] color->g4_name;
    color->g4_name = _NewString(g4name);
  }
      
  if ((gname) && ((!color->g_name) || (strcmp(gname, color->g_name)))) {
    if (color->g_name) delete[] color->g_name;
    color->g_name = _NewString(gname);
  }

  if ((cname) && ((!color->c_name) ||
                   ((strcmp(cname, color->c_name)) &&
                    ((cname[0] != '#') || (color->c_name[0] == '#'))))) {
    if (color->c_name) delete[] color->c_name;
      
    // try to get a name from rgb_table
    if (cname[0] == '#') {
      XColor xcolor;
      char *rgb_name;
     
      xcolor.pixel = pixel;
      XQueryColor(_dpy, _colormap, &xcolor);
      rgb_name = xpmGetRgbName(rgb_table, max_ncolors,
                                   (int) xcolor.red,
                                   (int) xcolor.green,
                                   (int) xcolor.blue);
        
      if (rgb_name) cname = rgb_name;
    }
         
    color->c_name = _NewString(cname);
  } else if ((!cname) && (!color->c_name)) {
    XColor xcolor;
    char *rgb_name;
                                   
    xcolor.pixel = pixel;
    XQueryColor(_dpy, _colormap, &xcolor);
        
    if (!(rgb_name = xpmGetRgbName(rgb_table, max_ncolors,
                                     (int) xcolor.red,
                                     (int) xcolor.green,
                                     (int) xcolor.blue))) {
      color->c_name = new char[15];
      sprintf(color->c_name, "#%04X%04X%04X",
                             xcolor.red, xcolor.green, xcolor.blue);
    } else {
      color->c_name = _NewString(rgb_name);
    }                          
  }

}

int OColorTable::UseColorInTable(Pixel pixel, char *symbol, char *sname, 
                       char *mname, char *g4name, char *gname, char *cname) {
  int is_new_color = False;

  if (!GetColor(pixel)) {
    // not yet used color, probably not in colorTable
    AddColor(new OColorInfo(pixel));
    is_new_color = True;
  }

  UpdateColor(pixel, symbol, sname, mname, g4name, gname, cname);

  return is_new_color;
}

char *OColorTable::_NewString(char *str) {
  if (!str) return NULL;
  char *s = new char[strlen(str)+1];
  if (s) strcpy(s, str);
  return s;
}
