/**************************************************************************

    This file is part of xclass.
    Copyright (C) 2004, Hector Peraza.                 

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

#ifndef __OXHELPWINDOW_H
#define __OXHELPWINDOW_H

#include <vector>

#include <xclass/utils.h>
#include <xclass/OXTransientFrame.h>
#include <xclass/OXToolBar.h>
#include <xclass/ORectangle.h>


#define HELP_NUM_RECENT   30

class OHtmlUri;
class OXHelpPopup;
class OXHelpDoc;

//----------------------------------------------------------------------

class OXHelpWindow : public OXTransientFrame {
public:
  OXHelpWindow(const OXWindow *p, const OXWindow *t,
               int w, int h, const char *rootfile, const char *curfile,
               const char *appname);
  ~OXHelpWindow();

  virtual int CloseWindow();
  virtual int HandleMapNotify(XMapEvent *event);
  virtual int ProcessMessage(OMessage *msg);

  void DoContents();
  void DoPrevPage();
  void DoNextPage();
  void DoSetCurrent(const char *curfile);

protected:
  void LoadDoc(OHtmlUri *uri, OXHelpDoc *dst);
  void UpdateButtons();
  void ReadIniFile();
  void SaveIniFile();
  void AddToHistory(const char *url);

  OXHelpDoc *_hview;
  OXHelpPopup *_hpopup;
  OXToolBar *_toolBar;
  OLayoutHints *_viewLayout;

  OXToolBarButton *_cnt, *_bck, *_fwd;

  char *_loadDoc, *_rootfile, *_filename, *_appName, *_lastUrl;
  char *_history[HELP_NUM_RECENT];

  ORectangle _geom;

  std::vector<char *> prev, next;
};


#endif  // __OXHELPWINDOW_H
