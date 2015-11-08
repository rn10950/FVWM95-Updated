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

#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <xclass/OXFileDialog.h>
#include <xclass/OResourcePool.h>
#include <xclass/OXMsgBox.h>

#include <vector>

#include "OXViewSrc.h"
#include "OXPrintBox.h"
#include "OXSearchBox.h"
#include "OXBodyView.h"

#include "xpm/attachment.xpm"
#include "xpm/nrxmail.s.xpm"
#include "xpm/nrxmail.t.xpm"


//----------------------------------------------------------------------

OXBodyView::OXBodyView(const OXWindow *p, OSession *session) :
  OXVerticalFrame(p, 10, 10) {

  showAttachments = False;
  search.direction = True;
  search.caseSensitive = False;
  search.buffer = NULL;
  _session = session;
  viewedAttachs = 0;
  startAttachList = NULL;
  currentAttach = NULL;
  _selected = 0;

  attachPopUp = new OXPopupMenu(_client->GetRoot());

  attachPopUp->AddEntry(new OHotString("&View"), VIEW_ATTACH);
  attachPopUp->AddEntry(new OHotString("&Save as..."), SAVE_ATTACH);
  attachPopUp->AddSeparator();
  attachPopUp->AddEntry(new OHotString("Copy"), COPY_ATTACH);
  attachPopUp->DisableEntry(COPY_ATTACH);
  attachPopUp->AddSeparator();
  attachPopUp->AddEntry(new OHotString("&Properties..."), PROP_ATTACH);
  attachPopUp->DisableEntry(PROP_ATTACH);

  attachPopUp->Associate(this);

  mail = NULL;
  message = new OXVerticalFrame(this, 10, 10, CHILD_FRAME/* | SUNKEN_FRAME*/);
  messageHeader = new OXCompositeFrame(message, 10, 10);
  messageHeader->AddFrame(new OXLabel(messageHeader, new OString("Message")),
                          new OLayoutHints(LHINTS_LEFT, 1, 1, 1, 1));
  body = new OXTextEdit(message, 10, 10, 1);
  body->SetReadOnly(True);
  body->Associate(this);

  message->AddFrame(messageHeader, new OLayoutHints(LHINTS_EXPAND_X));

  message->AddFrame(body, new OLayoutHints(LHINTS_EXPAND_Y | LHINTS_EXPAND_X));
  AddFrame(message, new OLayoutHints(LHINTS_EXPAND_Y | LHINTS_EXPAND_X));

  attachmentsframe = new OXCompositeFrame(this, 10, 10, VERTICAL_FRAME);
  attachmentsHeader = new OXCompositeFrame(attachmentsframe, 10, 10, HORIZONTAL_FRAME);
  attachments = new OXVerticalFrame(attachmentsframe, 10, 75, FIXED_HEIGHT);

  attPix = _client->GetPicture("attachment.xpm", attachment_xpm);
  attButton = new OXPictureButton((const OXWindow *) attachmentsHeader, attPix, 1);

  attachmentsHeader->AddFrame(attButton, new OLayoutHints(LHINTS_CENTER_Y,
                                         2, 0, 2, 2));
  attButton->Associate(this);

  attachmentsHeader->AddFrame(new OXLabel(attachmentsHeader,
                                  new OString("Attachments")),
	                      new OLayoutHints(LHINTS_CENTER_Y, 1, 1, 1, 1));
  attachmentsframe->AddFrame(attachmentsHeader,
			     new OLayoutHints(LHINTS_EXPAND_X));

  attachmentsView = new OXListView(attachments, 40, 10, HORIZONTAL_FRAME |
				   SUNKEN_FRAME | DOUBLE_BORDER);

  attachmentsView->Associate(this);
  attachmentsView->SetViewMode(LV_LARGE_ICONS);

  attachments->AddFrame(attachmentsView,
			new OLayoutHints(LHINTS_EXPAND_Y | LHINTS_EXPAND_X));
  attachmentsframe->AddFrame(attachments,
			     new OLayoutHints(LHINTS_EXPAND_X));
  AddFrame(attachmentsframe,
	   new OLayoutHints(LHINTS_EXPAND_X, 0, 0, 0, 1));

  MapSubwindows();
  Layout();

  HideFrame(attachmentsframe);

  MimeTypeList = _client->GetResourcePool()->GetMimeTypes();
//  Clear();
}

