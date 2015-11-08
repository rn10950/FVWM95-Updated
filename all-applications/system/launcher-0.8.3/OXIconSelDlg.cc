/**************************************************************************

    This file is part of xclauncher.
    Copyright (C) 1998-2002 Hector Peraza.

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

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <xclass/utils.h>
#include <xclass/OXMsgBox.h>
#include <xclass/OX3dLines.h>

#include "OXIconSelDlg.h"

// shall we substitute the contents of the OXFSDDListBox with the
// picture pool paths?


//----------------------------------------------------------------------

OXIconSelDialog::OXIconSelDialog(const OXWindow *p, const OXWindow *t,
                                 OString *ret) :
  OXFileDialog(p, t) {

    _FileDialog(FDLG_OPEN, new OFileInfo);

    _picstr = ret;

    AddFrame(new OXHorizontal3dLine(this), 
             new OLayoutHints(LHINTS_EXPAND_X | LHINTS_TOP, 3, 3, 4, 4));

    OXHorizontalFrame *hf = new OXHorizontalFrame(this, 10, 10);
    AddFrame(hf, new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y, 3, 3, 0, 3));

    OXVerticalFrame *vf = new OXVerticalFrame(hf, 10, 10);
    hf->AddFrame(vf, new OLayoutHints(LHINTS_EXPAND_X | LHINTS_TOP));

    _gf = new OXGroupFrame(hf, new OString("Preview"), HORIZONTAL_FRAME);
    hf->AddFrame(_gf, new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y));

    _lh1 = new OLayoutHints(LHINTS_LEFT | LHINTS_TOP, 10, 10, 10, 1);
    _lh2 = new OLayoutHints(LHINTS_LEFT | LHINTS_TOP, 10, 10, 1, 1);
    _lh3 = new OLayoutHints(LHINTS_LEFT | LHINTS_TOP, 10, 10, 1, 10);
    _lp = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_CENTER_Y, 10, 10, 0, 8);
    _lb = new OLayoutHints(LHINTS_CENTER_X | LHINTS_CENTER_Y, 0, 0, 0, 5);

    char cwd[PATH_MAX];
    getcwd(cwd, PATH_MAX);
    _picPool = new OPicturePool(_client, _client->GetResourcePool(), cwd);

    OString *s;

    s = new OString("Name:");
    _name = new OXLabel(vf, s);
    _name->SetTextAlignment(TEXT_LEFT | TEXT_CENTER_Y);
    vf->AddFrame(_name, new OLayoutHints(LHINTS_EXPAND_X | LHINTS_TOP,
                                         5, 2, 5, 5));

    s = new OString("Size:");
    _size = new OXLabel(vf, s);
    _size->SetTextAlignment(TEXT_LEFT | TEXT_CENTER_Y);
    vf->AddFrame(_size, new OLayoutHints(LHINTS_EXPAND_X | LHINTS_TOP,
                                         5, 2, 5, 5));

    _pic = _picPool->GetPicture("app.s.xpm");
    _icon = new OXIcon(_gf, _pic, 32, 32);
    _gf->AddFrame(_icon, new OLayoutHints(LHINTS_CENTER, 20, 20, 20, 20));

    SetWindowName("Select Icon");

    MapSubwindows();
    Resize(GetDefaultSize());

    //---- position relative to the parent's window

    if (t) {
      int ax, ay;
      Window wdummy;
      XTranslateCoordinates(GetDisplay(),
                            t->GetId(), GetParent()->GetId(),
                            (((OXFrame *) t)->GetWidth() - _w) >> 1,
                            (((OXFrame *) t)->GetHeight() - _h) >> 1,
                            &ax, &ay, &wdummy);
      Move(ax, ay);
      SetWMPosition(ax, ay);
    }

    //---- make the dialog box non-resizable

    SetWMSize(_w, _h);
    SetWMSizeHints(_w, _h, _w, _h, 0, 0);

    SetMWMHints(MWM_DECOR_ALL | MWM_DECOR_MAXIMIZE | MWM_DECOR_MENU,
                MWM_FUNC_ALL | MWM_FUNC_MAXIMIZE | MWM_FUNC_RESIZE,
                MWM_INPUT_MODELESS);

    MapWindow();

    _client->WaitFor(this);
}

OXIconSelDialog::~OXIconSelDialog() {
  if (_file_info->filename) {
    if (_picstr) {
      _picstr->Clear();
      _picstr->Append(_file_info->filename);
    }
  }
  delete _lh1;
  delete _lh2;
  delete _lh3;
  delete _lb;
  delete _file_info;
  delete _picPool;
}

int OXIconSelDialog::ProcessMessage(OMessage *msg) {
  OItemViewMessage *cmsg;
  OFileItem *f;
  std::vector<OItem *> items;

  switch (msg->type) {
#if 0
  case MSG_BUTTON:
    if (msg->action == MSG_CLICK && ((OWidgetMessage *)msg)->id == ID_OK) {
      if (_picstr) {
        _picstr->Clear();
        _picstr->Append(_fname->GetString());
      }
    }
    break;
#endif

  case MSG_LISTVIEW:
    cmsg = (OItemViewMessage *) msg;
    switch (cmsg->action) {
    case MSG_CLICK:
      if (cmsg->button == Button1) {
        if (_fv->NumSelected() == 1) {
          items = _fv->GetSelectedItems();
          f = (OFileItem *) items[0];
          if (!S_ISDIR(f->GetFileType())) {

            // hmmmm....
            char tmp[PATH_MAX];
            getcwd(tmp, PATH_MAX);
            _picPool->SetPath(tmp);

            const char *fname = f->GetName()->GetString();

            const OPicture *pic = _picPool->GetPicture(fname);

            sprintf(tmp, "Name: %s", pic ? fname : "");
            _name->SetText(new OString(tmp));

            if (pic)
              sprintf(tmp, "Size: %d x %d", pic->GetWidth(), pic->GetHeight());
            else
              sprintf(tmp, "Size:");

            _size->SetText(new OString(tmp));

            if (_pic) _picPool->FreePicture(_pic);
            _pic = _picPool->GetPicture(fname);
            _icon->SetPicture(_pic);
            _gf->Layout();

            if (pic) _picPool->FreePicture(pic);
          }
        }
      }
      break;
    }
  }

  return OXFileDialog::ProcessMessage(msg);
}
