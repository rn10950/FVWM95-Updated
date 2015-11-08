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
#include <stdlib.h>

#include "OXPreferences.h"

//----------------------------------------------------------------------

OXPreferences::OXPreferences(const OXWindow *p, const OXWindow *main,
			     int w, int h, OSession *session, int *ret,
			     unsigned long options) : 
  OXTransientFrame(p, main, w, h, options) {

  int ax, ay;
  Window wdummy;

  _ret = ret;
  _session = session;
  _tab = new OXTab(this, 10, 10);
  AddFrame(_tab, new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y,
                                  5, 5, 5, 5));

  _mailOptions = _tab->AddTab(new OString("Mail"));
  _viewerOptions = _tab->AddTab(new OString("Viewer"));

  _frameLayout  = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_TOP, 5, 5, 0, 0);
  _frametLayout = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_TOP, 5, 5, 10, 0);
  _framebLayout = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_TOP, 5, 5, 0, 10);
  _labelLayout = new OLayoutHints(LHINTS_LEFT | LHINTS_CENTER_Y, 5, 5, 0, 0);
  _teLayout = new OLayoutHints(LHINTS_CENTER_Y | LHINTS_RIGHT, 5, 5, 0, 0);

  _f1 = new OXCompositeFrame(_mailOptions, 10, 10, HORIZONTAL_FRAME);
  _f2 = new OXCompositeFrame(_mailOptions, 10, 10, HORIZONTAL_FRAME);
  _f3 = new OXCompositeFrame(_mailOptions, 10, 10, HORIZONTAL_FRAME);
  _f4 = new OXCompositeFrame(_mailOptions, 10, 10, HORIZONTAL_FRAME);

  _lrealName = new OXLabel(_f1, new OHotString("&Name:"));
  _lreturnPath = new OXLabel(_f2, new OHotString("&Return Path:"));
  _linbox = new OXLabel(_f3, new OHotString("&Incoming Mail:"));
  _lsmtpServer = new OXLabel(_f4, new OHotString("&Mail Server:"));

  int MARGIN = _lrealName->GetDefaultWidth() + 5;
  if (MARGIN < _lreturnPath->GetDefaultWidth() + 5)
    MARGIN = _lreturnPath->GetDefaultWidth() + 5;
  if (MARGIN < _linbox->GetDefaultWidth() + 5)
    MARGIN = _linbox->GetDefaultWidth() + 5;
  if (MARGIN < _lsmtpServer->GetDefaultWidth() + 5)
    MARGIN = _lsmtpServer->GetDefaultWidth() + 5;

  _tbrealName = new OTextBuffer(256);
  _tbrealName->AddText(0, _session->GetIdentity());
  _f1->AddFrame(_lrealName, _labelLayout);
  _realName = new OXTextEntry(_f1, _tbrealName, 1);
  _f1->AddFrame(_realName, new OLayoutHints(LHINTS_CENTER_Y | LHINTS_EXPAND_X,
			       MARGIN - _lrealName->GetDefaultWidth(), 5, 2, 2));
  _mailOptions->AddFrame(_f1, _frametLayout);

  _tbreturnPath = new OTextBuffer(256);
  _tbreturnPath->AddText(0, _session->GetReturnPath());
  _f2->AddFrame(_lreturnPath, _labelLayout);
  _returnPath = new OXTextEntry(_f2, _tbreturnPath, 2);
  _f2->AddFrame(_returnPath, new OLayoutHints(LHINTS_CENTER_Y | LHINTS_EXPAND_X,
			         MARGIN - _lreturnPath->GetDefaultWidth(), 5, 2, 2));
  _mailOptions->AddFrame(_f2, _frameLayout);

  _tbinbox = new OTextBuffer(256);
  _tbinbox->AddText(0, _session->GetInBox());
  _f3->AddFrame(_linbox, _labelLayout);
  _inbox = new OXTextEntry(_f3, _tbinbox, 3);
  _f3->AddFrame(_inbox, new OLayoutHints(LHINTS_CENTER_Y | LHINTS_EXPAND_X,
			    MARGIN - _linbox->GetDefaultWidth(), 5, 2, 2));
  _mailOptions->AddFrame(_f3, _frameLayout);

  _tbsmtpServer = new OTextBuffer(256);
  _tbsmtpServer->AddText(0, _session->GetSmtpServer());
  _f4->AddFrame(_lsmtpServer, _labelLayout);

  _smtpServer = new OXTextEntry(_f4, _tbsmtpServer, 4);
  _tbsmtpPort = new OTextBuffer(256);
  _tbsmtpPort->AddText(0, itoa(_session->GetSmtpPort()));
  _lsmtpPort = new OXLabel(_f4, new OHotString("&Port:"));
  _smtpPort = new OXTextEntry(_f4, _tbsmtpPort, 5);
  _f4->AddFrame(_smtpServer, new OLayoutHints(LHINTS_CENTER_Y | LHINTS_EXPAND_X,
		                 MARGIN - _lsmtpServer->GetDefaultWidth(), 5, 2, 2));
  _f4->AddFrame(_lsmtpPort, _labelLayout);
  _f4->AddFrame(_smtpPort, new OLayoutHints(LHINTS_CENTER_Y, 2, 5, 2, 2));

  _mailOptions->AddFrame(_f4, _framebLayout);

  //-------------------------------------------------------

  _f8 = new OXCompositeFrame(_viewerOptions, 10, 10, HORIZONTAL_FRAME);
  _f9 = new OXCompositeFrame(_viewerOptions, 10, 10, HORIZONTAL_FRAME);
  _f10 = new OXCompositeFrame(_viewerOptions, 10, 10, HORIZONTAL_FRAME);

  _lprintCommand = new OXLabel(_f8, new OHotString("Print &Command:"));
  _lprinter = new OXLabel(_f9, new OHotString("&Printer:"));
  _lsignature = new OXLabel(_f10, new OHotString("&Signature File:"));

  MARGIN = _lprintCommand->GetDefaultWidth() + 5;
  if (MARGIN < _lprinter->GetDefaultWidth() + 5)
    MARGIN = _lprinter->GetDefaultWidth() + 5;
  if (MARGIN < _lsignature->GetDefaultWidth() + 5)
    MARGIN = _lsignature->GetDefaultWidth() + 5;

  _tbprintCommand = new OTextBuffer(256);
  _tbprintCommand->AddText(0, _session->GetPrintCommand());
  _f8->AddFrame(_lprintCommand, _labelLayout);
  _printCommand = new OXTextEntry(_f8, _tbprintCommand, 6);
  _f8->AddFrame(_printCommand, new OLayoutHints(LHINTS_CENTER_Y | LHINTS_EXPAND_X,
 			           MARGIN - _lprintCommand->GetDefaultWidth(), 5, 2, 2));
  _viewerOptions->AddFrame(_f8, _frametLayout);

  _tbprinter = new OTextBuffer(256);
  _tbprinter->AddText(0, _session->GetPrinter());
  _f9->AddFrame(_lprinter, _labelLayout);
  _printer = new OXTextEntry(_f9, _tbprinter, 7);
  _f9->AddFrame(_printer, new OLayoutHints(LHINTS_CENTER_Y | LHINTS_EXPAND_X,
			      MARGIN - _lprinter->GetDefaultWidth(), 5, 2, 2));
  _viewerOptions->AddFrame(_f9, _frameLayout);

  _tbsignature = new OTextBuffer(256);
  _tbsignature->AddText(0, _session->GetSignatureFile());
  _f10->AddFrame(_lsignature, _labelLayout);
  _signature = new OXTextEntry(_f10, _tbsignature, 7);
  _f10->AddFrame(_signature, new OLayoutHints(LHINTS_CENTER_Y | LHINTS_EXPAND_X,
			         MARGIN - _lsignature->GetDefaultWidth(), 5, 2, 2));
  _viewerOptions->AddFrame(_f10, _framebLayout);

  _f7 = new OXCompositeFrame(this, 10, 10, HORIZONTAL_FRAME);
  _bf1 = new OXCompositeFrame(_f7, 75, 10, HORIZONTAL_FRAME | FIXED_WIDTH);
  _bf2 = new OXCompositeFrame(_f7, 75, 10, HORIZONTAL_FRAME | FIXED_WIDTH);

  _f7->AddFrame(_bf2, new OLayoutHints(LHINTS_RIGHT, 2, 2, 2, 2));
  _f7->AddFrame(_bf1, new OLayoutHints(LHINTS_RIGHT, 2, 2, 2, 2));
  AddFrame(_f7, new OLayoutHints(LHINTS_EXPAND_X, 2, 2, 2, 2));

  _okButton = new OXTextButton(_bf1, new OHotString("&OK"), 1);
  _cancelButton = new OXTextButton(_bf2, new OHotString("&Cancel"), 2);
  _bf1->AddFrame(_okButton, new OLayoutHints(LHINTS_EXPAND_X, 2, 2, 0, 0));
  _bf2->AddFrame(_cancelButton, new OLayoutHints(LHINTS_EXPAND_X, 2, 2, 0, 0));

  _okButton->Associate(this);
  _cancelButton->Associate(this);

  SetDefaultAcceptButton(_okButton);
  SetDefaultCancelButton(_cancelButton);

  XTranslateCoordinates(GetDisplay(),
			main->GetId(), GetParent()->GetId(),
			(((OXFrame *) main)->GetWidth() - _w) >> 1,
			(((OXFrame *) main)->GetHeight() - _h) >> 1,
			&ax, &ay, &wdummy);
  Move(ax, ay);

  MapSubwindows();
