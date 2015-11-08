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

#include <vector>

#include "OXMailList.h"

#include "xpm/unreadMail.xpm"


//----------------------------------------------------------------------

OXMailList::OXMailList(const OXWindow *p, int w, int h, OFolder *folder) :
  OXListView(p, w, h, -1) {

    SetViewMode(LV_DETAILS, True);

    _folder = folder;

    SetItemFont(GetResourcePool()->GetFontPool()->GetFont("courier -12"));
    SetHeaderFont(GetResourcePool()->GetFontPool()->GetFont("helvetica -12"));

    AddColumn(new OString("State"), 0);
    AddColumn(new OString("Date"), 1, TEXT_LEFT);
    AddColumn(new OString("From"), 2, TEXT_LEFT);
    AddColumn(new OString("Subject"), 3, TEXT_LEFT);
    AddColumn(new OString(""), -1);

    SetColumnWidth(2, 150);
    SetColumnWidth(3, 300);

    if (_folder) ReloadList();
    Layout();
}

OXMailList::~OXMailList() {
}

OMail *OXMailList::GetCurrentMail() {
  if (NumSelected() == 0) return NULL;
  return _folder->GetMail(_anchorItem->GetId());
}

int OXMailList::GetCurrentMailNum() {
  if (NumSelected() == 0) return -1;
  return _anchorItem->GetId();
}

void OXMailList::ReloadList() {
  int i;
  std::vector<OString *> names;

  OPicture *pic = (OPicture *) _client->GetPicture("mailIcon.xpm",
                                                   unreadMailData);
  Clear();

  i = 0;
  OMail *mailTravel = _folder->GetFirstMail();

  while (mailTravel) {
    names.clear();
    names.push_back(new OString(""));  // state icon has no name attached
    names.push_back(new OString(mailTravel->GetDate())); // date
    if (mailTravel->GetFrom()) {
      if (mailTravel->GetFrom()->name != NULL)
        names.push_back(new OString(mailTravel->GetFrom()->name)); // from
      else
        names.push_back(new OString(mailTravel->GetFrom()->adress)); // from
    } else {
      names.push_back(new OString("No address")); // from
    }
    names.push_back(new OString(mailTravel->GetSubject())); // subject

    AddItem(new OListViewItem(this, i++, pic, pic, names, LV_DETAILS, True));

    mailTravel = _folder->GetNextMail(mailTravel);
  }

  Layout();
}

void OXMailList::LoadList(OFolder *folder) {
  _folder = folder;
  ReloadList(); 
}

/*
void OXMailList::MoveMail(char *newfolder) {
  _folder->MoveMail(_selected, newfolder);
  DeleteEntry(_selected);
  UnMark();
}
*/
