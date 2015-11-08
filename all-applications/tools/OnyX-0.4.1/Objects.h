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

#include "Includes.h"

extern OnyxWindow *MainWindow;
extern OnyxWindow *ProjectExplorer;
extern OnyxWindow *PropertiesWindow;
extern OnyxWindow *Toolbox;
extern OnyxWindow *MakeWindow;
extern OnyxWindow *SplashScreen;
extern OnyxWindow *MenuEditor;
extern OnyxWindow *MenuEditor2;
extern OnyxWindow *XClassWarning;
extern OnyxMenuBar *MainMenuBar;
extern OnyxMenu *FileMenu;
extern OnyxMenuItem *NewProjectMenuItem;
extern OnyxMenuItem *OpenProjectMenuItem;
extern OnyxMenuItem *SaveObjectMenuItem;
extern OnyxMenuItem *SaveProjectMenuItem;
extern OnyxMenuItem *SaveProjectAsMenuItem;
extern OnyxMenuItem *FileMenuSep1;
extern OnyxMenuItem *FileMenuSep2;
extern OnyxMenuItem *ExitMenuItem;
extern OnyxButton *OpenButton;
extern OnyxButton *SaveButton;
extern OnyxButton *SaveProjectButton;
extern OnyxButton *NewWindowButton;
extern OnyxButton *ViewWindowButton;
extern OnyxButton *NextWindowButton;
extern OnyxButton *DeleteControlButton;
extern OnyxButton *LockControlsButton;
extern OnyxFileDialog *OpenFileDialog;
extern OnyxFileDialog *SaveFileDialog;
extern OnyxWindow *HiddenWindow;
extern OnyxMsgBox *CloseMessageBox;
extern OnyxButton *MakeButton;
extern OnyxButton *MakeRunButton;
extern OnyxButton *RunButton;
extern OnyxButton *StopButton;
extern OnyxButton *ShowPropertiesButton;
extern OnyxButton *ShowCodeButton;
extern OnyxButton *ShowToolboxButton;
extern OnyxButton *ProjectExplorerButton;

extern OnyxIcon *ToolbarSeparator;

extern OnyxTimer *MakeTimer;
extern OnyxTimer *EditTimer;
extern OnyxTimer *SplashTimer;
extern OnyxTextCanvas *MakeText;
extern OnyxScreen *ScreenInfo;
extern OnyxIcon *TLCorner;
extern OnyxIcon *TRCorner;
extern OnyxIcon *BLCorner;
extern OnyxIcon *BRCorner;

extern OnyxMenu *ViewMenu;
extern OnyxMenuItem *ViewCodeMenuItem;
extern OnyxMenuItem *ViewWindowMenuItem;
extern OnyxMenuItem *ViewGridMenuItem;
extern OnyxMenuItem *ViewMenuSep1;
extern OnyxMenuItem *ViewProjectMenuItem;
extern OnyxMenuItem *ViewPropertiesMenuItem;
extern OnyxMenuItem *ViewMenuSep2;
extern OnyxMenuItem *ToolBarMenuItem;
extern OnyxMenuItem *ToolBoxMenuItem;

extern OnyxMenu *RunMenu;
extern OnyxMenuItem *MakeMenuItem;
extern OnyxMenuItem *MakeCleanMenuItem;
extern OnyxMenuItem *MakeRunMenuItem;
extern OnyxMenuItem *RunMenuSep1;
extern OnyxMenuItem *RunMenuItem;
extern OnyxMenuItem *StopMenuItem;

extern OnyxWindow *NewProjectWindow;
extern OnyxLabel *NewProjectLabel;
extern OnyxTextBox *NewProjectName;
extern OnyxButton *NewProjectOpenButton;
extern OnyxButton *NewProjectCancelButton;

extern OnyxMenu *ToolsMenu;
extern OnyxMenuItem *MenuEditorItem;
extern OnyxMenuItem *CustomControlsItem;
extern OnyxMenu *ToolsMenuSep1;
extern OnyxMenuItem *OptionsItem;

extern OnyxWindow *CustomControlsWindow;
extern OnyxButton *CCHelp;
extern OnyxButton *CCRecompile;
extern OnyxVScrollBar *CCScroll;

extern OnyxWindow *CCHelpWindow;
extern OnyxButton *CCHelpOK;

extern OnyxWindow *OptionsWindow;
extern OnyxLabel *OLocationLabel;
extern OnyxTextBox *OLocationText;
extern OnyxLabel *OEditorLabel;
extern OnyxTextBox *OEditorText;

extern OnyxButton *OSaveButton;
extern OnyxButton *OCancelButton;

extern OnyxTree *ExplorerTree;

extern OnyxMsgBox *MakeMessageBox;
extern OnyxMsgBox *ExitSaveMessageBox;

extern OnyxIcon *SplashPicture;

extern OnyxMenu *HelpMenu;
extern OnyxMenuItem *ContentsMenuItem;
extern OnyxMenuItem *SearchMenuItem;
extern OnyxMenuItem *FileMenuSep1;
extern OnyxMenuItem *AboutMenuItem;

extern OnyxWindow *AboutWindow;
extern OnyxIcon *AboutPicture;
extern OnyxLabel *AboutLabel1;
extern OnyxLabel *AboutLabel2;
extern OnyxLabel *AboutLabel3;
extern OnyxLabel *AboutLabel4;
extern OnyxLabel *AboutLabel5;

extern OnyxMenuItem *ExportObjectMenuItem;
extern OnyxMenuItem *ImportObjectMenuItem;
extern OnyxMenuItem *FileMenuSep3;
extern OnyxMenuItem *ExportProjectMenuItem;
extern OnyxMenuItem *ImportProjectMenuItem;
extern OnyxMenuItem *FileMenuSep4;
extern OnyxMenuItem *CreateApplicationMenuItem;
extern OnyxMenuItem *FileMenuSep5;

extern OnyxCheckBox *SnapToGridDefault;
extern OnyxButton *SnapToGrid;
extern OnyxMenuItem *SnapToGridMenuItem;

extern OnyxTree *MenuTree;
extern OnyxButton *MenuAdd;
extern OnyxButton *MenuDelete;
extern OnyxButton *MenuItemAdd;
extern OnyxButton *MenuItemDelete;

extern OnyxButton *MenuEditor2Ok;
extern OnyxButton *MenuEditor2Cancel;
extern OnyxLabel *MenuEditor2Label;
extern OnyxTextBox *MenuEditor2TextBox;

extern OnyxButton *XClassWarningOK;
extern OnyxIcon *XClassWarningLabel1;
extern OnyxLabel *XClassWarningLabel2;
extern OnyxLabel *XClassWarningLabel3;
extern OnyxLabel *XClassWarningLabel4;
extern OnyxLabel *XClassWarningLabel5;
extern OnyxLabel *XClassWarningLabel6;
extern OnyxLabel *XClassWarningLabel7;

extern OnyxMsgBox *ConvertMsgBox;
extern OnyxMsgBox *ConfirmMsgBox;

extern OnyxColorDialog *ColorDialog;
extern OnyxFontDialog *FontDialog;
