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

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include <X11/Xatom.h>

#include <xclass/OXFileDialog.h>
#include <xclass/OXAboutDialog.h>
#include <xclass/OIniFile.h>

#include "OSession.h"
#include "OXComposer.h"
#include "OXPreferences.h"
#include "OXFolderProp.h"
#include "OXMoveMail.h"
#include "OXFolderDelete.h"
#include "main.h"

#include "xpm/composerButton.xpm"
#include "xpm/moveButton.xpm"
#include "xpm/forwardButton.xpm"
#include "xpm/replyButton.xpm"
#include "xpm/deleteButton.xpm"
#include "xpm/saveButton.xpm"
#include "xpm/printButton.xpm"

SToolBarData tb_data[] = {
  { "composerButton.xpm", composerButton_xpm, "New Mail",      BUTTON_NORMAL,  1, NULL },
  { "",                   NULL,               "",              0,             -1, NULL },
  { "replyButton.xpm",    replyButton_xpm,    "Reply To Mail", BUTTON_NORMAL,  2, NULL },
  { "forwardButton.xpm",  forwardButton_xpm,  "Forward Mail",  BUTTON_NORMAL,  3, NULL },
  { "moveButton.xpm",     moveButton_xpm,     "Move Mail",     BUTTON_NORMAL,  4, NULL },
  { "deleteButton.xpm",   deleteButton_xpm,   "Delete Mail",   BUTTON_NORMAL,  5, NULL },
  { "",                   NULL,               "",              0,             -1, NULL },
  { "saveButton.xpm",     saveButton_xpm,     "Save Mail",     BUTTON_NORMAL,  6, NULL },
  { "printButton.xpm",    printButton_xpm,    "Print Mail",    BUTTON_NORMAL,  7, NULL },
  { NULL,                 NULL, NULL,         0,             -1, NULL }
};


//----------------------------------------------------------------------

OXClient *clientX;

int main(int argc, char **argv) {
  clientX = new OXClient;

  OXAppMainFrame *mainWindow = new OXAppMainFrame(clientX->GetRoot(), 10, 10);

//  mainWindow.Resize(600, 400);
//  mainWindow.MapWindow();

  clientX->Run();

  return 0;
}

char *OXAppMainFrame::Init() {
  char *tmp, *initdir;

  initdir = new char[strlen(getenv("HOME")) + strlen("/.nrxmail/") + 1];
  sprintf(initdir, "%s%s", getenv("HOME"), "/.nrxmail/");

  tmp = new char[strlen(initdir) + strlen("nrxmail.ini") + 1];
  sprintf(tmp, "%s%s", initdir, "nrxmail.ini");

  if (access(tmp, R_OK) == -1) {
    session = new OSession();
    _1sttime = True;
  } else {
    session = new OSession(initdir);
    _1sttime = False;
  }

  delete[] initdir;
  delete[] tmp;
}

