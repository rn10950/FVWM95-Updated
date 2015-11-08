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

#include <xclass/OXFileDialog.h>
#include <xclass/OXMsgBox.h>

#include "OXFolderProp.h"


//----------------------------------------------------------------------

OXFolderProp::OXFolderProp(const OXWindow *p, const OXWindow *main,
			   int w, int h, int *ret_val, OSession *session,
                           OFolder *folder, unsigned long options) :
  OXTransientFrame(p, main, w, h, options) {
  int i, ax, ay;
  int margin;
  Window wdummy;

  ret = ret_val;
  const char *tmpstring;

  _session = session;
  _folder = folder;

  if (_folder != NULL) {
//    tmpstring = _folder->GetName(); 
    _mode = _folder->GetMode();
  } else {
    _mode = 0;
  }

  hasChanged = False;

  f1 = new OXCompositeFrame(this, 60, 20, HORIZONTAL_FRAME);
  f3 = new OXCompositeFrame(this, 60, 20, HORIZONTAL_FRAME);
  bf1 = new OXCompositeFrame(f3, 75, 20, HORIZONTAL_FRAME | FIXED_WIDTH);
  bf2 = new OXCompositeFrame(f3, 75, 20, HORIZONTAL_FRAME | FIXED_WIDTH);
  f3->AddFrame(bf2, new OLayoutHints(LHINTS_RIGHT, 10));
  f3->AddFrame(bf1, new OLayoutHints(LHINTS_RIGHT));

  namelabel = new OXLabel(f1, new OHotString("&Name:"));
  margin = namelabel->GetWidth();
  margin = margin + 20;

  f1->AddFrame(namelabel, new OLayoutHints(LHINTS_CENTER_Y));
  name = new OXTextEntry(f1, tname = new OTextBuffer(256), 1);
  f1->AddFrame(name, new OLayoutHints(LHINTS_CENTER_Y | LHINTS_EXPAND_X,
                                      margin - namelabel->GetWidth()));

  name->Associate(this);
  name->Resize(200, name->GetDefaultHeight());
  AddFrame(f1, new OLayoutHints(LHINTS_LEFT | LHINTS_EXPAND_X, 5, 5, 10, 5));

  if (_mode & IS_SYSTEM) {
    name->Disable();
    info = new OXGroupFrame(this, new OString("System Folder"), VERTICAL_FRAME);
    info->AddFrame(new OXLabel(info, new OString("System folders cannot be renamed or deleted")),
		 new OLayoutHints(LHINTS_CENTER_X | LHINTS_CENTER_Y, 0, 0, 8, 0));
    AddFrame(info, new OLayoutHints(LHINTS_EXPAND_X, 5, 5, 5, 5));
  } else {
    info = new OXGroupFrame(this, new OString("User Folder"), VERTICAL_FRAME);
    info->AddFrame(new OXLabel(info, new OString("User folders are fully customizable")),
		 new OLayoutHints(LHINTS_CENTER_X | LHINTS_CENTER_Y, 0, 0, 8, 0));
    AddFrame(info, new OLayoutHints(LHINTS_EXPAND_X, 5, 5, 5, 5));
  }

  ftype = new OXGroupFrame(this, new OString("Folder Type"), VERTICAL_FRAME);
  AddFrame(ftype, new OLayoutHints(LHINTS_EXPAND_X, 5, 5, 0, 0));

  f5 = new OXCompositeFrame(ftype, 60, 20, HORIZONTAL_FRAME);
  ftype->AddFrame(f5, new OLayoutHints(LHINTS_EXPAND_X));
  f6 = new OXCompositeFrame(ftype, 60, 20, VERTICAL_FRAME);
  ftype->AddFrame(f6, new OLayoutHints(LHINTS_EXPAND_X));

  regFolder = new OXRadioButton(f5, new OHotString("&Regular Folder"), 1);
  f5->AddFrame(regFolder, new OLayoutHints(LHINTS_LEFT, 0, 0, 5, 0));
  regFolder->Associate(this);

  linkToFolder = new OXRadioButton(f6, new OHotString("&Link To Folder"), 2);
  f6->AddFrame(linkToFolder, new OLayoutHints(LHINTS_TOP, 0, 0, 2, 0));
  linkToFolder->Associate(this);

  f7 = new OXCompositeFrame(f6, 60, 20, HORIZONTAL_FRAME);
  f6->AddFrame(f7, new OLayoutHints(LHINTS_LEFT, 0, 0, 5, 0));

  BrowseButton = new OXTextButton(f7, new OHotString("&Browse..."), 1);
  f7->AddFrame(BrowseButton, new OLayoutHints(LHINTS_RIGHT, 5, 0, 0, 0));
  BrowseButton->Associate(this);

  linkpath = new OXTextEntry(f7, tlinkpath = new OTextBuffer(256), 3);
  f7->AddFrame(linkpath, new OLayoutHints(LHINTS_EXPAND_X));
  linkpath->Associate(this);
  linkpath->Resize(200, linkpath->GetDefaultHeight());

  OkButton = new OXTextButton(bf1, new OHotString("&OK"), 2);
  bf1->AddFrame(OkButton, new OLayoutHints(LHINTS_LEFT | LHINTS_EXPAND_X));
  OkButton->Associate(this);

  CancelButton = new OXTextButton(bf2, new OHotString("&Cancel"), 3);
  bf2->AddFrame(CancelButton,
		new OLayoutHints(LHINTS_RIGHT | LHINTS_EXPAND_X));
  CancelButton->Associate(this);

  SetDefaultAcceptButton(OkButton);
  SetDefaultCancelButton(CancelButton);

  if (_folder != NULL) {
    tmpstring = _folder->GetName();
    if (tmpstring) tname->AddText(0, tmpstring);
    if (_folder->IsLink()) {
      tlinkpath->AddText(0, _folder->GetLinkPath());
      regFolder->SetState(BUTTON_UP);
      linkToFolder->SetState(BUTTON_DOWN);
      linkpath->Enable();
    } else {
      regFolder->SetState(BUTTON_DOWN);
      linkToFolder->SetState(BUTTON_UP);
      BrowseButton->Disable();
      linkpath->Disable();
    }
  } else {
    regFolder->SetState(BUTTON_DOWN);
    linkToFolder->SetState(BUTTON_UP);
    BrowseButton->Disable();
    linkpath->Disable();
  }

  AddFrame(f3, new OLayoutHints(LHINTS_LEFT | LHINTS_BOTTOM | LHINTS_EXPAND_X,
			        5, 5, 10, 5));

  SetMWMHints(MWM_DECOR_ALL | MWM_DECOR_MAXIMIZE | MWM_DECOR_MENU,
	      MWM_FUNC_ALL | MWM_FUNC_MAXIMIZE | MWM_FUNC_RESIZE,
	      MWM_INPUT_MODELESS);

  MapSubwindows();
  Resize(GetDefaultSize());

  // position relative to the parent's window
  XTranslateCoordinates(GetDisplay(),
			main->GetId(), GetParent()->GetId(),
			(((OXFrame *) main)->GetWidth() - _w) >> 1,
			(((OXFrame *) main)->GetHeight() - _h) >> 1,
			&ax, &ay, &wdummy);
  Move(ax, ay);

  SetWindowName("Folder Properties");

  MapWindow();
  _client->WaitFor(this);
}

