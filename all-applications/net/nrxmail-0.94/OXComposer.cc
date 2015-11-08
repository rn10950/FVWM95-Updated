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
#include <unistd.h>
#include <time.h>
#include <pwd.h>

#include <xclass/OResourcePool.h>
#include <xclass/OMimeTypes.h>
#include <xclass/OXMsgBox.h>
#include <xclass/OXFileDialog.h>

#include <vector>

#include "OXComposer.h"
#include "OXPrintBox.h"
#include "OXSearchBox.h"

#include "xpm/attachment.xpm"
#include "xpm/sendNowButton.xpm"
#include "xpm/sendLaterButton.xpm"
#include "xpm/saveButton.xpm"
#include "xpm/printButton.xpm"


// TODO: handle the text directly in te and not in _body.

//----------------------------------------------------------------------

OXComposer::OXComposer(const OXWindow *p, const OXWindow *main, int w,
		       int h, OSession *session, int state, OMail *mail) : 
  OXTransientFrame(p, main, w, h, MAIN_FRAME | VERTICAL_FRAME), OSmtp() {
    int ax, ay;
    Window wdummy;

    _my_cc = NULL;
    _bcopy_backup = NULL;

    MimeTypeList = _client->GetResourcePool()->GetMimeTypes();

    search.direction = True;
    search.caseSensitive = False;
    search.buffer = NULL;
    char *tmp, *tmp2;

    currentAttach = attList = NULL;
    showAttachments = False;
    SAddress *travel;
    char *headerString = NULL;
    OPosition pos, pos2, pos3;

    InitMenu();

    _line = new OXHorizontal3dLine(this, 4, 2);
    AddFrame(_line, new OLayoutHints(LHINTS_EXPAND_X, 0, 0, 0, 0));
    InitToolBar();

    f8 = new OXCompositeFrame(this, 60, 20, HORIZONTAL_FRAME);
    f6 = new OXCompositeFrame(f8, 60, 20, VERTICAL_FRAME);
    f7 = new OXCompositeFrame(f8, 60, 20, VERTICAL_FRAME);

    f1 = new OXCompositeFrame(f7, 60, 20, HORIZONTAL_FRAME);
    f2 = new OXCompositeFrame(f7, 60, 20, HORIZONTAL_FRAME);
    f3 = new OXCompositeFrame(f7, 60, 20, HORIZONTAL_FRAME);
    f4 = new OXCompositeFrame(f7, 60, 20, HORIZONTAL_FRAME);

    OText *tmptext = NULL;

    _mail = mail;
    _session = session;
    te = new OXTextEdit(this, 560, 300, 1);
    _body = new OText();

    long count = 0;
    pos.x = pos.y = 0;
    pos2.x = pos2.y = 0;

    if (state == REPLY) {
      delete _body;
      _body = _mail->GetMessage();
      pos.x = pos.y = 0;

      for (pos.y = 0; pos.y < _body->RowCount(); pos.y++)
        _body->InsText(pos, "> ");

      if (strlen(_mail->GetFrom()->name) > 0)
        tmp2 = _mail->GetFrom()->name;
      else
        tmp2 = _mail->GetFrom()->adress;

      tmp = new char[strlen(_mail->GetDate()) + strlen(tmp2) + 
                            strlen("On   ,wrote:") + 1];
      sprintf(tmp, "On %s, %s wrote:", _mail->GetDate(), tmp2);
      pos.x = pos.y = 0;
      _body->BreakLine(pos);
      _body->InsLine(0, tmp);
      delete[] tmp;
    } else if (state == FORWARD) {
      if (_mail != NULL) {
        _body->InsLine(pos.y, "------------Forwarded Message ------------");
        pos.y++;
        _body->InsLine(pos.y, "Date: ");
        pos.x = 6;
        headerString = _mail->GetDate();
        if (headerString != NULL)
	  _body->InsText(pos, headerString);
        pos.y++;
        travel = _mail->GetFrom();
        if (travel != NULL) {
	  _body->InsLine(pos.y, "From: ");
	  pos.x = 6;
	  _body->InsText(pos, travel->rawadress);
	  travel = travel->next;
	  pos.y++;
	  while (travel != NULL) {
	    _body->InsLine(pos.y, "    ");
	    _body->InsText(pos, travel->rawadress);
	    travel = travel->next;
	    pos.y++;
	  }
        }

        travel = _mail->GetTo();
        if (travel != NULL) {
	  _body->InsLine(pos.y, "To: ");
	  pos.x = 4;
	  _body->InsText(pos, travel->rawadress);
	  travel = travel->next;
	  pos.y++;
	  while (travel != NULL) {
	    _body->InsLine(pos.y, "    ");
	    _body->InsText(pos, travel->rawadress);
	    travel = travel->next;
	    pos.y++;
	  }
        }

        travel = _mail->GetCC();
        if (travel != NULL) {
	  _body->InsLine(pos.y, "CC: ");
	  pos.x = 4;
	  _body->InsText(pos, travel->rawadress);
	  travel = travel->next;
	  pos.y++;
	  while (travel != NULL) {
	    _body->InsLine(pos.y, "    ");
	    _body->InsText(pos, travel->rawadress);
	    travel = travel->next;
	    pos.y++;
	  }
        }

        _body->InsLine(pos.y, "Subject: ");
        pos.x = 9;
        headerString = _mail->GetSubject();
        if (headerString != NULL)
	  _body->InsText(pos, headerString);
        pos.x = _body->GetLineLength(pos.y);
        pos3.x = pos3.y = 0;
        _body->BreakLine(pos);
        pos.y = pos.y + 2;
        pos.x = 0;
        tmptext = _mail->GetMessage();
        pos2.y = tmptext->RowCount() - 1;
        pos2.x = tmptext->GetLineLength(pos2.y) - 1;
        _body->InsText(pos, tmptext, pos3, pos2);

      } else {

        tmptext = _mail->GetMessage();
        pos2.y = tmptext->RowCount() - 1;
        pos2.x = tmptext->GetLineLength(pos2.y) - 1;
        _body->InsText(pos, tmptext, pos, pos2);
      }

      delete tmptext;
    }

    tmptext = new OText();
    pos3.x = pos3.y = 0;
    pos.y = _body->RowCount();
    pos.x = 0;

    if (tmptext->Load(session->GetSignatureFile())) {
      pos2.y = tmptext->RowCount() - 1;
      pos2.x = tmptext->GetLineLength(pos2.y) - 1;
      _body->InsText(pos, tmptext, pos3, pos2);
      _body->InsLine(pos.y, "-- ");
      _body->BreakLine(pos);
    }

    delete tmptext;

#if 0
    te->SetText(_body);
#else
    {
      int ln;
      te->AutoUpdate(False);
      for (ln = 0; ln < _body->RowCount(); ++ln) {
        const char *str = _body->GetLine(OPosition(0, ln));
        if (str) {
          te->InsertText(str);
          te->InsertText("\n");
        }
      }
      te->AutoUpdate(True);
    }
#endif

    char *buffer;
    char spos = 0;
    lb = new OLayoutHints(LHINTS_TOP | LHINTS_CENTER_X, 2, 2, 3, 0);
    ll = new OLayoutHints(LHINTS_LEFT | LHINTS_CENTER_Y, 2, 2, 8, 3);
    lt = new OLayoutHints(LHINTS_CENTER_X | LHINTS_EXPAND_X, 2, 2, 3, 0);
    lto = new OXLabel(f6, new OHotString("&To:"));
    f6->AddFrame(lto, ll);
    to = new OXTextEntry(f1, bto = new OTextBuffer(256));
    f1->AddFrame(to, lt);
    to->Associate(this);
    toButton = new OXTextButton(f1, new OHotString("Add..."), 1);
    f1->AddFrame(toButton,
	         new OLayoutHints(LHINTS_CENTER_Y | LHINTS_RIGHT, 2, 5, 0, 0));
    toButton->Associate(this);
    toButton->Disable();

    lcopy = new OXLabel(f6, new OHotString("&CC:"));
    f6->AddFrame(lcopy, ll);
    copy = new OXTextEntry(f2, bcopy = new OTextBuffer(256));
    f2->AddFrame(copy, lt);
    copy->Associate(this);
    copy->Disable();
    copyButton = new OXTextButton(f2, new OHotString("Add..."), 2);
    f2->AddFrame(copyButton,
	         new OLayoutHints(LHINTS_CENTER_Y | LHINTS_RIGHT, 2, 5, 0, 0));
    copyButton->Associate(this);
    copyButton->Disable();

  lsubject = new OXLabel(f6, new OHotString("&Subject:"));
  f6->AddFrame(lsubject, ll);
  subject = new OXTextEntry(f3, bsubject = new OTextBuffer(256));
  f3->AddFrame(subject, lt);
  subject->Associate(this);

  flayout = new OLayoutHints(LHINTS_EXPAND_X, 3, 5, 2, 0);
  telayout =
      new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y, 0, 0, 3, 0);

  f6->Resize(GetWidth(), f7->GetHeight());

  f7->AddFrame(f1, flayout);
  f7->AddFrame(f2, flayout);
  f7->AddFrame(f3, flayout);
  f7->AddFrame(f4, flayout);
  f8->AddFrame(f6, ll);
  f8->AddFrame(f7, flayout);
  AddFrame(f8, flayout);

  toall = NULL;
  if ((state == REPLY) && (_mail->GetCC() != NULL)) {
    toall = new OXCheckButton(this, new OHotString("Reply to &all"), 1);
    AddFrame(toall, new OLayoutHints(LHINTS_CENTER_X));
    toall->SetState(BUTTON_DOWN);
    travel = _mail->GetCC();
    if (travel != NULL) {
      bcopy->AddText(bcopy->GetTextLength(), travel->adress);
      travel = travel->next;
      while (travel != NULL) {
	bcopy->AddText(bcopy->GetTextLength(), ", ");
	bcopy->AddText(bcopy->GetTextLength(), travel->adress);
	travel = travel->next;
      }
    }
  }

  AddFrame(te, telayout);

  pos.x = pos.y = 0;

  switch (state) {
  case REPLY:
    if (_mail != NULL) {
      travel = _mail->GetReplyTo();
      if (travel == NULL)
	travel = _mail->GetFrom();
      if (travel != NULL) {
	bto->AddText(bto->GetTextLength(), travel->adress);
	travel = travel->next;
	while (travel != NULL) {
	  bto->AddText(bto->GetTextLength(), ", ");
	  bto->AddText(bto->GetTextLength(), travel->adress);
	  travel = travel->next;
	}
      }
      headerString = _mail->GetSubject();
      if (headerString != NULL) {
	if (strncmp(headerString, "Re:", 3)) {
	  bsubject->AddText(0, "Re: ");
	  spos = 4;
	}
	if (strlen(headerString) > 0)
	  bsubject->AddText(spos, headerString);
	else
	  bsubject->AddText(spos, "Your Mail");
      } else {
	bsubject->AddText(0, "Re:");
      }
    }
    break;
  case FORWARD:
    headerString = _mail->GetSubject();
    if (headerString != NULL) {
      bsubject->AddText(0, headerString);
      bsubject->AddText(bsubject->GetTextLength(), " (fwd)");
    }
    break;
  default:
    break;
  }

  te->Layout();
  f4 = new OXCompositeFrame(this, 60, 20, HORIZONTAL_FRAME);

  attachments = new OXVerticalFrame(f4, 10, 75, FIXED_HEIGHT);
  attachmentsHeader = new OXCompositeFrame(this, 10, 10, HORIZONTAL_FRAME);

  attPix = _client->GetPicture("attachment.xpm", attachment_xpm);
  attButton =
      new OXPictureButton((const OXWindow *) attachmentsHeader, attPix, 3);
  attachmentsHeader->AddFrame(attButton,
			      new OLayoutHints(LHINTS_CENTER_Y, 2, 0, 2, 2));
  attButton->Associate(this);

  attachmentsHeader->AddFrame(new OXLabel(attachmentsHeader, new
					  OString("Attachments ")),
			      new OLayoutHints(LHINTS_CENTER_Y, 1, 1, 1, 1));
  AddFrame(attachmentsHeader,
	   new OLayoutHints(LHINTS_EXPAND_X, 0, 0, 0, 0));

  attachmentsView = new OXListView(attachments, 40, 10, HORIZONTAL_FRAME |
				   SUNKEN_FRAME | DOUBLE_BORDER);
  attachmentsView->Associate(this);
  attachmentsView->SetViewMode(LV_LARGE_ICONS);

  attachments->AddFrame(attachmentsView,
			new OLayoutHints(LHINTS_EXPAND_Y | LHINTS_EXPAND_X,
					 0, 0, 0, 0));
  f4->AddFrame(attachments, new OLayoutHints(LHINTS_EXPAND_X, 0, 0, 0, 5));

  attachButton = new OXTextButton(f4, new OHotString("Add..."), 4);
  f4->AddFrame(attachButton,
	       new OLayoutHints(LHINTS_CENTER_Y, 5, 5, 0, 0));
  AddFrame(f4, flayout);

  attachButton->Associate(this);

  SetWindowName("Composer");

  SetMWMHints(MWM_DECOR_ALL | MWM_DECOR_MAXIMIZE | MWM_DECOR_MENU,
	      MWM_FUNC_ALL | MWM_FUNC_MAXIMIZE | MWM_FUNC_RESIZE, 3);
  //Layout();
  MapSubwindows();
  Resize(GetDefaultSize());

  // position relative to the parent's window
  XTranslateCoordinates(GetDisplay(),
			main->GetId(), GetParent()->GetId(),
			(((OXFrame *) main)->GetWidth() - _w) >> 1,
			(((OXFrame *) main)->GetHeight() - _h) >> 1,
			&ax, &ay, &wdummy);
  Move(ax, ay);

  MapWindow();
  HideFrame(f4);
  timer = new OTimer(this, 100);
}

