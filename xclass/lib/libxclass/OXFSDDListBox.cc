/**************************************************************************

    This file is part of xclass, a Win95-looking GUI toolkit.
    Copyright (C) 1996, 1997 David Barth, Hector Peraza.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

**************************************************************************/

#include <stdio.h>
#include <unistd.h>

#include <xclass/utils.h>
#include <xclass/ODimension.h>
#include <xclass/OXFSDDListBox.h>
#include <xclass/OXFont.h>
#include <xclass/OResourcePool.h>
#include <xclass/OGC.h>

#include "icons/folder.t.xpm"


//--- this is temp here...

struct _lbc lbc[] = {
  { "Root",        "/",                     "hdisk.t.xpm",         1000, 0, 0 },
  { "Floppy",      "/floppy",               "fdisk.t.xpm",         2000, 1, 0 },
  { "CD-ROM",      "/cdrom",                "cdrom.t.xpm",         3000, 1, 0 },
  { "Home",        "$HOME",                 "home.t.xpm",          4000, 1, 0 },
  { "Desktop",     "$HOME/.xclass/desktop", "desktop.t.xpm",       5000, 2, 0 },
  { "Recycle Bin", "$HOME/.xclass/recycle", "recycle-empty.t.xpm", 6000, 2, 0 },
  { NULL,          NULL,                    NULL,                  7000, 0, 0 }
};

unsigned long OXTreeLBEntry::_selPixel;
OXGC  *OXTreeLBEntry::_defaultGC;            // this is not shared.
const OXFont *OXTreeLBEntry::_defaultFont;
int   OXTreeLBEntry::_init = False;

//-------------------------------------------------------------------

OXTreeLBEntry::OXTreeLBEntry(const OXWindow *p,
      OString *text, const OPicture *pic, int ID, OString *path,
      unsigned int options, unsigned long back) :
  OXLBEntry(p, ID, options, back) {

    if (!_init) {
      _defaultFont = GetResourcePool()->GetDefaultFont();
      _defaultGC = new OXGC(GetResourcePool()->GetDocumentGC()); //==!==
      _defaultGC->SetFont(_defaultFont->GetId());
      _selPixel = GetResourcePool()->GetSelectedFgndColor();
      _init = True;
    }

    _pic = pic;
    _selpic = NULL;
    _text = text;
    _path = path;

    _normGC = _defaultGC;
    _font = _defaultFont;

    _active = False;

    OFontMetrics fm;
    _font->GetFontMetrics(&fm);
    _tw = _font->TextWidth(_text->GetString(), _text->GetLength());
    _th = _font->TextHeight();
    _ta = fm.ascent;
}

OXTreeLBEntry::~OXTreeLBEntry() {
  delete _text;
  delete _path;
  if (_normGC != _defaultGC) delete _normGC;
  if (_font != _defaultFont) _client->FreeFont((OXFont *) _font);
}

void OXTreeLBEntry::Activate(int a) {
  if (_active == a) return;
  _active = a;
  
  if (_active) {
    _selpic = new OSelectedPicture(_client, _pic);
  } else {
    if (_selpic) delete _selpic;
    _selpic = NULL;
  }

  _DoRedraw();
}

void OXTreeLBEntry::_DoRedraw() {
  int ix, iy, lx, ly;

  ix = 0;
  iy = (_h - _pic->GetHeight()) >> 1;
  lx = _pic->GetWidth() + 4;
  ly = (_h - (_th+1)) >> 1;

  if (_active) {
    if (_selpic) _selpic->Draw(GetDisplay(), _id, _normGC->GetGC(), ix, iy);
    _normGC->SetForeground(_defaultSelectedBackground);
    FillRectangle(_normGC->GetGC(), lx, ly, _tw+1, _th+1);
    _normGC->SetForeground(_selPixel);
  } else {
    _pic->Draw(GetDisplay(), _id, _normGC->GetGC(), ix, iy);
    _normGC->SetForeground(_backPixel);
    FillRectangle(_normGC->GetGC(), lx, ly, _tw+1, _th+1);
    _normGC->SetForeground(GetResourcePool()->GetDocumentFgndColor());  // ==!== doc sel fgnd?
  }

  _text->Draw(GetDisplay(), _id, _normGC->GetGC(), lx, ly + _ta);

  if (_focused) {
    _normGC->SetForeground(_backPixel);
    DrawRectangle(_normGC->GetGC(),
                  lx, ly, _tw, _th);
    DrawRectangle(GetResourcePool()->GetFocusHiliteGC()->GetGC(),
                  lx, ly, _tw, _th);
    _normGC->SetForeground(GetResourcePool()->GetDocumentFgndColor());  // ==!== doc sel fgnd?
  }
}

ODimension OXTreeLBEntry::GetDefaultSize() const {
  ODimension isize(_pic->GetWidth(), _pic->GetHeight());
  ODimension lsize(_tw, _th+1);

  return ODimension(isize.w + lsize.w + 5, max(isize.h, lsize.h) + 2);
}

