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

#ifndef __GLOBAL_H
#define __GLOBAL_H

#include "GlobalExtern.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int TotalProjectWindows;
int NextProjectID;
int RunPID;

int NextDevelopmentItemID;

struct ObjectList *UsableObjects;
struct ProjectObjectList *ProjectObjects;
struct ProjectObjectList *DeletedObjects;
struct ProjectObjectList *SelectedObject;
struct ProjectObjectList *SelectedWindow;
struct ProjectObjectPropertyList *SelectedProperty;
struct ProjectObjectList *DevelopmentItem[DevelopmentItemIDMax];

struct OnyxWindow *DevelopmentWindow[MaxDevelopmentWindows];
struct ProjectObjectList *WindowToListMap[MaxDevelopmentWindows];

OnyxObject *DevelopmentObject[MaxDevelopmentObjects];

char ProjectFile[50];
char ProjectDirectory[255];
char ProjectApplicationName[50];
char OnyxLocation[255];
char EditorCommand[255];
char OnyxIconLocation[255];

OnyxTextBox *PropertyValue[50];
int SelectedValue;

int ObjectTypeKnown(char *ObjectTypeName) {
  struct ObjectList *OList;

  OList = UsableObjects;

  while (OList != NULL) {
    if (!strcmp(OList->Name, ObjectTypeName)) {
      break;
    } else {
      OList = OList->Next;
    }
  }

  if (OList == NULL) {
    return 0;
  } else {
    return 1;
  }
}  

#endif  // __GLOBAL_H
