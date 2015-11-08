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

#ifndef __OXBODYVIEW_H
#define __OXBODYVIEW_H

#include <xclass/OXCompositeFrame.h>
#include <xclass/OMimeTypes.h>
#include <xclass/OXListView.h>
#include <xclass/OXMenu.h>
#include <xclass/OXResizer.h>
#include <xclass/OXListView.h>
#include <xclass/OXTextEdit.h>

#include "OMailList.h"
#include "OMimeAtt.h"
#include "OSession.h"

#define VIEW_ATTACH	1
#define SAVE_ATTACH	2
#define COPY_ATTACH	3
#define PROP_ATTACH	4
#define DEL_ATTACH	5


typedef struct list_type {
  OListViewItem *item;
  OMimeAtt *att;
  struct list_type *next;
} AttachList;


//----------------------------------------------------------------------

class OXBodyView : public OXVerticalFrame {
public:
  OXBodyView(const OXWindow *p, OSession *session);
  virtual ~OXBodyView();

  virtual int ProcessMessage(OMessage *msg);

  void Clear();
  void ShowMessage(OMail *mail);
  OXTextEdit *GetMessageView() { return body; }
  OMail *GetMail() { return mail; }
  OText *GetMessageText(); /* { return body->GetText(); } */

  void SaveAttachment(char *filename = NULL);
  void ViewAttachment();
  void ViewBodySource();
  void BodySave();
  void BodyPrint();
  void BodySearch(bool firstTime);

protected:
  bool showAttachments;
  search_struct search;
  OSession *_session;
  long viewedAttachs;
  AttachList *startAttachList;
  int _selected;

  OMimeAtt *currentAttach;
  OXPopupMenu *attachPopUp;
  OMimeTypes *MimeTypeList;
  OMail *mail;
  OXVerticalFrame *attachments, *message;
  OXCompositeFrame *attachmentsHeader, *messageHeader, *attachmentsframe;
  const OPicture *attPix;
  OXPictureButton *attButton; 
  OXTextEdit *body;
  OXListView *attachmentsView;
  OXHorizontalResizer *resizer;
  bool SetCurrentAttachment();
};

#endif  // __OXBODYVIEW_H
