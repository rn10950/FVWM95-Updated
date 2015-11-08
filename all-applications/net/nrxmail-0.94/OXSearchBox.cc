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

#include "OXSearchBox.h"

#include "xpm/searchIcon.xpm"
#include "xpm/question.xpm"


//----------------------------------------------------------------------

OXSearchBox::OXSearchBox(const OXWindow *p, const OXWindow *main, int w,
			 int h, search_struct *sstruct, int *ret_val,
			 unsigned long options) : 
  OXTransientFrame(p, main, w, h, options) {
  int i, ax, ay;
  Window wdummy;

  ret = ret_val;
  *ret = False;
  s = sstruct;

  ChangeOptions((GetOptions() & ~VERTICAL_FRAME) | HORIZONTAL_FRAME);

  f1 = new OXCompositeFrame(this, 60, 20, VERTICAL_FRAME | FIXED_WIDTH);
  f2 = new OXCompositeFrame(this, 60, 20, HORIZONTAL_FRAME);
  f5 = new OXCompositeFrame(f2, 60, 20, VERTICAL_FRAME);
  f3 = new OXCompositeFrame(f5, 60, 20, HORIZONTAL_FRAME);
  f4 = new OXCompositeFrame(f5, 60, 20, HORIZONTAL_FRAME);

  searchPicture = _client->GetPicture("mailsearch.xpm", searchIconData);
  searchIcon = new OXIcon(f2, searchPicture, 32, 32);
  f2->AddFrame(searchIcon, new OLayoutHints(LHINTS_LEFT, 10, 10, 10, 10));

  SearchButton = new OXTextButton(f1, new OHotString("&Search"), 1);
  CancelButton = new OXTextButton(f1, new OHotString("&Cancel"), 2);

  SearchButton->Disable();
  f1->Resize(SearchButton->GetDefaultWidth() + 40, GetDefaultHeight());

  SearchButton->Associate(this);
  CancelButton->Associate(this);

  L1 = new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 2, 2, 3, 0);
  L2 = new OLayoutHints(LHINTS_TOP | LHINTS_RIGHT | LHINTS_EXPAND_X,
			2, 5, 0, 2);
  L21 = new OLayoutHints(LHINTS_TOP | LHINTS_RIGHT, 2, 5, 10, 0);

  f1->AddFrame(SearchButton, L1);
  f1->AddFrame(CancelButton, L1);
  AddFrame(f1, L21);

  lsearch = new OXLabel(f3, new OHotString("Search &for:"));

  search = new OXTextEntry(f3, tbsearch = new OTextBuffer(100));
  search->Associate(this);
  search->Resize(150, search->GetDefaultHeight());

  L5 = new OLayoutHints(LHINTS_LEFT | LHINTS_CENTER_Y, 3, 5, 0, 0);
  L6 = new OLayoutHints(LHINTS_LEFT | LHINTS_CENTER_Y, 0, 2, 0, 0);

  f3->AddFrame(lsearch, L5);
  f3->AddFrame(search, L6);

  L3 = new OLayoutHints(LHINTS_TOP | LHINTS_RIGHT, 2, 2, 2, 2);
  L9 = new OLayoutHints(LHINTS_BOTTOM | LHINTS_LEFT, 0, 0, 0, 0);
  L4 = new OLayoutHints(LHINTS_BOTTOM | LHINTS_LEFT, 0, 0, 5, 0);
  L10 = new OLayoutHints(LHINTS_BOTTOM | LHINTS_RIGHT, 0, 0, 5, 0);

  case_check = new OXCheckButton(f4, new OHotString("&Case sensitive"), 1);
  case_check->Associate(this);
  f4->AddFrame(case_check, L9);

  if (s->caseSensitive == False)
    case_check->SetState(BUTTON_UP);
  else
    case_check->SetState(BUTTON_DOWN);

  f5->AddFrame(f3, L1);
  f5->AddFrame(f4, L1);

  f2->AddFrame(f5, L1);
  AddFrame(f2, L2);
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

  SetWindowName("Search in mail");

  MapWindow();
  _client->WaitFor(this);
}

OXSearchBox::~OXSearchBox() {
  delete L1;
  delete L2;
  delete L3;
  delete L4;
  delete L5;
  delete L6;
  delete L21;
  delete L9;
  delete L10;
}

int OXSearchBox::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;

  switch (msg->type) {
  case MSG_BUTTON:

    switch (msg->action) {
    case MSG_CLICK:
      switch (wmsg->id) {
      case 1:
	s->buffer = StrDup(tbsearch->GetString());
	*ret = True;
	CloseWindow();
	break;

      case 2:
	*ret = False;
	CloseWindow();
	break;
      }
      break;

    default:
      break;
    }
    break;

  case MSG_CHECKBUTTON:
    switch (msg->action) {
    case MSG_CLICK:
      s->caseSensitive = !s->caseSensitive;
      break;

    default:
      break;
    }
    break;

  case MSG_TEXTENTRY:
    switch (msg->action) {
    case MSG_TEXTCHANGED:
      if (strlen(tbsearch->GetString()) == 0)
	SearchButton->Disable();
      else
	SearchButton->Enable();
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
