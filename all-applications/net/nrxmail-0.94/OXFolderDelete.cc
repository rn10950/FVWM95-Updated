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

#include "OXFolderDelete.h"

#include "xpm/question.xpm"


//----------------------------------------------------------------------

OXFolderDelete::OXFolderDelete(const OXWindow *p, const OXWindow *main,
			       int w, int h, int *ret_val,
			       unsigned long options) :
  OXTransientFrame(p, main, w, h, options) {

  int i, ax, ay;
  Window wdummy;

  _ret = ret_val;
  *_ret = 0;

  AddFrame(new OXLabel(this, new OString("This is a link to another folder")),
	   new OLayoutHints(LHINTS_CENTER_X | LHINTS_EXPAND_X, 2, 2, 5, 0));

  f4 = new OXCompositeFrame(this, 60, 20, HORIZONTAL_FRAME);
  AddFrame(f4, new OLayoutHints(LHINTS_EXPAND_X, 0, 0, 0, 0));
  picture = _client->GetPicture("question.xpm", questionIconData);
  icon = new OXIcon(f4, picture, 32, 32);
  f4->AddFrame(icon,
	       new OLayoutHints(LHINTS_LEFT | LHINTS_CENTER_Y, 5, 0, 0, 0));
  f5 = new OXCompositeFrame(f4, 10, 10, VERTICAL_FRAME);
  f4->AddFrame(f5, new OLayoutHints(LHINTS_NORMAL, 5, 0, 0, 0));

  f1 = new OXCompositeFrame(this, 60, 20, HORIZONTAL_FRAME);
  f2 = new OXCompositeFrame(f1, 75, 20, HORIZONTAL_FRAME | FIXED_WIDTH);
  f3 = new OXCompositeFrame(f1, 75, 20, HORIZONTAL_FRAME | FIXED_WIDTH);
  f1->AddFrame(f2, new OLayoutHints(LHINTS_LEFT, 2, 0, 0, 0));
  f1->AddFrame(f3, new OLayoutHints(LHINTS_RIGHT, 0, 2, 0, 0));

  link = new OXRadioButton(f5, new OHotString("&Delete only link"), 1);
  f5->AddFrame(link, new OLayoutHints(LHINTS_LEFT, 2, 2, 10, 0));
  link->Associate(this);

  link->SetState(BUTTON_DOWN);

  folder =
      new OXRadioButton(f5, new OHotString("&Delete also linked folder"), 2);
  f5->AddFrame(folder, new OLayoutHints(LHINTS_LEFT, 2, 2, 0, 0));
  folder->Associate(this);

  OkButton = new OXTextButton(f2, new OHotString("&OK"), 1);
  f2->AddFrame(OkButton, new OLayoutHints(LHINTS_LEFT | LHINTS_EXPAND_X));
  OkButton->Associate(this);

  CancelButton = new OXTextButton(f3, new OHotString("&Cancel"), 2);
  f3->AddFrame(CancelButton,
	       new OLayoutHints(LHINTS_RIGHT | LHINTS_EXPAND_X));
  CancelButton->Associate(this);

  AddFrame(f1,
	   new OLayoutHints(LHINTS_CENTER_X | LHINTS_EXPAND_X, 2, 2, 10, 0));

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

  SetWindowName("Delete Folder");

  MapWindow();
  _client->WaitFor(this);
}

OXFolderDelete::~OXFolderDelete() {
}

int OXFolderDelete::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;

  switch (msg->action) {
  case MSG_CLICK:
    switch (msg->type) {
    case MSG_BUTTON:
      switch (wmsg->id) {
      case 1:
	CloseWindow();
	break;

      case 2:
	*_ret = -1;
	CloseWindow();
	break;
      }
      break;

    case MSG_RADIOBUTTON:
      switch (wmsg->id) {
      case 1:
	link->SetState(BUTTON_DOWN);
	folder->SetState(BUTTON_UP);
	*_ret = 0;
	break;

      case 2:
	link->SetState(BUTTON_UP);
	folder->SetState(BUTTON_DOWN);
	*_ret = 1;
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
