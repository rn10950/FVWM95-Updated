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

#include <stdio.h>
#include "Includes.h"
#include "Events.h"
#include "Global.h"

#define ApplicationName "OnyX"

#define MainWindowID 1
OnyxWindow *MainWindow;

#define MainMenuBarID 2
OnyxMenuBar *MainMenuBar;

#define OpenButtonID 3
OnyxButton *OpenButton;

#define SaveButtonID 4
OnyxButton *SaveButton;

#define NewWindowButtonID 5
OnyxButton *NewWindowButton;

#define OpenFileDialogID 6
OnyxFileDialog *OpenFileDialog;

#define SaveFileDialogID 7
OnyxFileDialog *SaveFileDialog;

#define HiddenWindowID 8
OnyxWindow *HiddenWindow;

#define CloseMessageBoxID 9
OnyxMsgBox *CloseMessageBox;

#define MakeButtonID 10
OnyxButton *MakeButton;

#define MakeRunButtonID 11
OnyxButton *MakeRunButton;

#define RunButtonID 12
OnyxButton *RunButton;

#define StopButtonID 13
OnyxButton *StopButton;

#define ShowPropertiesButtonID 14
OnyxButton *ShowPropertiesButton;

#define ShowCodeButtonID 15
OnyxButton *ShowCodeButton;

#define ShowToolboxButtonID 16
OnyxButton *ShowToolboxButton;

#define ProjectExplorerButtonID 17
OnyxButton *ProjectExplorerButton;

#define ProjectExplorerID 18
OnyxWindow *ProjectExplorer;

#define PropertiesWindowID 19
OnyxWindow *PropertiesWindow;

#define ToolboxID 20
OnyxWindow *Toolbox;

#define MakeWindowID 21
OnyxWindow *MakeWindow;

#define MakeTimerID 22
OnyxTimer *MakeTimer;

#define MakeTextID 23
OnyxTextCanvas *MakeText;

#define ScreenInfoID 24
OnyxScreen *ScreenInfo;

#define TLCornerID 26 /* Must also be defined exactly the same in MainWindow.cc */
OnyxIcon *TLCorner;

#define TRCornerID 27 /* Must also be defined exactly the same in MainWindow.cc */
OnyxIcon *TRCorner;

#define BLCornerID 28 /* Must also be defined exactly the same in MainWindow.cc */
OnyxIcon *BLCorner;

#define BRCornerID 29 /* Must also be defined exactly the same in MainWindow.cc */
OnyxIcon *BRCorner;

#define FileMenuID 30
OnyxMenu *FileMenu;

#define OpenProjectMenuItemID 31
OnyxMenuItem *OpenProjectMenuItem;

#define SaveProjectMenuItemID 32
OnyxMenuItem *SaveProjectMenuItem;

#define FileMenuSep1ID 33
OnyxMenuItem *FileMenuSep1;

#define ExitMenuItemID 34
OnyxMenuItem *ExitMenuItem;

#define ViewMenuID 35
OnyxMenu *ViewMenu;

#define ViewWindowMenuItemID 36
OnyxMenuItem *ViewWindowMenuItem;

#define ViewProjectMenuItemID 37
OnyxMenuItem *ViewProjectMenuItem;

#define ViewMenuSep1ID 38
OnyxMenuItem *ViewMenuSep1;

#define MakeMenuItemID 39
OnyxMenuItem *MakeMenuItem;

#define RunMenuItemID 40
OnyxMenuItem *RunMenuItem;

#define StopMenuItemID 41
OnyxMenuItem *StopMenuItem;

#define NewProjectMenuItemID 42
OnyxMenuItem *NewProjectMenuItem;

#define SaveProjectAsMenuItemID 43
OnyxMenuItem *SaveProjectAsMenuItem;

#define FileMenuSep2ID 44
OnyxMenuItem *FileMenuSep2;

#define MakeCleanMenuItemID 45
OnyxMenuItem *MakeCleanMenuItem;

