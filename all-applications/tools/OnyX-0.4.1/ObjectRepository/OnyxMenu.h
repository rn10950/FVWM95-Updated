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

#ifndef __ONYXMENU_H
#define __ONYXMENU_H

#include <xclass/OXMenu.h>

#include "OnyxBase.h"
#include "OnyxMenuBar.h"


//----------------------------------------------------------------------

class OnyxMenu : public OnyxObject {
public:
  OnyxMenu();
  virtual ~OnyxMenu();

  virtual int Create();
  virtual void Enable();
  virtual void Disable();
  virtual void AddEntry(char *MenuText, int ID);
  virtual void AddMenu(char *MenuName, OXPopupMenu *Menu);
  virtual void AddSeparator();
  virtual void Check();
  virtual void UnCheck();
  virtual void CheckEntry(int ID);
  virtual void UnCheckEntry(int ID);
  virtual void EnableEntry(int ID);
  virtual void DisableEntry(int ID);

  int IsChecked;

  OnyxMenuBar *MenuBarParent;
  OnyxMenu *MenuParent;

protected:
  OXPopupMenu *InternalPopupMenu;
  int MenuExists;
};


#endif  // __ONYXMENU_H