OXBodyView::~OXBodyView() {
}

void OXBodyView::Clear() {
  AttachList *travel = startAttachList;
  body->Clear();
  mail = NULL;

  attachmentsView->Clear();

  while (travel != NULL) {
    startAttachList = travel->next;
    delete travel;
    travel = startAttachList;
  }

  startAttachList = NULL;
  currentAttach = NULL;
  HideFrame(attachmentsframe);
  Layout();
}

void OXBodyView::ShowMessage(OMail *newmail) {
  OPicture *pic1 = NULL;
  OPicture *pic2 = NULL;
  OMimeAtt *travel;
  OMime *travelmime;
  AttachList *travelAttach;
  std::vector<OString *> names;

  Clear();
  mail = newmail;
  OText *temptext = mail->GetMessagePlusHeader();
#if 0
  body->SetText(temptext);
#else
  {
    int ln;
    body->AutoUpdate(False);
    for (ln = 0; ln < temptext->RowCount(); ++ln) {
      const char *str = temptext->GetLine(OPosition(0, ln));
      if (str) {
        body->InsertText(str);
        body->InsertText("\n");
      }
    }
    body->SetCursorPosition(OPosition(0, 0), True);
    body->AutoUpdate(True);
  }
#endif
  if (mail->IsMultipart()) {
    ShowFrame(attachmentsframe);
    if (showAttachments) {
      attachmentsframe->ShowFrame(attachments);
    } else {
      attachmentsframe->HideFrame(attachments);
      Layout();
    }
    travel = mail->GetMime();
    travel = travel->GetNext();
    while (travel != NULL) {
      travelmime = NULL;
      travelmime = MimeTypeList->NextMime(travelmime);
      while (travelmime != NULL) {
	if (!(strcasecmp(travelmime->type, travel->GetType()))) {
	  travel->SetKnown();
	  break;
	}
	travelmime = MimeTypeList->NextMime(travelmime);
      }
      if (travel->IsKnown()) {
	pic1 = (OPicture *) MimeTypeList->GetIcon(travelmime, False);
	pic2 = (OPicture *) MimeTypeList->GetIcon(travelmime, True);
      } else {
	if (!(strcasecmp(travel->GetType(), "message/rfc822"))) {
	  pic1 = (OPicture *) _client->GetPicture("nrxmail.s.xpm",
					           nrxmail_s_xpm);
	  pic2 = (OPicture *) _client->GetPicture("nrxmail.t.xpm",
					           nrxmail_t_xpm);
	} else {
	  pic1 = (OPicture *) _client->GetPicture("doc.s.xpm");
	  pic2 = (OPicture *) _client->GetPicture("doc.t.xpm");
	}
      }
      if (startAttachList == NULL) {
        names.clear();
        names.push_back(new OString(travel->GetName()));

	startAttachList = new AttachList;
	startAttachList->item = new OListViewItem(attachmentsView, -1, 
                                               pic1, pic2, names,
					       LV_LARGE_ICONS);
	startAttachList->att = travel;
	startAttachList->next = NULL;
	travelAttach = startAttachList;
      } else {
        names.clear();
        names.push_back(new OString(travel->GetName()));

	travelAttach->next = new AttachList;
	travelAttach = travelAttach->next;
	travelAttach->item = new OListViewItem(attachmentsView, -1,
                                            pic1, pic2, names,
					    LV_LARGE_ICONS);
	travelAttach->att = travel;
	travelAttach->next = NULL;
      }
      attachmentsView->AddItem(travelAttach->item);
      travel = travel->GetNext();
      attachmentsView->Layout();

    }
  } else {
    attachmentsView->Clear();
    HideFrame(attachmentsframe);
  }
}

