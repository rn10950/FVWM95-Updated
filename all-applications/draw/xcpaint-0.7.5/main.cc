/**************************************************************************

    This file is part of xcpaint, a XPM pixmap editor.
    Copyright (C) 1996-2004 David Barth, Hector Peraza.

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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <X11/keysym.h>

#include <xclass/OXMsgBox.h>
#include <xclass/OXColorDialog.h>

#include "OColorTable.h"
#include "OXDialogs.h"
#include "OXPaintCanvas.h"
#include "OXPaletteFrame.h"
#include "main.h"

#include "ptb-point.xpm"
#include "ptb-mark.xpm"
#include "ptb-rect.xpm"
#include "ptb-frect.xpm"
#include "ptb-circle.xpm"
#include "ptb-fcircle.xpm"
#include "ptb-text.xpm"
#include "ptb-ffillb.xpm"
#include "ptb-line.xpm"
#include "ptb-curve.xpm"
#include "ptb-zoomin.xpm"
#include "ptb-zoomout.xpm"

#include "pbg.xpm"

#define XCPAINT_VERSION "0.7.5"

#ifndef max
#define max(a,b) (((a)>(b)) ? (a) : (b))
#endif

struct _popup file_popup = {
  NULL, {
  { "&New...",         M_FILE_NEW,        0,             NULL },
  { "",                -1,                0,             NULL },
  { "&Open...",        M_FILE_OPEN,       0,             NULL },
  { "&Save",           M_FILE_SAVE,       MENU_DISABLED, NULL },
  { "Save &as...",     M_FILE_SAVEAS,     0,             NULL },
  { "",                -1,                0,             NULL },
  { "&Print",          M_FILE_PRINT,      MENU_DISABLED, NULL },
  { "P&rint setup...", M_FILE_PRINTSETUP, MENU_DISABLED, NULL },
  { "",                -1,                0,             NULL },
  { "E&xit",           M_FILE_EXIT,       0,             NULL },
  { NULL,              -1,                0,             NULL } } };

struct _popup edit_popup = {
  NULL, {
  { "&Undo",           M_EDIT_UNDO,       0,             NULL },
  { "",                -1,                0,             NULL },
  { "Cu&t",            M_EDIT_CUT,        0,             NULL },
  { "&Copy",           M_EDIT_COPY,       0,             NULL },
  { "&Paste",          M_EDIT_PASTE,      0,             NULL },
  { "",                -1,                0,             NULL },
  { "Select &all",     M_EDIT_SELECTALL,  0,             NULL },
  { NULL,              -1,                0,             NULL } } };

struct _popup view_popup = {
  NULL, {
  { "&Toolbar",        M_VIEW_TOOLBAR,    MENU_CHECKED,  NULL },
  { "Status &Bar",     M_VIEW_STATUSBAR,  MENU_CHECKED,  NULL },
  { "Color &Palette",  M_VIEW_PALETTE,    MENU_CHECKED,  NULL },
  { "",                -1,                0,             NULL },
  { "&Grid",           M_VIEW_GRID,       MENU_CHECKED | MENU_DISABLED, NULL },
  { "",                -1,                0,             NULL },
  { "Zoom &In",        M_VIEW_ZOOMIN,     0,             NULL },
  { "Zoom &Out",       M_VIEW_ZOOMOUT,    MENU_DISABLED, NULL },
  { NULL,              -1,                0,             NULL } } };

struct _popup rotate_popup = {
  NULL, {
  { "90 Deg &Right",   M_ROTATE_RIGHT,    0,             NULL },
  { "90 Deg &Left",    M_ROTATE_LEFT,     0,             NULL },
  { NULL,              -1,                0,             NULL } } };

struct _popup flip_popup = {
  NULL, {
  { "&Horizontal",     M_FLIP_HORIZONTAL, 0,             NULL },
  { "&Vertical",       M_FLIP_VERTICAL,   0,             NULL },
  { NULL,              -1,                0,             NULL } } };

struct _popup shift_popup = {
  NULL, {
  { "&Up",             M_SHIFT_UP,        0,             NULL },
  { "&Down",           M_SHIFT_DOWN,      0,             NULL },
  { "&Left",           M_SHIFT_LEFT,      0,             NULL },
  { "&Right",          M_SHIFT_RIGHT,     0,             NULL },
  { NULL,              -1,                0,             NULL } } };

struct _popup image_popup = {
  NULL, {
  { "&Resize...",      M_IMAGE_RESIZE,    0,             NULL },
  { "Re&scale...",     M_IMAGE_RESCALE,   0,             NULL },
  { "",                -1,                0,             NULL },
  { "R&otate",         M_IMAGE_ROTATE,    0,             &rotate_popup },
  { "&Flip",           M_IMAGE_FLIP,      0,             &flip_popup },
  { "S&hift",          M_IMAGE_SHIFT,     0,             &shift_popup },
  { "",                -1,                0,             NULL },
  { "Set Fo&nt...",    M_IMAGE_SETFONT,   0,             NULL },
  { "Set &Text...",    M_IMAGE_SETTEXT,   0,             NULL },
  { NULL,              -1,                0,             NULL } } };

struct _popup palette_popup = {
  NULL, {
  { "&Load...",        M_PALETTE_LOAD,    0,             NULL },
  { "&Save...",        M_PALETTE_SAVE,    0,             NULL },
  { "",                -1,                0,             NULL },
  { "&Add color...",   M_PALETTE_ADDCOLOR,0,            NULL },
  { NULL,              -1,                0,             NULL } } };

struct _popup help_popup = {
  NULL, {
  { "&Contents...",    M_HELP_CONTENTS,   MENU_DISABLED, NULL },
  { "&Search...",      M_HELP_SEARCH,     MENU_DISABLED, NULL },
  { "",                -1,                0,             NULL },
  { "&About...",       M_HELP_ABOUT,      0,             NULL },
  { NULL,              -1,                0,             NULL } } };

struct _tb_data tb_data[] = {
  { "ptb-point.xpm",   ptb_point_xpm,   "Draw",             BUTTON_STAYDOWN, 1001, NULL },
  { "ptb-mark.xpm",    ptb_mark_xpm,    "Mark region",      BUTTON_STAYDOWN, 1002, NULL },
  { "ptb-line.xpm",    ptb_line_xpm,    "Line",             BUTTON_STAYDOWN, 1007, NULL },
  { "ptb-curve.xpm",   ptb_curve_xpm,   "Curve",            BUTTON_STAYDOWN, 1008, NULL },
  { "ptb-rect.xpm",    ptb_rect_xpm,    "Rectangle",        BUTTON_STAYDOWN, 1003, NULL },
  { "ptb-frect.xpm",   ptb_frect_xpm,   "Filled rectangle", BUTTON_STAYDOWN, 1004, NULL },
  { "ptb-circle.xpm",  ptb_circle_xpm,  "Circle",           BUTTON_STAYDOWN, 1005, NULL },
  { "ptb-fcircle.xpm", ptb_fcircle_xpm, "Filled circle",    BUTTON_STAYDOWN, 1006, NULL },
  { "ptb-ffillb.xpm",  ptb_ffillb_xpm,  "Flood fill",       BUTTON_STAYDOWN, 1009, NULL },
  { "ptb-text.xpm",    ptb_text_xpm,    "Text",             BUTTON_STAYDOWN, 1010, NULL },
  { "ptb-zoomin.xpm",  ptb_zoomin_xpm,  "Zoom In",          BUTTON_NORMAL,   1011, NULL },
  { "ptb-zoomout.xpm", ptb_zoomout_xpm, "Zoom Out",         BUTTON_NORMAL,   1012, NULL },
  { NULL,              NULL,            NULL,               0,               0,    NULL }
};

char *filetypes[] = { "All files",           "*",
                      "Bitmap files",        "*.xbm",
                      "Pixmap files",        "*.xpm",
                      "Bitmaps and Pixmaps", "*.x?m",
                      NULL,                  NULL };


//---------------------------------------------------------------------

OXClient *clientX;
OXAppMainFrame *mainWindow;

int Grid, ZoomFactor;

int main(int argc, char *argv[]) {

  clientX = new OXClient(argc, argv);

  mainWindow = new OXAppMainFrame(clientX->GetRoot(), 400, 200);
  if (argc > 1) mainWindow->LoadFile(argv[1]);
  mainWindow->MapWindow();

  clientX->Run();

  return 0;
}

OXAppMainFrame::OXAppMainFrame(const OXWindow *p, int w, int h) :
  OXMainFrame(p, w, h) {
  char tmp[BUFSIZ];
  int i;

  _exiting = False;

  _menuBarLayout = new OLayoutHints(LHINTS_TOP | LHINTS_LEFT | LHINTS_EXPAND_X, 0, 0, 1, 1);
  _menuBarItemLayout = new OLayoutHints(LHINTS_TOP | LHINTS_LEFT, 0, 4, 0, 0);

  _rotatePopup = _MakePopup(&rotate_popup);
  _flipPopup   = _MakePopup(&flip_popup);
  _shiftPopup  = _MakePopup(&shift_popup);

  _menuFile    = _MakePopup(&file_popup);
  _menuEdit    = _MakePopup(&edit_popup);
  _menuView    = _MakePopup(&view_popup);
  _menuImage   = _MakePopup(&image_popup);
  _menuPalette = _MakePopup(&palette_popup);
  _menuHelp    = _MakePopup(&help_popup);

  _menuFile->Associate(this);
  _menuEdit->Associate(this);
  _menuView->Associate(this);
  _menuImage->Associate(this);
  _menuPalette->Associate(this);
  _menuHelp->Associate(this);
  _rotatePopup->Associate(this);
  _flipPopup->Associate(this);
  _shiftPopup->Associate(this);


  //------ Menu bar

  _menuBar = new OXMenuBar(this, 1, 1, HORIZONTAL_FRAME);
  _menuBar->AddPopup(new OHotString("&File"), _menuFile, _menuBarItemLayout);
  _menuBar->AddPopup(new OHotString("&Edit"), _menuEdit, _menuBarItemLayout);
  _menuBar->AddPopup(new OHotString("&View"), _menuView, _menuBarItemLayout);
  _menuBar->AddPopup(new OHotString("&Image"), _menuImage, _menuBarItemLayout);
  _menuBar->AddPopup(new OHotString("&Palette"), _menuPalette, _menuBarItemLayout);
  _menuBar->AddPopup(new OHotString("&Help"), _menuHelp, _menuBarItemLayout);

  AddFrame(_menuBar, _menuBarLayout);


  //------ _f1: horizontal frame containing _vf1 (_toolBar + _ddlb) and _canvas

  _f1 = new OXCompositeFrame(this, 10, 10, HORIZONTAL_FRAME);
  _vf1 = new OXVerticalFrame(_f1, 0, 0, 0);

  //-------------- ListBox
 
  _ddlb = new OXDDListBox(_vf1, MISC_DDLB_SELECT);
 
  _vf1->AddFrame(_ddlb, new OLayoutHints(LHINTS_LEFT | LHINTS_EXPAND_X,
                                         0, 3, 2, 2));
//  _ddlb->Resize(32, _ddlb->GetDefaultHeight());
  _ddlb->Resize(30, 20);

  for (i=1; i<11; i++) {
    sprintf(tmp, "1:%d", i);
    _ddlb->AddEntry(new OString(tmp), i);
  }
  _ddlb->Select(ZOOM_MIN);
  _ddlb->Associate(this);

  _toolBar = new OXCompositeFrame(_vf1, 60, 20, VERTICAL_FRAME);
  _vf1->AddFrame(_toolBar, new OLayoutHints(LHINTS_LEFT | LHINTS_EXPAND_Y,
                                            0, 3, 1, 1));

  _toolBar->SetLayoutManager(new OMatrixLayout(_toolBar, 0, 2, 0, 0));

  _InitToolBar();

  _f1->AddFrame(_vf1, new OLayoutHints(LHINTS_LEFT | LHINTS_TOP));

  _cbkgnd = _client->GetPicture("pbg.xpm", pbg_xpm);
  if (!_cbkgnd) FatalError("Bad or missing internal pixmap");

  _canvas = new OXCanvas(_f1, 500, 300);
  _container = new OXCompositeFrame(_canvas->GetViewPort(), 10, 10, 
                                    HORIZONTAL_FRAME | OWN_BKGND);
  _container->SetBackgroundPixmap(_cbkgnd->GetPicture());
  _canvas->SetContainer(_container);
  _canvas->SetScrollDelay(30, 20);
  _canvas->SetScrollMode(SB_ACCELERATED);

  _pcanvas = new OXPaintCanvas(_container, 10, 10, 
                               HORIZONTAL_FRAME | OWN_BKGND,
                               _whitePixel);
  _container->AddFrame(_pcanvas, new OLayoutHints(LHINTS_LEFT | LHINTS_TOP));

  _pcanvas->Associate(this);
  _pcanvas->SetZoom(ZoomFactor = 1);
  _pcanvas->SetGrid(Grid = True);

  _f1->AddFrame(_canvas, new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y,
                                     0, 0, 0, 0));

  AddFrame(_f1, new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y));


  //------ Status bar

  _statusBar = new OXStatusBar(this);
  AddFrame(_statusBar, new OLayoutHints(LHINTS_BOTTOM | LHINTS_EXPAND_X,
                                        0, 0, 2, 0));

  _statusBar->SetText(0, new OString("640 x 480"));


  //------ _f2: horizontal frame containing _currentColor and _palette

  _f2 = new OXCompositeFrame(this, 10, 10, HORIZONTAL_FRAME);

  _currentColor = new OXFrame(_f2, 26, 26,
                              SUNKEN_FRAME | FIXED_WIDTH | OWN_BKGND);
  _f2->AddFrame(_currentColor, new OLayoutHints(LHINTS_LEFT | LHINTS_EXPAND_Y));

  _palette = new OXPaletteFrame(_f2, 10, 10, HORIZONTAL_FRAME);
  _f2->AddFrame(_palette, new OLayoutHints(LHINTS_LEFT | LHINTS_EXPAND_X,
                                           3, 0, 0, 0));

  AddFrame(_f2, new OLayoutHints(LHINTS_BOTTOM | LHINTS_EXPAND_X,
                                 0, 0, 2, 1));

  UpdateStatus();
  _palette->UpdateColors(_pcanvas->GetColorTable());
  _palette->Associate(this);

  DoSetColor(_palette->GetColorByIndex(0));

  SetWindowTitle("Untitled.xpm");
  _pcanvas->ChangeFilename("Untitled.xpm");

  SetIconName("Paint");
  SetClassHints("XCPaint", "xpaint");
  MapSubwindows();

  Resize(GetDefaultSize());

  AddInput(KeyPressMask | KeyReleaseMask);
}

OXAppMainFrame::~OXAppMainFrame() {

  delete _menuBarLayout;
  delete _menuBarItemLayout;

  delete _menuFile;
  delete _menuEdit;
  delete _menuView;
  delete _menuImage;
  delete _menuPalette;
  delete _menuHelp;
  delete _rotatePopup;
  delete _flipPopup;
  delete _shiftPopup;

}

int OXAppMainFrame::CloseWindow() {
  if (_exiting) {
    XBell(GetDisplay(), 0);
    return False;
  }
  _exiting = True;
  if (SaveIfChanged() == ID_CANCEL) {
    _exiting = False;
    return False;
  }
  return OXMainFrame::CloseWindow();
}

OXPopupMenu *OXAppMainFrame::_MakePopup(struct _popup *p) {

  OXPopupMenu *popup = new OXPopupMenu(_client->GetRoot());

  for (int i=0; p->popup[i].name != NULL; ++i) {
    if (strlen(p->popup[i].name) == 0) {
      popup->AddSeparator();
    } else {
      if (p->popup[i].popup_ref == NULL) {
        popup->AddEntry(new OHotString(p->popup[i].name), p->popup[i].id);
      } else {
        struct _popup *p1 = p->popup[i].popup_ref;
        popup->AddPopup(new OHotString(p->popup[i].name), p1->ptr);
      }
      if (p->popup[i].state & MENU_DISABLED) popup->DisableEntry(p->popup[i].id);
      if (p->popup[i].state & MENU_CHECKED) popup->CheckEntry(p->popup[i].id);
      if (p->popup[i].state & MENU_RCHECKED) popup->RCheckEntry(p->popup[i].id,
                                                                p->popup[i].id,
                                                                p->popup[i].id);
    }
  }
  p->ptr = popup;

  return popup;
}

void OXAppMainFrame::_InitToolBar() {
  int i;
  OXPictureButton *button;
  const OPicture *buttonpic;    

  for (i=0; tb_data[i].pixmap_name != NULL; ++i) {
    buttonpic = _client->GetPicture(tb_data[i].pixmap_name,
                                    tb_data[i].pixmap_data);
    if (!buttonpic)
      FatalError("Bad or missing inlined pixmap: %s", tb_data[i].pixmap_name);

    button = new OXPictureButton(_toolBar, buttonpic, tb_data[i].id);
    _toolBar->AddFrame(button, NULL);
    button->Associate(this);

    button->SetType(tb_data[i].type);
    button->SetTip(tb_data[i].tip_text);
    tb_data[i].button = button;
  }
  tb_data[0].button->SetState(BUTTON_ENGAGED);
}

int OXAppMainFrame::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;

  switch (msg->type) {

    case MSG_BUTTON:
      if (msg->action == MSG_CLICK) {
        if (wmsg->id == 1011)
          DoZoomIn();
        else if (wmsg->id == 1012)
          DoZoomOut();
        else
          DoSetMode(wmsg->id);
        break;
      }
      break;

    case MSG_DDLISTBOX:
      if (msg->action == MSG_CLICK) {
        if (wmsg->id == MISC_DDLB_SELECT) {
          DoZoomSet();
        }
      }
      break;

    case MSG_PALETTE:
      if (msg->action == MSG_CLICK) {
        DoSetColor(wmsg->id);
      }
      break;

    case MSG_MENU:
      if (msg->action == MSG_CLICK) {
        switch (wmsg->id) {

          //--------------------------------------------- File

          case M_FILE_NEW:
            DoNew();
            break;

          case M_FILE_OPEN:
            DoLoad();
            break;

          case M_FILE_SAVE:
            DoSave();
            break;

          case M_FILE_SAVEAS:
            DoSaveAs();
            break;

          case M_FILE_EXIT:
            CloseWindow();
            break;

          //--------------------------------------------- Edit

          case M_EDIT_UNDO:
            _pcanvas->Undo();
            break;

          case M_EDIT_CUT:
            DoCut();
            break;

          case M_EDIT_COPY:
            DoCopy();
            break;

          case M_EDIT_PASTE:
            DoPaste();
            break;

          case M_EDIT_SELECTALL:
            _pcanvas->Select(0, 0,
                             _pcanvas->GetImageWidth() - 1,
                             _pcanvas->GetImageHeight() - 1,
                             CurrentTime);
            break;

          //--------------------------------------------- View

          case M_VIEW_TOOLBAR:
            DoToggleToolBar();
            break;

          case M_VIEW_STATUSBAR:
            DoToggleStatusBar();
            break;

          case M_VIEW_PALETTE:
            DoTogglePalette();
            break;

          case M_VIEW_GRID:
            DoToggleGrid();
            break;

          case M_VIEW_ZOOMIN:
            DoZoomIn();
            break;

          case M_VIEW_ZOOMOUT:
            DoZoomOut();
            break;

          //--------------------------------------------- Image

          case M_FLIP_HORIZONTAL:
            _pcanvas->FlipVert();
            break;

          case M_FLIP_VERTICAL:
            _pcanvas->FlipHoriz();
            break;

          case M_ROTATE_LEFT:
            _pcanvas->RotateLeft();
            break;

          case M_ROTATE_RIGHT:
            _pcanvas->RotateRight();
            break;

          case M_SHIFT_UP:
            _pcanvas->ShiftUp();
            break;

          case M_SHIFT_DOWN:
            _pcanvas->ShiftDown();
            break;

          case M_SHIFT_LEFT:
            _pcanvas->ShiftLeft();
            break;

          case M_SHIFT_RIGHT:
            _pcanvas->ShiftRight();
            break;

          case M_IMAGE_RESIZE:
            DoResize();
            break;

          case M_IMAGE_RESCALE:
            DoRescale();
            break;

          case M_IMAGE_SETFONT:
            DoSetFont();
            break;

          case M_IMAGE_SETTEXT:
            DoSetText();
            break;

          //--------------------------------------------- Palette

          case M_PALETTE_ADDCOLOR:
            DoAddColor();
            break;

          //--------------------------------------------- Help

          case M_HELP_CONTENTS:
            break;

          case M_HELP_ABOUT:
            DoHelpAbout();
            break;

          default:
            break;
        }
      }
      break;

    case MSG_PAINTCANVAS:
      if (msg->action == MSG_IMAGEPASTED) {
        _palette->UpdateColors(_pcanvas->GetColorTable());
      }
      break;

    default:
      break;

  }

  return True;
}

int OXAppMainFrame::HandleKey(XKeyEvent *event) {
  char tmp[10];
  KeySym keysym;
  XComposeStatus compose = { NULL, 0 };

  if (OXMainFrame::HandleKey(event)) return True;

  if (event->type == KeyPress) {
    XLookupString(event, tmp, sizeof(tmp)-1, &keysym, &compose);
    switch (keysym) {
/*
      case XK_Left:  MoveWest();  break;
      case XK_Right: MoveEast();  break;
      case XK_Up:    MoveNorth(); break;
      case XK_Down:  MoveSouth(); break;
*/
      default:
        if (tmp[0] == 'i' || tmp[0] == 'I')
          DoZoomIn();
        else if (tmp[0] == 'o' || tmp[0] == 'O')
          DoZoomOut();
        else if (tmp[0] == 'g' || tmp[0] == 'G')
          DoToggleGrid();
        break;        
    }
  }

  return True;
}


