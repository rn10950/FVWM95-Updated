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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <time.h>
#include <sys/stat.h>

#include <xclass/utils.h>
#include <xclass/OXClient.h>
#include <xclass/OX3dLines.h>
#include <xclass/OXMsgBox.h>
#include <xclass/OXPropertiesDialog.h>
#include <xclass/OXTextEntry.h>
#include <xclass/OXCheckButton.h>
#include <xclass/OXGroupFrame.h>
#include <xclass/OXIcon.h>
#include <xclass/OXLabel.h>
#include <xclass/OString.h>
#include <xclass/OPicture.h>
#include <xclass/O2ColumnsLayout.h>

#include "OFile.h"
#include "OXProperties.h"


//---------------------------------------------------------------------

OXFtpPropertiesDialog::OXFtpPropertiesDialog(const OXWindow *p, const OXWindow *main,
                    int *rtc, char *path, OFile *fitem,
                    const OPicture *pic, int local, unsigned long options) :
  OXTransientFrame(p, main, 100, 100, options) {
    int  width, height;
    char wname[PATH_MAX];
    OXCompositeFrame *frame;
    OXButton *b1, *b2;

    _rtc = rtc;
    _local = local;
    _path = StrDup(path);
    _fitem = fitem;
    _pic = pic;
    _link = _client->GetPicture("slink.s.xpm");

    frame = new OXHorizontalFrame(this, 60, 20, FIXED_WIDTH);
    b1 = new OXTextButton(frame, new OHotString("OK"), ID_OK);
    b2 = new OXTextButton(frame, new OHotString("Cancel"), ID_CANCEL);
    _apply = new OXTextButton(frame, new OHotString("&Apply"), ID_APPLY);
    b1->Associate(this);
    b2->Associate(this);
    _apply->Associate(this);
    _bl1 = new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 3, 3, 0, 0);
    _bl2 = new OLayoutHints(LHINTS_BOTTOM | LHINTS_RIGHT, 0, 5, 0, 4);
    frame->AddFrame(b1, _bl1);
    frame->AddFrame(b2, _bl1);
    frame->AddFrame(_apply, _bl1);
    width = b1->GetDefaultWidth();   
    width = max(width, b2->GetDefaultWidth());
    width = max(width, _apply->GetDefaultWidth());
    frame->Resize((width + 20) * 3, frame->GetDefaultHeight());
    AddFrame(frame, _bl2);

    _l1 = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_TOP, 10, 0, 10, 5);
    _l2 = new OLayoutHints(LHINTS_CENTER_Y | LHINTS_EXPAND_X, 10, 0, 0, 0);
    _l3 = new OLayoutHints(LHINTS_CENTER_Y | LHINTS_LEFT, 5, 0, 5, 5);
    _l4 = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_TOP, 5, 5, 5, 5);
    _l5 = new OLayoutHints(LHINTS_NORMAL);
    _l6 = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y, 1, 1, 0, 0);
    _l7 = new OLayoutHints(LHINTS_NORMAL, 5, 0, 3, 0);
    _l8 = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_TOP, 5, 5, 5, 5);

    AddFrame(frame = new OXHorizontalFrame(this, 10, 10), _l1);
    _fileIcon = new OXFileIcon(frame, _pic, _fitem->_is_link ? _link : NULL,
                               CHILD_FRAME, _defaultFrameBackground);

    _fileName = new OXTextEntry(frame, NULL, 101);
    _fileName->ChangeOptions(_fileName->GetOptions() & 
              ~(RAISED_FRAME | SUNKEN_FRAME | DOUBLE_BORDER));
    _fileName->SetBackgroundColor(_defaultFrameBackground);
    _fileName->Associate(this);
    frame->AddFrame(_fileIcon, _l3);
    frame->AddFrame(_fileName, _l2);

    AddFrame(new OXHorizontal3dLine(this), _l4);

    //--- type, location, owner, group...

    OXCompositeFrame *d1;
    AddFrame(d1 = new OXCompositeFrame(this, 10, 10), _l1);
    d1->SetLayoutManager(new O2ColumnsLayout(d1, 10, 1));
    d1->AddFrame(new OXLabel(d1, new OString("Type:")), _l5);
    d1->AddFrame(ltype = new OXLabel(d1, new OString("")), _l5);
    d1->AddFrame(new OXLabel(d1, new OString("Location:")), _l5);
    d1->AddFrame(ldev = new OXLabel(d1, new OString("")), _l5);
    d1->AddFrame(new OXLabel(d1, new OString("Owner:")), _l5);
    d1->AddFrame(lusr = new OXLabel(d1, new OString("")), _l5);
    d1->AddFrame(new OXLabel(d1, new OString("Group:")), _l5);
    d1->AddFrame(lgrp = new OXLabel(d1, new OString("")), _l5);
    d1->AddFrame(new OXLabel(d1, new OString("Size:")), _l5);
    d1->AddFrame(lsz = new OXLabel(d1, new OString("")), _l5);
    d1->AddFrame(new OXLabel(d1, new OString("Last modified:")), _l5);
    d1->AddFrame(lmod = new OXLabel(d1, new OString("")), _l5);

    AddFrame(new OXHorizontal3dLine(this), _l4);

    AddFrame(new OXLabel(this, new OString("Access permissions:")), _l7);
    AddFrame(frame = new OXHorizontalFrame(this, 10, 10), _l8);

    OXGroupFrame *umode, *gmode, *omode;
    umode = new OXGroupFrame(frame, new OString("User"));
    gmode = new OXGroupFrame(frame, new OString("Group"));
    omode = new OXGroupFrame(frame, new OString("Others"));
    frame->AddFrame(umode, _l6);
    frame->AddFrame(gmode, _l6);
    frame->AddFrame(omode, _l6);

    _ur = new OXCheckButton(umode, new OHotString("Read"),    71);
    _uw = new OXCheckButton(umode, new OHotString("Write"),   72);
    _ux = new OXCheckButton(umode, new OHotString("Execute"), 73);
    _gr = new OXCheckButton(gmode, new OHotString("Read"),    74);
    _gw = new OXCheckButton(gmode, new OHotString("Write"),   75);
    _gx = new OXCheckButton(gmode, new OHotString("Execute"), 76);
    _or = new OXCheckButton(omode, new OHotString("Read"),    77);
    _ow = new OXCheckButton(omode, new OHotString("Write"),   78);
    _ox = new OXCheckButton(omode, new OHotString("Execute"), 79);

    _ur->Associate(this);
    _uw->Associate(this);
    _ux->Associate(this);
    _gr->Associate(this);
    _gw->Associate(this);
    _gx->Associate(this);
    _or->Associate(this);
    _ow->Associate(this);
    _ox->Associate(this);

    umode->AddFrame(_ur, _l5);
    umode->AddFrame(_uw, _l5);
    umode->AddFrame(_ux, _l5);
    gmode->AddFrame(_gr, _l5);
    gmode->AddFrame(_gw, _l5);
    gmode->AddFrame(_gx, _l5);
    omode->AddFrame(_or, _l5);
    omode->AddFrame(_ow, _l5);
    omode->AddFrame(_ox, _l5);

    AddFrame(new OXHorizontal3dLine(this), _l4);

    InitControls();

    MapSubwindows();

    width  = GetDefaultWidth();   // 330
    height = GetDefaultHeight();  // 336
    Resize(width, height);

    CenterOnParent();

    //---- make dialog non-resizable

    SetWMSize(width, height);
    SetWMSizeHints(width, height, width, height, 0, 0);

    sprintf(wname, "%s Properties", _fitem->_name);
    SetWindowName(wname);
    SetIconName("Properties");
    SetClassHints("XCLASS", "dialog");

    SetMWMHints(MWM_DECOR_ALL | MWM_DECOR_RESIZEH | MWM_DECOR_MAXIMIZE /*|
                                MWM_DECOR_MINIMIZE*/ | MWM_DECOR_MENU,
                MWM_FUNC_ALL | MWM_FUNC_RESIZE | MWM_FUNC_MAXIMIZE /*| 
                               MWM_FUNC_MINIMIZE*/,
                MWM_INPUT_MODELESS);

    MapWindow();
    _client->WaitFor(this);
}