OXComposer::~OXComposer() {
  if (timer) delete timer;
  delete telayout;
  delete flayout;
  delete attachPopUp;
  delete _menuFile;
  delete _menuEdit;
  delete _menuSearch;
  delete _menuHelp;
  delete _menuBarLayout;
  delete _menuBarItemLayout;
  CmpAttachList *travel = attList;
  CmpAttachList *toDelete;
  while (travel != NULL) {
    toDelete = travel;
    travel = travel->next;
    if (toDelete->file) delete[] toDelete->file;
    if (toDelete->name) delete[] toDelete->name;
    delete toDelete;
  }
}

void OXComposer::Search(int ret) {
  char tmp[1024];
  if ((ret == False) || (search.buffer == NULL))
    new OXSearchBox(_client->GetRoot(), this, 400, 150, &search, &ret);
  if (ret == True)
    if (!(te->Search(search.buffer, search.direction,
                     search.caseSensitive))) {
      sprintf(tmp, "Couldn't find \"%s\" ", search.buffer);
      new OXMsgBox(_client->GetRoot(), this, new OString("Composer"),
		   new OString(tmp), MB_ICONEXCLAMATION, ID_OK);
    } else {
      _menuEdit->EnableEntry(CMP_EDIT_CUT);
      _menuEdit->EnableEntry(CMP_EDIT_COPY);
      _menuEdit->EnableEntry(CMP_EDIT_DELETE);
    }
}