OXFolderProp::~OXFolderProp() {
}

void OXFolderProp::SetLinkPath() {
  OFileInfo fi;
  char *p, tmp[1024];

  static char *filetypes[] = {
    "All files", "*",
    "Document files", "*.doc",
    "Text files", "*.txt",
    NULL, NULL
  };

  fi.filename = NULL;
  fi.MimeTypesList = NULL;
  fi.file_types = filetypes;

  new OXFileDialog(_client->GetRoot(), this, FDLG_BROWSE, &fi);
  if (fi.filename != NULL) {
    hasChanged = True;
    strcpy(tmp, fi.ini_dir);
    tmp[strlen(fi.ini_dir)] = '/';
    strcpy(tmp + strlen(fi.ini_dir) + 1, fi.filename);
    tlinkpath->Clear();
    tlinkpath->AddText(0, tmp);
  }
}

int OXFolderProp::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;
  const char *string;
  char *tmp;
  switch (msg->type) {
  case MSG_BUTTON:

    switch (msg->action) {
    case MSG_CLICK:
      switch (wmsg->id) {
      case 1:
	SetLinkPath();
	break;

      case 2:
	if (tname->GetTextLength() == 0) {
	  new OXMsgBox(_client->GetRoot(), this, new OString("NRX Mail"),
		       new OString("No foldername specified"), MB_ICONSTOP,
		       ID_OK);
	  break;
	}
	if (_session->FolderNameExists((char *) tname->GetString()))
	  if ((_folder == NULL)
	      || (strcmp((char *) tname->GetString(), _folder->GetName()))) {
	    new OXMsgBox(_client->GetRoot(), this, new OString("NRX Mail"),
			 new OString("Foldername already exists"),
			 MB_ICONSTOP, ID_OK);
	    break;
	  }
	if (_mode & IS_LINK)
	  if (tlinkpath->GetTextLength() == 0) {
	    new OXMsgBox(_client->GetRoot(), this, new OString("NRX Mail"),
			 new OString("No linkpath specified"), MB_ICONSTOP,
			 ID_OK);
	    break;
	  } else if (access(tlinkpath->GetString(), R_OK | W_OK) == -1) {
	    new OXMsgBox(_client->GetRoot(), this, new OString("NRX Mail"),
			 new OString("Linked  folder not accessable"),
			 MB_ICONSTOP, ID_OK);
	    break;
	  }
	if (hasChanged) {
	  if (_folder == NULL)
	    _session->AddFolder((char *) tname->GetString(), _mode,
				(char *) tlinkpath->GetString());
	  else
	    _session->ChangeFolder(_folder->GetName(),
				   (char *) tname->GetString(), _mode,
				   (char *) tlinkpath->GetString());
	  *ret = 1;
	} else
	  *ret = 0;
	CloseWindow();
	break;

      case 3:
	*ret = -1;
	CloseWindow();
	break;
      }
      break;
    }
    break;

  case MSG_RADIOBUTTON:
    switch (msg->action) {
    case MSG_CLICK:
      switch (wmsg->id) {
      case 1:
	if (linkToFolder->GetState() == BUTTON_DOWN) {
	  _mode = _mode & !IS_LINK;
	  hasChanged = True;
	  regFolder->SetState(BUTTON_DOWN);
	  linkToFolder->SetState(BUTTON_UP);
	  BrowseButton->Disable();
	  linkpath->Disable();
	}
	break;

      case 2:
	if (regFolder->GetState() == BUTTON_DOWN) {
	  _mode = _mode | IS_LINK;
	  hasChanged = True;
	  regFolder->SetState(BUTTON_UP);
	  linkToFolder->SetState(BUTTON_DOWN);
	  BrowseButton->Enable();
	  linkpath->Enable();
	}
	break;
      }
      break;

    default:
      break;
    }
    break;

  case MSG_TEXTENTRY:
    switch (msg->action) {
    case MSG_TEXTCHANGED:
      hasChanged = True;
      if (wmsg->id == 3)
	_mode = _mode | IS_LINK;
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
