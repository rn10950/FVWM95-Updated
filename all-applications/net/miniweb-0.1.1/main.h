#ifndef __MAIN_H
#define __MAIN_H

#include <vector>

#include <xclass/utils.h>
#include <xclass/OXMainFrame.h>
#include <xclass/OXMenu.h>
#include <xclass/OXTextEdit.h>
#include <xclass/OXComboBox.h>
#include <xclass/OX3dLines.h>
#include <xclass/OXToolBar.h>
#include <xclass/OXStatusBar.h>
#include <xclass/ORectangle.h>

#include "OXWebHtml.h"

#define APP_NAME          "miniweb"
#define APP_VERSION       "0.1.1"

#define M_FILE_OPEN       101
#define M_FILE_SAVE       102
#define M_FILE_PRINT      103
#define M_FILE_EXIT       104

#define M_EDIT_CUT        201
#define M_EDIT_COPY       202
#define M_EDIT_PASTE      203

#define M_VIEW_TOOLBAR    301
#define M_VIEW_STATUSBAR  302
#define M_VIEW_PREVPAGE   303
#define M_VIEW_NEXTPAGE   304
#define M_VIEW_UNDERLINE  305
#define M_VIEW_SOURCE     306

#define M_HELP_CONTENTS   401
#define M_HELP_SEARCH     402
#define M_HELP_ABOUT      403

#define C_LOCATION        501

#define NUM_RECENT        15


//----------------------------------------------------------------------

class OXMain : public OXMainFrame {
public:
  OXMain(const OXWindow *p, char *fname);
  ~OXMain();

  virtual int CloseWindow();
  virtual int HandleMapNotify(XMapEvent *event);
  virtual int ProcessMessage(OMessage *msg);

  void DoOpen();
  void DoViewSource();
  void DoToggleToolBar();
  void DoToggleStatusBar();
  void DoToggleUnderlineLinks();
  void DoPrevPage();
  void DoNextPage();
  void DoAbout();

protected:
  void LoadDoc(OHtmlUri *uri);
  void UpdateButtons();
  void ReadIniFile();
  void SaveIniFile();
  void AddToHistory(const char *url);
  void UpdateHistory();

  OXWebHtml *_htmlview;
  OXMenuBar *_menuBar;
  OXPopupMenu *_menuFile, *_menuEdit, *_menuView, *_menuHelp;
  OXToolBar *_toolBar;
  OXHorizontal3dLine *_toolBarSep;
  OXStatusBar *_statusBar;
  OXComboBox *_location;
  OLayoutHints *_menuBarLayout, *_menuBarItemLayout,
               *_viewLayout, *_statusLayout;

  char *_loadDoc, *_filename, *_lastUrl;
  char *_history[NUM_RECENT];

  ORectangle _geom;

  std::vector<char *> prev, next;
};


#endif  // __MAIN_H
