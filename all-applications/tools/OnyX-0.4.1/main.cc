/**************************************************************************

   This file is part of OnyX, a visual development environment 
   using the xclass toolkit. Copyright (C) 1997, 1998 Frank Hall.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as published 
   by the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version. 

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
   Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   675 Mass Ave, Cambridge, MA 02139, USA.

**************************************************************************/

#include "main.h"

#define spacing 8

void ApplicationInitialize(int argc, char *argv[]);
extern void ApplicationPreload(int argc, char *argv[]);
extern void ApplicationLoad(int argc, char *argv[]);
extern void GeneralEvent(OnyxMessage *msg);

int debug;

#include <signal.h>

#define DEBUG_X
//#define DEBUG_X_HALT

#ifdef DEBUG_X
XErrorHandler errh(Display *dpy, XErrorEvent *e) {
  char text[1024];

  XGetErrorText(dpy, e->error_code, text, 1024);
  fprintf(stderr, "X error %u: %s,\n\tID=%#x, request %u, serial %lu\n",
          (int) e->error_code, text, (int) e->resourceid, 
          (int) e->request_code, e->serial);
#ifdef DEBUG_X_HALT
  raise(SIGSEGV);
  pause();
#endif
  return 0;
}
#endif

int main(int argc, char *argv[]) {
#ifdef DEBUG_X
  XSetErrorHandler((XErrorHandler) errh);
#endif

  ApplicationPreload(argc, argv);
  ApplicationInitialize(argc, argv);
  ApplicationLoad(argc, argv);

#ifdef DEBUG_X
  XSynchronize(ApplicationClient()->GetDisplay(),True);
#endif
  ApplicationRun();

  return 0;
}