OXAppMainFrame::OXAppMainFrame(const OXWindow * p, int w, int h) :
  OXMainFrame(p, w, h) {
  int ret = -1;
  char *folder;
  long folderNum;

  Init();
  SetupMenu();

  _line = new OXHorizontal3dLine(this, 4, 2);
  AddFrame(_line, new OLayoutHints(LHINTS_EXPAND_X));

  _toolBar = new OXToolBar(this);
  AddFrame(_toolBar, new OLayoutHints(LHINTS_EXPAND_X | LHINTS_TOP,
                                      0, 0, 2, 2));
  _toolBar->Associate(this);
  _toolBar->AddButtons(tb_data);

  OXLabel *lfolder = new OXLabel(_toolBar, new OString("Current folder: "));
  _toolBar->AddFrame(lfolder, new OLayoutHints(LHINTS_CENTER_Y, 5, 0, 0, 0));

  _folderBox = new OXDDListBox(_toolBar, 100);
  _toolBar->AddFrame(_folderBox, new OLayoutHints(LHINTS_CENTER_Y, 0, 5, 0, 0));
  _folderBox->Resize(100, 20);
  _folderBox->Associate(this);

  _ReloadFolderBox(session->GetFolderNames());

  _list = new OXMailList(this, 10, 155);
  _list->Associate(this);
  AddFrame(_list, new OLayoutHints(LHINTS_EXPAND_X));

  _resizer = new OXHorizontalResizer(this);
  AddFrame(_resizer, new OLayoutHints(LHINTS_EXPAND_X));

  _message = new OXBodyView(this, session);
  AddFrame(_message, new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y));

  _resizer->SetPrev(_list);
  _resizer->SetNext(_message);

  _currentFolder = session->GetFolder("Inbox");

  _menuMessage->DisableEntry(M_MESSAGE_DELETE);
  _menuMessage->DisableEntry(M_MESSAGE_REPLY);
  _menuMessage->DisableEntry(M_MESSAGE_FORWARD);
  _menuMessage->DisableEntry(M_MESSAGE_MOVE);
  _menuMessage->DisableEntry(M_MESSAGE_SRC);

  _menuFile->DisableEntry(M_FILE_SAVE);
  _menuFile->DisableEntry(M_FILE_PRINT);

  _menuEdit->DisableEntry(M_EDIT_SELECTALL);
  _menuEdit->DisableEntry(M_EDIT_COPY);
  _menuEdit->DisableEntry(M_EDIT_SEARCH);
  _menuEdit->DisableEntry(M_EDIT_SEARCHAGAIN);

  _toolBarEnabled = False;

  _status = new OXStatusBar(this);
  AddFrame(_status, new OLayoutHints(LHINTS_BOTTOM | LHINTS_EXPAND_X,
                                     0, 0, 2, 0));
  _status->AddLabel(120, LHINTS_RIGHT);
  _status->AddLabel(120, LHINTS_RIGHT);

  _status->SetWidth(0, 300);
  _status->SetText(2, new OString(""));

  ChangeCurrentFolder("Inbox");

  SetWindowName("NRX Mail");
  SetClassHints("NRX Mail", "NRX Mail");

  Resize(600, 420);
  MapSubwindows();
  MapWindow();

  Layout();

  _message->Clear();
  if (_1sttime) {
    while (ret == -1)
      new OXPreferences(clientX->GetRoot(), this, 320, 200, session, &ret);
    Reload();
  }
}

OXAppMainFrame::~OXAppMainFrame() {
  delete _menuBarLayout;
  delete _menuBarItemLayout;
  delete _menuFile;
  delete _menuEdit;
  delete _menuMessage;
  delete _menuMessage2;
  delete _menuFolder;
  delete _menuAdressBook;
  delete _menuHelp;
}

int OXAppMainFrame::HandleFocusChange(XFocusChangeEvent *event) {
  /* Set the Input Focus to the Text Entry Widget */

  if ((event->mode == NotifyNormal) && (event->detail != NotifyPointer) &&
      (event->type == FocusIn)) {
    XSetInputFocus(GetDisplay(), GetId(), RevertToParent, CurrentTime);
  }

  return True;
}