#if 0
  Resize(GetDefaultSize());
#else
  Resize(w, h);
  Layout();
#endif
  SetWMMinSize(_w, _h);

  SetWindowName("Preferences");

  MapWindow();

  _client->WaitFor(this);
}

OXPreferences::~OXPreferences() {
}

int OXPreferences::ProcessMessage(OMessage *msg) {
  switch (msg->type) {
  case MSG_BUTTON:
    switch (msg->action) {
    case MSG_CLICK:
      switch (((OWidgetMessage *) msg)->id) {
      case 1:
	*_ret = 1;
	SaveData();
	CloseWindow();
	break;

      case 2:
	*_ret = -1;
	CloseWindow();
	break;

      }
      break;
    }
    break;
  }
  return True;
}

char *OXPreferences::itoa(int number) {
  static char numString[40];
  long divider = 10;
  long count = 1;
  int i;

  while (number / divider) {
    count++;
    divider = divider * 10;
  }
  numString[count] = '\0';
  divider = 1;
  for (int x = 0; x < count - 1; x++)
    divider = divider * 10;
  for (i = 0; i < count - 1; i++) {
    numString[i] = '0' + number / divider;
    number = number - (number / divider) * divider;
    divider = divider / 10;
  }
  numString[i] = '0' + number % 10;

  return numString;
}

void OXPreferences::SaveData() {

  _session->SetIdentity((char *) _tbrealName->GetString());
  _session->SetReturnPath((char *) _tbreturnPath->GetString());
  _session->SetSmtpServer((char *) _tbsmtpServer->GetString());
  _session->SetSignatureFile((char *) _tbsignature->GetString());
  _session->SetPrintCommand((char *) _tbprintCommand->GetString());
  _session->SetPrinter((char *) _tbprinter->GetString());
  _session->SetSmtpPort(atoi((char *) _tbsmtpPort->GetString()));

  if (strcmp((char *) _tbinbox->GetString(), _session->GetInBox())) {
    _session->SetInBox((char *) _tbinbox->GetString());
    _session->ChangeFolder("Inbox", "Inbox", 6,
			   (char *) _tbinbox->GetString());
    *_ret = 2;
  }

  _session->Save();
}