void OXAppMainFrame::SetWindowTitle(char *title) {
  char *wname = new char[strlen(title) + 20];
  sprintf(wname, "Pixmap Editor - %s", title);
  SetWindowName(wname);
  delete wname;
}


void OXAppMainFrame::LoadFile(char *fname) {
  int  retc;
  char *msg;

  retc = _pcanvas->ReadFile(fname);
  if (retc >= XpmSuccess) {
    _canvas->Layout();
    SetWindowTitle(fname);
    UpdateStatus();
    _palette->UpdateColors(_pcanvas->GetColorTable());
    _menuFile->EnableEntry(M_FILE_SAVE);
  } else {
    OString stitle("Pixmap Editor - Load Failed");
    switch (retc) {
      default:
      case XpmOpenFailed:
      case XpmFileInvalid:
        msg = "Missing or bad pixmap file.";
        break;

      case XpmColorFailed:
        msg = "Could not allocate colors.";
        break;

      case XpmNoMemory:
        msg = "Not enough memory.";
        break;
    }
    OString smsg(msg);
    new OXMsgBox(_client->GetRoot(), this, &stitle, new OString(&smsg),
                                     MB_ICONSTOP, ID_OK);
  }
}


void OXAppMainFrame::DoLoad() {
  int retc;
  char *msg;
  OFileInfo fi;

  // check first for unsaved changes.
  if (SaveIfChanged() == ID_CANCEL) return;

  fi.MimeTypesList = NULL;
  fi.file_types = filetypes;
  new OXFileDialog(_client->GetRoot(), this, FDLG_OPEN, &fi);
  if (fi.filename) {
    retc = _pcanvas->ReadFile(fi.filename);
    if (retc >= XpmSuccess) {
      _canvas->Layout();
      SetWindowTitle(fi.filename);
      UpdateStatus();
      _palette->UpdateColors(_pcanvas->GetColorTable());
      _menuFile->EnableEntry(M_FILE_SAVE);
    } else {
      OString stitle("Load Failed");
      switch (retc) {
        default:
        case XpmOpenFailed:
        case XpmFileInvalid:
          msg = "Missing or bad pixmap file.";
          break;

        case XpmColorFailed:
          msg = "Could not allocate colors.";
          break;

        case XpmNoMemory:
          msg = "Not enough memory.";
          break;
      }
      OString smsg(msg);
      new OXMsgBox(_client->GetRoot(), this, &stitle, new OString(&smsg),
                                       MB_ICONSTOP, ID_OK);
    }
  }
}


