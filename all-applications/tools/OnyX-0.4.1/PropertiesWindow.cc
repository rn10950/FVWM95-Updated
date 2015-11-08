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

#define MaxProperties 50
#define PropertyHeight 22
#define PropertyButtonSize 50
#define RightEdge 16

OnyxLabel *PropertyItem[MaxProperties];		/* This is an array for the left hand side of the window. */
OnyxComboBox *PropertyCombo[MaxProperties];	/* These are the combo boxes that might be used. */
OnyxButton *PropertyButton[MaxProperties];	/* These are the buttons that might be used. */

struct ProjectObjectList *SelectedObjectLocal = NULL;
int LocalTrustable = 0;

void PropertiesWindow_CloseSignaled()
{
	ShowPropertiesButton->Pop();
	ViewPropertiesMenuItem->UnCheck();
	LocalTrustable = 0;
}

void PropertiesWindow_Closed()
{
}

void PropertiesWindow_Update()
{
	int i, j;
	struct ProjectObjectPropertyList *ObjectProperty;
	struct ObjectList *UsableObject;
	struct ObjectProperty *UsableProperty;

	/* If this isn't just a property update but instead a new object has been selected then rebuild */
	/*   the entire window from scratch. */
	if(SelectedObjectLocal != SelectedObject || !LocalTrustable)
	{
		for(i = 0; i < MaxProperties; i++)	/* Cycle through the currently displayed Property */
		{					/* listings and remove them. */
			if(PropertyItem[i] != NULL)
			{
				delete PropertyItem[i];	/* Destroy object on screen and internally */
				delete PropertyValue[i];/* make it NULL. */

				/* Remove leftover combo boxes. */
				if(PropertyCombo[i] != NULL)
				{
					delete PropertyCombo[i];
					PropertyCombo[i] = NULL;
				}

				/* Remove left over buttons. */
				if(PropertyButton[i] != NULL)
				{
					delete PropertyButton[i];
					PropertyButton[i] = NULL;
				}
				PropertyItem[i] = NULL;
				PropertyValue[i] = NULL;
			}
		}
		i = 0;

		if(SelectedObject != NULL)	/* Make sure "Application isn't selected. */
		{
			ObjectProperty = SelectedObject->Property;	/* Start at first property. */
	
				/* Add pseudo property, object name */
			PropertyItem[i] = new OnyxLabel;	/* Add left hand description. */
			PropertyItem[i]->ID = -1;
			PropertyItem[i]->Parent = PropertiesWindow->Self();
			PropertyItem[i]->X = 0;
			PropertyItem[i]->Y = i * PropertyHeight;
			PropertyItem[i]->Width = PropertiesWindow->Width /2 - RightEdge;
			PropertyItem[i]->Height = PropertyHeight;
			strcpy(PropertyItem[i]->Text,"Object Name");
			PropertyItem[i]->Create();

			PropertyValue[i] = new OnyxTextBox;	/* Add right hand value. */
			PropertyValue[i]->ID = PropertyIDModifier + i;
			PropertyValue[i]->Parent = PropertiesWindow->Self();
			PropertyValue[i]->X = PropertyItem[i]->X + PropertyItem[i]->Width;
			PropertyValue[i]->Y = PropertyItem[i]->Y;
			PropertyValue[i]->Width = PropertiesWindow->Width - PropertyItem[i]->Width - RightEdge;
			PropertyValue[i]->Height = PropertyItem[i]->Height;
			PropertyValue[i]->Size = 255;
			PropertyValue[i]->Create();
			PropertyValue[i++]->AddText(SelectedObject->Name);

			/* Locate UsableObject to identify PropertySettings. */
			UsableObject = UsableObjects;
			while(strcmp(UsableObject->Name,SelectedObject->Type))
			{
				UsableObject = UsableObject->Next;
			}
		}
		else
		{
			ObjectProperty = NULL;	/* "Application" */

			PropertyItem[i] = new OnyxLabel;	/* Add left hand description. */
			PropertyItem[i]->ID = -1;
			PropertyItem[i]->Parent = PropertiesWindow->Self();
			PropertyItem[i]->X = 0;
			PropertyItem[i]->Y = i * PropertyHeight;
			PropertyItem[i]->Width = PropertiesWindow->Width /2;
			PropertyItem[i]->Height = PropertyHeight;
			strcpy(PropertyItem[i]->Text,"Application Name");
			PropertyItem[i]->Create();

			PropertyValue[i] = new OnyxTextBox;	/* Add right hand value. */
			PropertyValue[i]->ID = PropertyIDModifier + i;
			PropertyValue[i]->Parent = PropertiesWindow->Self();
			PropertyValue[i]->X = PropertyItem[i]->X + PropertyItem[i]->Width;
			PropertyValue[i]->Y = PropertyItem[i]->Y;
			PropertyValue[i]->Width = PropertiesWindow->Width - PropertyItem[i]->Width - RightEdge;
			PropertyValue[i]->Height = PropertyItem[i]->Height;
			PropertyValue[i]->Size = 255;
			PropertyValue[i]->Create();
			PropertyValue[i++]->AddText(ProjectApplicationName);
		}


		while(ObjectProperty != NULL)	/* Cycle through current properties and display the list. */
		{
			/* Avoid listing two properties from the onyxmenu series. */
			/* Hey, its my control I can cheat like this if I want to. ;) */
			if(!((!strcmp(SelectedObject->Type,"OnyxMenu") || !strcmp(SelectedObject->Type,"OnyxMenuItem")) &&
				(!strcmp(ObjectProperty->Name,"MenuParent") || !strcmp(ObjectProperty->Name,"MenuBarParent"))))
			{
				PropertyItem[i] = new OnyxLabel;	/* Add left hand description. */
				PropertyItem[i]->ID = -1;
				PropertyItem[i]->Parent = PropertiesWindow->Self();
				PropertyItem[i]->X = 0;
				PropertyItem[i]->Y = i * PropertyHeight;
				PropertyItem[i]->Width = PropertiesWindow->Width /2 - RightEdge;
				PropertyItem[i]->Height = PropertyHeight;
				strcpy(PropertyItem[i]->Text,ObjectProperty->Name);
				PropertyItem[i]->Create();

				PropertyValue[i] = new OnyxTextBox;	/* Add right hand value. */
				PropertyValue[i]->ID = PropertyIDModifier + i;
				PropertyValue[i]->Parent = PropertiesWindow->Self();
				PropertyValue[i]->X = PropertyItem[i]->X + PropertyItem[i]->Width;
				PropertyValue[i]->Y = PropertyItem[i]->Y;
				PropertyValue[i]->Width = PropertiesWindow->Width - PropertyItem[i]->Width - RightEdge;
				PropertyValue[i]->Height = PropertyItem[i]->Height;
				PropertyValue[i]->Size = 255;
				PropertyValue[i]->Create();
				PropertyValue[i]->AddText(ObjectProperty->Value);

				/* Locate PropertySetting info to see if we need anything else. */
				UsableProperty = UsableObject->Property;
				while(strcmp(UsableProperty->Name,ObjectProperty->Name))
				{
					UsableProperty = UsableProperty->Next;
				}

				if(UsableProperty->Type == ComboBox)
				{
					PropertyCombo[i] = new OnyxComboBox;
					PropertyCombo[i]->ID = PropertyIDModifier + i;
					PropertyCombo[i]->Parent = PropertiesWindow->Self();
					PropertyCombo[i]->X = PropertyValue[i]->X;
					PropertyCombo[i]->Y = PropertyValue[i]->Y;
					PropertyCombo[i]->Width = PropertyValue[i]->Width;
					PropertyCombo[i]->Height = PropertyValue[i]->Height;
					if(UsableProperty->Access == ReadOnly)
					{
						PropertyCombo[i]->ReadOnly = 1;
					}
					else
					{
						PropertyCombo[i]->ReadOnly = 0;
					}
					PropertyCombo[i]->Create();
					for(j = 0; j < UsableProperty->Entries; j++)
					{
						PropertyCombo[i]->AddEntry(UsableProperty->Entry[j]);
					}
					PropertyCombo[i]->AddText(ObjectProperty->Value);
				}
				else if(UsableProperty->Type == FileBrowser)
				{
					PropertyButton[i] = new OnyxButton;
					PropertyButton[i]->ID = PropertyButtonIDModifier + i;
					PropertyButton[i]->Parent = PropertiesWindow->Self();
					PropertyButton[i]->X = PropertyValue[i]->X + PropertyValue[i]->Width - PropertyButtonSize;
					PropertyButton[i]->Y = PropertyValue[i]->Y;
					PropertyButton[i]->Width = PropertyButtonSize;
					PropertyButton[i]->Height = PropertyValue[i]->Height;
					sprintf(PropertyButton[i]->Text,"Browse");
					PropertyButton[i]->Create();
				}
				else if(UsableProperty->Type == ColorBrowser)
				{
					PropertyButton[i] = new OnyxButton;
					PropertyButton[i]->ID = PropertyButtonIDModifier + i;
					PropertyButton[i]->Parent = PropertiesWindow->Self();
					PropertyButton[i]->X = PropertyValue[i]->X + PropertyValue[i]->Width - PropertyButtonSize;
					PropertyButton[i]->Y = PropertyValue[i]->Y;
					PropertyButton[i]->Width = PropertyButtonSize;
					PropertyButton[i]->Height = PropertyValue[i]->Height;
					sprintf(PropertyButton[i]->Text,"Select");
					PropertyButton[i]->Create();
				}
				else if(UsableProperty->Type == FontBrowser)
				{
					PropertyButton[i] = new OnyxButton;
					PropertyButton[i]->ID = PropertyButtonIDModifier + i;
					PropertyButton[i]->Parent = PropertiesWindow->Self();
					PropertyButton[i]->X = PropertyValue[i]->X + PropertyValue[i]->Width - PropertyButtonSize;
					PropertyButton[i]->Y = PropertyValue[i]->Y;
					PropertyButton[i]->Width = PropertyButtonSize;
					PropertyButton[i]->Height = PropertyValue[i]->Height;
					sprintf(PropertyButton[i]->Text,"Select");
					PropertyButton[i]->Create();
				}
				i++;
			}

	
			ObjectProperty = ObjectProperty->Next;	/* Go to next property and display it. */
		}

		PropertiesWindow->Update();
	}
	else /* Save some redraw calls by only updating the text boxes representing changed values. */
	{
		if(SelectedObject != NULL)	/* Make sure "Application isn't selected. */
		{
			i = 1;
			ObjectProperty = SelectedObject->Property;	/* Start at first property. */

			while(ObjectProperty != NULL)	/* Cycle through current properties and update the changes. */
			{
				/* Weed out those unshowable properties. *grin* */
				if(!((!strcmp(SelectedObject->Type,"OnyxMenu") || !strcmp(SelectedObject->Type,"OnyxMenuItem")) &&
					(!strcmp(ObjectProperty->Name,"MenuParent") || !strcmp(ObjectProperty->Name,"MenuBarParent"))))
				{

					/* If these dont match then update that text box. */
					if(strcmp(PropertyValue[i]->Text,ObjectProperty->Value))
					{
						PropertyValue[i]->ClearText();
						PropertyValue[i]->AddText(ObjectProperty->Value);
						PropertyValue[i]->Update();

						/* Update the combo box too if it exists. */
						if(PropertyCombo[i] != NULL)
						{
							PropertyCombo[i]->ClearText();
							PropertyCombo[i]->AddText(ObjectProperty->Value);
							PropertyCombo[i]->Update();
						}
					}

					i++;
				}
				ObjectProperty = ObjectProperty->Next;	/* Go to next property and check it. */
			}
		}
	}

	SelectedObjectLocal = SelectedObject;
	LocalTrustable = 1;
}

