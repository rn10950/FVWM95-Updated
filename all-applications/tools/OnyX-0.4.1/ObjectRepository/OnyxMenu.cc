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
#include "OnyxMenu.h"

extern MessageCenter *CentralMessageCenter;


//----------------------------------------------------------------------

OnyxMenu::OnyxMenu() : OnyxObject("OnyxMenu") {
  MenuExists = 0;
  IsChecked = False;
  MenuBarParent = NULL;
  MenuParent = NULL;
}

int OnyxMenu::Create() {
  OnyxObject::Create();
  if (!MenuExists) {
    InternalPopupMenu = new OXPopupMenu(ApplicationRoot());
    if (MenuBarParent) MenuBarParent->AddMenu(Text, InternalPopupMenu);
    if (MenuParent) MenuParent->AddMenu(Text, InternalPopupMenu);
    if (IsChecked) Check();
    if (!IsEnabled) Disable();
    MenuExists++;
  }
  return MenuExists;
}

void OnyxMenu::AddEntry(char *MenuText, int ID) {
  InternalPopupMenu->AddEntry(new OHotString(MenuText), ID);
}

void OnyxMenu::AddMenu(char *MenuName, OXPopupMenu *Menu) {
  InternalPopupMenu->AddPopup(new OHotString(MenuName), Menu);
}

void OnyxMenu::Check() {
  InternalPopupMenu->CheckEntry(ID);
}

void OnyxMenu::UnCheck() {
  IsChecked = False;
  InternalPopupMenu->UnCheckEntry(ID);
}

void OnyxMenu::Enable() {
  OnyxObject::Enable();
  InternalPopupMenu->EnableEntry(ID);
}

void OnyxMenu::Disable() {
  OnyxObject::Disable();
  InternalPopupMenu->DisableEntry(ID);
}

void OnyxMenu::CheckEntry(int ID) {
  InternalPopupMenu->CheckEntry(ID);
}

void OnyxMenu::UnCheckEntry(int ID) {
  InternalPopupMenu->UnCheckEntry(ID);
}

void OnyxMenu::EnableEntry(int ID) {
  InternalPopupMenu->EnableEntry(ID);
}

void OnyxMenu::DisableEntry(int ID) {
  InternalPopupMenu->DisableEntry(ID);
}

void OnyxMenu::AddSeparator() {
  InternalPopupMenu->AddSeparator();
}

OnyxMenu::~OnyxMenu() {
  delete InternalPopupMenu;
}
