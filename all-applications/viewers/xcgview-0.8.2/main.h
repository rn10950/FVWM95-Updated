#ifndef __MAIN_H
#define __MAIN_H

#include <stdlib.h>
#include <stdio.h>

#include <xclass/OXClient.h>
#include <xclass/OXWindow.h>
#include <xclass/OXFrame.h>
#include <xclass/OXMenu.h>
#include <xclass/OX3dLines.h>
#include <xclass/OXResizer.h>
#include <xclass/OXToolBar.h>
#include <xclass/OXStatusBar.h>
#include <xclass/OXMsgBox.h>
#include <xclass/OString.h>
#include <xclass/OIniFile.h>
#include <xclass/utils.h>

#include "ps.h"


//----------------------------------------------------------------------

#define XCGVIEW_VERSION   "0.8.2"
#define XCGVIEW_INI       "xcgviewrc"


#define MENU_DISABLED     (1<<0)
#define MENU_CHECKED      (1<<1)
#define MENU_RCHECKED     (1<<2)


#define M_FILE_OPEN        101
#define M_FILE_REOPEN      102
#define M_FILE_SAVEMARKED  103
#define M_FILE_PRINT       104
#define M_FILE_PRINTSETUP  105
#define M_FILE_EXIT        106
#define M_FILE_RECENT      107

#define M_VIEW_TOOLBAR     201
#define M_VIEW_STATUSBAR   202
#define M_VIEW_PAGEINDEX   203
#define M_VIEW_MAGSTEP     204
#define M_VIEW_ORIENTATION 205
#define M_VIEW_MEDIA       206
#define M_VIEW_OUTPUT      207

#define M_MAGSTEP_M5           211
#define M_MAGSTEP_M4           212
#define M_MAGSTEP_M3           213
#define M_MAGSTEP_M2           214
#define M_MAGSTEP_M1           215
#define M_MAGSTEP_0            216
#define M_MAGSTEP_P1           217
#define M_MAGSTEP_P2           218
#define M_MAGSTEP_P3           219
#define M_MAGSTEP_P4           220
#define M_MAGSTEP_P5           221

#define M_ORIEN_PORTRAIT       241
#define M_ORIEN_LANDSCAPE      242
#define M_ORIEN_UPDOWN         243
#define M_ORIEN_SEASCAPE       244
#define M_ORIEN_DEFAULT        245
#define M_ORIEN_SWAPLANDSCAPE  246

// this one should have the highest menu ID
#define M_PAGEMEDIA           1000  

#define M_ZOOM_IN              291
#define M_ZOOM_OUT             292

#define M_PAGE_NEXT        301
#define M_PAGE_PREVIOUS    302
#define M_PAGE_REDISPLAY   303
#define M_PAGE_MARK        304
#define M_PAGE_UNMARK      305

#define M_HELP_CONTENTS    401
#define M_HELP_SEARCH      402
#define M_HELP_ABOUT       403


#define GV_MAIN            800
#define LB_PAGE            801


#define NUM_RECENT  5

struct _popup {
  OXPopupMenu *ptr;
  struct {
    char *name;
    int  id, state;
    struct _popup *popup_ref;
  } popup[20];
};


//----------------------------------------------------------------------

class OXMain : public OXMainFrame {
public:
  OXMain(const OXWindow *p, int w, int h);
  virtual ~OXMain();

  virtual int HandleKey(XKeyEvent *event);
  virtual int ProcessMessage(OMessage *msg);
  virtual int CloseWindow();

  void SetWindowTitle(const char *title);
  void UpdateStatus();
  void OpenFile(const char *fname, int die = False);
  void DoOpen();
  void DoReopen();
  void DoSaveMarked();
  void DoPrint();
  void DoRefresh();
  int  DoPagePrev();
  int  DoPageNext();
  void DoMark();
  void DoUnmark();
  void DoToggleDefaultOrientation();
  void DoSwapLandscape();
  void DoSetOrientation(int orien);
  void DoSetPageMedia(int media);
  void DoSetMagstep(int mstep);
  void DoViewOutput();
  void DoToggleToolBar();
  void DoTogglePageIndex();
  void DoToggleStatusBar();
  void DoHelpAbout();

  void ErrorMsg(int icon_type, char *msg);

protected:
  void _SetupGhostview();
  void _NewFile(int n, int home = True);
  void _ShowPage(int n);
  void _BuildPagemediaMenu();
  void _SetOrientation(int n);
  void _SetPagemedia(int n, int media = -1);
  int  _NumMarkedPages();
  void _WriteDoc(FILE *fp);

  void _ReadIniFile();
  void _SaveIniFile();
  void _AddToRecent(const char *fname);
  void _UpdateRecentList();
  void _GrabKeys();
  OXPopupMenu *_MakePopup(struct _popup *);

  OXCompositeFrame *_hf;
  OXToolBar *_toolBar;
  OXHorizontal3dLine *_toolBarSep;
  OXStatusBar *_statusBar;
  OXVerticalResizer *_resizer;
  OXCanvas *_canvas;
  OXCompositeFrame *_container;
  OXGhostView *_gv;
  OXListBox *_lb;

  OLayoutHints *_menuBarLayout, *_menuBarItemLayout;

  OXMenuBar *_menuBar;
  OXPopupMenu *_menuFile, *_menuView, *_menuPage, *_menuHelp,
              *_magstepPopup, *_orienPopup, *_mediaPopup;
              
  FILE *_psFile;
  struct document *doc;
  int current_page, current_pagemedia, swap_landscape;
  int default_document_pagemedia, default_document_orientation;
  int last_pagemedia_entry;
  int has_toc, mag_step, base_papersize;
  XCPageOrientation current_orientation;

  char *_filename, *_full_filename;
  char *_recent_files[NUM_RECENT];
  char *_printerName, *_printProg;
  char *_gsCommand;
  
  time_t mtime;
  int _winx, _winy;
  
  OXPageLBEntry **toc;
};

#endif  // __MAIN_H
