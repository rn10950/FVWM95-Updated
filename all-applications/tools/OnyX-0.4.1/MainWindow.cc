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

/* Old notes to myself. Please ignore. Good chance they are out of date anyway. */
/* Please dont report these as bugs since I already know about them. */
/* ----------------------------- */
/* Remove absolute path from brcorner picture or it will not work */
/* Possible crash of any object of unaccessible type (no .oob) is used. strcmp crash in InitInternEvents and such. */
/* Delete needs to refresh window. */
/* Nothing to make fails again. */

#include "Objects.h"
#include "Events.h"
#include "GlobalExtern.h"

#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <signal.h>

#define TLCornerID 26
#define TRCornerID 27
#define BLCornerID 28
#define BRCornerID 29

char *EscapeQuotes(char *string);

void CreateDevelopmentWindow(int WindowNum, struct ProjectObjectList *ProjectObject);
void CreateDevelopmentItems();
OnyxObject *CreateDevelopmentObject(char name);
void CreateDevelopmentItem(struct ProjectObjectList *ProjectObject);
void CreateBrackets();
void MoveBrackets(int X, int Y, int Width, int Height);
void DestroyBrackets();

void DestroyDevelopmentObjects();
void PrepareForMake(char *WindowName);

void CloseAllActiveWindows();

int ValidProject;
void SetMainWindowButtonStatus();
void SetMainWindowMenuStatus();

int CurrentDevelopmentWindow;
ObjectList *GetNewObjectType(char *ObjectTypeName);
int ControlsLocked;

int SnapToGridIsOn;
int SnapToGridIsDefault;
int GridSize;

extern struct ProjectObjectList *AddDevelopmentObject(struct ObjectList *UsableObject, char *Window, long x, long y, char *ParameterOverride, char *POValue);
extern void OpenProject();
extern void ResetProject();

extern int CustomControlsNeedToBeCompiled;

extern char MakeType[20];

extern InternalCodeFile *InternalOxcCode;
extern InternalCodeFile *InternalOxpCode;
extern InternalCodeFile *InternalMakefile;
extern InternalCodeFile *InternalBaseMakefile;
extern InternalCodeFile *InternalHeader;
extern InternalCodeFile *InternalMaincc;

void InitializeInternalEvents_h();
void InitializeInternalObjects_h();
void InitializeInternalIncludes_h();
void InitializeInternalMain_h();
void InitializeInternalMakefile();
void InitializeInternalBaseMakefile();
void InitializeInternalProjectFile(char *ProjectFile);
void InitializeInternalMain_cc();

extern int SelectedItemID;

extern void ProjectExplorer_Update();
extern void PropertiesWindow_Update();

extern void DeleteDevelopmentObject(struct ProjectObjectList *Object);


/* This doesn't really go here but its the only event in the warning window. */
void XClassWarningOK_ButtonClick()
{
	XClassWarning->Close();
}

void MainWindow_CloseSignaled()
{
	ProjectObjectList *ProjectObject;
	InternalCodeFile *InternalFile;

	InternalFile = InternalOxcCode;
	while (InternalFile != NULL)
	{
		if (InternalFile->HasBeenModified && !InternalFile->HasBeenDeleted)
		{
			sprintf(ExitSaveMessageBox->Message, "%s has changed. Save before exiting?", InternalFile->ObjectName);
			ExitSaveMessageBox->Show();
			if (ExitSaveMessageBox->Response == YesButton)
			{
				ProjectObject = ProjectObjects; 
				while (ProjectObject != NULL && strcmp(ProjectObject->Name, InternalFile->ObjectName))
				{
					ProjectObject = ProjectObject->Next;
				}
				SelectedObject = ProjectObject;
				SaveButton_ButtonClick();
			}
			else if (ExitSaveMessageBox->Response == CancelButton)
			{
				MainWindow->AbortClose = 1;
				break;
			}
		}
		InternalFile = InternalFile->Next;
	}
}

void MainWindow_Load()
{
        /* Set the check box properly in the options window. */
        if (SnapToGridIsDefault)
        {
                SnapToGridDefault->Check();
                SnapToGridMenuItem->Check();
                SnapToGrid->Push();     /* Put it on by default then. */
        }
        else
        {   
                SnapToGridDefault->UnCheck();
                SnapToGridMenuItem->UnCheck();
                SnapToGrid->Pop();
        }
}

void MainWindow_Closed()
{
	ApplicationClose();	/* Close the application when the main window closes. */
}

void MainWindow_WindowResized(int OldWidth, int OldHeight, int NewWidth, int NewHeight)
{
	ToolbarSeparator->MoveResize(0, MainMenuBar->Height, NewWidth, 2);
}

void MakeButton_ButtonClick()
{
	struct ObjectList *UsableObject;
	char temp[255];
	struct stat buf;
	int Abort;
	int i;

	Abort = 0;
	if (stat("Makefile", &buf) == -1)	/* If there is no make file */
	{
		MakeMessageBox->Show();	/* Show message to prompt for save. */
		if (MakeMessageBox->Response == YesButton)	/* If they said yes to the message... */
		{
			SaveProjectMenuItem_MenuItemSelected();	/* Select the save project menu item. :) */
			if (stat("Makefile", &buf) == -1)	/* Make sure they didn't back out by looking for the file again. */
			{
				Abort = 1;	/* Don't make. */
			}
		}
		else /* If they don't want to save... */
		{
			AsteriskBox(MainWindow->Self(), "Stop all engines.", "Compilation has been aborted.");
			Abort = 1;	/* Don't compile. */
		}
	}

	if (!Abort)	/* If it's is still a go, then let's start up the works. */
	{
		/* Pull in all of the OnyX wrappers into .onyx */

		if (stat(".onyx", &buf) == -1)	/* Make sure the directory exists. */
		{
			mkdir(".onyx", S_IRWXU);	
			chmod(".onyx", S_IRWXU); /* Yes, I am purposefully overriding the umask here. Maybe I will change this. */
		}

		/* Make sure all of the code for the objects is there. */
		for (UsableObject = UsableObjects; UsableObject != NULL; UsableObject = UsableObject->Next)
		{
	
			/* Verify that the .cc file is there. */
			sprintf(temp, ".onyx/%s.cc", UsableObject->Name);
			i = stat(temp, &buf);

			if (i == 0)	/* If it was then verify the .h file. */
			{
				sprintf(temp, ".onyx/%s.h", UsableObject->Name);
				i = stat(temp, &buf);
			}

			if (i != 0)	/* If either was missing then grab them both again. */
			{
				/* Copy over the c++ file. */
				sprintf(temp, "cp %s/ObjectRepository/%s.cc .onyx", OnyxLocation, UsableObject->Name);
				system(temp);

				/* Copy over the header file. */
				sprintf(temp, "cp %s/ObjectRepository/%s.h .onyx", OnyxLocation, UsableObject->Name);
				system(temp);

				/* Announce that we need to recompile these guys. */
				CustomControlsNeedToBeCompiled = 1; 
			}
		}

		/* Write base make file. */
		InternalBaseMakefile->Save();

		InitializeInternalMakefile();
		InternalMakefile->Save();

		/* Make sure the .cc files are up to date */
		if(CustomControlsNeedToBeCompiled)
		{
			system("touch main.cc");	/* Will force a relink of pre compiled code. */
			strcpy(MakeType, "all");
		}
		else
		{
			strcpy(MakeType, "");
		}

		MakeWindow->Show();	/* Open the MakeWindow. */
	}
}


void MakeRunButton_ButtonClick()
{
	extern int MakeRunActive;

	MakeRunActive = True;
	MakeButton_ButtonClick();
}

void NewWindowButton_ButtonClick()
{
	struct ObjectList *WindowObject;
	struct ProjectObjectList *NewWindow;

	/* Make sure we know what one is. If its a new project then we don't. */
        /* (See the comments in NewProjectOpenButton_ButtonClick() */
	if (!ObjectTypeKnown("OnyxWindow"))
	{
		if (UsableObjects == NULL)
		{
			UsableObjects = GetNewObjectType("OnyxWindow");
			WindowObject = UsableObjects;
		}
		else
		{
			/* Locate end of usable object types. */
			for (WindowObject = UsableObjects; WindowObject->Next != NULL; WindowObject = WindowObject->Next)
			{}
			WindowObject->Next = GetNewObjectType("OnyxWindow");
			WindowObject = WindowObject->Next;
		}
		/* Set first window to close the application when it is closed. */
		NewWindow = AddDevelopmentObject(WindowObject, "Application", 80, 160, "CloseExitsApplication", "True");
	}
	else
	{
		/* Otherwise locate OnyxWindow object in usable object types. */
		for (WindowObject = UsableObjects; strcmp(WindowObject->Name, "OnyxWindow"); WindowObject = WindowObject->Next)
		{}
		NewWindow = AddDevelopmentObject(WindowObject, "Application", 80, 160, "", "");
	}
 
	/* Update seleted items. */
	SelectedWindow = NewWindow;
	SelectedObject = NewWindow;

	if (ProjectExplorer->IsVisible)
	{
		ProjectExplorer_Update();
	}

	/* Make the new development window visible. */
	ViewWindowButton_ButtonClick();
}

void ViewWindowButton_ButtonClick()
{
	int i;

	CurrentDevelopmentWindow = 0;

	CreateDevelopmentWindow(CurrentDevelopmentWindow, SelectedWindow);

	if (SelectedWindow != NULL)
	{
		CreateDevelopmentItems();

		/* Create Brackets. */
		CreateBrackets();

		DevelopmentWindow[CurrentDevelopmentWindow]->Show();

                for (i = 0; i < MaxDevelopmentObjects; i++)
                {
                	if (DevelopmentItem[i] != NULL && SelectedObject != NULL && !strcmp(DevelopmentItem[i]->Name, SelectedObject->Name))
                        {
                                SelectedItemID = DevelopmentObject[i]->ID;
				MoveBrackets(DevelopmentObject[i]->X, DevelopmentObject[i]->Y,
                                             DevelopmentObject[i]->Width, DevelopmentObject[i]->Height);
			}
		}
	}
}

void NextWindowButton_ButtonClick()
{
	struct ProjectObjectList *Tmp;

	if (SelectedWindow == NULL)	/* Application is selected, then start at begining of list. */
	{
		Tmp = ProjectObjects;
	}
	else				/* Otherwise start at the location of the currently selected window. */
	{
		Tmp = SelectedWindow->Next;
	}

	while (Tmp != NULL && strcmp(Tmp->Type, "OnyxWindow"))	/* Locate next window. */
	{
		Tmp = Tmp->Next;
	}

	/* Set selected window to next window. Tmp == NULL will signal that application is selected. */
	SelectedWindow = Tmp;
	SelectedObject = SelectedWindow;

	ViewWindowButton_ButtonClick();	/* Show the window. */

	if (ProjectExplorer->IsVisible)
	{
		ProjectExplorer_Update();
	}

	if (PropertiesWindow->IsVisible)
	{
		PropertiesWindow_Update();
	}
}

void OpenButton_ButtonClick()
{
	char OldDir[255];

	OpenFileDialog->ClearFileTypes(); /* Initialize since New Project also uses this. */
	OpenFileDialog->AddFileType("Onyx Project", "*.oxp");

	getcwd(OldDir, 255);	/* Save current position. */

	if (ValidProject)	/* If currently in an onyx created project directory get out so the user isn't confused. */
	{
		chdir("..");
	}

	OpenFileDialog->Show();		/* Open a dialog to retrieve project file name. */
	if (!OpenFileDialog->Cancelled)	/* If dialog not cancelled */
	{
		strcpy(ProjectDirectory, OpenFileDialog->DirectorySelected);	/* Store ProjectDirectory. */
		strcpy(ProjectFile, OpenFileDialog->FileSelected);		/* Store ProjectFile. */

		/* Close all windows that are open. */
		CloseAllActiveWindows();
		OpenProject();		/* Open the project! */
	}
	else
	{
		chdir(OldDir);	/* Go back to where we came from. */
	}

}

void ProjectExplorerButton_ButtonClick()
{
	if (ProjectExplorer->IsVisible)
	{
		ProjectExplorer->Close();	/* Close the Project Explorer button. */
	}
	else
	{
		ProjectExplorer->Show();	/* Show the Project Explorer button. */
	}
}

void RunButton_ButtonClick()
{
	char temp[255];

	RunPID = fork();	/* For a new process. */
	if (RunPID == 0)	/* Have new process run the program. */
	{
		sprintf(temp, "./%s", ProjectApplicationName);
		execlp(temp, temp, NULL);
		/* Code message box in case of failure. */
		exit(0);
	}
}

void SaveButton_ButtonClick()
{
	char OldDir[255];
	InternalCodeFile *InternalFile;

	char temp[255];
	struct stat junk;
	int LocationSet = 0;
	char *Marker;


	InternalFile = InternalOxcCode;

	/* If we are dealing with the application... */
	if (SelectedObject == NULL)
	{
		/* Locate "Application" file from internal code. */
		for (InternalFile = InternalOxcCode; InternalFile != NULL && (InternalFile->HasBeenDeleted || strcmp(InternalFile->ObjectName, "Application")); InternalFile = InternalFile->Next)
		{}
	}
	else
	{
		/* Locate file from internal code. */
		for (InternalFile = InternalOxcCode; InternalFile != NULL && (InternalFile->HasBeenDeleted || strcmp(InternalFile->ObjectName, SelectedObject->Name)); InternalFile = InternalFile->Next)
		{}
	}

	/* If it was found then save it. */
	if (InternalFile != NULL)
	{

		/* Try to locate the project to see if it has ever been saved. It should */
		/* be in the parent directory. (Symbolic links could be a problem.) */
		sprintf(temp, "../%s", ProjectFile);

		if (stat(temp, &junk))	/* If the project file is not in the parent directory... */
		{
			sprintf(temp, "%s/%s", ProjectDirectory, ProjectFile);	/* Chech the absolute path. Hopefully this will */
			if (stat(temp, &junk))					/* avoid a symlink problem and not introduce new mistakes. */
			{
				getcwd(OldDir, 255);	/* Save current position. */

				/* Initialize this to be different than the usual open project dialog. */
				OpenFileDialog->ClearFileTypes();
				sprintf(temp, "%s.oxp", NewProjectName->Text);
				OpenFileDialog->AddFileType("Give New Project Location", temp);
				OpenFileDialog->Show();
	
				if (!OpenFileDialog->Cancelled)
				{
					strcpy(ProjectDirectory, OpenFileDialog->DirectorySelected);
					strcpy(ProjectFile, OpenFileDialog->FileSelected);
					sprintf(temp, ".%s", ProjectFile);

					if (strlen(temp) >= 4)
					{
						Marker = temp + strlen(temp) - 4;	/* Change to .oxd */
						if (!strcmp(Marker, ".oxp"))
						{
							Marker = Marker + 3;
							*Marker = 'd';
						}
					}
					else
					{
						strcat(temp, ".oxd");
					}

					mkdir(temp, S_IRWXU);
					chmod(temp, S_IRWXU);
					chdir(temp);
					LocationSet = 1;	/* Saving is allowed. We are in the right place. */
				}
				else
				{
					chdir(OldDir);
				}
			}
			else
			{
				LocationSet = 1;	/* Saving is allowed. We are in the right place. */
			}
			
		}
		else
		{
			LocationSet = 1;	/* Saving is allowed. We are in the right place. */
		}
	
		if (LocationSet)
		{
			InternalFile->Save();
			InternalFile->IsIncorporated = True;

			/* If this is not the application. */
			if (SelectedObject != NULL)
			{
				SelectedObject->IsIncorporated = True;

				/* Locate the associated window and make the c++ file. */
				if (!strcmp(SelectedObject->Type, "OnyxWindow"))
				{
					PrepareForMake(SelectedObject->Name);
				}
				else
				{
					PrepareForMake(SelectedObject->Parent);
				}
			}
			else
			{
				/* Prepare the c++ file. */
				PrepareForMake("Application");
			}

			if (InternalOxpCode->HasBeenModified)
			{
				InternalOxpCode->Save();
			}


			InitializeInternalProjectFile(ProjectFile);	/* Sync (Here until I create the insert/replace code in internalfile::) */
			InternalOxpCode->Save();			/* Write new project file. */
			InternalMakefile->Save();			/* Write new Makefile. */

									/* Save all the headers files. */
			InitializeInternalEvents_h();
			InitializeInternalObjects_h();
			InitializeInternalIncludes_h();
			InitializeInternalMain_h();
			for (InternalFile = InternalHeader; InternalFile != NULL; InternalFile = InternalFile->Next)
			{
				InternalFile->Save();
			}

			InitializeInternalMain_cc();				/* Create and save main.cc */
			InternalMaincc->Save();

			/* Pull in the OnyX wrapper into .onyx */

			mkdir(".onyx", S_IRWXU);
			chmod(".onyx", S_IRWXU); /* Yes, I am purposefully overriding the umask here. Maybe I will change this. */

			InternalBaseMakefile->Save();				/* Write new Base Makefile. */

		}
	}
}

