/**************************************************************************

    This file is part of xclass, a Win95-looking GUI toolkit.
    Copyright (C) 1996, 1997 David Barth, Hector Peraza.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

**************************************************************************/

#include <xclass/utils.h>
#include <xclass/OXClient.h>
#include <xclass/OXPictureButton.h>
#include <xclass/OGC.h>


//-----------------------------------------------------------------

OXPictureButton::OXPictureButton(const OXWindow *p, const OPicture *pic,
                                 int ID, unsigned int option)
  : OXButton(p, ID, option) {

    _widgetType = "OXPictureButton";

    _pic = pic;
    _disabledPic = NULL;

    _tw = _pic->GetWidth();
    _th = _pic->GetHeight();

    TakeFocus(False);
}

void OXPictureButton::_DoRedraw() {

  int x = (_w - _tw) >> 1;
  int y = (_h - _th) >> 1;

  OXFrame::_DoRedraw();
  if (_state == BUTTON_DOWN || _state == BUTTON_ENGAGED) { ++x; ++y; }
  if (_state == BUTTON_ENGAGED) {
    FillRectangle(_hibckgndGC->GetGC(), 2, 2, _w-4, _h-4);
    DrawLine(_hilightGC, 2, 2, _w-3, 2);
  }

  if (IsEnabled()) {
    _pic->Draw(_client->GetDisplay(), _id, _bckgndGC, x, y);
  } else {
    if (_disabledPic) {
      _disabledPic->Draw(_client->GetDisplay(), _id, _bckgndGC, x, y);
    } else if (_pic->GetMask()) {
#if 0
      _pic->DrawMask(GetDisplay(), _id, _hilightGC, x+1, y+1);
      _pic->DrawMask(GetDisplay(), _id, _shadowGC,  x-1, y-1);
      _pic->DrawMask(GetDisplay(), _id, _bckgndGC,  x,   y);
#else
      _pic->DrawMask(GetDisplay(), _id, _hilightGC, x+1, y+1);
      _pic->DrawMask(GetDisplay(), _id, _shadowGC,  x,   y);
#endif
    } else {
      _pic->Draw(_client->GetDisplay(), _id, _bckgndGC, x, y);
    }
  }
}