int OXAppMainFrame::ProcessMessage(OMessage * msg) {
  char **tmpname = NULL;
  long count = 0;
  OMimeAtt *travel;
  OText *body = NULL;
  OFolder **folder = NULL;
  OPosition *pos;
  OPosition mailpos;
  long num = -1;
  int ret = -1;
  char tmp[1024];

  OContainerMessage *cmsg = (OContainerMessage *) msg;
  OItemViewMessage *vmsg = (OItemViewMessage *) msg;
  OListBoxMessage *lbmsg = (OListBoxMessage *) msg;

  switch (msg->type) {
/*
  case MSG_TEXTVIEW:
    switch (msg->action) {
    case MSG_SELCHANGED:
      if (==!== text is selected ==!==)
	_menuEdit->EnableEntry(M_EDIT_COPY);
      else
	_menuEdit->DisableEntry(M_EDIT_COPY);
      break;

    case MSG_CLICK:
      if (vmsg->button == Button3)
        _menuEdit->PlaceMenu(vmsg->pos.x, vmsg->pos.y, True, True);
      break;
    }
    break;
*/

  // list view:

  case MSG_LISTVIEW:
    switch (msg->action) {
    case MSG_CLICK:
      if (_list->NumSelected() > 0) {
	if (vmsg->button == Button3) {
	  _menuMessage2->PlaceMenu(vmsg->pos.x, vmsg->pos.y, True, True);
	}
	_message->Clear();
	_currentMail = NULL;
	_currentMail = _list->GetCurrentMail();
	_currentMailNum = _list->GetCurrentMailNum();
	_message->ShowMessage(_currentMail);
	if (_message->GetMessageText() != NULL) {
	  _status->SetText(0, new OString(""));
	  _menuFile->EnableEntry(M_FILE_SAVE);
	  _menuFile->EnableEntry(M_FILE_PRINT);

	  _menuEdit->EnableEntry(M_EDIT_SELECTALL);
	  _menuEdit->EnableEntry(M_EDIT_SEARCH);
	  _menuEdit->EnableEntry(M_EDIT_SEARCHAGAIN);

	  _menuMessage->EnableEntry(M_MESSAGE_DELETE);
	  _menuMessage->EnableEntry(M_MESSAGE_REPLY);
	  _menuMessage->EnableEntry(M_MESSAGE_FORWARD);
	  _menuMessage->EnableEntry(M_MESSAGE_MOVE);
	  _menuMessage->EnableEntry(M_MESSAGE_SRC);

	  _toolBarEnabled = True;
	}
      } else {
	_menuFile->DisableEntry(M_FILE_SAVE);
	_menuFile->DisableEntry(M_FILE_PRINT);

	_menuEdit->DisableEntry(M_EDIT_SELECTALL);
	_menuEdit->DisableEntry(M_EDIT_COPY);
	_menuEdit->DisableEntry(M_EDIT_SEARCH);
	_menuEdit->DisableEntry(M_EDIT_SEARCHAGAIN);

	_menuMessage->DisableEntry(M_MESSAGE_DELETE);
	_menuMessage->DisableEntry(M_MESSAGE_REPLY);
	_menuMessage->DisableEntry(M_MESSAGE_FORWARD);
	_menuMessage->DisableEntry(M_MESSAGE_MOVE);
	_menuMessage->DisableEntry(M_MESSAGE_SRC);

	_message->Clear();
	_currentMail = NULL;
	_toolBarEnabled = False;
      }
      break;
    }
    break;

  case MSG_BUTTON:
    switch (msg->action) {
    case MSG_CLICK:
      switch (cmsg->id) {
      case 1:
	new OXComposer(clientX->GetRoot(), this, 500, 400, session, COMPOSER);
	break;

      case 2:
	if (!_toolBarEnabled) {
	  _status->SetText(0, new OString("No mail selected"));
	  break;
	}
	body = _currentMail->GetMessage();
	new OXComposer(clientX->GetRoot(), this, 500, 400, session, REPLY,
		       _list->GetCurrentMail());
	break;

      case 3:
	if (!_toolBarEnabled) {
	  _status->SetText(0, new OString("No mail selected"));
	  break;
	}
	body = _currentMail->GetMessage();
	new OXComposer(clientX->GetRoot(), this, 500, 400, session, FORWARD,
                       _list->GetCurrentMail());
	break;

      case 4:
	if (!_toolBarEnabled) {
	  _status->SetText(0, new OString("No mail selected"));
	  break;
	}
	new OXMoveMail(_client->GetRoot(), this, 200, 300,
		       session->GetFolderNames(), &num, &ret);
	if (ret != -1) {
	  _message->Clear();
	  MoveMail(num);
	}
	break;

      case 5:
	if (!_toolBarEnabled) {
	  _status->SetText(0, new OString("No mail selected"));
	  break;
	}
	DeleteMail();
	break;

      case 6:
	if (!_toolBarEnabled) {
	  _status->SetText(0, new OString("No mail selected"));
	  break;
	}
	_message->BodySave();
	break;

      case 7:
	if (!_toolBarEnabled) {
	  _status->SetText(0, new OString("No mail selected"));
	  break;
	}
	_message->BodyPrint();
	break;
      }
      break;
    }
    break;

  case MSG_DDLISTBOX:
    if (msg->action == MSG_CLICK) {
      ChangeCurrentFolder(session->GetFolder(lbmsg->entryID)->GetName());
    }
    break;

  case MSG_MENU:
    switch (msg->action) {
    case MSG_CLICK:
      _status->SetText(0, new OString(""));
      switch (cmsg->id) {

      case M_FILE_COMPOSER:
	new OXComposer(clientX->GetRoot(), this, 500, 400, session, COMPOSER);
	break;

      case M_FILE_CHECKMAIL:
	ChangeCurrentFolder("Inbox");
	Reload();
        _SetCounter(_currentFolder->GetNumberOfMails());
	break;

      case M_FILE_SAVE:
	_message->BodySave();
	break;

      case M_FILE_PRINT:
	_message->BodyPrint();
	break;

      case M_FILE_EMPTYTRASH:
	EmptyTrash();
	break;

      case M_FILE_SEND_UNSENT:
	SendUnsentMails();
	break;

      case M_FILE_EXIT:
	exit(0);
	break;

      case M_EDIT_COPY:
	BodyCopy();
	break;

      case M_EDIT_SELECTALL:
	BodySelectAll();
	_menuMessage->EnableEntry(M_EDIT_COPY);
	break;

      case M_EDIT_SEARCH:
	_message->BodySearch(True);
	break;

      case M_EDIT_SEARCHAGAIN:
	_message->BodySearch(False);
	break;

      case M_EDIT_PREF:
	new OXPreferences(clientX->GetRoot(), this, 320, 200, session, &ret);
	if (ret == 2) Reload();
	break;

      case M_MESSAGE_DELETE:
	DeleteMail();
	break;

      case M_MESSAGE_REPLY:
	body = _currentMail->GetMessage();
	new OXComposer(clientX->GetRoot(), this, 500, 400, session, REPLY,
		       _list->GetCurrentMail());
	break;

      case M_MESSAGE_MOVE:
	new OXMoveMail(_client->GetRoot(), this, 200, 300,
		       session->GetFolderNames(), &num, &ret);
	if (ret != -1) {
	  _message->Clear();
	  MoveMail(num);
	}
	break;

      case M_MESSAGE_SRC:
	_message->ViewBodySource();
	break;

      case M_FOLDER_NEW:
	new OXFolderProp(_client->GetRoot(), this, 100, 300, &ret, session);
	if (ret == 1) {
	  _currentFolder = session->GetLastFolder();
	  _ReloadFolderBox(session->GetFolderNames());
	  ChangeCurrentFolder(_currentFolder->GetName());
	}
	break;

      case M_FOLDER_CLEAR:
	ClearFolder();
	break;

      case M_FOLDER_DELETE:
	DeleteFolder();
	break;

      case M_FOLDER_PROPERTIES:
	new OXFolderProp(_client->GetRoot(), this, 100, 300, &ret, session,
			 _currentFolder);
	if (ret == 1) {
	  _ReloadFolderBox(session->GetFolderNames());
	  ChangeCurrentFolder(_currentFolder->GetName());
	}
	delete folder;
	break;

      case M_ADRESS_VIEW:
	break;

      case M_HELP_ABOUT:
        About();
	break;

      default:
	break;

      }
      break;

    default:
      break;

    }
    break;

  default:
    break;
  }

  return True;
}

