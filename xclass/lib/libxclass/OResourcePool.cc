/**************************************************************************

    This file is part of xclass, a Win95-looking GUI toolkit.
    Copyright (C) 1996, 1997 David Barth, Ricky Ralston, Hector Peraza.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>

#include <X11/Xatom.h>
#include <X11/cursorfont.h>

#include <xclass/OXClient.h>
#include <xclass/OXWindow.h>
#include <xclass/OResourcePool.h>
#include <xclass/OGC.h>
#include <xclass/OPicture.h>
#include <xclass/OXFont.h>
#include <xclass/utils.h>
#include <xclass/OMimeTypes.h>


static int gray_width  = 8;
static int gray_height = 8;
static unsigned char gray_bits[] = {
   0x55, 0xaa, 0x55, 0xaa,
   0x55, 0xaa, 0x55, 0xaa
};

#define SB_WIDTH 16;


//----------------------------------------------------------------

OResourcePool::OResourcePool(const OXClient *client, OIniFile *ini) {
  XGCValues gval;
  unsigned long gmask;
  char *picturePoolDir;
  int  retc;
  char *sroot, *uroot;

  _homeDir = getenv("HOME");
  if (!_homeDir) _homeDir = "/";

  Sconfig config;

  int sb_width;

  _client = client;
  _dpy = _client->GetDisplay();
  _defaultColormap = _client->GetDefaultColormap();

  _colorServer = False;

#if 0
  //--- Fall into the defaults if nothing is defined...

  SetDefaults(&config);
  sb_width = SB_WIDTH;

  //--- Then try to load values from the xclass root property, if it exists...

  retc = LoadFromServer(&config);

  //--- Otherwise, try to get the values directly from the .xclassrc file...

  if (!retc) {
    char inirc[PATH_MAX];

    sprintf(inirc, "%s/.xclassrc", _homeDir);
    if (access(inirc, R_OK) != 0) {
      sprintf(inirc, "%s/.xclass/etc/xclassrc", _homeDir);
      if (access(inirc, R_OK) != 0) {
        sprintf(inirc, "%s/etc/xclassrc", OX_DEFAULT_ROOT);
      }
    }

    OIniFile *xcini;
    int ini_ours = False;

    if (ini) {
      xcini = ini;
    } else {
      xcini = new OIniFile(inirc, INI_READ);
      ini_ours = True;
    }
    LoadFromIni(xcini, &config);
    if (ini_ours) delete xcini;
  }
#else
  //--- Fall into the defaults if nothing is defined...

  SetDefaults(&config);
  sb_width = SB_WIDTH;

  //--- Then try to load values from the xclass root property, if it exists...

  //retc = LoadFromServer(&config);

  //--- Get resource values from the .xclassrc file...

  char inirc[PATH_MAX];

  sprintf(inirc, "%s/.xclassrc", _homeDir);
  if (access(inirc, R_OK) != 0) {
    sprintf(inirc, "%s/etc/xclassrc", _homeDir);
    if (access(inirc, R_OK) != 0) {
      sprintf(inirc, "%s/etc/xclassrc", OX_DEFAULT_ROOT);
    }
  }

  OIniFile *xcini;
  int ini_ours = False;

  if (ini) {
    xcini = ini;
  } else {
    xcini = new OIniFile(inirc, INI_READ);
    ini_ours = True;
  }
  LoadFromIni(xcini, &config);
  if (ini_ours) delete xcini;

  //--- Load values from the xclass root property, if it exists, overriding
  //    what we have so far...

  retc = LoadFromServer(&config);

#endif

  //--- If everything failed, then we're here with just the defaults...

  //--- Setup colors...

  _white = _client->GetColorByName("#ffffff");
  _black = _client->GetColorByName("#000000");

  if (!_colorServer) {
    _frameFgnd = _client->GetColorByName(config.frame_fg_color);
    _frameBgnd = _client->GetColorByName(config.frame_bg_color);
    _hilite    = _client->GetHilite(_frameBgnd);
    _shadow    = _client->GetShadow(_frameBgnd);
    _selFgnd   = _client->GetColorByName(config.sel_fg_color);
    _selBgnd   = _client->GetColorByName(config.sel_bg_color);
  }

  _docFgnd   = _client->GetColorByName(config.doc_fg_color);
  _docBgnd   = _client->GetColorByName(config.doc_bg_color);
  _tipFgnd   = _client->GetColorByName(config.tip_fg_color);
  _tipBgnd   = _client->GetColorByName(config.tip_bg_color);


  //--- Create picture pool, gc pool, font pool, etc...

  if ((picturePoolDir = getenv("OX_POOL")) == NULL)
    picturePoolDir = config.icon_pool;

  // OX_ROOT is an absolute path

  if ((sroot = getenv("OX_ROOT")) == NULL)
    sroot = config.sys_resource_root;

  // OX_USER_ROOT should be a relative path, pointing to some 
  // directory in the user's home directory

  if ((uroot = getenv("OX_USER_ROOT")) == NULL)
    uroot = config.user_resource_root;

  _userRoot = new char[strlen(_homeDir)+strlen(uroot)+2];
  sprintf(_userRoot, "%s/%s", _homeDir, uroot);

  _systemRoot = StrDup(sroot);

  _globalPicPool = new OPicturePool(_client, this, picturePoolDir);
  _appPicPool = new OPicturePool(_client, this, picturePoolDir);
  _gcPool = new OGCPool(_client);
  _fontPool = new OFontPool(_client);

  const OXWindow *_Root = _client->GetRoot();

  //--- Load fonts...

  _defaultFont  = _LoadFont(config.default_font);
  _menuFont     = _LoadFont(config.menu_font);
  _menuHiFont   = _LoadFont(config.menu_hi_font);
  _docFixedFont = _LoadFont(config.doc_fixed_font);
  _docPropFont  = _LoadFont(config.doc_prop_font);
  _iconFont     = _LoadFont(config.icon_font);
  _statusFont   = _LoadFont(config.status_font);

  //--- Setup checkered pix/bit-maps...

  _checkered = XCreatePixmapFromBitmapData(_dpy, _Root->GetId(),
                     (char *)gray_bits, gray_width, gray_height,
                     _frameBgnd, _white /*_hilite*/,
                     DefaultDepth(_dpy, DefaultScreen(_dpy)));

  _checkered_bitmap = XCreatePixmapFromBitmapData(_dpy, _Root->GetId(),
                     (char *)gray_bits, gray_width, gray_height,
                     1, 0, 1);

  //--- Setup GCs...

  gmask = GCForeground | GCBackground | GCFont |
          GCFillStyle | GCGraphicsExposures;
  gval.fill_style = FillSolid;
  gval.graphics_exposures = False;
  gval.font = _defaultFont->GetId();
  gval.background = _frameBgnd;
  gval.foreground = _black;
  _blackGC = (OXGC *) _gcPool->GetGC(_Root->GetId(), gmask, &gval, True);

  gval.foreground = _white;
  _whiteGC = (OXGC *) _gcPool->GetGC(_Root->GetId(), gmask, &gval, True);

  gval.foreground = _hilite;
  _hiliteGC = (OXGC *) _gcPool->GetGC(_Root->GetId(), gmask, &gval, True);

  gval.foreground = _shadow;
  _shadowGC = (OXGC *) _gcPool->GetGC(_Root->GetId(), gmask, &gval, True);

  gval.foreground = _frameBgnd;
  _bckgndGC = (OXGC *) _gcPool->GetGC(_Root->GetId(), gmask, &gval, True);

  gval.foreground = _frameFgnd;
  _frameGC = (OXGC *) _gcPool->GetGC(_Root->GetId(), gmask, &gval, True);

  gval.foreground = _selBgnd;
  _selbgndGC = (OXGC *) _gcPool->GetGC(_Root->GetId(), gmask, &gval, True);

  gval.foreground = _selFgnd;
  gval.background = _selBgnd;
  _selGC = (OXGC *) _gcPool->GetGC(_Root->GetId(), gmask, &gval, True);

  gval.font = _docPropFont->GetId();
  gval.foreground = _docFgnd;
  gval.background = _docBgnd;
  _docGC = (OXGC *) _gcPool->GetGC(_Root->GetId(), gmask, &gval, True);

  gval.foreground = _docBgnd;
  _docbgndGC = (OXGC *) _gcPool->GetGC(_Root->GetId(), gmask, &gval, True);

  gval.font = _statusFont->GetId();
  gval.foreground = _tipFgnd;
  gval.background = _tipBgnd;
  _tipGC = (OXGC *) _gcPool->GetGC(_Root->GetId(), gmask, &gval, True);

  gmask = GCForeground | GCBackground | GCFillStyle |
          GCLineWidth  | /*GCLineStyle  |*/
          GCSubwindowMode | GCGraphicsExposures;

  gmask |= GCStipple;
  gval.stipple = _checkered_bitmap;

  gval.foreground = _black; //_shadow;
  gval.background = _selBgnd;
  gval.line_width = 0;