void OXAppMainFrame::DoNew() {

  // check first for unsaved changes.
  if (SaveIfChanged() == ID_CANCEL) return;

  int newW = _pcanvas->GetImageWidth();
  int newH = _pcanvas->GetImageHeight();

  new OXSizeDialog(_client->GetRoot(), this, 10, 10, 0,
                   DIALOG_NEW_IMAGE, &newW, &newH);
  if (newW == -1) return;
  newW = max(newW, 1);
  newH = max(newH, 1);
  _pcanvas->ResizeImage(newW, newH);
  _pcanvas->ClearAll();
  _pcanvas->ClearHotSpot();
  _pcanvas->ClearChanged();
  _pcanvas->ChangeFilename("Untitled.xpm");
  _canvas->Layout();
  SetWindowTitle("Untitled.xpm");
  UpdateStatus();
  _menuFile->DisableEntry(M_FILE_SAVE);
}


void OXAppMainFrame::DoSave() {
  int retc;
  OFileInfo fi;

  while (_pcanvas->WriteFile(NULL) != XpmSuccess) {
    OString stitle("Save Failed");
    OString smsg("Can't write pixmap file.");
    new OXMsgBox(_client->GetRoot(), this, &stitle, new OString(&smsg),
                                 MB_ICONSTOP, ID_RETRY | ID_CANCEL, &retc);
    if (retc == ID_CANCEL) break;
  }
}