void SaveProjectButton_ButtonClick()
{
	char OldDir[255];
	struct ProjectObjectList *ProjectObject;
	InternalCodeFile *InternalFile;
	char *Marker;

	char temp[255];
	struct stat junk;
	int LocationSet = 0;

	/* Try to locate the project to see if it has ever been saved. It should */
	/* be in the parent directory. (Symbolic links could be a problem.) */
	sprintf(temp, "../%s", ProjectFile);

	if (stat(temp, &junk))	/* If the project file is not in the parent directory... */
	{
		sprintf(temp, "%s/%s", ProjectDirectory, ProjectFile);	/* Chech the absolute path. Hopefully this will */
		if (stat(temp, &junk))				/* avoid a symlink problem and not introduce new mistakes. */
		{
			getcwd(OldDir, 255);	/* Save currentposition. */

			/* Initialize this to be different than the usual open project dialog. */
			OpenFileDialog->ClearFileTypes();
			sprintf(temp, "%s.oxp", NewProjectName->Text);
			OpenFileDialog->AddFileType("Give New Project Location", temp);
			OpenFileDialog->Show();
	
			if (!OpenFileDialog->Cancelled)
			{
				strcpy(ProjectDirectory, OpenFileDialog->DirectorySelected);
				strcpy(ProjectFile, OpenFileDialog->FileSelected);
				sprintf(temp, ".%s", ProjectFile);

				if (strlen(temp) >= 4)
				{
					Marker = temp + strlen(temp) - 4;	/* Change to .oxd */
					if (!strcmp(Marker, ".oxp"))
					{
						Marker = Marker + 3;
						*Marker = 'd';
					}
				}
				else
				{
					strcat(temp, ".oxd");
				}
				mkdir(temp, S_IRWXU);
				chmod(temp, S_IRWXU);
				chdir(temp);
				LocationSet = 1;	/* Saving is allowed. We are in the right place. */
			}
			else
			{
				chdir(OldDir);
			}
		}
		else
		{
			LocationSet = 1;	/* Saving is allowed. We are in the right place. */
		}
		
	}
	else
	{
		LocationSet = 1;	/* Saving is allowed. We are in the right place. */
	}
	
	if(LocationSet)
	{
									/* Save all of the objects. */
									/* This must be done first since the incorporation of */
									/* objects will affect all other file saves. */
		for (InternalFile = InternalOxcCode; InternalFile != NULL; InternalFile = InternalFile->Next)
		{
			if (!InternalFile->HasBeenDeleted)
			{
				InternalFile->Save();
				InternalFile->IsIncorporated = True;
			}
		}

									/* Mark all objects as incorporated. */
		for (ProjectObject = ProjectObjects; ProjectObject != NULL; ProjectObject = ProjectObject->Next)
		{
			ProjectObject->IsIncorporated = True;
		}

							/* Remake the c++ files. This MUST be after the incorporation loop. */
		for (ProjectObject = ProjectObjects; ProjectObject != NULL; ProjectObject = ProjectObject->Next)
		{
			/* Locate the associated window and make the c++ file. */
			if (!strcmp(ProjectObject->Type, "OnyxWindow"))
			{
				PrepareForMake(ProjectObject->Name);
			}
		}

		PrepareForMake("Application");

		InitializeInternalProjectFile(ProjectFile); /* Sync (Here until I create the insert/replace code in internalfile::) */
		InternalOxpCode->Save();				/* Write new project file. */

		InitializeInternalMakefile();
		InternalMakefile->Save();				/* Write new Makefile. */

		InitializeInternalEvents_h();
		InitializeInternalObjects_h();
		InitializeInternalIncludes_h();
		InitializeInternalMain_h();
								/* Save all the headers files. */
		for (InternalFile = InternalHeader; InternalFile != NULL; InternalFile = InternalFile->Next)
		{
			InternalFile->Save();
		}

		InitializeInternalMain_cc();				/* Create and save main.cc */
		InternalMaincc->Save();

		/* Pull in the OnyX wrapper into .onyx */

		mkdir(".onyx", S_IRWXU);
		chmod(".onyx", S_IRWXU); /* Yes, I am purposefully overriding the umask here. Maybe I will change this. */

		InternalBaseMakefile->Save();				/* Write new Base Makefile. */
	}
}

void ShowCodeButton_ButtonClick()
{
	InternalCodeFile *CurrentCode;
	char ObjectName[255];


	if (SelectedObject == NULL)	/* If we are dealing with "Application" then */
	{
		sprintf(ObjectName, "Application");
	}
	else	/* Otherwise use the name of the object. */
	{
		sprintf(ObjectName, SelectedObject->Name);
	}

	/* Locate code if it is currently in memory. */
	CurrentCode = InternalOxcCode;
	while (CurrentCode != NULL)
	{
		if (!CurrentCode->HasBeenDeleted && !strcmp(ObjectName, CurrentCode->ObjectName))
		{
			break;
		} 
		CurrentCode = CurrentCode->Next;
	}

	/* If it wasn't found... */
	if (CurrentCode == NULL)
	{
		/* Add code file to list, if there is no list then start a new one. */
		InternalOxcCode = new InternalCodeFile(ObjectName, NULL, InternalOxcCode);
		CurrentCode = InternalOxcCode;
		CurrentCode->Open();
	}

	CurrentCode->Edit(EditorCommand);
	EditTimer->Start();	/* Start the monitor to watch for when the user quits the editor. */
}

void SnapToGrid_ButtonClick()
{
	SnapToGridIsOn = !SnapToGridIsOn;
	if (SnapToGridIsOn)
	{
		SnapToGridMenuItem->Check();
	}
	else
	{
		SnapToGridMenuItem->UnCheck();
	}
}

void ShowPropertiesButton_ButtonClick()
{
	if (PropertiesWindow->IsVisible)
	{
		PropertiesWindow->Close();	/* Close the Properties window. */
	}
	else
	{
		PropertiesWindow->Show();	/* Show the Properties window. */
	}
}

void ShowToolboxButton_ButtonClick()
{
	if (Toolbox->IsVisible) {
		Toolbox->Close();
	} else {
		Toolbox->Show();		/* Show the toolbox. */
	}
}

void StopButton_ButtonClick()
{
	if (RunPID != 0) {
		kill(RunPID, 9);		/* Kill the running development process. */
	}
}

void DeleteControlButton_ButtonClick()
{
	struct ProjectObjectList *SelectedObjectLocal;

	SelectedObjectLocal = SelectedObject; /* Make local copy to avoid conflicts */

	if (ControlsLocked)  /* If controls locked then beep error. */
	{
		XBell(ApplicationRoot()->GetDisplay(), 0);
	}
	else  /* Else delete the object. */
	{
		XSetInputFocus(ApplicationRoot()->GetDisplay(), None, None, CurrentTime);
		DeleteDevelopmentObject(SelectedObjectLocal);

		// Delete the visible object
		int i = SelectedItemID - DevelopmentItemIDModifier;
		if (DevelopmentObject[i] != NULL)
		{
			delete DevelopmentObject[i];
			DevelopmentObject[i] = NULL;
			// Hide the brackets since there is no longer
			// the object for them to surround.
			MoveBrackets(-10, -10, -10, -10);
		}

		if (PropertiesWindow->IsVisible)
		{
			PropertiesWindow_Update();
		}

		if (ProjectExplorer->IsVisible)
		{
			ProjectExplorer_Update();
		}

		if (DevelopmentWindow[CurrentDevelopmentWindow]->IsVisible)
		{
			DevelopmentWindow[CurrentDevelopmentWindow]->Update();
		}
	}
}

void LockControlsButton_ButtonClick()
{
	ControlsLocked = !ControlsLocked;
}

void NewProjectMenuItem_MenuItemSelected()
{
	NewProjectWindow->Show();
}

void NewProjectMenuItem_MenuItemHighlighted()
{
}

void OpenProjectMenuItem_MenuItemSelected()
{
	OpenButton_ButtonClick();
}

void OpenProjectMenuItem_MenuItemHighlighted()
{
}

void SaveObjectMenuItem_MenuItemSelected()
{
	SaveButton_ButtonClick();
}

void SaveObjectMenuItem_MenuItemHighlighted()
{
}

void SaveProjectMenuItem_MenuItemSelected()
{
	SaveProjectButton_ButtonClick();
}

void SaveProjectMenuItem_MenuItemHighlighted()
{
}

void SaveProjectAsMenuItem_MenuItemSelected()
{
	char OldDir[255];
	InternalCodeFile *InternalFile;
	char temp[255];
	char *Marker;
	struct stat fstat;
	int AskAgain;


	SaveFileDialog->ClearFileTypes();	/* Re-initialize Save File Dialog. (For future reasons.) */
	SaveFileDialog->AddFileType("Current Project", ProjectFile);	/* Default is current project. */
	SaveFileDialog->AddFileType("Onyx Project", "*.oxp");

	getcwd(OldDir, 255);	/* Save current location. */
	chdir("..");	/* Jump back to the directory where the current project (no symlinks) is. */

	AskAgain = 1;

	while (AskAgain > 0)
	{
		SaveFileDialog->Show();		/* Show the save dialog. */

		if (SaveFileDialog->Cancelled)	/* If cancelled then don't ask and don't save. */
		{
			AskAgain = -1;
		}
		else if (strlen(SaveFileDialog->FileSelected) && (SaveFileDialog->FileSelected[0] != '*'))
		{
			/* Add the .oxp extension if not supplied */
			Marker = strrchr(SaveFileDialog->FileSelected, '.');
			if (!Marker || strcmp(Marker, ".oxp"))
			{
				strcat(SaveFileDialog->FileSelected, ".oxp");
			}

			/* If we have a potentially valid file then */
			/* see if it already exists. */
			if (stat(SaveFileDialog->FileSelected, &fstat) == 0)
			{
				/* Uhh, ohh. Attempted overwrite. Better ask! */
				sprintf(ConfirmMsgBox->Message, "%s already exists! Overwrite?", SaveFileDialog->FileSelected);
				ConfirmMsgBox->Show();
				if (ConfirmMsgBox->Response == YesButton)
				{
					AskAgain = 0;
				}
				else if (ConfirmMsgBox->Response == NoButton)
				{
					AskAgain = 1;
				}
				else
				{
					AskAgain = -1;
				}
			}
			else
			{
				AskAgain = 0;
			}
		}
		else
		{
			AskAgain = 1;
		}
	}
	
	if (AskAgain == 0)	/* If there wasn't a cancel */
	{
		strcpy(ProjectDirectory, SaveFileDialog->DirectorySelected);
		strcpy(ProjectFile, SaveFileDialog->FileSelected);
		sprintf(temp, ".%s", ProjectFile);

		if (strlen(temp) >= 4)
		{
			Marker = temp + strlen(temp) - 4;	/* Change to .oxd */
			if (!strcmp(Marker, ".oxp"))
			{
				Marker = Marker + 3;
				*Marker = 'd';
			}
		}
		mkdir(temp, S_IRWXU);
		chmod(temp, S_IRWXU);
		chdir(temp);

		InitializeInternalProjectFile(ProjectFile);	/* Sync (Here until I create the insert/replace code in internalfile::) */
		InternalOxpCode->Save();			/* Write new project file. */
		InternalMakefile->Save();			/* Write new Makefile. */
		InternalBaseMakefile->Save();			/* Write new Base Makefile. */

								/* Save all the headers files. */
		for (InternalFile = InternalHeader; InternalFile != NULL; InternalFile = InternalFile->Next)
		{
			InternalFile->Save();
		}

		InitializeInternalMain_cc();			/* Create and save main.cc */
		InternalMaincc->Save();

								/* Save all of the objects. */
		for (InternalFile = InternalOxcCode; InternalFile != NULL; InternalFile = InternalFile->Next)
		{
			if (!InternalFile->HasBeenDeleted)
			{
				InternalFile->Save();
			}
		}

                /* Modify name of main window to reflect project that is open. */
                sprintf(MainWindow->Name, "OnyX - %s", ProjectFile);
                Marker = MainWindow->Name + strlen(MainWindow->Name) - 4;
                if (!strcmp(Marker, ".oxp"))
                {
                        *Marker = '\0';
                }
                MainWindow->Update();

	}
	else
	{
		chdir(OldDir);	/* Go back to were we started. */
	}
}

void SaveProjectAsMenuItem_MenuItemHighlighted()
{
}

void ExportObjectMenuItem_MenuItemSelected()
{
	InternalCodeFile *InternalCode;
	struct ProjectObjectPropertyList *ObjectProperty;
	char OldDir[255];
	char temp[255];
	FILE *ExportFile;
	int AskAgain;
	struct stat fstat;

	/* If this is the Application */
	if (SelectedObject != NULL)
	{
		SaveFileDialog->ClearFileTypes();	/* Reinit to clear out save file and save project stuff. */
		SaveFileDialog->AddFileType("Onyx Exported Object", "*.oeb");

		getcwd(OldDir, 255);	/* Save current location. */
		chdir("..");		/* Jump back to the directory where the current project (no symlinks) is. */

		AskAgain = 1;
	
		while (AskAgain > 0)
		{
			SaveFileDialog->Show();		/* Show the save dialog. */

			if (SaveFileDialog->Cancelled)	/* If cancelled then don't ask and don't save. */
			{
				AskAgain = -1;
			}
			else if (strlen(SaveFileDialog->FileSelected) && (SaveFileDialog->FileSelected[0] != '*'))
			{
				/* If we have a potentially valid file then */
				/*  see if it already exists. */
				if (stat(SaveFileDialog->FileSelected, &fstat) == 0)
				{
					/* Uhh, ohh. Attempted overwrite. Better ask! */
					sprintf(ConfirmMsgBox->Message, "%s already exists! Overwrite?", SaveFileDialog->FileSelected);
					ConfirmMsgBox->Show();
					if (ConfirmMsgBox->Response == YesButton)
					{
						AskAgain = 0;
					}
					else if (ConfirmMsgBox->Response == NoButton)
					{
						AskAgain = 1;
					}
					else
					{
						AskAgain = -1;
					}
				}
				else
				{
					AskAgain = 0;
				}
			}
			else
			{
				AskAgain = 1;
			}
		}


		if (AskAgain == 0) /* If the save file dialog wasn't cancelled */
		{
			/* Create the full name of the export file. */
			sprintf(temp, "%s/%s", SaveFileDialog->DirectorySelected, SaveFileDialog->FileSelected);

			/* Locate internal code for the object. */
			for (InternalCode = InternalOxcCode; InternalCode->HasBeenDeleted || strcmp(InternalCode->ObjectName, SelectedObject->Name); InternalCode = InternalCode->Next)
			{}

			/* Open the file. */
			ExportFile = fopen(temp, "w");
			if (!ExportFile)
			{
				ExclamationBox(MainWindow->Self(), "Now this sucks.", "Could not open the export file.");
			}
			else
			{
				/* Write header. */
				fprintf(ExportFile, "OnyX Exported Object\n");

				/* Write properties. */
				fprintf(ExportFile, "StartObject\n");
				fprintf(ExportFile, "ObjectName %s\n", SelectedObject->Name);
				fprintf(ExportFile, "ObjectType %s\n", SelectedObject->Type);

				ObjectProperty = SelectedObject->Property;
				while (ObjectProperty != NULL)
				{
					fprintf(ExportFile, "%s %s\n", ObjectProperty->Name, ObjectProperty->Value);
					ObjectProperty = ObjectProperty->Next;
				}
				fprintf(ExportFile, "EndObject\n");

				/* Close the file. */
				fclose(ExportFile);

				/* Append the code from the oxc. */
				InternalCode->AppendToFile(temp);
			}
		}

		chdir(OldDir);
	}
}

