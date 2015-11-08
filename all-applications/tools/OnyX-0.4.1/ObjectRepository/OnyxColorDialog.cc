/**************************************************************************

    This file is part of OnyX, a visual development environment
    using the xclass toolkit. Copyright (C) 1997, 1998 Frank Hall.

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

#include <X11/X.h>

#include <xclass/OXMsgBox.h>

#include "OnyxColorDialog.h"

extern MessageCenter *CentralMessageCenter;


//----------------------------------------------------------------------

OnyxColorDialog::OnyxColorDialog() : OnyxObject("OnyxColorDialog") {
  strcpy(CurrentColor, "white");
}

void OnyxColorDialog::Show() {
  int retc;
  XColor xcolor;
  OColor color;

  retc = XParseColor(ApplicationClient()->GetDisplay(),
                     ApplicationClient()->GetDefaultColormap(),
                     CurrentColor, &xcolor);
  if (retc) {
    color = OColor(xcolor.red * 255 / 65535,
                   xcolor.green * 255 / 65535,
                   xcolor.blue * 255 / 65535);
  } else {
    color = OColor(255, 255, 255);
  }

  new OXColorDialog(DisplayWindow, Parent, &retc, &color);

  if (retc == ID_OK) {
    Cancelled = 0;
    sprintf(CurrentColor, "#%02x%02x%02x",
                          color.GetR(), color.GetG(), color.GetB());
  } else {
    Cancelled = 1;
  }
}

int OnyxColorDialog::Create() {
  OnyxObject::Create();
  return True;
}

OnyxColorDialog::~OnyxColorDialog() {
}