int OXAppMainFrame::DoSaveAs() {
  int retc;
  OFileInfo fi;

  fi.MimeTypesList = NULL;
  fi.file_types = filetypes;
  new OXFileDialog(_client->GetRoot(), this, FDLG_SAVE, &fi);
  if (fi.filename) {

    // check whether the file already exists, and ask
    // permission to overwrite if so.

    if (access(fi.filename, F_OK) == 0) {
      new OXMsgBox(_client->GetRoot(), this,
            new OString("Save"),
            new OString("A file with the same name already exists. Overwrite?"),
            MB_ICONQUESTION, ID_YES | ID_NO, &retc);
      if (retc == ID_NO) return ID_CANCEL;
    }

    if (_pcanvas->WriteFile(fi.filename) != XpmSuccess) {
      OString stitle("Save Failed");
      OString smsg("Can't write pixmap file.");
      new OXMsgBox(_client->GetRoot(), this, &stitle, new OString(&smsg),
                               MB_ICONSTOP, ID_OK);
      return ID_CANCEL;
    }

    SetWindowTitle(fi.filename);
    UpdateStatus();
    _menuFile->EnableEntry(M_FILE_SAVE);

    return ID_YES;
  } else {

    // user cancelled file dialog
    return ID_CANCEL;
  }
}