#define MakeRunMenuItemID 46
OnyxMenuItem *MakeRunMenuItem;

#define RunMenuSep1ID 47
OnyxMenuItem *RunMenuSep1;

#define RunMenuID 48
OnyxMenu *RunMenu;

#define ViewCodeMenuItemID 49
OnyxMenuItem *ViewCodeMenuItem;

#define ViewPropertiesMenuItemID 50
OnyxMenuItem *ViewPropertiesMenuItem;

#define ViewMenuSep2ID 51
OnyxMenuItem *ViewMenuSep2;

#define ToolBarMenuItemID 52
OnyxMenuItem *ToolBarMenuItem;

#define ToolBoxMenuItemID 53
OnyxMenuItem *ToolBoxMenuItem;

#define NewProjectWindowID 54
OnyxWindow *NewProjectWindow;

#define NewProjectLabelID 55
OnyxLabel *NewProjectLabel;

#define NewProjectNameID 56
OnyxTextBox *NewProjectName;

#define NewProjectOpenButtonID 57
OnyxButton *NewProjectOpenButton;

#define NewProjectCancelButtonID 58
OnyxButton *NewProjectCancelButton;

#define SaveObjectMenuItemID 59
OnyxMenuItem *SaveObjectMenuItem;

#define ToolsMenuID 61
OnyxMenu *ToolsMenu;

#define MenuEditorItemID 62
OnyxMenuItem *MenuEditorItem;

#define CustomControlsItemID 63
OnyxMenuItem *CustomControlsItem;

#define ToolsMenuSep1ID 64
OnyxMenuItem *ToolsMenuSep1;

#define OptionsItemID 65
OnyxMenuItem *OptionsItem;

#define CustomControlsWindowID 66
OnyxWindow *CustomControlsWindow;

#define CCHelpID 67
OnyxButton *CCHelp;

#define CCRecompileID 68
OnyxButton *CCRecompile;

#define CCHelpWindowID 69
OnyxWindow *CCHelpWindow;

#define CCHelpOKID 70
OnyxButton *CCHelpOK;

#define OptionsWindowID 75
OnyxWindow *OptionsWindow;

#define OLocationLabelID 76
OnyxLabel *OLocationLabel;

#define OLocationTextID 77
OnyxTextBox *OLocationText;

#define OEditorLabelID 78
OnyxLabel *OEditorLabel;

#define OEditorTextID 79
OnyxTextBox *OEditorText;

#define OSaveButtonID 80
OnyxButton *OSaveButton;

#define OCancelButtonID 81
OnyxButton *OCancelButton;

#define ExplorerTreeID 82
OnyxTree *ExplorerTree;

#define MakeMessageBoxID 83
OnyxMsgBox *MakeMessageBox;

#define LockControlsButtonID 84
OnyxButton *LockControlsButton;

#define SplashScreenID 85
OnyxWindow *SplashScreen;

#define SplashPictureID 86
OnyxIcon *SplashPicture;

#define SplashTimerID 87
OnyxTimer *SplashTimer;

#define EditTimerID 88
OnyxTimer *EditTimer;

#define ExitSaveMessageBoxID 89
OnyxMsgBox *ExitSaveMessageBox;

#define AboutWindowID 90
OnyxWindow *AboutWindow;

#define HelpMenuID 91
OnyxMenu *HelpMenu;

#define AboutMenuItemID 92
OnyxMenuItem *AboutMenuItem;

#define HelpMenuSep1ID 93
OnyxMenuItem *HelpMenuSep1;

#define ContentsMenuItemID 94
OnyxMenuItem *ContentsMenuItem;

#define SearchMenuItemID 95
OnyxMenuItem *SearchMenuItem;

#define AboutPictureID 96
OnyxIcon *AboutPicture;

#define AboutLabel1ID 97
OnyxLabel *AboutLabel1;