int OXComposer::HandleTimer(OTimer *t) {
  if (t != timer) return True;
  if (XGetSelectionOwner(GetDisplay(), XA_PRIMARY) == None)
    _menuEdit->DisableEntry(CMP_EDIT_PASTE);
  else
    _menuEdit->EnableEntry(CMP_EDIT_PASTE);
  delete timer;
  timer = new OTimer(this, 100);
  return True;
}


int OXComposer::ProcessMessage(OMessage *msg) {
  OItemViewMessage *cmsg = (OItemViewMessage *) msg;

  switch (msg->type) {
  case MSG_LISTVIEW:
    switch (msg->action) {
    case MSG_SELCHANGED:
      if (attachmentsView->NumSelected() > 0) {
	_menuEdit->EnableEntry(CMP_EDIT_CUT);
	_menuEdit->EnableEntry(CMP_EDIT_COPY);
	_menuEdit->EnableEntry(CMP_EDIT_DELETE);
      } else {
	_menuEdit->DisableEntry(CMP_EDIT_CUT);
	_menuEdit->DisableEntry(CMP_EDIT_COPY);
	_menuEdit->DisableEntry(CMP_EDIT_DELETE);
      }
      break;

//        case VIEW_F3:
//         Search(True);
//         break;

/* OXTextEdit:
         case MSG_CLICK:
           if (cmsg->button == Button3)
             _menuEdit->PlaceMenu(cmsg->pos.x, cmsg->pos.y, True, True);
         break;

OXListView: */

    case MSG_CLICK:
      if (!SetCurrentAttachment())
	return True;
      if (cmsg->button == Button3) {
	attachPopUp->PlaceMenu(cmsg->pos.x, cmsg->pos.y, True, True);
      }
      break;
    }
    break;

  case MSG_MENU:
    switch (msg->action) {
    case MSG_CLICK:
      switch (cmsg->id) {
      case CMP_FILE_EXIT:
	CloseWindow();
	break;

      case CMP_FILE_SAVE:
	SaveMail();
	break;

      case CMP_FILE_PRINT:
	PrintMail();
	break;

      case CMP_FILE_INCLUDE:
	IncludeFile();
	break;

      case CMP_FILE_SEND_NOW:
	if (strlen(bto->GetString()) == 0) {
	  new OXMsgBox(_client->GetRoot(), this, new OString("NRX Mail"),
		       new OString("No Recipient specified !"),
		       MB_ICONSTOP, ID_OK);
	  break;
	}
	if (strlen(bsubject->GetString()) == 0)
	  bsubject->AddText(0, " ");
	Send(CMP_FILE_SEND_NOW);
	CloseWindow();
	break;

      case CMP_FILE_SEND_LATER:
	if (strlen(bto->GetString()) == 0) {
	  new OXMsgBox(_client->GetRoot(), this, new OString("NRX Mail"),
		       new OString("No Recipient specified !"),
		       MB_ICONSTOP, ID_OK);
	  break;
	}
	if (strlen(bsubject->GetString()) == 0)
	  bsubject->AddText(0, " ");
	Send(CMP_FILE_SEND_LATER);
	CloseWindow();
	break;

      case CMP_EDIT_CUT:
	if (te->Cut() == True)
	  _menuEdit->EnableEntry(CMP_EDIT_PASTE);
	break;

      case CMP_EDIT_COPY:
	if (te->Copy() == True)
	  _menuEdit->EnableEntry(CMP_EDIT_PASTE);
	break;

      case CMP_EDIT_PASTE:
	te->Paste();
	break;

      case CMP_EDIT_DELETE:
	te->DeleteSelection();
	break;

      case CMP_EDIT_SELECTALL:
        te->SelectAll();
	if (te->HasSelection()) {
	  _menuEdit->EnableEntry(CMP_EDIT_CUT);
	  _menuEdit->EnableEntry(CMP_EDIT_COPY);
	  _menuEdit->EnableEntry(CMP_EDIT_DELETE);
	}
	break;

      case CMP_SEARCH_AGAIN:
	Search(True);
	break;

      case CMP_SEARCH_SEARCH:
	Search(False);
	break;

      case CMP_DEL_ATTACH:
	DelAttach();
	break;

      default:
	break;

      }
      break;
    }
    break;

  case MSG_CHECKBUTTON:
    switch (msg->action) {
    case MSG_CLICK:
      if (toall->GetState() == BUTTON_UP) {
	delete[] _bcopy_backup;
	_bcopy_backup = new char[strlen(bcopy->GetString()) + 1];
	strcpy(_bcopy_backup, bcopy->GetString());
	bcopy->Clear();
	_client->NeedRedraw(copy);
//        copy->SetState(ENABLED);
      } else {
	bcopy->AddText(0, _bcopy_backup);
	_client->NeedRedraw(copy);
//        copy->SetState(DISABLED);
      }
      break;
    }
    break;

  case MSG_BUTTON:
    switch (msg->action) {
    case MSG_CLICK:
      switch (cmsg->id) {
      case 3:
	showAttachments = !showAttachments;
	if (showAttachments)
	  ShowFrame(f4);
	else
	  HideFrame(f4);
	break;

      case 4:
	AddAttach();
	break;

      case 5:
	if (strlen(bto->GetString()) == 0) {
	  new OXMsgBox(_client->GetRoot(), this, new OString("NRX Mail"),
		       new OString("No Recipient specified !"),
		       MB_ICONSTOP, ID_OK);
	  break;
	}
	if (strlen(bsubject->GetString()) == 0)
	  bsubject->AddText(0, " ");
	Send(CMP_FILE_SEND_NOW);
	CloseWindow();
	break;

      case 6:
	if (strlen(bto->GetString()) == 0) {
	  new OXMsgBox(_client->GetRoot(), this, new OString("NRX Mail"),
		       new OString("No Recipient specified !"),
		       MB_ICONSTOP, ID_OK);
	  break;
	}
	if (strlen(bsubject->GetString()) == 0)
	  bsubject->AddText(0, " ");
	Send(CMP_FILE_SEND_LATER);
	CloseWindow();
	break;

      case 7:
	IncludeFile();
	break;

      case 8:
	SaveMail();
	break;

      case 9:
	PrintMail();
	break;

      }
      break;
    }
  }
  return True;
}