int OXAppMainFrame::SaveIfChanged() {
  int retc = ID_YES;

  if (_pcanvas->HasChanged()) {

    OString stitle(_pcanvas->GetFilename());
    OString smsg("There are unsaved changes. Save current image first?");
    new OXMsgBox(_client->GetRoot(), this, &stitle, new OString(&smsg),
                                     MB_ICONEXCLAMATION,
                                     ID_YES | ID_NO | ID_CANCEL, &retc);
    if (retc == ID_YES) {
      if (_menuFile->IsEntryEnabled(M_FILE_SAVE))
        DoSave();
      else
        return DoSaveAs();
    }
  }

  return (retc == ID_CLOSE) ? ID_CANCEL : retc;
}


void OXAppMainFrame::DoCut() {
}


void OXAppMainFrame::DoCopy() {
}


void OXAppMainFrame::DoPaste() {
  _pcanvas->Paste();
  _palette->UpdateColors(_pcanvas->GetColorTable());
}


void OXAppMainFrame::DoSetMode(int mode) {
  int i;

  for (i=0; tb_data[i].pixmap_name != NULL; ++i)
    tb_data[i].button->SetState((mode == tb_data[i].id) ? 
                                         BUTTON_ENGAGED : BUTTON_UP);

  switch (mode) {
    case 1001: _pcanvas->SetMode(DRAW_POINT); break;
    case 1002: _pcanvas->SetMode(SELECT_REGION); break;
    case 1003: _pcanvas->SetMode(DRAW_RECTANGLE); break;
    case 1004: _pcanvas->SetMode(DRAW_FILLED_RECTANGLE); break;
    case 1005: _pcanvas->SetMode(DRAW_CIRCLE); break;
    case 1006: _pcanvas->SetMode(DRAW_FILLED_CIRCLE); break;
    case 1007: _pcanvas->SetMode(DRAW_LINE); break;
    case 1008: _pcanvas->SetMode(DRAW_CURVE); break;
    case 1009: _pcanvas->SetMode(FLOOD_FILL); break;
    case 1010: DoSetText(); _pcanvas->SetMode(DRAW_TEXT); break;
  }
}