//  gval.line_style = LineOnOffDash;
  gval.fill_style = FillStippled; //FillSolid;
  gval.subwindow_mode = IncludeInferiors;
  gval.graphics_exposures = False;
  _focusGC = (OXGC *) _gcPool->GetGC(_Root->GetId(), gmask, &gval, True);
//  XSetDashes(_dpy, _focusGC->GetGC(), 0, "\x1\x1", 2);

  //--- Setup cursors...

  _defaultCursor = None;
  _grabCursor = XCreateFontCursor(_dpy, XC_arrow);
  _textCursor = XCreateFontCursor(_dpy, XC_xterm);
  _waitCursor = XCreateFontCursor(_dpy, XC_watch);

  //--- Setup background pixmaps...

  if (*config.frame_bg_pixmap)
    _defaultBackgroundPicture = _globalPicPool->GetPicture(config.frame_bg_pixmap);
  else
    _defaultBackgroundPicture = NULL;

  if (*config.doc_bg_pixmap)
    _defaultDocumentBackgroundPicture = _globalPicPool->GetPicture(config.doc_bg_pixmap);
  else
    _defaultDocumentBackgroundPicture = NULL;

  char mimerc[PATH_MAX];
  sprintf(mimerc, "%s/etc/%s", _userRoot, config.mime_type);

  _MimeTypeList = new OMimeTypes((OXClient *)_client, mimerc);
}

