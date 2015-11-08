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

#include "OnyxBase.h"
#include "OnyxMenuItem.h"

extern MessageCenter *CentralMessageCenter;


//----------------------------------------------------------------------

OnyxMenuItem::OnyxMenuItem() : OnyxObject("OnyxMenuItem") {
  MenuItemExists = 0;
  IsChecked = False;
  IsSeparator = False;
  X = 0;
  Y = 0;
  Width = 0;
  Height = 0;
}

int OnyxMenuItem::Create() {
  OnyxObject::Create();
  if (!MenuItemExists) {
    if (strlen(Text)) {
      IsSeparator = False;
      MenuParent->AddEntry(Text, ID);
      if (IsChecked) Check();
      if (!IsEnabled) Disable();
    } else {
      IsSeparator = True;
      MenuParent->AddSeparator();
    }
    MenuItemExists++;
  }
  return MenuItemExists;
}

void OnyxMenuItem::Check() {
  if (!IsSeparator) {
    IsChecked = True;
    MenuParent->CheckEntry(ID);
  }    
}

void OnyxMenuItem::UnCheck() {
  if (!IsSeparator) {
    IsChecked = False;
    MenuParent->UnCheckEntry(ID);
  }    
}

void OnyxMenuItem::Enable() {
  OnyxObject::Enable();
  if (!IsSeparator) {
    MenuParent->EnableEntry(ID);
  }    
}

void OnyxMenuItem::Disable() {
  OnyxObject::Disable();
  if (!IsSeparator) {
    MenuParent->DisableEntry(ID);
  }    
}

OnyxMenuItem::~OnyxMenuItem() {
}
