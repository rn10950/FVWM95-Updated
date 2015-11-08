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

extern struct ProjectObjectList *AddDevelopmentObject(struct ObjectList *UsableObject, char *Window, long x, long y, char *ParameterOverride, char *POValue);
extern void RenameDevelopmentObject(struct ProjectObjectList *Object, char*NewName);
extern void DeleteDevelopmentObject(struct ProjectObjectList *Object);

char NewMenuName[255];
char NewMenuText[255];

int AddingEntryOnly;

extern struct ProjectObjectList *BaseWindow;
extern struct ProjectObjectList *BaseMenuBar;
extern struct ObjectList *MenuObject;
extern struct ObjectList *MenuItemObject;

extern void ProjectExplorer_Update();

void MenuEditor2_CloseSignaled()
{
	NewMenuName[0] = '\0';
	NewMenuText[0] = '\0';
}

void MenuEditor2_Closed()
{
}


void MenuEditor2_Load()
{
	/* Center window. */
	MenuEditor2->Move((ScreenInfo->Width - MenuEditor2->Width)/2,(ScreenInfo->Height - MenuEditor2->Height)/2);
	NewMenuName[0] = '\0';
	NewMenuText[0] = '\0';
	MenuEditor2TextBox->ClearText();
	if(AddingEntryOnly)
	{
		sprintf(MenuEditor2Label->Text,"Name of new menu item?");
	}
	else
	{
		sprintf(MenuEditor2Label->Text,"Name of new menu?");
	}

	MenuEditor2Label->Update();
}

void MenuEditor2_WindowResized(int OldWidth, int OldHeight, int NewWidth, int NewHeight)
{
}

void MenuEditor2Ok_ButtonClick()
{
	ProjectObjectPropertyList *Property;
	struct ProjectObjectList *NewObject;
	struct ProjectObjectList *SavedSelected;


	SavedSelected = SelectedObject;	/* Save which object is currently selected. */

	if(!strlen(NewMenuName))
	{
		if(!strlen(MenuEditor2TextBox->Text))
		{
			if(AddingEntryOnly)
			{
				ExclamationBox(MenuEditor2->Self(),"Please supply a name for the menu item.");
			}
			else
			{
				ExclamationBox(MenuEditor2->Self(),"Please supply a name for the menu.");
			}
		}
		else
		{
			strcpy(NewMenuName,MenuEditor2TextBox->Text);
			MenuEditor2TextBox->ClearText();
			sprintf(MenuEditor2Label->Text,"Text to display for %s?",NewMenuName);
			MenuEditor2Label->Update();
		}
	}
	else
	{
		strcpy(NewMenuText,MenuEditor2TextBox->Text);
		if(AddingEntryOnly)
		{
		        if(!strcmp(MenuTree->SelectedItem,BaseMenuBar->Name))
		        {
		                NewObject = AddDevelopmentObject(MenuItemObject,BaseWindow->Name,-100,-100,"MenuBarParent",BaseMenuBar->Name);
		        }
		        else
		        {
		                NewObject = AddDevelopmentObject(MenuItemObject,BaseWindow->Name,-100,-100,"MenuParent",MenuTree->SelectedItem);
		        }
		}
		else
		{
		        if(!strcmp(MenuTree->SelectedItem,BaseMenuBar->Name))
		        {
		                NewObject = AddDevelopmentObject(MenuObject,BaseWindow->Name,-100,-100,"MenuBarParent",BaseMenuBar->Name);
		        }
		        else
		        {
		                NewObject = AddDevelopmentObject(MenuObject,BaseWindow->Name,-100,-100,"MenuParent",MenuTree->SelectedItem);
		        }
		}

		for(Property = NewObject->Property; strcmp(Property->Name,"Text"); Property = Property->Next)
		{}
		sprintf(Property->Value,NewMenuText);
		RenameDevelopmentObject(NewObject,NewMenuName);
		SelectedObject = NewObject;
		MenuTree->AddItem(NewMenuName);
		MenuTree->MoveUp();
	        MenuTree->Update();                     /* Update the screen. */
		MenuEditor2->Close();

		SelectedObject = SavedSelected;	/* Undo move of selected object to allow multiple menu adds to the same object. */
		if(ProjectExplorer->IsVisible)
		{
//printf("SelectedObject Name %s\n",SelectedObject->Name); fflush(stdout);
//			ProjectExplorer->Show();	/* Update explorer. */
			ProjectExplorer_Update();
		}
	}
}

void MenuEditor2Cancel_ButtonClick()
{
	MenuEditor2->Close();
}

void MenuEditor2TextBox_EnterPressed()
{
	MenuEditor2Ok_ButtonClick();
}
