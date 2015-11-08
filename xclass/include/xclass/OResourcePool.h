/**************************************************************************

    This file is part of xclass, a Win95-looking GUI toolkit.
    Copyright (C) 1996, 1997 David Barth, Hector Peraza.

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

#ifndef __ORESOURCEPOOL_H
#define __ORESOURCEPOOL_H

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/xpm.h>

#include <xclass/OComponent.h>
#include <xclass/ODimension.h>
#include <xclass/OIniFile.h>
//#include <xclass/OGC.h>
//#include <xclass/OXFont.h>
#include <xclass/OXSList.h>

#undef NULL
#define NULL 0

class OXClient;
class OIniFile;
class OMessage;
class OFontPool;
class OXFont;
class OGCPool;
class OXGC;
class OPicturePool;
class OPicture;
class OMimeTypes;


#define COLORS_CHANGED    (1<<0)
#define FONTS_CHANGED     (1<<1)
#define PIXMAPS_CHANGED   (1<<2)
#define CURSORS_CHANGED   (1<<3)


//----------------------------------------------------------------------
// OResourcePool is a class implementing a pool for all the default
// resource set, like GCs, colors, fonts, etc. neccessary to frames
// in order to perform drawing functions.

class OResourcePool : public OComponent {
private:
  struct Sconfig {
  public:
    char default_font[256];
    char menu_font[256];
    char menu_hi_font[256];
    char doc_fixed_font[256];
    char doc_prop_font[256];
    char icon_font[256];
    char status_font[256];
  
    char frame_bg_color[256];
    char frame_fg_color[256];
    char doc_bg_color[256];
    char doc_fg_color[256];
    char sel_bg_color[256];
    char sel_fg_color[256];
    char tip_bg_color[256];
    char tip_fg_color[256];
  
    char frame_bg_pixmap[256];
    char doc_bg_pixmap[256];
  
    char icon_pool[256];
    char sys_resource_root[256];
    char user_resource_root[256];
  
    char mime_type[256];
  };
  int _colorServer, _changeMask;

public:
  OResourcePool(const OXClient *client, OIniFile *ini = NULL);
  virtual ~OResourcePool();

  void InitColors();
  void InitFonts();
  void InitGCs();

  int  SetDefaults(Sconfig *);
  int  LoadFromServer(Sconfig *);
  int  LoadFromIni(OIniFile *ini, Sconfig*);
  
  int  Reload();

  // This should be for loading applications inifile.
  // the idea being that the system default $OX_SYSTEM_ROOT/etc is for
  // the system defaults. and $OX_USER_ROOT/etc for the users own
  // settings.

  // OIniFile GetIni(const char *name);

  // Apps should avoid calling directly the following funcions:

  OGCPool *GetGCPool() const { return _gcPool; }
  OFontPool *GetFontPool() const { return _fontPool; }
  OPicturePool *GetPicturePool() const { return _globalPicPool; }

  virtual int ProcessMessage(OMessage *msg);

protected:
  OXFont *_LoadFont(char *font_name);
  int _SetValue(Sconfig *conf, const char *resource, const char *value,
                int update = False);

public:
  //--- inline functions:

  // Color values...

  const Pixel GetWhiteColor()        const { return _white; }
  const Pixel GetBlackColor()        const { return _black; }

  const Pixel GetFrameFgndColor()    const { return _frameFgnd; }
  const Pixel GetFrameBgndColor()    const { return _frameBgnd; }
  const Pixel GetFrameHiliteColor()  const { return _hilite; }
  const Pixel GetFrameShadowColor()  const { return _shadow; }

  const Pixel GetDocumentFgndColor() const { return _docFgnd; }
  const Pixel GetDocumentBgndColor() const { return _docBgnd; }

  const Pixel GetSelectedFgndColor() const { return _selFgnd; }
  const Pixel GetSelectedBgndColor() const { return _selBgnd; }

  const Pixel GetTipFgndColor()      const { return _tipFgnd; }
  const Pixel GetTipBgndColor()      const { return _tipBgnd; }

  // Fonts...

  const OXFont *GetDefaultFont()       const { return _defaultFont; }
  const OXFont *GetMenuFont()          const { return _menuFont; }
  const OXFont *GetMenuHiliteFont()    const { return _menuHiFont; }
  const OXFont *GetDocumentFixedFont() const { return _docFixedFont; }
  const OXFont *GetDocumentPropFont()  const { return _docPropFont; }
  const OXFont *GetIconFont()          const { return _iconFont; }
  const OXFont *GetStatusFont()        const { return _statusFont; }

  // GCs...

  const OXGC *GetWhiteGC()          const { return _whiteGC; }
  const OXGC *GetBlackGC()          const { return _blackGC; }

  const OXGC *GetFrameGC()          const { return _frameGC; }
  const OXGC *GetFrameBckgndGC()    const { return _bckgndGC; }
  const OXGC *GetFrameHiliteGC()    const { return _hiliteGC; }
  const OXGC *GetFrameShadowGC()    const { return _shadowGC; }
  const OXGC *GetFocusHiliteGC()    const { return _focusGC; }

  const OXGC *GetDocumentGC()       const { return _docGC; }
  const OXGC *GetDocumentBckgndGC() const { return _docbgndGC; }

  const OXGC *GetSelectedGC()       const { return _selGC; }
  const OXGC *GetSelectedBckgndGC() const { return _selbgndGC; }

  const OXGC *GetTipGC()            const { return _tipGC; }

  // Pixmaps...

  const Pixmap GetCheckeredPixmap() const { return _checkered; }
  const Pixmap GetCheckeredBitmap() const { return _checkered_bitmap; }

  const OPicture *GetFrameBckgndPicture() const 
        { return _defaultBackgroundPicture; }
  const OPicture *GetDocumentBckgndPicture() const 
        { return _defaultDocumentBackgroundPicture; }
  // default document background pixmap

  // Cursors...

  const Cursor GetDefaultCursor() const { return _defaultCursor; }
  const Cursor GetGrabCursor()    const { return _grabCursor; }
  const Cursor GetTextCursor()    const { return _textCursor; }
  const Cursor GetWaitCursor()    const { return _waitCursor; }

  // Colormaps...

  const Colormap GetDefaultColormap() const { return _defaultColormap; }

  // Other useful stuff that should be here...

  OMimeTypes *GetMimeTypes() const { return _MimeTypeList; }
  const char *GetSystemRoot() const { return _systemRoot; }
  const char *GetUserRoot() const { return _userRoot; }
  char *FindIniFile(const char *name, int mode) const;
  char *FindHelpFile(const char *name, const char *appname) const;

protected:
  const OXClient *_client;
  Display *_dpy;

  Colormap _defaultColormap;

  Pixel _frameBgnd, _frameFgnd, _hilite, _shadow;
  Pixel _docBgnd, _docFgnd;
  Pixel _tipBgnd, _tipFgnd;
  Pixel _selBgnd, _selFgnd;
  Pixel _white, _black;

  OFontPool *_fontPool;

  OXFont *_defaultFont;
  OXFont *_menuFont, *_menuHiFont;
  OXFont *_docFixedFont, *_docPropFont;
  OXFont *_iconFont;
  OXFont *_statusFont;

  const OPicture *_defaultBackgroundPicture;
  const OPicture *_defaultDocumentBackgroundPicture;

  OPicturePool *_globalPicPool, *_appPicPool;

  OGCPool *_gcPool;

  OXGC *_whiteGC, *_blackGC;
  OXGC *_frameGC, *_bckgndGC, *_hiliteGC, *_shadowGC, *_focusGC;
  OXGC *_docGC,   *_docbgndGC;
  OXGC *_selGC,   *_selbgndGC;
  OXGC *_tipGC;

  Cursor _defaultCursor, _grabCursor, _textCursor, _waitCursor;

  Pixmap _checkered, _checkered_bitmap;

  char *_systemRoot;
  char *_userRoot;
  char *_homeDir;
  OMimeTypes *_MimeTypeList;
};


#endif  // __ORESOURCEPOOL_H