void ExportObjectMenuItem_MenuItemHighlighted()
{
}

void ImportObjectMenuItem_MenuItemSelected()
{
	InternalCodeFile *InternalCode;
	struct ObjectList *UsableObject;
	struct ProjectObjectList *CurrentObject;
	struct ProjectObjectPropertyList *CurrentProperty;
	struct ObjectProperty *UsableProperty;
	char OldDir[255];
	char temp[255];
	char InLine[255];
	FILE *ExportFile;
	int ExportVerified;
	int ExportIdentified;
	char *tmp;
	char *Marker;

	OpenFileDialog->ClearFileTypes();	/* Reinit to clear out save file and save project stuff. */
	OpenFileDialog->AddFileType("Onyx Exported Object","*.oeb");

	getcwd(OldDir,255);	/* Save current location. */
	chdir("..");	/* Jump back to the directory where the current project (no symlinks) is. */

	OpenFileDialog->Show();		/* Show the open dialog. */
	chdir(OldDir);			/* Jump back to where we were. */

		/* If not cancelled and FileSelected != '*'  then import the object. */
	if((!OpenFileDialog->Cancelled) && (strlen(OpenFileDialog->FileSelected)) && (OpenFileDialog->FileSelected[0] != '*'))
	{
		/* Create the full name of the export file. */
		sprintf(temp,"%s/%s",OpenFileDialog->DirectorySelected,OpenFileDialog->FileSelected);

		/* Open the file. */
		ExportFile = fopen(temp,"r");
		if(!ExportFile)
		{
			ExclamationBox(MainWindow->Self(),"Umm... Ok. This ain't good.","Could not open the export file.");
		}
		else
		{
			ExportIdentified = 0;

			/* Verify that this is an OnyX Exported Object */
			while(!ExportIdentified && fgets(InLine,255,ExportFile))
			{
				ExportIdentified = !strncmp(InLine,"OnyX Exported Object",20);
			}

			if(ExportIdentified)
			{
				ExportVerified = 1;	/* Actually a reversed boolean so pay attention. */
				while(ExportVerified && fgets(InLine,255,ExportFile))
				{
					if(!strncmp("StartObject",InLine,11)) /* If this line starts an object description */
					{
						/* Get to the end of the list. */
						for (CurrentObject = ProjectObjects; CurrentObject->Next != NULL; CurrentObject = CurrentObject->Next)
						{}
						CurrentObject->Next = new ProjectObjectList;
						CurrentObject = CurrentObject->Next;
	
						CurrentObject->Next = NULL;
						CurrentObject->Property = NULL;
						CurrentObject->IsIncorporated = False;
	
						CurrentProperty = NULL;
	
						/* Grab all the properties/info about the object. */
						while((fgets(InLine,255,ExportFile) != NULL) && (ExportVerified = strncmp("EndObject",InLine,9)))
						{
							InLine[strlen(InLine) -1] = '\0';

							if(!strncmp(InLine,"ObjectName",10)) /* If this line names the object */
							{
								tmp = InLine;
								tmp = tmp + 11;
								strcpy(CurrentObject->Name,tmp);
							}
							else if(!strncmp(InLine,"ObjectType",10)) /* Else if this line says what type of object it is */
							{
								tmp = InLine;
								tmp = tmp + 11;
								strcpy(CurrentObject->Type,tmp);
	
								/* If we have never seen this object type then go get it from the repository */
								if(!ObjectTypeKnown(CurrentObject->Type))
								{
									/* Find the end of the usable object list. */
									for (UsableObject = UsableObjects; UsableObject->Next != NULL; UsableObject = UsableObject->Next)
									{}
	
									UsableObject->Next = GetNewObjectType(CurrentObject->Type);
									UsableObject = UsableObject->Next;
									UsableObject->Next = NULL;
	
								}
								else /* Find the object type. */
								{
									for (UsableObject = UsableObjects; strcmp(UsableObject->Name,CurrentObject->Type); UsableObject = UsableObject->Next)
									{}
								}

								/* Add all of the property slots needed. */
								/* This will help aid in the conversion of older objects to */
								/* updated ones when the .oob files are changed. */
								CurrentProperty = NULL;
								for (UsableProperty = UsableObject->Property; UsableProperty != NULL; UsableProperty = UsableProperty->Next)
								{
									/* Create property list. */
									if(CurrentProperty == NULL)
									{
										CurrentObject->Property = new ProjectObjectPropertyList;
										CurrentProperty = CurrentObject->Property;
									}
									else /* ...or append to it. */
									{
										CurrentProperty->Next = new ProjectObjectPropertyList;
										CurrentProperty = CurrentProperty->Next;
									}
									CurrentProperty->Next = NULL;
	
									/* Store property name. */
									strcpy(CurrentProperty->Name,UsableProperty->Name);
									/* Assign the default value. */
									strcpy(CurrentProperty->Value,UsableProperty->Default);
								}
							}
							else /* else this line must be a property description. */
							{
								/* Separate the property from its value. */
								Marker = strchr(InLine,' ');	/* Locate the space divider. */
								if(Marker != NULL)	/* Ensure that a value is present. */
								{
									*Marker = '\0';	/* Cut it off. */
									Marker++;	/* Move to the begining of the property value. */
								}
	
								/* Locate its proper slot. */
								for (CurrentProperty = CurrentObject->Property; CurrentProperty != NULL && strcmp(CurrentProperty->Name,InLine); CurrentProperty = CurrentProperty->Next)
								{}
	
								/* If it was found put in the value. */
								if(CurrentProperty != NULL)
								{
									/* If the property value was not found. */
									if(Marker == NULL)
									{
										/* Alert user of missing value. */
										printf("Warning: Property %s has no value.\n",InLine);
									}
									else
									{
										/* Store property value. */
										strcpy(CurrentProperty->Value,Marker);
									}
								}
								else	/* Alert user of extraneous property setting. */
								{
									printf("Warning: Unknown property %s with value %s removed.\n",InLine,Marker);
								}
							}
						}
					}
				}
				if(ExportVerified != 0)	/* strncmp didn't event return a 0 for a match. */
				{
					StopBox(MainWindow->Self(),"Things weren't all there","Missing information, Object not imported.");

					/* Free space allocated to new object. */
					/* Locate the next to last object. */
					for (CurrentObject = ProjectObjects; CurrentObject->Next->Next != NULL; CurrentObject = CurrentObject->Next)
					{}
					delete CurrentObject->Next;
					CurrentObject->Next = NULL;

				}
				else
				{
					if(SelectedWindow == NULL)
					{
						if(strcmp(CurrentObject->Type,"OnyxWindow"))
						{
							sprintf(temp,"Cannot attach %ss to Application. Please select a window first.",CurrentObject->Type);
							ExclamationBox(MainWindow->Self(),"Won't go there",temp);
	
							/* Free space allocated to new object. */
							/* Locate the next to last object. */
							for (CurrentObject = ProjectObjects; CurrentObject->Next->Next != NULL; CurrentObject = CurrentObject->Next)
							{}
							delete CurrentObject->Next;
							CurrentObject->Next = NULL;
						}
						else
						{
							strcpy(CurrentObject->Parent,"Application");

					                /* Create new internal oxc file */
						        InternalOxcCode = new InternalCodeFile(CurrentObject->Name,NULL,InternalOxcCode);
						        InternalCode = InternalOxcCode; 
						        InternalCode->IsIncorporated = False;
						        InternalCode->HasBeenModified = True;

							/* Read the rest of the export file in as the matching code. */
							while(fgets(InLine,255,ExportFile))
							{
								InternalCode->Append("%s", InLine);
							}

							if(ProjectExplorer->IsVisible)
							{
								ProjectExplorer_Update();
							}
						}
					}
					else
					{
						if(!strcmp(CurrentObject->Type,"OnyxWindow"))
						{
							AsteriskBox(MainWindow->Self(),"Won't go there","Cannot embed OnyxWindow objects.\nAttaching it to the Application instead.");
							strcpy(CurrentObject->Parent,"Application");
						}
						else
						{
							strcpy(CurrentObject->Parent,SelectedWindow->Name);
						}


					        /* Create new internal oxc file */
						InternalOxcCode = new InternalCodeFile(CurrentObject->Name,NULL,InternalOxcCode);
						InternalCode = InternalOxcCode; 
						InternalCode->IsIncorporated = False;
						InternalCode->HasBeenModified = True;

						/* Read the rest of the export file in as the matching code. */
						while(fgets(InLine,255,ExportFile))
						{
							InternalCode->Append("%s", InLine);
						}

						ViewWindowButton_ButtonClick();
						if(ProjectExplorer->IsVisible)
						{
							ProjectExplorer_Update();
						}
					}
				}
			}
			else
			{
				sprintf(temp,"%s did not identify itself as an Onyx Export File",OpenFileDialog->FileSelected);
				StopBox(MainWindow->Self(),"Objection, Your Honour!",temp); 
			}
			fclose(ExportFile);
		}
	}
}

void ImportObjectMenuItem_MenuItemHighlighted()
{
}


void ExportProjectMenuItem_MenuItemSelected()
{
	InternalCodeFile *InternalCode;
	char OldDir[255];
	char temp[255];
	FILE *ExportFile;
	int AskAgain;
	struct stat fstat;

	SaveFileDialog->ClearFileTypes();	/* Reinit to clear out save file and save project stuff. */
	SaveFileDialog->AddFileType("Onyx Exported Project","*.oep");


	getcwd(OldDir,255);	/* Save current location. */
	chdir("..");	/* Jump back to the directory where the current project (no symlinks) is. */


	AskAgain = 1;

	while(AskAgain > 0)
	{
		SaveFileDialog->Show();		/* Show the save dialog. */

		if(SaveFileDialog->Cancelled)	/* If cancelled then don't ask and don't save. */
		{
			AskAgain = -1;
		}
		else if (strlen(SaveFileDialog->FileSelected) && (SaveFileDialog->FileSelected[0] != '*'))
		{
			/* If we have a potentially valid file then */
			/*  see if it already exists. */
			if(stat(SaveFileDialog->FileSelected,&fstat) == 0)
			{
				/* Uhh, ohh. Attempted overwrite. Better ask! */
				sprintf(ConfirmMsgBox->Message,"%s already exists! Overwrite?",SaveFileDialog->FileSelected);
				ConfirmMsgBox->Show();
				if(ConfirmMsgBox->Response == YesButton)
				{
					AskAgain = 0;
				}
				else if(ConfirmMsgBox->Response == NoButton)
				{
					AskAgain = 1;
				}
				else
				{
					AskAgain = -1;
				}
			}
			else
			{
				AskAgain = 0;
			}
		}
		else
		{
			AskAgain = 1;
		}
	}


	if(AskAgain == 0) /* If the save file dialog wasn't cancelled */
	{
		/* Create the full name of the export file. */
		sprintf(temp,"%s/%s",SaveFileDialog->DirectorySelected,SaveFileDialog->FileSelected);

		/* Open the file. */
		ExportFile = fopen(temp,"w");
		if(!ExportFile)
		{
			ExclamationBox(MainWindow->Self(),"Now this sucks.","Could not open the export file.");
		}
		else
		{
			/* Write header. */
			fprintf(ExportFile,"OnyX Exported Project\n");
			fprintf(ExportFile,"Start Oxp\n");
			fclose(ExportFile);

			/* Append the oxp file. */
			InternalOxpCode->AppendToFile(temp);
			ExportFile = fopen(temp,"a");
			fprintf(ExportFile,"End Oxp\n");

			/* Locate internal code for all objects. */
			for (InternalCode = InternalOxcCode; InternalCode != NULL; InternalCode = InternalCode->Next)
			{
				if (!InternalCode->HasBeenDeleted)
				{
					fprintf(ExportFile,"Start Oxc %s\n",InternalCode->ObjectName);
					fclose(ExportFile);
					InternalCode->AppendToFile(temp);
					ExportFile = fopen(temp,"a");
					fprintf(ExportFile,"End Oxc\n");
				}
			}
			fclose(ExportFile);
		}
	}
	chdir(OldDir);
}

void ExportProjectMenuItem_MenuItemHighlighted()
{
}


void ImportProjectMenuItem_MenuItemSelected()
{
	char OldDir[255];
	char temp[255];
	char temp2[255];
	char InLine[255];
	FILE *ExportFile;
	FILE *NewFile;
	int ExportVerified;
	int ExportIdentified;
	char *Marker;

	OpenFileDialog->ClearFileTypes();	/* Reinit to clear out save file and save project stuff. */
	OpenFileDialog->AddFileType("Onyx Exported Project","*.oep");

	if(ValidProject)
	{
		getcwd(OldDir,255);	/* Save current location. */
		chdir("..");	/* Jump back to the directory where the current project (no symlinks) is. */
	}

	OpenFileDialog->Show();		/* Show the open dialog. */

		/* If not cancelled and FileSelected != '*'  then request save location. */
	if((!OpenFileDialog->Cancelled) && (strlen(OpenFileDialog->FileSelected)) && (OpenFileDialog->FileSelected[0] != '*'))
	{
		/* Locate the .oep if it exists. */
		strcpy(temp2,OpenFileDialog->FileSelected);
		Marker = strrchr(temp2,'.');
		if(Marker)
		{
			/* Terminate the name right there. */
			*Marker = '\0';
		}

		/* Append .oxp to the name. */
		strcat(temp2,".oxp");

		MsgBox(MainWindow->Self(),"Please select location of new project");

		SaveFileDialog->ClearFileTypes();	/* Re-initialize Save File Dialog. */
		SaveFileDialog->AddFileType("Default Name",temp2);
		SaveFileDialog->AddFileType("Onyx Project","*.oxp");

		SaveFileDialog->Show();	/* Show the save dialog. */

		if((!SaveFileDialog->Cancelled) && (strlen(SaveFileDialog->FileSelected)) && (SaveFileDialog->FileSelected[0] != '*'))
		{
			/* Create the full name of the export file. */
			sprintf(temp,"%s/%s",OpenFileDialog->DirectorySelected,OpenFileDialog->FileSelected);

			/* Open the file. */
			ExportFile = fopen(temp,"r");
			if(!ExportFile)
			{
				ExclamationBox(MainWindow->Self(),"Umm... Ok. This ain't good.","Could not open the export file.");
			}
			else
			{
				ExportIdentified = 0;

				/* Verify that this is an OnyX Exported Project */
				while(!ExportIdentified && fgets(InLine,255,ExportFile))
				{
					ExportIdentified = !strncmp(InLine,"OnyX Exported Project",20);
				}

				if(ExportIdentified)
				{
					/* Change to the new directory. */
					chdir(SaveFileDialog->DirectorySelected);
					sprintf(temp,".%s",SaveFileDialog->FileSelected);

					if(strlen(temp) >= 4)
					{
						Marker = temp + strlen(temp) - 4;	/* Change to .oxd */
						if(!strcmp(Marker,".oxp"))
						{
							Marker = Marker + 3;
							*Marker = 'd';
						}
					}
					
					
					if(mkdir(temp,S_IRWXU) == 0)	/* Make the .oxd directory. */
					{
						chmod(temp,S_IRWXU);	/* Make sure the privs are correct. */
						chdir(temp);		/* Change to the .oxd directory. */
						ExportVerified = 1;	/* Actually a reversed boolean so pay attention. */
						while(fgets(InLine,255,ExportFile))
						{
							if(!strncmp("Start Oxp",InLine,9)) /* Start of the oxp file. */
							{
								sprintf(temp,"../%s",SaveFileDialog->FileSelected);
								NewFile = fopen(temp,"w");
								while(fgets(InLine,255,ExportFile) && strncmp(InLine,"End Oxp",7))
								{
									fprintf(NewFile,"%s",InLine);
								}
								fclose(NewFile);
								chmod(temp,S_IRWXU);
							}
							else if(!strncmp("Start Oxc ",InLine,10))	/* Start of oxc code. */
							{
								if(sscanf(InLine,"%s %s %s",temp,temp,temp2) == 3)/* Get the name. */
								{
									strcat(temp2,".oxc");
									NewFile = fopen(temp2,"w");
									while(fgets(InLine,255,ExportFile) && strncmp(InLine,"End Oxc",7))
									{
										fprintf(NewFile,"%s",InLine);
									}
									fclose(NewFile);
								}
							}
						}
					}
					else
					{
						ExclamationBox(MainWindow->Self(),"Phooey.","Could not create the new project.");
					}
				}
				else
				{
					sprintf(temp,"%s did not identify itself as an Onyx Project File",OpenFileDialog->FileSelected);
					StopBox(MainWindow->Self(),"Objection, Your Honour!",temp); 
				}
				fclose(ExportFile);
			}
		}
	}

	if(ValidProject)
	{
		chdir(OldDir);			/* Jump back to where we were. */
	}
	else
	{
		chdir("..");
	}
}

