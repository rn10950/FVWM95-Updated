/**************************************************************************

    This file is part of notepad, a simple text editor.
    Copyright (C) 1997-2004, Harald Radke, Hector Peraza.

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

#include <xclass/OXClient.h>
#include <xclass/OXWindow.h>
#include <xclass/OXMainFrame.h>
#include <xclass/OXMenu.h>
#include <xclass/OXLabel.h>
#include <xclass/OXMsgBox.h>
#include <xclass/OXStatusBar.h>
#include <xclass/OXTextEdit.h>
#include <xclass/OXHelpWindow.h>
#include <xclass/OString.h>

#define NOTEPAD_VERSION    "0.9.5"

#define M_FILE_NEW         101
#define M_FILE_OPEN        102
#define M_FILE_SAVE        103
#define M_FILE_SAVEAS      104
#define M_FILE_PRINT       105
#define M_FILE_EXIT        106

#define M_EDIT_UNDO        201
#define M_EDIT_CUT         202
#define M_EDIT_COPY        203
#define M_EDIT_PASTE       204
#define M_EDIT_DELETE      205
#define M_EDIT_SELECTALL   206
#define M_EDIT_TIMEDATE    207
#define M_EDIT_LINEBREAK   208
#define M_EDIT_SELFONT     209
#define M_EDIT_SETTABS     210

#define M_SEARCH_FIND      301
#define M_SEARCH_FINDNEXT  302
#define M_SEARCH_GOTO      303

#define M_HELP_CONTENTS    401
#define M_HELP_ABOUT       402


//---------------------------------------------------------------------

class OXMain : public OXMainFrame {
public:
  OXMain(const OXWindow *p, int w, int h);
  virtual ~OXMain();

  void Clear();
  void LoadFile(char *fname = NULL);
  void SaveFile(char *fname);
  void SaveFileAs();
  void Print();
  void Search(int ret);
  void Goto();
  void TimeDate();
  void ShowHelp();
  void About();

  int IsSaved();

  virtual int ProcessMessage(OMessage *msg);
  virtual int HandleKey(XKeyEvent *event);
  virtual int CloseWindow();
  virtual int HandleTimer(OTimer *t);

protected:
  char *filename;
  char *printer, *printProg;
  OTimer *timer;
  OXStatusBar *_status;
  OXTextEdit *_te;
  OLayoutHints *_menuBarLayout, *_menuBarItemLayout;

  OXMenuBar *_menuBar;
  OXPopupMenu *_menuFile, *_menuEdit, *_menuSearch, *_menuHelp;
  OString *_fontName;

  OXHelpWindow *_helpWindow;
  
  int _exiting;
};