void ApplicationInitialize(int argc, char *argv[]) {

  Initialize(ApplicationName, argc, argv);

  MainWindow = new OnyxWindow;
  MainWindow->ID = MainWindowID;
  MainWindow->Parent = ApplicationRoot();
  MainWindow->X = 0;
  MainWindow->Y = 0;
  MainWindow->Width = 630;
  MainWindow->Height = 51;
  strcpy(MainWindow->Name, "OnyX");
  strcpy(MainWindow->Class, "OnyX");
  strcpy(MainWindow->Resource, "OnyX");
  strcpy(MainWindow->BackgroundColor, "#c0c0c0");
  strcpy(MainWindow->Directory, OnyxIconLocation);
  MainWindow->MinimumHeight = 51;
  MainWindow->MaximumHeight = 51;
  MainWindow->Create();

  SplashScreen = new OnyxWindow;
  SplashScreen->ID = SplashScreenID;
  SplashScreen->Parent = ApplicationRoot();
  SplashScreen->X = 0;
  SplashScreen->Y = 0;
  SplashScreen->Width = 400;
  SplashScreen->Height = 240;
  strcpy(SplashScreen->Name, "Starting OnyX");
  strcpy(SplashScreen->Class, "OnyX");
  strcpy(SplashScreen->Resource, "OnyX");
  strcpy(SplashScreen->BackgroundColor, "#c0c0c0");
  SplashScreen->HasBorder = True;
  SplashScreen->HasTitleBar = False;
  SplashScreen->HasMenu = False;
  SplashScreen->HasMinimizeButton = False;
  SplashScreen->HasMaximizeButton = False;
  SplashScreen->HasMaximizeOption = False;
  SplashScreen->HasResizeOption = False;
  SplashScreen->HasMoveOption = True;
  SplashScreen->HasCloseOption = False;
  SplashScreen->MinimumWidth = 400;
  SplashScreen->MaximumWidth = 400;
  SplashScreen->MinimumHeight = 240;
  SplashScreen->MaximumHeight = 240;
  SplashScreen->Create();

  SplashPicture = new OnyxIcon;
  SplashPicture->ID = SplashPictureID;
  SplashPicture->Parent = SplashScreen->Self();
  SplashPicture->X = 0;
  SplashPicture->Y = 0;
  SplashPicture->Width = 400;
  SplashPicture->Height = 240;
  strcpy(SplashPicture->Picture, "onyx_intro.xpm");
  SplashPicture->Create();

  SplashTimer = new OnyxTimer;
  SplashTimer->ID = SplashTimerID;
  SplashTimer->Parent = SplashScreen->Self();
  SplashTimer->X = 1;
  SplashTimer->Y = 1;
  SplashTimer->Interval = 2000;
  SplashTimer->Create();

  MainMenuBar = new OnyxMenuBar;
  MainMenuBar->Parent = MainWindow->Self();
  MainMenuBar->ID = MainMenuBarID;
  MainMenuBar->X = 0;
  MainMenuBar->Y = 0;
  MainMenuBar->Width = 625;
  MainMenuBar->Height = 24;
  MainMenuBar->Create();

  FileMenu = new OnyxMenu;
  FileMenu->Parent = MainWindow->Self();
  FileMenu->MenuBarParent = MainMenuBar;
  FileMenu->ID = FileMenuID;
  FileMenu->IsEnabled = True;
  strcpy(FileMenu->Text, "&File");
  FileMenu->Create();

  RunMenu = new OnyxMenu;
  RunMenu->Parent = MainWindow->Self();
  RunMenu->MenuBarParent = MainMenuBar;
  RunMenu->ID = RunMenuID;
  RunMenu->IsEnabled = False;
  strcpy(RunMenu->Text, "&Run");
  RunMenu->Create();

  ViewMenu = new OnyxMenu;
  ViewMenu->Parent = MainWindow->Self();
  ViewMenu->MenuBarParent = MainMenuBar;
  ViewMenu->ID = ViewMenuID;
  ViewMenu->IsEnabled = False;
  strcpy(ViewMenu->Text, "&View");
  ViewMenu->Create();

  ToolsMenu = new OnyxMenu;
  ToolsMenu->Parent = MainWindow->Self();
  ToolsMenu->MenuBarParent = MainMenuBar;
  ToolsMenu->ID = ToolsMenuID;
  ToolsMenu->IsEnabled = False;
  strcpy(ToolsMenu->Text, "&Tools");
  ToolsMenu->Create();

  NewProjectMenuItem = new OnyxMenuItem;
  NewProjectMenuItem->Parent = MainWindow->Self();
  NewProjectMenuItem->MenuParent = FileMenu;
  NewProjectMenuItem->ID = NewProjectMenuItemID;
  strcpy(NewProjectMenuItem->Text, "&New Project");
  NewProjectMenuItem->Create();

  OpenProjectMenuItem = new OnyxMenuItem;
  OpenProjectMenuItem->Parent = MainWindow->Self();
  OpenProjectMenuItem->MenuParent = FileMenu;
  OpenProjectMenuItem->ID = OpenProjectMenuItemID;
  strcpy(OpenProjectMenuItem->Text, "&Open Project...");
  OpenProjectMenuItem->Create();

  FileMenuSep1 = new OnyxMenuItem;
  FileMenuSep1->Parent = MainWindow->Self();
  FileMenuSep1->MenuParent = FileMenu;
  FileMenuSep1->ID = FileMenuSep1ID;
  strcpy(FileMenuSep1->Text, "");
  FileMenuSep1->Create();

  SaveObjectMenuItem = new OnyxMenuItem;
  SaveObjectMenuItem->Parent = MainWindow->Self();
  SaveObjectMenuItem->MenuParent = FileMenu;
  SaveObjectMenuItem->ID = SaveObjectMenuItemID;
  strcpy(SaveObjectMenuItem->Text, "&Save Object");
  SaveObjectMenuItem->Create();

  SaveProjectMenuItem = new OnyxMenuItem;
  SaveProjectMenuItem->Parent = MainWindow->Self();
  SaveProjectMenuItem->MenuParent = FileMenu;
  SaveProjectMenuItem->ID = SaveProjectMenuItemID;
  strcpy(SaveProjectMenuItem->Text, "Sa&ve Project");
  SaveProjectMenuItem->Create();

  SaveProjectAsMenuItem = new OnyxMenuItem;
  SaveProjectAsMenuItem->Parent = MainWindow->Self();
  SaveProjectAsMenuItem->MenuParent = FileMenu;
  SaveProjectAsMenuItem->ID = SaveProjectAsMenuItemID;
  strcpy(SaveProjectAsMenuItem->Text, "Sav&e Project As...");
  SaveProjectAsMenuItem->Create();

  FileMenuSep2 = new OnyxMenuItem;
  FileMenuSep2->Parent = MainWindow->Self();
  FileMenuSep2->MenuParent = FileMenu;
  FileMenuSep2->ID = FileMenuSep2ID;
  strcpy(FileMenuSep2->Text, "");
  FileMenuSep2->Create();

  ImportObjectMenuItem = new OnyxMenuItem;
  ImportObjectMenuItem->Parent = MainWindow->Self();
  ImportObjectMenuItem->MenuParent = FileMenu;
  ImportObjectMenuItem->ID = ImportObjectMenuItemID;
  strcpy(ImportObjectMenuItem->Text, "I&mport Object...");
  ImportObjectMenuItem->IsEnabled = True;
  ImportObjectMenuItem->Create();

  ExportObjectMenuItem = new OnyxMenuItem;
  ExportObjectMenuItem->Parent = MainWindow->Self();
  ExportObjectMenuItem->MenuParent = FileMenu;
  ExportObjectMenuItem->ID = ExportObjectMenuItemID;
  strcpy(ExportObjectMenuItem->Text, "Ex&port Object...");
  ExportObjectMenuItem->IsEnabled = True;
  ExportObjectMenuItem->Create();

  FileMenuSep3 = new OnyxMenuItem;
  FileMenuSep3->Parent = MainWindow->Self();
  FileMenuSep3->MenuParent = FileMenu;
  FileMenuSep3->ID = FileMenuSep3ID;
  strcpy(FileMenuSep3->Text, "");
  FileMenuSep3->Create();

  ImportProjectMenuItem = new OnyxMenuItem;
  ImportProjectMenuItem->Parent = MainWindow->Self();
  ImportProjectMenuItem->MenuParent = FileMenu;
  ImportProjectMenuItem->ID = ImportProjectMenuItemID;
  strcpy(ImportProjectMenuItem->Text, "&Import Project...");
  ImportProjectMenuItem->IsEnabled = True;
  ImportProjectMenuItem->Create();

  ExportProjectMenuItem = new OnyxMenuItem;
  ExportProjectMenuItem->Parent = MainWindow->Self();
  ExportProjectMenuItem->MenuParent = FileMenu;
  ExportProjectMenuItem->ID = ExportProjectMenuItemID;
  strcpy(ExportProjectMenuItem->Text, "&Export Project...");
  ExportProjectMenuItem->IsEnabled = True;
  ExportProjectMenuItem->Create();

  FileMenuSep4 = new OnyxMenuItem;
  FileMenuSep4->Parent = MainWindow->Self();
  FileMenuSep4->MenuParent = FileMenu;
  FileMenuSep4->ID = FileMenuSep4ID;
  strcpy(FileMenuSep4->Text, "");
  FileMenuSep4->Create();

  CreateApplicationMenuItem = new OnyxMenuItem;
  CreateApplicationMenuItem->Parent = MainWindow->Self();
  CreateApplicationMenuItem->MenuParent = FileMenu;
  CreateApplicationMenuItem->ID = CreateApplicationMenuItemID;
  strcpy(CreateApplicationMenuItem->Text, "&Create Application...");
  CreateApplicationMenuItem->Create();

  FileMenuSep5 = new OnyxMenuItem;
  FileMenuSep5->Parent = MainWindow->Self();
  FileMenuSep5->MenuParent = FileMenu;
  FileMenuSep5->ID = FileMenuSep5ID;
  strcpy(FileMenuSep5->Text, "");
  FileMenuSep5->Create();

  ExitMenuItem = new OnyxMenuItem;
  ExitMenuItem->Parent = MainWindow->Self();
  ExitMenuItem->MenuParent = FileMenu;
  ExitMenuItem->ID = ExitMenuItemID;
  strcpy(ExitMenuItem->Text, "E&xit");
  ExitMenuItem->Create();

  ViewCodeMenuItem = new OnyxMenuItem;
  ViewCodeMenuItem->Parent = MainWindow->Self();
  ViewCodeMenuItem->MenuParent = ViewMenu;
  ViewCodeMenuItem->ID = ViewCodeMenuItemID;
  strcpy(ViewCodeMenuItem->Text, "&Code");
  ViewCodeMenuItem->Create();

  ViewWindowMenuItem = new OnyxMenuItem;
  ViewWindowMenuItem->Parent = MainWindow->Self();
  ViewWindowMenuItem->MenuParent = ViewMenu;
  ViewWindowMenuItem->ID = ViewWindowMenuItemID;
  strcpy(ViewWindowMenuItem->Text, "&Window");
  ViewWindowMenuItem->Create();

  ViewGridMenuItem = new OnyxMenuItem;
  ViewGridMenuItem->Parent = MainWindow->Self();
  ViewGridMenuItem->MenuParent = ViewMenu;
  ViewGridMenuItem->ID = ViewGridMenuItemID;
  strcpy(ViewGridMenuItem->Text, "&Grid");
  ViewGridMenuItem->IsChecked = True;
  ViewGridMenuItem->Create();

  ViewMenuSep1 = new OnyxMenuItem;
  ViewMenuSep1->Parent = MainWindow->Self();
  ViewMenuSep1->MenuParent = ViewMenu;
  ViewMenuSep1->ID = ViewMenuSep1ID;
  strcpy(ViewMenuSep1->Text, "");
  ViewMenuSep1->Create();

  ViewProjectMenuItem = new OnyxMenuItem;
  ViewProjectMenuItem->Parent = MainWindow->Self();
  ViewProjectMenuItem->MenuParent = ViewMenu;
  ViewProjectMenuItem->ID = ViewProjectMenuItemID;
  strcpy(ViewProjectMenuItem->Text, "&Project");
  ViewProjectMenuItem->Create();

  ViewPropertiesMenuItem = new OnyxMenuItem;
  ViewPropertiesMenuItem->Parent = MainWindow->Self();
  ViewPropertiesMenuItem->MenuParent = ViewMenu;
  ViewPropertiesMenuItem->ID = ViewPropertiesMenuItemID;
  strcpy(ViewPropertiesMenuItem->Text, "Propertie&s");
  ViewPropertiesMenuItem->Create();

  ViewMenuSep2 = new OnyxMenuItem;
  ViewMenuSep2->Parent = MainWindow->Self();
  ViewMenuSep2->MenuParent = ViewMenu;
  ViewMenuSep2->ID = ViewMenuSep2ID;
  strcpy(ViewMenuSep2->Text, "");
  ViewMenuSep2->Create();

  ToolBoxMenuItem = new OnyxMenuItem;
  ToolBoxMenuItem->Parent = MainWindow->Self();
  ToolBoxMenuItem->MenuParent = ViewMenu;
  ToolBoxMenuItem->ID = ToolBoxMenuItemID;
  strcpy(ToolBoxMenuItem->Text, "ToolBo&x");
  ToolBoxMenuItem->Create();

  ToolBarMenuItem = new OnyxMenuItem;
  ToolBarMenuItem->Parent = MainWindow->Self();
  ToolBarMenuItem->MenuParent = ViewMenu;
  ToolBarMenuItem->ID = ToolBarMenuItemID;
  ToolBarMenuItem->IsChecked = True;
  strcpy(ToolBarMenuItem->Text, "&ToolBar");
  ToolBarMenuItem->Create();

  MakeMenuItem = new OnyxMenuItem;
  MakeMenuItem->Parent = MainWindow->Self();
  MakeMenuItem->MenuParent = RunMenu;
  MakeMenuItem->ID = MakeMenuItemID;
  strcpy(MakeMenuItem->Text, "&Make");
  MakeMenuItem->Create();

  MakeCleanMenuItem = new OnyxMenuItem;
  MakeCleanMenuItem->Parent = MainWindow->Self();
  MakeCleanMenuItem->MenuParent = RunMenu;
  MakeCleanMenuItem->ID = MakeCleanMenuItemID;
  strcpy(MakeCleanMenuItem->Text, "Make &Clean");
  MakeCleanMenuItem->Create();

  MakeRunMenuItem = new OnyxMenuItem;
  MakeRunMenuItem->Parent = MainWindow->Self();
  MakeRunMenuItem->MenuParent = RunMenu;
  MakeRunMenuItem->ID = MakeRunMenuItemID;
  strcpy(MakeRunMenuItem->Text, "M&ake and Run");
  MakeRunMenuItem->Create();

  RunMenuSep1 = new OnyxMenuItem;
  RunMenuSep1->Parent = MainWindow->Self();
  RunMenuSep1->MenuParent = RunMenu;
  RunMenuSep1->ID = RunMenuSep1ID;
  strcpy(RunMenuSep1->Text, "");
  RunMenuSep1->Create();

  RunMenuItem = new OnyxMenuItem;
  RunMenuItem->Parent = MainWindow->Self();
  RunMenuItem->MenuParent = RunMenu;
  RunMenuItem->ID = RunMenuItemID;
  strcpy(RunMenuItem->Text, "&Run");
  RunMenuItem->Create();

  StopMenuItem = new OnyxMenuItem;
  StopMenuItem->Parent = MainWindow->Self();
  StopMenuItem->MenuParent = RunMenu;
  StopMenuItem->ID = StopMenuItemID;
  strcpy(StopMenuItem->Text, "&Stop");
  StopMenuItem->Create();

  ToolbarSeparator = new OnyxIcon;
  ToolbarSeparator->ID = ToolbarSeparatorID;
  ToolbarSeparator->Parent = MainWindow->Self();
  ToolbarSeparator->X = 0;
  ToolbarSeparator->Y = MainMenuBar->Height;
  ToolbarSeparator->Width = MainWindow->Width;
  ToolbarSeparator->Height = 2;
  ToolbarSeparator->BorderStyle = SunkenSingle;
  ToolbarSeparator->IsShaped = False;
  strcpy(ToolbarSeparator->Picture, "");
  ToolbarSeparator->Create();

  OpenButton = new OnyxButton;
  OpenButton->ID = OpenButtonID;
  OpenButton->Parent = MainWindow->Self();
  OpenButton->X = 0;
  OpenButton->Y = ToolbarSeparator->Y + ToolbarSeparator->Height + 3;
  OpenButton->Width = 23;
  OpenButton->Height = 22;
  strcpy(OpenButton->Picture, "onyx_openbutton.xpm");
  strcpy(OpenButton->ToolTip, "Open Project");
  OpenButton->EnableToolTip = True;
  OpenButton->Create();

  SaveButton = new OnyxButton;
  SaveButton->ID = SaveButtonID;
  SaveButton->Parent = MainWindow->Self();
  SaveButton->X = OpenButton->X + 23;
  SaveButton->Y = ToolbarSeparator->Y + ToolbarSeparator->Height + 3;
  SaveButton->Width = 23;
  SaveButton->Height = 22;
  SaveButton->IsEnabled = False;
  strcpy(SaveButton->Picture, "onyx_savebutton.xpm");
  strcpy(SaveButton->ToolTip, "Save Object");
  SaveButton->EnableToolTip = True;
  SaveButton->Create();

  SaveProjectButton = new OnyxButton;
  SaveProjectButton->ID = SaveProjectButtonID;
  SaveProjectButton->Parent = MainWindow->Self();
  SaveProjectButton->X = SaveButton->X + 23;
  SaveProjectButton->Y = ToolbarSeparator->Y + ToolbarSeparator->Height + 3;
  SaveProjectButton->Width = 23;
  SaveProjectButton->Height = 22;
  SaveProjectButton->IsEnabled = False;
  strcpy(SaveProjectButton->Picture, "onyx_saveprojectbutton.xpm");
  strcpy(SaveProjectButton->ToolTip, "Save Project");
  SaveProjectButton->EnableToolTip = True;
  SaveProjectButton->Create();

  NewWindowButton = new OnyxButton;
  NewWindowButton->ID = NewWindowButtonID;
  NewWindowButton->Parent = MainWindow->Self();
  NewWindowButton->X = SaveProjectButton->X + 23 + spacing;
  NewWindowButton->Y = ToolbarSeparator->Y + ToolbarSeparator->Height + 3;
  NewWindowButton->Width = 23;
  NewWindowButton->Height = 22;
  NewWindowButton->IsEnabled = False;
  strcpy(NewWindowButton->Picture, "onyx_newwindowbutton.xpm");
  strcpy(NewWindowButton->ToolTip, "Add New Window");
  NewWindowButton->EnableToolTip = True;
  NewWindowButton->Create();

  ViewWindowButton = new OnyxButton;
  ViewWindowButton->ID = ViewWindowButtonID;
  ViewWindowButton->Parent = MainWindow->Self();
  ViewWindowButton->X = NewWindowButton->X + 23 + spacing;
  ViewWindowButton->Y = ToolbarSeparator->Y + ToolbarSeparator->Height + 3;
  ViewWindowButton->Width = 23;
  ViewWindowButton->Height = 22;
  ViewWindowButton->IsEnabled = False;
  strcpy(ViewWindowButton->Picture, "onyx_viewwindowbutton.xpm");
  strcpy(ViewWindowButton->ToolTip, "View Window");
  ViewWindowButton->EnableToolTip = True;
  ViewWindowButton->Create();

  NextWindowButton = new OnyxButton;
  NextWindowButton->ID = NextWindowButtonID;
  NextWindowButton->Parent = MainWindow->Self();
  NextWindowButton->X = ViewWindowButton->X + 23;
  NextWindowButton->Y = ToolbarSeparator->Y + ToolbarSeparator->Height + 3;
  NextWindowButton->Width = 23;
  NextWindowButton->Height = 22;
  NextWindowButton->IsEnabled = False;
  strcpy(NextWindowButton->Picture, "onyx_nextwindowbutton.xpm");
  strcpy(NextWindowButton->ToolTip, "Next Window");
  NextWindowButton->EnableToolTip = True;
  NextWindowButton->Create();

  MakeButton = new OnyxButton;
  MakeButton->ID = MakeButtonID;
  MakeButton->Parent = MainWindow->Self();
  MakeButton->X = NextWindowButton->X + 23 + spacing;
  MakeButton->Y = ToolbarSeparator->Y + ToolbarSeparator->Height + 3;
  MakeButton->Width = 23;
  MakeButton->Height = 22;
  MakeButton->IsEnabled = False;
  strcpy(MakeButton->Picture, "onyx_makebutton.xpm");
  strcpy(MakeButton->ToolTip, "Make Project");
  MakeButton->EnableToolTip = True;
  MakeButton->Create();

  MakeRunButton = new OnyxButton;
  MakeRunButton->ID = MakeRunButtonID;
  MakeRunButton->Parent = MainWindow->Self();
  MakeRunButton->X = MakeButton->X + 23;
  MakeRunButton->Y = ToolbarSeparator->Y + ToolbarSeparator->Height + 3;
  MakeRunButton->Width = 23;
  MakeRunButton->Height = 22;
  MakeRunButton->IsEnabled = False;
  strcpy(MakeRunButton->Picture, "onyx_makerunbutton.xpm");
  strcpy(MakeRunButton->ToolTip, "Make and Run");
  MakeRunButton->EnableToolTip = True;
  MakeRunButton->Create();

  RunButton = new OnyxButton;
  RunButton->ID = RunButtonID;
  RunButton->Parent = MainWindow->Self();
  RunButton->X = MakeRunButton->X + 23;
  RunButton->Y = ToolbarSeparator->Y + ToolbarSeparator->Height + 3;
  RunButton->Width = 23;
  RunButton->Height = 22;
  RunButton->IsEnabled = False;
  strcpy(RunButton->Picture, "onyx_runbutton.xpm");
  strcpy(RunButton->ToolTip, "Run Application");
  RunButton->EnableToolTip = True;
  RunButton->Create();

  StopButton = new OnyxButton;
  StopButton->ID = StopButtonID;
  StopButton->Parent = MainWindow->Self();
  StopButton->X = RunButton->X + 23;
  StopButton->Y = ToolbarSeparator->Y + ToolbarSeparator->Height + 3;
  StopButton->Width = 23;
  StopButton->Height = 22;
  StopButton->IsEnabled = False;
  strcpy(StopButton->Picture, "onyx_stopbutton.xpm");
  strcpy(StopButton->ToolTip, "Stop Application");
  StopButton->EnableToolTip = True;
  StopButton->Create();

  ShowCodeButton = new OnyxButton;
  ShowCodeButton->ID = ShowCodeButtonID;
  ShowCodeButton->Parent = MainWindow->Self();
  ShowCodeButton->X = StopButton->X + 23 + spacing;
  ShowCodeButton->Y = ToolbarSeparator->Y + ToolbarSeparator->Height + 3;
  ShowCodeButton->Width = 23;
  ShowCodeButton->Height = 22;
  ShowCodeButton->IsEnabled = False;
  strcpy(ShowCodeButton->Picture, "onyx_showcodebutton.xpm");
  strcpy(ShowCodeButton->ToolTip, "Show Code for Object");
  ShowCodeButton->EnableToolTip = True;
  ShowCodeButton->Create();

  DeleteControlButton = new OnyxButton;
  DeleteControlButton->ID = DeleteControlButtonID;
  DeleteControlButton->Parent = MainWindow->Self();
  DeleteControlButton->X = ShowCodeButton->X + 23 + spacing;
  DeleteControlButton->Y = ToolbarSeparator->Y + ToolbarSeparator->Height + 3;
  DeleteControlButton->Width = 23;
  DeleteControlButton->Height = 22;
  DeleteControlButton->IsSticky = False;
  DeleteControlButton->IsEnabled = False;
  strcpy(DeleteControlButton->Picture, "onyx_deletebutton.xpm");
  strcpy(DeleteControlButton->ToolTip, "Delete Control");
  DeleteControlButton->EnableToolTip = True;
  DeleteControlButton->Create();

  ShowPropertiesButton = new OnyxButton;
  ShowPropertiesButton->ID = ShowPropertiesButtonID;
  ShowPropertiesButton->Parent = MainWindow->Self();
  ShowPropertiesButton->X = DeleteControlButton->X + 23;
  ShowPropertiesButton->Y = ToolbarSeparator->Y + ToolbarSeparator->Height + 3;
  ShowPropertiesButton->Width = 23;
  ShowPropertiesButton->Height = 22;
  ShowPropertiesButton->IsEnabled = False;
  ShowPropertiesButton->IsSticky = False;
  ShowPropertiesButton->IsToggle = True;
  strcpy(ShowPropertiesButton->Picture, "onyx_propertiesbutton.xpm");
  strcpy(ShowPropertiesButton->ToolTip, "Show Properties for Object");
  ShowPropertiesButton->EnableToolTip = True;
  ShowPropertiesButton->Create();

  LockControlsButton = new OnyxButton;
  LockControlsButton->ID = LockControlsButtonID;
  LockControlsButton->Parent = MainWindow->Self();
  LockControlsButton->X = ShowPropertiesButton->X + 23;
  LockControlsButton->Y = ToolbarSeparator->Y + ToolbarSeparator->Height + 3;
  LockControlsButton->Width = 23;
  LockControlsButton->Height = 22;
  LockControlsButton->IsSticky = False;
  LockControlsButton->IsToggle = True;
  LockControlsButton->IsEnabled = False;
  strcpy(LockControlsButton->Picture, "onyx_lockcontrolsbutton.xpm");
  strcpy(LockControlsButton->ToolTip, "Lock Controls");
  LockControlsButton->EnableToolTip = True;
  LockControlsButton->Create();

  SnapToGrid = new OnyxButton;
  SnapToGrid->ID = SnapToGridID;
  SnapToGrid->Parent = MainWindow->Self();
  SnapToGrid->X = LockControlsButton->X + 23;
  SnapToGrid->Y = ToolbarSeparator->Y + ToolbarSeparator->Height + 3;
  SnapToGrid->Width = 23;
  SnapToGrid->Height = 22;
  SnapToGrid->IsSticky = False;
  SnapToGrid->IsToggle = True;
  strcpy(SnapToGrid->Picture, "onyx_snapbutton.xpm");
  strcpy(SnapToGrid->ToolTip, "Snap to Grid");
  SnapToGrid->EnableToolTip = True;
  SnapToGrid->Create();

  ShowToolboxButton = new OnyxButton;
  ShowToolboxButton->ID = ShowToolboxButtonID;
  ShowToolboxButton->Parent = MainWindow->Self();
  ShowToolboxButton->X = SnapToGrid->X + 23;
  ShowToolboxButton->Y = ToolbarSeparator->Y + ToolbarSeparator->Height + 3;
  ShowToolboxButton->Width = 23;
  ShowToolboxButton->Height = 22;
  ShowToolboxButton->IsEnabled = False;
  ShowToolboxButton->IsSticky = False;
  ShowToolboxButton->IsToggle = True;
  strcpy(ShowToolboxButton->Picture, "onyx_showtoolboxbutton.xpm");
  strcpy(ShowToolboxButton->ToolTip, "Toolbox");
  ShowToolboxButton->EnableToolTip = True;
  ShowToolboxButton->Create();

  ProjectExplorerButton = new OnyxButton;
  ProjectExplorerButton->ID = ProjectExplorerButtonID;
  ProjectExplorerButton->Parent = MainWindow->Self();
  ProjectExplorerButton->X = ShowToolboxButton->X + 23 + spacing;
  ProjectExplorerButton->Y = ToolbarSeparator->Y + ToolbarSeparator->Height + 3;
  ProjectExplorerButton->Width = 23;
  ProjectExplorerButton->Height = 22;
  ProjectExplorerButton->IsSticky = False;
  ProjectExplorerButton->IsToggle = True;
  ProjectExplorerButton->IsEnabled = False;
  strcpy(ProjectExplorerButton->Picture, "onyx_explorerbutton.xpm");
  strcpy(ProjectExplorerButton->ToolTip, "Project Explorer");
  ProjectExplorerButton->EnableToolTip = True;
  ProjectExplorerButton->Create();

  OpenFileDialog = new OnyxFileDialog;
  OpenFileDialog->ID = OpenFileDialogID;
  OpenFileDialog->Parent = MainWindow->Self();
  OpenFileDialog->DisplayWindow = ApplicationRoot();
  OpenFileDialog->DialogType = OnyxOpenFileDialog;
  OpenFileDialog->Create();

  SaveFileDialog = new OnyxFileDialog;
  SaveFileDialog->ID = SaveFileDialogID;
  SaveFileDialog->Parent = MainWindow->Self();
  SaveFileDialog->DisplayWindow = ApplicationRoot();
  SaveFileDialog->DialogType = OnyxSaveFileDialog;
  OpenFileDialog->Create();

  /* I don't think I use this anymore. */
  CloseMessageBox = new OnyxMsgBox;
  CloseMessageBox->ID = CloseMessageBoxID;
  CloseMessageBox->Parent = MainWindow->Self();
  CloseMessageBox->DisplayWindow = ApplicationRoot();
  strcpy(CloseMessageBox->Message, "Close Onyx?");
  CloseMessageBox->Icon = QuestionIcon;
  CloseMessageBox->AddButton(YesButton);
  CloseMessageBox->AddButton(NoButton);

  ExitSaveMessageBox = new OnyxMsgBox;
  ExitSaveMessageBox->ID = ExitSaveMessageBoxID;
  ExitSaveMessageBox->Parent = MainWindow->Self();
  ExitSaveMessageBox->DisplayWindow = ApplicationRoot();
  strcpy(ExitSaveMessageBox->Message, "Save Changes?");
  ExitSaveMessageBox->Icon = QuestionIcon;
  ExitSaveMessageBox->AddButton(YesButton);
  ExitSaveMessageBox->AddButton(NoButton);
  ExitSaveMessageBox->AddButton(CancelButton);

  ProjectExplorer = new OnyxWindow;
  ProjectExplorer->ID = ProjectExplorerID;
  ProjectExplorer->Parent = ApplicationRoot();
  ProjectExplorer->X = 385;
  ProjectExplorer->Y = 130;
  ProjectExplorer->Width = 250;
  ProjectExplorer->Height = 295;
  strcpy(ProjectExplorer->Name, "Project Explorer");
  strcpy(ProjectExplorer->Class, "OnyX");
  strcpy(ProjectExplorer->Resource, "OnyX");
  strcpy(ProjectExplorer->BackgroundColor, "#ffffff");
  ProjectExplorer->Create();

  PropertiesWindow = new OnyxWindow;
  PropertiesWindow->ID = PropertiesWindowID;
  PropertiesWindow->Parent = ApplicationRoot();
  PropertiesWindow->X = 350;
  PropertiesWindow->Y = 120;
  PropertiesWindow->Width = 275;
  PropertiesWindow->Height = 295;
  strcpy(PropertiesWindow->Name, "Properties");
  strcpy(PropertiesWindow->Class, "OnyX");
  strcpy(PropertiesWindow->Resource, "OnyX");
  PropertiesWindow->CloseDestroysWindow = False;
  strcpy(PropertiesWindow->BackgroundColor, "#c0c0c0");
  PropertiesWindow->Create();

  Toolbox = new OnyxWindow;
  Toolbox->ID = ToolboxID;
  Toolbox->Parent = ApplicationRoot();
  Toolbox->X = 420;
  Toolbox->Y = 125;
  Toolbox->Width = 97;
  Toolbox->Height = 295;
  strcpy(Toolbox->Name, "Toolbox");
  strcpy(Toolbox->Class, "OnyX");
  strcpy(Toolbox->Resource, "OnyX");
  strcpy(Toolbox->BackgroundColor, "#c0c0c0");
  strcpy(Toolbox->Directory, OnyxLocation);
  strcat(Toolbox->Directory, "/ObjectRepository");
  Toolbox->ResizeWidthIncrement = 32;
  Toolbox->ResizeHeightIncrement = 32;
  Toolbox->Create();

  MenuEditor = new OnyxWindow;
  MenuEditor->ID = MenuEditorID;
  MenuEditor->Parent = ApplicationRoot();
  MenuEditor->X = 385;
  MenuEditor->Y = 130;
  MenuEditor->Width = 250;
  MenuEditor->Height = 290;
  strcpy(MenuEditor->Name, "Menu Editor");
  strcpy(MenuEditor->Class, "OnyX");
  strcpy(MenuEditor->Resource, "OnyX");
  strcpy(MenuEditor->BackgroundColor, "#c0c0c0");
  MenuEditor->Create();

  MenuEditor2 = new OnyxWindow;
  MenuEditor2->ID = MenuEditor2ID;
  MenuEditor2->Parent = ApplicationRoot();
  MenuEditor2->X = 0;
  MenuEditor2->Y = 0;
  MenuEditor2->Width = 275;
  MenuEditor2->Height = 140;
  strcpy(MenuEditor2->Name, "Menu Questions");
  strcpy(MenuEditor2->Class, "OnyX");
  strcpy(MenuEditor2->Resource, "OnyX");
  strcpy(MenuEditor2->BackgroundColor, "#c0c0c0");
  MenuEditor2->Create();

  MakeWindow = new OnyxWindow;
  MakeWindow->ID = MakeWindowID;
  MakeWindow->Parent = ApplicationRoot();
  MakeWindow->X = 55;
  MakeWindow->Y = 100;
  MakeWindow->Width = 540;
  MakeWindow->Height = 300;
  MakeWindow->ButtonReleaseActive = False;
  MakeWindow->PointerMotionActive = False;
  strcpy(MakeWindow->Name, "Making Project");
  strcpy(MakeWindow->Class, "OnyX");
  strcpy(MakeWindow->Resource, "OnyX");
  strcpy(MakeWindow->BackgroundColor, "#c0c0c0");
  MakeWindow->Create();

  MakeTimer = new OnyxTimer;
  MakeTimer->ID = MakeTimerID;
  MakeTimer->Parent = MakeWindow->Self();
  MakeTimer->X = 1;
  MakeTimer->Y = 1;
  MakeTimer->Interval = 100;
  MakeTimer->Create();

  EditTimer = new OnyxTimer;
  EditTimer->ID = EditTimerID;
  EditTimer->Parent = MainWindow->Self();
  EditTimer->X = 1;
  EditTimer->Y = 1;
  EditTimer->Interval = 100;
  EditTimer->Create();

  MakeText = new OnyxTextCanvas;
  MakeText->ID = MakeTextID;
  MakeText->Parent = MakeWindow->Self();
  MakeText->X = 0;
  MakeText->Y = 0;
  MakeText->Width = MakeWindow->Width;
  MakeText->Height = MakeWindow->Height;
  MakeText->Create();

  ScreenInfo = new OnyxScreen;
  ScreenInfo->ID = ScreenInfoID;
  ScreenInfo->Parent = MainWindow->Self();
  ScreenInfo->Create();

  NewProjectWindow = new OnyxWindow;
  NewProjectWindow->ID = NewProjectWindowID;
  NewProjectWindow->Parent = ApplicationRoot();
  NewProjectWindow->X = ScreenInfo->Width / 2 - 137;
  NewProjectWindow->Y = ScreenInfo->Height / 2 - 65;
  NewProjectWindow->Width = 274;
  NewProjectWindow->Height = 140;
  strcpy(NewProjectWindow->Name, "Start new project.");
  strcpy(NewProjectWindow->Class, "OnyX");
  strcpy(NewProjectWindow->Resource, "OnyX");
  strcpy(NewProjectWindow->BackgroundColor, "#c0c0c0");
  NewProjectWindow->Create();

  NewProjectLabel = new OnyxLabel;
  NewProjectLabel->ID = NewProjectLabelID;
  NewProjectLabel->Parent = NewProjectWindow->Self();
  NewProjectLabel->X = 45;
  NewProjectLabel->Y = 20;
  NewProjectLabel->Width = 200;
  NewProjectLabel->Height = 20;
  strcpy(NewProjectLabel->Text, "Enter name of new application.");
  NewProjectLabel->Create();

  NewProjectName = new OnyxTextBox;
  NewProjectName->ID = NewProjectNameID;
  NewProjectName->Parent = NewProjectWindow->Self();
  NewProjectName->Size = 255;
  NewProjectName->X = 45;
  NewProjectName->Y = 60;
  NewProjectName->Width = 200;
  NewProjectName->Height = 22;
  NewProjectName->QuickFocusRelease = False;
  NewProjectName->Create();

  NewProjectOpenButton = new OnyxButton;
  NewProjectOpenButton->ID = NewProjectOpenButtonID;
  NewProjectOpenButton->Parent = NewProjectWindow->Self();
  NewProjectOpenButton->X = 8;
  NewProjectOpenButton->Y = 105;
  NewProjectOpenButton->Width = 120;
  NewProjectOpenButton->Height = 25;
  strcpy(NewProjectOpenButton->Text, "Start New Project");
  NewProjectOpenButton->Create();

  NewProjectCancelButton = new OnyxButton;
  NewProjectCancelButton->ID = NewProjectCancelButtonID;
  NewProjectCancelButton->Parent = NewProjectWindow->Self();
  NewProjectCancelButton->X = 146;
  NewProjectCancelButton->Y = 105;
  NewProjectCancelButton->Width = 120;
  NewProjectCancelButton->Height = 25;
  strcpy(NewProjectCancelButton->Text, "Cancel");
  NewProjectCancelButton->Create();

  MenuEditorItem = new OnyxMenuItem;
  MenuEditorItem->Parent = MainWindow->Self();
  MenuEditorItem->MenuParent = ToolsMenu;
  MenuEditorItem->ID = MenuEditorItemID;
  MenuEditorItem->IsEnabled = False;
  strcpy(MenuEditorItem->Text, "&Menu Editor...");
  MenuEditorItem->Create();

  CustomControlsItem = new OnyxMenuItem;
  CustomControlsItem->Parent = MainWindow->Self();
  CustomControlsItem->MenuParent = ToolsMenu;
  CustomControlsItem->ID = CustomControlsItemID;
  strcpy(CustomControlsItem->Text, "&Custom Controls...");
  CustomControlsItem->Create();

  ToolsMenuSep1 = new OnyxMenuItem;
  ToolsMenuSep1->Parent = MainWindow->Self();
  ToolsMenuSep1->MenuParent = ToolsMenu;
  ToolsMenuSep1->ID = ToolsMenuSep1ID;
  strcpy(ToolsMenuSep1->Text, "");
  ToolsMenuSep1->Create();

  OptionsItem = new OnyxMenuItem;
  OptionsItem->Parent = MainWindow->Self();
  OptionsItem->MenuParent = ToolsMenu;
  OptionsItem->ID = OptionsItemID;
  strcpy(OptionsItem->Text, "&Options...");
  OptionsItem->Create();

  CustomControlsWindow = new OnyxWindow;
  CustomControlsWindow->ID = CustomControlsWindowID;
  CustomControlsWindow->Parent = ApplicationRoot();
  CustomControlsWindow->X = ScreenInfo->Width / 2 - 150;
  CustomControlsWindow->Y = ScreenInfo->Height / 2 - 150;
  CustomControlsWindow->Width = 290;
  CustomControlsWindow->Height = 262;
  strcpy(CustomControlsWindow->Name, "Custom Controls");
  strcpy(CustomControlsWindow->Class, "OnyX");
  strcpy(CustomControlsWindow->Resource, "OnyX");
  strcpy(CustomControlsWindow->BackgroundColor, "#c0c0c0");
  CustomControlsWindow->Create();

  CCHelp = new OnyxButton;
  CCHelp->ID = CCHelpID;
  CCHelp->Parent = CustomControlsWindow->Self();
  CCHelp->X = 200;
  CCHelp->Y = 60;
  CCHelp->Width = 75;
  CCHelp->Height = 25;
  strcpy(CCHelp->Text, "Information");
  CCHelp->Create();

  CCRecompile = new OnyxButton;
  CCRecompile->ID = CCRecompileID;
  CCRecompile->Parent = CustomControlsWindow->Self();
  CCRecompile->X = 200;
  CCRecompile->Y = 160;
  CCRecompile->Width = 75;
  CCRecompile->Height = 25;
  strcpy(CCRecompile->Text,"Recompile");
  CCRecompile->Create();

  CCHelpWindow = new OnyxWindow;
  CCHelpWindow->ID = CCHelpWindowID;
  CCHelpWindow->Parent = ApplicationRoot();
  CCHelpWindow->X = ScreenInfo->Width / 2 - 150;
  CCHelpWindow->Y = ScreenInfo->Height / 2 - 150;
  CCHelpWindow->Width = 300;
  CCHelpWindow->Height = 290;
  strcpy(CCHelpWindow->Name,"Information on custom control");
  strcpy(CCHelpWindow->Class,"OnyX");
  strcpy(CCHelpWindow->Resource,"OnyX");
  strcpy(CCHelpWindow->BackgroundColor,"#c0c0c0");
  CCHelpWindow->Create();

  CCHelpOK = new OnyxButton;
  CCHelpOK->ID = CCHelpOKID;
  CCHelpOK->Parent = CCHelpWindow->Self();
  CCHelpOK->X = 115;
  CCHelpOK->Y = 250;
  CCHelpOK->Width = 70;
  CCHelpOK->Height = 25;
  strcpy(CCHelpOK->Text,"OK");
  CCHelpOK->Create();

  OptionsWindow = new OnyxWindow;
  OptionsWindow->ID = OptionsWindowID;
  OptionsWindow->Parent = ApplicationRoot();
  OptionsWindow->X = ScreenInfo->Width / 2 - 150;
  OptionsWindow->Y = ScreenInfo->Height / 2 - 150;
  OptionsWindow->Width = 300;
  OptionsWindow->Height = 290;
  strcpy(OptionsWindow->Name,"Options");
  strcpy(OptionsWindow->Class,"OnyX");
  strcpy(OptionsWindow->Resource,"OnyX");
  strcpy(OptionsWindow->BackgroundColor,"#c0c0c0");
  OptionsWindow->Create();

  OLocationLabel = new OnyxLabel;
  OLocationLabel->ID = OLocationLabelID;
  OLocationLabel->Parent = OptionsWindow->Self();
  OLocationLabel->X = 100;
  OLocationLabel->Y = 25;
  OLocationLabel->Width = 100;
  OLocationLabel->Height = 20;
  strcpy(OLocationLabel->Text,"Onyx Location:");
  OLocationLabel->Create();

  OLocationText = new OnyxTextBox;
  OLocationText->ID = OLocationTextID;
  OLocationText->Parent = OptionsWindow->Self();
  OLocationText->X = 10;
  OLocationText->Y = 50;
  OLocationText->Width = 280;
  OLocationText->Height = 22;
  OLocationText->Create();

  OEditorLabel = new OnyxLabel;
  OEditorLabel->ID = OEditorLabelID;
  OEditorLabel->Parent = OptionsWindow->Self();
  OEditorLabel->X = 100;
  OEditorLabel->Y = 100;
  OEditorLabel->Width = 100;
  OEditorLabel->Height = 20;
  strcpy(OEditorLabel->Text,"Editor Command:");
  OEditorLabel->Create();

  OEditorText = new OnyxTextBox;
  OEditorText->ID = OEditorTextID;
  OEditorText->Parent = OptionsWindow->Self();
  OEditorText->X = 10;
  OEditorText->Y = 125;
  OEditorText->Width = 280;
  OEditorText->Height = 22;
  OEditorText->Create();

  SnapToGridDefault = new OnyxCheckBox;
  SnapToGridDefault->ID = SnapToGridDefaultID;
  SnapToGridDefault->Parent = OptionsWindow->Self();
  SnapToGridDefault->X = 40;
  SnapToGridDefault->Y = 175;
  SnapToGridDefault->Width = 280;
  SnapToGridDefault->Height = 20;
  SnapToGridDefault->IsChecked = True;
  SnapToGridDefault->IsEnabled = True;
  strcpy(SnapToGridDefault->Text, "Enable Snap To Grid By Default");
  SnapToGridDefault->Create();

  OSaveButton = new OnyxButton;
  OSaveButton->ID = OSaveButtonID;
  OSaveButton->Parent = OptionsWindow->Self();
  OSaveButton->X = 50;
  OSaveButton->Y = 250;
  OSaveButton->Width = 75;
  OSaveButton->Height = 25;
  strcpy(OSaveButton->Text,"Save");
  OSaveButton->Create();

  OCancelButton = new OnyxButton;
  OCancelButton->ID = OCancelButtonID;
  OCancelButton->Parent = OptionsWindow->Self();
  OCancelButton->X = 175;
  OCancelButton->Y = 250;
  OCancelButton->Width = 75;
  OCancelButton->Height = 25;
  strcpy(OCancelButton->Text,"Cancel");
  OCancelButton->Create();

  ExplorerTree = new OnyxTree;
  ExplorerTree->ID = ExplorerTreeID;
  ExplorerTree->Parent = ProjectExplorer->Self();
  ExplorerTree->X = 0;
  ExplorerTree->Y = 0;
  ExplorerTree->Width = ProjectExplorer->Width;
  ExplorerTree->Height = ProjectExplorer->Height;
  ExplorerTree->Create();

  MenuTree = new OnyxTree;
  MenuTree->ID = MenuTreeID;
  MenuTree->Parent = MenuEditor->Self();
  MenuTree->X = 5;
  MenuTree->Y = 60;
  MenuTree->Width = MenuEditor->Width - MenuTree->X - 5;
  MenuTree->Height = MenuEditor->Height - MenuTree->Y - 5;
  MenuTree->Create();

  MakeMessageBox = new OnyxMsgBox;
  MakeMessageBox->ID = MakeMessageBoxID;
  MakeMessageBox->Parent = MainWindow->Self();
  MakeMessageBox->DisplayWindow = ApplicationRoot();
  strcpy(MakeMessageBox->Message,"Project has never been initialy saved. Take care of that now?");
  MakeMessageBox->Icon = QuestionIcon;
  MakeMessageBox->AddButton(YesButton);
  MakeMessageBox->AddButton(NoButton);
/*  MakeMessageBox->AddButton(CancelButton); */

  AboutWindow = new OnyxWindow;
  AboutWindow->ID = AboutWindowID;
  AboutWindow->Parent = ApplicationRoot();
  AboutWindow->X = 0;
  AboutWindow->Y = 0;
  AboutWindow->Width = 186;
  AboutWindow->Height = 248;
  strcpy(AboutWindow->Name,"About OnyX");
  strcpy(AboutWindow->Class,"OnyX");
  strcpy(AboutWindow->Resource,"OnyX");
  strcpy(AboutWindow->BackgroundColor,"#c0c0c0");
  AboutWindow->HasMinimizeButton = False;
  AboutWindow->HasMaximizeButton = False;
  AboutWindow->HasMaximizeOption = False;
  AboutWindow->HasResizeOption = False;
  AboutWindow->Create();

  HelpMenu = new OnyxMenu;
  HelpMenu->Parent = MainWindow->Self();
  HelpMenu->MenuBarParent = MainMenuBar;
  HelpMenu->ID = HelpMenuID;
  HelpMenu->IsEnabled = True;
  strcpy(HelpMenu->Text,"&Help");
  HelpMenu->Create();

  ContentsMenuItem = new OnyxMenuItem;
  ContentsMenuItem->Parent = MainWindow->Self();
  ContentsMenuItem->MenuParent = HelpMenu;
  ContentsMenuItem->ID = ContentsMenuItemID;
  ContentsMenuItem->IsEnabled = False;
  strcpy(ContentsMenuItem->Text,"&Contents");
  ContentsMenuItem->Create();

  SearchMenuItem = new OnyxMenuItem;
  SearchMenuItem->Parent = MainWindow->Self();
  SearchMenuItem->MenuParent = HelpMenu;
  SearchMenuItem->ID = SearchMenuItemID;
  SearchMenuItem->IsEnabled = False;
  strcpy(SearchMenuItem->Text,"&Search");
  SearchMenuItem->Create();

  HelpMenuSep1 = new OnyxMenuItem;
  HelpMenuSep1->Parent = MainWindow->Self();
  HelpMenuSep1->MenuParent = HelpMenu;
  HelpMenuSep1->ID = FileMenuSep1ID;
  strcpy(HelpMenuSep1->Text,"");
  HelpMenuSep1->Create();

  AboutMenuItem = new OnyxMenuItem;
  AboutMenuItem->Parent = MainWindow->Self();
  AboutMenuItem->MenuParent = HelpMenu;
  AboutMenuItem->ID = AboutMenuItemID;
  strcpy(AboutMenuItem->Text,"&About OnyX");
  AboutMenuItem->Create();

  AboutPicture = new OnyxIcon;
  AboutPicture->ID = AboutPictureID;
  AboutPicture->Parent = AboutWindow->Self();
  AboutPicture->X = 15;
  AboutPicture->Y = 15;
  AboutPicture->Width = 155;
  AboutPicture->Height = 93;
  strcpy(AboutPicture->Picture,"onyx_small.xpm");
  AboutPicture->Create();

  AboutLabel1 = new OnyxLabel;
  AboutLabel1->ID = AboutLabel1ID;
  AboutLabel1->Parent = AboutWindow->Self();
  AboutLabel1->X = 15;
  AboutLabel1->Y = 113;
  AboutLabel1->Width = 155;
  AboutLabel1->Height = 20;
  strcpy(AboutLabel1->Text,"");
  AboutLabel1->Create();

  AboutLabel2 = new OnyxLabel;
  AboutLabel2->ID = AboutLabel2ID;
  AboutLabel2->Parent = AboutWindow->Self();
  AboutLabel2->X = 15;
  AboutLabel2->Y = 133;
  AboutLabel2->Width = 155;
  AboutLabel2->Height = 20;
  strcpy(AboutLabel2->Text,"Written by");
  AboutLabel2->Create();

  AboutLabel3 = new OnyxLabel;
  AboutLabel3->ID = AboutLabel3ID;
  AboutLabel3->Parent = AboutWindow->Self();
  AboutLabel3->X = 15;
  AboutLabel3->Y = 153;
  AboutLabel3->Width = 155;
  AboutLabel3->Height = 20;
  strcpy(AboutLabel3->Text,"Frank Hall");
  AboutLabel3->Create();

  AboutLabel4 = new OnyxLabel;
  AboutLabel4->ID = AboutLabel4ID;
  AboutLabel4->Parent = AboutWindow->Self();
  AboutLabel4->X = 15;
  AboutLabel4->Y = 193;
  AboutLabel4->Width = 155;
  AboutLabel4->Height = 20;
  strcpy(AboutLabel4->Text,"Compiled with Xclass");
  AboutLabel4->Create();

  AboutLabel5 = new OnyxLabel;
  AboutLabel5->ID = AboutLabel5ID;
  AboutLabel5->Parent = AboutWindow->Self();
  AboutLabel5->X = 15;
  AboutLabel5->Y = 213;
  AboutLabel5->Width = 155;
  AboutLabel5->Height = 20;
  strcpy(AboutLabel5->Text,"");
  AboutLabel5->Create();

  SnapToGridMenuItem = new OnyxMenuItem;
  SnapToGridMenuItem->Parent = MainWindow->Self();
  SnapToGridMenuItem->MenuParent = ToolsMenu;
  SnapToGridMenuItem->ID = SnapToGridMenuItemID;
  SnapToGridMenuItem->IsEnabled = True;
  strcpy(SnapToGridMenuItem->Text,"&Snap To Grid");
  SnapToGridMenuItem->Create();

  MenuAdd = new OnyxButton;
  MenuAdd->ID = MenuAddID;
  MenuAdd->Parent = MenuEditor->Self();
  MenuAdd->X = 20;
  MenuAdd->Y = 5;
  MenuAdd->Width = 90;
  MenuAdd->Height = 20;
  strcpy(MenuAdd->Text,"Add Menu");
  MenuAdd->Create();

  MenuDelete = new OnyxButton;
  MenuDelete->ID = MenuDeleteID;
  MenuDelete->Parent = MenuEditor->Self();
  MenuDelete->X = 20;
  MenuDelete->Y = 30;
  MenuDelete->Width = 90;
  MenuDelete->Height = 20;
  strcpy(MenuDelete->Text,"Remove Menu");
  MenuDelete->Create();

  MenuItemAdd = new OnyxButton;
  MenuItemAdd->ID = MenuItemAddID;
  MenuItemAdd->Parent = MenuEditor->Self();
  MenuItemAdd->X = 120;
  MenuItemAdd->Y = 5;
  MenuItemAdd->Width = 90;
  MenuItemAdd->Height = 20;
  strcpy(MenuItemAdd->Text,"Add Entry");
  MenuItemAdd->Create();

  MenuItemDelete = new OnyxButton;
  MenuItemDelete->ID = MenuItemDeleteID;
  MenuItemDelete->Parent = MenuEditor->Self();
  MenuItemDelete->X = 120;
  MenuItemDelete->Y = 30;
  MenuItemDelete->Width = 90;
  MenuItemDelete->Height = 20;
  strcpy(MenuItemDelete->Text,"Remove Entry");
  MenuItemDelete->Create();

  MenuEditor2Ok = new OnyxButton;
  MenuEditor2Ok->ID = MenuEditor2OkID;
  MenuEditor2Ok->Parent = MenuEditor2->Self();
  MenuEditor2Ok->X = 35;
  MenuEditor2Ok->Y = 100;
  MenuEditor2Ok->Width = 75;
  MenuEditor2Ok->Height = 25;
  strcpy(MenuEditor2Ok->Text,"Ok");
  MenuEditor2Ok->Create();

  MenuEditor2Cancel = new OnyxButton;
  MenuEditor2Cancel->ID = MenuEditor2CancelID;
  MenuEditor2Cancel->Parent = MenuEditor2->Self();
  MenuEditor2Cancel->X = 165;
  MenuEditor2Cancel->Y = 100;
  MenuEditor2Cancel->Width = 75;
  MenuEditor2Cancel->Height = 25;
  strcpy(MenuEditor2Cancel->Text,"Cancel");
  MenuEditor2Cancel->Create();

  MenuEditor2Label = new OnyxLabel;
  MenuEditor2Label->ID = MenuEditor2LabelID;
  MenuEditor2Label->Parent = MenuEditor2->Self();
  MenuEditor2Label->X = 45;
  MenuEditor2Label->Y = 20;
  MenuEditor2Label->Width = 190;
  MenuEditor2Label->Height = 20;
  strcpy(MenuEditor2Label->Text,"Name of new menu item?");
  MenuEditor2Label->Create();

  MenuEditor2TextBox = new OnyxTextBox;
  MenuEditor2TextBox->ID = MenuEditor2TextBoxID;
  MenuEditor2TextBox->Parent = MenuEditor2->Self();
  MenuEditor2TextBox->X = 40;
  MenuEditor2TextBox->Y = 55;
  MenuEditor2TextBox->Width = 195;
  MenuEditor2TextBox->Height = 22;
  MenuEditor2TextBox->Size = 255;
  MenuEditor2TextBox->Create();

  CCScroll = new OnyxVScrollBar();
  CCScroll->ID = CCScrollID;
  CCScroll->Parent = CustomControlsWindow->Self();
  CCScroll->X = 170;
  CCScroll->Y = 15;
  CCScroll->Width = 16;
  CCScroll->Height = 232;
  CCScroll->Range = 100;
  CCScroll->PageSize = 10;
  CCScroll->Position = 0;
  CCScroll->Create();

  XClassWarning = new OnyxWindow;
  XClassWarning->ID = XClassWarningID;
  XClassWarning->Parent = ApplicationRoot();
  XClassWarning->X = 0;
  XClassWarning->Y = 0;
  XClassWarning->Width = 200;
  XClassWarning->Height = 248;
  strcpy(XClassWarning->Name, "OnyX - Warning");
  strcpy(XClassWarning->Class, "OnyX");
  strcpy(XClassWarning->Resource, "OnyX");
  strcpy(XClassWarning->BackgroundColor, "#c0c0c0");
  XClassWarning->HasMinimizeButton = False;
  XClassWarning->HasMaximizeButton = False;
  XClassWarning->HasMaximizeOption = False;
  XClassWarning->HasResizeOption = False;
  XClassWarning->Create();

  XClassWarningOK = new OnyxButton;
  XClassWarningOK->ID = XClassWarningOKID;
  XClassWarningOK->Parent = XClassWarning->Self();
  XClassWarningOK->X = 65;
  XClassWarningOK->Y = 200;
  XClassWarningOK->Width = 70;
  XClassWarningOK->Height = 25;
  strcpy(XClassWarningOK->Text, "OK");
  XClassWarningOK->Create();

  XClassWarningLabel1 = new OnyxIcon;
  XClassWarningLabel1->ID = XClassWarningLabel1ID;
  XClassWarningLabel1->Parent = XClassWarning->Self();
  XClassWarningLabel1->X = 36;
  XClassWarningLabel1->Y = 15;
  XClassWarningLabel1->Width = 128;
  XClassWarningLabel1->Height = 32;
  strcpy(XClassWarningLabel1->Picture, "onyx_warning.xpm");
  XClassWarningLabel1->Create();

  XClassWarningLabel2 = new OnyxLabel;
  XClassWarningLabel2->ID = XClassWarningLabel2ID;
  XClassWarningLabel2->Parent = XClassWarning->Self();
  XClassWarningLabel2->X = 15;
  XClassWarningLabel2->Y = 60;
  XClassWarningLabel2->Width = 175;
  XClassWarningLabel2->Height = 20;
  strcpy(XClassWarningLabel2->Text, "This version of OnyX was");
  XClassWarningLabel2->Create();

  XClassWarningLabel3 = new OnyxLabel;
  XClassWarningLabel3->ID = XClassWarningLabel3ID;
  XClassWarningLabel3->Parent = XClassWarning->Self();
  XClassWarningLabel3->X = 15;
  XClassWarningLabel3->Y = 80;
  XClassWarningLabel3->Width = 175;
  XClassWarningLabel3->Height = 20;
  strcpy(XClassWarningLabel3->Text, "developed with XClass version");
  XClassWarningLabel3->Create();

  XClassWarningLabel4 = new OnyxLabel;
  XClassWarningLabel4->ID = XClassWarningLabel4ID;
  XClassWarningLabel4->Parent = XClassWarning->Self();
  XClassWarningLabel4->X = 15;
  XClassWarningLabel4->Y = 100;
  XClassWarningLabel4->Width = 175;
  XClassWarningLabel4->Height = 20;
  strcpy(XClassWarningLabel4->Text, "");
  XClassWarningLabel4->Create();

  XClassWarningLabel5 = new OnyxLabel;
  XClassWarningLabel5->ID = XClassWarningLabel5ID;
  XClassWarningLabel5->Parent = XClassWarning->Self();
  XClassWarningLabel5->X = 15;
  XClassWarningLabel5->Y = 120;
  XClassWarningLabel5->Width = 175;
  XClassWarningLabel5->Height = 20;
  strcpy(XClassWarningLabel5->Text, "and compiled with version");
  XClassWarningLabel5->Create();

  XClassWarningLabel6 = new OnyxLabel;
  XClassWarningLabel6->ID = XClassWarningLabel6ID;
  XClassWarningLabel6->Parent = XClassWarning->Self();
  XClassWarningLabel6->X = 15;
  XClassWarningLabel6->Y = 140;
  XClassWarningLabel6->Width = 175;
  XClassWarningLabel6->Height = 20;
  strcpy(XClassWarningLabel6->Text, "");
  XClassWarningLabel6->Create();

  XClassWarningLabel7 = new OnyxLabel;
  XClassWarningLabel7->ID = XClassWarningLabel7ID;
  XClassWarningLabel7->Parent = XClassWarning->Self();
  XClassWarningLabel7->X = 15;
  XClassWarningLabel7->Y = 160;
  XClassWarningLabel7->Width = 175;
  XClassWarningLabel7->Height = 20;
  strcpy(XClassWarningLabel7->Text, "Strange behaviors may arise.");
  XClassWarningLabel7->Create();

  ConvertMsgBox = new OnyxMsgBox;
  ConvertMsgBox->ID = ConvertMsgBoxID;
  ConvertMsgBox->Parent = MainWindow->Self();
  ConvertMsgBox->DisplayWindow = ApplicationRoot();
  ConvertMsgBox->Icon = QuestionIcon;
  ConvertMsgBox->AddButton(YesButton);
  ConvertMsgBox->AddButton(NoButton);
  ConvertMsgBox->Create();

  ConfirmMsgBox = new OnyxMsgBox;
  ConfirmMsgBox->ID = ConfirmMsgBoxID;
  ConfirmMsgBox->Parent = MainWindow->Self();
  ConfirmMsgBox->DisplayWindow = ApplicationRoot();
  ConfirmMsgBox->Icon = QuestionIcon;
  ConfirmMsgBox->AddButton(YesButton);
  ConfirmMsgBox->AddButton(NoButton);
  ConfirmMsgBox->AddButton(CancelButton);
  ConfirmMsgBox->Create();

  ColorDialog = new OnyxColorDialog;
  ColorDialog->ID = ColorDialogID;
  ColorDialog->Parent = MainWindow->Self();
  ColorDialog->DisplayWindow = ApplicationRoot();
  ColorDialog->Create();

  FontDialog = new OnyxFontDialog;
  FontDialog->ID = FontDialogID;
  FontDialog->Parent = MainWindow->Self();
  FontDialog->DisplayWindow = ApplicationRoot();
  FontDialog->Create();
}

