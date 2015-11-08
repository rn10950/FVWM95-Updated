#ifndef __OXVIEWLOGFILE_H
#define __OXVIEWLOGFILE_H

#include <xclass/OXClient.h>
#include <xclass/OXTransientFrame.h>
#include <xclass/OXMenu.h>
#include <xclass/OXToolBar.h>
#include <xclass/OXStatusBar.h>
#include <xclass/OXMsgBox.h>
#include <xclass/OX3dLines.h>

#include "OXViewDoc.h"
#include "OTextDoc.h"

class OXIrc;


//---------------------------------------------------------------------

class OXViewLogFile : public OXTransientFrame {
public:
  OXViewLogFile(const OXWindow *p, const OXWindow *main, OXIrc *irc,
                const char *fname = NULL);
  virtual ~OXViewLogFile();

  virtual int CloseWindow();
  virtual int ProcessMessage(OMessage *msg);

  int Load(const char *fname);

protected:
  OTextDoc *_log;
  OXViewDoc *_logw;
  OXIrc *_irc;
  
  OXMenuBar *_menuBar;
  OXPopupMenu *_menuFile, *_menuEdit, *_menuView, *_menuHelp;
  OXToolBar *_toolBar;
  OXStatusBar *_statusBar;
};


#endif  // __OXVIEWLOGFILE_H