OXFtpPropertiesDialog::~OXFtpPropertiesDialog() {
  delete _bl1; delete _bl2;
  delete _l1; delete _l2; delete _l3;
  delete _l4; delete _l5; delete _l6;
  delete _l7; delete _l8;
  delete[] _path;
}

void OXFtpPropertiesDialog::InitControls() {
  char *ftype, tmp[256], *tformat;
  struct tm *tms;
  int type;

  _fileName->Clear();
  _fileName->AddText(0, _fitem->_name);
  _client->NeedRedraw(_fileName);

  // type of file...
  type = _fitem->_type;
  if (S_ISLNK(type)) ftype = "Soft Link";
  else if (S_ISREG(type)) ftype = "Regular file";
  else if (S_ISDIR(type)) ftype = "File folder";
  else if (S_ISCHR(type)) ftype = "Character device";
  else if (S_ISBLK(type)) ftype = "Block device";
  else if (S_ISFIFO(type)) ftype = "FIFO";
  else if (S_ISSOCK(type)) ftype = "Socket";
  else ftype = "Unknown";
  ltype->SetText(new OString(ftype));

#if 0
  char *p1, *p2;
  p1 = _path;
  p2 = p1 + strlen(p1);
  while (p2[1] != '/' && p2 > p1) p2--;
  while (*p2 != '/' && p2 > p1) p2--;
  if (p2 != p1)
    sprintf(tmp, "...%s [%s]", p2, _local ? "local" : "remote");
  else
    sprintf(tmp, "%s [%s]", p1, _local ? "local" : "remote");
#else
  char *p1, *p = _path;

  while (strlen(p) > 20) {
    if (*p == '/') ++p;
    p1 = strchr(p, '/');
    if (!p1) break;
    p = p1;
  }
  if (p != _path)
    sprintf(tmp, "...%s [%s]", p, _local ? "local" : "remote");
  else
    sprintf(tmp, "%s [%s]", p, _local ? "local" : "remote");
#endif

  ldev->SetText(new OString(tmp));

  // file ownership...
  lusr->SetText(new OString(_fitem->_user));
  lgrp->SetText(new OString(_fitem->_group));

  // file size
  sprintf(tmp, "%ld bytes", _fitem->_size);
  lsz->SetText(new OString(tmp));

  // last access and modification...
  tformat = "%b %d, %Y  %R";
  tms = localtime(&_fitem->_time);
  strftime(tmp, 255, tformat, tms);
  lmod->SetText(new OString(tmp));

  // access permissions...
  _ur->SetState((type & S_IRUSR) ? BUTTON_DOWN : BUTTON_UP);
  _uw->SetState((type & S_IWUSR) ? BUTTON_DOWN : BUTTON_UP);
  _ux->SetState((type & S_IXUSR) ? BUTTON_DOWN : BUTTON_UP);
  _gr->SetState((type & S_IRGRP) ? BUTTON_DOWN : BUTTON_UP);
  _gw->SetState((type & S_IWGRP) ? BUTTON_DOWN : BUTTON_UP);
  _gx->SetState((type & S_IXGRP) ? BUTTON_DOWN : BUTTON_UP);
  _or->SetState((type & S_IROTH) ? BUTTON_DOWN : BUTTON_UP);
  _ow->SetState((type & S_IWOTH) ? BUTTON_DOWN : BUTTON_UP);
  _ox->SetState((type & S_IXOTH) ? BUTTON_DOWN : BUTTON_UP);

  _apply->Disable();

  Layout();
}

