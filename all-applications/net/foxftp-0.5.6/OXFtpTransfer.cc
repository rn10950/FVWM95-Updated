/**************************************************************************

    This file is part of foxftp, a ftp client for fOX.
    Copyright (C) 1998, 1999, G. Matzka.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

**************************************************************************/

#include <sys/stat.h>
#include <unistd.h>

#include <xclass/OString.h>
#include <xclass/OPicture.h>
#include <xclass/OXTextButton.h>
#include <xclass/OXCheckButton.h>
#include <xclass/OXPictureButton.h>
#include <xclass/OXClient.h>
#include <xclass/OXLabel.h>
#include <xclass/OXProgressBar.h>
#include <xclass/OXStatusBar.h>

#include "TDList.h"
#include "OFile.h"
#include "OFileSystem.h"
#include "OFtpFileHandler.h"
#include "OFtpMessage.h"
#include "OXFtpTransfer.h"
#include "OXSiteView.h"

#include "icons/earth0.xpm"
#include "icons/earth1.xpm"
#include "icons/earth2.xpm"
#include "icons/earth3.xpm"
#include "icons/earth4.xpm"
#include "icons/earth5.xpm"
#include "icons/earth6.xpm"
#include "icons/earth7.xpm"
#include "icons/earth8.xpm"
#include "icons/earth9.xpm"


//----------------------------------------------------------------------

OXFtpTransfer::OXFtpTransfer(const OXWindow *p, const OXWindow *main,
              int w, int h, OXSiteView *from, OXSiteView *to,
              TDDLList<ODir *> *list, unsigned long options) :
  OXTransientFrame(p, main, w, h, options) {

  _l1 = new OLayoutHints(LHINTS_NORMAL, 3, 3, 3, 3);
  _l2 = new OLayoutHints(LHINTS_CENTER_X | LHINTS_CENTER_Y, 5, 5, 0, 0);
  _l3 = new OLayoutHints(LHINTS_CENTER_X, 3, 3, 3, 3);
  _l4 = new OLayoutHints(LHINTS_CENTER_Y, 3, 3, 3, 3);
  _l5 = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_CENTER_Y);
  _l6 = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_BOTTOM, 0, 0, 5, 0);

  _filepic = _client->GetPicture("doc.t.xpm");

  _npic = 0;
  _pics[0] = _client->GetPicture("earth0.xpm", earth0_xpm);
  _pics[1] = _client->GetPicture("earth1.xpm", earth1_xpm);
  _pics[2] = _client->GetPicture("earth2.xpm", earth2_xpm);
  _pics[3] = _client->GetPicture("earth3.xpm", earth3_xpm);
  _pics[4] = _client->GetPicture("earth4.xpm", earth4_xpm);
  _pics[5] = _client->GetPicture("earth5.xpm", earth5_xpm);
  _pics[6] = _client->GetPicture("earth6.xpm", earth6_xpm);
  _pics[7] = _client->GetPicture("earth7.xpm", earth7_xpm);
  _pics[8] = _client->GetPicture("earth8.xpm", earth8_xpm);
  _pics[9] = _client->GetPicture("earth9.xpm", earth9_xpm);

  _frame1 = new OXCompositeFrame(this, 1, 1, HORIZONTAL_FRAME);

  _ftpIcon = new OXIcon(_frame1, _pics[0], 40, 40);
  _ftpIcon->ChangeOptions(RAISED_FRAME | DOUBLE_BORDER);

  _frame2 = new OXCompositeFrame(_frame1, 1, 1, VERTICAL_FRAME);

  _frame3 = new OXCompositeFrame(_frame2, 1, 1, HORIZONTAL_FRAME);
  _dirLabel = new OXLabel(_frame3, new OString("Dir: "));
  _dirName = new OXLabel(_frame3, new OString(""));
  _frame3->AddFrame(_dirLabel, _l4);
  _frame3->AddFrame(_dirName, _l5);

  _frame4 = new OXCompositeFrame(_frame2, 1, 1, HORIZONTAL_FRAME);
  _fileLabel = new OXLabel(_frame4, new OString("File: "));
  _fileIcon = new OXIcon(_frame4, _filepic, 16, 16);
  _fileName = new OXLabel(_frame4, new OString(""));
  _frame4->AddFrame(_fileLabel, _l4);
  _frame4->AddFrame(_fileIcon, _l4);
  _frame4->AddFrame(_fileName, _l5);

  _fileProg = new OXProgressBar(_frame2, 180, 18, PROGBAR_MODE1);
  _ftpProg = new OXProgressBar(_frame2, 180, 18, PROGBAR_MODE1);
  _fileProg->ShowPercentage(false);
  _ftpProg->ShowPercentage(false);
  
  _frame2->AddFrame(_frame3, _l1);
  _frame2->AddFrame(_frame4, _l1);
  _frame2->AddFrame(_fileProg, _l3);
  _frame2->AddFrame(_ftpProg, _l3);

  _frame1->AddFrame(_ftpIcon, _l2);
  _frame1->AddFrame(_frame2, _l1);

  _cancelButton = new OXTextButton(this, new  OHotString("&Cancel"), 20);
  _cancelButton->Associate(this);

  _statusBar = new OXStatusBar(this);

  AddFrame(_frame1, _l1);
  AddFrame(_cancelButton, _l3);
  AddFrame(_statusBar, _l6);

  SetDefaultCancelButton(_cancelButton);
  SetFocusOwner(_cancelButton);

  SetWindowName("Transfer Info");

  SetMWMHints(MWM_DECOR_ALL | MWM_DECOR_MAXIMIZE | MWM_DECOR_MENU,
            MWM_FUNC_ALL | MWM_FUNC_MAXIMIZE | MWM_FUNC_RESIZE,
            MWM_INPUT_MODELESS);

  MapSubwindows();
  Resize(GetDefaultSize());
  CenterOnParent();

  MapWindow();

  _rh = NULL;
  _wh = NULL;
  _main = main;
  _from = from;
  _to = to;
  _list = list;
  _ioBuf = new char[1024];
  _cFelem = NULL;
  _cDelem = NULL;
  _cList = NULL;
  _timer = NULL;

  _total = 0;
  for (int i = 0; i < list->GetSize(); i++) {
    ODir *delem = list->GetAt(i + 1);
    TDDLList<OFile *> *flist = delem->GetList();
    for (int j = 0; j < flist->GetSize(); j++) {
      OFile *felem = flist->GetAt(j + 1);
      if (!S_ISDIR(felem->_type))
        _total += felem->_size;
    }
  }

  _ftpProg->SetPosition(0);
  _fileProg->SetPosition(0);
  _ftpProg->SetRange(0, _total);

  DoStart();
}