int OXBodyView::ProcessMessage(OMessage *msg) {
  OItemViewMessage *cmsg = (OItemViewMessage *) msg;

  switch (msg->type) {
//  case MSG_VIEW:
//    SendMessage(_parent, msg);
//    break;

  case MSG_LISTVIEW:
    switch (msg->action) {
    case MSG_CLICK:
      if (!SetCurrentAttachment()) return True;
      if (cmsg->button == 3) {
	attachPopUp->PlaceMenu(cmsg->pos.x, cmsg->pos.y, True, True);
      }
      break;

    case MSG_DBLCLICK:
      if (!SetCurrentAttachment()) return True;
      if (!currentAttach->IsKnown())
	SaveAttachment();
      else
	ViewAttachment();
      break;
    }
    break;

  case MSG_BUTTON:
    switch (msg->action) {
    case MSG_CLICK:
      switch (cmsg->id) {
      case 1:
	showAttachments = !showAttachments;
	if (showAttachments) {
	  attachmentsframe->ShowFrame(attachments);
	  Layout();
	} else {
	  attachmentsframe->HideFrame(attachments);
	  Layout();
	}
	break;
      }
      break;
    }
    break;

  case MSG_MENU:
    switch (msg->action) {
    case MSG_CLICK:
      switch (cmsg->id) {
      case VIEW_ATTACH:
	ViewAttachment();
	break;

      case SAVE_ATTACH:
	SaveAttachment();
	break;

      case COPY_ATTACH:
	break;

      case PROP_ATTACH:
	break;
      }
      break;
    }
    break;
  }
  return True;
}

void OXBodyView::SaveAttachment(char *filename) {
  char *fullpath = NULL;
  char *fname = NULL;

  char *name = StrDup(currentAttach->GetName());

  static const char *filetypes[4] = {
    "All files", "*",
    NULL, NULL
  };

  OFileInfo fi;
  char *p, tmp[1024];

  fi.MimeTypesList = MimeTypeList;
  fi.file_types = (char **) filetypes;
  fi.filename = name;

  if (!filename) {
    new OXFileDialog(_client->GetRoot(), _parent, FDLG_SAVE, &fi);
    if (fi.filename != NULL) {
      fname = fi.filename;
      fullpath = new char[strlen(fname) + strlen(fi.ini_dir) + 2];
      sprintf(fullpath, "%s/%s", fi.ini_dir, fname);
      fname = fullpath;
    } else {
      fname = NULL;
    }
  } else {
    fname = filename;
  }
  if (fname) {
    if (!currentAttach->Save(fname)) {
      sprintf(tmp, "Error saving mail to file \"%s\"", fname);
      new OXMsgBox(_client->GetRoot(), _parent, new OString("NRX MAIL"),
		   new OString(tmp), MB_ICONSTOP, ID_OK);
      return;
    }
    if (fullpath != NULL) delete[] fullpath;
  }
}

bool OXBodyView::SetCurrentAttachment() {
  AttachList *travelAttach = startAttachList;
  const OListViewItem *item;
  std::vector<OItem *> items;
  void *iterator = NULL;

  if (attachmentsView->NumSelected() == 0) {
    currentAttach = NULL;
    return False;
  }
  items = attachmentsView->GetSelectedItems();
  item = (OListViewItem *) items[0];
  while (travelAttach != NULL) {
    if (travelAttach->item == item) {
      currentAttach = travelAttach->att;
      break;
    }
    travelAttach = travelAttach->next;
  }
  if (travelAttach == NULL) {
    return False;
  }
  if (!currentAttach->IsKnown())
    attachPopUp->DisableEntry(VIEW_ATTACH);
  else
    attachPopUp->EnableEntry(VIEW_ATTACH);
  return True;
}

void OXBodyView::BodySearch(bool firstTime) {
  int ret;
  char tmp[1024];

  if (mail == NULL) return;
  if ((firstTime == True) || (search.buffer == NULL))
    new OXSearchBox(_client->GetRoot(), _parent, 400, 150, &search, &ret);

  if (ret == True)
    if (!(GetMessageView()->Search(search.buffer, search.direction,
                                   search.caseSensitive))) {
      sprintf(tmp, "Couldn't find \"%s\"", search.buffer);
      new OXMsgBox(_client->GetRoot(), _parent, new OString("NRX Mail"),
		   new OString(tmp), MB_ICONEXCLAMATION, ID_OK);
    }
}

void OXBodyView::ViewBodySource() {
  OText *src = mail->GetMessageSrc();
  new OXViewSrc(_client->GetRoot(), _parent, 500, 400,
		_session->GetPrintCommand(), _session->GetPrinter(), src);
}