void OXAppMainFrame::Reload() {
  _SetCounter(_currentFolder->GetNumberOfMails());
  _currentFolder->Reload();
  _list->ReloadList();
  _message->Clear();
  _currentMail = NULL;
  _currentMailNum = -1;
}

void OXAppMainFrame::Update() {
  _SetCounter(_currentFolder->GetNumberOfMails());
  _currentFolder->Reload();
  _message->Clear();
  _currentMail = NULL;
  _currentMailNum = -1;
}

void OXAppMainFrame::DeleteMail() {
  if (strcmp(_currentFolder->GetName(), "Trash"))
    _currentFolder->MoveMail(_currentMail->GetMessageID(),
			     session->AbsolutePath("Trash"));
  else
    _currentFolder->EraseMail(_currentMail->GetMessageID());
  _SetCounter(_currentFolder->GetNumberOfMails());
  session->GetFolder("Trash")->Reload();
  _list->DeleteItem(_currentMailNum);
  Update();
}

void OXAppMainFrame::MoveMail(long folder) {

  _currentFolder->MoveMail(_currentMail->GetMessageID(),
			   session->AbsolutePath(session->
						 GetFolder(folder)->
						 GetName()));
  _list->DeleteItem(_currentMailNum);
  Update();
  session->GetFolder(folder)->Reload();
}

