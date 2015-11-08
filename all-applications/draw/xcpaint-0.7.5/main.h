/**************************************************************************

    This file is part of xcpaint, a XPM pixmap editor.
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

#ifndef __MAIN_H
#define __MAIN_H

#include <xclass/OXClient.h>
#include <xclass/OXWindow.h>
#include <xclass/OXMainFrame.h>
#include <xclass/OXStatusBar.h>
#include <xclass/OXButton.h>
#include <xclass/OXMenu.h>
#include <xclass/OXCanvas.h>
#include <xclass/OXMsgBox.h>
#include <xclass/OXAboutDialog.h>
#include <xclass/OXFontDialog.h>
#include <xclass/OMimeTypes.h>
#include <xclass/OXFileDialog.h>
#include <xclass/OString.h>
#include <xclass/utils.h>


#define MENU_DISABLED     (1<<0)
#define MENU_CHECKED      (1<<1)
#define MENU_RCHECKED     (1<<2)


#define M_FILE_NEW         101
#define M_FILE_OPEN        102
#define M_FILE_SAVE        103
#define M_FILE_SAVEAS      104
#define M_FILE_PRINT       105
#define M_FILE_PRINTSETUP  106
#define M_FILE_EXIT        107

#define M_EDIT_UNDO        201
#define M_EDIT_CUT         202
#define M_EDIT_COPY        203
#define M_EDIT_PASTE       204
#define M_EDIT_SELECTALL   205

#define M_VIEW_TOOLBAR     301
#define M_VIEW_STATUSBAR   302
#define M_VIEW_PALETTE     303
#define M_VIEW_ZOOMIN      304
#define M_VIEW_ZOOMOUT     305
#define M_VIEW_GRID        306

#define M_IMAGE_RESIZE     401
#define M_IMAGE_RESCALE    402
#define M_IMAGE_ROTATE     403
#define M_IMAGE_FLIP       404
#define M_IMAGE_SHIFT      405
#define M_IMAGE_SETFONT    406
#define M_IMAGE_SETTEXT    407

#define M_FLIP_HORIZONTAL      411
#define M_FLIP_VERTICAL        412

#define M_ROTATE_LEFT          421
#define M_ROTATE_RIGHT         422

#define M_SHIFT_UP             431
#define M_SHIFT_DOWN           432
#define M_SHIFT_LEFT           433
#define M_SHIFT_RIGHT          434

#define M_PALETTE_LOAD     501
#define M_PALETTE_SAVE     502
#define M_PALETTE_ADDCOLOR 503

#define M_HELP_CONTENTS    601
#define M_HELP_SEARCH      602
#define M_HELP_ABOUT       603

#define MISC_DDLB_SELECT   2001

#define ZOOM_MIN           1
#define ZOOM_MAX           10
#define ZOOM_GRID_BOUND1   5
#define ZOOM_GRID_BOUND2   4


struct _popup {
  OXPopupMenu *ptr;
  struct {
    char *name;
    int  id, state;
    struct _popup *popup_ref;
  } popup[12];
};

struct _tb_data {
  char *pixmap_name;
  char **pixmap_data;
  char *tip_text;
  int  type;
  int  id;
  OXButton *button;
};


//---------------------------------------------------------------------

class OXAppMainFrame : public OXMainFrame {
public:
  OXAppMainFrame(const OXWindow *p, int w, int h);
  virtual ~OXAppMainFrame();

  virtual int ProcessMessage(OMessage *msg);
  virtual int HandleKey(XKeyEvent * event);
  virtual int CloseWindow();

  void SetWindowTitle(char *title);
  void UpdateStatus();

  void LoadFile(char *);
  void DoNew();
  void DoLoad();
  void DoSave();
  int  DoSaveAs();
  int  SaveIfChanged();
  void DoCut();
  void DoCopy();
  void DoPaste();
  void DoSetMode(int mode);
  void DoZoomIn();
  void DoZoomOut();
  void DoZoomSet();
  void DoResize();
  void DoRescale();
  void DoSetFont();
  void DoSetText();
  void DoAddColor();
  void DoSetColor(unsigned long color);
  void DoToggleGrid();
  void DoToggleToolBar();
  void DoToggleStatusBar();
  void DoTogglePalette();
  void DoHelpAbout();

protected:
  void _InitToolBar();
  OXPopupMenu *_MakePopup(struct _popup *);

  OXCompositeFrame *_vf1, *_f1, *_f2, *_toolBar, *_container;
  OXStatusBar *_statusBar;
  OXFrame *_currentColor;
  OXCanvas *_canvas;
  OXPaintCanvas *_pcanvas;
  OXPaletteFrame *_palette;

  OLayoutHints *_menuBarLayout, *_menuBarItemLayout;

  OXMenuBar *_menuBar;
  OXPopupMenu *_menuFile, *_menuEdit, *_menuView,
              *_menuImage, *_menuHelp, *_menuPalette,
              *_rotatePopup, *_flipPopup, *_shiftPopup;

  OXDDListBox *_ddlb;

  const OPicture *_cbkgnd;
  int _exiting;
};


#endif  // __MAIN_H