void OXBodyView::BodySave() {
  char *fullpath = NULL;

  static const char *filetypes[8] = {
    "All files", "*",
    "Document files", "*.doc",
    "Text files", "*.txt",
    NULL, NULL
  };

  OFileInfo fi;
  char *p, tmp[1024];

  if (mail == NULL) return;
  OText *body = mail->GetMessagePlusHeader();
  if (body == NULL) return;

  fi.MimeTypesList = MimeTypeList;
  fi.file_types = (char **) filetypes;
  new OXFileDialog(_client->GetRoot(), _parent, FDLG_SAVE, &fi);
  if (fi.filename) {
    fullpath = new char[strlen(fi.ini_dir) + strlen(fi.filename) + 2];
    sprintf(fullpath, "%s/%s", fi.ini_dir, fi.filename);
    if (!body->Save(fullpath)) {
      sprintf(tmp, "Error saving mail to file \"%s\"", fullpath);
      new OXMsgBox(_client->GetRoot(), _parent, new OString("NRX MAIL"),
		   new OString(tmp), MB_ICONSTOP, ID_OK);
      return;
    }
  }
  if (fullpath) delete[] fullpath;
  delete body;
}

void OXBodyView::BodyPrint() {
  OText *text;
  OPosition pos;
  pos.x = pos.y = 0;
  int ret;
  long count;
  char command[1024];
  char *tmpname = tmpnam(NULL);
  char *textBuffer;
  char *printer, *printcommand;

  printer = StrDup(_session->GetPrinter());
  printcommand = StrDup(_session->GetPrintCommand());

  if (mail == NULL) return;
  new OXPrintBox(_client->GetRoot(), _parent, 400, 150, &printer,
		 &printcommand, &ret);
  if (ret == True) {
    text = mail->GetMessagePlusHeader();
    if (text == NULL) return;
    text->Save(tmpname);
    sprintf(command, "%s -P%s %s", printcommand, printer, tmpname);	//, filename);
    FILE *p = popen(command, "w");
    pclose(p);
    unlink(tmpname);
    delete text;
  }
}

void OXBodyView::ViewAttachment() {
  FILE *g;
  viewedAttachs++;
  int pid, pid2, ftype;
  OMime *travelmime;
  char *eof;
  char line[1025];

  char *mimeaction;
  char action[PATH_MAX];
  char fullaction[PATH_MAX];
  char command[PATH_MAX];
  char filename[PATH_MAX];
  char *argv[PATH_MAX];
  int argc = 0;
  char *argptr;
  int status;
  char *temporaryfile;
  travelmime = NULL;
  travelmime = MimeTypeList->NextMime(travelmime);

  while (travelmime != NULL) {
    if (!(strcasecmp(travelmime->type, currentAttach->GetType()))) {
      break;
    }
    travelmime = MimeTypeList->NextMime(travelmime);
  }
  if (travelmime == NULL) {
    printf("Error viewing Attachment !!");
    return;
  }
  temporaryfile = tmpnam(NULL);
  SaveAttachment(temporaryfile);
  g = fopen(temporaryfile, "r");   // ==!==
  mimeaction = new char[strlen(travelmime->action) + 1];
  strcpy(mimeaction, travelmime->action);
  argptr = strtok(mimeaction, " ");
  while (argptr) {
    if (strcmp(argptr, "%s") == 0) {
      argv[argc] = new char[strlen(temporaryfile) + 1];
      strcpy(argv[argc], temporaryfile);
      argc++;
    } else {
      argv[argc] = new char[strlen(argptr) + 1];
      strcpy(argv[argc], argptr);
      argc++;
    }
    argptr = strtok(NULL, " ");
  }
  argv[argc] = NULL;
  pid = fork();
  delete mimeaction;
  if (pid == 0) {
    execvp(argv[0], argv);
    // if we are here then execlp failed!
    fprintf(stderr, "Cannot spawn \"%s\": execvp failed!\n", argv[0]);
    exit(1);
  } else {
    sleep(1);
    unlink(temporaryfile);
//       delete temporaryfile;
  }
//       fclose(g);
}

OText *OXBodyView::GetMessageText() {
  OText *txt = new OText();

  //...

  return txt;
}
