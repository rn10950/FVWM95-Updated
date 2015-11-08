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

#include "OXMoveMail.h"

//----------------------------------------------------------------------------------------

OXMoveMail::OXMoveMail(const OXWindow *p, const OXWindow *main, int w,
		       int h, char **names, long *num_val, int *ret_val,
                       unsigned long options) : 
  OXTransientFrame(p, main, w, h, options) {

  int i, ax, ay;
  Window wdummy;

  ret = ret_val;
  num = num_val;
  long x = 0;

  list = new OXListBox(this, 10, 10, 1);
  AddFrame(list, new OLayoutHints(LHINTS_CENTER_X));

  while (names[x] != NULL) {
    list->AddEntry(new OString(names[x]), x);
    delete names[x];
    x++;
  }
  delete[] names;
  list->Resize(150, 200);

  f1 = new OXCompositeFrame(this, 60, 20, HORIZONTAL_FRAME);
  MoveButton = new OXTextButton(f1, new OHotString("&Move"), 1);
  f1->AddFrame(MoveButton, new OLayoutHints(LHINTS_LEFT));
  MoveButton->Associate(this);

  CancelButton = new OXTextButton(f1, new OHotString("&Cancel"), 2);
  f1->AddFrame(CancelButton, new OLayoutHints(LHINTS_RIGHT));
  CancelButton->Associate(this);

  AddFrame(f1, new OLayoutHints(LHINTS_BOTTOM | LHINTS_EXPAND_X,
			    0, 0, 10, 0));

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

  SetWindowName("Move Mail To Folder");

  MapWindow();
  _client->WaitFor(this);
}

OXMoveMail::~OXMoveMail() {
}

int OXMoveMail::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;

  switch (msg->action) {
  case MSG_CLICK:
    switch (msg->type) {
      case MSG_BUTTON:
      switch (wmsg->id) {
      case 1:
	*ret = True;
	*num = list->GetSelected();
	CloseWindow();
	break;

      case 2:
	*ret = -1;
	CloseWindow();
	break;
      }
      break;
    }
    break;

  default:
    break;
  }
  return True;
}