OResourcePool::~OResourcePool() {
  delete[] _userRoot;
  delete[] _systemRoot;

  delete _globalPicPool;
  delete _appPicPool;
  delete _gcPool;        // this frees all X GC's

  _fontPool->FreeFont(_defaultFont);
  _fontPool->FreeFont(_menuFont);
  _fontPool->FreeFont(_menuHiFont);
  _fontPool->FreeFont(_docFixedFont);
  _fontPool->FreeFont(_docPropFont);
  _fontPool->FreeFont(_iconFont);
  _fontPool->FreeFont(_statusFont);
  delete _fontPool;

  XFreePixmap(_dpy, _checkered);
  XFreePixmap(_dpy, _checkered_bitmap);
}

// Fill in the resource names array with the default values

int OResourcePool::SetDefaults(Sconfig *conf) {

  char *hm10 = "-adobe-helvetica-medium-r-*-*-10-*-*-*-*-*-iso8859-1";
  char *hm12 = "-adobe-helvetica-medium-r-*-*-12-*-*-*-*-*-iso8859-1";
  char *hb12 = "-adobe-helvetica-bold-r-*-*-12-*-*-*-*-*-iso8859-1";
  char *cm12 = "-adobe-courier-medium-r-*-*-12-*-*-*-*-*-iso8859-1";

  strcpy(conf->default_font,   hm12);
  strcpy(conf->menu_font,      hm12);
  strcpy(conf->menu_hi_font,   hb12);
  strcpy(conf->doc_fixed_font, cm12);
  strcpy(conf->doc_prop_font,  hm12);
  strcpy(conf->icon_font,      hm10);
  strcpy(conf->status_font,    hm10);

  strcpy(conf->frame_bg_color, "#c0c0c0");
  strcpy(conf->frame_fg_color, "#000000");  // black
  strcpy(conf->doc_bg_color,   "#ffffff");  // white
  strcpy(conf->doc_fg_color,   "#000000");
  strcpy(conf->sel_bg_color,   "#000080");
  strcpy(conf->sel_fg_color,   "#ffffff");
  strcpy(conf->tip_bg_color,   "#ffffe0");  // lightyellow
  strcpy(conf->tip_fg_color,   "#000000");

  strcpy(conf->frame_bg_pixmap, "");
  strcpy(conf->doc_bg_pixmap, "");

  strcpy(conf->icon_pool, OX_DEFAULT_POOL);
  strcpy(conf->sys_resource_root, OX_DEFAULT_ROOT);
  strcpy(conf->user_resource_root, ".xclass");
  strcpy(conf->mime_type, "mime.types");

  return True;
}