void OXComposer::Send(int mode) {
  char *bound = "-----NrXMaIl-UniQUE--MULtiPART---BoUndAry";
  int sendOK = 0;
  OPosition pos, endpos;
  FILE *f, *g;
  char *tempfile = tmpnam(NULL);
  char *my_cc = NULL;
  char *my_to = NULL;
  char *buffer;
  char *temp, *temp2;
  char *cc_string = "cc: ";
  char *date_string = "Date: ";
  char *sender_string = "Sender: ";
  char *from_string = "From: ";
  char *to_string = "To: ";
  char *reply_string = "Reply-To: ";
  char *subject_string = "Subject: ";

  pos.y = pos.x = 0;

#if 0
  _body = te->GetText();
#else
#endif

// my_to = (char*) bto->GetString();

  if (attList == NULL) {
    GenerateMessagePlusHeader(_body);
    _body->InsLine(0, "Content-Type: text/plain");
    _body->InsLine(0, "Mime-Version: 1.0");
  } else {
    _body->InsLine(0, "Content-Type: text/plain");
    temp = new char[strlen(bound) + 3];
    sprintf(temp, "--%s", bound);
    _body->InsLine(0, temp);
    delete[] temp;
    GenerateMessagePlusHeader(_body);
    temp = new char[strlen("Content-Type: multipart/mixed;Boundary=\"\"") +
		    strlen(bound) + 1];
    sprintf(temp, "Content-Type: multipart/mixed; Boundary=\"%s\"", bound);
    _body->InsLine(0, temp);
    delete[] temp;
    _body->InsLine(0, "Mime-Version: 1.0");
  }

  char *sender = Sender();
  temp = new char[strlen(sender_string) + strlen(sender) + 1];
  sprintf(temp, "%s%s", sender_string, sender);
  _body->InsLine(0, temp);
  delete[] temp;
  delete[] sender;

  temp = GenerateMessageId();
  temp2 = new char[strlen("Message-Id: ") + strlen(temp) + 1];
  sprintf(temp2, "Message-Id: %s", temp);
  _body->InsLine(0, temp2);
  delete[] temp2;
  delete[] temp;

  temp = new char[strlen("From ") + strlen(_session->GetReturnPath()) + 1];
  sprintf(temp, "From %s", _session->GetReturnPath());
  _body->InsLine(0, temp);
  delete[] temp;

  _body->Save(tempfile);
  if (attList != NULL) {
    g = fopen(tempfile, "a");  // ==!==
    fseek(g, 0, SEEK_END);
    CmpAttachList *travel = attList;
    while (travel != NULL) {
      fprintf(g, "\n");
      fprintf(g, "--%s\n", bound);
      fprintf(g, "%s\n", GenerateContentType(travel));
      fprintf(g, "Content-Transfer-Encoding: base64\n");
      fprintf(g, "\n");
      f = fopen(travel->file, "r");  // ==!==
      EncodeBase64(f, g);
      fclose(f);
      fprintf(g, "\n");
      travel = travel->next;
    }
    fprintf(g, "--%s--\n", bound);
    fclose(g);
  }
  _body->Clear();
  _body->Load(tempfile);
  f = fopen(tempfile, "r");
  if (mode == CMP_FILE_SEND_NOW) {
    SetupSmtp(_session->GetSmtpServer(), _session->GetSmtpPort());
    sendOK = SendMail(f, _session->GetIdentity(), _session->GetReturnPath(),
		 (char *) bto->GetString(), NULL, _my_cc);
    CloseConnect();
    if (sendOK == SEND_OK) {
      _body->Append(_session->AbsolutePath(_session->GetFolder("Sentmail")->GetName()));
      _session->GetFolder("Sentmail")->Reload();
    }
  }
  fclose(f);
  if ((mode == CMP_FILE_SEND_LATER) || (sendOK != SEND_OK)) {
    _body->Append(_session->AbsolutePath(_session->GetFolder("Unsent")->GetName()));
    _session->GetFolder("Unsent")->Reload();
  }
  unlink(tempfile);
}

