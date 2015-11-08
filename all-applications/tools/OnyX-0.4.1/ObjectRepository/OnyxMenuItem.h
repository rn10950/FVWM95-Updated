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

#ifndef __ONYXMENUITEM_H
#define __ONYXMENUITEM_H

#include <xclass/OXMenu.h>

#include "OnyxBase.h"
#include "OnyxMenu.h"


//----------------------------------------------------------------------

class OnyxMenuItem : public OnyxObject {
public:
  OnyxMenuItem();
  virtual ~OnyxMenuItem();

  virtual int  Create();
  virtual void Check();
  virtual void UnCheck();
  virtual void Enable();
  virtual void Disable();

  int IsChecked;

  OnyxMenu* MenuParent;

protected:
  int IsSeparator;
  int MenuItemExists;
};


#endif  // __ONYXMENUITEM_H
