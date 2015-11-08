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

#include "OXPrintBox.h"

#include "xpm/printerIcon.xpm"


//----------------------------------------------------------------------------------------

OXPrintBox::OXPrintBox(const OXWindow *p, const OXWindow *main, int w,
		       int h, char **printerName, char **printProg,
                       int *ret_val, unsigned long options) :
  OXTransientFrame(p, main, w, h, options) {
  int i, ax, ay;
  Window wdummy;

  pprinter = printerName;
  pprintCommand = printProg;

  ret = ret_val;
  ChangeOptions((GetOptions() & ~VERTICAL_FRAME) | HORIZONTAL_FRAME);

  f1 = new OXCompositeFrame(this, 60, 20, VERTICAL_FRAME | FIXED_WIDTH);
  f5 = new OXCompositeFrame(this, 60, 20, HORIZONTAL_FRAME);
  f2 = new OXCompositeFrame(this, 60, 20, VERTICAL_FRAME);
  f3 = new OXCompositeFrame(f2, 60, 20, HORIZONTAL_FRAME);
  f4 = new OXCompositeFrame(f2, 60, 20, HORIZONTAL_FRAME);

  PrintButton = new OXTextButton(f1, new OHotString("&Print"), 1);
  CancelButton = new OXTextButton(f1, new OHotString("&Cancel"), 2);
  f1->Resize(PrintButton->GetDefaultWidth() + 40, GetDefaultHeight());

  PrintButton->Associate(this);
  CancelButton->Associate(this);

  L1 = new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 2, 2, 2, 0);
  L2 = new OLayoutHints(LHINTS_TOP | LHINTS_RIGHT | LHINTS_EXPAND_X,
			2, 5, 0, 2);
  L21 = new OLayoutHints(LHINTS_TOP | LHINTS_RIGHT, 2, 5, 10, 0);

  f1->AddFrame(PrintButton, L1);
  f1->AddFrame(CancelButton, L1);
  AddFrame(f1, L21);

  lprintCommand = new OXLabel(f3, new OHotString("Print &command:"));

  tbprintCommand = new OTextBuffer(20);
  tbprintCommand->AddText(0, *printProg);

  printCommand = new OXTextEntry(f3, tbprintCommand);
  printCommand->Associate(this);
  printCommand->Resize(100, printCommand->GetDefaultHeight());

  L5 = new OLayoutHints(LHINTS_LEFT | LHINTS_CENTER_Y, 3, 5, 0, 0);
  L6 = new OLayoutHints(LHINTS_RIGHT | LHINTS_CENTER_Y, 0, 2, 0, 0);

  f3->AddFrame(lprintCommand, L5);
  f3->AddFrame(printCommand, L6);

  lprinter = new OXLabel(f4, new OHotString("&Printer:"));

  tbprinter = new OTextBuffer(20);
  tbprinter->AddText(0, *printerName);

  printer = new OXTextEntry(f4, tbprinter);
  printer->Associate(this);
  printer->Resize(100, printer->GetDefaultHeight());

  f4->AddFrame(lprinter, L5);
  f4->AddFrame(printer, L6);

  L3 = new OLayoutHints(LHINTS_TOP | LHINTS_RIGHT, 2, 2, 2, 2);

  f2->AddFrame(f3, L1);
  f2->AddFrame(f4, L1);

  printerPicture = _client->GetPicture("printer.xpm", printerIcon_xpm);
  printerIcon = new OXIcon(f5, printerPicture, 32, 32);

  f5->AddFrame(printerIcon, new OLayoutHints(LHINTS_LEFT, 10, 10, 10, 10));
  f5->AddFrame(f2, L21);

  AddFrame(f5, L21);

  MapSubwindows();
  Resize(GetDefaultSize());

  // position relative to the parent's window
  XTranslateCoordinates(GetDisplay(),
			main->GetId(), GetParent()->GetId(),
			(((OXFrame *) main)->GetWidth() - _w) >> 1,
			(((OXFrame *) main)->GetHeight() - _h) >> 1,
			&ax, &ay, &wdummy);
  Move(ax, ay);
  SetMWMHints(MWM_DECOR_ALL | MWM_DECOR_MAXIMIZE | MWM_DECOR_MENU,
	      MWM_FUNC_ALL | MWM_FUNC_MAXIMIZE | MWM_FUNC_RESIZE,
	      MWM_INPUT_MODELESS);

  SetWindowName("Print mail");
  MapWindow();

  _client->WaitFor(this);
}

OXPrintBox::~OXPrintBox() {
  delete L1;
  delete L2;
  delete L3;
  delete L5;
  delete L6;
  delete L21;
}

int OXPrintBox::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;

  switch (msg->type) {
  case MSG_BUTTON:
    switch (msg->action) {
      case MSG_CLICK:
      switch (wmsg->id) {
      case 1:
	delete[] *pprinter;
	*pprinter = StrDup(tbprinter->GetString());

	delete[] *pprintCommand;
	*pprintCommand = StrDup(tbprintCommand->GetString());

	*ret = True;
	CloseWindow();
	break;

      case 2:
	*ret = False;
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
