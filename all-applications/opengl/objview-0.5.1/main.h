/**************************************************************************

    This is an example xclass OpenGL application.
    Copyright (C) 2000, 2001, Hector Peraza.

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

#include <stdlib.h>
#include <stdio.h>

#include <xclass/utils.h>
#include <xclass/OXClient.h>
#include <xclass/OXMainFrame.h>
#include <xclass/OXToolBar.h>
#include <xclass/OXStatusBar.h>
#include <xclass/OXMenu.h>
#include <xclass/OX3dLines.h>

class GLView;


//----------------------------------------------------------------------

class MainFrame : public OXMainFrame {
public:
  MainFrame(const OXWindow *p, int w, int h);
  virtual ~MainFrame();

  virtual int ProcessMessage(OMessage *msg);
  virtual int CloseWindow();

  void SetWindowTitle(char *title);
  void UpdateStatus();

  void ReadFile(char *fname);
  void WriteFile(char *fname);

  void DoOpen();
  void DoSave(char *fname);
  void DoPrint();
  void DoAbout();
  void DoToggleToolBar();
  void DoToggleStatusBar();

protected:
  OXPopupMenu *_MakePopup(struct _popup *);

  OXToolBar *_toolBar;
  OXStatusBar *_statusBar;
  OXHorizontal3dLine *_toolBarSep;
  GLView *_glView;

  OLayoutHints *_menuBarLayout, *_menuBarItemLayout;

  OXMenuBar *_menuBar;
  OXPopupMenu *_menuFile, *_menuEdit, *_menuView, *_menuHelp;
};


#endif  // __MAIN_H