void OXAppMainFrame::DoZoomIn() {
  if (ZoomFactor < ZOOM_MAX) {
    ++ZoomFactor; 

    _pcanvas->SetZoom(ZoomFactor);
    _canvas->Layout();
    _ddlb->Select(ZoomFactor);

    switch (ZoomFactor) {
      case ZOOM_GRID_BOUND1: 
        _menuView->EnableEntry(M_VIEW_GRID);
        break;

      case ZOOM_MAX: 
        _menuView->DisableEntry(M_VIEW_ZOOMIN);
        break;

      default: 
        _menuView->EnableEntry(M_VIEW_ZOOMOUT);
        break;
    }
  }
}


void OXAppMainFrame::DoZoomOut() {
  if (ZoomFactor > ZOOM_MIN) {
    --ZoomFactor; 

    _pcanvas->SetZoom(ZoomFactor);
    _canvas->Layout();
    _ddlb->Select(ZoomFactor);

    switch (ZoomFactor) {
      case ZOOM_MIN: 
        _menuView->DisableEntry(M_VIEW_ZOOMOUT);
        break;

      case ZOOM_GRID_BOUND2:
        _menuView->DisableEntry(M_VIEW_GRID);
        break;

      default: 
        _menuView->EnableEntry(M_VIEW_ZOOMIN);
        break;
    }
  }
}


