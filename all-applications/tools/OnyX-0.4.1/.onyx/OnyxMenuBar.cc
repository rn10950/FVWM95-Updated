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
#include "OnyxMenuBar.h"

extern MessageCenter *CentralMessageCenter;


//----------------------------------------------------------------------

RedirectedMenuBar::RedirectedMenuBar(const OXWindow *p, int w, int h, int ID) :
  OXMenuBar(p, w, h, HORIZONTAL_FRAME) {

  InternalID = ID;
}

int RedirectedMenuBar::ProcessMessage(OMessage *msg) {
  OnyxMessage omsg;

  omsg.onyx_type = MenuType;
  omsg.id = ((OWidgetMessage *) msg)->id;

  if (msg->type == MSG_MENU) {
    if (msg->action == MSG_SELECT) {
      omsg.action = MenuItemHighlighted;
    } else if (msg->action == MSG_CLICK) {
      omsg.action = MenuItemSelected;
    }
    return CentralMessageCenter->ProcessMessage(&omsg);
  }

  return OXMenuBar::ProcessMessage(msg);
}

RedirectedMenuBar::~RedirectedMenuBar() {
}


//----------------------------------------------------------------------

OnyxMenuBar::OnyxMenuBar() : OnyxObject("OnyxMenuBar") {
  MenuBarExists = 0;
  X = 0;
  Y = 0;
}

int OnyxMenuBar::Create() {
  OnyxObject::Create();
  if (!MenuBarExists) {
    InternalMenuBar = new RedirectedMenuBar(Parent, Width, Height, ID);
    InternalMenuBar->Associate(InternalMenuBar);  /* loop xclass messages to itself */
    InternalMenuBar->MapSubwindows();
    InternalMenuBar->Resize(Width, Height = 24);
    InternalMenuBar->Layout();
    InternalMenuBar->Move(X, Y);
    MenuBarExists++;
  }
  return MenuBarExists;
}

void OnyxMenuBar::AddMenu(char *MenuName, OXPopupMenu *Menu) {
  InternalMenuBar->AddPopup(new OHotString(MenuName), Menu, new OLayoutHints(LHINTS_TOP | LHINTS_LEFT, 0, 4, 1, 1));
  if (Menu) Menu->Associate(InternalMenuBar);
  InternalMenuBar->MapSubwindows();
  InternalMenuBar->Resize(Width, Height = InternalMenuBar->GetDefaultHeight());
  InternalMenuBar->Layout();
}

OnyxMenuBar::~OnyxMenuBar() {
  delete InternalMenuBar;
}