// Get color scheme from the color server, if there is such thing
// running (this is just experimental and very rudimentary code).

int OResourcePool::LoadFromServer(Sconfig *conf) {
  int  fmt;
  unsigned long nret, nafter, retc = False;
  unsigned char *data;
  Atom ret_atom;
  Window Root = _client->GetRoot()->GetId();

  if (XGetWindowProperty(_dpy, Root, _XCLASS_RESOURCES, 0, 8192,
                         False, XA_STRING, &ret_atom, &fmt, &nret, &nafter,
                         (unsigned char **) &data) == Success) {

    if ((ret_atom == XA_STRING) && (fmt == 8) && (nret > 0)) {
      char *str = strtok((char *) data, "\n");
      _changeMask = 0;

      while (str) {
        char *p = strchr(str, '=');

        if (p) {
          char *arg = p+1;
          while (*arg && (*arg == ' ')) ++arg;
          --p;
          while (p >= str && (*p == ' ')) *p-- = '\0';

          _SetValue(conf, str, arg);
        }

        str = strtok(NULL, "\n");
      }
    }

    retc = True;

    XFree(data);

  } else if (XGetWindowProperty(_dpy, Root, _XCLASS_COLORS, 0, 6,
                    False, _XCLASS_COLORS, &ret_atom, &fmt, &nret, &nafter,
                    (unsigned char **) &data) == Success) {

    if ((ret_atom != None) && (fmt == 32) && (nret == 6)) {
      unsigned long *d = (unsigned long *) data;

      _colorServer = True;

      _frameBgnd = d[0];
      _frameFgnd = d[1];
      _hilite    = d[2];
      _shadow    = d[3];
      _selFgnd   = d[4];
      _selBgnd   = d[5];

      retc = True;
    }

    XFree(data);
  }

  return retc;
}

