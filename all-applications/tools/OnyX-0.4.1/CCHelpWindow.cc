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

extern int CCSelected;
extern char CCFileList[CCIDMax][255];
OnyxLabel *CCHelpInfo[20];

void CCHelpWindow_CloseSignaled()
{
}

void CCHelpWindow_Closed()
{
}


void CCHelpWindow_Load()
{
	OnyxLabel *CCHelpTitle;

	FILE *InFile;
	char *Marker, InLine[255];
	int i;

	for (i = 0; (i < 20) && (CCHelpInfo[i] != NULL); i++)
	{
		delete CCHelpInfo[i];
		CCHelpInfo[i] = NULL;
	}

	InFile = fopen(CCFileList[CCSelected], "r");
	if (InFile)
	{
		CCHelpTitle = new OnyxLabel;
		CCHelpTitle->ID = -1;
		CCHelpTitle->Parent = CCHelpWindow->Self();
		CCHelpTitle->X = 5;
		CCHelpTitle->Y = 30;
		CCHelpTitle->Width = CCHelpWindow->Width - 10;
		CCHelpTitle->Height = 20;
		strcat(CCHelpTitle->Font, " bold");
		fgets(CCHelpTitle->Text, 255, InFile);
		Marker = strrchr(CCHelpTitle->Text, '\n');
		if (Marker) *Marker = '\0';
		CCHelpTitle->Create();

		i = 0;
		fgets(InLine, 1000, InFile);	/* Skip oob file(s) designator. */

		while (fgets(InLine, 255, InFile))
		{
			CCHelpInfo[i] = new OnyxLabel;
			CCHelpInfo[i]->ID = -1;
			CCHelpInfo[i]->Parent = CCHelpWindow->Self();
			CCHelpInfo[i]->X = 5;
			CCHelpInfo[i]->Y = 60 + i * 20;
			CCHelpInfo[i]->Width = CCHelpWindow->Width - 10;
			CCHelpInfo[i]->Height = 20;
			strcpy(CCHelpInfo[i]->Text, InLine);
			Marker = strrchr(CCHelpInfo[i]->Text, '\n');
			if (Marker) *Marker = '\0';
			CCHelpInfo[i++]->Create();
			if (i > 19)
			{
				break;
			}
		}
		fclose(InFile);
	}
	else
	{
		printf("Could not open repository information for object.\n");
	}
}

void CCHelpOK_ButtonClick()
{
	CCHelpWindow->Close();
}
