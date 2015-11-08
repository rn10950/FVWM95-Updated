/**************************************************************************

    This file is part of xclass.
    Copyright (C) 1998, 1999, H. Peraza.

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

#include <xclass/utils.h>
#include <xclass/OXLabel.h>
#include <xclass/OX3dLines.h>
#include <xclass/OXIcon.h>
#include <xclass/OXAboutDialog.h>

#include "icons/fOX.xpm"

//----------------------------------------------------------------------

OAboutInfo::OAboutInfo() {
  wname = title = copyright = text = NULL;
  title_justify = cprt_justify = text_justify = TEXT_JUSTIFY_CENTER;
  title_font = cprt_font = text_font = NULL;
}

//----------------------------------------------------------------------

OXAboutDialog::OXAboutDialog(const OXWindow *p, const OXWindow *main,
                             OAboutInfo *info, unsigned long options) :
  OXTransientFrame(p, main, 50, 50, options) {

    int width = 0;

    _lh1 = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_TOP, 0, 0, 10, 0);
    _lh2 = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_TOP, 0, 0, 5, 0);
    _lh3 = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_CENTER_Y, 0, 0, 10, 0);

    _lhi = new OLayoutHints(LHINTS_LEFT | LHINTS_CENTER_Y, 2, 2, 2, 2);
    _lhe = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_BOTTOM, 0, 0, 5, 5);
    _lhb = new OLayoutHints(LHINTS_BOTTOM | LHINTS_CENTER_X);
    _lhf = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y, 5, 5, 2, 2);

    ChangeOptions(HORIZONTAL_FRAME);
  
    _pic = _client->GetPicture("fOX.xpm", fOX_xpm);

    AddFrame(new OXIcon(this, _pic, 40, 40), _lhi);

    OXCompositeFrame *f1 = new OXCompositeFrame(this, 1, 1);

    OXLabel *l;

    if (info->title) {
      f1->AddFrame(l = new OXLabel(f1, new OString(info->title)), _lh1);
      l->SetTextJustify(info->title_justify);
      if (info->title_font) l->SetFont(info->title_font);
      width = l->GetDefaultWidth();
    }

    if (info->copyright) {
      f1->AddFrame(l = new OXLabel(f1, new OString(info->copyright)), _lh2);
      l->SetTextJustify(info->cprt_justify);
      if (info->cprt_font) l->SetFont(info->cprt_font);
      width = max(width, l->GetDefaultWidth());
    }

    if (info->text) {
      f1->AddFrame(l = new OXLabel(f1, new OString(info->text)), _lh3);
      l->SetTextJustify(info->text_justify);
      if (info->text_font) l->SetFont(info->text_font);
      if (width > 0) 
        l->SetWrapLength(width*4/3);
      else
        l->SetWrapLength(_pic->GetWidth() * 2);
    }

    _ok = new OXTextButton(f1, new OHotString("    &OK    "), 1);
    _ok->Associate(this);

    SetDefaultAcceptButton(_ok);
    SetDefaultCancelButton(_ok);
    SetFocusOwner(_ok);

    f1->AddFrame(_ok, _lhb);
    f1->AddFrame(new OXHorizontal3dLine(f1), _lhe);

    AddFrame(f1, _lhf);

    SetWindowName(info->wname ? info->wname : (char *) "About fOX");
    SetClassHints("XCLASS", "dialog");

    MapSubwindows();

    Resize(GetDefaultSize());

    //---- position relative to the parent's window

    CenterOnParent(False);

    //---- make the dialog box non-resizable

    SetWMSize(_w, _h);
    SetWMSizeHints(_w, _h, _w, _h, 0, 0);

    SetMWMHints(MWM_DECOR_ALL | MWM_DECOR_MAXIMIZE | MWM_DECOR_MENU,
                MWM_FUNC_ALL | MWM_FUNC_MAXIMIZE | MWM_FUNC_RESIZE,
                MWM_INPUT_MODELESS);

    MapWindow();

    _client->WaitFor(this);
}

OXAboutDialog::~OXAboutDialog() {
  delete _lh1;
  delete _lh2;
  delete _lh3;
  delete _lhi;
  delete _lhe;
  delete _lhb;
  delete _lhf;
  _client->FreePicture(_pic);
}

int OXAboutDialog::ProcessMessage(OMessage *msg) {
  switch (msg->type) {
    case MSG_BUTTON:
      switch (msg->action) {
        case MSG_CLICK:
          CloseWindow();
          break;
      }
      break;
  }
  return True;
}
