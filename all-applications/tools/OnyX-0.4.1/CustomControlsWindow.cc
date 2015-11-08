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
#include <sys/types.h>
#include <dirent.h>

int CCSelected;
OnyxCheckBox *CCList[CCIDMax];
char CCFileList[CCIDMax][255];
int TotalCCObjects;

extern char MakeType[10];

void CustomControlsWindow_CloseSignaled()
{
}

void CustomControlsWindow_Closed()
{
	int i;

	for (i = 0; i < TotalCCObjects; i++)
	{
		delete CCList[i];
		CCList[i] = NULL;
	}
}


void CustomControlsWindow_Load()
{
	DIR *ReposDir;
	struct dirent *rdfile;
	char *Marker;
	int i;
	FILE *oocfile;
	char temp[255];
	char temp2[255];


	if (!CustomControlsWindow->IsVisible)	/* If the window isn't already open then add all of the choices. */
	{

		CCSelected = 0;
		i = 0;
	
		sprintf(temp, "%s/ObjectRepository", OnyxLocation);
		ReposDir = opendir(temp);
	
		if (ReposDir)
		{
			rdfile = readdir(ReposDir);	/* Skip past . and .. */
			rdfile = readdir(ReposDir);
			rdfile = readdir(ReposDir);
			while (rdfile != NULL)
			{
				Marker = strstr(rdfile->d_name, ".ooc");
				if (Marker != NULL)
				{
					CCList[i] = new OnyxCheckBox;
					CCList[i]->ID = i + CCIDModifier;
					CCList[i]->Parent = CustomControlsWindow->Self();
					CCList[i]->X = 20;
					CCList[i]->Y = 20 + i * 20;
					CCList[i]->Width = 135;
					CCList[i]->Height = 20;
	
					sprintf(CCFileList[i], "%s/ObjectRepository/%s", OnyxLocation, rdfile->d_name);
					oocfile = fopen(CCFileList[i], "r");
					fgets(temp, 255, oocfile);
					Marker = strrchr(temp, '\n');
					if (Marker) *Marker = '\0';
					strcpy(CCList[i]->Text, temp);
					fgets(temp, 255, oocfile);
					sscanf(temp, "%s", temp2);
					CCList[i]->IsChecked = ObjectTypeKnown(temp2);
					fclose(oocfile);
					CCList[i++]->Create();
				}
				rdfile = readdir(ReposDir);
			}
			closedir(ReposDir);
			TotalCCObjects = i;
			CCScroll->ChangeRangeAndPageSize(TotalCCObjects, (CustomControlsWindow->Height - 25) / 20);
		}
		else
		{
			printf("Could not open repository information for object\n");
		}
	}
}

void CustomControlsWindow_WindowResized(int OldWidth, int OldHeight, int NewWidth, int NewHeight)
{
	int i, j;

	if ((NewHeight - 25) / CCList[0]->Height >= TotalCCObjects + 1)	/* If everything currently fits in the window */
	{
		CCScroll->Move(CustomControlsWindow->Width + CCScroll->Width * 2, CCScroll->Y);	/* Hide the scroll bar. It's not needed. */
	}
	else
	{
		CCScroll->MoveResize(170, CCScroll->Y, CCScroll->Width, NewHeight - 30);	/* Make the scroll bar visible. */
		CCScroll->ChangeRangeAndPageSize(TotalCCObjects, (NewHeight - 25) / 20);	/* Set it to the right page size. */
		CCScroll->Update();								/* Update the display for it. */
	}


	if (CCList[0]->Y < 20)	/* If the items have been raised up above the starting location */
	{
		i =  -1 *  CCList[0]->Y + 20;	/* Compute how far they have been moved up. */
		j = CustomControlsWindow->Height - CCList[TotalCCObjects - 1]->Y - 2 * CCList[0]->Height; /* Compute how much room there is at the bottom. */
		if (j > CCList[0]->Height)	/* if there is room to move something down. */
		{
			j = i < j ? i : j;	/* Make j the minimum of i and j. */

			for (i = 0; i < TotalCCObjects; i++)	/* Move everything down to use the new room. */
			{
				CCList[i]->Move(CCList[i]->X, CCList[i]->Y + j);
			}
		}
	}
}

void CCHelp_ButtonClick()
{
	CCHelpWindow->Show();
}

void CCRecompile_ButtonClick()
{
	struct ObjectList *UsableObject;
	char temp[255];

	/* Recopy all of the code for all of the objects. */
        for (UsableObject = UsableObjects; UsableObject != NULL; UsableObject = UsableObject->Next)
        {
        	/* Copy over the c++ file. */
                sprintf(temp, "cp %s/ObjectRepository/%s.cc .onyx", OnyxLocation, UsableObject->Name);
                system(temp);

                /* Copy over the header file. */
                sprintf(temp, "cp %s/ObjectRepository/%s.h .onyx", OnyxLocation, UsableObject->Name);
                system(temp);
	}

//	system("make baseclean 2>&1 > /dev/null");
	strcpy(MakeType, "base");
	MakeWindow->Show();
}


void CCScroll_ScrollBarMoved(int Position)
{
	int i;

	if (TotalCCObjects == 0) return;

	for (i = 0; i < TotalCCObjects; i++)
	{
		CCList[i]->Move(CCList[i]->X, (20 + i * 20) - CCList[0]->Height * Position);
	}
}