int OResourcePool::LoadFromIni(OIniFile *xcini, Sconfig *conf) {
  char line[1024], arg[256];

  while (xcini->GetNext(line)) {
    if (strcasecmp(line, "defaults") == 0) {
      if (xcini->GetItem("normal font", arg))
        strcpy(conf->default_font, arg);

      if (xcini->GetItem("menu font", arg))
        strcpy(conf->menu_font, arg);

      if (xcini->GetItem("menu highlight font", arg))
        strcpy(conf->menu_hi_font, arg);

      if (xcini->GetItem("small font", arg)) {
        strcpy(conf->icon_font, arg);
        strcpy(conf->status_font, arg); 
      }

      if (xcini->GetItem("proportional font", arg))
        strcpy(conf->doc_prop_font, arg);

      if (xcini->GetItem("monospaced font", arg))
        strcpy(conf->doc_fixed_font, arg);

      if (xcini->GetItem("fore color", arg))
        strcpy(conf->frame_fg_color, arg);

      if (xcini->GetItem("back color", arg))
        strcpy(conf->frame_bg_color, arg);

      if (xcini->GetItem("hifore color", arg))
        strcpy(conf->sel_fg_color, arg);

      if (xcini->GetItem("hiback color", arg))
        strcpy(conf->sel_bg_color, arg);

      if (xcini->GetItem("doc back color", arg))	
	strcpy(conf->doc_bg_color, arg);

      if (xcini->GetItem("doc fore color", arg))	
	strcpy(conf->doc_fg_color, arg);

      if (xcini->GetItem("tip back color", arg))	
	strcpy(conf->tip_bg_color, arg);

      if (xcini->GetItem("tip fore color", arg))	
	strcpy(conf->tip_fg_color, arg);

      //if (xcini->GetItem("scrollbar width", arg)) sb_width = atoi(arg);
      //if (xcini->GetItem("scrollbar delay", arg)) {
      //  sscanf(arg, "%d,%d", &sb_predly, &sb_rptdly);
      //  IN_RANGE(sb_predly, 1, 10000);
      //  IN_RANGE(sb_rptdly, 1, 10000);
      //}

      if (xcini->GetItem("icon dir", arg))
        strcpy(conf->icon_pool, arg);

      if (xcini->GetItem("background pixmap", arg))
        strcpy(conf->frame_bg_pixmap, arg);

      if (xcini->GetItem("document background pixmap", arg))
        strcpy(conf->doc_bg_pixmap, arg);

      if (xcini->GetItem("system resource root", arg))
        strcpy(conf->sys_resource_root, arg);

      if (xcini->GetItem("user resource root", arg))
        strcpy(conf->user_resource_root, arg);

      if (xcini->GetItem("mime type file", arg))
        strcpy(conf->mime_type, arg);
    }
  }

  return True;
}

int OResourcePool::_SetValue(Sconfig *conf, const char *resource,
                             const char *value, int update) {

  if (strcmp(resource, "normal font") == 0) {
    strcpy(conf->default_font, value);
    _changeMask |= FONTS_CHANGED;

  } else if (strcmp(resource, "menu font") == 0) {
    strcpy(conf->menu_font, value);
    _changeMask |= FONTS_CHANGED;

  } else if (strcmp(resource, "menu highlight font") == 0) {
    strcpy(conf->menu_hi_font, value);
    _changeMask |= FONTS_CHANGED;

  } else if (strcmp(resource, "small font") == 0) {
    strcpy(conf->icon_font, value);
    strcpy(conf->status_font, value); 
    _changeMask |= FONTS_CHANGED;

  } else if (strcmp(resource, "proportional font") == 0) {
    strcpy(conf->doc_prop_font, value);
    _changeMask |= FONTS_CHANGED;

  } else if (strcmp(resource, "monospaced font") == 0) {
    strcpy(conf->doc_fixed_font, value);
    _changeMask |= FONTS_CHANGED;

  } else if (strcmp(resource, "fore color") == 0) {
    strcpy(conf->frame_fg_color, value);
    _changeMask |= COLORS_CHANGED;

  } else if (strcmp(resource, "back color") == 0) {
    strcpy(conf->frame_bg_color, value);
    _changeMask |= COLORS_CHANGED;

  } else if (strcmp(resource, "hifore color") == 0) {
    strcpy(conf->sel_fg_color, value);
    _changeMask |= COLORS_CHANGED;

  } else if (strcmp(resource, "hiback color") == 0) {
    strcpy(conf->sel_bg_color, value);
    _changeMask |= COLORS_CHANGED;

  } else if (strcmp(resource, "doc back color") == 0) {	
    strcpy(conf->doc_bg_color, value);
    _changeMask |= COLORS_CHANGED;

  } else if (strcmp(resource, "doc fore color") == 0) {	
    strcpy(conf->doc_fg_color, value);
    _changeMask |= COLORS_CHANGED;

  } else if (strcmp(resource, "tip back color") == 0) {	
    strcpy(conf->tip_bg_color, value);
    _changeMask |= COLORS_CHANGED;

  } else if (strcmp(resource, "tip fore color") == 0) {	
    strcpy(conf->tip_fg_color, value);
    _changeMask |= COLORS_CHANGED;

  //  } else if (strcmp(resource, "scrollbar width") == 0) {
  //    sb_width = atoi(value);
  //
  //  } else if (strcmp(resource, "scrollbar delay") == 0) {
  //    sscanf(value, "%d,%d", &sb_predly, &sb_rptdly);
  //    IN_RANGE(sb_predly, 1, 10000);
  //    IN_RANGE(sb_rptdly, 1, 10000);

  //  } else if (strcmp(resource, "icon dir") == 0) {
  //    strcpy(conf->icon_pool, value);

  } else if (strcmp(resource, "background pixmap") == 0) {
    strcpy(conf->frame_bg_pixmap, value);
    _changeMask |= PIXMAPS_CHANGED;

  } else if (strcmp(resource, "document background pixmap") == 0) {
    strcpy(conf->doc_bg_pixmap, value);
    _changeMask |= PIXMAPS_CHANGED;

  }

  return _changeMask;
}

