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

struct ProjectObjectList *BaseWindow;
struct ProjectObjectList *BaseMenuBar;

struct ObjectList *MenuBarObject;
struct ObjectList *MenuObject;
struct ObjectList *MenuItemObject;

extern int AddingEntryOnly;

extern void DeleteDevelopmentObject(struct ProjectObjectList *Object);

extern void ProjectExplorer_Update();
extern void PropertiesWindow_Update();

void UpdateMenuTree(char *Entry)
{
	struct ProjectObjectList *Object;
	struct ProjectObjectPropertyList *Property;

	for(Object = ProjectObjects; Object != NULL; Object = Object->Next)
	{
		if(!strcmp("OnyxMenu",Object->Type))	/* Is this object a menu? */
		{
			/* Scan through the properties. */
			for(Property = Object->Property; Property != NULL; Property = Property->Next)
			{
				if(!strcmp(Property->Name,"MenuBarParent"))	/* Locate menu bar parent */
				{
					if(!strcmp(Property->Value,Entry))	/* If the parent is our Entry. */
					{
						MenuTree->AddItem(Object->Name);	/* Add it to the tree. */
						MenuTree->SelectItem(Object->Name);	/* Select it. */
						UpdateMenuTree(Object->Name);		/* Add its children. */
						MenuTree->MoveUp();	/* Move back to wehre we were before UpdateMenuTree() */
						break;			/* No need to search properties further. */
					}
				}
				if(!strcmp(Property->Name,"MenuParent"))	/* Locate menu parent */
				{
					if(!strcmp(Property->Value,Entry))	/* If the parent is our Entry. */
					{
						MenuTree->AddItem(Object->Name);	/* Add it to the tree. */
						MenuTree->SelectItem(Object->Name);	/* Select it. */
						UpdateMenuTree(Object->Name);		/* Add its children. */
						MenuTree->MoveUp();	/* Move back to wehre we were before UpdateMenuTree() */
						break;			/* No need to search properties further. */
					}
				}
			}
		}
		else if(!strcmp("OnyxMenuItem",Object->Type)) /* Is this object a menu item? */
		{
			/* Scan through the properties. */
			for(Property = Object->Property; Property != NULL; Property = Property->Next)
			{
				if(!strcmp(Property->Name,"MenuParent"))	/* Locate menu parent */
				{
					if(!strcmp(Property->Value,Entry))	/* If the parent is our Entry. */
					{
						MenuTree->AddItem(Object->Name);	/* Add it to the tree. */
						MenuTree->MoveUp();
						break;			/* No need to search properties further. */
					}
				}
			}
		}
	}
}

void MenuEditor_CloseSignaled()
{
}

void MenuEditor_Closed()
{
}


void MenuEditor_Load()
{
	BaseWindow = SelectedWindow;
	BaseMenuBar = SelectedObject;

	for(MenuBarObject = UsableObjects; strcmp(MenuBarObject->Name,"OnyxMenuBar"); MenuBarObject = MenuBarObject->Next)
	{}

	for(MenuObject = UsableObjects; strcmp(MenuObject->Name,"OnyxMenu"); MenuObject = MenuObject->Next)
	{}

	for(MenuItemObject = UsableObjects; strcmp(MenuItemObject->Name,"OnyxMenuItem"); MenuItemObject = MenuItemObject->Next)
	{}

	MenuTree->SelectRoot();			/* Find the old root. */
	MenuTree->DeleteItem();			/* Delete it. */
	MenuTree->AddItem(BaseMenuBar->Name);	/* Add the new root. */
	MenuTree->SelectRoot();			/* Select it. */
	UpdateMenuTree(BaseMenuBar->Name);	/* Put it all the children. */
	MenuTree->SelectRoot();			/* Reselect root since update changed the selectection. */
	MenuTree->OpenItem();			/* Open the root item. */
	MenuTree->HighlightItem();		/* Highlight it. */
	MenuTree->Update();			/* Update the screen. */

	MenuItemAdd->Disable();			/* Start items disabled since menu bar is selected. */
	MenuItemDelete->Disable();
	MenuAdd->Enable();			/* New menus can be added. */
	MenuDelete->Disable();			/* Cant delete the bar from here. */
}

void MenuEditor_WindowResized(int OldWidth, int OldHeight, int NewWidth, int NewHeight)
{
	MenuTree->Resize(NewWidth - MenuTree->X - 5, NewHeight - MenuTree->Y - 5);
}

void MenuTree_ButtonPressed(int Button, int x, int y)
{
	struct ProjectObjectList *ProjectObject;

	/* If nothing was selected. */
	if(MenuTree->SelectedItem == NULL)
	{
		MenuAdd->Disable();
		MenuDelete->Disable();
		MenuItemAdd->Disable();
		MenuItemDelete->Disable();
	}
	else
	{
		for(ProjectObject = ProjectObjects; strcmp(ProjectObject->Name,MenuTree->SelectedItem); ProjectObject = ProjectObject->Next)
		{}

		if(!strcmp(ProjectObject->Type,"OnyxMenuItem"))	/* MenuItem selected. Only option is to remove it. */
		{
			MenuAdd->Disable();
			MenuDelete->Disable();
			MenuItemAdd->Disable();
			MenuItemDelete->Enable();
		}
		else if(!strcmp(ProjectObject->Type,"OnyxMenu"))
		{
			MenuAdd->Enable();	/* Onyx menu selected. We can do everything except delete item. */
			MenuDelete->Enable();
			MenuItemAdd->Enable();
			MenuItemDelete->Disable();
		}
		else
		{				/* Onyx Menu Bar selected. */
			MenuAdd->Enable();			/* New menus can be added. */
			MenuDelete->Disable();			/* Cant delete the bar from here. */
			MenuItemAdd->Disable();			/* Start items disabled since menu bar is selected. */
			MenuItemDelete->Disable();
			SelectedObject = ProjectObject;
		}

		SelectedObject = ProjectObject;

		if(ProjectExplorer->IsVisible)
		{
//			ProjectExplorer->Show();
			ProjectExplorer_Update();
		}

		if(PropertiesWindow->IsVisible)
		{
//			PropertiesWindow->Show();
			PropertiesWindow_Update();
		}
	}
}

void MenuAdd_ButtonClick()
{
	AddingEntryOnly = False;
	MenuEditor2->Show();
}

void MenuItemAdd_ButtonClick()
{
	AddingEntryOnly = True;
	MenuEditor2->Show();
}


void MenuDelete_ButtonClick()
{
	struct ProjectObjectList *ProjectObject;
	
	/* Find the object. */
	for(ProjectObject = ProjectObjects; ProjectObject != NULL && strcmp(ProjectObject->Name,MenuTree->SelectedItem); ProjectObject = ProjectObject->Next);
	{}

	/* If this is NULL we have an inconsistency. Skip delete and go straight ot the reload of the menu editor window. */
	if(ProjectObject != NULL)
	{
		/* Delete the object and its children. */
		DeleteDevelopmentObject(ProjectObject);
	}

	MenuEditor->Show();

	/* Update the explorer, properties window, and the main window */
	if(ProjectExplorer->IsVisible)
	{
//		ProjectExplorer->Show();
		ProjectExplorer_Update();
	}	

	if(PropertiesWindow->IsVisible)
	{
//		PropertiesWindow->Show();
		PropertiesWindow_Update();
	}	
}

void MenuItemDelete_ButtonClick()
{
	/* For now these are the same. */
	MenuDelete_ButtonClick();
}

