#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>

//#define USE_FAVOR_DLG

#include <xclass/utils.h>
#include <xclass/OXMsgBox.h>
#include <xclass/OXAboutDialog.h>
#include <xclass/OMimeTypes.h>
#include <xclass/OXFileDialog.h>

#include <X11/keysym.h>

#include "OXGhostView.h"
#include "OXDialogs.h"
#include "OXPageLBEntry.h"
#include "main.h"

#include "gtb-open.xpm"
#include "gtb-save.xpm"
#include "gtb-print.xpm"
#include "gtb-zoomin.xpm"
#include "gtb-zoomout.xpm"
#include "gtb-prev.xpm"
#include "gtb-next.xpm"
#include "pbg.xpm"


OXOutput *output_dlg = NULL;

struct _popup file_popup = {
  NULL, {
  { "&Open...",               M_FILE_OPEN,       0,             NULL },
  { "&Reopen",                M_FILE_REOPEN,     MENU_DISABLED, NULL },
  { "&Save marked pages...",  M_FILE_SAVEMARKED, MENU_DISABLED, NULL },
  { "",                       -1,                0,             NULL },
  { "&Print...",              M_FILE_PRINT,      MENU_DISABLED, NULL },
  { "P&rint setup...",        M_FILE_PRINTSETUP, MENU_DISABLED, NULL },
  { "",                       -1,                0,             NULL },
  { "E&xit",                  M_FILE_EXIT,       0,             NULL },
  { NULL,                     -1,                0,             NULL } } };

struct _popup media_popup = {
  NULL, {
  { NULL,              -1,                0,             NULL } } };

struct _popup magstep_popup = {
  NULL, {
  { "-5",              M_MAGSTEP_M5,      0,             NULL },
  { "-4",              M_MAGSTEP_M4,      0,             NULL },
  { "-3",              M_MAGSTEP_M3,      0,             NULL },
  { "-2",              M_MAGSTEP_M2,      0,             NULL },
  { "-1",              M_MAGSTEP_M1,      0,             NULL },
  { "0",               M_MAGSTEP_0,       MENU_RCHECKED, NULL },
  { "1",               M_MAGSTEP_P1,      0,             NULL },
  { "2",               M_MAGSTEP_P2,      0,             NULL },
  { "3",               M_MAGSTEP_P3,      0,             NULL },
  { "4",               M_MAGSTEP_P4,      0,             NULL },
  { "5",               M_MAGSTEP_P5,      0,             NULL },
  { NULL,              -1,                0,             NULL } } };

struct _popup orien_popup = {
  NULL, {
  { "&Document Default", M_ORIEN_DEFAULT, MENU_CHECKED,  NULL },
  { "",                -1,                0,             NULL },
  { "&Portrait",       M_ORIEN_PORTRAIT,  MENU_RCHECKED, NULL },
  { "&Landscape",      M_ORIEN_LANDSCAPE, 0,             NULL },
  { "&Upside-down",    M_ORIEN_UPDOWN,    0,             NULL },
  { "&Seascape",       M_ORIEN_SEASCAPE,  0,             NULL },
  { "",                -1,                0,             NULL },
  { "S&wap Landscape", M_ORIEN_SWAPLANDSCAPE, 0,         NULL },
  { NULL,              -1,                0,             NULL } } };

struct _popup view_popup = {
  NULL, {
  { "&Toolbar",        M_VIEW_TOOLBAR,     MENU_CHECKED,  NULL },
  { "Status &Bar",     M_VIEW_STATUSBAR,   MENU_CHECKED,  NULL },
  { "&Page Index",     M_VIEW_PAGEINDEX,   MENU_CHECKED | MENU_DISABLED,  NULL },
  { "",                -1,                 0,             NULL },
  { "Mag&step",        M_VIEW_MAGSTEP,     0,             &magstep_popup },
  { "&Orientation",    M_VIEW_ORIENTATION, 0,             &orien_popup },
  { "&Media",          M_VIEW_MEDIA,       0,             &media_popup },
  { "",                -1,                 0,             NULL },
  { "&Ghostscript output...", M_VIEW_OUTPUT,   0,             NULL },
  { NULL,              -1,                 0,             NULL } } };

struct _popup page_popup = {
  NULL, {
  { "&Next\tPgDn",     M_PAGE_NEXT,       MENU_DISABLED, NULL },
  { "&Previous\tPgUp", M_PAGE_PREVIOUS,   MENU_DISABLED, NULL },
  { "&Redisplay",      M_PAGE_REDISPLAY,  MENU_DISABLED, NULL },
  { "",                -1,                0,             NULL },
  { "&Mark",           M_PAGE_MARK,       MENU_DISABLED, NULL },
  { "&Unmark",         M_PAGE_UNMARK,     MENU_DISABLED, NULL },
  { NULL,              -1,                0,             NULL } } };

struct _popup help_popup = {
  NULL, {
  { "&Contents...",    M_HELP_CONTENTS,   MENU_DISABLED, NULL },
  { "&Search...",      M_HELP_SEARCH,     MENU_DISABLED, NULL },
  { "",                -1,                0,             NULL },
  { "&About...",       M_HELP_ABOUT,      0,             NULL },
  { NULL,              -1,                0,             NULL } } };


SToolBarData tb_data[] = {
  { "gtb-open.xpm",   gtb_open_xpm,   "Open File",         BUTTON_NORMAL, M_FILE_OPEN,  NULL },
  { "gtb-save.xpm",   gtb_save_xpm,   "Save Marked Pages", BUTTON_NORMAL, M_FILE_SAVEMARKED, NULL },
  { "gtb-print.xpm",  gtb_print_xpm,  "Print",             BUTTON_NORMAL, M_FILE_PRINT, NULL },
  { "",               NULL,           0,                   0,             -1,           NULL },
  { "gtb-prev.xpm",   gtb_prev_xpm,   "Previous Page",     BUTTON_NORMAL, M_PAGE_PREVIOUS, NULL },
  { "gtb-next.xpm",   gtb_next_xpm,   "Next Page",         BUTTON_NORMAL, M_PAGE_NEXT,  NULL },
  { "",               NULL,           0,                   0,             -1,           NULL },
  { "gtb-zoomin.xpm", gtb_zoomin_xpm, "Zoom In",           BUTTON_NORMAL, M_ZOOM_IN,    NULL },
  { "gtb-zoomout.xpm",gtb_zoomout_xpm,"Zoom Out",          BUTTON_NORMAL, M_ZOOM_OUT,   NULL },
  { NULL,             NULL,           NULL,                0,             0,            NULL }
};


char *filetypes[] = { "Postscript files", "*.ps|*.eps",
                      "All files",        "*",
		      NULL,               NULL };

//----------------------------------------------------------------------

int main(int argc, char **argv) {

  OXClient *clientX = new OXClient(argc, argv);

  OXMain *mainw = new OXMain(clientX->GetRoot(), 600, 400);
  mainw->MapWindow();

  if (argc > 1) mainw->OpenFile(argv[1], True);

  clientX->Run();

  exit(0);
}