void OXComposer::InitMenu() {
  _menuBarLayout = new OLayoutHints(LHINTS_TOP | LHINTS_LEFT | LHINTS_EXPAND_X, 0, 0, 1, 1);
  _menuBarItemLayout = new OLayoutHints(LHINTS_TOP | LHINTS_LEFT, 0, 4, 0, 0);

  _menuFile = new OXPopupMenu(_client->GetRoot());
  _menuFile->AddEntry(new OHotString("Send &Message"), CMP_FILE_SEND_NOW);
  _menuFile->AddEntry(new OHotString("Send &Later"), CMP_FILE_SEND_LATER);
  _menuFile->AddSeparator();
  _menuFile->AddEntry(new OHotString("&Save Message..."), CMP_FILE_SAVE);
  _menuFile->AddEntry(new OHotString("&Print Message..."), CMP_FILE_PRINT);
  _menuFile->AddSeparator();
  _menuFile->AddEntry(new OHotString("&Include Text File..."), CMP_FILE_INCLUDE);
  _menuFile->AddSeparator();
  _menuFile->AddEntry(new OHotString("&Close"), CMP_FILE_EXIT);

  _menuEdit = new OXPopupMenu(_client->GetRoot());
  _menuEdit->AddEntry(new OHotString("&Cut"), CMP_EDIT_CUT);
  _menuEdit->AddEntry(new OHotString("C&opy"), CMP_EDIT_COPY);
  _menuEdit->AddEntry(new OHotString("&Paste"), CMP_EDIT_PASTE);
  _menuEdit->AddEntry(new OHotString("&Delete"), CMP_EDIT_DELETE);
  _menuEdit->AddSeparator();
  _menuEdit->AddEntry(new OHotString("&Select all"), CMP_EDIT_SELECTALL);

  _menuEdit->DisableEntry(CMP_EDIT_COPY);
  _menuEdit->DisableEntry(CMP_EDIT_CUT);
  _menuEdit->DisableEntry(CMP_EDIT_DELETE);

  _menuSearch = new OXPopupMenu(_client->GetRoot());
  _menuSearch->AddEntry(new OHotString("&Search..."), CMP_SEARCH_SEARCH);
  _menuSearch->AddEntry(new OHotString("&Search again"), CMP_SEARCH_AGAIN);

  _menuHelp = new OXPopupMenu(_client->GetRoot());
  _menuHelp->AddEntry(new OHotString("&Contents..."), CMP_HELP_CONTENTS);
  _menuHelp->AddEntry(new OHotString("&Search..."), CMP_HELP_SEARCH);
  _menuHelp->AddSeparator();
  _menuHelp->AddEntry(new OHotString("&About..."), CMP_HELP_ABOUT);
  _menuHelp->DisableEntry(CMP_HELP_CONTENTS);
  _menuHelp->DisableEntry(CMP_HELP_SEARCH);

  _menuFile->Associate(this);
  _menuEdit->Associate(this);
  _menuSearch->Associate(this);
  _menuHelp->Associate(this);

  _menuBar = new OXMenuBar(this, 1, 1, HORIZONTAL_FRAME);
  _menuBar->AddPopup(new OHotString("&File"), _menuFile, _menuBarItemLayout);
  _menuBar->AddPopup(new OHotString("&Edit"), _menuEdit, _menuBarItemLayout);
  _menuBar->AddPopup(new OHotString("&Search"), _menuSearch, _menuBarItemLayout);
  _menuBar->AddPopup(new OHotString("&Help"), _menuHelp, _menuBarItemLayout);

  attachPopUp = new OXPopupMenu(_client->GetRoot());
  attachPopUp->AddEntry(new OHotString("&Delete"), CMP_DEL_ATTACH);
  attachPopUp->AddSeparator();
  attachPopUp->AddEntry(new OHotString("Copy"), CMP_COPY_ATTACH);
  attachPopUp->DisableEntry(CMP_COPY_ATTACH);
  attachPopUp->AddSeparator();
  attachPopUp->AddEntry(new OHotString("&Properties..."), CMP_PROP_ATTACH);
  attachPopUp->DisableEntry(CMP_PROP_ATTACH);
  attachPopUp->Associate(this);

  AddFrame(_menuBar, _menuBarLayout);
}

