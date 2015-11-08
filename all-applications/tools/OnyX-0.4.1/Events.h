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

extern void OpenButton_ButtonClick();
extern void SaveButton_ButtonClick();
extern void SaveProjectButton_ButtonClick();
extern void NewWindowButton_ButtonClick();
extern void ViewWindowButton_ButtonClick();
extern void NextWindowButton_ButtonClick();
extern void MakeButton_ButtonClick();
extern void RunButton_ButtonClick();
extern void MakeRunButton_ButtonClick();
extern void StopButton_ButtonClick();
extern void DeleteControlButton_ButtonClick();
extern void LockControlsButton_ButtonClick();
extern void ProjectExplorerButton_ButtonClick();
extern void ShowCodeButton_ButtonClick();
extern void ShowToolboxButton_ButtonClick();
extern void ShowPropertiesButton_ButtonClick();
extern void MainWindow_Closed();
extern void MainWindow_CloseSignaled();
extern void MainWindow_Load();
extern void MainWindow_WindowResized(int OldWidth, int OldHeight, int NewWidth, int NewHeight);
extern void ProjectExplorer_Closed();
extern void ProjectExplorer_CloseSignaled();
extern void ProjectExplorer_Load();
extern void ProjectExplorer_WindowResized(int OldWidth, int OldHeight, int NewWidth, int NewHeight);
extern void PropertiesWindow_Closed();
extern void PropertiesWindow_CloseSignaled();
extern void PropertiesWindow_Load();
extern void PropertiesWindow_WindowResized(int OldWidth, int OldHeight, int NewWidth, int NewHeight);
extern void Toolbox_Closed();
extern void Toolbox_CloseSignaled();
extern void Toolbox_Load();
extern void Toolbox_WindowResized(int OldWidth, int OldHeight, int NewWidth, int NewHeight);
extern void Toolbox_ButtonReleased(int Button, int x, int y);
extern void MakeWindow_Closed();
extern void MakeWindow_CloseSignaled();
extern void MakeWindow_Load();
extern void MakeWindow_WindowResized(int OldWidth, int OldHeight, int NewWidth, int NewHeight);
extern void MakeTimer_TimerElapsed();
extern void TLCorner_ButtonPressed(int Button, int x, int y);
extern void TLCorner_ButtonReleased(int Button, int x, int y);
extern void TLCorner_PointerMoved(int x, int y, int rootX, int rootY);
extern void TRCorner_ButtonPressed(int Button, int x, int y);
extern void TRCorner_ButtonReleased(int Button, int x, int y);
extern void TRCorner_PointerMoved(int x, int y, int rootX, int rootY);
extern void BLCorner_ButtonPressed(int Button, int x, int y);
extern void BLCorner_ButtonReleased(int Button, int x, int y);
extern void BLCorner_PointerMoved(int x, int y, int rootX, int rootY);
extern void BRCorner_ButtonPressed(int Button, int x, int y);
extern void BRCorner_ButtonReleased(int Button, int x, int y);
extern void BRCorner_PointerMoved(int x, int y, int rootX, int rootY);
extern void NewProjectMenuItem_MenuItemSelected();
extern void NewProjectMenuItem_MenuItemHighlighted();
extern void OpenProjectMenuItem_MenuItemSelected();
extern void OpenProjectMenuItem_MenuItemHighlighted();
extern void SaveObjectMenuItem_MenuItemSelected();
extern void SaveObjectMenuItem_MenuItemHighlighted();
extern void SaveProjectMenuItem_MenuItemSelected();
extern void SaveProjectMenuItem_MenuItemHighlighted();
extern void SaveProjectAsMenuItem_MenuItemSelected();
extern void SaveProjectAsMenuItem_MenuItemHighlighted();
extern void ExitMenuItem_MenuItemSelected();
extern void ExitMenuItem_MenuItemHighlighted();

extern void ViewCodeMenuItem_MenuItemSelected();
extern void ViewCodeMenuItem_MenuItemHighlighted();
extern void ViewWindowMenuItem_MenuItemSelected();
extern void ViewWindowMenuItem_MenuItemHighlighted();
extern void ViewGridMenuItem_MenuItemSelected();
extern void ViewGridMenuItem_MenuItemHighlighted();
extern void ViewProjectMenuItem_MenuItemSelected();
extern void ViewProjectMenuItem_MenuItemHighlighted();
extern void ViewPropertiesMenuItem_MenuItemSelected();
extern void ViewPropertiesMenuItem_MenuItemHighlighted();
extern void ToolBarMenuItem_MenuItemSelected();
extern void ToolBarMenuItem_MenuItemHighlighted();
extern void ToolBoxMenuItem_MenuItemSelected();
extern void ToolBoxMenuItem_MenuItemHighlighted();

