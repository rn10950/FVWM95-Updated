#include <stdlib.h>
#include <stdio.h>

#include <xclass/OXClient.h>
#include <xclass/OXWindow.h>
#include <xclass/OXMainFrame.h>
#include <xclass/OXStatusBar.h>
#include <xclass/OXMenu.h>
#include <xclass/OXLabel.h>
#include <xclass/OXMsgBox.h>
#include <xclass/OString.h>
#include <xclass/utils.h>

#define M_FILE_OPEN        1
#define M_FILE_PRINT       2
#define M_FILE_PRINTSETUP  3
#define M_FILE_EXIT        4

#define M_EDIT_COPY        5
#define M_EDIT_CUT         6
#define M_EDIT_PASTE       7

#define M_VIEW_STATUSBAR   8
#define M_VIEW_CHANGEFONT  9

#define M_HELP_CONTENTS   10
#define M_HELP_SEARCH     11
#define M_HELP_ABOUT      12


//---------------------------------------------------------------------

class OXAppMainFrame : public OXMainFrame {
public:
  OXAppMainFrame(const OXWindow *p, int w, int h);
  virtual ~OXAppMainFrame();

  void LoadFile(char *filename);
  virtual int ProcessMessage(OMessage *msg);

protected:
  OXStatusBar *_status;
  OXTextView *_tv;

  OLayoutHints *_menuBarLayout, *_menuBarItemLayout;

  OXMenuBar *_menuBar;
  OXPopupMenu *_menuFile, *_menuEdit, *_menuView, *_menuHelp;
};