void OXAppMainFrame::ClearFolder() {
  OMail *mailToBeMoved;
  if (strcmp(_currentFolder->GetName(), "Trash")) {
    while (_currentFolder->GetNumberOfMails() > 0) {
      mailToBeMoved = _currentFolder->GetFirstMail();
      _currentFolder->MoveMail(mailToBeMoved->GetMessageID(),
			       session->AbsolutePath(session->
						     GetFolder("Trash")->
						     GetName()));
    }
  } else {
    session->ClearFolder("Trash");
  }
  session->GetFolder("Trash")->Reload();
  Reload();
}

void OXAppMainFrame::DeleteFolder() {
  int ret = 0;

  if (_currentFolder->IsSystem()) {
    new OXMsgBox(_client->GetRoot(), this, new OString("NRX Mail"),
		 new OString("System folders can't be deleted"),
		 MB_ICONSTOP, ID_OK);
    return;
  }
  if (_currentFolder->IsLink()) {
    new OXFolderDelete(_client->GetRoot(), this, 300, 200, &ret);
  }
  if (ret == -1) return;
  if (ret == 1) unlink(_currentFolder->GetLinkPath());

  session->DeleteFolder(_currentFolder->GetName());
  ChangeCurrentFolder("Inbox");
  _ReloadFolderBox(session->GetFolderNames());
}

void OXAppMainFrame::ChangeCurrentFolder(const char *name) {
  _currentFolder = session->GetFolder(name);
  _folderBox->Select(session->GetFolderNumber(name));
  _status->SetText(0, new OString("Loading Folder..."));
  _list->LoadList(_currentFolder);
  _message->Clear();
  _status->SetText(0, new OString("Ready"));
  _SetCounter(_currentFolder->GetNumberOfMails());
}

void OXAppMainFrame::EmptyTrash() {
  session->ClearFolder("Trash");
  if (!strcmp(_currentFolder->GetName(), "Trash")) Reload();
}


void OXAppMainFrame::SendUnsentMails() {
  char *tempname = tmpnam(NULL);
  SAddress *travel;
  OSmtp smtp;
  FILE *f;
  OTextBuffer *tobuffer, *ccbuffer;
  OText *src;
  OMail *mailToBeSent;

  OFolder *outbox = session->GetFolder("Unsent");
  OFolder *sentmail = session->GetFolder("Sentmail");

  while (outbox->GetNumberOfMails() > 0) {
    tobuffer = new OTextBuffer(512);
    ccbuffer = new OTextBuffer(512);
    mailToBeSent = outbox->GetFirstMail();
    if (mailToBeSent->GetCC() != NULL) {
      travel = mailToBeSent->GetCC();
      if (travel != NULL) {
	ccbuffer->AddText(ccbuffer->GetTextLength(), travel->adress);
	travel = travel->next;
	while (travel != NULL) {
	  ccbuffer->AddText(ccbuffer->GetTextLength(), ", ");
	  ccbuffer->AddText(ccbuffer->GetTextLength(), travel->adress);
	  travel = travel->next;
	}
      }
    }
    travel = mailToBeSent->GetTo();
    if (travel != NULL) {
      tobuffer->AddText(tobuffer->GetTextLength(), travel->adress);
      travel = travel->next;
      while (travel != NULL) {
	tobuffer->AddText(tobuffer->GetTextLength(), ", ");
	tobuffer->AddText(tobuffer->GetTextLength(), travel->adress);
	travel = travel->next;
      }
    }
    src = mailToBeSent->GetMessageSrc();
    src->Save(tempname);
    f = fopen(tempname, "r");
    smtp.SetupSmtp(session->GetSmtpServer(), session->GetSmtpPort());
    smtp.SendMail(f, session->GetIdentity(), session->GetReturnPath(),
		  (char *) tobuffer->GetString(), NULL,
		  (char *) ccbuffer->GetString());
    smtp.CloseConnect();
    fclose(f);
    unlink(tempname);
    outbox->MoveMail(mailToBeSent->GetMessageID(),
		     session->AbsolutePath(sentmail->GetName()));
    delete src;
    delete tobuffer;
    delete ccbuffer;
  }
  outbox->Reload();
  sentmail->Reload();
// Reload();
}