int OResourcePool::Reload() {
  Sconfig config;

  SetDefaults(&config);

  int retc = LoadFromServer(&config);
  if (!retc) return 0;

  // TODO: free old pixels, update _checkered pixmap

  if (!_colorServer) {
    _frameFgnd = _client->GetColorByName(config.frame_fg_color);
    _frameBgnd = _client->GetColorByName(config.frame_bg_color);
    _hilite    = _client->GetHilite(_frameBgnd);
    _shadow    = _client->GetShadow(_frameBgnd);
    _selFgnd   = _client->GetColorByName(config.sel_fg_color);
    _selBgnd   = _client->GetColorByName(config.sel_bg_color);
  }

  _docFgnd   = _client->GetColorByName(config.doc_fg_color);
  _docBgnd   = _client->GetColorByName(config.doc_bg_color);
  _tipFgnd   = _client->GetColorByName(config.tip_fg_color);
  _tipBgnd   = _client->GetColorByName(config.tip_bg_color);


  _blackGC->SetBackground(_frameBgnd);

  _whiteGC->SetBackground(_frameBgnd);

  _hiliteGC->SetForeground(_hilite);
  _hiliteGC->SetBackground(_frameBgnd);

  _shadowGC->SetForeground(_shadow);
  _shadowGC->SetBackground(_frameBgnd);

  _bckgndGC->SetForeground(_frameBgnd);
  _bckgndGC->SetBackground(_frameBgnd);

  _frameGC->SetForeground(_frameFgnd);
  _frameGC->SetBackground(_frameBgnd);

  _selbgndGC->SetForeground(_selBgnd);
  _selbgndGC->SetBackground(_frameBgnd);

  _selGC->SetForeground(_selFgnd);
  _selGC->SetBackground(_selBgnd);

  _docGC->SetForeground(_docFgnd);
  _docGC->SetBackground(_docBgnd);

  _docbgndGC->SetForeground(_docBgnd);
  _docbgndGC->SetBackground(_docBgnd);

  _tipGC->SetForeground(_tipFgnd);
  _tipGC->SetBackground(_tipBgnd);

  _focusGC->SetBackground(_selBgnd);

  // update _checkered pixmap

  XFillRectangle(_dpy, _checkered, _whiteGC->GetGC(), /* _hiliteGC->GetGC() */
                 0, 0, gray_width, gray_height);
  _focusGC->SetForeground(_frameBgnd);
  XFillRectangle(_dpy, _checkered, _focusGC->GetGC(),
                 0, 0, gray_width, gray_height);
  _focusGC->SetForeground(_black);
  
  return _changeMask;
}