char *OXComposer::Sender() {
  int pos;
  char *buffer;
  char *retstring;
  char *host;

  host = new char[256];
  gethostname(host, 256);
  struct hostent *hostname = gethostbyname(host);
  delete[] host;

  host = new char[strlen(hostname->h_name) + 1];
  strcpy(host, hostname->h_name);
  struct passwd *user;
  int user_id;
  user_id = (int) getuid();
  user = getpwuid(user_id);
  buffer = new char[strlen(user->pw_name) + 2];
  strcpy(buffer, user->pw_name);
  buffer[strlen(user->pw_name)] = '@';
  buffer[strlen(user->pw_name) + 1] = '\0';
  retstring = new char[strlen(buffer) + strlen(host) + 1];
  sprintf(retstring, "%s%s", buffer, host);
  return retstring;
}

void OXComposer::AddAttach(char *fname) {
  OPicture *pic1, *pic2;
  OMime *travelmime;
  char *filetypes[] = {
    "All files",      "*",
    "Document files", "*.doc",
    "Text files",     "*.txt",
    NULL, NULL
  };

  OFileInfo fi;
  char *p, tmp[1024];

  std::vector<OString *> names;

  fi.filename = NULL;

  if (fname == NULL) {
    fi.MimeTypesList = MimeTypeList;
    fi.file_types = filetypes;
    new OXFileDialog(_client->GetRoot(), this, FDLG_OPEN, &fi);
    fname = fi.filename;
  }

  if (fname) {
    if (access(fname, R_OK) == -1) {
      sprintf(tmp, "Error opening file \"%s\"", fname);
      new OXMsgBox(_client->GetRoot(), this, new OString("Composer"),
                   new OString(tmp), MB_ICONSTOP, ID_OK);
    } else {
      if ((p = strrchr(fname, '/')) == NULL)
        p = fname;
      else
        ++p;
      CmpAttachList *travel = attList;
      if (travel == NULL) {
        attList = new CmpAttachList;
	travel = attList;
	travel->prev = NULL;
      } else {
        while (travel->next != NULL) travel = travel->next;
	travel->next = new CmpAttachList;
	travel->next->prev = travel;
	travel = travel->next;
      }
      travel->next = NULL;
      travel->name = new char[strlen(fname) + 1];
      strcpy(travel->name, fname);
      travel->file = new char[strlen(travel->name) + strlen(fi.ini_dir) + 2];
      sprintf(travel->file, "%s/%s", fi.ini_dir, travel->name);

      pic1 = (OPicture *) MimeTypeList->GetIcon(travel->file, False);
      pic2 = (OPicture *) MimeTypeList->GetIcon(travel->file, True);
      MimeTypeList->GetType(travel->file, travel->type);
      if (pic1 == NULL) {
        pic1 = (OPicture *) _client->GetPicture("doc.s.xpm");
	pic2 = (OPicture *) _client->GetPicture("doc.t.xpm");
	strcpy(travel->type, "application/unknown");
      }

      names.clear();
      names.push_back(new OString(travel->name));

      travel->item = new OListViewItem(attachmentsView, -1, pic1,
				       pic2, names, LV_LARGE_ICONS);
      attachmentsView->AddItem(travel->item);

      attachmentsView->Layout();
    }
  }
}

bool OXComposer::EncodeBase64(FILE *src, FILE *dest) {
  long countedBytes = 0;
  unsigned char value[4], newVal[4];
  unsigned char tempVal;
  unsigned char writeLine[62];
  unsigned char readLine[45];
  int more = 0;
  int count, count2;

  fseek(src, 0, SEEK_SET);
  fseek(dest, 0, SEEK_END);
  count2 = 0;

  do {
    countedBytes = fread(readLine, sizeof(char), 45, src);
    for (int i = 0; i < 62; i++) writeLine[i] = 0;
    more = (countedBytes % 3 != 0) ? 1 : 0;
    count = 0;
    for (int i = 0; i < countedBytes / 3 + more; i++) {
      for (int k = 0; k < 4; k++) newVal[k] = '\0';
      newVal[0] = readLine[i * 3];
      newVal[0] = newVal[0] >> 2;
      newVal[1] = readLine[i * 3];
      newVal[1] = newVal[1] << 6;
      newVal[1] = newVal[1] | (readLine[i * 3 + 1] >> 2);
      newVal[1] = newVal[1] >> 2;
      newVal[2] = readLine[i * 3 + 1];
      newVal[2] = newVal[2] << 4;
      newVal[2] = newVal[2] | (readLine[i * 3 + 2] >> 4);
      newVal[2] = newVal[2] >> 2;
      newVal[3] = readLine[i * 3 + 2];
      newVal[3] = newVal[3] << 2;
      newVal[3] = newVal[3] >> 2;
      writeLine[count++] = ToASCII64(newVal[0]);
      writeLine[count++] = ToASCII64(newVal[1]);
      writeLine[count++] = ToASCII64(newVal[2]);
      writeLine[count++] = ToASCII64(newVal[3]);
    }
    writeLine[count] = '\n';
    fputs((const char *) writeLine, dest);
  } while (countedBytes == 45);

  return True;
}