void OXAppMainFrame::SetupMenu() {
  _menuBarLayout = new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 0, 0, 1, 1);
  _menuBarItemLayout = new OLayoutHints(LHINTS_TOP | LHINTS_LEFT, 0, 4, 0, 0);

  _menuFile = new OXPopupMenu(_client->GetRoot());
  _menuFile->AddEntry(new OHotString("&Composer..."), M_FILE_COMPOSER);
  _menuFile->AddSeparator();
  _menuFile->AddEntry(new OHotString("&Save Mail..."), M_FILE_SAVE);
  _menuFile->AddEntry(new OHotString("&Print Mail..."), M_FILE_PRINT);
  _menuFile->AddSeparator();
  _menuFile->AddEntry(new OHotString("C&heck Mail"), M_FILE_CHECKMAIL);
  _menuFile->AddSeparator();
  _menuFile->AddEntry(new OHotString("&Empty Trash"), M_FILE_EMPTYTRASH);
  _menuFile->AddEntry(new OHotString("Send &Unsent Messages"), M_FILE_SEND_UNSENT);
  _menuFile->AddSeparator();
  _menuFile->AddEntry(new OHotString("E&xit"), M_FILE_EXIT);

  _menuEdit = new OXPopupMenu(_client->GetRoot());
  _menuEdit->AddEntry(new OHotString("&Copy"), M_EDIT_COPY);
  _menuEdit->AddEntry(new OHotString("&Mark all"), M_EDIT_SELECTALL);
  _menuEdit->AddSeparator();
  _menuEdit->AddEntry(new OHotString("&Search..."), M_EDIT_SEARCH);
  _menuEdit->AddEntry(new OHotString("Search &again"), M_EDIT_SEARCHAGAIN);
  _menuEdit->AddSeparator();
  _menuEdit->AddEntry(new OHotString("&Preferences..."), M_EDIT_PREF);

  _menuMessage = new OXPopupMenu(_client->GetRoot());
  _menuMessage->AddEntry(new OHotString("&Reply"), M_MESSAGE_REPLY);
  _menuMessage->AddEntry(new OHotString("&Delete"), M_MESSAGE_DELETE);
  _menuMessage->AddEntry(new OHotString("&Forward"), M_MESSAGE_FORWARD);
  _menuMessage->AddEntry(new OHotString("&Move"), M_MESSAGE_MOVE);
  _menuMessage->AddSeparator();
  _menuMessage->AddEntry(new OHotString("&View Source..."), M_MESSAGE_SRC);
  _menuMessage->AddEntry(new OHotString("View &Attachment..."), M_MESSAGE_ATT);

  _menuMessage->DisableEntry(M_MESSAGE_FORWARD);
  _menuMessage->DisableEntry(M_MESSAGE_ATT);

  _menuFolder = new OXPopupMenu(_client->GetRoot());
  _menuFolder->AddEntry(new OHotString("&New..."), M_FOLDER_NEW);
  _menuFolder->AddSeparator();
  _menuFolder->AddEntry(new OHotString("&Delete"), M_FOLDER_DELETE);
  _menuFolder->AddEntry(new OHotString("&Clear"), M_FOLDER_CLEAR);
  _menuFolder->AddSeparator();
  _menuFolder->AddEntry(new OHotString("&Properties..."), M_FOLDER_PROPERTIES);

  _menuAdressBook = new OXPopupMenu(_client->GetRoot());
  _menuAdressBook->AddEntry(new OHotString("&Add Sender"), M_ADRESS_ADD);
  _menuAdressBook->AddEntry(new OHotString("&Delete Sender"), M_ADRESS_DELETE);
  _menuAdressBook->AddSeparator();
  _menuAdressBook->AddEntry(new OHotString("&View Adressbook..."), M_ADRESS_VIEW);

  _menuAdressBook->DisableEntry(M_ADRESS_ADD);
  _menuAdressBook->DisableEntry(M_ADRESS_DELETE);
  _menuAdressBook->DisableEntry(M_ADRESS_VIEW);

  _menuHelp = new OXPopupMenu(_client->GetRoot());
  _menuHelp->AddEntry(new OHotString("&Contents..."), M_HELP_CONTENTS);
  _menuHelp->AddEntry(new OHotString("&Search..."), M_HELP_SEARCH);
  _menuHelp->AddSeparator();
  _menuHelp->AddEntry(new OHotString("&About..."), M_HELP_ABOUT);

  _menuHelp->DisableEntry(M_HELP_CONTENTS);
  _menuHelp->DisableEntry(M_HELP_SEARCH);

  _menuFile->Associate(this);
  _menuEdit->Associate(this);
  _menuMessage->Associate(this);
  _menuFolder->Associate(this);
  _menuAdressBook->Associate(this);
  _menuHelp->Associate(this);

  _menuBar = new OXMenuBar(this, 1, 1, HORIZONTAL_FRAME);
  _menuBar->AddPopup(new OHotString("&File"), _menuFile, _menuBarItemLayout);
  _menuBar->AddPopup(new OHotString("&Edit"), _menuEdit, _menuBarItemLayout);
  _menuBar->AddPopup(new OHotString("&Message"), _menuMessage, _menuBarItemLayout);
  _menuBar->AddPopup(new OHotString("&Folder"), _menuFolder, _menuBarItemLayout);
  _menuBar->AddPopup(new OHotString("&Adressbook"), _menuAdressBook, _menuBarItemLayout);
  _menuBar->AddPopup(new OHotString("&Help"), _menuHelp, _menuBarItemLayout);

  AddFrame(_menuBar, _menuBarLayout);

  _menuMessage2 = new OXPopupMenu(_client->GetRoot());
  _menuMessage2->AddEntry(new OHotString("&Reply"), M_MESSAGE_REPLY);
  _menuMessage2->AddEntry(new OHotString("&Forward"), M_MESSAGE_FORWARD);
  _menuMessage2->AddEntry(new OHotString("&Delete"), M_MESSAGE_DELETE);
  _menuMessage2->AddEntry(new OHotString("&Move"), M_MESSAGE_MOVE);
  _menuMessage2->AddSeparator();
  _menuMessage2->AddEntry(new OHotString("View Message in &New Window"),
			  M_MESSAGE2_SEPWINDOW);
  _menuMessage2->AddEntry(new OHotString("&View Source"), M_MESSAGE_SRC);
  _menuMessage->DisableEntry(M_MESSAGE_FORWARD);

  _menuMessage2->Associate(this);
}

void OXAppMainFrame::About() {
  OAboutInfo info;

  info.wname = "About NRX mail";
  info.title = "NRX Mail version " NRX_VERSION "\n"
               "A mail client program using xclass";
  info.copyright = "Copyright © 1998-1999 by Harald Radke";
  info.text = "This program is free software; you can redistribute it "
              "and/or modify it under the terms of the GNU "
              "General Public License.\n\n"
              "http://www.foxproject.org";

  new OXAboutDialog(_client->GetRoot(), this, &info);
}

void OXAppMainFrame::_ReloadFolderBox(char **names) {
  long count = 0;

  while (names[count] != NULL) count++;
  count--;

  _folderBox->RemoveAllEntries();
  for (long i = 0; i <= count; i++)
    _folderBox->AddEntry(new OString(names[i]), i);

  _folderBox->Select(0);
}

void OXAppMainFrame::_SetCounter(int num) {
  char tmp[100];

  sprintf(tmp, "%d Messages", num);
  _status->SetText(1, new OString(tmp));
}