int OnyxProcessMessage(OnyxMessage *msg) {

  switch (msg->id) {
    case MainWindowID:
      switch (msg->action) {
        case WindowCloseSignaled:
          MainWindow_CloseSignaled();
          break;

        case WindowClosed:
          MainWindow_Closed();
          break;

        case WindowLoad:
          MainWindow_Load();
          break;

        case WindowResized:
          MainWindow_WindowResized(msg->old_width, msg->old_height, msg->width, msg->height);
          break;
      }
      break;

    case ProjectExplorerID:
      switch (msg->action) {
        case WindowCloseSignaled:
          ProjectExplorer_CloseSignaled();
          break;

        case WindowClosed:
          ProjectExplorer_Closed();
          break;

        case WindowLoad:
          ProjectExplorer_Load();
          break;

        case WindowResized:
          ProjectExplorer_WindowResized(msg->old_width, msg->old_height, msg->width, msg->height);
          break;
      }
      break;

    case MenuEditorID:
      switch (msg->action) {
        case WindowCloseSignaled:
          MenuEditor_CloseSignaled();
          break;

        case WindowClosed:
          MenuEditor_Closed();
          break;

        case WindowLoad:
          MenuEditor_Load();
          break;

        case WindowResized:
          MenuEditor_WindowResized(msg->old_width, msg->old_height, msg->width, msg->height);
          break;
      }
      break;

    case MenuEditor2ID:
      switch (msg->action) {
        case WindowCloseSignaled:
          MenuEditor2_CloseSignaled();
          break;

        case WindowClosed:
          MenuEditor2_Closed();
          break;

        case WindowLoad:
          MenuEditor2_Load();
          break;

        case WindowResized:
          MenuEditor2_WindowResized(msg->old_width, msg->old_height, msg->width, msg->height);
          break;
      }
      break;

    case PropertiesWindowID:
      switch (msg->action) {
        case WindowCloseSignaled:
          PropertiesWindow_CloseSignaled();
          break;

        case WindowClosed:
          PropertiesWindow_Closed();
          break;

        case WindowLoad:
          PropertiesWindow_Load();
          break;

        case WindowResized:
          PropertiesWindow_WindowResized(msg->old_width, msg->old_height, msg->width, msg->height);
          break;
      }
      break;

    case ToolboxID:
      switch (msg->action) {
        case WindowCloseSignaled:
          Toolbox_CloseSignaled();
          break;

        case WindowClosed:
          Toolbox_Closed();
          break;

        case WindowLoad:
          Toolbox_Load();
          break;

        case ButtonReleased:
          Toolbox_ButtonReleased(msg->button, msg->x, msg->y);
          break;

        case WindowResized:
          Toolbox_WindowResized(msg->old_width, msg->old_height, msg->width, msg->height);
          break;
      }
      break;

    case MakeWindowID:
      switch (msg->action) {
        case WindowCloseSignaled:
          MakeWindow_CloseSignaled();
          break;

        case WindowClosed:
          MakeWindow_Closed();
          break;

        case WindowLoad:
          MakeWindow_Load();
          break;

        case WindowResized:
          MakeWindow_WindowResized(msg->old_width, msg->old_height, msg->width, msg->height);
          break;
      }
      break;

    case CustomControlsWindowID:
      switch (msg->action) {
        case WindowCloseSignaled:
          CustomControlsWindow_CloseSignaled();
          break;

        case WindowClosed:
          CustomControlsWindow_Closed();
          break;

        case WindowLoad:
          CustomControlsWindow_Load();
          break;

        case WindowResized:
          CustomControlsWindow_WindowResized(msg->old_width, msg->old_height, msg->width, msg->height);
          break;
      }
      break;

    case CCScrollID:
      switch (msg->action) {
        case ScrollBarMoved:
          CCScroll_ScrollBarMoved(msg->position);
          break;
      }
      break;

    case CCHelpID:
      switch (msg->action) {
        case ButtonClicked:
          CCHelp_ButtonClick();
          break;
      }
      break;

    case CCRecompileID:
      switch (msg->action) {
        case ButtonClicked:
          CCRecompile_ButtonClick();
          break;
      }
      break;

    case CCHelpWindowID:
      switch (msg->action) {
        case WindowCloseSignaled:
          CCHelpWindow_CloseSignaled();
          break;

        case WindowClosed:
          CCHelpWindow_Closed();
          break;

        case WindowLoad:
          CCHelpWindow_Load();
          break;
      }
      break;

    case CCHelpOKID:
      switch (msg->action) {
        case ButtonClicked:
          CCHelpOK_ButtonClick();
          break;
      }
      break;

    case OpenButtonID:
      switch (msg->action) {
        case ButtonClicked:
          OpenButton_ButtonClick();
          break;
      }
      break;

    case SaveButtonID:
      switch (msg->action) {
        case ButtonClicked:
          SaveButton_ButtonClick();
          break;
      }
      break;

    case SaveProjectButtonID:
      switch (msg->action) {
        case ButtonClicked:
          SaveProjectButton_ButtonClick();
          break;
      }
      break;

    case NewWindowButtonID:
      switch (msg->action) {
        case ButtonClicked:
          NewWindowButton_ButtonClick();
          break;
      }
      break;

    case ViewWindowButtonID:
      switch (msg->action) {
        case ButtonClicked:
          ViewWindowButton_ButtonClick();
          break;
      }
      break;

    case NextWindowButtonID:
      switch (msg->action) {
        case ButtonClicked:
          NextWindowButton_ButtonClick();
          break;
      }
      break;

    case MakeButtonID:
      switch (msg->action) {
        case ButtonClicked:
          MakeButton_ButtonClick();
          break;
      }
      break;

    case RunButtonID:
      switch (msg->action) {
        case ButtonClicked:
          RunButton_ButtonClick();
          break;
      }
      break;

    case StopButtonID:
      switch (msg->action) {
        case ButtonClicked:
          StopButton_ButtonClick();
          break;
      }
      break;

    case MakeRunButtonID:
      switch (msg->action) {
        case ButtonClicked:
          MakeRunButton_ButtonClick();
          break;
      }
      break;

    case DeleteControlButtonID:
      switch (msg->action) {
        case ButtonClicked:
          DeleteControlButton_ButtonClick();
          break;
      }
      break;

    case LockControlsButtonID:
      switch (msg->action) {
        case ButtonClicked:
          LockControlsButton_ButtonClick();
          break;
      }
      break;

    case ProjectExplorerButtonID:
      switch (msg->action) {
        case ButtonClicked:
          ProjectExplorerButton_ButtonClick();
          break;
      }
      break;

    case ShowCodeButtonID:
      switch (msg->action) {
        case ButtonClicked:
          ShowCodeButton_ButtonClick();
          break;
      }
      break;

    case ShowPropertiesButtonID:
      switch (msg->action) {
        case ButtonClicked:
          ShowPropertiesButton_ButtonClick();
          break;
      }
      break;

    case SnapToGridID:
      switch (msg->action) {
        case ButtonClicked:
          SnapToGrid_ButtonClick();
          break;
      }
      break;

    case ShowToolboxButtonID:
      switch (msg->action) {
        case ButtonClicked:
          ShowToolboxButton_ButtonClick();
          break;
      }
      break;

    case MakeTimerID:
      switch (msg->action) {
        case TimerElapsed:
          MakeTimer_TimerElapsed();
          break;
      }
      break;

    case EditTimerID:
      switch (msg->action) {
        case TimerElapsed:
          EditTimer_TimerElapsed();
          break;
      }
      break;

    case TLCornerID:
      switch (msg->action) {
        case ButtonPressed:
          TLCorner_ButtonPressed(msg->button, msg->x, msg->y);
          break;

        case ButtonReleased:
          TLCorner_ButtonReleased(msg->button, msg->x, msg->y);
          break;

        case PointerMoved:
          TLCorner_PointerMoved(msg->x, msg->y, msg->x_root, msg->y_root);
          break;
      }
      break;

    case TRCornerID:
      switch (msg->action) {
        case ButtonPressed:
          TRCorner_ButtonPressed(msg->button, msg->x, msg->y);
          break;

        case ButtonReleased:
          TRCorner_ButtonReleased(msg->button, msg->x, msg->y);
          break;

        case PointerMoved:
          TRCorner_PointerMoved(msg->x, msg->y, msg->x_root, msg->y_root);
          break;
      }
      break;

    case BLCornerID:
      switch (msg->action) {
        case ButtonPressed:
          BLCorner_ButtonPressed(msg->button, msg->x, msg->y);
          break;

        case ButtonReleased:
          BLCorner_ButtonReleased(msg->button, msg->x, msg->y);
          break;

        case PointerMoved:
          BLCorner_PointerMoved(msg->x, msg->y, msg->x_root, msg->y_root);
          break;
      }
      break;

    case BRCornerID:
      switch (msg->action) {
        case ButtonPressed:
          BRCorner_ButtonPressed(msg->button, msg->x, msg->y);
          break;

        case ButtonReleased:
          BRCorner_ButtonReleased(msg->button, msg->x, msg->y);
          break;

        case PointerMoved:
          BRCorner_PointerMoved(msg->x, msg->y, msg->x_root, msg->y_root);
          break;
      }
      break;

    case NewProjectMenuItemID:
      switch (msg->action) {
        case MenuItemSelected:
          NewProjectMenuItem_MenuItemSelected();
          break;

        case MenuItemHighlighted:
          NewProjectMenuItem_MenuItemHighlighted();
          break;
      }
      break;

    case OpenProjectMenuItemID:
      switch (msg->action) {
        case MenuItemSelected:
          OpenProjectMenuItem_MenuItemSelected();
          break;

        case MenuItemHighlighted:
          OpenProjectMenuItem_MenuItemHighlighted();
          break;
      }
      break;

    case SaveObjectMenuItemID:
      switch (msg->action) {
        case MenuItemSelected:
          SaveObjectMenuItem_MenuItemSelected();
          break;

        case MenuItemHighlighted:
          SaveObjectMenuItem_MenuItemHighlighted();
          break;
      }
      break;

    case SaveProjectAsMenuItemID:
      switch (msg->action) {
        case MenuItemSelected:
          SaveProjectAsMenuItem_MenuItemSelected();
          break;

        case MenuItemHighlighted:
          SaveProjectAsMenuItem_MenuItemHighlighted();
          break;
      }
      break;

    case SaveProjectMenuItemID:
      switch (msg->action) {
        case MenuItemSelected:
          SaveProjectMenuItem_MenuItemSelected();
          break;

        case MenuItemHighlighted:
          SaveProjectMenuItem_MenuItemHighlighted();
          break;
      }
      break;

    case ExportObjectMenuItemID:
      switch (msg->action) {
        case MenuItemSelected:
          ExportObjectMenuItem_MenuItemSelected();
          break;

        case MenuItemHighlighted:
          ExportObjectMenuItem_MenuItemHighlighted();
          break;
      }
      break;

    case ImportObjectMenuItemID:
      switch (msg->action) {
        case MenuItemSelected:
          ImportObjectMenuItem_MenuItemSelected();
          break;

        case MenuItemHighlighted:
          ImportObjectMenuItem_MenuItemHighlighted();
          break;
      }
      break;

    case ExportProjectMenuItemID:
      switch (msg->action) {
        case MenuItemSelected:
          ExportProjectMenuItem_MenuItemSelected();
          break;

        case MenuItemHighlighted:
          ExportProjectMenuItem_MenuItemHighlighted();
          break;
      }
      break;

    case ImportProjectMenuItemID:
      switch (msg->action) {
        case MenuItemSelected:
          ImportProjectMenuItem_MenuItemSelected();
          break;

        case MenuItemHighlighted:
          ImportProjectMenuItem_MenuItemHighlighted();
          break;
      }
      break;

    case CreateApplicationMenuItemID:
      switch (msg->action) {
        case MenuItemSelected:
          CreateApplicationMenuItem_MenuItemSelected();
          break;
      }
      break;

    case ExitMenuItemID:
      switch (msg->action) {
        case MenuItemSelected:
          ExitMenuItem_MenuItemSelected();
          break;

        case MenuItemHighlighted:
          ExitMenuItem_MenuItemHighlighted();
          break;
      }
      break;

    case ViewCodeMenuItemID:
      switch (msg->action) {
        case MenuItemSelected:
          ViewCodeMenuItem_MenuItemSelected();
          break;

        case MenuItemHighlighted:
          ViewCodeMenuItem_MenuItemHighlighted();
          break;
      }
      break;

    case ViewWindowMenuItemID:
      switch (msg->action) {
        case MenuItemSelected:
          ViewWindowMenuItem_MenuItemSelected();
          break;

        case MenuItemHighlighted:
          ViewWindowMenuItem_MenuItemHighlighted();
          break;
      }
      break;

    case ViewGridMenuItemID:
      switch (msg->action) {
        case MenuItemSelected:
          ViewGridMenuItem_MenuItemSelected();
          break;

        case MenuItemHighlighted:
          ViewGridMenuItem_MenuItemHighlighted();
          break;
      }
      break;

    case ViewProjectMenuItemID:
      switch (msg->action) {
        case MenuItemSelected:
          ViewProjectMenuItem_MenuItemSelected();
          break;

        case MenuItemHighlighted:
          ViewProjectMenuItem_MenuItemHighlighted();
          break;
      }
      break;

    case ViewPropertiesMenuItemID:
      switch (msg->action) {
        case MenuItemSelected:
          ViewPropertiesMenuItem_MenuItemSelected();
          break;

        case MenuItemHighlighted:
          ViewPropertiesMenuItem_MenuItemHighlighted();
          break;
      }
      break;

    case ToolBarMenuItemID:
      switch (msg->action) {
        case MenuItemSelected:
          ToolBarMenuItem_MenuItemSelected();
          break;

        case MenuItemHighlighted:
          ToolBarMenuItem_MenuItemHighlighted();
          break;
      }
      break;

    case ToolBoxMenuItemID:
      switch (msg->action) {
        case MenuItemSelected:
          ToolBoxMenuItem_MenuItemSelected();
          break;

        case MenuItemHighlighted:
          ToolBoxMenuItem_MenuItemHighlighted();
          break;
      }
      break;

    case MakeMenuItemID:
      switch (msg->action) {
        case MenuItemSelected:
          MakeMenuItem_MenuItemSelected();
          break;

        case MenuItemHighlighted:
          MakeMenuItem_MenuItemHighlighted();
          break;
      }
      break;

    case MakeCleanMenuItemID:
      switch (msg->action) {
        case MenuItemSelected:
          MakeCleanMenuItem_MenuItemSelected();
          break;

        case MenuItemHighlighted:
          MakeCleanMenuItem_MenuItemHighlighted();
          break;
      }
      break;

    case MakeRunMenuItemID:
      switch (msg->action) {
        case MenuItemSelected:
          MakeRunMenuItem_MenuItemSelected();
          break;

        case MenuItemHighlighted:
          MakeRunMenuItem_MenuItemHighlighted();
          break;
      }
      break;

    case RunMenuItemID:
      switch (msg->action) {
        case MenuItemSelected:
          RunMenuItem_MenuItemSelected();
          break;

        case MenuItemHighlighted:
          RunMenuItem_MenuItemHighlighted();
          break;
      }
      break;

    case StopMenuItemID:
      switch (msg->action) {
        case MenuItemSelected:
          StopMenuItem_MenuItemSelected();
          break;

        case MenuItemHighlighted:
          StopMenuItem_MenuItemHighlighted();
          break;
      }
      break;

    case NewProjectWindowID:
      switch (msg->action) {
        case WindowCloseSignaled:
          NewProjectWindow_CloseSignaled();
          break;

        case WindowClosed:
          NewProjectWindow_Closed();
          break;
      }
      break;

    case NewProjectOpenButtonID:
      switch (msg->action) {
        case ButtonClicked:
          NewProjectOpenButton_ButtonClick();
          break;
      }
      break;

    case NewProjectCancelButtonID:
      switch (msg->action) {
        case ButtonClicked:
          NewProjectCancelButton_ButtonClick();
          break;
      }
      break;

    case MenuEditorItemID:
      switch (msg->action) {
        case MenuItemSelected:
          MenuEditorItem_MenuItemSelected();
          break;

        case MenuItemHighlighted:
          MenuEditorItem_MenuItemHighlighted();
          break;
      }
      break;

    case SnapToGridMenuItemID:
      switch (msg->action) {
        case MenuItemSelected:
          SnapToGridMenuItem_MenuItemSelected();
          break;
      }
      break;

    case CustomControlsItemID:
      switch (msg->action) {
        case MenuItemSelected:
          CustomControlsItem_MenuItemSelected();
          break;

        case MenuItemHighlighted:
          CustomControlsItem_MenuItemHighlighted();
          break;
      }
      break;

    case OptionsItemID:
      switch (msg->action) {
        case MenuItemSelected:
          OptionsItem_MenuItemSelected();
          break;

        case MenuItemHighlighted:
          OptionsItem_MenuItemHighlighted();
          break;
      }
      break;

    case NewProjectNameID:
      switch (msg->action) {
        case EnterPressed:
          NewProjectName_EnterPressed();
          break;
      }
      break;

    case OptionsWindowID:
      switch (msg->action) {
        case WindowCloseSignaled:
          OptionsWindow_CloseSignaled();
          break;

        case WindowClosed:
          OptionsWindow_Closed();
          break;

        case WindowLoad:
          OptionsWindow_Load();
          break;
      }
      break;

    case AboutMenuItemID:
      switch (msg->action) {
        case MenuItemSelected:
          AboutMenuItem_MenuItemSelected();
          break;
      }
      break;

    case OLocationTextID:
      switch (msg->action) {
        case EnterPressed:
          OLocationText_EnterPressed();
          break;
      }
      break;

    case OEditorTextID:
      switch (msg->action) {
        case EnterPressed:
          OEditorText_EnterPressed();
          break;
      }
      break;

    case SnapToGridDefaultID:
      switch (msg->action) {
        case ButtonPressed:
          SnapToGridDefault_ButtonClicked();
          break;
      }
      break;

    case OCancelButtonID:
      switch (msg->action) {
        case ButtonClicked:
          OCancelButton_ButtonClick();
          break;
      }
      break;

    case OSaveButtonID:
      switch (msg->action) {
        case ButtonClicked:
          OSaveButton_ButtonClick();
          break;
      }
      break;

    case ExplorerTreeID:
      switch (msg->action) {
        case ItemSelected:
          ExplorerTree_ItemSelected();
          break;
      }
      break;

    case MenuTreeID:
      switch (msg->action) {
        case ButtonPressed:
          MenuTree_ButtonPressed(msg->button, msg->x, msg->y);
          break;
      }
      break;

    case SplashScreenID:
      switch (msg->action) {
        case WindowClosed:
          SplashScreen_Closed();
          break;
      }
      break;

    case SplashTimerID:
      switch (msg->action) {
        case TimerElapsed:
          SplashTimer_TimerElapsed();
          break;
      }
      break;

    case MenuAddID:
      switch (msg->action) {
        case ButtonClicked:
          MenuAdd_ButtonClick();
          break;
      }
      break;

    case MenuDeleteID:
      switch (msg->action) {
        case ButtonClicked:
          MenuDelete_ButtonClick();
          break;
      }
      break;

    case MenuItemAddID:
      switch (msg->action) {
        case ButtonClicked:
          MenuItemAdd_ButtonClick();
          break;
      }
      break;

    case MenuItemDeleteID:
      switch (msg->action) {
        case ButtonClicked:
          MenuItemDelete_ButtonClick();
          break;
      }
      break;

    case MenuEditor2OkID:
      switch (msg->action) {
        case ButtonClicked:
          MenuEditor2Ok_ButtonClick();
          break;
      }
      break;

    case MenuEditor2CancelID:
      switch (msg->action) {
        case ButtonClicked:
          MenuEditor2Cancel_ButtonClick();
          break;
      }
      break;

    case MenuEditor2TextBoxID:
      switch (msg->action) {
        case EnterPressed:
          MenuEditor2TextBox_EnterPressed();
          break;
      }
      break;

    case XClassWarningOKID:
      switch (msg->action) {
        case ButtonClicked:
          XClassWarningOK_ButtonClick();
          break;
      }
      break;

    default:
      GeneralEvent(msg);
      break;
  }

  return True;
}