unsigned char OXComposer::ToASCII64(unsigned char character) {
  if (character <= 25) {
    return character + 65;
  } else if ((character > 25) && (character <= 51)) {
    return character + 71;
  } else if ((character > 51) && (character <= 61)) {
    return character - 4;
  } else if (character == 62) {
    return 43;
  } else if (character == 63) {
    return 47;
  } else {
    return '=';
  }
}

bool OXComposer::SetCurrentAttachment() {
  CmpAttachList *travelAttach = attList;
  const OListViewItem *item;
  std::vector<OItem *> items;

  if (attachmentsView->NumSelected() == 0) {
    currentAttach = NULL;
    return False;
  }
  items = attachmentsView->GetSelectedItems();
  item = (OListViewItem *) items[0];
  while (travelAttach != NULL) {
    if (travelAttach->item == item) {
      currentAttach = travelAttach;
      break;
    }
    travelAttach = travelAttach->next;
  }
  if (travelAttach == NULL) {
    return False;
  }
  return True;
}

void OXComposer::DelAttach() {
  OPicture *pic1, *pic2;
  std::vector<OString *> names;

  if (currentAttach == NULL)
    return;
  CmpAttachList *toDelete = currentAttach;
  if (currentAttach->next != NULL)
    currentAttach->next->prev = currentAttach->prev;
  if (currentAttach->prev != NULL)
    currentAttach->prev->next = currentAttach->next;
  if (currentAttach == attList)
    attList = currentAttach->next;
  delete[] currentAttach->file;
  delete[] currentAttach->name;
  attachmentsView->Clear();
  CmpAttachList *travel = attList;
  while (travel != NULL) {
    pic1 = (OPicture *) MimeTypeList->GetIcon(travel->file, False);
    pic2 = (OPicture *) MimeTypeList->GetIcon(travel->file, True);
    if (pic1 == NULL) {
      pic1 = (OPicture *) _client->GetPicture("doc.s.xpm");
      pic2 = (OPicture *) _client->GetPicture("doc.t.xpm");
    }

    names.clear();
    names.push_back(new OString(travel->name));

    travel->item = new OListViewItem(attachmentsView, -1, pic1, pic2,
				     names, LV_LARGE_ICONS);
    attachmentsView->AddItem(travel->item);
    travel = travel->next;
  }
  delete toDelete;
  currentAttach = NULL;
  attachmentsView->Layout();
}


char *OXComposer::GenerateContentType(CmpAttachList *travel) {
  char *ret =
      new char[strlen("Content-Type: ; name=\"\"") + strlen(travel->type) +
	       strlen(travel->name) + 1];
  sprintf(ret, "Content-Type: %s; name=\"%s\"", travel->type,
	  travel->name);
  return ret;
}

char *OXComposer::GenerateDate() {
  char *date = new char[256];
  struct tm *loctime;
  time_t curtime;
  curtime = time(NULL);
  loctime = localtime(&curtime);
  strftime(date, 256, "%a, %d %b %Y", loctime);
  return date;
}


char *OXComposer::GenerateMessageId() {
  char *id;
  time_t now;
  struct tm *now_x;

  now = time((time_t *) 0);
  now_x = localtime(&now);
  id = new char[256];
  char *tmp_string = Sender();
  char *host = strchr(tmp_string, '@');
  sprintf(id, "<NRXMail.%s.%d%d%d%d%d%d.%d%s>",
	  _session->GetClientVersion(), now_x->tm_year, now_x->tm_mon + 1,
	  now_x->tm_mday, now_x->tm_hour, now_x->tm_min, now_x->tm_sec,
	  getpid(), host);
  delete[] tmp_string;
  return id;
}


void OXComposer::InitToolBar() {
  _toolBar = new OXCompositeFrame(this, 10, 10, HORIZONTAL_FRAME);
  AddFrame(_toolBar, new OLayoutHints(LHINTS_EXPAND_X, 0, 0, 2, 2));
  _toolBarLine = new OXHorizontal3dLine(this, 4, 2);
  AddFrame(_toolBarLine, new OLayoutHints(LHINTS_EXPAND_X, 0, 0, 0, 0));

  sendNowButton =
      new OXPictureButton(_toolBar,
			  _client->GetPicture("sendNowButton.xpm",
					      sendNowButton_xpm), 5);
  _toolBar->AddFrame(sendNowButton,
		     new OLayoutHints(LHINTS_NORMAL, 2, 0, 0, 0));
  sendNowButton->Associate(this);
  sendNowButton->SetTip("Send Now");

  sendLaterButton =
      new OXPictureButton(_toolBar,
			  _client->GetPicture("sendLaterButton.xpm",
					      sendLaterButton_xpm), 6);
  _toolBar->AddFrame(sendLaterButton,
		     new OLayoutHints(LHINTS_NORMAL, 0, 0, 0, 0));
  sendLaterButton->Associate(this);
  sendLaterButton->SetTip("Send Later");

  includeFileButton =
      new OXPictureButton(_toolBar, _client->GetPicture("tb-open.xpm"), 7);
  _toolBar->AddFrame(includeFileButton,
		     new OLayoutHints(LHINTS_NORMAL, 5, 0, 0, 0));
  includeFileButton->Associate(this);
  includeFileButton->SetTip("Include Text File");

  saveButton =
      new OXPictureButton(_toolBar,
			  _client->GetPicture("saveButton.xpm",
					      saveButton_xpm), 8);
  _toolBar->AddFrame(saveButton,
		     new OLayoutHints(LHINTS_NORMAL, 5, 0, 0, 0));
  saveButton->Associate(this);
  saveButton->SetTip("Save Message");

  printButton =
      new OXPictureButton(_toolBar,
			  _client->GetPicture("printButton.xpm",
					      printButton_xpm), 9);
  _toolBar->AddFrame(printButton,
		     new OLayoutHints(LHINTS_NORMAL, 0, 0, 0, 0));
  printButton->Associate(this);
  printButton->SetTip("Print Message");
}