void OXAppMainFrame::DoZoomSet() {
  ZoomFactor = _ddlb->GetSelected();

  _pcanvas->SetZoom(ZoomFactor);
  _canvas->Layout();

  if (ZoomFactor > ZOOM_GRID_BOUND2) {

    _menuView->EnableEntry(M_VIEW_GRID);
    _menuView->EnableEntry(M_VIEW_ZOOMOUT);

    if (ZoomFactor == ZOOM_MAX)
      _menuView->DisableEntry(M_VIEW_ZOOMIN);
    else
      _menuView->EnableEntry(M_VIEW_ZOOMIN);

  } else {

    _menuView->DisableEntry(M_VIEW_GRID);
    _menuView->EnableEntry(M_VIEW_ZOOMIN);

    if (ZoomFactor == ZOOM_MIN)
      _menuView->DisableEntry(M_VIEW_ZOOMOUT);
    else
      _menuView->EnableEntry(M_VIEW_ZOOMOUT);

  }

}


void OXAppMainFrame::DoResize() {
  int newW = _pcanvas->GetImageWidth();
  int newH = _pcanvas->GetImageHeight();

  new OXSizeDialog(_client->GetRoot(), this, 10, 10, 0,
                   DIALOG_RESIZE, &newW, &newH);
  if (newW == -1) return;
  newW = max(newW, 1);
  newH = max(newH, 1);
  _pcanvas->ResizeImage(newW, newH);
  _canvas->Layout();
  UpdateStatus();
}