OXFtpTransfer::~OXFtpTransfer() {
  DoFinish();
  if (_timer) delete _timer;
  _client->FreePicture(_filepic);
  for (int i = 0; i < 10; i++)
    _client->FreePicture(_pics[i]);
  if (_list) delete _list;
  if (_ioBuf) delete[] _ioBuf;
  delete _l1; delete _l2; delete _l3;
  delete _l4; delete _l5; delete _l6;
}

void OXFtpTransfer::DoFinish() {

  if (_cFelem) {
    char buf[256];
    sprintf(buf, "Transfer of %s finished", _cFelem->_name);
    OStatusMessage m1(STATUS_MESSAGE, STATUS_CHANGED, buf);
    SendMessage(GetTopLevel(), &m1);
  }
  if (_rh) {
    OHandle *h = ((OFtpFileHandler *)_rh)->GetHandle();
    _from->CloseFile(h);
    delete _rh;
    _rh = NULL;
  }
  if (_wh) {
    OHandle *h = ((OFtpFileHandler *)_wh)->GetHandle();
    _to->CloseFile(h);
    delete _wh;
    _wh = NULL;
  }
}

void OXFtpTransfer::DoStart() {
  int loop = 1;
  char buf[PATH_MAX];
  OHandle *h1 = NULL, *h2 = NULL;

  if (_list == NULL || _list->GetSize() == 0) {
    CloseWindow();
    return;
  }
  do {
    if (_cFelem == NULL) {
      if (_cDelem == NULL) {
        _cDelem = _list->GetHead();
      } else {
        ODir *tmp = _list->GetNext();
        if (tmp == NULL || tmp == _cDelem) {
          if (_to) _to->DisplayDirectory(true);
          CloseWindow();
          return;
        } else {
          _cDelem = tmp;
        }
      }
      if (_cDelem) {
        char *p1, *p2;
        p1 = _cDelem->GetPath();
        p2 = p1 + strlen(p1);
        while (p2[1] != '/' && p2 > p1) { p2--;}
        while (*p2 != '/' && p2 > p1) { p2--;}
	if (p2 != p1)
	  sprintf(buf, "...%s", p2);
        else
	  sprintf(buf, "%s", p1);
        _dirName->SetText(new OString(buf));
        _frame3->Resize(_frame3->GetDefaultSize());
        _client->NeedRedraw(_frame3);
        _cList = _cDelem->GetList();
        _cFelem = NULL;
        if (_cList == NULL || _cList->GetSize() == 0)
          continue;
      }
    }
    if (_cFelem == NULL) {
      _cFelem = _cList->GetHead();
    } else {
      OFile *tmp = _cList->GetNext();
      if (tmp == NULL || tmp == _cFelem) {
        _cFelem = NULL;
        continue;
      } else {
        _cFelem = tmp;
      }
    } 

    if (S_ISDIR(_cFelem->_type)) {
      _to->Mkdir(_cFelem->_name);
      continue;
    } else {

      char *p = strrchr(_cFelem->_name, '/');
      if (p) {
        sprintf(buf, "%s/%s", _cDelem->GetPath(), &p[1]);
        _fileName->SetText(new OString(&p[1]));
      } else {
        sprintf(buf, "%s/%s", _cDelem->GetPath(), _cFelem->_name);
        _fileName->SetText(new OString(_cFelem->_name));
      }
      _fileProg->SetRange(0, _cFelem->_size);
      _frame4->Resize(_frame4->GetDefaultSize());
      _client->NeedRedraw(_frame4);

      h1 = _from->OpenFile(buf, MODE_TRANS_GET | MODE_TRANS_IMAGE);
      if (h1) {
        h2 = _to->OpenFile(_cFelem->_name, MODE_TRANS_PUT | MODE_TRANS_IMAGE);
      }
      if (h1 == NULL || h2 == NULL) {
    	if (h1)
          _from->CloseFile(h1);
    	if (h2)
          _to->CloseFile(h2);
      } else {
    	loop = 0;
      }
    }
  } while (loop);

  sprintf(buf, "Transfer of %s started", _cFelem->_name);
  OStatusMessage m1(STATUS_MESSAGE, STATUS_CHANGED, buf);
  SendMessage(GetTopLevel(), &m1);

  _pending = _transfered = _stall = 0;
  gettimeofday(&_startTime, NULL);
  _rh = new OFtpFileHandler(this, XCM_READABLE, h1, NULL);
  _wh = new OFtpFileHandler(this, XCM_WRITABLE, h2, NULL);
  _client->UnregisterFileEvent(_wh);
  if (_timer == NULL)
    _timer = new OTimer(this, 500);
}

