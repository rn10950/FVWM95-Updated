/**************************************************************************

    This file is part of xclass.
    Copyright (C) 1996, 1997 David Barth, Hector Peraza.

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
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <xclass/OXToolBar.h>
#include <xclass/OXPictureButton.h>
#include <xclass/OXToolBarButton.h>
#include <xclass/OX3dLines.h>


//-------------------------------------------------------------------

OXToolBar::OXToolBar(const OXWindow *p, int w, int h,
                     unsigned int options) :
  OXHorizontalFrame(p, w, h, options) {

  _msgObject = p;
  _spacing = 0;
}

OXToolBar::~OXToolBar() {
  SListFrameElt *ptr;

  for (ptr = _flist; ptr; ptr = ptr->next) {
    delete ptr->layout;
  }
}

void OXToolBar::AddButton(OXButton *b, char *tip_text) {
  AddFrame(b, new OLayoutHints(LHINTS_TOP | LHINTS_LEFT, _spacing, 0, 0, 0));
  if (tip_text) b->SetTip(tip_text);
  _spacing = 0;
}

OXButton *OXToolBar::AddButton(const OPicture *pic, char *tip_text,
                               int type, int id) {
  OXPictureButton *button;

  button = new OXPictureButton(this, pic, id);
  AddFrame(button, new OLayoutHints(LHINTS_TOP | LHINTS_LEFT, 
                                    _spacing, 0, 0, 0));
  button->TakeFocus(False);
  button->RemoveInput(FocusChangeMask);
  button->Associate(_msgObject);

  button->SetType(type);
  button->SetTip(tip_text);

  _spacing = 0;

  return button;
}

void OXToolBar::AddSeparator() {
  _spacing = 8;
}

void OXToolBar::AddButtons(SToolBarData *tdata) {
  OXPictureButton *button;
  const OPicture *bpic;
  int i;

  for (i = 0; tdata[i].pixmap_name != NULL; ++i) {
    if (strlen(tdata[i].pixmap_name) == 0) {
      AddSeparator();
    } else {
      if (tdata[i].pixmap_data)
        bpic = _client->GetPicture(tdata[i].pixmap_name,
                                   tdata[i].pixmap_data);
      else
        bpic = _client->GetPicture(tdata[i].pixmap_name);
      if (!bpic) {
        FatalError("OXToolBar: bad or missing %spixmap: %s",
                   tdata[i].pixmap_data ? "inlined " : "",
                   tdata[i].pixmap_name);
      }
      tdata[i].button = AddButton(bpic, tdata[i].tip_text, tdata[i].type,
                                  tdata[i].id);
    }
  }
}

void OXToolBar::AddButtons(SToolBarDataEx *tdata,
                           int show_labels, int equal_widths) {
  int i, spacing = 0;
  OXToolBarButton *button;
  const OPicture *pic;

  for (i = 0; tdata[i].pixmap_name != NULL; ++i) {

    if (strlen(tdata[i].pixmap_name) > 0) {
      pic = _client->GetPicture(tdata[i].pixmap_name,
                                tdata[i].pixmap_data);

      button = new OXToolBarButton(this, pic,
                                   new OHotString(tdata[i].label),
                                   tdata[i].id);
      button->ShowLabel(show_labels);

      AddFrame(button, new OLayoutHints(LHINTS_CENTER_Y | LHINTS_LEFT,
                                        spacing, 0, 2, 2));

      if (tdata[i].pixmap_on_data) {
        pic = _client->GetPicture(tdata[i].pixmap_on_name,
                                  tdata[i].pixmap_on_data);
        button->SetOnPic(pic);
      }

      if (tdata[i].pixmap_disabled_data) {
        pic = _client->GetPicture(tdata[i].pixmap_disabled_name,
                                  tdata[i].pixmap_disabled_data);
        button->SetDisabledPic(pic);
      }

      button->Associate(_msgObject);
      button->SetType(tdata[i].type);

      if (tdata[i].id == -1) {
        button->Disable();
        button->ChangeOptions(CHILD_FRAME);
      } else {
        button->SetTip(tdata[i].tip_text);
      }
      tdata[i].button = button;

    } else {

      AddFrame(new OXVertical3dLine(this),
               new OLayoutHints(LHINTS_EXPAND_Y | LHINTS_LEFT,
                                spacing + 4, 4, 2, 2));

    }

    spacing = 4;
  }

  if (equal_widths) {
    int maxw = 0;

    for (i = 0; tdata[i].pixmap_name != NULL; ++i) {
      if (tdata[i].button) {
        ODimension size = tdata[i].button->GetDefaultSize();
        if (size.w > maxw) maxw = size.w;
      }
    }

    for (i = 0; tdata[i].pixmap_name != NULL; ++i) {
      if (tdata[i].button) {
        ODimension size = tdata[i].button->GetDefaultSize();
        tdata[i].button->Resize(maxw, size.h);
        tdata[i].button->ChangeOptions(tdata[i].button->GetOptions() | FIXED_WIDTH);
      }
    }
  }

}
