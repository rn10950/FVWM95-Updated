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

#ifndef __OXCOMPOSER_H
#define __OXCOMPOSER_H

#include <X11/Xlib.h>

#include <xclass/OXTransientFrame.h>
#include <xclass/OXMenu.h>
#include <xclass/OString.h>   
#include <xclass/OX3dLines.h>   
#include <xclass/OXListView.h>
#include <xclass/OTimer.h>
#include <xclass/OXListView.h>
#include <xclass/OXTextEdit.h>

#include "OMailList.h"
#include "OSmtp.h"
#include "OSession.h"
#include "OXSearchBox.h"

#define COMPOSER              0
#define REPLY                 1
#define FORWARD               2

#define CMP_COPY_ATTACH       15
#define CMP_PROP_ATTACH       16
#define CMP_DEL_ATTACH        17

#define CMP_FILE_SEND_NOW     1
#define CMP_FILE_SEND_LATER   2
#define CMP_FILE_SAVE         3
#define CMP_FILE_PRINT        4
#define CMP_FILE_INCLUDE      5
#define CMP_FILE_EXIT         6

#define CMP_EDIT_CUT          7
#define CMP_EDIT_COPY         8
#define CMP_EDIT_PASTE        9
#define CMP_EDIT_DELETE       10
#define CMP_EDIT_SELECTALL    11
#define CMP_SEARCH_SEARCH     12
#define CMP_SEARCH_AGAIN      13

#define CMP_HELP_CONTENTS     14
#define CMP_HELP_SEARCH       15
#define CMP_HELP_ABOUT        16

typedef struct attList_Type {
 char *file;
 char *name;
 char type[256];
 OListViewItem *item;
 struct attList_Type *prev, *next;
} CmpAttachList;


class OXComposer : public OXTransientFrame, OSmtp {
public:
  OXComposer(const OXWindow *p, const OXWindow *main, int w, int h,
             OSession *session, int state, OMail *mail = NULL);
  virtual ~OXComposer();

  virtual int ProcessMessage(OMessage *msg);
  virtual int HandleTimer(OTimer *t);

protected:
  OXPopupMenu *attachPopUp;
  OMimeTypes *MimeTypeList;
  CmpAttachList  *attList, *currentAttach;
  search_struct search;
  OTimer *timer;
  bool showAttachments;
  OXHorizontal3dLine *_line, *_toolBarLine;
  OLayoutHints *_menuBarItemLayout, *_menuBarLayout;
  OXMenuBar *_menuBar;
  OXPopupMenu *_menuFile, *_menuEdit, *_menuSearch, *_menuHelp;
  OMail *_mail;
  OSession *_session;
  OText *_body;
  OLayoutHints *flayout, *telayout, *lb, *lt, *ll;
  OXTextEdit *te;
  OXCheckButton *toall;
  OXButton *toButton, *copyButton, *attachButton;
  OXCompositeFrame *_toolBar;
  OXPictureButton *sendNowButton, *sendLaterButton, *printButton, *saveButton, 
                  *includeFileButton;
  OXTextEntry *to, *subject, *copy;
  OXLabel *lto, *lsubject, *lcopy;
  OXCompositeFrame *f1, *f2, *f3, *f4, *f6, *f7, *f8;
  OTextBuffer *bto, *bsubject, *bcopy, *battach;
  OXVerticalFrame *attachments;
  OXCompositeFrame *attachmentsHeader;
  OXListView *attachmentsView;
  OXPictureButton *attButton;
  const OPicture *attPix;
  char *_my_cc;
  char *_bcopy_backup;

  void SaveSource() {};
  void PrintSource() {};   
  void InitMenu();   
  void Send(int mode);
  char *Sender();
  bool EncodeBase64(FILE *f, FILE *g);
  unsigned char ToASCII64(unsigned char character);
  bool SetCurrentAttachment();
  void Search(int ret);
  void AddAttach(char *name=NULL);
  void DelAttach();
  char *GenerateContentType(CmpAttachList *item);
  char *GenerateDate();
  char *GenerateMessageId();
  void InitToolBar();
  void SaveMail();
  void PrintMail();
  void IncludeFile();
  void GenerateMessagePlusHeader(OText *msg);
};


#endif  // __OXCOMPOSER_H