void ImportProjectMenuItem_MenuItemHighlighted()
{
}

void CreateApplicationMenuItem_MenuItemSelected()
{
	char OldDir[255];
	char temp[255], tempdir[20];
	char *Marker;
	int AskAgain;
	struct stat fstat;

	sprintf(temp,"%s.tgz",ProjectApplicationName);

	SaveFileDialog->ClearFileTypes();	/* Re-initialize Save File Dialog. */
	SaveFileDialog->AddFileType("Default Name",temp);
	SaveFileDialog->AddFileType("Other tar-gzip file","*.tgz");

	
	getcwd(OldDir,255);	/* Save current location. */
	chdir("..");	/* Jump back to the directory where the current project (no symlinks) is. */

	AskAgain = 1;

	while(AskAgain > 0)
	{
		SaveFileDialog->Show();		/* Show the save dialog. */

		if(SaveFileDialog->Cancelled)	/* If cancelled then don't ask and don't save. */
		{
			AskAgain = -1;
		}
		else if (strlen(SaveFileDialog->FileSelected) && (SaveFileDialog->FileSelected[0] != '*'))
		{
			/* If we have a potentially valid file then */
			/*  see if it already exists. */
			if(stat(SaveFileDialog->FileSelected,&fstat) == 0)
			{
				/* Uhh, ohh. Attempted overwrite. Better ask! */
				sprintf(ConfirmMsgBox->Message,"%s already exists! Overwrite?",SaveFileDialog->FileSelected);
				ConfirmMsgBox->Show();
				if(ConfirmMsgBox->Response == YesButton)
				{
					AskAgain = 0;
				}
				else if(ConfirmMsgBox->Response == NoButton)
				{
					AskAgain = 1;
				}
				else
				{
					AskAgain = -1;
				}
			}
			else
			{
				AskAgain = 0;
			}
		}
		else
		{
			AskAgain = 1;
		}
	}
	
	chdir(OldDir);	/* Return to original location. */

	if(AskAgain == 0)	/* If there wasn't a cancel */
	{
#ifdef HAVE_MKDTEMP
		strcpy(tempdir, "OnyXtempXXXXXX");
		if((Marker = mkdtemp(tempdir)))		/* Create a temporary directory. */
		{
#else
		if((Marker = tmpnam(NULL)))		/* Get a temporary name. */
		{
			mkdir(Marker,S_IRWXU);				/* Use the name for a temporary directory */
#endif
			chmod(Marker,S_IRWXU);

			sprintf(temp,"%s/%s",Marker,ProjectApplicationName);
			mkdir(temp,S_IRWXU);				/* Create the application directory inside of it. */
			chmod(temp,S_IRWXU);

			sprintf(temp,"%s/%s/.onyx",Marker,ProjectApplicationName);
			mkdir(temp,S_IRWXU);				/* Create the .onyx inside of it. */
			chmod(temp,S_IRWXU);

			/* Copy all main files needed */
			sprintf(temp,"cp -r *.cc *.h Makefile %s/%s",Marker,ProjectApplicationName);
			system(temp);

			/* Copy all .onyx files needed */
			sprintf(temp,"cp -r .onyx/*.cc .onyx/*.h .onyx/Makefile %s/%s/.onyx",Marker,ProjectApplicationName);
			system(temp);

			/* Tar ang gzip the directory. */
			chdir(Marker);
			sprintf(temp,"tar -zcf %s/%s %s",SaveFileDialog->DirectorySelected,SaveFileDialog->FileSelected,ProjectApplicationName);
			system(temp);
			chdir(OldDir);

			/* Remove the temporary directory. */
			sprintf(temp,"rm -r %s",Marker);
			system(temp);
		}
	}			
}

void ExitMenuItem_MenuItemSelected()
{
	ApplicationClose();
}

void ExitMenuItem_MenuItemHighlighted()
{
}

void ViewCodeMenuItem_MenuItemSelected()
{
	ShowCodeButton_ButtonClick();
}

void ViewCodeMenuItem_MenuItemHighlighted()
{
}

void ViewWindowMenuItem_MenuItemSelected()
{
	ViewWindowButton_ButtonClick();
}

void ViewWindowMenuItem_MenuItemHighlighted()
{
}

void ViewGridMenuItem_MenuItemSelected()
{
	int i;
        struct ProjectObjectPropertyList *UsableProperty;

	if(ViewGridMenuItem->IsChecked)
	{
		ViewGridMenuItem->UnCheck();

		for (i = 0; i < MaxDevelopmentWindows; i++)	/* Ungrid every development window. */
		{
			if(DevelopmentWindow[i]  != NULL)	/* If this window exists */
			{
				/* Clear the old directory and picture name. */
				DevelopmentWindow[i]->Directory[0] = '\0';
				DevelopmentWindow[i]->Picture[0] = '\0';
				DevelopmentWindow[i]->SetBackgroundColor();	/* Set the background color */
	                        UsableProperty = WindowToListMap[i]->Property;	/* Start at the begining of the property list */

				while(UsableProperty)	/* Find the picture setting. */
				{
					if(!strcmp(UsableProperty->Name,"Picture")) /* If this is the setting */
					{
						/* Copy the value into the development windows picture setting. */
						strcpy(DevelopmentWindow[i]->Picture,UsableProperty->Value);
					}
					else if(!strcmp(UsableProperty->Name,"Directory"))	/* If this is the directory of the picture */
					{
						strcpy(DevelopmentWindow[i]->Picture,UsableProperty->Value);
					}
					UsableProperty = UsableProperty->Next;
				}

				if(strlen(DevelopmentWindow[i]->Picture) > 0)	/* If the picture was used */
				{
					DevelopmentWindow[i]->SetBackgroundPicture();	/* Reset the picture. */
				}

				DevelopmentWindow[i]->Update();	/* Update the window */

			}
		}
	}
	else
	{
		ViewGridMenuItem->Check();

		for (i = 0; i < MaxDevelopmentWindows; i++)	/* For each possible development window */
		{
			if(DevelopmentWindow[i] != NULL)	/* If it exists */
			{
				/* Put the grid back on. */
				strcpy(DevelopmentWindow[i]->Picture,"onyx_grid.xpm");
				strcpy(DevelopmentWindow[i]->Directory,OnyxIconLocation);
				DevelopmentWindow[i]->SetBackgroundPicture();
				DevelopmentWindow[i]->Update();
			}
		}
	}

	/* Now update all existing development objects to make sure they reflect the changes. */
	/* Otherwise labes, buttons, etc tend to keep the old look for a while. */
	for (i = 0; i < MaxDevelopmentObjects; i++)
	{
		if(DevelopmentObject[i] != NULL)
		{
			DevelopmentObject[i]->Update();
		}
	}
}

void ViewGridMenuItem_MenuItemHighlighted()
{
}

void ViewProjectMenuItem_MenuItemSelected()
{
	ProjectExplorerButton->Push();
	ProjectExplorerButton_ButtonClick();
}

void ViewProjectMenuItem_MenuItemHighlighted()
{
}

void ViewPropertiesMenuItem_MenuItemSelected()
{
	ShowPropertiesButton->Push();
	ShowPropertiesButton_ButtonClick();
}

void ViewPropertiesMenuItem_MenuItemHighlighted()
{
}

void ToolBoxMenuItem_MenuItemSelected()
{
	ShowToolboxButton->Push();
	ShowToolboxButton_ButtonClick();
}

void ToolBoxMenuItem_MenuItemHighlighted()
{
}

void ToolBarMenuItem_MenuItemSelected()
{
	/* Resize the toolbar. */
	if(ToolBarMenuItem->IsChecked)
	{
		ToolBarMenuItem->UnCheck();
		//ToolbarSeparator->Hide();
		MainWindow->Resize(MainWindow->Width, MainMenuBar->Height);
	}
	else
	{
		ToolBarMenuItem->Check();
		//ToolbarSeparator->UnHide();
		MainWindow->Resize(MainWindow->Width, OpenButton->Y + OpenButton->Height);
	}
	MainWindow->ResetMinimumSize(MainWindow->MinimumWidth, MainWindow->Height);
	MainWindow->ResetMaximumSize(MainWindow->MaximumWidth, MainWindow->Height);
}

void ToolBarMenuItem_MenuItemHighlighted()
{
}


void MakeMenuItem_MenuItemSelected()
{
	MakeButton_ButtonClick();
}

void MakeMenuItem_MenuItemHighlighted()
{
}

void MakeCleanMenuItem_MenuItemSelected()
{
	strcpy(MakeType, "clean");
	MakeWindow->Show();
}

void MakeCleanMenuItem_MenuItemHighlighted()
{
}

void MakeRunMenuItem_MenuItemSelected()
{
	MakeRunButton_ButtonClick();
}

void MakeRunMenuItem_MenuItemHighlighted()
{
}

void RunMenuItem_MenuItemSelected()
{
	RunButton_ButtonClick();
}

void RunMenuItem_MenuItemHighlighted()
{
}

void StopMenuItem_MenuItemSelected()
{
	StopButton_ButtonClick();
}

void StopMenuItem_MenuItemHighlighted()
{
}

void MenuEditorItem_MenuItemSelected()
{
	/* Make sure we have a menu bar selected. */
	if (strcmp(SelectedObject->Type, "OnyxMenuBar"))
	{
		AsteriskBox(MainWindow->Self(), "An onyx menu bar must be selected.");
	}
	else
	{
		MenuEditor->Show();
	}
}

void MenuEditorItem_MenuItemHighlighted()
{
}

void SnapToGridMenuItem_MenuItemSelected()
{
	SnapToGridIsOn = !SnapToGridIsOn;
	if(SnapToGridIsOn)
	{
		SnapToGrid->Push();
		SnapToGridMenuItem->Check();
	}
	else
	{
		SnapToGrid->Pop();
		SnapToGridMenuItem->UnCheck();
	}
}

void CustomControlsItem_MenuItemSelected()
{
	CustomControlsWindow->Show();
}

void CustomControlsItem_MenuItemHighlighted()
{
}

void OptionsItem_MenuItemSelected()
{
	OptionsWindow->Show();
}

void OptionsItem_MenuItemHighlighted()
{
}

void EditTimer_TimerElapsed()
{
	InternalCodeFile *CurrentCode;
	int ActiveFiles;

	for (CurrentCode = InternalOxcCode, ActiveFiles = 0; CurrentCode != NULL; CurrentCode = CurrentCode->Next)
	{
		if (CurrentCode->EditorPID && !CurrentCode->HasBeenDeleted)
		{
			ActiveFiles++;
			CurrentCode->Check();
		}
	}
	if (!ActiveFiles)
	{
		EditTimer->Stop();
	}
}


char *EscapeQuotes(char *string)
{
	static char temp[1024];
	char *p = temp;

	while (*string) {
		if (*string == '"') *p++ = '\\';
		*p++ = *string++;
	}
	*p = '\0';

	return temp;
}


OnyxObject *CreateDevelopmentObject(char *name) {
	OnyxObject *obj;

        if (!strcmp(name, "OnyxLabel")) {
		obj = new OnyxLabel();
        } else if (!strcmp(name, "OnyxIcon")) {
		obj = new OnyxIcon();
        } else if (!strcmp(name, "OnyxButton")) {
		obj = new OnyxButton();
        } else if (!strcmp(name, "OnyxCheckBox")) {
		obj = new OnyxCheckBox();
        } else if (!strcmp(name, "OnyxRadioButton")) {
		obj = new OnyxRadioButton();
        } else if (!strcmp(name, "OnyxSpinButton")) {
		obj = new OnyxSpinButton();
        } else if (!strcmp(name, "OnyxTextBox")) {
		obj = new OnyxTextBox();
        } else if (!strcmp(name, "OnyxComboBox")) {
		obj = new OnyxComboBox();
        } else if (!strcmp(name, "OnyxListBox")) {
		obj = new OnyxListBox();
        } else if (!strcmp(name, "OnyxDraw")) {
		obj = new OnyxDraw();
        } else if (!strcmp(name, "OnyxProgressBar")) {
		obj = new OnyxProgressBar();
        } else if (!strcmp(name, "OnyxTree")) {
		obj = new OnyxTree();
        } else if (!strcmp(name, "OnyxHScrollBar")) {
		obj = new OnyxHScrollBar();
        } else if (!strcmp(name, "OnyxVScrollBar")) {
		obj = new OnyxVScrollBar();
        } else {
	//	printf("Unknown object requested: %s\n", name);
		obj = new OnyxIcon();
		if (!strcmp(name, "OnyxMenuBar")) {
			obj->X = 0;
			obj->Y = 0;
			obj->Width = 100;
			obj->Height = 22;
			strcpy(obj->Picture, "onyx_menubarpix.xpm");
		} else {
			obj->X = 0;
			obj->Y = 15;
			strcpy(obj->Picture, name);
			strcat(obj->Picture, ".xpm");
		}
		obj->IsShaped = True;
        }

	return obj;
}


/* CreateDevelopmentWindow will dynamically create a window that "should" look exactly like the 
   final application window. This allows the user to add, delete, modify objects on the window. */
/* Note: This window will contain a grid that is not really on the users window. */

void CreateDevelopmentWindow(int WindowNum, struct ProjectObjectList *ProjectObject) {
  struct ProjectObjectPropertyList *Property;
  int i;

  if (DevelopmentWindow[WindowNum] != NULL) {  /* If used before then see if it exists. */
    if (DevelopmentWindow[WindowNum]->WindowExists) {

      /* If it exists then close it which will */
      /* destroy it (CloseDestroysWindow = True) */
      DevelopmentWindow[WindowNum]->Hide();

      /* Remove all items on the currently open window. */
      DestroyBrackets();
                                 
      /* Delete all attached objects and set their values to NULL. */
      for (i = 0; i < MaxDevelopmentObjects; i++) {
        /* If this item exists and has the closing window as it parent */
        if (DevelopmentObject[i] != NULL && DevelopmentObject[i]->Parent == DevelopmentWindow[WindowNum]->Self()) {
          delete DevelopmentObject[i];
          DevelopmentObject[i] = NULL;
        }
      }

      DevelopmentWindow[WindowNum]->Close();  /* was specified at creation. */
    }
    DevelopmentWindow[WindowNum] = NULL;
  }

  if (ProjectObject != NULL) {  /* If "Application" is not selected then make the window. */
    DevelopmentWindow[WindowNum] = new OnyxWindow;

    /* The ID is development use only. It is not the final ID */
    /* that will show up in the users program. */
    DevelopmentWindow[WindowNum]->ID = DevelopmentWindowIDModifier + WindowNum;
    DevelopmentWindow[WindowNum]->Parent = ApplicationRoot();

    /* Use the desired X, Y, Width, Height, and Name */
    for (Property = SelectedWindow->Property; Property != NULL && strcmp(Property->Name, "X"); Property = Property->Next) {}
    if (Property != NULL) {  /* If the property was found. */
      DevelopmentWindow[WindowNum]->X = atoi(Property->Value);
    }

    for (Property = SelectedWindow->Property; Property != NULL && strcmp(Property->Name, "Y"); Property = Property->Next) {}
    if (Property != NULL) {  /* If the property was found. */
      DevelopmentWindow[WindowNum]->Y = atoi(Property->Value);
    }

    for (Property = SelectedWindow->Property; Property != NULL && strcmp(Property->Name, "Width"); Property = Property->Next) {}
    if (Property != NULL) {  /* If the property was found. */
      DevelopmentWindow[WindowNum]->Width = atoi(Property->Value);
    }

    for (Property = SelectedWindow->Property; Property != NULL && strcmp(Property->Name, "Height"); Property = Property->Next) {}
    if (Property != NULL) {  /* If the property was found. */
      DevelopmentWindow[WindowNum]->Height = atoi(Property->Value);
    }

    for (Property = SelectedWindow->Property; Property != NULL && strcmp(Property->Name, "Name"); Property = Property->Next) {}
    if (Property != NULL) {  /* If the property was found. */
      strcpy(DevelopmentWindow[WindowNum]->Name, Property->Value);
    }

    for (Property = SelectedWindow->Property; Property != NULL && strcmp(Property->Name, "Class"); Property = Property->Next) {}
    if (Property != NULL) {  /* If the property was found. */
      strcpy(DevelopmentWindow[WindowNum]->Class, Property->Value);
    }

    for (Property = SelectedWindow->Property; Property != NULL && strcmp(Property->Name, "Resource"); Property = Property->Next) {}
    if (Property != NULL) {  /* If the property was found. */
      strcpy(DevelopmentWindow[WindowNum]->Resource, Property->Value);
    }

    for (Property = SelectedWindow->Property; Property != NULL && strcmp(Property->Name, "BackgroundColor"); Property = Property->Next) {}
    if (Property != NULL) {  /* If the property was found. */
      strcpy(DevelopmentWindow[WindowNum]->BackgroundColor, Property->Value);
    }

    /* During development we want the window to be destroyed when it */
    /* closes. In the users program it is up to them. */
    DevelopmentWindow[WindowNum]->CloseDestroysWindow = True;

    /* Better set this to false or OnyX will exit */
    /* as soon as they close the development window. ;) */
    DevelopmentWindow[WindowNum]->CloseExitsApplication = False;

    /* Add the grid. */
    if (ViewGridMenuItem->IsChecked) {
      strcpy(DevelopmentWindow[WindowNum]->Picture, "onyx_grid.xpm");
      strcpy(DevelopmentWindow[WindowNum]->Directory, OnyxIconLocation);
    }

    /* Update the mapping from DevelopmentWindows to ProjecttObjects. */
    WindowToListMap[WindowNum] = ProjectObject;

    DevelopmentWindow[WindowNum]->Create();  /* Create the window. */
  }
}


/* Create all development items for a window designated by DevelopmentWindow[CurrentWindow]. */

void CreateDevelopmentItems()
{
	struct ProjectObjectList *ProjectObject;

	if(SelectedWindow != NULL)
	{
		ProjectObject = ProjectObjects;

		while(ProjectObject != NULL)
		{
			if(!strcmp(ProjectObject->Parent,SelectedWindow->Name))
			{
				CreateDevelopmentItem(ProjectObject);
			}
			ProjectObject = ProjectObject->Next;
		}
	}

}


/* Create a single development item for a window designated by DevelopmentWindow[CurrentWindow]. */

void CreateDevelopmentItem(struct ProjectObjectList *ProjectObject)
{
	int i;

	struct ProjectObjectPropertyList *Property;

	if(SelectedWindow != NULL)
	{
		i = NextDevelopmentItemID;

		DevelopmentObject[i] = CreateDevelopmentObject(ProjectObject->Type);

		DevelopmentObject[i]->ID = NextDevelopmentItemID + DevelopmentItemIDModifier;
		DevelopmentObject[i]->Parent = DevelopmentWindow[CurrentDevelopmentWindow]->Self(); 

		/* This severely depends on incomplete AND determination! */

		for (Property = ProjectObject->Property; (Property != NULL) && (strcmp(Property->Name, "X")); Property = Property->Next)
		{}
		if(Property != NULL)
		{
			DevelopmentObject[i]->X = atoi(Property->Value);
		}

		for (Property = ProjectObject->Property; (Property != NULL) && (strcmp(Property->Name, "Y")); Property = Property->Next)
		{}
		if(Property != NULL)
		{
			DevelopmentObject[i]->Y = atoi(Property->Value);
		}

		for (Property = ProjectObject->Property; (Property != NULL) && (strcmp(Property->Name, "Width")); Property = Property->Next)
		{}
		if(Property != NULL)
		{
			DevelopmentObject[i]->Width = atoi(Property->Value);
		}

		for (Property = ProjectObject->Property; (Property != NULL) && (strcmp(Property->Name, "Height")); Property = Property->Next)
		{}
		if(Property != NULL)
		{
			DevelopmentObject[i]->Height = atoi(Property->Value);
		}

		for (Property = ProjectObject->Property; (Property != NULL) && (strcmp(Property->Name, "Text")); Property = Property->Next)
		{}
		if(Property != NULL)
		{
			strcpy(DevelopmentObject[i]->Text,Property->Value);
		}

		for (Property = ProjectObject->Property; (Property != NULL) && (strcmp(Property->Name, "Font")); Property = Property->Next)
		{}
		if(Property != NULL)
		{
			strcpy(DevelopmentObject[i]->Font,Property->Value);
		}

		for (Property = ProjectObject->Property; (Property != NULL) && (strcmp(Property->Name, "Picture")); Property = Property->Next)
		{}
		if(Property != NULL)
		{
			strcpy(DevelopmentObject[i]->Picture,Property->Value);
		}

		for (Property = ProjectObject->Property; (Property != NULL) && (strcmp(Property->Name, "Directory")); Property = Property->Next)
		{}
		if(Property != NULL)
		{
//==!==			strcpy(DevelopmentObject[i]->Directory,Property->Value);
		}

		for (Property = ProjectObject->Property; (Property != NULL) && (strcmp(Property->Name, "IsShaped")); Property = Property->Next)
		{}
		if(Property != NULL)
		{
			if(!strcmp(Property->Value, "False"))
			{
				DevelopmentObject[i]->IsShaped = 0;
			}
			else if(!strcmp(Property->Value, "True"))
			{
				DevelopmentObject[i]->IsShaped = 1;
			}
			else
			{
				DevelopmentObject[i]->IsShaped = atoi(Property->Value);
			}
		}

		for (Property = ProjectObject->Property; (Property != NULL) && (strcmp(Property->Name, "IsChecked")); Property = Property->Next)
		{}
		if(Property != NULL)
		{
			if(!strcmp(Property->Value, "False"))
			{
				DevelopmentObject[i]->IsChecked = 0;
			}
			else if(!strcmp(Property->Value, "True"))
			{
				DevelopmentObject[i]->IsChecked = 1;
			}
			else
			{
				DevelopmentObject[i]->IsChecked = atoi(Property->Value);
			}
		}

		for (Property = ProjectObject->Property; (Property != NULL) && (strcmp(Property->Name, "Size")); Property = Property->Next)
		{}
		if(Property != NULL)
		{
			DevelopmentObject[i]->Size = atoi(Property->Value);
		}

		for (Property = ProjectObject->Property; (Property != NULL) && (strcmp(Property->Name, "BorderStyle")); Property = Property->Next)
		{}
		if(Property != NULL)
		{
			if(!strcmp(Property->Value, "NoBorder"))
			{
				DevelopmentObject[i]->BorderStyle = NoBorder;
			}
			else if(!strcmp(Property->Value, "SunkenSingle"))
			{
				DevelopmentObject[i]->BorderStyle = SunkenSingle;
			}
			else if(!strcmp(Property->Value, "SunkenDouble"))
			{
				DevelopmentObject[i]->BorderStyle = SunkenDouble;
			}
			else if(!strcmp(Property->Value, "RaisedSingle"))
			{
				DevelopmentObject[i]->BorderStyle = RaisedSingle;
			}
			else if(!strcmp(Property->Value, "RaisedDouble"))
			{
				DevelopmentObject[i]->BorderStyle = RaisedDouble;
			}
			else
			{
				DevelopmentObject[i]->BorderStyle = atoi(Property->Value);
			}
		}

		for (Property = ProjectObject->Property; (Property != NULL) && (strcmp(Property->Name, "VerticalAlignment")); Property = Property->Next)
		{}
		if(Property != NULL)
		{
			if(!strcmp(Property->Value, "AlignTop"))
			{
				DevelopmentObject[i]->VerticalAlignment = AlignTop;
			}
			else if(!strcmp(Property->Value, "AlignCenter"))
			{
				DevelopmentObject[i]->VerticalAlignment = AlignCenter;
			}
			else if(!strcmp(Property->Value, "AlignBottom"))
			{
				DevelopmentObject[i]->VerticalAlignment = AlignBottom;
			}
			else
			{
				DevelopmentObject[i]->VerticalAlignment = atoi(Property->Value);
			}
		}

		for (Property = ProjectObject->Property; (Property != NULL) && (strcmp(Property->Name, "HorizontalAlignment")); Property = Property->Next)
		{}
		if(Property != NULL)
		{
			if(!strcmp(Property->Value, "AlignLeft"))
			{
				DevelopmentObject[i]->HorizontalAlignment = AlignLeft;
			}
			else if(!strcmp(Property->Value, "AlignCenter"))
			{
				DevelopmentObject[i]->HorizontalAlignment = AlignCenter;
			}
			else if(!strcmp(Property->Value, "AlignRight"))
			{
				DevelopmentObject[i]->HorizontalAlignment = AlignRight;
			}
			else
			{
				DevelopmentObject[i]->HorizontalAlignment = atoi(Property->Value);
			}
		}

		DevelopmentObject[i]->Create();
		i++;

		DevelopmentItem[NextDevelopmentItemID] = ProjectObject;
		NextDevelopmentItemID = (NextDevelopmentItemID++) % DevelopmentItemIDMax;
	}
}


/* Delete all objects on DevelopmentWindow[CurrentDevelopmentWindow] */

void DestroyDevelopmentObjects()
{
	int i;

	for (i = 0; i < MaxDevelopmentObjects; i++)
	{
		if(DevelopmentWindow[i] != NULL && (i < 50 || i > 60))
		{
			if (DevelopmentObject[i]->Parent == DevelopmentWindow[CurrentDevelopmentWindow]->Self())
			{
				delete DevelopmentObject[i];
				DevelopmentObject[i] = NULL;
			}
		}
	}
}


void UpdateDevelopmentObject(int ObjectToUpdate, struct ProjectObjectPropertyList *Property)
{
	if(DevelopmentObject[ObjectToUpdate] != NULL)
	{
		if(Property != NULL)
		{
			if(!strcmp(Property->Name,"X"))
			{
				DevelopmentObject[ObjectToUpdate]->X = atoi(Property->Value);
				DevelopmentObject[ObjectToUpdate]->Move(DevelopmentObject[ObjectToUpdate]->X,DevelopmentObject[ObjectToUpdate]->Y);
				MoveBrackets(DevelopmentObject[ObjectToUpdate]->X,DevelopmentObject[ObjectToUpdate]->Y,DevelopmentObject[ObjectToUpdate]->Width,DevelopmentObject[ObjectToUpdate]->Height);
			}
			else if(!strcmp(Property->Name,"Y"))
			{
				DevelopmentObject[ObjectToUpdate]->Y = atoi(Property->Value);
				DevelopmentObject[ObjectToUpdate]->Move(DevelopmentObject[ObjectToUpdate]->X,DevelopmentObject[ObjectToUpdate]->Y);
				MoveBrackets(DevelopmentObject[ObjectToUpdate]->X,DevelopmentObject[ObjectToUpdate]->Y,DevelopmentObject[ObjectToUpdate]->Width,DevelopmentObject[ObjectToUpdate]->Height);
			}
			else if(!strcmp(Property->Name,"Width"))
			{
				DevelopmentObject[ObjectToUpdate]->Width = atoi(Property->Value);
				DevelopmentObject[ObjectToUpdate]->Resize(DevelopmentObject[ObjectToUpdate]->Width,DevelopmentObject[ObjectToUpdate]->Height);
				MoveBrackets(DevelopmentObject[ObjectToUpdate]->X,DevelopmentObject[ObjectToUpdate]->Y,DevelopmentObject[ObjectToUpdate]->Width,DevelopmentObject[ObjectToUpdate]->Height);
			}
			else if(!strcmp(Property->Name,"Height"))
			{
				DevelopmentObject[ObjectToUpdate]->Height = atoi(Property->Value);
				DevelopmentObject[ObjectToUpdate]->Resize(DevelopmentObject[ObjectToUpdate]->Width,DevelopmentObject[ObjectToUpdate]->Height);
				MoveBrackets(DevelopmentObject[ObjectToUpdate]->X,DevelopmentObject[ObjectToUpdate]->Y,DevelopmentObject[ObjectToUpdate]->Width,DevelopmentObject[ObjectToUpdate]->Height);
			}
			else if(!strcmp(Property->Name,"Text"))
			{
				strcpy(DevelopmentObject[ObjectToUpdate]->Text,Property->Value);
				DevelopmentObject[ObjectToUpdate]->Update();
			}
			else if(!strcmp(Property->Name,"Font"))
			{
				strcpy(DevelopmentObject[ObjectToUpdate]->Font,Property->Value);
				DevelopmentObject[ObjectToUpdate]->Update();
			}
			else if(!strcmp(Property->Name,"Picture"))
			{
				strcpy(DevelopmentObject[ObjectToUpdate]->Picture,Property->Value);
				DevelopmentObject[ObjectToUpdate]->Update();
			}
			else if(!strcmp(Property->Name,"Directory"))
			{
//==!==				strcpy(DevelopmentObject[ObjectToUpdate]->Directory,Property->Value);
				DevelopmentObject[ObjectToUpdate]->Update();
			}
			else if(!strcmp(Property->Name,"IsShaped"))
			{
				if(!strcmp(Property->Value,"False"))
				{
					DevelopmentObject[ObjectToUpdate]->IsShaped = 0;
				}
				else if(!strcmp(Property->Value,"True"))
				{
					DevelopmentObject[ObjectToUpdate]->IsShaped = 1;
				}
				else
				{
					DevelopmentObject[ObjectToUpdate]->IsShaped = atoi(Property->Value);
				}

				DevelopmentObject[ObjectToUpdate]->Update();
			}
			else if(!strcmp(Property->Name,"IsChecked"))
			{
				if(!strcmp(Property->Value,"False"))
				{
					DevelopmentObject[ObjectToUpdate]->IsChecked = 0;
				}
				else if(!strcmp(Property->Value,"True"))
				{
					DevelopmentObject[ObjectToUpdate]->IsChecked = 1;
				}
				else
				{
					DevelopmentObject[ObjectToUpdate]->IsChecked = atoi(Property->Value);
				}

				DevelopmentObject[ObjectToUpdate]->Update();
			}
			else if(!strcmp(Property->Name,"BorderStyle"))
			{
				if(!strcmp(Property->Value,"NoBorder"))
				{
					DevelopmentObject[ObjectToUpdate]->BorderStyle = NoBorder;
				}
				else if(!strcmp(Property->Value,"SunkenSingle"))
				{
					DevelopmentObject[ObjectToUpdate]->BorderStyle = SunkenSingle;
				}
				else if(!strcmp(Property->Value,"SunkenDouble"))
				{
					DevelopmentObject[ObjectToUpdate]->BorderStyle = SunkenDouble;
				}
				else if(!strcmp(Property->Value,"RaisedSingle"))
				{
					DevelopmentObject[ObjectToUpdate]->BorderStyle = RaisedSingle;
				}
				else if(!strcmp(Property->Value,"RaisedDouble"))
				{
					DevelopmentObject[ObjectToUpdate]->BorderStyle = RaisedDouble;
				}
				else
				{
					DevelopmentObject[ObjectToUpdate]->BorderStyle = atoi(Property->Value);
				}

				DevelopmentObject[ObjectToUpdate]->Update();
			}
			else if(!strcmp(Property->Name,"VerticalAlignment"))
			{
				if(!strcmp(Property->Value,"AlignTop"))
				{
					DevelopmentObject[ObjectToUpdate]->VerticalAlignment = AlignTop;
				}
				else if(!strcmp(Property->Value,"AlignCenter"))
				{
					DevelopmentObject[ObjectToUpdate]->VerticalAlignment = AlignCenter;
				}
				else if(!strcmp(Property->Value,"AlignBottom"))
				{
					DevelopmentObject[ObjectToUpdate]->VerticalAlignment = AlignBottom;
				}
				else
				{
					DevelopmentObject[ObjectToUpdate]->VerticalAlignment = atoi(Property->Value);
				}

				DevelopmentObject[ObjectToUpdate]->Update();
			}
			else if(!strcmp(Property->Name,"HorizontalAlignment"))
			{
				if(!strcmp(Property->Value,"AlignLeft"))
				{
					DevelopmentObject[ObjectToUpdate]->HorizontalAlignment = AlignLeft;
				}
				else if(!strcmp(Property->Value,"AlignCenter"))
				{
					DevelopmentObject[ObjectToUpdate]->HorizontalAlignment = AlignCenter;
				}
				else if(!strcmp(Property->Value,"AlignRight"))
				{
					DevelopmentObject[ObjectToUpdate]->HorizontalAlignment = AlignRight;
				}
				else
				{
					DevelopmentObject[ObjectToUpdate]->HorizontalAlignment = atoi(Property->Value);
				}

				DevelopmentObject[ObjectToUpdate]->Update();
			}


		}
	}
}

void UpdateDevelopmentWindow(int WindowToUpdate, struct ProjectObjectPropertyList *Property)
{
	if(DevelopmentWindow[WindowToUpdate] != NULL)
	{
		if(Property != NULL)
		{
			if(!strcmp(Property->Name,"X"))
			{
				DevelopmentWindow[WindowToUpdate]->X = atoi(Property->Value);
				DevelopmentWindow[WindowToUpdate]->Move(DevelopmentWindow[WindowToUpdate]->X,DevelopmentWindow[WindowToUpdate]->Y);
			}
			else if(!strcmp(Property->Name,"Y"))
			{
				DevelopmentWindow[WindowToUpdate]->Y = atoi(Property->Value);
				DevelopmentWindow[WindowToUpdate]->Move(DevelopmentWindow[WindowToUpdate]->X,DevelopmentWindow[WindowToUpdate]->Y);
			}
			else if(!strcmp(Property->Name,"Width"))
			{
				DevelopmentWindow[WindowToUpdate]->Width = atoi(Property->Value);
				DevelopmentWindow[WindowToUpdate]->Resize(DevelopmentWindow[WindowToUpdate]->Width,DevelopmentWindow[WindowToUpdate]->Height);
			}
			else if(!strcmp(Property->Name,"Height"))
			{
				DevelopmentWindow[WindowToUpdate]->Height = atoi(Property->Value);
				DevelopmentWindow[WindowToUpdate]->Resize(DevelopmentWindow[WindowToUpdate]->Width,DevelopmentWindow[WindowToUpdate]->Height);
			}
			else if(!strcmp(Property->Name,"Name"))
			{
				strcpy(DevelopmentWindow[WindowToUpdate]->Name,Property->Value);
				DevelopmentWindow[WindowToUpdate]->Update();
			}
			else if(!strcmp(Property->Name,"Class"))
			{
				strcpy(DevelopmentWindow[WindowToUpdate]->Class,Property->Value);
				DevelopmentWindow[WindowToUpdate]->Update();
			}
			else if(!strcmp(Property->Name,"Resource"))
			{
				strcpy(DevelopmentWindow[WindowToUpdate]->Resource,Property->Value);
				DevelopmentWindow[WindowToUpdate]->Update();
			}
		}
	}
}


void CreateBrackets() {
  TLCorner = new OnyxIcon;
  TRCorner = new OnyxIcon;
  BLCorner = new OnyxIcon;
  BRCorner = new OnyxIcon;

  TLCorner->ID = TLCornerID;
  TRCorner->ID = TRCornerID;
  BLCorner->ID = BLCornerID;
  BRCorner->ID = BRCornerID;

  TLCorner->Parent = DevelopmentWindow[CurrentDevelopmentWindow]->Self();
  TRCorner->Parent = DevelopmentWindow[CurrentDevelopmentWindow]->Self();
  BLCorner->Parent = DevelopmentWindow[CurrentDevelopmentWindow]->Self();
  BRCorner->Parent = DevelopmentWindow[CurrentDevelopmentWindow]->Self();

  /* Start off screen until first object selected. */
  TLCorner->X = -20;
  TRCorner->X = -20;
  BLCorner->X = -20;
  BRCorner->X = -20;

  TLCorner->Y = -20;
  TRCorner->Y = -20;
  BLCorner->Y = -20;
  BRCorner->Y = -20;

  TLCorner->Width = 7;
  TRCorner->Width = 7;
  BLCorner->Width = 7;
  BRCorner->Width = 7;

  TLCorner->Height = 7;
  TRCorner->Height = 7;
  BLCorner->Height = 7;
  BRCorner->Height = 7;

  TLCorner->IsShaped = True;
  TRCorner->IsShaped = True;
  BLCorner->IsShaped = True;
  BRCorner->IsShaped = True;

  strcpy(TLCorner->Picture, "onyx_tlcorner.xpm");
  strcpy(TRCorner->Picture, "onyx_trcorner.xpm");
  strcpy(BLCorner->Picture, "onyx_blcorner.xpm");
  strcpy(BRCorner->Picture, "onyx_brcorner.xpm");

  TLCorner->Create();
  TRCorner->Create();
  BLCorner->Create();
  BRCorner->Create();
}

void MoveBrackets(int X, int Y, int Width, int Height) {

  /* If the brackets are non existant then create them. */
  if (TLCorner == NULL) {
    CreateBrackets();
  }

  /* Now move them around the object specs given. */
  TLCorner->Move(X - 1, Y - 1);
  TRCorner->Move(X + Width - 7 + 1, Y - 1);
  BLCorner->Move(X - 1, Y + Height - 7 + 1);
  BRCorner->Move(X + Width - 7 + 1, Y + Height - 7 + 1);
}

void DestroyBrackets() {
  if (TLCorner) delete TLCorner;
  if (TRCorner) delete TRCorner;
  if (BLCorner) delete BLCorner;
  if (BRCorner) delete BRCorner;

  TLCorner = NULL;
  TRCorner = NULL;
  BLCorner = NULL;
  BRCorner = NULL;
}

void SetMainWindowButtonStatus()
{
	if(ValidProject)
	{
		SaveButton->Enable();
		SaveProjectButton->Enable();
		NewWindowButton->Enable();
		ViewWindowButton->Enable();
		NextWindowButton->Enable();
		MakeButton->Enable();
		MakeRunButton->Enable();
		RunButton->Enable();
		StopButton->Enable();
		ShowCodeButton->Enable();
		DeleteControlButton->Enable();
		LockControlsButton->Enable();
		SnapToGrid->Enable();
		ShowPropertiesButton->Enable();
		ShowToolboxButton->Enable();
		ProjectExplorerButton->Enable();
	}
	else
	{
		SaveButton->Disable();
		SaveProjectButton->Disable();
		NewWindowButton->Disable();
		ViewWindowButton->Disable();
		NextWindowButton->Disable();
		MakeButton->Disable();
		MakeRunButton->Disable();
		RunButton->Disable();
		StopButton->Disable();
		ShowCodeButton->Disable();
		DeleteControlButton->Disable();
		LockControlsButton->Disable();
		SnapToGrid->Disable();
		ShowPropertiesButton->Disable();
		ShowToolboxButton->Disable();
		ProjectExplorerButton->Disable();
	}
}

void SetMainWindowMenuStatus()
{
	if(ValidProject)
	{
		SaveObjectMenuItem->Enable();
		SaveProjectMenuItem->Enable();
		SaveProjectAsMenuItem->Enable();
		CreateApplicationMenuItem->Enable();
		ViewCodeMenuItem->Enable();
		ViewWindowMenuItem->Enable();
		ViewProjectMenuItem->Enable();
		ViewPropertiesMenuItem->Enable();
		ToolBoxMenuItem->Enable();
		MakeMenuItem->Enable();
		MakeCleanMenuItem->Enable();
		MakeRunMenuItem->Enable();
		RunMenuItem->Enable();
		StopMenuItem->Enable();
		MenuEditorItem->Enable();
		CustomControlsItem->Enable();
		SnapToGridMenuItem->Enable();
		ExportObjectMenuItem->Enable();
		ImportObjectMenuItem->Enable();
		ExportProjectMenuItem->Enable();
	}
	else
	{
		SaveObjectMenuItem->Disable();
		SaveProjectMenuItem->Disable();
		SaveProjectAsMenuItem->Disable();
		CreateApplicationMenuItem->Disable();
		ViewCodeMenuItem->Disable();
		ViewWindowMenuItem->Disable();
		ViewProjectMenuItem->Disable();
		ViewPropertiesMenuItem->Disable();
		ToolBoxMenuItem->Disable();
		MakeMenuItem->Disable();
		MakeCleanMenuItem->Disable();
		MakeRunMenuItem->Disable();
		RunMenuItem->Disable();
		StopMenuItem->Disable();
		MenuEditorItem->Disable();
		CustomControlsItem->Disable();
		SnapToGridMenuItem->Disable();
		ExportObjectMenuItem->Disable();
		ImportObjectMenuItem->Disable();
		ExportProjectMenuItem->Disable();
	}
}


void InitializeInternalProjectFile(char *ProjectFileName)
{
	struct ObjectList *UsableObject;
	struct ProjectObjectList *ProjectObject;
	struct ProjectObjectPropertyList *ObjectProperty;
	char temp[255];


	if(InternalOxpCode != NULL)	/* Remove any previous instance. */
	{
		InternalOxpCode->Close();
		delete InternalOxpCode;
	}

	sprintf(temp,"../%s",ProjectFileName);	/* Project file is in the parent directory. */
	InternalOxpCode = new InternalCodeFile(temp,NULL,NULL);
	InternalOxpCode->CodeStyle = 5;
	InternalOxpCode->HasBeenModified = True;

	/* Add that nifty header to make a project launch onyx itself ;) */
	InternalOxpCode->Append("#!%s/OnyX\n", OnyxLocation);

	/* Save OnyX version this project was developed with. */
	InternalOxpCode->Append("OnyXVersion %i.%i.%i\n", MajorVersionNumber, MinorVersionNumber, ReleaseVersionNumber);

	/* Save name of application. */
	InternalOxpCode->Append("ApplicationName %s\n", ProjectApplicationName);

	/* Save toolbar information. */
	for (UsableObject = UsableObjects; UsableObject != NULL; UsableObject = UsableObject->Next)
	{
		/* Save tools needed. */
		InternalOxpCode->Append("ToolboxItem %s\n", UsableObject->Name);
	}

	/* Save the individual objects. */
	ProjectObject = ProjectObjects;

	while(ProjectObject != NULL)
	{
		if(ProjectObject->IsIncorporated)	/* Only if the object has ever been saved before. */
		{
			InternalOxpCode->Append("StartObject\n");
			InternalOxpCode->Append("ObjectName %s\n", ProjectObject->Name);
			InternalOxpCode->Append("ObjectType %s\n", ProjectObject->Type);
			InternalOxpCode->Append("ObjectParent %s\n", ProjectObject->Parent);

			/* Save properties of the current object. */
			ObjectProperty = ProjectObject->Property;
			while(ObjectProperty != NULL)
			{
				InternalOxpCode->Append("%s %s\n", ObjectProperty->Name, ObjectProperty->Value);
				ObjectProperty = ObjectProperty->Next;
			}
			InternalOxpCode->Append("EndObject\n");
		}

		ProjectObject = ProjectObject->Next;
	}
}


void InitializeInternalApplication()
{
	InternalCodeFile *InternalFile;
	char temp[255];

	strcpy(temp, "Application");	/* This is the application.oxp */

	/* Remove any previous instance. */
	InternalFile = InternalOxcCode;
	while (InternalFile != NULL)
	{
		if (!strcmp(InternalFile->ObjectName, temp) && !InternalFile->HasBeenDeleted)	/* If this is the previous instance. */
		{
			InternalFile->Close();
			if (InternalOxcCode == InternalFile)
			{
				InternalOxcCode = InternalFile->Next;
			}
			delete InternalFile;
			InternalFile = NULL;
		}
		else
		{
			InternalFile = InternalFile->Next;	/* Go to next internal file. */
		}
	}

	/* Yep, thats right, we are building the list backwards. It saves a pointer. */
	/* Of course non-init code won't know the difference. */
	InternalOxcCode = new InternalCodeFile(temp, NULL, InternalOxcCode);
	InternalFile = InternalOxcCode;

	InternalFile->Append("void ApplicationPreload(int argc, char *argv[])\n{\n}\n\n");
	InternalFile->Append("void ApplicationLoad(int argc, char *argv[])\n");
	InternalFile->Append("{\n\tOpenStartingForm();\n}\n\n");
	InternalFile->Append("void GeneralEvent(OnyxMessage *event)\n{\n}\n");

	InternalFile->HasBeenModified = True;
	InternalFile->IsIncorporated = False;
}

void InitializeInternalMakefile()
{
	InternalCodeFile *InternalFile;

	struct ObjectList *UsedObject;
	struct ProjectObjectList *ProjectObject;
	char temp[255];


	if (InternalMakefile != NULL)	/* Remove any previous instance. */
	{
		InternalMakefile->Close();
		delete InternalMakefile;
	}

	strcpy(temp, "Makefile");	 /* This is the main Makefile */
	InternalMakefile = new InternalCodeFile(temp, NULL, NULL);
	InternalFile = InternalMakefile;

	InternalFile->CodeStyle = 4;
	InternalFile->HasBeenModified = True;

	InternalFile->Append("# Generated automatically by OnyX. Someday this will be done with autoconf.\n");

	/* Write top part of the make file. */
	InternalFile->Append("\n");
	InternalFile->Append("SHELL=/bin/sh\n");
	InternalFile->Append("\n");
	InternalFile->Append("CXX=c++\n");
	InternalFile->Append("CXXFLAGS=`xc-config --cflags` -g\n");
	InternalFile->Append("LIBS=`xc-config --libs`\n");
	InternalFile->Append("LDFLAGS=\n");
	InternalFile->Append("INSTALL=install -c\n");
	InternalFile->Append("BINDIR=`xc-config --exec-prefix`/bin\n");
	InternalFile->Append("MAKEDEPEND=@MAKEDEPEND@\n");
	InternalFile->Append("\n");

	InternalFile->Append("# Rules for compiling .cc sources\n");
	InternalFile->Append(".SUFFIX: .cc .o\n");
	InternalFile->Append(".cc.o:\n");
	InternalFile->Append("\t@echo Compiling $<\n");
	InternalFile->Append("\t@$(CXX) $(CXXFLAGS) -c $<\n");
	InternalFile->Append("\n");

	InternalFile->Append("ob=./.onyx/\n");

	/* Add the object types to the onyxo= line. */
	UsedObject = UsableObjects;

	InternalFile->Append("onyxo = ");

	while (UsedObject != NULL)
	{
		InternalFile->Append("${ob}%s.o", UsedObject->Name);
		UsedObject = UsedObject->Next;
		if (UsedObject) InternalFile->Append(" \\\n\t");
	}
	InternalFile->Append("\n");
	InternalFile->Append("\n");

	/* Add the objects of the project to the SRCS= line. */
	ProjectObject = ProjectObjects;

	/* These are the minimal neccessary additions. */
	InternalFile->Append("SRCS = main.cc \\\n\tApplication.cc");

	/* Add the window.cc files. These contain all the objects attached to them already. */
	while (ProjectObject != NULL)
	{
		if (!strcmp(ProjectObject->Type, "OnyxWindow") && ProjectObject->IsIncorporated)
		{
			InternalFile->Append(" \\\n\t%s.cc", ProjectObject->Name);
		}
		ProjectObject = ProjectObject->Next;
	}
	InternalFile->Append("\n\n");
	InternalFile->Append("OBJS = $(SRCS:.cc=.o)\n");

	InternalFile->Append("\n");
	InternalFile->Append("PROG = %s\n", ProjectApplicationName);
	InternalFile->Append("\n");
	InternalFile->Append("all: complete\n");
	InternalFile->Append("\n");
	InternalFile->Append("${PROG}: ${OBJS}\n");
	InternalFile->Append("\t@echo Linking all objects\n");
	InternalFile->Append("\t@$(CXX) -o $@ ${OBJS} ${onyxo} $(LDFLAGS) $(LIBS) $(EXTRALIBS)\n");
	InternalFile->Append("\t@echo Stripping extra symbols from the $@ executable\n");
	InternalFile->Append("\t@strip $@\n");
	InternalFile->Append("\t@echo $@ is ready.\n");
	InternalFile->Append("\n");
	InternalFile->Append("complete: base ${PROG}\n");
	InternalFile->Append("\n");
	InternalFile->Append("base: \n");
	InternalFile->Append("\t@for i in ${ob}; do \\\n");
	InternalFile->Append("\t\techo Making Onyx Base ; \\\n");
	InternalFile->Append("\t\t(cd $$i; ${MAKE} all); \\\n");
	InternalFile->Append("\tdone\n");
	InternalFile->Append("\n");
	InternalFile->Append("depend:\n");
	InternalFile->Append("\t${MAKEDEPEND} ${CXXFLAGS} ${SRCS}\n");
	InternalFile->Append("\n");
	InternalFile->Append("install: \n");
	InternalFile->Append("\n");
	InternalFile->Append("clean:\n");
	InternalFile->Append("\t${RM} ${OBJS} $(PROG) *~ core *.bak\n");
	InternalFile->Append("\n");
	InternalFile->Append("baseclean:\n");
	InternalFile->Append("\t@for i in ${ob}; do \\\n");
	InternalFile->Append("\t\techo Making Onyx Base ; \\\n");
	InternalFile->Append("\t\t(cd $$i; ${MAKE} clean); \\\n");
	InternalFile->Append("\tdone\n");
	InternalFile->Append("\n");
	InternalFile->Append("completeclean: baseclean clean\n");
	InternalFile->Append("\n");
	InternalFile->Append("distclean: baseclean\n");
	InternalFile->Append("\n");
	InternalFile->Append("allclean: completeclean\n");

}

void InitializeInternalBaseMakefile()
{
	InternalCodeFile *InternalFile;
	ObjectList *Object;

	char temp[255];

	if(InternalBaseMakefile != NULL)	/* Remove any previous instance. */
	{
		InternalBaseMakefile->Close();
		delete InternalBaseMakefile;
	}

	strcpy(temp, ".onyx/Makefile");	/* This is the Base Makefile */
	InternalBaseMakefile = new InternalCodeFile(temp, NULL, NULL);
	InternalFile = InternalBaseMakefile;

	InternalFile->CodeStyle = 5;	/* General file code style is 5 */
	InternalFile->HasBeenModified = True;

	InternalFile->Append("# Generated automatically by OnyX. Someday this will be done with autoconf.\n");

	/* Write top part of the make file. */
	InternalFile->Append("\n");
	InternalFile->Append("SHELL=/bin/sh\n");
	InternalFile->Append("\n");
	InternalFile->Append("CXX=c++\n");
	InternalFile->Append("CXXFLAGS=`xc-config --cflags` -g\n");
	InternalFile->Append("LIBS=`xc-config --libs`\n");
	InternalFile->Append("LDFLAGS=\n");
	InternalFile->Append("INSTALL=install -c\n");
	InternalFile->Append("BINDIR=`xc-config --exec-prefix`/bin\n");
	InternalFile->Append("MAKEDEPEND=@MAKEDEPEND@\n");
	InternalFile->Append("\n");

	InternalFile->Append("# Rules for compiling .cc sources\n");
	InternalFile->Append(".SUFFIX: .cc .o\n");
	InternalFile->Append(".cc.o:\n");
	InternalFile->Append("\t@echo Compiling $<\n");
	InternalFile->Append("\t@$(CXX) $(CXXFLAGS) -c $<\n");
	InternalFile->Append("\n");

	InternalFile->Append("SRCS = ");

	for (Object = UsableObjects; Object != NULL; Object = Object->Next)
	{
		InternalFile->Append("%s.cc", Object->Name);
		if (Object->Next) InternalFile->Append(" \\\n\t");
	}
	InternalFile->Append("\n\n");
	InternalFile->Append("OBJS = $(SRCS:.cc=.o)\n");

	InternalFile->Append("\n");
	InternalFile->Append("all: onyx\n");
	InternalFile->Append("\n");
	InternalFile->Append("depend:\n");
	InternalFile->Append("\t${MAKEDEPEND} ${CXXFLAGS} ${SRCS}\n");
	InternalFile->Append("\n");
	InternalFile->Append("onyx: ${OBJS}\n");
	InternalFile->Append("\n");
	InternalFile->Append("install:\n");
	InternalFile->Append("\n");
	InternalFile->Append("clean:\n");
	InternalFile->Append("\t${RM} ${OBJS} *~ core *.bak\n");
	InternalFile->Append("\n");
	InternalFile->Append("distclean: clean\n");
}

void InitializeInternalEvents_h()
{
	InternalCodeFile *InternalFile;
	char temp[255];

	struct ProjectObjectList *ProjectObject;
	struct ObjectList *UsableObject;
	struct ObjectEvent *UsableEvent;

	strcpy(temp, "Events");	/* This is the Events.h */

	/* Remove any previous instance. */
	InternalFile = InternalHeader;
	while(InternalFile != NULL)
	{
		if(!strcmp(InternalFile->ObjectName,temp))	/* If this is the previous instance. */
		{
			InternalFile->Close();
			if (InternalHeader == InternalFile)
			{
				InternalHeader = InternalFile->Next;
			}
			delete InternalFile;
			InternalFile = NULL;
		}
		else
		{
			InternalFile = InternalFile->Next;	/* Go to next internal file. */
		}
	}


	/* Yep, thats right, we are building the list backwards. It saves a pointer. */
	/* Of course non-init code won't know the difference. */
	InternalHeader = new InternalCodeFile(temp,NULL,InternalHeader);
	InternalFile = InternalHeader;

	InternalFile->CodeStyle = 2;	/* This is a type 2 or "Header Style" file. */
	InternalFile->HasBeenModified = True;


	/* Declare the OpenStartingForm() event. */
	InternalFile->Append("extern void OpenStartingForm();\n");

	/* Cycle through all of the objects and record their possible events. */
	ProjectObject = ProjectObjects;

	while(ProjectObject != NULL)
	{
		if(ProjectObject->IsIncorporated)	/* Only if this object has ever been saved before. */
		{
			/* Locate the UsableObject description so we know what events this ProjectObject has. */
			UsableObject = UsableObjects;
			while(strcmp(UsableObject->Name,ProjectObject->Type))
			{
				UsableObject = UsableObject->Next;
			}

			/* Now that it has been found cycle through the events. */
			UsableEvent = UsableObject->Event;

			while(UsableEvent != NULL)
			{
				/* Write the name of the event. */
				InternalFile->Append("extern void %s_%s(", ProjectObject->Name, UsableEvent->Name);

				/* If there is a first parameter... */
				if(strlen(UsableEvent->Parm1) > 0)
				{
					/* Write it. */
					InternalFile->Append("long %s",UsableEvent->Parm1);

					/* If there is a second parameter... */
					if(strlen(UsableEvent->Parm2) > 0)
					{
						/* Write it. */
						InternalFile->Append(", long %s",UsableEvent->Parm2);

						/* If there is a third parameter... */
						if(strlen(UsableEvent->Parm3) > 0)
						{
							/* Write it... */
							InternalFile->Append(", long %s",UsableEvent->Parm3);

							/* If there is a fourth parameter... */
							if(strlen(UsableEvent->Parm4) > 0)
							{
								/* Write it. */
								InternalFile->Append(", long %s",UsableEvent->Parm4);
							}
						}
					}
				}

				/* Close off the parameter list of the function. */
				InternalFile->Append(");\n");

				/* Cycle to the next event for this ProjectObject. */
				UsableEvent = UsableEvent->Next;
			}

		}
		/* Cycle to the next ProjectObject contained in the users project. */
		ProjectObject = ProjectObject->Next;
	}
}

void InitializeInternalObjects_h()
{
	InternalCodeFile *InternalFile;
	char temp[255];
	struct ProjectObjectList *ProjectObject;

	strcpy(temp,"Objects");	/* This is the Objects.h */

	/* Remove any previous instance. */
	InternalFile = InternalHeader;
	while(InternalFile != NULL)
	{
		if(!strcmp(InternalFile->ObjectName,temp))	/* If this is the previous instance. */
		{
			InternalFile->Close();
			if (InternalHeader == InternalFile)
			{
				InternalHeader = InternalFile->Next;
			}
			delete InternalFile;
			InternalFile = NULL;
		}
		else
		{
			InternalFile = InternalFile->Next;	/* Go to next internal file. */
		}
	}

	/* Yep, thats right, we are building the list backwards. It saves a pointer. */
	/* Of course non-init code won't know the difference. */
	InternalHeader = new InternalCodeFile(temp,NULL,InternalHeader);
	InternalFile = InternalHeader;

	InternalFile->CodeStyle = 2;	/* This is a type 2 or "Header Style" file. */
	InternalFile->HasBeenModified = True;

	/* Add the minimal line of #include "Includes.h". This allows for the object types to be identified. */
	InternalFile->Append("#include \"Includes.h\"\n\n");

	/* Now cycle thorugh the object and add the exten refrences so that everyone can see them. */
	/* Remember, the actual objects and their IDs are in main.h. */
	ProjectObject = ProjectObjects;

	while(ProjectObject != NULL)
	{
		if(ProjectObject->IsIncorporated)	/* Only if this object has ever been saved before. */
		{
			InternalFile->Append("extern %s *%s;\n", ProjectObject->Type, ProjectObject->Name);
		}
		ProjectObject = ProjectObject->Next;
	}
	
}

void InitializeInternalIncludes_h()
{
	InternalCodeFile *InternalFile;
	char temp[255];
	struct ObjectList *UsableObject;
	

	strcpy(temp, "Includes");	/* This is the Includes.h */

	/* Remove any previous instance. */
	InternalFile = InternalHeader;
	while(InternalFile != NULL)
	{
		if(!strcmp(InternalFile->ObjectName,temp))	/* If this is the previous instance. */
		{
			InternalFile->Close();
			if (InternalHeader == InternalFile)
			{
				InternalHeader = InternalFile->Next;
			}
			delete InternalFile;
			InternalFile = NULL;
		}
		else
		{
			InternalFile = InternalFile->Next;	/* Go to next internal file. */
		}
	}

	/* Yep, thats right, we are building the list backwards. It saves a pointer. */
	/* Of course non-init code won't know the difference. */
	InternalHeader = new InternalCodeFile(temp,NULL,InternalHeader);

	InternalFile = InternalHeader;

	InternalFile->CodeStyle = 2;	/* This is a type 2 or "Header Style" file. */
	InternalFile->HasBeenModified = True;

	/* Cycle through the usable object types and write them into the Includes.h */
	UsableObject = UsableObjects;

	while(UsableObject != NULL)
	{
		InternalFile->Append("#include \".onyx/%s.h\"\n", UsableObject->Name);
		UsableObject = UsableObject->Next;
	}
}

void InitializeInternalMain_h()
{
	InternalCodeFile *InternalFile;
	char temp[255];
	struct ProjectObjectList *ProjectObject;
	int i;

	strcpy(temp, "main");	/* This is the main.h */

	/* Remove any previous instance. */
	InternalFile = InternalHeader;
	while(InternalFile != NULL)
	{
		if(!strcmp(InternalFile->ObjectName,temp))	/* If this is the previous instance. */
		{
			InternalFile->Close();
			if (InternalHeader == InternalFile)
			{
				InternalHeader = InternalFile->Next;
			}
			delete InternalFile;
			InternalFile = NULL;
		}
		else
		{
			InternalFile = InternalFile->Next;	/* Go to next internal file. */
		}
	}

	/* Yep, thats right, we are building the list backwards. It saves a pointer. */
	/* Of course non-init code won't know the difference. */
	InternalHeader = new InternalCodeFile(temp,NULL,InternalHeader);
	InternalFile = InternalHeader;

	InternalFile->CodeStyle = 2;	/* This is a type 2 or "Header Style" file. */
	InternalFile->HasBeenModified = True;

	/* Add the first two lines which are #include "Includes.h" and #include "Events.h" */
	/* This will make all events available to the main.cc as well as making sure it knows of each object type. */
	InternalFile->Append("#include \"Includes.h\"\n");
	InternalFile->Append("#include \"Events.h\"\n\n");
	
	/* This is where the Application name is defined. Not that it can not be changed at run time. */
	/* What does everyone thing about that? Should it be a variable instead? */
	InternalFile->Append("#define ApplicationName \"%s\"\n\n", ProjectApplicationName);

	/* Now cycle through the objects defining the IDs and the actual objects with their types. */
	ProjectObject = ProjectObjects;

	i = 1;

	while(ProjectObject != NULL)
	{
		if(ProjectObject->IsIncorporated)	/* Only if this object has ever been saved before. */
		{
			/* The ID line. */
			InternalFile->Append("#define %sID %i\n", ProjectObject->Name, i++);

			/* The "ObjectType ObjectName;" line. */
			InternalFile->Append("%s *%s;\n\n", ProjectObject->Type, ProjectObject->Name);
		}

		ProjectObject = ProjectObject->Next;
	}
}

void InitializeInternalMain_cc()
{
	InternalCodeFile *InternalFile;

	struct ProjectObjectList *ProjectObject;
	struct ProjectObjectPropertyList *ObjectProperty;
	struct ObjectList *UsableObject;
	struct ObjectProperty *UsableProperty;
	struct ObjectEvent *UsableEvent;
	char *Marker;
	char temp[255];

	if(InternalMaincc != NULL)	/* Remove any previous instance. */
	{
		InternalMaincc->Close();
		delete InternalMaincc;
		InternalMaincc = NULL;
	}

	strcpy(temp, "main");	/* This is the main.cc */
	/* Yep, thats right, we are building the list backwards. It saves a pointer. */
	/* Of course non-init code won't know the difference. */
	InternalMaincc = new InternalCodeFile(temp,NULL,InternalMaincc);
	InternalFile = InternalMaincc;

	InternalFile->CodeStyle = 3;	/* This is a type 3 or "main.cc" file. */
	InternalFile->HasBeenModified = True;

	InternalFile->Append("#include \"main.h\"\n\n");

	InternalFile->Append("void ApplicationInitialize(int argc, char *argv[]);\n");
	InternalFile->Append("extern void ApplicationPreload(int argc, char *argv[]);\n");
	InternalFile->Append("extern void ApplicationLoad(int argc, char *argv[]);\n");
	InternalFile->Append("extern void GeneralEvent(OnyxMessage *event);\n\n");

	InternalFile->Append("main(int argc, char *argv[])\n");
	InternalFile->Append("{\n");
	InternalFile->Append("\tApplicationPreload(argc, argv);\n");
	InternalFile->Append("\tApplicationInitialize(argc, argv);\n");
	InternalFile->Append("\tApplicationLoad(argc, argv);\n");
	InternalFile->Append("\tApplicationRun();\n");
	InternalFile->Append("}\n\n");

	InternalFile->Append("void ApplicationInitialize(int argc, char *argv[])\n");
	InternalFile->Append("{\n");

	InternalFile->Append("\tInitialize(ApplicationName, argc, argv);\n\n");


	ProjectObject = ProjectObjects;

	while(ProjectObject != NULL)
	{
		if(ProjectObject->IsIncorporated)	/* Only if this object has ever been saved before. */
		{
			InternalFile->Append("\t%s = new %s;\n", ProjectObject->Name, ProjectObject->Type);
			InternalFile->Append("\t%s->ID = %sID;\n", ProjectObject->Name, ProjectObject->Name);
			if(!strcmp(ProjectObject->Parent,"ApplicationRoot"))
			{
				InternalFile->Append("\t%s->Parent = %s();\n", ProjectObject->Name, ProjectObject->Parent);
			}
			else if(!strcmp(ProjectObject->Parent,"Application"))
			{
				InternalFile->Append("\t%s->Parent = %s();\n", ProjectObject->Name, ProjectObject->Parent);
			}
			else
			{
				InternalFile->Append("\t%s->Parent = %s->Self();\n", ProjectObject->Name,ProjectObject->Parent);
			}

			UsableObject = UsableObjects;
			while(strcmp(UsableObject->Name,ProjectObject->Type))
			{
				UsableObject = UsableObject->Next;
			}


			ObjectProperty = ProjectObject->Property;
			while(ObjectProperty != NULL)
			{
				InternalFile->Append("\t");

				UsableProperty = UsableObject->Property;
				while(strcmp(UsableProperty->Name,ObjectProperty->Name))
				{
					UsableProperty = UsableProperty->Next;
				}
			
				Marker = &UsableProperty->Assignment[0];
				while(strlen(Marker))
				{
					if(!strncmp(Marker,"$X",2))
					{
						InternalFile->Append("%s->%s", ProjectObject->Name, ObjectProperty->Name);
						Marker++;
					}
					else if(!strncmp(Marker,"$Y",2))
					{
						InternalFile->Append("%s", EscapeQuotes(ObjectProperty->Value));
						Marker++;
					}
					else
					{
						InternalFile->Append("%c", *Marker);
					}

					Marker++;
				}

				InternalFile->Append(";\n");
				ObjectProperty = ObjectProperty->Next;
			}

			InternalFile->Append("\t%s->Create();\n\n", ProjectObject->Name);
		}
		ProjectObject = ProjectObject->Next;
	}
	InternalFile->Append("}\n\n");


	InternalFile->Append("void OpenStartingForm()\n{\n");

	for (ProjectObject = ProjectObjects; (ProjectObject != NULL) && strcmp(ProjectObject->Type,"OnyxWindow"); ProjectObject = ProjectObject->Next)
	{}

	if(ProjectObject != NULL)
	{
		InternalFile->Append("\t%s->Show();\n}\n\n",ProjectObject->Name);
	}
	else
	{
		InternalFile->Append("\n}\n\n");
	}


	InternalFile->Append("int OnyxProcessMessage(OnyxMessage *msg)\n");
	InternalFile->Append("{\n");
	InternalFile->Append("\tswitch (msg->id)\n");
	InternalFile->Append("\t{\n");

	ProjectObject = ProjectObjects;
	while(ProjectObject != NULL)
	{
		if(ProjectObject->IsIncorporated)	/* Only if this object has ever been saved before. */
		{
			InternalFile->Append("\t\tcase %sID:\n", ProjectObject->Name);

			UsableObject = UsableObjects;
			while(strcmp(UsableObject->Name,ProjectObject->Type))
			{
				UsableObject = UsableObject->Next;
			}

			InternalFile->Append("\t\t\tswitch (msg->action)\n");
			InternalFile->Append("\t\t\t{\n");

			UsableEvent = UsableObject->Event;
			while(UsableEvent != NULL)
			{
				InternalFile->Append("\t\t\t\tcase %s:\n", UsableEvent->Name);
				InternalFile->Append("\t\t\t\t\t%s_%s(", ProjectObject->Name, UsableEvent->Name);

				if(strlen(UsableEvent->Parm1) > 0)
				{
					InternalFile->Append("msg->%s", UsableEvent->Parm1);
					if(strlen(UsableEvent->Parm2) > 0)
					{
						InternalFile->Append(", msg->%s", UsableEvent->Parm2);
						if(strlen(UsableEvent->Parm3) > 0)
						{
							InternalFile->Append(", msg->%s", UsableEvent->Parm3);
							if(strlen(UsableEvent->Parm4) > 0)
							{
								InternalFile->Append(", msg->%s", UsableEvent->Parm4);
							}
						}
					}
				}
				InternalFile->Append(");\n");
				InternalFile->Append("\t\t\t\t\tbreak;\n\n");
				UsableEvent = UsableEvent->Next;
			}

			InternalFile->Append("\t\t\t}\n");
			InternalFile->Append("\t\t\tbreak;\n\n");
		}
		ProjectObject = ProjectObject->Next;
	}

	InternalFile->Append("\t\tdefault:\n");
	InternalFile->Append("\t\t\tGeneralEvent(msg);\n");
	InternalFile->Append("\t\t\tbreak;\n\n");
	InternalFile->Append("\t}\n");
	InternalFile->Append("\treturn True;\n");
	InternalFile->Append("}\n");
}


void PrepareForMake(char *WindowName)
{
	FILE *OutFile;
	char FileName[255];
	ProjectObjectList *ProjectObject;
	InternalCodeFile *InternalFile;


	if (!strcmp(WindowName,"Application"))
	{
		/* Save Applicaton.cc */
		for (InternalFile = InternalOxcCode; InternalFile != NULL; InternalFile = InternalFile->Next)
		{
			if (!strcmp(InternalFile->ObjectName, "Application") && !InternalFile->HasBeenDeleted)
			{
				OutFile = fopen("Application.cc","w+");
				if(OutFile)
				{
					/* Write initial header of includes. */
					fprintf(OutFile,"#include \"Events.h\"\n");
					fprintf(OutFile,"#include \"Objects.h\"\n\n");
					fflush(OutFile);
					fclose(OutFile);
					/* Append the current buffer to it. */
					InternalFile->AppendToFile("Application.cc");
				}
				else
				{
					printf("Error appending to Application.cc\n");
				}
			}
		}
	}
	else
	{
		/* Save the window. */
		for (ProjectObject = ProjectObjects; ProjectObject != NULL; ProjectObject = ProjectObject->Next)
		{
			if(!strcmp(ProjectObject->Name,WindowName))
			{
				/* Find the internal code that matches. */
				for (InternalFile = InternalOxcCode; InternalFile != NULL; InternalFile = InternalFile->Next)
				{
					/* If this is the matching code... */
					if (!strcmp(WindowName, InternalFile->ObjectName) && !InternalFile->HasBeenDeleted)
					{
						/* Open the c++ file */
						sprintf(FileName,"%s.cc",WindowName);
						OutFile = fopen(FileName,"w+");

						/* If it opened... */
						if(OutFile)
						{
							/* Write initial header of includes. */
							fprintf(OutFile,"#include \"Events.h\"\n");
							fprintf(OutFile,"#include \"Objects.h\"\n\n");
							fflush(OutFile);
							fclose(OutFile);

							/* Append the current buffer to it. */
							InternalFile->AppendToFile(FileName);
						}
						else
						{
							printf("Error writing to %s\n",FileName);
						}
					}
				}
			}
		}


		/* Save the objects attached to the above window. */
		for (ProjectObject = ProjectObjects; ProjectObject != NULL; ProjectObject = ProjectObject->Next)
		{
			/* If it is incorporated and not a window... */
			if (!strcmp(ProjectObject->Parent,WindowName))
			{
				/* Find the internal code that matches. */
				for (InternalFile = InternalOxcCode; InternalFile != NULL; InternalFile = InternalFile->Next)
				{
					/* If this is the matching code... */
					if (!strcmp(InternalFile->ObjectName, ProjectObject->Name) && !InternalFile->HasBeenDeleted)
					{
						/* Open the c++ file */
						sprintf(FileName,"%s.cc",ProjectObject->Parent);
						InternalFile->AppendToFile(FileName);
					}
				}
			}
		}
	}
}

ObjectList *GetNewObjectType(char *ObjectTypeName) {
	FILE *ObjectFile;
	char ObjectFileName[255];
	char InLine[255];
	char temp[255];
	struct ObjectList *NewObject;
	struct ObjectProperty *CurrentProperty;
	struct ObjectProperty *TmpProperty;
	struct ObjectEvent *CurrentEvent;
	char *Sstream;
	int jump;

	NewObject = new ObjectList;
	strcpy(NewObject->Name, ObjectTypeName);
	NewObject->Property = NULL;
	NewObject->Event = NULL;
	NewObject->Next = NULL;
	NewObject->ToolboxViewable = 0;
	strcpy(NewObject->ToolTip, ObjectTypeName);


	sprintf(ObjectFileName, "%s/ObjectRepository/%s.oob", OnyxLocation, ObjectTypeName);

	ObjectFile = fopen(ObjectFileName, "r");

	if (ObjectFile) {
		CurrentProperty = NULL;
		CurrentEvent = NULL;

		while (fgets(InLine, 254, ObjectFile) != NULL) {
			if (InLine[strlen(InLine)-1] == '\n') InLine[strlen(InLine)-1] = '\0';
			if (!strncmp(InLine, "Property ", 9)) {
				if (CurrentProperty == NULL) {
					NewObject->Property = new ObjectProperty;
					CurrentProperty = NewObject->Property;
				} else {
					CurrentProperty->Next = new ObjectProperty;
					CurrentProperty = CurrentProperty->Next;
				}
				CurrentProperty->Next = NULL;

				Sstream = InLine;
				Sstream = Sstream + 9;

				strcpy(CurrentProperty->Name, "");
				strcpy(CurrentProperty->Assignment, "");
				strcpy(CurrentProperty->Default, "");
				CurrentProperty->Access = ReadWrite;
				CurrentProperty->Type = TextBox;
				CurrentProperty->Entries = 0;

				sscanf(Sstream, "%s %s %s%n", CurrentProperty->Name, CurrentProperty->Assignment, CurrentProperty->Default, &jump);
				if (CurrentProperty->Default[0] == '"') {  /* quoted entry, contain spaces */
					strcpy(CurrentProperty->Default, &CurrentProperty->Default[1]);
					int i = strlen(CurrentProperty->Default);
					while (Sstream[jump] != '"' && Sstream[jump] != '\0') {
						CurrentProperty->Default[i++] = Sstream[jump++];
					}
					CurrentProperty->Default[i] = '\0';
				}

			} else if (!strncmp(InLine, "PropertySetting ", 16)) {
				Sstream = InLine;
				Sstream = Sstream + 16;

				/* Grab name of property that this setting belongs to. */
				sscanf(Sstream, "%s%n", temp, &jump);

				/* Find it in the list of currently known properties. */
				TmpProperty = NewObject->Property;
				while (TmpProperty != NULL && strcmp(TmpProperty->Name, temp)) {
					TmpProperty = TmpProperty->Next;
				}

				if (TmpProperty == NULL)	{ /* It wasn't there. */
					printf("Object description error. Received settings for %s before receiving the original property statement.\n", temp);
				} else {
					Sstream = Sstream + jump;	/* Advance stream marker. */

					/* Get the access type. */
					sscanf(Sstream, "%s%n", temp, &jump);

					if (!strcmp("ReadOnly", temp)) {
						TmpProperty->Access = ReadOnly;
					} else if (!strcmp("WriteOnly", temp)) {
						TmpProperty->Access = WriteOnly;
					} else {
						TmpProperty->Access = ReadWrite;
					}

					Sstream = Sstream + jump;	/* Advance stream marker. */

					/* Get the display type. */
					sscanf(Sstream, "%s%n", temp, &jump);

					if (!strcmp("TextBox", temp)) {
						TmpProperty->Type = TextBox;
					} else if(!strcmp("ComboBox", temp)) {
						TmpProperty->Type = ComboBox;
					} else if(!strcmp("ListBox", temp)) {
						TmpProperty->Type = ListBox;
					} else if(!strcmp("ColorBrowser", temp)) {
						TmpProperty->Type = ColorBrowser;
					} else if(!strcmp("FontBrowser", temp)) {
						TmpProperty->Type = FontBrowser;
					} else if(!strcmp("FileBrowser", temp)) {
						TmpProperty->Type = FileBrowser;
					} else {
						TmpProperty->Type = TextBox;
					}


					Sstream = Sstream + jump;	/* Advance stream marker. */

					/* Get the allowed values. */
					TmpProperty->Entries = 0;
					temp[0] = '\0';
					sscanf(Sstream, "%s%n", temp, &jump);
					while (temp[0] != '\0' && TmpProperty->Entries < 10) {	/* Maximum of 10 entries. */
						if (temp[0] == '"') {  /* quoted entry, contains spaces */
							strcpy(temp, &temp[1]);
							int i = strlen(temp);
							while (Sstream[jump] != '"' && Sstream[jump] != '\0') {
								temp[i++] = Sstream[jump++];
							}
							temp[i] = '\0';
							++jump;
						}
						strcpy(TmpProperty->Entry[TmpProperty->Entries++], temp);

						Sstream = Sstream + jump;	/* Advance stream marker. */
						temp[0] = '\0';
						sscanf(Sstream, "%s%n", temp, &jump);
					}
				}			
			} else if (!strncmp(InLine, "Event", 5)) {
				if (CurrentEvent == NULL) {
					NewObject->Event = new ObjectEvent;
					CurrentEvent = NewObject->Event;
				} else {
					CurrentEvent->Next = new ObjectEvent;
					CurrentEvent = CurrentEvent->Next;
				}
				CurrentEvent->Next = NULL;

				Sstream = InLine;
				Sstream = Sstream + 5;

				strcpy(CurrentEvent->Parm1, "");
				strcpy(CurrentEvent->Parm2, "");
				strcpy(CurrentEvent->Parm3, "");
				strcpy(CurrentEvent->Parm4, "");
				sscanf(Sstream, "%s %s %s %s %s", CurrentEvent->Name,CurrentEvent->Parm1, CurrentEvent->Parm2, CurrentEvent->Parm3, CurrentEvent->Parm4);
			} else if (!strncmp(InLine, "Toolbox", 7)) {
				Sstream = InLine + 7;
				NewObject->ToolboxViewable = atoi(Sstream);
			} else if (!strncmp(InLine, "ToolTip", 7)) {
				Sstream = InLine + 7;
				while (*Sstream == ' ') ++Sstream;
				strcpy(NewObject->ToolTip, Sstream);
			}
		}

		CustomControlsNeedToBeCompiled = True;

		fclose(ObjectFile);

	} else {

		printf("GetNewObjectType failed for %s\n", ObjectTypeName);

	}

	InitializeInternalBaseMakefile();	/* Sync the Base makefile */

	return NewObject;
}

void CloseAllActiveWindows()
{
	int i;

	if (ProjectExplorer->IsVisible)
	{
		ProjectExplorer->Close();
	}
	if (Toolbox->IsVisible)
	{
		Toolbox->Close();
	}
	if (PropertiesWindow->IsVisible)
	{
		PropertiesWindow->Close();
	}
	if (CustomControlsWindow->IsVisible)
	{
		CustomControlsWindow->Close();
	}
	if (CCHelpWindow->IsVisible)
	{
		CCHelpWindow->Close();
	}
	if (MenuEditor->IsVisible)
	{
		MenuEditor->Close();
	}
	if (MenuEditor2->IsVisible)
	{
		MenuEditor2->Close();
	}
	if (NewProjectWindow->IsVisible)
	{
		NewProjectWindow->Close();
	}
	if (OptionsWindow->IsVisible)
	{
		OptionsWindow->Close();
	}
	if (AboutWindow->IsVisible)
	{
		AboutWindow->Close();
	}

	for (i = 0; i < MaxDevelopmentWindows; i++)
	{
		if (DevelopmentWindow[i] != NULL && DevelopmentWindow[i]->IsVisible)
		{
			DevelopmentWindow[i]->Close();
		}
	}

	SelectedWindow = NULL;
	SelectedObject = NULL;
}