extern void MakeMenuItem_MenuItemSelected();
extern void MakeMenuItem_MenuItemHighlighted();
extern void MakeCleanMenuItem_MenuItemSelected();
extern void MakeCleanMenuItem_MenuItemHighlighted();
extern void MakeRunMenuItem_MenuItemSelected();
extern void MakeRunMenuItem_MenuItemHighlighted();
extern void RunMenuItem_MenuItemSelected();
extern void RunMenuItem_MenuItemHighlighted();
extern void StopMenuItem_MenuItemSelected();
extern void StopMenuItem_MenuItemHighlighted();

extern void NewProjectOpenButton_ButtonClick();
extern void NewProjectCancelButton_ButtonClick();
extern void NewProjectWindow_CloseSignaled();
extern void NewProjectWindow_Closed();

extern void ViewWindowButton_ButtonClick();

extern void MenuEditorItem_MenuItemSelected();
extern void MenuEditorItem_MenuItemHighlighted();
extern void CustomControlsItem_MenuItemSelected();
extern void CustomControlsItem_MenuItemHighlighted();
extern void OptionsItem_MenuItemSelected();
extern void OptionsItem_MenuItemHighlighted();

extern void CustomControlsWindow_Closed();
extern void CustomControlsWindow_CloseSignaled();
extern void CustomControlsWindow_Load();
extern void CustomControlsWindow_WindowResized(int OldWidth, int OldHeight, int NewWidth, int NewHeight);

extern void CCHelp_ButtonClick();
extern void CCRecompile_ButtonClick();


extern void CCHelpWindow_Closed();
extern void CCHelpWindow_CloseSignaled();
extern void CCHelpWindow_Load();

extern void CCHelpOK_ButtonClick();

extern void OptionsWindow_Closed();
extern void OptionsWindow_CloseSignaled();
extern void OptionsWindow_Load();

extern void NewProjectName_EnterPressed();
extern void OEditorText_EnterPressed();
extern void OLocationText_EnterPressed();
extern void SnapToGridDefault_ButtonClicked();

extern void OSaveButton_ButtonClick();
extern void OCancelButton_ButtonClick();

extern void ExplorerTree_ButtonPressed(int Button, int x, int y);
extern void ExplorerTree_ItemSelected();

extern void SplashScreen_Closed();
extern void SplashTimer_TimerElapsed();

extern void EditTimer_TimerElapsed();

extern void AboutWindow_ButtonPressed(int Button, int x, int y);
extern void AboutMenuItem_MenuItemSelected();

extern void CreateApplicationMenuItem_MenuItemSelected();
extern void ExportObjectMenuItem_MenuItemSelected();
extern void ExportObjectMenuItem_MenuItemHighlighted();
extern void ImportObjectMenuItem_MenuItemSelected();
extern void ImportObjectMenuItem_MenuItemHighlighted();
extern void ExportProjectMenuItem_MenuItemSelected();
extern void ExportProjectMenuItem_MenuItemHighlighted();
extern void ImportProjectMenuItem_MenuItemSelected();
extern void ImportProjectMenuItem_MenuItemHighlighted();

extern void SnapToGrid_ButtonClick();
extern void SnapToGridMenuItem_MenuItemSelected();

extern void MenuEditor_Closed();
extern void MenuEditor_CloseSignaled();
extern void MenuEditor_Load();
extern void MenuEditor_WindowResized(int OldWidth, int OldHeight, int NewWidth, int NewHeight);

extern void MenuTree_ButtonPressed(int Button, int x, int y);
extern void MenuAdd_ButtonClick();
extern void MenuDelete_ButtonClick();
extern void MenuItemAdd_ButtonClick();
extern void MenuItemDelete_ButtonClick();

extern void MenuEditor2_Closed();
extern void MenuEditor2_CloseSignaled();
extern void MenuEditor2_Load();
extern void MenuEditor2_WindowResized(int OldWidth, int OldHeight, int NewWidth, int NewHeight);

extern void MenuEditor2Ok_ButtonClick();
extern void MenuEditor2Cancel_ButtonClick();
extern void MenuEditor2TextBox_EnterPressed();

extern void CCScroll_ScrollBarMoved(int Position);
extern void XClassWarningOK_ButtonClick();