void OXFtpPropertiesDialog::ApplyProperties() {
  mode_t newstate = 0;
  char msg[256];

  if (_local == false) {
    int retval;

    new OXMsgBox(_client->GetRoot(), this,
          new OString("Apply"),
          new OString("Can't apply changes on a remote file!"),
          MB_ICONSTOP, ID_OK, &retval);

    InitControls();  // restore old filename
    return;
  }
  if (strcmp(_fitem->_name, _fileName->GetString()) != 0) {
    if (access(_fileName->GetString(), F_OK) == 0) {
      int retval;
      new OXMsgBox(_client->GetRoot(), this,
            new OString("Rename"),
            new OString("A file with the same name already exists. Overwrite?"),
            MB_ICONQUESTION, ID_YES | ID_NO, &retval);

      if (retval != ID_YES) {
        InitControls();  // restore old filename
        return;
      }
    }

    if (rename(_fitem->_name, _fileName->GetString()) != 0) {
      sprintf(msg, "Can't rename file: %s.", strerror(errno));
      new OXMsgBox(_client->GetRoot(), this,
                   new OString("Error"), new OString(msg),
                   MB_ICONSTOP, ID_OK);
      InitControls();
      return;
    }
    _fitem->SetName((char *)_fileName->GetString());
    //fileWindow->DisplayDirectory();
  }
  if (_ur->GetState() == BUTTON_DOWN) newstate = newstate | S_IRUSR;
  if (_uw->GetState() == BUTTON_DOWN) newstate = newstate | S_IWUSR;
  if (_ux->GetState() == BUTTON_DOWN) newstate = newstate | S_IXUSR;
  if (_gr->GetState() == BUTTON_DOWN) newstate = newstate | S_IRGRP;
  if (_gw->GetState() == BUTTON_DOWN) newstate = newstate | S_IWGRP;
  if (_gx->GetState() == BUTTON_DOWN) newstate = newstate | S_IXGRP;
  if (_or->GetState() == BUTTON_DOWN) newstate = newstate | S_IROTH;
  if (_ow->GetState() == BUTTON_DOWN) newstate = newstate | S_IWOTH;
  if (_ox->GetState() == BUTTON_DOWN) newstate = newstate | S_IXOTH;

  if (chmod(_fileName->GetString(), newstate) != 0){
    sprintf(msg, "Can't change file mode: %s.", strerror(errno));
    new OXMsgBox(_client->GetRoot(), this,
                 new OString("Error"), new OString(msg),
                 MB_ICONSTOP, ID_OK);
    InitControls();
  }
}

int OXFtpPropertiesDialog::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;

  switch (msg->type) {
    case MSG_BUTTON:
      switch (msg->action) {
        case MSG_CLICK:
          switch (wmsg->id) {
            case ID_OK:
              ApplyProperties();
              *_rtc = ID_OK;
              CloseWindow();
              break;

            case ID_CANCEL:
              *_rtc = ID_CANCEL;
              CloseWindow();
              break;

            case ID_APPLY:
              ApplyProperties();
              _apply->Disable();
              break;
          }
          break;
      }
      break;

    case MSG_CHECKBUTTON:
      switch (msg->action) {
        case MSG_CLICK:
          switch (wmsg->id) {
            /* nothing */
          }
          _apply->Enable();
          break;

        default:
          break;
      }
      break;

    case MSG_TEXTENTRY:
      switch (msg->action) {
        case MSG_TEXTCHANGED:
          if (wmsg->id == 101) _apply->Enable();
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
