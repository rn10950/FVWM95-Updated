/**************************************************************************

    This file is part of NRX mail, a mail client using the XClass95 toolkit.
    Copyright (C) 1998 by Harald Radke.

    This application is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This application is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

**************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>

#include <xclass/OXClient.h>
#include <xclass/OXWindow.h>
#include <xclass/OXMainFrame.h>
#include <xclass/OXMsgBox.h>
#include <xclass/OXButton.h>
#include <xclass/OString.h>
#include <xclass/OXMenu.h>
#include <xclass/OXFileDialog.h>
#include <xclass/OXToolBar.h>
#include <xclass/OXStatusBar.h>
#include <xclass/OXDDListBox.h>
#include <xclass/OLayout.h>

#include "OSession.h"
#include "OXViewSrc.h"
#include "OXMailList.h"
#include "OFolder.h"
#include "OXBodyView.h"


#define M_FILE_COMPOSER         1
#define M_FILE_SAVE             2
#define M_FILE_PRINT            3
#define M_FILE_CHECKMAIL        4
#define M_FILE_EMPTYTRASH       5
#define M_FILE_SEND_UNSENT      6
#define M_FILE_EXIT             7

#define M_EDIT_COPY             8
#define M_EDIT_SELECTALL        9
#define M_EDIT_SEARCH           10
#define M_EDIT_SEARCHAGAIN      11
#define M_EDIT_PREF	        12

#define M_MESSAGE_DELETE        13
#define M_MESSAGE_REPLY         14
#define M_MESSAGE_FORWARD       15
#define M_MESSAGE_MOVE          16
#define M_MESSAGE_SRC           17
#define M_MESSAGE_ATT           18
#define M_MESSAGE2_SEPWINDOW    19

#define M_FOLDER_NEW            20
#define M_FOLDER_DELETE         21
#define M_FOLDER_CLEAR          22
#define M_FOLDER_PROPERTIES     23

#define M_ADRESS_ADD            24
#define M_ADRESS_DELETE         25
#define M_ADRESS_VIEW           26

#define M_HELP_CONTENTS         27
#define M_HELP_SEARCH           28
#define M_HELP_ABOUT            29

//----------------------------------------------------------------------

class OXAppMainFrame:public OXMainFrame {
public:
  OXAppMainFrame(const OXWindow *p, int w, int h);
  virtual ~OXAppMainFrame();

  virtual int ProcessMessage(OMessage *msg);
  virtual int HandleFocusChange(XFocusChangeEvent *event);

//long GetNumOfHeaders() { return _list->GetNumberOfMails();} 
  void Reload();
  void Update();
  void DeleteMail();
  void MoveMail(long folder);
  void BodySearch(bool firstTime);
  void BodyCopy() { _message->GetMessageView()->Copy(); }
  bool BodySelectAll() { _message->GetMessageView()->SelectAll(); }
  void ChangeCurrentFolder(const char *name);

  virtual void ClearFolder();
  virtual void DeleteFolder();

  void EmptyTrash();
  void ChangeFolderProp(OFolder *folder);
  OFolder *GetCurrentFolder();

protected:
  void SendUnsentMails();
  void SetupMenu();
  void About();
  char *Init();
  char *AbsolutePath(char *name);
  void _ReloadFolderBox(char **names);
  void _SetCounter(int num);

  int _currentMailNum;
  bool _1sttime, _toolBarEnabled;

  OMail *_currentMail;
  OFolder *_currentFolder;
  OSession *session;

  OXMailList *_list;
  OXHorizontal3dLine *_line;
  OXHorizontalResizer *_resizer;
  OXToolBar *_toolBar;
  OXDDListBox *_folderBox;
  OXBodyView *_message;
  OXStatusBar *_status;
  OXMenuBar *_menuBar;
  OXCompositeFrame *bodyFrame;

  OXPopupMenu *_menuMessage, *_menuFolder, *_menuAdressBook, *_menuHelp,
              *_menuFile, *_menuMessage2, *_menuEdit;
  OLayoutHints *_menuBarItemLayout, *_menuBarLayout;
};
