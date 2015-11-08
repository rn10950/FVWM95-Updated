/**************************************************************************

    This program acts as a X shell for the 'find' command line utility.
    Copyright (C) 2001, Michael Gibson, Hector Peraza.

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

#include <xclass/OXMainFrame.h>
#include <xclass/OXMenu.h>
#include <xclass/OXTab.h>
#include <xclass/OXIcon.h>
#include <xclass/OXLabel.h>
#include <xclass/OXTextButton.h>
#include <xclass/OXCheckButton.h>
#include <xclass/OXTextEntry.h>
#include <xclass/OXComboBox.h>
#include <xclass/OXListView.h>
#include <xclass/OXStatusBar.h>
#include <xclass/OFileHandler.h>
#include <xclass/OExec.h>
#include <xclass/OTimer.h>
#include <xclass/utils.h>


//----------------------------------------------------------------------

class OXMain : public OXMainFrame {
public:
  OXMain(const OXWindow *p, int w, int h);
  virtual ~OXMain();

  virtual int ProcessMessage(OMessage *msg);
  virtual int HandleTimer(OTimer *t);
  virtual int HandleFileEvent(OFileHandler *fh, unsigned int mask);

protected:
  void StartSearch();
  void StopSearch();

  void AddFileName();
  
  void StartAnimation();
  void StopAnimation();

  OXPopupMenu *_MakePopup(struct _popup *);
  
  OXStatusBar *_statusBar;
  OXMenuBar *_menuBar;
  OXPopupMenu *_menuFile, *_menuEdit, *_menuView, *_menuHelp;

  OLayoutHints *_menuBarLayout, *_menuBarItemLayout;

  OFileHandler *_ifh, *_ofh, *_efh;
  OExec *_exec;
  OString *_cmd, *_output;
  int _exited, _retc;

  OLayoutHints *CommonCLayout, *CommonXLayout;
  OLayoutHints *CommonLLayout, *CommonRLayout;
  OLayoutHints *TabLayout;

  OXTab *_tab;

  OXCheckButton *Subfolders;
  OXIcon *mainicon;
  OXListView *Output;
  OXComboBox *SearchIn;

  OTimer *_animTimer;
  const OPicture *_animation[24];
  int _animIndex;

  OXTextEntry *TextE, *TextU, *TextG;
  OXTextEntry *Text2, *Text3, *Text4;
  OXButton *Find, *Stop, *NewSearch;
  OLayoutHints *ButtonLayout, *ButtonFrameLayout;

  int _nitems;
};


#endif  // __MAIN_H