OXMain::OXMain(const OXWindow *p, int w, int h) : OXMainFrame(p, w, h) {
  char tmp[BUFSIZ];
  const OPicture *cbgnd;

  _menuBarLayout = new OLayoutHints(LHINTS_TOP | LHINTS_LEFT | LHINTS_EXPAND_X, 
                                    0, 0, 1, 1);
  _menuBarItemLayout = new OLayoutHints(LHINTS_TOP | LHINTS_LEFT,
                                    0, 4, 0, 0);

  _magstepPopup = _MakePopup(&magstep_popup);
  _orienPopup   = _MakePopup(&orien_popup);
  _mediaPopup   = _MakePopup(&media_popup);

  _menuFile = _MakePopup(&file_popup);
  _menuView = _MakePopup(&view_popup);
  _menuPage = _MakePopup(&page_popup);
  _menuHelp = _MakePopup(&help_popup);

  _menuFile->Associate(this);
  _menuView->Associate(this);
  _menuPage->Associate(this);
  _menuHelp->Associate(this);
  _magstepPopup->Associate(this);
  _orienPopup->Associate(this);
  _mediaPopup->Associate(this);

  _BuildPagemediaMenu();

  //------ menu bar

  _menuBar = new OXMenuBar(this, 1, 1, HORIZONTAL_FRAME);
  _menuBar->AddPopup(new OHotString("&File"), _menuFile, _menuBarItemLayout);
  _menuBar->AddPopup(new OHotString("&View"), _menuView, _menuBarItemLayout);
  _menuBar->AddPopup(new OHotString("&Page"), _menuPage, _menuBarItemLayout);
  _menuBar->AddPopup(new OHotString("&Help"), _menuHelp, _menuBarItemLayout);

  AddFrame(_menuBar, _menuBarLayout);

  //---- toolbar

  _toolBarSep = new OXHorizontal3dLine(this);

  _toolBar = new OXToolBar(this);
  _toolBar->Associate(this);
  _toolBar->AddButtons(tb_data);

  AddFrame(_toolBarSep, new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X));
  AddFrame(_toolBar, new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 
                                      0, 0, 3, 3));

  //------ horizontal frame containing _lb and _canvas

  _hf = new OXCompositeFrame(this, 10, 10, HORIZONTAL_FRAME);

  //------ page index (table of contents)

  _lb = new OXListBox(_hf, LB_PAGE);
  _lb->SetOptions(FIXED_WIDTH);
  _lb->Resize(54, 50);
  _lb->IntegralHeight(False);
  _lb->SetMultipleSelect(True);
  _lb->Associate(this);

  _hf->AddFrame(_lb, new OLayoutHints(LHINTS_LEFT | LHINTS_EXPAND_Y,
                     0, 0/*3*/, 0, 0));

  _resizer = new OXVerticalResizer(_hf);
  _hf->AddFrame(_resizer, new OLayoutHints(LHINTS_LEFT | LHINTS_EXPAND_Y,
                          0, 0, 0, 0));

  //------ Canvas frame and ghostview widget

  _canvas = new OXCanvas(_hf, 10, 10, SUNKEN_FRAME | DOUBLE_BORDER);
  _hf->AddFrame(_canvas, new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y));

  _container = new OXCompositeFrame(_canvas->GetViewPort(), 640, 480,
                                    OWN_BKGND);
  _canvas->SetContainer(_container);

  cbgnd = _client->GetPicture("pbg.xpm", pbg_xpm);
  if (!cbgnd) FatalError("Bad or missing internal pixmap");
  _container->SetBackgroundPixmap(cbgnd->GetPicture());

  _gv = new OXGhostView(_container, 10, 10, GV_MAIN,
                        OWN_BKGND, _client->GetColorByName("white"));
  _container->AddFrame(_gv, new OLayoutHints(LHINTS_CENTER_X | LHINTS_CENTER_Y,
                                             5, 5, 5, 5));

  _gv->DisableInterpreter();
  _gv->Associate(this);

  AddFrame(_hf, new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y));

  _resizer->SetPrev(_lb);
  _resizer->SetNext(_canvas);

  //------ status bar

  _statusBar = new OXStatusBar(this);
  AddFrame(_statusBar, new OLayoutHints(LHINTS_BOTTOM | LHINTS_EXPAND_X,
                                        0, 0, 3, 0));

  SetWindowTitle(NULL);
  SetClassHints("XCGhostView", "XCGhostView");

  //------ init variables...

  _psFile = NULL;

  doc = NULL;
  current_page = -1;
  current_pagemedia = 0;
  current_orientation = PageOrientationPortrait;
  swap_landscape = False;
  has_toc = False;
  mag_step = 0;
  base_papersize = 0;
  default_document_pagemedia = True;
  default_document_orientation = True;
  last_pagemedia_entry = M_PAGEMEDIA;

  _filename = NULL;
  _full_filename = NULL;

  for (int i = 0; i < NUM_RECENT; ++i) _recent_files[i] = NULL;

  _printerName = StrDup("lp");
  _printProg = StrDup("lpr");

  _gsCommand = StrDup("gs");

  _winx = w;
  _winy = h;

  // we will keep our own list of listbox entries,
  // as that makes operations with marked pages easier
  toc = NULL;

  AddInput(KeyPressMask);
  SetFocusOwner(this);

  //--- read .ini file settings...

  _ReadIniFile();

  MapSubwindows();

  _hf->HideFrame(_lb);
  _hf->HideFrame(_resizer);

  _GrabKeys();

  Resize(_winx, _winy);
  Layout();
}

OXMain::~OXMain() {
  delete _menuBarLayout;
  delete _menuBarItemLayout;

  delete _menuFile;
  delete _menuView;
  delete _menuPage;
  delete _menuHelp;
  delete _mediaPopup;
  delete _orienPopup;
  delete _magstepPopup;

  if (_filename) delete[] _filename;
  if (_full_filename) delete[] _full_filename;
  for (int i = 0; i < NUM_RECENT; ++i)
    if (_recent_files[i]) delete[] _recent_files[i];
  if (_printerName) delete[] _printerName;
  if (_printProg) delete[] _printProg;
  if (_gsCommand) delete[] _gsCommand;
}

int OXMain::CloseWindow() {
  _gv->DisableInterpreter();
  _SaveIniFile();
  return OXMainFrame::CloseWindow();
}