#define AboutLabel2ID 98
OnyxLabel *AboutLabel2;

#define AboutLabel3ID 99
OnyxLabel *AboutLabel3;

#define AboutLabel4ID 100
OnyxLabel *AboutLabel4;

#define AboutLabel5ID 101
OnyxLabel *AboutLabel5;

#define ExportObjectMenuItemID 102
OnyxMenuItem *ExportObjectMenuItem;

#define ImportObjectMenuItemID 103
OnyxMenuItem *ImportObjectMenuItem;

#define FileMenuSep3ID 104
OnyxMenuItem *FileMenuSep3;

#define ExportProjectMenuItemID 105
OnyxMenuItem *ExportProjectMenuItem;

#define ImportProjectMenuItemID 106
OnyxMenuItem *ImportProjectMenuItem;

#define FileMenuSep4ID 107
OnyxMenuItem *FileMenuSep4;

#define CreateApplicationMenuItemID 108
OnyxMenuItem *CreateApplicationMenuItem;

#define FileMenuSep5ID 109
OnyxMenuItem *FileMenuSep5;

#define SnapToGridDefaultID 110
OnyxCheckBox *SnapToGridDefault;

#define SnapToGridID 111
OnyxButton *SnapToGrid;

#define SnapToGridMenuItemID 112
OnyxMenuItem *SnapToGridMenuItem;

#define MenuEditorID 113
OnyxWindow *MenuEditor;

#define MenuTreeID 114
OnyxTree *MenuTree;

#define MenuAddID 115
OnyxButton *MenuAdd;

#define MenuDeleteID 116
OnyxButton *MenuDelete;

#define MenuEditor2ID 117
OnyxWindow *MenuEditor2;

#define MenuEditor2OkID 118
OnyxButton *MenuEditor2Ok;

#define MenuEditor2CancelID 119
OnyxButton *MenuEditor2Cancel;

#define MenuEditor2LabelID 120
OnyxLabel *MenuEditor2Label;

#define MenuEditor2TextBoxID 121
OnyxTextBox *MenuEditor2TextBox;

#define MenuItemAddID 122
OnyxButton *MenuItemAdd;

#define MenuItemDeleteID 123
OnyxButton *MenuItemDelete;

#define CCScrollID 124
OnyxVScrollBar *CCScroll;

#define XClassWarningID 125
OnyxWindow *XClassWarning;

#define XClassWarningOKID 126
OnyxButton *XClassWarningOK;

#define XClassWarningLabel1ID 127
OnyxIcon *XClassWarningLabel1;

#define XClassWarningLabel2ID 128
OnyxLabel *XClassWarningLabel2;

#define XClassWarningLabel3ID 129
OnyxLabel *XClassWarningLabel3;

#define XClassWarningLabel4ID 130
OnyxLabel *XClassWarningLabel4;

#define XClassWarningLabel5ID 131
OnyxLabel *XClassWarningLabel5;

#define XClassWarningLabel6ID 132
OnyxLabel *XClassWarningLabel6;

#define XClassWarningLabel7ID 133
OnyxLabel *XClassWarningLabel7;

#define ConvertMsgBoxID 135
OnyxMsgBox *ConvertMsgBox;

#define ConfirmMsgBoxID 136
OnyxMsgBox *ConfirmMsgBox;

#define ColorDialogID 137
OnyxColorDialog *ColorDialog;

#define FontDialogID 138
OnyxFontDialog *FontDialog;

#define ViewWindowButtonID 139
OnyxButton *ViewWindowButton;

#define NextWindowButtonID 140
OnyxButton *NextWindowButton;

#define ViewGridMenuItemID 141
OnyxMenuItem *ViewGridMenuItem;

#define DeleteControlButtonID 142
OnyxButton *DeleteControlButton;

#define SaveProjectButtonID 143
OnyxButton *SaveProjectButton;

#define ToolbarSeparatorID 144
OnyxIcon *ToolbarSeparator;