void OXAppMainFrame::DoRescale() {
  int newW = _pcanvas->GetImageWidth();
  int newH = _pcanvas->GetImageHeight();

  new OXSizeDialog(_client->GetRoot(), this, 10, 10, 0,
                   DIALOG_RESCALE, &newW, &newH);
  if (newW == -1) return;
  newW = max(newW, 1);
  newH = max(newH, 1);
  _pcanvas->RescaleImage(newW, newH);
  _canvas->Layout();
  UpdateStatus();
}


void OXAppMainFrame::DoSetFont() {
  OString fname("");

  new OXFontDialog(_client->GetRoot(), this, &fname);
  if (fname.GetLength() == 0) return;

  _pcanvas->SetFont(_client->GetFont(fname.GetString()));
}

void OXAppMainFrame::DoSetText() {
  char text[256];

  strcpy(text, _pcanvas->GetText());

  new OXTextDialog(_client->GetRoot(), this, text);
  if (*text) _pcanvas->SetText(text);
}

void OXAppMainFrame::DoAddColor() {
  int retc;
  static OColor color(255, 255, 255);

  new OXColorDialog(_client->GetRoot(), this, &retc, &color);

  if (retc == ID_OK) {
    char tmp[10];

    OColorTable *ct = _pcanvas->GetColorTable();

    sprintf(tmp, "#%02x%02x%02x",
                 color.GetR(), color.GetG(), color.GetB());

    ct->UseColorInTable(_client->GetColorByName(tmp), NULL,
                        NULL, NULL, NULL, NULL, tmp);
    _palette->UpdateColors(ct);
  }
}

void OXAppMainFrame::DoSetColor(unsigned long color) {
  _pcanvas->SetDrawingForeground(color);
  if (color == TRANSPARENT(GetDisplay(), _client->GetScreenNumber())) {
    _currentColor->SetBackgroundPixmap(_palette->GetBgndPic()->GetPicture());
  } else {
    _currentColor->SetBackgroundPixmap(None);
    _currentColor->SetBackgroundColor(color);
  }
  _client->NeedRedraw(_currentColor);
}

void OXAppMainFrame::DoToggleGrid() {
  Grid = !Grid;
  _pcanvas->SetGrid(Grid);
  if (Grid)
    _menuView->CheckEntry(M_VIEW_GRID);
  else
    _menuView->UnCheckEntry(M_VIEW_GRID);
}


void OXAppMainFrame::DoToggleToolBar() {
  if (_vf1->IsVisible()) {
    _f1->HideFrame(_vf1);
    _menuView->UnCheckEntry(M_VIEW_TOOLBAR);
  } else {
    _f1->ShowFrame(_vf1);
    _menuView->CheckEntry(M_VIEW_TOOLBAR);
  }
  Resize(GetDefaultSize());
}

void OXAppMainFrame::DoToggleStatusBar() {
  if (_statusBar->IsVisible()) {
    HideFrame(_statusBar);
    _menuView->UnCheckEntry(M_VIEW_STATUSBAR);
  } else {
    ShowFrame(_statusBar);
    _menuView->CheckEntry(M_VIEW_STATUSBAR);
  }
  Resize(GetDefaultSize());
}


void OXAppMainFrame::DoTogglePalette() {
  if (_f2->IsVisible()) {
    HideFrame(_f2);
    _menuView->UnCheckEntry(M_VIEW_PALETTE);
  } else {
    ShowFrame(_f2);
    _menuView->CheckEntry(M_VIEW_PALETTE);
  }
  Resize(GetDefaultSize());
}


void OXAppMainFrame::DoHelpAbout() {
  OAboutInfo info;

  info.wname = "About fOX Paint";
  info.title = "fOX Paint Version " XCPAINT_VERSION "\nA simple XPM editor";
  info.copyright = "Copyright © 1998-2001 by H. Peraza";
  info.text = "This program is free software; you can redistribute it "
              "and/or modify it under the terms of the GNU "
              "General Public License.\n\n"
              "http://www.foxproject.org";

  new OXAboutDialog(_client->GetRoot(), this, &info);
}


void OXAppMainFrame::UpdateStatus() {
  char tmp[100];

  sprintf(tmp, "%d x %d", _pcanvas->GetImageWidth(),
                          _pcanvas->GetImageHeight());
  _statusBar->SetText(0, new OString(tmp));
}
