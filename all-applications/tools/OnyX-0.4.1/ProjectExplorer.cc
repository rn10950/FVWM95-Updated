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

extern int CurrentDevelopmentWindow;
extern int SelectedItemID;
extern void MoveBrackets(int X, int Y, int Width, int Height);
extern void PropertiesWindow_Update();


void UpdateExplorerTree(char *Entry)
{
        struct ProjectObjectList *Object;
        struct ProjectObjectPropertyList *Property;
                                 
        for(Object = ProjectObjects; Object != NULL; Object = Object->Next)
        {
                if(!strcmp("OnyxMenu",Object->Type))    /* Is this object a menu? */
                {
                        /* Scan through the properties. */
                        for(Property = Object->Property; Property != NULL; Property = Property->Next)   
                        {
                                if(!strcmp(Property->Name,"MenuBarParent"))     /* Locate menu bar parent */
                                {
                                        if(!strcmp(Property->Value,Entry))      /* If the parent is our Entry. */
                                        {
                                                ExplorerTree->AddItem(Object->Name);        /* Add it to the tree. */
                                                UpdateExplorerTree(Object->Name);           /* Add its children. */
						ExplorerTree->MoveUp();
                                                break;                  /* No need to search properties further. */
                                        }
                                }
                                else if(!strcmp(Property->Name,"MenuParent"))        /* Locate menu parent */
                                {
                                        if(!strcmp(Property->Value,Entry))      /* If the parent is our Entry. */
                                        {
                                                ExplorerTree->AddItem(Object->Name);        /* Add it to the tree. */
                                                UpdateExplorerTree(Object->Name);           /* Add its children. */
						ExplorerTree->MoveUp();
                                                break;                  /* No need to search properties further. */
                                        }
                                }
                        }
                }
                else if(!strcmp("OnyxMenuItem",Object->Type)) /* Is this object a menu item? */
                {
                        /* Scan through the properties. */
                        for(Property = Object->Property; Property != NULL; Property = Property->Next)
                        {
                                if(!strcmp(Property->Name,"MenuParent"))        /* Locate menu parent */
                                {
                                        if(!strcmp(Property->Value,Entry))      /* If the parent is our Entry. */
                                        {
                                                ExplorerTree->AddItem(Object->Name);        /* Add it to the tree. */  
						ExplorerTree->MoveUp();
                                                break;                  /* No need to search properties further. */
                                        }
                                }
                        }
                }
		else
		{
			if(!strcmp(Object->Parent,Entry))	/* If the parent of this object is the current object. */
			{
                        	ExplorerTree->AddItem(Object->Name);        /* Add it to the tree. */
                                UpdateExplorerTree(Object->Name);           /* Add its children. */
				ExplorerTree->MoveUp();
                         }
		}
        }
}





void ProjectExplorer_CloseSignaled()
{
	ProjectExplorerButton->Pop();
	ViewProjectMenuItem->UnCheck();
}

void ProjectExplorer_Closed()
{
}


void ProjectExplorer_Update()
{
	ViewProjectMenuItem->Check();

	ExplorerTree->SelectRoot();
	ExplorerTree->DeleteItem();
	ExplorerTree->AddItem("Application");
	ExplorerTree->SelectRoot();
	UpdateExplorerTree("Application");
	if(SelectedObject == NULL)
	{
		ExplorerTree->SelectItem("Application");
	}
	else
	{
		ExplorerTree->SelectItem(SelectedObject->Name);
	}
	ExplorerTree->Update();
}

void ProjectExplorer_Load()
{
	ProjectExplorer_Update();
}

void ExplorerTree_ButtonPressed(int Button, int x, int y)
{}

void ExplorerTree_ItemSelected()
{
	struct ProjectObjectList *ProjectObject;
	struct ProjectObjectList *ParentObject;
	int i;
	int OnScreen;

	if(ExplorerTree->SelectedItem != NULL && strcmp(ExplorerTree->SelectedItem,"Application"))
	{
		for(ProjectObject = ProjectObjects; ProjectObject != NULL; ProjectObject = ProjectObject->Next)
		{
			if(!strcmp(ExplorerTree->SelectedItem,ProjectObject->Name))
			{
				SelectedObject = ProjectObject;
				if(!strcmp(SelectedObject->Type,"OnyxWindow"))
				{
					SelectedWindow = SelectedObject;
					if(TLCorner != NULL)	/* Window selected. Hide brackets. */
					{
						MoveBrackets(-10,-10,-10,-10);
					}

					/* Locate window if visible to set SelectedItemID. Otherwise it remains -1 (not visible). */
					for(i = 0, SelectedItemID = -1; i < MaxDevelopmentWindows; i++)
					{
						/* If the window exists and the mapping matches the SelectedWindow */
						if(DevelopmentWindow[i] != NULL && WindowToListMap[i] == SelectedWindow)
						{
							/* Development window[i] is mapped to the current SelectedWindow, */
							/*   set the SelectedItemID to be it. */
							SelectedItemID = DevelopmentWindow[i]->ID;
							i = MaxDevelopmentWindows; /* Kick us out of the loop. */
						}
					}

				}
				else
				{
					/* Locate parent window. */
					for(ParentObject = ProjectObjects; (ParentObject != NULL) && strcmp(ParentObject->Name,ProjectObject->Parent); ParentObject = ParentObject->Next)
					{}
					SelectedWindow = ParentObject;	/* Set selected window. */

					OnScreen = False;
					for(i = 0; i < MaxDevelopmentObjects; i++)
					{
						if(DevelopmentItem[i] != NULL && !strcmp(DevelopmentItem[i]->Name,SelectedObject->Name))
						{
							OnScreen = True;
							SelectedItemID = DevelopmentObject[i]->ID;
							if(TLCorner != NULL)
							{
								MoveBrackets(DevelopmentObject[i]->X,DevelopmentObject[i]->Y,DevelopmentObject[i]->Width,DevelopmentObject[i]->Height);
							}
							break;
						}
					}
					if(!OnScreen)
					{
						if(TLCorner != NULL)	/* Window of object not visible. Hide brackets. */
						{
							MoveBrackets(-10,-10,-10,-10);
						}
						SelectedItemID = -1;
					}
				}
				break;
			}
		}
	}
	else
	{
		SelectedObject = NULL;
		SelectedWindow = NULL;
		if(TLCorner != NULL)	/* Application selected. Hide brackets. */
		{
			MoveBrackets(-10,-10,-10,-10);
		}
	}


	if(DevelopmentWindow[CurrentDevelopmentWindow] != NULL)
	{
		if(DevelopmentWindow[CurrentDevelopmentWindow]->IsVisible)
		{
			DevelopmentWindow[CurrentDevelopmentWindow]->Update();
		}
	}

	if(PropertiesWindow->IsVisible)
	{
		PropertiesWindow_Update();
	}

}

void ProjectExplorer_WindowResized(int OldWidth, int OldHeight, int NewWidth, int NewHeight)
{
	ExplorerTree->MoveResize(0, 0, NewWidth, NewHeight);
}