void OXTreeLBEntry::UpdateEntry(OXLBEntry *e) {
  OXTreeLBEntry *te = (OXTreeLBEntry *) e;

  _text = new OString(te->GetText());
  _pic = te->GetPicture();
  _tw = _font->TextWidth(_text->GetString(), _text->GetLength());

  if (_active) {
    if (_selpic) delete _selpic;
    _selpic = new OSelectedPicture(_client, _pic);
  }

  ClearWindow();  // need this because _DoRedraw does not call OXFrame::_DoRedraw()
  NeedRedraw();
}


//-------------------------------------------------------------------

OXFileSystemDDListBox::OXFileSystemDDListBox(const OXWindow *parent,
              int ID, unsigned int options, unsigned long back) :
  OXDDListBox(parent, ID, options | OWN_BKGND, back) {
    int   i, indent;
    const OPicture *pic;
    char  *p;

    pic = _client->GetPicture("folder.t.xpm");
    if (!pic)
      pic = _client->GetPicture("folder.t.xpm", XCP_folder_t_xpm);
    if (!pic)
      FatalError("OXFSDDListBox: Pixmap not found: folder.t.xpm");

    SetTopEntry(new OXTreeLBEntry(this, new OString("Current dir"), pic, 0),
                new OLayoutHints(LHINTS_LEFT | LHINTS_EXPAND_X |
                         LHINTS_EXPAND_Y, 4, 0, 0, 0));

    ((OXFrame *)_lb->GetContainer())->AddInput(ButtonPressMask |
                                               ButtonReleaseMask |
                                               PointerMotionMask);

    //--- first check for the existence of some directories...

    char *HomeDir = getenv("HOME");

    for (i = 0; lbc[i].path != NULL; ++i) {
      if (strstr(lbc[i].path, "$HOME") != NULL) {
        if (HomeDir) {
          int hlen = strlen(HomeDir);
          p = new char[hlen + strlen(lbc[i].path) - 3];
          strcpy(p, HomeDir);
          strcat(p, &lbc[i].path[5]);
          lbc[i].path = p;
        } else {
          lbc[i].flags = 0;
        }
      }
      if (access(lbc[i].path, F_OK) == 0) lbc[i].flags = 1;
    }

    //--- then init the contents...

    for (i = 0; lbc[i].name != NULL; ++i) {
      if (lbc[i].flags) {
        indent = 4 + (lbc[i].indent * 10);
        pic = _client->GetPicture(lbc[i].pixmap);
        if (!pic)
          pic = _client->GetPicture("folder.t.xpm");
        if (!pic)
          pic = _client->GetPicture("folder.t.xpm", XCP_folder_t_xpm);
        if (!pic)
          FatalError("OXFSDDListBox: pixmap not found: %s", lbc[i].pixmap);
        AddEntry(new OXTreeLBEntry(_lb->GetContainer(),
                       new OString(lbc[i].name), pic, lbc[i].ID,
                       new OString(lbc[i].path)),
                 new OLayoutHints(LHINTS_LEFT | LHINTS_TOP,
                       indent, 0, 0, 0));
      }
    }

}

void OXFileSystemDDListBox::UpdateContents(char *path) {
  char dirname[PATH_MAX], mpath[PATH_MAX],
       *tailpath = NULL;
  int  i, indent_lvl, afterID, sel = -1; 
  
  if (!path) return;

  for (i = 0; lbc[i].path != NULL; ++i)
    RemoveEntries(lbc[i].ID+1, lbc[i+1].ID-1);

  int len = 0;
  for (i = 0; lbc[i].name != NULL; ++i) {
    if (lbc[i].flags) {
      int slen = strlen(lbc[i].path);
      if (strncmp(path, lbc[i].path, slen) == 0) {
        if (slen > len) {
          sel = afterID = lbc[i].ID;
          indent_lvl = lbc[i].indent + 1;
          tailpath = path + slen;
          strcpy(mpath, lbc[i].path);
          len = slen;
        }
      }
    }
  }

  if (tailpath && *tailpath) {
    if (*tailpath == '/') ++tailpath;
    if (*tailpath)
      while (1) {
        char *picname; 
        char *semi = strchr(tailpath, '/');
        if (semi == NULL) {
          strcpy(dirname, tailpath);
          picname = "ofolder.t.xpm";
        } else {
          strncpy(dirname, tailpath, semi-tailpath);
          dirname[semi-tailpath] = 0;
          picname = "folder.t.xpm";
        }
        if (mpath[strlen(mpath)-1] != '/') strcat(mpath, "/");
        strcat(mpath, dirname);
        int indent = 4 + (indent_lvl * 10);
        const OPicture *pic = _client->GetPicture(picname);
        if (!pic)
          pic = _client->GetPicture("folder.t.xpm");
        if (!pic)
          pic = _client->GetPicture("folder.t.xpm", XCP_folder_t_xpm);
        if (!pic)
          FatalError("OXFSDDListBox: pixmap not found: %s", picname);
        InsertEntry(new OXTreeLBEntry(_lb->GetContainer(),
                            new OString(dirname), pic, afterID+1,
                            new OString(mpath)),
                    new OLayoutHints(LHINTS_LEFT | LHINTS_TOP,
                                     indent, 0, 0, 0),
                    afterID);
        sel = ++afterID;
        ++indent_lvl;
        if (semi == NULL) break;
        tailpath = ++semi;
      }
  }

  if (sel > 0) Select(sel);
}
