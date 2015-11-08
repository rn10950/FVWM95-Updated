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

extern int WriteOnyxrcFile();

extern int SnapToGridIsDefault;

void OptionsWindow_CloseSignaled()
{
}

void OptionsWindow_Closed()
{
}

void OptionsWindow_Load()
{
	OLocationText->ClearText();
	OLocationText->AddText(OnyxLocation);
	OEditorText->ClearText();
	OEditorText->AddText(EditorCommand);
}

void OLocationText_EnterPressed()
{
}

void OEditorText_EnterPressed()
{
}

void OSaveButton_ButtonClick()
{
	int rc;

	strcpy(OnyxLocation, OLocationText->Text);
	strcpy(EditorCommand, OEditorText->Text);

	rc = WriteOnyxrcFile();
	if (rc == -1)
	{
		StopBox(OptionsWindow->Self(), "HOME variable not set. Cannot save options.");
	}
	else if (rc == -2)
	{
		StopBox(OptionsWindow->Self(), "Unable to open .onyxrc for write.\n"
					       "Check file permissions and HOME environment variable.");
	} else {
#if 0
		sprintf(OnyxIconLocation, "%s/icons", OnyxLocation);
		strcpy(MainWindow->Directory, OnyxIconLocation);
		/* update the pixmap path in MainWindow */
		MainWindow->Update();
		/* update the controls that use pixmaps */
		OpenButton->Update();
		SaveButton->Update();
		SaveProjectButton->Update();
		NewWindowButton->Update();
		ViewWindowButton->Update();
		NextWindowButton->Update();
		DeleteControlButton->Update();
		LockControlsButton->Update();
		MakeButton->Update();
		MakeRunButton->Update();
		RunButton->Update();
		StopButton->Update();
		ShowPropertiesButton->Update();
		ShowCodeButton->Update();
		ShowToolboxButton->Update();
		ProjectExplorerButton->Update();
		SnapToGrid->Update();
		SplashPicture->Update();
		AboutPicture->Update();
		XClassWarningLabel1->Update();
#endif
	}
	OptionsWindow->Close();
}

void OCancelButton_ButtonClick()
{
	OptionsWindow->Close();
}

void SnapToGridDefault_ButtonClicked()
{
	SnapToGridIsDefault = !SnapToGridIsDefault;
}