void PropertiesWindow_Load()
{
	ViewPropertiesMenuItem->Check();
	PropertiesWindow_Update();
}


void PropertiesWindow_WindowResized(int OldWidth, int OldHeight, int NewWidth, int NewHeight)
{
	int i;

	for(i = 0; i < MaxProperties; i++)	/* Cycle through the currently displayed Property */
	{					/* listings and resize them. */
		if(PropertyItem[i] != NULL)
		{
			PropertyItem[i]->MoveResize(0, i * PropertyHeight, PropertiesWindow->Width /2, PropertyHeight);
			PropertyValue[i]->MoveResize(PropertyItem[i]->X + PropertyItem[i]->Width, PropertyItem[i]->Y,PropertiesWindow->Width - PropertyItem[i]->Width - RightEdge, PropertyItem[i]->Height);

			/* Move combo box if its there. */
			if(PropertyCombo[i] != NULL)
			{
				PropertyCombo[i]->MoveResize(PropertyValue[i]->X, PropertyValue[i]->Y,PropertyValue[i]->Width, PropertyValue[i]->Height);
			}

			/* Move browse button if its there. */
			if(PropertyButton[i] != NULL)
			{
				PropertyButton[i]->MoveResize(PropertyValue[i]->X + PropertyValue[i]->Width - PropertyButtonSize,
								PropertyValue[i]->Y,
								PropertyButtonSize,
								PropertyValue[i]->Height);
			}
		}
	}
}
