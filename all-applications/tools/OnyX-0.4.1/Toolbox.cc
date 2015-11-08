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

#include "Objects.h"
#include "Events.h"
#include "GlobalExtern.h"

OnyxButton *ToolboxItem[50];
int LastToolSelected;

void Toolbox_CloseSignaled() {
  ShowToolboxButton->Pop();
  ToolBoxMenuItem->UnCheck();
}

void Toolbox_Closed() {
}

void Toolbox_Load() {
  int i, x, y, buttonsize;
  struct ObjectList *UsableObject;

  i = 0;
  x = 0;
  y = 0;

  ToolBoxMenuItem->Check();

  while (i < 50) {
    if (ToolboxItem[i] != NULL) {
      delete ToolboxItem[i];
      ToolboxItem[i] = NULL;
    } else {
      break;
    }
    i++;
  }

  i = 0;

  buttonsize = 32;  // This will be dynamic later. Probably 16, 32, and 64.

  ToolboxItem[i] = new OnyxButton;  // First button is the "Pointer".
  ToolboxItem[i]->ID = ToolboxButtonIDModifier + i;
  ToolboxItem[i]->Parent = Toolbox->Self();
  ToolboxItem[i]->X = x;
  ToolboxItem[i]->Y = y;
  ToolboxItem[i]->Width = buttonsize;
  ToolboxItem[i]->Height = buttonsize;
  ToolboxItem[i]->IsSticky = True;
  ToolboxItem[i]->IsToggle = False;
  strcpy(ToolboxItem[i]->Picture, "onyx_POINT.xpm");
  strcpy(ToolboxItem[i]->ToolTip, "Object Selector");
  ToolboxItem[i]->EnableToolTip = True;
  ToolboxItem[i++]->Create();
  
  x = x + buttonsize;

  // Check to see if we need to go to the next line of buttons on the
  // toolbar.

  if (x + buttonsize > Toolbox->Width) {
    x = 0;
    y = y + buttonsize;
  }

  UsableObject = UsableObjects;

  // Cycle through the usable objects and add their buttons to the toolbox.

  while (UsableObject != NULL) {
    if (UsableObject->ToolboxViewable) {
      ToolboxItem[i] = new OnyxButton;
      ToolboxItem[i]->ID = ToolboxButtonIDModifier + i;
      ToolboxItem[i]->Parent = Toolbox->Self();
      ToolboxItem[i]->X = x;
      ToolboxItem[i]->Y = y;
      ToolboxItem[i]->Width = buttonsize;
      ToolboxItem[i]->Height = buttonsize;
      ToolboxItem[i]->IsSticky = True;
      ToolboxItem[i]->IsToggle = False;
      sprintf(ToolboxItem[i]->Picture, "%s.xpm", UsableObject->Name);
      strcpy(ToolboxItem[i]->ToolTip, UsableObject->ToolTip);
      ToolboxItem[i]->EnableToolTip = True;
      ToolboxItem[i++]->Create();
  
      x = x + buttonsize;

      // Check to see if we need to go to the next row of buttons on the
      // toolbar.

      if (x + buttonsize > Toolbox->Width) {
        x = 0;
        y = y + buttonsize;
      }
    }
    UsableObject = UsableObject->Next;
  }

  ToolboxItem[i] = NULL;  // Terminate.

  // Make sure last tool selected is available. It might not be if user
  // removes a usable object. Default to "Pointer"

  if (i <= LastToolSelected)
    LastToolSelected = 0;   // if other button is not available.

  // Size to exact button size dimmensions (in other words, trim off extra
  // space around the sides.)

  if (ToolboxItem[i - 1]->Y == 0) {
    // Single row, requires a possible greater chop off on the width.
    Toolbox->Resize(ToolboxItem[i-1]->X + buttonsize,
                    ToolboxItem[i-1]->Y + buttonsize);
  } else {
    // Multiple rows exists. Chop to the nearest button size.
    Toolbox->Resize(Toolbox->Width - (Toolbox->Width % buttonsize),
                    ToolboxItem[i-1]->Y + buttonsize);
  }

  ToolboxItem[LastToolSelected]->Push();  // Push selected button.
}

void Toolbox_WindowResized(int OldWidth, int OldHeight, int NewWidth, int NewHeight) {
  Toolbox->Show();  // Reload and move buttons if necessary.
}

void Toolbox_ButtonReleased(int Button, int x, int y) {
  if (Button == 3) CustomControlsWindow->Show();
}