OXPopupMenu *OXMain::_MakePopup(struct _popup *p) {

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

int OXMain::ProcessMessage(OMessage *msg) {
  OWidgetMessage  *wmsg;
  OListBoxMessage *lbmsg;

  switch (msg->type) {

    case MSG_MENU:
    case MSG_BUTTON:
      wmsg = (OWidgetMessage *) msg;
      switch (msg->action) {

        case MSG_CLICK:
          if (wmsg->id >= M_PAGEMEDIA) DoSetPageMedia(wmsg->id);

          if ((wmsg->id >= M_FILE_RECENT + 1) &&
              (wmsg->id <= M_FILE_RECENT + NUM_RECENT)) {
            if (_recent_files[wmsg->id - M_FILE_RECENT - 1])
              OpenFile(_recent_files[wmsg->id - M_FILE_RECENT - 1]);
              break;
          }

          switch (wmsg->id) {

            //--------------------------------------- File

            case M_FILE_OPEN:
              DoOpen();
              break;

            case M_FILE_REOPEN:
              DoReopen();
              break;

            case M_FILE_SAVEMARKED:
              DoSaveMarked();
              break;

            case M_FILE_PRINT:
              DoPrint();
              break;

            case M_FILE_EXIT:
              CloseWindow();
              break;

            //--------------------------------------- View

            case M_VIEW_OUTPUT:
              DoViewOutput();
              break;

            case M_VIEW_TOOLBAR:
              DoToggleToolBar();
              break;

            case M_VIEW_PAGEINDEX:
              DoTogglePageIndex();
              break;

            case M_VIEW_STATUSBAR:
              DoToggleStatusBar();
              break;

            //--------------------------------------- Page

            case M_PAGE_NEXT:
              DoPageNext();
              break;

            case M_PAGE_PREVIOUS:
              DoPagePrev();
              break;

            case M_PAGE_MARK:
              DoMark();
              break;

            case M_PAGE_UNMARK:
              DoUnmark();
              break;

            case M_ORIEN_DEFAULT:
              DoToggleDefaultOrientation();
              break;

            case M_ORIEN_PORTRAIT:
            case M_ORIEN_LANDSCAPE:
            case M_ORIEN_UPDOWN:
            case M_ORIEN_SEASCAPE:
              DoSetOrientation(wmsg->id);
              break;

            case M_ORIEN_SWAPLANDSCAPE:
              DoSwapLandscape();
              break;

            case M_MAGSTEP_M5:
            case M_MAGSTEP_M4:
            case M_MAGSTEP_M3:
            case M_MAGSTEP_M2:
            case M_MAGSTEP_M1:
            case M_MAGSTEP_0:
            case M_MAGSTEP_P1:
            case M_MAGSTEP_P2:
            case M_MAGSTEP_P3:
            case M_MAGSTEP_P4:
            case M_MAGSTEP_P5:
              DoSetMagstep(wmsg->id - M_MAGSTEP_0);
              break;

            case M_ZOOM_IN:
              DoSetMagstep(mag_step+1);
              break;

            case M_ZOOM_OUT:
              DoSetMagstep(mag_step-1);
              break;

            default:
              break;

            //--------------------------------------- Help

            case M_HELP_ABOUT:
              DoHelpAbout();
              break;

          } // switch (id)
          break;

        default:
          break;

      } // switch (action)
      break;

    case MSG_LISTBOX:
      lbmsg = (OListBoxMessage *) msg;
      if (lbmsg->action == MSG_CLICK) {
        switch (lbmsg->id) {
          case LB_PAGE:
            if (lbmsg->entryID != current_page)
              _ShowPage(lbmsg->entryID); ///////
            break;

          default:
            break;
        }
      }
      break;

    case MSG_GHOSTVIEW:
      wmsg = (OWidgetMessage *) msg;
      switch (msg->action) {
        case GV_PAGE:
          break;

        case GV_DONE:
          break;

        case GV_REFRESH:
          DoRefresh();
          break;

        case GV_BADALLOC:
          ErrorMsg(MB_ICONASTERISK,
                   "Could not allocate backing pixmap in main window.");
          break;

        case GV_FAILED:
          ErrorMsg(MB_ICONSTOP,
                   "PostScript interpreter failed in main window");
          break;

        case GV_OUTPUT:
          DoViewOutput();
          if (output_dlg) {
            output_dlg->AddText(_gv->GetOutputBuffer());
            _gv->ClearOutputBuffer();
          }
          break;

        default:
          break;
      }
      break;

    default:
      break;

  } // switch (type)

  return True;
}

//----------------------------------------------------------------------

void OXMain::SetWindowTitle(const char *title) {
  static char *pname = "fOX Ghostview";

  if (title) {
    char *wname = new char[strlen(title) + 20];
    sprintf(wname, "%s - %s", pname, title);
    SetWindowName(wname);
    delete[] wname;
  } else {
    SetWindowName(pname);
  }
}

void OXMain::UpdateStatus() {
  char tmp[1024];

  strcpy(tmp, (doc && doc->title) ? doc->title : "");
  _statusBar->SetText(0, new OString(tmp));
}

void OXMain::ErrorMsg(int icon_type, char *msg) {
  OString stitle((icon_type == MB_ICONSTOP) ? "Error" : "Warning");
  OString smsg(msg);
  new OXMsgBox(_client->GetRoot(), this, &stitle, new OString(&smsg),
               icon_type, ID_OK);
}

//----------------------------------------------------------------------

void OXMain::OpenFile(const char *fname, int die) {
  struct stat sbuf;
  char buf[BUFSIZ];
  FILE *fp;

  if (!fname) return;

  if ((fp = fopen(fname, "r")) == NULL) {
    if (die) {
      fprintf(stderr, "Could not open input file: \"%s\"\n", fname);
      exit(1);
    } else {
      ErrorMsg(MB_ICONSTOP, "Could not open file.");
    }
  } else {
    _filename = StrDup(fname);
    if (*fname == '/') {
      _full_filename = StrDup(fname);
    } else {
      getcwd(buf, BUFSIZ);
      _full_filename = new char[strlen(buf)+strlen(fname)+2];
      sprintf(_full_filename, "%s/%s", buf, fname);
    }
    //if (_psFile) fclose(_psFile);
    _psFile = fp;
    _AddToRecent(_full_filename);
    stat(_full_filename, &sbuf);
    mtime = sbuf.st_mtime;
    _NewFile(0);
    SetWindowTitle(_filename);
    UpdateStatus();
    _ShowPage(0);
  }
}

void OXMain::DoOpen() {
  OFileInfo fi;
  struct stat sbuf;
  FILE *fp;

  fi.MimeTypesList = GetResourcePool()->GetMimeTypes();
  fi.file_types = filetypes;
#ifdef USE_FAVOR_DLG
  new OXFileDialog(_client->GetRoot(), this, FDLG_OPEN | FDLG_FAVOURITES, &fi);
#else
  new OXFileDialog(_client->GetRoot(), this, FDLG_OPEN, &fi);
#endif
  if (fi.filename) {
    if ((fp = fopen(fi.filename, "r")) == NULL) {
      ErrorMsg(MB_ICONSTOP, "Could not open file.");
    } else {
      if (_filename) delete[] _filename;
      if (_full_filename) delete[] _full_filename;
      _filename = StrDup(fi.filename);
      _full_filename = new char[strlen(fi.ini_dir)+strlen(fi.filename)+2];
      sprintf(_full_filename, "%s/%s", fi.ini_dir, fi.filename);
      if (_psFile) fclose(_psFile);
      _psFile = fp;
      _AddToRecent(_full_filename);
      stat(_full_filename, &sbuf);
      mtime = sbuf.st_mtime;
      _NewFile(0);
      SetWindowTitle(_filename);
      UpdateStatus();
      _ShowPage(0);
    }
  }
}

//--- Explicitly reopen the file.

void OXMain::DoReopen() {
  struct stat sbuf;
  int page = current_page;

  if (_psFile) fclose(_psFile);
  _psFile = fopen(_full_filename, "r");
  if (!_psFile) {
    ErrorMsg(MB_ICONSTOP, "Could not reopen file.");
    return;
  }
  stat(_full_filename, &sbuf);
  mtime = sbuf.st_mtime;
  _NewFile(page, False);
  _ShowPage(page);
}


void OXMain::DoSaveMarked() {
  int retc;
  OFileInfo fi;
  FILE *pswrite;

  if (!_NumMarkedPages()) {
    XBell(GetDisplay(), 0);
    return;
  }

  fi.MimeTypesList = GetResourcePool()->GetMimeTypes();
  fi.file_types = filetypes;
#ifdef USE_FAVOR_DLG
  new OXFileDialog(_client->GetRoot(), this, FDLG_SAVE | FDLG_FAVOURITES, &fi);
#else
  new OXFileDialog(_client->GetRoot(), this, FDLG_SAVE, &fi);
#endif
  if (fi.filename) {

    // check whether the file already exists, and ask
    // permission to overwrite if so.

    if (access(fi.filename, F_OK) == 0) {
      new OXMsgBox(_client->GetRoot(), this,
            new OString("Save"),
            new OString("A file with the same name already exists. Overwrite?"),
            MB_ICONQUESTION, ID_YES | ID_NO, &retc);
      if (retc == ID_NO) return;
    }

    pswrite = fopen(fi.filename, "w");
    if (pswrite == NULL) {
      OString stitle("Write Error");
      ErrorMsg(MB_ICONSTOP, "Can't write postscript file.");
    } else {
      _WriteDoc(pswrite);
      fclose(pswrite);
    }

  }

}

void OXMain::DoPrint() {
  int  retc = PB_PRINTALL;
  char tmp[1024];
  int  whole_mode = True;

  if (!_menuFile->IsEntryEnabled(M_FILE_PRINT)) {
    XBell(GetDisplay(), 0);
    return;
  }

  if (doc && toc) {
    if (_NumMarkedPages()) {
      whole_mode = False;
      retc |= PB_PRINTMARKED;
    }
  }

  new OXPrintBox(_client->GetRoot(), this, 400, 150, 
                 &_printerName, &_printProg, &retc);

  if (retc & PB_OK) {

    if (retc & PB_PRINTALL) whole_mode = True;

    sprintf(tmp, "%s -P%s", _printProg, _printerName);
    FILE *p = popen(tmp, "w");
    if (p == NULL) {
      ErrorMsg(MB_ICONSTOP, "Print command failed.");
      return;
    }

    if (toc && !whole_mode) {
      _WriteDoc(p);
    } else {
      FILE *psf = fopen(_full_filename, "r");
      if (psf) {
        int  bytes;
        char buf[BUFSIZ];

        while (bytes = read(fileno(psf), buf, BUFSIZ))
          bytes = write(fileno(p), buf, bytes);
        fclose(psf);
      } else {
        ErrorMsg(MB_ICONSTOP, "Could not open source file.");
      }
    }

    if (pclose(p) != 0)
      ErrorMsg(MB_ICONSTOP, "Print command failed.");      

  }      

}

int OXMain::DoPageNext() {
  int new_page;

  if (has_toc && doc) {
    new_page = current_page + 1;
    if (new_page < doc->numpages) {
      _ShowPage(new_page);
      return True;
    } else {
      XBell(GetDisplay(), 0);
      return False;
    }
  } else {
    _ShowPage(0);
    return True;
  }
}

int OXMain::DoPagePrev() {
  int new_page;

  if (has_toc) {
    new_page = current_page - 1;
    if (new_page >= 0) {
      _ShowPage(new_page);
      return True;
    } else {
      XBell(GetDisplay(), 0);
      return False;
    }
  }
  return False;
}

void OXMain::DoToggleDefaultOrientation() {
  if (default_document_orientation) {
    default_document_orientation = False;
    _orienPopup->UnCheckEntry(M_ORIEN_DEFAULT);
  } else {
    default_document_orientation = True;
    _orienPopup->CheckEntry(M_ORIEN_DEFAULT);
  }
  _SetOrientation(current_page);
}

void OXMain::DoSwapLandscape() {
  if (swap_landscape) {
    swap_landscape = False;
    _orienPopup->UnCheckEntry(M_ORIEN_SWAPLANDSCAPE);
  } else {
    swap_landscape = True;
    _orienPopup->CheckEntry(M_ORIEN_SWAPLANDSCAPE);
  }
  _ShowPage(current_page);
}

void OXMain::DoMark() {
  if (toc) {
    int i;

    for (i=0; i<doc->numpages; ++i)
      if (toc[i]->IsActive()) 
        toc[i]->SetMark(PLBE_CHECKMARK, True);
  }
  if (_NumMarkedPages())
    _menuFile->EnableEntry(M_FILE_SAVEMARKED);
  else
    _menuFile->DisableEntry(M_FILE_SAVEMARKED);
}

void OXMain::DoUnmark() {
  if (toc) {
    int i;

    for (i=0; i<doc->numpages; ++i)
      if (toc[i]->IsActive())
        toc[i]->SetMark(PLBE_CHECKMARK, False);
  }
  if (_NumMarkedPages())
    _menuFile->EnableEntry(M_FILE_SAVEMARKED);
  else
    _menuFile->DisableEntry(M_FILE_SAVEMARKED);
}

void OXMain::DoSetMagstep(int mstep) {
  float xdpi, ydpi;

  if (mstep == mag_step) return;

  if (mstep < -5 || mstep > 5) {
    XBell(GetDisplay(), 0);
    return;
  }

  mag_step = mstep;

  xdpi = _gv->ComputeXdpi();
  ydpi = _gv->ComputeYdpi();

  if (mstep < 0) {
    while (mstep++) xdpi /= 1.2, ydpi /= 1.2;
  } else {
    while (mstep--) xdpi *= 1.2, ydpi *= 1.2;
  }

  _magstepPopup->RCheckEntry(mag_step+M_MAGSTEP_0, 
                             M_MAGSTEP_M5, M_MAGSTEP_P5);
  if (_gv->SetDpi(xdpi, ydpi)) {
    _canvas->Layout();
  }
}

void OXMain::DoSetOrientation(int orien) {
  default_document_orientation = False;
  _orienPopup->UnCheckEntry(M_ORIEN_DEFAULT);
  switch (orien) {
    default:
    case M_ORIEN_PORTRAIT:
      current_orientation = PageOrientationPortrait;
      break;

    case M_ORIEN_LANDSCAPE:
      current_orientation = PageOrientationLandscape;
      break;

    case M_ORIEN_UPDOWN:
      current_orientation = PageOrientationUpsideDown;
      break;

    case M_ORIEN_SEASCAPE:
      current_orientation = PageOrientationSeascape;
      break;
  }
  _SetOrientation(current_page);
}

void OXMain::DoSetPageMedia(int media) {
  int n = media - M_PAGEMEDIA;

  default_document_pagemedia = False;
  _SetPagemedia(current_page, n);
}

void OXMain::DoRefresh() {
  int i;

  if (has_toc && doc && doc->pages) {
    _gv->SendPS(_psFile, doc->beginprolog,
                         doc->lenprolog, False);
    _gv->SendPS(_psFile, doc->beginsetup,
                         doc->lensetup, False);
    if (doc->pageorder == PS_DESCEND)
      i = (doc->numpages - 1) - current_page;
    else
      i = current_page;
    _gv->SendPS(_psFile, doc->pages[i].begin,
                         doc->pages[i].len, False);
  }
}

void OXMain::DoToggleToolBar() {
  if (_toolBar->IsVisible()) {
    HideFrame(_toolBar);
    HideFrame(_toolBarSep);
    _menuView->UnCheckEntry(M_VIEW_TOOLBAR);
  } else {
    ShowFrame(_toolBar);
    ShowFrame(_toolBarSep);
    _menuView->CheckEntry(M_VIEW_TOOLBAR);
  }
}

void OXMain::DoTogglePageIndex() {
  if (_menuView->IsEntryChecked(M_VIEW_PAGEINDEX)) {
    _hf->HideFrame(_lb);
    _hf->HideFrame(_resizer);
    _menuView->UnCheckEntry(M_VIEW_PAGEINDEX);
  } else {
    _hf->ShowFrame(_lb);
    _hf->ShowFrame(_resizer);
    _menuView->CheckEntry(M_VIEW_PAGEINDEX);
  }
}

void OXMain::DoToggleStatusBar() {
  if (_statusBar->IsVisible()) {
    HideFrame(_statusBar);
    _menuView->UnCheckEntry(M_VIEW_STATUSBAR);
  } else {
    ShowFrame(_statusBar);
    _menuView->CheckEntry(M_VIEW_STATUSBAR);
  }
}

void OXMain::DoViewOutput() {
  if (output_dlg) {
    output_dlg->RaiseWindow();
  } else {
    output_dlg = new OXOutput(_client->GetRoot(), this, 100, 100);
  }
}

void OXMain::DoHelpAbout() {
  OAboutInfo info;

  info.wname = "About fOX Ghostview";
  info.title = "fOX Ghostview version " XCGVIEW_VERSION "\n"
               "A Postscript file viewer program";
  info.copyright = "Copyright © 1998-2002 by H. Peraza";
  info.text = "This program is free software; you can redistribute it "
              "and/or modify it under the terms of the GNU "
              "General Public License.\n\n"
              "http://www.foxproject.org";

  new OXAboutDialog(_client->GetRoot(), this, &info);
}

//----------------------------------------------------------------------

void OXMain::_ReadIniFile() {
  char *inipath, line[1024], arg[PATH_MAX];

  inipath = GetResourcePool()->FindIniFile(XCGVIEW_INI, INI_READ);
  if (!inipath) return;

  OIniFile ini(inipath, INI_READ);

  while (ini.GetNext(line)) {

    if (strcasecmp(line, "defaults") == 0) {
      if (ini.GetItem("window size", arg)) {
        if (sscanf(arg, "%d x %d", &_winx, &_winy) == 2) {
          if (_winx < 10 || _winx > 32000 || _winy < 10 || _winy > 32000) {
            _winx = 600;
            _winy = 400;
          }
        } else {
          _winx = 600;
          _winy = 400;
        }
      }
      if (!ini.GetBool("show toolbar", true)) {
        HideFrame(_toolBar);
        HideFrame(_toolBarSep);
        _menuView->UnCheckEntry(M_VIEW_TOOLBAR);
      }
      if (!ini.GetBool("show status bar", true)) {
        HideFrame(_statusBar);
        _menuView->UnCheckEntry(M_VIEW_STATUSBAR);
      }
      if (!ini.GetBool("show page index", true)) {
        _hf->HideFrame(_lb);
        _hf->HideFrame(_resizer);
        _menuView->UnCheckEntry(M_VIEW_PAGEINDEX);
      }
      if (ini.GetItem("index width", arg)) {
        int iw = 54;

        if (sscanf(arg, "%d", &iw) == 1) {
          if (iw < 1) iw = 1;
          if (iw > 32000) iw = 54;
        } else {
          iw = 54;
        }
        _lb->Resize(iw, 50);
      }

    } else if (strcasecmp(line, "document") == 0) {
      if (ini.GetItem("magstep", arg)) {
        int ms = 0;

        if (sscanf(arg, "%d", &ms) == 1) DoSetMagstep(ms);
      }

    } else if (strcasecmp(line, "recent files") == 0) {
      char tmp[50];

      for (int i = NUM_RECENT-1; i >= 0; --i) {
        sprintf(tmp, "file%d", i+1);
        if (ini.GetItem(tmp, arg)) _AddToRecent(arg);
      }

    } else if (strcasecmp(line, "printer") == 0) {
      if (ini.GetItem("name", arg)) {
        if (_printerName) delete[] _printerName;
        _printerName = StrDup(arg);
      }
      if (ini.GetItem("command", arg)) {
        if (_printProg) delete[] _printProg;
        _printProg = StrDup(arg);
      }

    } else if (strcasecmp(line, "ghostscript") == 0) {
      if (ini.GetItem("command", arg)) {
        if (_gsCommand) delete[] _gsCommand;
        _gsCommand = StrDup(arg);
        _gv->SetInterpreterCommand(_gsCommand);
      }

    }
  }

  delete[] inipath;
}

void OXMain::_SaveIniFile() {
  char *inipath, tmp[256];

  inipath = GetResourcePool()->FindIniFile(XCGVIEW_INI, INI_WRITE);
  if (!inipath) return;

  OIniFile ini(inipath, INI_WRITE);

  ini.PutNext("defaults");
  ini.PutBool("show toolbar", _toolBar->IsVisible());
  ini.PutBool("show status bar", _statusBar->IsVisible());
  ini.PutBool("show page index", _menuView->IsEntryChecked(M_VIEW_PAGEINDEX));
  sprintf(tmp, "%d x %d", _w, _h);
  ini.PutItem("window size", tmp);
  sprintf(tmp, "%d", _lb->GetWidth());
  ini.PutItem("index width", tmp);
  ini.PutNewLine();

  ini.PutNext("document");
  switch (current_orientation) {
    default:
    case PageOrientationPortrait:   strcpy(tmp, "portrait");    break;
    case PageOrientationLandscape:  strcpy(tmp, "landscape");   break;
    case PageOrientationUpsideDown: strcpy(tmp, "upside down"); break;
    case PageOrientationSeascape:   strcpy(tmp, "seascape"); break;
  }
  ini.PutItem("orientation", tmp);
  ini.PutBool("use default orientation", default_document_orientation);
  ini.PutBool("swap landscape", swap_landscape);
  ini.PutItem("media", papersizes[current_pagemedia].name);
  ini.PutBool("use default page media", default_document_pagemedia);
  sprintf(tmp, "%d", mag_step);
  ini.PutItem("magstep", tmp);
  ini.PutNewLine();

  ini.PutNext("recent files");
  for (int i = 0; i < NUM_RECENT; ++i) {
    if (_recent_files[i]) {
      sprintf(tmp, "file%d", i+1);
      ini.PutItem(tmp, _recent_files[i]);
    }
  }
  ini.PutNewLine();

  ini.PutNext("printer");
  ini.PutItem("name", _printerName);
  ini.PutItem("command", _printProg);
  ini.PutNewLine();

  ini.PutNext("ghostscript");
  ini.PutItem("command", _gsCommand);
  ini.PutNewLine();

  delete[] inipath;
}

void OXMain::_AddToRecent(const char *filename) {
  int i;

  if (!filename) return;

  // first, see if the file is already there

  for (i = 0; i < NUM_RECENT; ++i) {
    if (_recent_files[i] && (strcmp(filename, _recent_files[i]) == 0)) break;
  }

  if (i == 0) {

    return; // nothing to do, the file already was the most recent

  } else if (i < NUM_RECENT) {

    // the file was there, move it to the top of the list

    char *tmp = _recent_files[i];

    for ( ; i > 0; --i) _recent_files[i] = _recent_files[i-1];
    _recent_files[0] = tmp;

  } else {

    // new file: shift all the entries down and add the filename to the head

    if (_recent_files[NUM_RECENT-1]) delete[] _recent_files[NUM_RECENT-1];
    for (i = NUM_RECENT-1; i > 0; --i)
      _recent_files[i] = _recent_files[i-1];
    _recent_files[0] = StrDup(filename);

  }

  _UpdateRecentList();
}

void OXMain::_UpdateRecentList() {
  int  i;
  char tmp[PATH_MAX];

  _menuFile->RemoveEntry(M_FILE_RECENT, MENU_SEPARATOR);
  for (i = 0; i < NUM_RECENT; ++i) _menuFile->RemoveEntry(M_FILE_RECENT + i+1);

  if (!_recent_files[0]) return;

  _menuFile->AddSeparator(M_FILE_RECENT);
  for (i = 0; i < NUM_RECENT; ++i) {
    if (_recent_files[i]) {
      const char *p = strrchr(_recent_files[i], '/');
      if (p) ++p; else p = _recent_files[i];
      sprintf(tmp, "&%d. %s", i+1, p);
      _menuFile->AddEntry(new OHotString(tmp), M_FILE_RECENT + i+1);
    }
  }
}


//----------------------------------------------------------------------

//  Setup ghostview. This includes:
//    - scanning the PostScript file,
//    - setting the status bar label,
//    - building the pagemedia menu,
//    - building the table of contents,
//    - sensitizing the appropriate menu buttons,
//    - erasing the stdout popup.

void OXMain::_SetupGhostview() {

  //--- reset to a known state.

  psfree(doc);
  current_page = -1;
  _gv->DisableInterpreter();
  if (_psFile) doc = psscan(_psFile);

  default_document_pagemedia = True;
  current_pagemedia = 0;

  _BuildPagemediaMenu();

  //--- build table of contents

  _lb->RemoveAllEntries();
  if (toc) delete[] toc;
  toc = NULL;

  if (doc && (!doc->epsf && doc->numpages > 0 ||
               doc->epsf && doc->numpages > 1)) {
    int maxlen = 0;
    int i, j;
    int useful_page_labels = False;
    char tmp[100];

    if (doc->numpages == 1) useful_page_labels = True;

    for (i=1; i<doc->numpages; i++)
      if (useful_page_labels = (useful_page_labels ||
          strcmp(doc->pages[i-1].label, doc->pages[i].label))) break;

    if (useful_page_labels) {
      for (i=0; i<doc->numpages; i++) 
        maxlen = max(maxlen, (int) strlen(doc->pages[i].label));
    } else {
      //double x;
      //x = doc->numpages;
      maxlen = 5; //log10(x) + 1;  // should resize _lb accordingly...
    }

    toc = new OXPageLBEntry*[doc->numpages];

    _lb->AutoUpdate(False);

    for (i=0; i<doc->numpages; i++) {
      if (useful_page_labels) {
        if (doc->pageorder == PS_DESCEND) {
          j = (doc->numpages - 1) - i;
        } else {
          j = i;
        }
        sprintf(tmp, " %*s ", maxlen, doc->pages[j].label);
      } else {
        sprintf(tmp, " %*d ", maxlen, i+1);
      }
      toc[i] = new OXPageLBEntry(_lb->GetContainer(), new OString(tmp), i);
      _lb->AddEntry(toc[i], new OLayoutHints(LHINTS_EXPAND_X | LHINTS_TOP));
    }
    _lb->AutoUpdate(True);

    _gv->SetFilename(NULL);

    if (_menuView->IsEntryChecked(M_VIEW_PAGEINDEX)) {
      _hf->ShowFrame(_lb);
      _hf->ShowFrame(_resizer);
    }
    _menuView->EnableEntry(M_VIEW_PAGEINDEX);
    _menuPage->EnableEntry(M_PAGE_PREVIOUS);
    _menuPage->EnableEntry(M_PAGE_MARK);
    _menuPage->EnableEntry(M_PAGE_UNMARK);
    _menuPage->EnableEntry(M_PAGE_REDISPLAY);

    has_toc = True;

  } else {

    _gv->SetFilename(_full_filename);
    _hf->HideFrame(_lb);
    _hf->HideFrame(_resizer);
    _menuView->DisableEntry(M_VIEW_PAGEINDEX);
    _menuPage->DisableEntry(M_PAGE_PREVIOUS);
    _menuPage->DisableEntry(M_PAGE_MARK);
    _menuPage->DisableEntry(M_PAGE_UNMARK);
    _menuPage->DisableEntry(M_PAGE_REDISPLAY);

    has_toc = False;

  }

  //_lb->MapSubwindows();
  _lb->Update();
  _hf->Layout();

  if (_psFile) {
    _menuFile->EnableEntry(M_FILE_REOPEN);
    _menuFile->EnableEntry(M_FILE_PRINT);
    _menuPage->EnableEntry(M_PAGE_NEXT);
  } else {
    _menuFile->DisableEntry(M_FILE_REOPEN);
    _menuFile->DisableEntry(M_FILE_PRINT);
    _menuPage->DisableEntry(M_PAGE_NEXT);
  }
  _menuFile->DisableEntry(M_FILE_SAVEMARKED);

  UpdateStatus();

  if (output_dlg) output_dlg->Clear();
}

void OXMain::_NewFile(int n, int home) {
  _SetupGhostview();
  if (home) {
    _canvas->SetHPos(0);
    _canvas->SetVPos(0);
  }
}

void OXMain::_ShowPage(int n) {
  struct stat sbuf;
  int i;

  if (!_filename) return;

  // If the file has changed, rescan it so that offsets into the file
  // are still correct. If the file is rescanned, we must setup ghostview
  // again. Also, force a new copy of ghostscript to start.
  if (_psFile) {
    if (!stat(_full_filename, &sbuf) && (mtime != sbuf.st_mtime)) {
      fclose(_psFile);
      _psFile = fopen(_full_filename, "r");
      if (!_psFile) {
        ErrorMsg(MB_ICONSTOP, "Could not reopen file.");
        return;
      }
      mtime = sbuf.st_mtime;
      _NewFile(n, False);
    }
  }

  // coerce page number to fall in range
  if (has_toc) {
    if (n >= doc->numpages) n = doc->numpages - 1;
    if (n < 0) n = 0;
  }

  _SetOrientation(n);
  _SetPagemedia(n);

  if (has_toc && doc && doc->pages) {
    OXPageLBEntry *e;

    e = (OXPageLBEntry *) _lb->FindEntry(current_page);
    if (e) e->SetMark(PLBE_RADIOMARK, False);

    current_page = n;
    _gv->EnableInterpreter();
    _gv->SendPS(_psFile, doc->beginprolog, doc->lenprolog, False);
    _gv->SendPS(_psFile, doc->beginsetup, doc->lensetup, False);
    if (doc->pageorder == PS_DESCEND) {
      i = (doc->numpages-1)-current_page;
    } else {
      i = current_page;
    }

    _gv->SendPS(_psFile, doc->pages[i].begin, doc->pages[i].len, False);
    e = (OXPageLBEntry *) _lb->Select(current_page);
    if (e) e->SetMark(PLBE_RADIOMARK, True);

    _menuPage->EnableEntry(M_PAGE_REDISPLAY);

    if (current_page != 0)
      _menuPage->EnableEntry(M_PAGE_PREVIOUS);
    else
      _menuPage->DisableEntry(M_PAGE_PREVIOUS);

    if (current_page != doc->numpages-1)
      _menuPage->EnableEntry(M_PAGE_NEXT);
    else
      _menuPage->DisableEntry(M_PAGE_NEXT);

  } else {
    if (!_gv->IsInterpreterRunning())
      _gv->EnableInterpreter();
    else if (_gv->IsInterpreterReady())
      _gv->NextPage();
    else
      XBell(GetDisplay(), 0);
  }
}

// Build the Page Media menu, it has two parts:
//  - the document defined page medias
//  - the standard page media defined from Adobe's PDD

void OXMain::_BuildPagemediaMenu() {
  int i;

  _mediaPopup->RemoveAllEntries();

  base_papersize = 0;
  if (doc) base_papersize = doc->nummedia;

  if (doc && doc->nummedia) {
    for (i=0; i<doc->nummedia; ++i) {
      _mediaPopup->AddEntry(new OHotString(doc->media[i].name),
                            M_PAGEMEDIA + i);
    }
    _mediaPopup->AddSeparator();
  }
  for (i=0; papersizes[i].name; ++i) {
    if (i > 0) {
      // skip over same paper size with small imageable area
      if ((papersizes[i].width  == papersizes[i-1].width)  &&
          (papersizes[i].height == papersizes[i-1].height))
        continue;
    }
    _mediaPopup->AddEntry(new OHotString(papersizes[i].name),
                          M_PAGEMEDIA + i+base_papersize);
  }
  last_pagemedia_entry = M_PAGEMEDIA + i+base_papersize -1;
}

void OXMain::_SetOrientation(int n) {
  int psorient;
  XCPageOrientation gvorient;

  psorient = PS_NONE;
  if (default_document_orientation) {
    if (doc) {
      if (doc->pages) psorient = doc->pages[n].orientation;
      if (psorient == PS_NONE) psorient = doc->default_page_orientation;
      if (psorient == PS_NONE) psorient = doc->orientation;
    }
  }

  switch (psorient) {
    default:
    case PS_PORTRAIT:
      gvorient = PageOrientationPortrait;
      break;

    case PS_LANDSCAPE:
      gvorient = swap_landscape ? PageOrientationSeascape :
                                  PageOrientationLandscape;
      break;

    case PS_NONE:
      gvorient = current_orientation;
      break;
  }

  current_orientation = gvorient;

  if (_gv->SetOrientation(gvorient)) {
    _canvas->Layout();
  }

  switch (current_orientation) {
    default:
    case PageOrientationPortrait:
      psorient = M_ORIEN_PORTRAIT;
      break;

    case PageOrientationLandscape:
      psorient = M_ORIEN_LANDSCAPE;
      break;

    case PageOrientationUpsideDown:
      psorient = M_ORIEN_UPDOWN;
      break;

    case PageOrientationSeascape:
      psorient = M_ORIEN_SEASCAPE;
      break;
  }
  _orienPopup->RCheckEntry(psorient,
                           M_ORIEN_PORTRAIT,
                           M_ORIEN_SEASCAPE);
}

void OXMain::_SetPagemedia(int n, int media) {
  int llx, lly, urx, ury;
  int new_pagemedia;

  if (doc && doc->epsf &&
      (doc->boundingbox[URX] > doc->boundingbox[LLX]) &&
      (doc->boundingbox[URY] > doc->boundingbox[LLY])) {

    llx = doc->boundingbox[LLX];
    lly = doc->boundingbox[LLY];
    urx = doc->boundingbox[URX];
    ury = doc->boundingbox[URY];

  } else {

    if (media > 0) {
      new_pagemedia = media;
    } else if (default_document_pagemedia &&
               doc && doc->pages && doc->pages[n].media) {
      new_pagemedia = doc->pages[n].media - doc->media;
    } else if (default_document_pagemedia &&
               doc && doc->default_page_media) {
      new_pagemedia = doc->default_page_media - doc->media;
    } else {
      new_pagemedia = current_pagemedia;
    }

    _mediaPopup->RCheckEntry(M_PAGEMEDIA+new_pagemedia,
                             M_PAGEMEDIA,
                             last_pagemedia_entry);

    current_pagemedia = new_pagemedia;

    llx = lly = 0;
    if (doc && doc->media && new_pagemedia < base_papersize) {
      urx = doc->media[new_pagemedia].width;
      ury = doc->media[new_pagemedia].height;
    } else {
      urx = papersizes[new_pagemedia-base_papersize].width;
      ury = papersizes[new_pagemedia-base_papersize].height;
    }
  }

  if (_gv->SetBoundingBox(llx, lly, urx, ury)) {
    _canvas->Layout();
  }
}

//--- Count marked pages

int OXMain::_NumMarkedPages() {
  int i, pages;

  if (!toc || !doc) return 0;

  for (pages = 0, i = 0; i < doc->numpages; i++)
    if (toc[i]->GetFlags() & PLBE_CHECKMARK) pages++;

  return pages;
}

// length calculates string length at compile time,
// can only be used with character constants

#define length(a) (sizeof(a)-1)

//--- Write the headers, marked pages, and trailer to the specified file

void OXMain::_WriteDoc(FILE *fp) {
  FILE *psfile;
  char text[PS_LINELENGTH];
  char *comment;
  int pages_written = False;
  int pages_atend = False;
  int pages = 0;
  int page = 1;
  int i, j;
  long here;

  if (!toc) return;

  pages = _NumMarkedPages();

  if (!pages) return;

  psfile = fopen(_full_filename, "r");
  if (!psfile) return;

  here = doc->beginheader;
  while (comment = pscopyuntil(psfile, fp, here,
                               doc->endheader, "%%Pages:")) {
    here = ftell(psfile);
    if (pages_written || pages_atend) {
      free(comment);
      continue;
    }
    sscanf(comment+length("%%Pages:"), "%s", text);
    if (strcmp(text, "(atend)") == 0) {
      fputs(comment, fp);
      pages_atend = True;
    } else {
      switch (sscanf(comment+length("%%Pages:"), "%*d %d", &i)) {
        case 1:
          fprintf(fp, "%%%%Pages: %d %d\n", pages, i);
          break;
        default:
          fprintf(fp, "%%%%Pages: %d\n", pages);
          break;
      }
      pages_written = True;
    }
    free(comment);
  }
  pscopy(psfile, fp, doc->beginpreview, doc->endpreview);
  pscopy(psfile, fp, doc->begindefaults, doc->enddefaults);
  pscopy(psfile, fp, doc->beginprolog, doc->endprolog);
  pscopy(psfile, fp, doc->beginsetup, doc->endsetup);

  for (i=0; i<doc->numpages; i++) {
    if (doc->pageorder == PS_DESCEND) 
      j = (doc->numpages - 1) - i;
    else
      j = i;
    if (toc[j]->GetFlags() & PLBE_CHECKMARK) {
      comment = pscopyuntil(psfile, fp, doc->pages[i].begin,
                            doc->pages[i].end, "%%Page:");
      fprintf(fp, "%%%%Page: %s %d\n",
                  doc->pages[i].label, page++);
      free(comment);
      pscopy(psfile, fp, -1, doc->pages[i].end);
    }
  }

  here = doc->begintrailer;
  while (comment = pscopyuntil(psfile, fp, here,
                               doc->endtrailer, "%%Pages:")) {
    here = ftell(psfile);
    if (pages_written) {
      free(comment);
      continue;
    }
    switch (sscanf(comment+length("%%Pages:"), "%*d %d", &i)) {
      case 1:
        fprintf(fp, "%%%%Pages: %d %d\n", pages, i);
        break;
      default:
        fprintf(fp, "%%%%Pages: %d\n", pages);
        break;
    }
    pages_written = True;
    free(comment);
  }

  fclose(psfile);
}

//-----------------------------------------------------------------------------

void OXMain::_GrabKeys() {
  static int keys[] = { XK_Left, XK_KP_Left, XK_Right, XK_KP_Right,
                        XK_Up, XK_KP_Up, XK_Down, XK_KP_Down,
                        XK_Home, XK_KP_Home, XK_End, XK_KP_End,
                        XK_Page_Up, XK_KP_Page_Up,
                        XK_Page_Down, XK_KP_Page_Down,
                        XK_plus, XK_equal, XK_KP_Add,
                        XK_minus, XK_KP_Subtract };

  for (int i = 0; i < sizeof(keys) / sizeof(keys[0]); ++i) {
    int keycode = XKeysymToKeycode(GetDisplay(), keys[i]);
    XGrabKey(GetDisplay(), keycode, AnyModifier, _id, True,
             GrabModeAsync, GrabModeAsync);
  }
}

// HandleKey:
//		A simple attempt to make xcgview a finished app.

int OXMain::HandleKey(XKeyEvent *event) {
  char tmp[10];
  KeySym keysym;
  XComposeStatus compose = { NULL, 0 };
  int n, po;

  if (event->type == KeyPress) {
    n = XLookupString(event, tmp, sizeof(tmp)-1, &keysym, &compose);
    tmp[n] = 0;
    switch (keysym) {
      case XK_Left:
      case XK_KP_Left:
    	po = -((OXViewPort *)_canvas->GetViewPort())->GetHPos();
        po -= 10;
	if (po < 0) po = 0;
        _canvas->SetHPos(po);
        _client->NeedRedraw(_canvas->GetViewPort());
      	break;

      case XK_Right:
      case XK_KP_Right:
    	po = -((OXViewPort *)_canvas->GetViewPort())->GetHPos();
        po += 10;
	if (po > _container->GetWidth()-_canvas->GetViewPort()->GetWidth())
	  po = _container->GetWidth()-_canvas->GetViewPort()->GetWidth();
	_canvas->SetHPos(po);
	_client->NeedRedraw(_canvas->GetViewPort());
    	break;

      case XK_Home:
      case XK_KP_Home:
    	_canvas->SetVPos(0);
	_client->NeedRedraw(_canvas->GetViewPort());
    	break;

      case XK_End:
      case XK_KP_End:
        _canvas->SetVPos(_container->GetHeight() - _canvas->GetViewPort()->GetHeight());
	_client->NeedRedraw(_canvas->GetViewPort());
    	break;

      case XK_Up:
      case XK_KP_Up:
    	po = -((OXViewPort *)_canvas->GetViewPort())->GetVPos();
        po -= 10;
	if (po < 0) po = 0;
        _canvas->SetVPos(po);
	_client->NeedRedraw(_canvas->GetViewPort());
    	break;

      case XK_Down:
      case XK_KP_Down:
    	po = -((OXViewPort *)_canvas->GetViewPort())->GetVPos();
        po += 10;
	if (po > _container->GetHeight()-_canvas->GetViewPort()->GetHeight())
	  po = _container->GetHeight()-_canvas->GetViewPort()->GetHeight();
	_canvas->SetVPos(po);
	_client->NeedRedraw(_canvas->GetViewPort());
	break;

      case XK_Page_Up:
      case XK_KP_Page_Up:
#if 1
    	po = -((OXViewPort *)_canvas->GetViewPort())->GetVPos();
        if (po > 0) {
          po -= _canvas->GetViewPort()->GetHeight() - 20;
          if (po < 0) po = 0;
          _canvas->SetVPos(po);
          _client->NeedRedraw(_canvas->GetViewPort());
        } else {
          if (DoPagePrev()) {
            po = _container->GetHeight()-_canvas->GetViewPort()->GetHeight();
            _canvas->SetVPos(po);
          }
        }
#else
    	DoPagePrev();
#endif
	break;

      case XK_Page_Down:
      case XK_KP_Page_Down:
#if 1
    	po = -((OXViewPort *)_canvas->GetViewPort())->GetVPos();
        if (po < _container->GetHeight()-_canvas->GetViewPort()->GetHeight()) {
          po += _canvas->GetViewPort()->GetHeight() - 20;
          if (po > _container->GetHeight()-_canvas->GetViewPort()->GetHeight())
            po = _container->GetHeight()-_canvas->GetViewPort()->GetHeight();
          _canvas->SetVPos(po);
          _client->NeedRedraw(_canvas->GetViewPort());
        } else {
          if (DoPageNext()) {
            po = 0;
            _canvas->SetVPos(po);
          }
        }
#else
    	DoPageNext();
#endif
	break;

      case XK_plus:
      case XK_equal:
      case XK_KP_Add:
        DoSetMagstep(mag_step+1);
    	break;

      case XK_minus:
      case XK_KP_Subtract:
        DoSetMagstep(mag_step-1);
   	break;

      default:
        return OXMainFrame::HandleKey(event);
    }

    return True;
  }

  return OXMainFrame::HandleKey(event);
}