void OXComposer::SaveMail() {
  char *fullpath = NULL;

  const char *filetypes[8] = {
    "All files", "*",
    "Document files", "*.doc",
    "Text files", "*.txt",
    NULL, NULL
  };

  OFileInfo fi;
  char *p, tmp[1024];

  OText msg(_body);
  GenerateMessagePlusHeader(&msg);
  fi.MimeTypesList = MimeTypeList;
  fi.file_types = (char **) filetypes;
  new OXFileDialog(_client->GetRoot(), this, FDLG_SAVE, &fi);
  if (fi.filename) {
    fullpath = new char[strlen(fi.ini_dir) + strlen(fi.filename) + 2];
    sprintf(fullpath, "%s/%s", fi.ini_dir, fi.filename);
    if (!msg.Save(fullpath)) {
      sprintf(tmp, "Error saving mail to file \"%s\"", fullpath);
      new OXMsgBox(_client->GetRoot(), this, new OString("NRX MAIL"),
		   new OString(tmp), MB_ICONSTOP, ID_OK);
      return;
    }
  }
  if (fullpath) delete[] fullpath;
}


void OXComposer::PrintMail() {
  OText msg(_body);
  OPosition pos;
  pos.x = pos.y = 0;
  int ret;
  long count;
  char command[1024];
  char *tmpname = tmpnam(NULL);
  char *textBuffer;
  char *printer, *printcommand;

  printer = new char[strlen(_session->GetPrinter()) + 1];
  printcommand = new char[strlen(_session->GetPrintCommand()) + 1];
  strcpy(printer, _session->GetPrinter());
  strcpy(printcommand, _session->GetPrintCommand());
  new OXPrintBox(_client->GetRoot(), this, 400, 150, &printer,
		 &printcommand, &ret);
  if (ret == True) {
    GenerateMessagePlusHeader(&msg);
    msg.Save(tmpname);
    sprintf(command, "%s -P%s %s", printcommand, printer, tmpname);
    FILE *p = popen(command, "w");
    pclose(p);
    unlink(tmpname);
  }
}


void OXComposer::IncludeFile() {
  OPosition pos, pos2, pos3;
  OText *includedText = new OText();
  char *fullpath = NULL;

  const char *filetypes[8] = {
    "All files", "*",
    "Document files", "*.doc",
    "Text files", "*.txt",
    NULL, NULL
  };

  OFileInfo fi;
  char *p, tmp[1024];

  fi.MimeTypesList = MimeTypeList;
  fi.file_types = (char **) filetypes;
  new OXFileDialog(_client->GetRoot(), this, FDLG_BROWSE, &fi);
  if (fi.filename) {
    fullpath = new char[strlen(fi.ini_dir) + strlen(fi.filename) + 2];
    sprintf(fullpath, "%s/%s", fi.ini_dir, fi.filename);
    if (!includedText->Load(fullpath)) {
      sprintf(tmp, "Error loading file \"%s\"", fullpath);
      new OXMsgBox(_client->GetRoot(), this, new OString("NRX MAIL"),
		   new OString(tmp), MB_ICONSTOP, ID_OK);
    } else {
      pos.y = _body->RowCount();
      pos.x = 0;
      pos2.x = pos2.y = 0;
      pos3.y = includedText->RowCount();
      pos3.x = includedText->GetLineLength(pos3.y - 1);
      _body->InsText(pos, includedText, pos2, pos3);
// ==!== add the text to te too!
      te->Layout();
    }
  }
  if (fullpath) delete[] fullpath;
  delete includedText;
}


void OXComposer::GenerateMessagePlusHeader(OText *msg) {
  char *temp;

  if (((toall != NULL) && (toall->GetState() == BUTTON_DOWN))
      || ((toall == NULL) && (bcopy->GetTextLength() > 0)))
    _my_cc = (char *) bcopy->GetString();

  OPosition pos;
  pos.x = pos.y = 0;
  msg->BreakLine(pos);
  temp = new char[strlen("Subject: ") +
                  strlen((char *) bsubject->GetString()) + 1];
  sprintf(temp, "Subject: %s", (char *) bsubject->GetString());
  msg->InsLine(0, temp);
  delete[] temp;

  if ((_my_cc != NULL) && (strlen(_my_cc) > 0)) {
    temp = new char[strlen("CC: ") + strlen(_my_cc) + 1];
    sprintf(temp, "Cc: %s", _my_cc);
    msg->InsLine(0, temp);
    delete[] temp;
  }

  temp = new char[strlen("To: ") + strlen((char *) bto->GetString()) + 1];
  sprintf(temp, "To: %s", (char *) bto->GetString());
  msg->InsLine(0, temp);
  delete[] temp;

  char *tempdate = GenerateDate();
  temp = new char[strlen("Date: ") + strlen(tempdate) + 1];
  sprintf(temp, "Date: %s", tempdate);
  msg->InsLine(0, temp);
  delete[] temp;
  delete[] tempdate;
  if (strcmp(_session->GetIdentity(), "")) {
    temp = new char[strlen("From: ") + strlen(_session->GetIdentity()) + 2 +
                    strlen(_session->GetReturnPath()) + 1 + 1];
    sprintf(temp, "From: %s <%s>", _session->GetIdentity(),
	    _session->GetReturnPath());
  } else {
    temp = new char[strlen("From: ") + strlen(_session->GetReturnPath()) + 1];
    sprintf(temp, "From: %s", _session->GetReturnPath());
  }
  msg->InsLine(0, temp);
  delete[] temp;
}