OXFont *OResourcePool::_LoadFont(char *font_name) {
  OXFont *fs;

  fs = _fontPool->GetFont(font_name);
  if (!fs) {
    Debug(DBG_WARN, "Could not load font \"%s\", trying \"fixed\"...\n",
                    font_name);
    fs = _fontPool->GetFont("fixed");
    if (!fs) FatalError("Could not load font \"fixed\", exiting.");
  }
  return fs;
}

int OResourcePool::ProcessMessage(OMessage *msg) {
  return False;
}

char *OResourcePool::FindIniFile(const char *name, int mode) const {
  char *inipath = new char[PATH_MAX];

  if (mode == INI_READ) {

    // 1: search for the ini file in ${_userRoot}/etc

    sprintf(inipath, "%s/etc/%s", _userRoot, name);
    if (access(inipath, R_OK) == 0) return inipath;

    // 2: if not found, try just ${HOME}/.name

    sprintf(inipath, "%s/%s%s", _homeDir, (*name == '.') ? "" : ".", name);
    if (access(inipath, R_OK) == 0) return inipath;

    // 3: if still not found, search in ${_systemRoot}/etc

    sprintf(inipath, "%s/etc/%s", _systemRoot, name);
    if (access(inipath, R_OK) == 0) return inipath;

    // 4: not found, return NULL...

    delete[] inipath;
    return NULL;

  } else {

    // we'll only attempt to save the file in ${_userRoot}/etc

    sprintf(inipath, "%s/etc", _userRoot);

    if (access(inipath, W_OK) == 0) {
      sprintf(inipath, "%s/etc/%s", _userRoot, name);
      return inipath;
    }

    // the _userRoot directory apparently doesn't exist yet, create it

    if (MakePath(inipath, 0777) == 0) {
      sprintf(inipath, "%s/etc/%s", _userRoot, name);
      return inipath;
    }

    // well, it seems that we do not have write permissions, go figure...

    delete[] inipath;
    return NULL;

  }
}

char *OResourcePool::FindHelpFile(const char *name, const char *appname) const {
  char *help_path = new char[PATH_MAX];

  // 1: search for the help file in ${_userRoot}/docs/appname/

  sprintf(help_path, "%s/docs/%s/%s", _userRoot, appname, name);
  if (access(help_path, R_OK) == 0) return help_path;

  // 2: if not found try in ${_userRoot}/apps/docs/appname/

  sprintf(help_path, "%s/apps/docs/%s/%s", _userRoot, appname, name);
  if (access(help_path, R_OK) == 0) return help_path;

  // 3: if not found, try just in ${_userRoot}/apps/appname/

  sprintf(help_path, "%s/apps/%s/%s", _userRoot, appname, name);
  if (access(help_path, R_OK) == 0) return help_path;


  // 4: if nothing found so far, try then ${_systemRoot}/docs/appname

  sprintf(help_path, "%s/docs/%s/%s", _systemRoot, appname, name);
  if (access(help_path, R_OK) == 0) return help_path;

  // 5: if not found try in ${_systemRoot}/apps/docs/appname/

  sprintf(help_path, "%s/apps/docs/%s/%s", _systemRoot, appname, name);
  if (access(help_path, R_OK) == 0) return help_path;

  // 6: if still not found, try in ${_systemRoot}/apps/appname/

  sprintf(help_path, "%s/apps/%s/%s", _systemRoot, appname, name);
  if (access(help_path, R_OK) == 0) return help_path;


  // 7: nothing found, return NULL...

  delete[] help_path;
  return NULL;
}