int OXFtpTransfer::HandleTimer(OTimer *t) {
  char buf[256], buf2[256];
  struct timeval now;
  int sec, usec, diff;

  if (t != _timer) return false;

  if (++_npic >= 10) _npic = 0;

  _ftpIcon->SetPicture(_pics[_npic]);
  _client->NeedRedraw(_ftpIcon);

  _stall++;

  gettimeofday(&now, NULL);
  sec = now.tv_sec - _startTime.tv_sec;
  usec = now.tv_usec - _startTime.tv_usec;

  if (usec < 0) {
    usec += 1000000;
    sec--;
  }
  diff = usec/100000 + sec * 10;
  if (diff != 0 && _cFelem->_size > 0) {
    sprintf(buf, "%ld%% of %ld ",
            (100 * _transfered) / _cFelem->_size, _cFelem->_size);
    if (_stall < 6) {
      unsigned long bps = (10 * (unsigned int)_transfered) / diff;
      if (bps > 1024L) {
        unsigned long tbps = bps / 1024L;
        if (tbps > 1024L) {
          sprintf(buf2, "%s (at %ld.%ld Mb/sec)", buf, tbps/1024, (tbps%1024)/103);
        } else {
          sprintf(buf2, "%s (at %ld.%ld Kb/sec)", buf, bps/1024, (bps%1024)/103);
        }
      } else {
        sprintf(buf2, "%s (at %ld bytes/sec)", buf, bps);
      }
    } else {
      sprintf(buf2, "%s (stalled)", buf);
    }
    _statusBar->SetText(0, new OString(buf2));
  }
  delete _timer;
  _timer = new OTimer(this, 500);
 
  return true;
}

int OXFtpTransfer::HandleFileEvent(OFileHandler *fh, unsigned int mask) {
  if (fh == _rh && mask == XCM_READABLE) {
    _pending = read(fh->GetFd(), _ioBuf, 1024);
    if (_pending == 0) {
      DoFinish();
      DoStart();
    } else {
      _transfered += _pending;
      _fileProg->SetPosition(_transfered);
      _ftpProg->SetPosition(_ftpProg->GetPosition() + _pending);
      _client->NeedRedraw(_fileProg);
      _client->NeedRedraw(_ftpProg);
      _client->UnregisterFileEvent(_rh);
      _client->RegisterFileEvent(_wh);
      _stall = 0;
    }
  } else if (fh == _wh && mask == XCM_WRITABLE) {
    int i = 0;
    do {
      i += write(fh->GetFd(), &_ioBuf[i], _pending - i);
    } while (i != _pending);
    _client->UnregisterFileEvent(_wh);
    _client->RegisterFileEvent(_rh);
  }
  return true;
}

int OXFtpTransfer::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;

  switch (msg->type) {
    case MSG_BUTTON:
      switch (msg->action) {
        case MSG_CLICK:
          switch (wmsg->id) {
            case 20:
   //           OWidgetMessage message(MSG_BUTTON, MSG_CLICK, 18);
   //           SendMessage(_main, &message);
              CloseWindow();
              break;
          }
          break;
      }
      break;
  }
  return true;
}
