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
#include <sys/stat.h>
#include <unistd.h>


extern int SetMainWindowButtonStatus();
extern int SetMainWindowMenuStatus();
extern int ValidProject;
extern void ResetProject();
extern void InitializeInternalProjectFile(char *ProjectFileName);
extern void InitializeInternalApplication();
extern void InitializeInternalMakefile();
extern void InitializeInternalBaseMakefile();
extern void InitializeInternalMain_h();
extern void InitializeInternalEvents_h();
extern void InitializeInternalObjects_h();
extern void InitializeInternalIncludes_h();
extern void InitializeInternalMain_cc();
extern void CloseAllActiveWindows();

void NewProjectWindow_CloseSignaled()
{
}

void NewProjectWindow_Closed()
{
}

void NewProjectWindow_Load()
{
}

void NewProjectOpenButton_ButtonClick()
{
	struct ObjectList *UsableObject;

	if (!strlen(NewProjectName->Text))
	{
		AsteriskBox(NewProjectWindow->Self(), "Please supply an application name.");
	}
	else
	{
		if (ValidProject)	/* If currently in an onyx created project
					   directory get out so the user isn't confused. */
		{
			chdir("..");
		}

		strcpy(ProjectDirectory, "./");
		sprintf(ProjectFile, "%s.oxp", NewProjectName->Text);
		strcpy(ProjectApplicationName, NewProjectName->Text);

		NewProjectWindow->Close();

		/* Clear out any old objects. */
		ResetProject();

		UsableObjects = GetNewObjectType("OnyxBase");
		UsableObject = UsableObjects;

		/* Add in the starting objects. Someday this will be user selectable. */
		UsableObject->Next = GetNewObjectType("OnyxButton");
		if (UsableObject->Next)
		{
			UsableObject = UsableObject->Next;
		}

		UsableObject->Next = GetNewObjectType("OnyxCheckBox");
		if (UsableObject->Next)
		{
			UsableObject = UsableObject->Next;
		}

		UsableObject->Next = GetNewObjectType("OnyxComboBox");
		if (UsableObject->Next)
		{
			UsableObject = UsableObject->Next;
		}

		UsableObject->Next = GetNewObjectType("OnyxFileDialog");
		if (UsableObject->Next)
		{
			UsableObject = UsableObject->Next;
		}

		UsableObject->Next = GetNewObjectType("OnyxIcon");
		if (UsableObject->Next)
		{
			UsableObject = UsableObject->Next;
		}

		UsableObject->Next = GetNewObjectType("OnyxLabel");
		if (UsableObject->Next)
		{
			UsableObject = UsableObject->Next;
		}

		UsableObject->Next = GetNewObjectType("OnyxListBox");
		if (UsableObject->Next)
		{
			UsableObject = UsableObject->Next;
		}

		UsableObject->Next = GetNewObjectType("OnyxMsgBox");
		if (UsableObject->Next)
		{
			UsableObject = UsableObject->Next;
		}

		UsableObject->Next = GetNewObjectType("OnyxRadioButton");
		if (UsableObject->Next)
		{
			UsableObject = UsableObject->Next;
		}

		UsableObject->Next = GetNewObjectType("OnyxTextBox");
		if (UsableObject->Next)
		{
			UsableObject = UsableObject->Next;
		}

// This is a trick: OnyxWindow objects are not added to the UsableObjects
// list until the first window is created. The NewWindowButton_ButtonClick()
// code uses this fact to detect when the very first window is created
// in order to set the default value of CloseExitsApplication to False.
// See NewWindowButton_ButtonClick() in MainWindow.cc for more details

//		UsableObject->Next = GetNewObjectType("OnyxWindow");
//		if (UsableObject->Next)
//		{
//			UsableObject = UsableObject->Next;
//		}

		/* Close windows that may be open. */
		CloseAllActiveWindows();

		/* Enable the menus and buttons. */
		ValidProject = True;
		SetMainWindowButtonStatus();
		SetMainWindowMenuStatus();

		NewWindowButton_ButtonClick();	/* Add new window. */

		InitializeInternalProjectFile(ProjectFile);
		InitializeInternalApplication();
		InitializeInternalMain_h();
		InitializeInternalEvents_h();
		InitializeInternalObjects_h();
		InitializeInternalIncludes_h();
		InitializeInternalMain_cc();
		InitializeInternalMakefile();
		InitializeInternalBaseMakefile();
	}
}

void NewProjectCancelButton_ButtonClick()
{
	NewProjectWindow->Close();
}

void NewProjectName_EnterPressed()
{
	NewProjectOpenButton_ButtonClick();
}
