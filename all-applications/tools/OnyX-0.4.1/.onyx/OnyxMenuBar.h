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

#ifndef __ONYXMENUBAR_H
#define __ONYXMENUBAR_H

#include <xclass/OXMenu.h>
#include <xclass/OXCompositeFrame.h>

#include "OnyxBase.h"


#define MenuType             MSG_MENU  /* Define Menu Type */

#define MenuItemSelected     1   /* Define Select Event ID */
#define MenuItemHighlighted  2   /* Define Highlight Event ID */


//----------------------------------------------------------------------

class RedirectedMenuBar : public OXMenuBar {
public:
  RedirectedMenuBar(const OXWindow *p, int w, int h, int ID);
  virtual ~RedirectedMenuBar();

  virtual int ProcessMessage(OMessage *msg);

private:
  int InternalID;
};


//----------------------------------------------------------------------

class OnyxMenuBar : public OnyxObject {
public:
  OnyxMenuBar();
  virtual ~OnyxMenuBar();

  virtual int  Create();
  virtual void AddMenu(char *MenuName, OXPopupMenu *Menu);

protected:
  RedirectedMenuBar *InternalMenuBar;
  int MenuBarExists;
};


#endif  // __ONYXMENUBAR_H
