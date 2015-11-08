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

/* Re-Move old file when saving after a rename. */
/* Watch delete, it may try the occasional double delete. Is this a prob? */
/* Deny removing of object types if they are being used. */
/* Set property. Hit return. Took. Then bad window exit. Object was vscrollbar. height 128 set from 255. Manual. ListedX_ClearArea() */
/* Again. Clicked on textbox. Moved it. Selected scrollbar off of explorer. Set X and it died. */

#include "Objects.h"  
#include "Events.h"  
#include "GlobalExtern.h" 

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>

#include <X11/keysym.h>

extern int debug;


extern int CurrentDevelopmentWindow;
extern OnyxLabel *CCHelpInfo[20];

extern int SnapToGridIsOn;
extern int SnapToGridIsDefault;
extern int GridSize;

extern OnyxButton *ToolboxItem[50];
extern int LastToolSelected;

extern void CreateDevelopmentItems();
extern void CreateDevelopmentItem(struct ProjectObjectList *NewObject);
extern void CreateBrackets();
extern void MoveBrackets(int X, int Y, int Width, int Height);
extern void DestroyBrackets();
extern int DestroyDevelopmentObjects();

extern void ProjectExplorer_Update();
extern void PropertiesWindow_Update();

extern void UpdateDevelopmentObject(int ObjectToUpdate, struct ProjectObjectPropertyList *Property);
extern void UpdateDevelopmentWindow(int WindowToUpdate, struct ProjectObjectPropertyList *Property);

extern int CCSelected;
extern OnyxCheckBox *CCList[CCIDMax];
extern char CCFileList[CCIDMax][255];

extern int CustomControlsNeedToBeCompiled;

extern int ValidProject;
extern void SetMainWindowButtonStatus();
extern void SetMainWindowMenuStatus();

extern int ControlsLocked;

int NextAvailableNewObjectNumber(char *ObjectName);

int DevelopmentButtonDown;
int ItemClickX;
int ItemClickY;
int SelectedItemID;


struct ProjectObjectList *AddDevelopmentObject(struct ObjectList *UsableObject, char *Window, long x, long y, char *ParameterOverride, char *POValue);
void RenameDevelopmentObject(struct ProjectObjectList *Object, char*NewName);
void DeleteDevelopmentObject(struct ProjectObjectList *Object);

void RemoveObjectType(char *ObjectTypeName);

void OpenProject();
void ResetProject();

extern void InitializeInternalProjectFile(char *ProjectFileName);
extern void InitializeInternalMain_h();
extern void InitializeInternalEvents_h();
extern void InitializeInternalObjects_h();
extern void InitializeInternalIncludes_h();
extern void InitializeInternalMakefile();
extern void InitializeInternalBaseMakefile();
extern void InitializeInternalApplication();

int WriteOnyxrcFile();


int RCError;

InternalCodeFile *InternalOxcCode;
InternalCodeFile *InternalHeader;
InternalCodeFile *InternalMaincc;
InternalCodeFile *InternalMakefile;
InternalCodeFile *InternalBaseMakefile;
InternalCodeFile *InternalOxpCode;


#define SNAP(x) ((((x) + GridSize/2) / GridSize) * GridSize)


/***********************************************************************/

InternalCodeFile::InternalCodeFile(char *ObjectNam,
                                   InternalCodeFile *Prev,
                                   InternalCodeFile *Nex) {
  if (ObjectNam) {
    strcpy(ObjectName, ObjectNam);
  } else {
    strcpy(ObjectName, "");
  }

  strcpy(OldObjectName, "");

  Buffer = NULL;

  Next = Nex;
  Previous = Prev;
  if (Previous) Previous->Next = this;
  if (Next) Next->Previous = this;

  EditorPID = 0;
  HasBeenModified = False;
  HasBeenRenamed = False;
  HasBeenDeleted = False;
  IsIncorporated = True;
  CodeStyle = 0;
  FileStats.st_mtime = 0;
  FileStats.st_size = 0;
  Breaker = 0;
}

void InternalCodeFile::Open() {
  char CurrentFileName[255];
  int  OpenFile;
  char temp[255];

  switch (CodeStyle) {
    case 1:
      sprintf(CurrentFileName, "%s.cc", ObjectName);
      break;

    case 2:
      sprintf(CurrentFileName, "%s.h", ObjectName);
      break;

    case 3:
      strcpy(CurrentFileName, "main.cc");
      break;

    case 4:
      strcpy(CurrentFileName, "Makefile");
      break;

    case 5:
      strcpy(CurrentFileName, ObjectName);
      break;

    default:
      sprintf(CurrentFileName, "%s.oxc", ObjectName);
      break;
  }

  if (!stat(CurrentFileName, &FileStats)) {
    Buffer = (char *) malloc(FileStats.st_size + 128);  /* Grab memory needed for file. 128 Extra for overflow. */
    if ((OpenFile = open(CurrentFileName, O_RDONLY)) > 0) {
      if (read(OpenFile, Buffer, FileStats.st_size) != FileStats.st_size) {
        sprintf(temp, "Error reading %s", CurrentFileName);
        StopBox(MainWindow->Self(), temp);
      }
      close(OpenFile);
    } else {
      sprintf(temp, "Error opening %s", CurrentFileName);
      StopBox(MainWindow->Self(), temp);
    }
  } else {
    sprintf(temp, "Could not find file %s", CurrentFileName);
    StopBox(MainWindow->Self(), temp);
  }
}

void InternalCodeFile::Append(const char *fmt, ...) {
  va_list args;

  char temp[2048];
  va_start(args, fmt);
  vsnprintf(temp, 2048, fmt, args);
  va_end(args);

  AppendString(temp, strlen(temp));
}

void InternalCodeFile::AppendString(char *Data, long Size) {
  long OldSize;
  char temp[128];
  char *Marker;
//  char *OldBuffer;

  /* I should probably use realloc here. */

  OldSize = FileStats.st_size;  /* Retain this so we know where to append at. */
  FileStats.st_size += Size;  /* Increase size of file. */
  Marker = (char *) realloc(Buffer, FileStats.st_size);  /* Re allocate the new memory needed. */

  if (Marker) {  /* If the realloc succeeded */
    Buffer = Marker;  /* Move the buffer */
    Marker += OldSize;  /* Jump to the end of the "file" */
    memcpy(Marker, Data, Size);  /* Append the new data */
  } else {
    sprintf("Error appending data to internal code for %s", ObjectName);
    StopBox(MainWindow->Self(), temp);
  }

//  OldBuffer = Buffer;    /* Momentarily save the old buffer. */
//  Buffer = (char *) malloc(FileStats.st_size + 128);  /* Grab memory needed for new buffer. */
//  memcpy(Buffer, OldBuffer, OldSize);  /* Save the old information. */
//  free(OldBuffer);    /* Free the old memory we no longer need. */

//  Marker = Buffer;    /* Jump the marker to */
//  Marker += OldSize;    /* the end of the "file" */

//  strncpy(Marker, Data, Size);  /* Write date to the end of the file. */

}

void InternalCodeFile::Replace(char *Original, long OriginalSize, char *New, long NewSize) {
  int i;
  int Occurrences;
  long OldSize;
  char *Marker;
  char *Marker2;
  char *OldBuffer;

  /* Count the number of instances of Original to determine size of new buffer. */
  Occurrences = 0;
  for (i = 0, Marker = Buffer; i < FileStats.st_size - OriginalSize; Marker++, i++) {  /* - OriginalSize to prevent segv on strncmp */
    if (Original[0] == *Marker) {  /* If there is a match onthe first letter. */
      if (!strncmp(Original, Marker, OriginalSize))  /* Check for occurrence of Original. */
        Occurrences++;
    }
  }

  if (Occurrences > 0) {  /* If there was at least one occurrence */
    OldSize = FileStats.st_size;      /* Save current size. */
    FileStats.st_size = FileStats.st_size + Occurrences * (NewSize - OriginalSize);  /* Change size of buffer. */
    OldBuffer = Buffer;        /* Save location to old buffer */
    Buffer = (char *) malloc(FileStats.st_size + 128);  /* Grab memory needed for new buffer. */

    /* Copy data making replacement. */

    for (i = 0, Marker = OldBuffer, Marker2 = Buffer; i < OldSize - OriginalSize; Marker++, Marker2++, i++) {
      if (Original[0] == *Marker) {      /* If there is a match onthe first letter. */
        if (!strncmp(Original, Marker, OriginalSize)) {  /* Check for occurrence of Original. */
          strncpy(Marker2, New, NewSize);  /* Write replacement. */
         
          Marker = Marker + OriginalSize - 1;  /* Move to new location in OldBuffer. */
          Marker2 = Marker2 + NewSize - 1;  /* Move to new location in Buffer. */
          i = i + OriginalSize - 1;    /* Increment i to reflect change in Marker. */
        } else {
          *Marker2 = *Marker;    /* Copy character from old to new buffer. */
        }
      } else {      
        *Marker2 = *Marker;    /* Copy character from old to new buffer. */
      }
    }

    strncpy(Marker2, Marker, OriginalSize);    /* Get remaining data */
    free(OldBuffer);        /* Free the old memory we no longer need. */
  }
}

void InternalCodeFile::AppendToFile(char *NewFileName) {
  int  EditFile;
  char temp[128];

  if ((EditFile = open(NewFileName, O_RDWR | O_APPEND)) > 0) {
    if (write(EditFile, Buffer, FileStats.st_size) != FileStats.st_size) {
      sprintf(temp, "Error appending to %s", NewFileName);
      StopBox(MainWindow->Self(), temp);
    }
    close(EditFile);
  } else {
    sprintf(temp, "Could not open file %s for write", NewFileName);
    StopBox(MainWindow->Self(), temp);
  }
}

void InternalCodeFile::WriteForEdit() {
  char CurrentFileName[128];
  int  EditFile;
  char temp[128];

  if (CodeStyle == 0) {  /* If this is an oxc file */
    sprintf(CurrentFileName, "%s.oxt", ObjectName);
    if ((EditFile = open(CurrentFileName, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR)) > 0) {
      if (write(EditFile, Buffer, FileStats.st_size) != FileStats.st_size) {
        sprintf(temp, "Error writing %s", CurrentFileName);
        StopBox(MainWindow->Self(), temp);
      }
      close(EditFile);
      stat(CurrentFileName, &FileStats);
    } else {
      sprintf(temp, "Could not open %s for write", CurrentFileName);
      StopBox(MainWindow->Self(), temp);
    }
  }
}

void InternalCodeFile::Edit(char *EditorCommand) {
  char command[255];
  char temp[512];

  if (CodeStyle == 0) {  /* If this is an oxc file */
    if (EditorPID == 0) {
      WriteForEdit();
  
      EditorPID = fork();
      if (EditorPID == 0) {
        sprintf(command, "%s %s.oxt", EditorCommand, ObjectName);
        EditorPID = execlp("/bin/sh", "/bin/sh", "-c", command, NULL);
        if (EditorPID == -1) {
          sprintf(temp, "Error invoking editor.\nCommand was: /bin/sh -c %s", command);
          StopBox(MainWindow->Self(), temp);
          exit(1);
        }
      }
    }
  }
}

void InternalCodeFile::Check() {
  struct stat NewStats;
  char CurrentFileName[255];
  char temp[255];
  int  OpenFile;

  if (CodeStyle == 0) {  /* If this is an oxc file */
    if (waitpid(EditorPID, NULL, WNOHANG)) {
      EditorPID = 0;
      sprintf(CurrentFileName, "%s.oxt", ObjectName);
      stat(CurrentFileName, &NewStats);
      if (NewStats.st_mtime != FileStats.st_mtime) {  /* Check modification time to see if file was changed. */
        free(Buffer);  /* If so then ditch our current buffer and read the new file in. */
        stat(CurrentFileName, &FileStats);
        Buffer = (char *) malloc(FileStats.st_size + 128);  /* Grab memory needed for file. */
        if ((OpenFile = open(CurrentFileName, O_RDONLY)) > 0) {
          if (read(OpenFile, Buffer, FileStats.st_size) != FileStats.st_size) {
            sprintf(temp, "Error reading %s", CurrentFileName);
            StopBox(MainWindow->Self(), temp);
          }
          close(OpenFile);
          HasBeenModified = True;
        } else {
          sprintf(temp, "Error opening %s", CurrentFileName);
          StopBox(MainWindow->Self(), temp);
        }
      }
      remove(CurrentFileName);
    }
  }
}

void InternalCodeFile::Save() {
  char CurrentFileName[255];
  char OldFileName[255];
  char temp[255];
  int  EditFile;

  switch (CodeStyle) {
    case 1:
      sprintf(CurrentFileName, "%s.cc", ObjectName);
      sprintf(OldFileName, "%s.cc", OldObjectName);
      break;

    case 2:
      sprintf(CurrentFileName, "%s.h", ObjectName);
      sprintf(OldFileName, "%s.h", OldObjectName);
      break;

    case 3:
      sprintf(CurrentFileName, "main.cc");
      break;

    case 4:
      sprintf(CurrentFileName, "Makefile");
      break;

    case 5: 
      sprintf(CurrentFileName, "%s", ObjectName);
      break;

    default: 
      sprintf(CurrentFileName, "%s.oxc", ObjectName);
      sprintf(OldFileName, "%s.oxc", OldObjectName);
      break;
  }

  if ((EditFile = open(CurrentFileName, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR)) > 0) {
    if (write(EditFile, Buffer, FileStats.st_size) != FileStats.st_size) {  /* Write file. */
      sprintf(temp, "Error writing %s", CurrentFileName);
      StopBox(MainWindow->Self(), temp);
    }
    close(EditFile);                             /* Close file. */
    stat(CurrentFileName, &FileStats);           /* Update internal time stamp. */
    HasBeenModified = False;                     /* It is now saved. */
    if (HasBeenRenamed) {                        /* If it was recently renamed. */
      remove(OldFileName);
      HasBeenRenamed = False;
    }

    if (CodeStyle == 5) {  /* For the general type we will open up the permissions. */
       chmod(CurrentFileName, S_IRWXU);
    }
  } else {
    sprintf(temp, "Could not open %s for write", CurrentFileName);
    StopBox(MainWindow->Self(), temp);
  }

}

void InternalCodeFile::Close() {
  if (Buffer) {     // Destroy the internal buffer if it is still around.
    free(Buffer);
    Buffer = NULL;
  }
}

InternalCodeFile::~InternalCodeFile() {
  if (Buffer) {     // Destroy the internal buffer if it is still around.
    free(Buffer);
    Buffer = NULL;
  }

  if (Previous)     // Remove ourselves from the list.
    Previous->Next = Next;
  if (Next)
    Next->Previous = Previous;
}


/*********************************************************************/


char *LocateOnyx(char *name, char *path) {
  char test[1000];
  char filename[1000];
  char linkname[1000];
  char *pc;
  char save;
  int  len;
  int  namelen;
  int  found;
  char *TrueName;
  char *PathGiven;
  struct stat buf;
  int  linklen;

  namelen = strlen(name);

  PathGiven = strrchr(name, '/');
  if (!PathGiven) {
    pc = path;
    found = 0;
    while (*pc && (found == 0)) {
      len = 0;
      while (*pc && (*pc != ':')) {
        len++;
        pc++;
      }

      save = *pc;
      *pc = '\0';
      sprintf(test, "%s/%s", pc-len, name);
      *pc = save;
      if (*pc)
      pc++;

      found = (0 == access(test, X_OK));  /* executable */
    }

    if (!found) {
      PathGiven = NULL;
      TrueName = NULL;
    } else {
      PathGiven = strrchr(test, '/');
      if (PathGiven) *PathGiven = '\0';
      PathGiven = test;
      TrueName = name;
    }
  } else {
    *PathGiven = '\0';
    TrueName = PathGiven + 1;
    PathGiven = name;
  }

  if (PathGiven) {
    sprintf(filename, "%s/ObjectRepository", PathGiven);
    while (stat(filename, &buf) != 0) {
      sprintf(filename, "%s/%s", PathGiven, TrueName);
      linklen = readlink(filename, linkname, 1000);
      if (linklen == -1) {
        PathGiven = NULL;
        break; /* not link. No repository. Back out. */
      }
      linkname[linklen] = '\0';
      PathGiven = strrchr(linkname,'/');
      if (PathGiven) {  /* If a path was found in the link name */
        *PathGiven = '\0';  /* Terminate the name there. */
        TrueName = PathGiven + 1;  /* Set true name to name of file. */
        PathGiven = linkname;    /* Set PathGiven to path at the begining of linkname. */
      } else { /* Symbolic link without any path. */
        TrueName = linkname;
        sprintf(&linkname[linklen+1], filename);  /* Grab last path. */
        PathGiven = strrchr(&linkname[linklen+1], '/');  /* Locate last '/' */
        *PathGiven = '\0';      /* Terminate the name. */
        PathGiven = &linkname[linklen + 1];
      }
      sprintf(filename, "%s/ObjectRepository", PathGiven);
    }
  }

  return PathGiven;
}


void ApplicationPreload(int argc, char *argv[]) {
  FILE *Onyxrc;
  char InLine[255];
  char *Marker;
  char rcfilename[255];
  char *Location;
  char *Path;

  /* Check for debugging immediately. -d # must be the last thing on the command line (for now).*/
  debug = 0;

  if (argc > 1) {
    if (!strcmp(argv[argc - 2], "-d")) {
      debug = atoi(argv[argc -1]);
    }
  }

  Path = getenv("PATH");
  Location = LocateOnyx(*argv, Path);

  RCError = 0;

  if (Location) {
    strcpy(OnyxLocation, Location);
  } else {
    RCError = -3;
  }

  /* Defaults. */
  strcpy(EditorCommand, "xterm -title \"OnyX -edit\" -e pico");
  SnapToGridIsOn = True;
  SnapToGridIsDefault = True;
  GridSize = 10; ////5;

  Marker = getenv("HOME");
  if (Marker) {
    sprintf(rcfilename, "%s/.onyxrc", Marker);
    Onyxrc = fopen(rcfilename, "r");
    if (!Onyxrc) {
      RCError = WriteOnyxrcFile();
    } else {
      // ==!== ReadOnyxrcFile()
      while (fgets(InLine, 255, Onyxrc)) {
        if (!strncmp(InLine, "EditorCommand ", 14)) {
          Marker = InLine + 14;
          strcpy(EditorCommand, Marker);
          Marker = strchr(EditorCommand, '\n');
          if (Marker) *Marker = '\0';
        } else if (!strncmp(InLine, "SnapToGridByDefault ", 20)) {
          Marker = InLine + 20;
          SnapToGridIsDefault = atoi(Marker);
          SnapToGridIsOn = SnapToGridIsDefault;
#if 1
        } else if (!strncmp(InLine, "OnyxLocation ", 13)) {
          Marker = InLine + 13;
          strcpy(OnyxLocation, Marker);
          Marker = strchr(OnyxLocation, '\n');
          if (Marker) *Marker = '\0';
#endif
        }
      }
      fclose(Onyxrc);
    }
  } else {
    RCError = -1;
  }

  sprintf(OnyxIconLocation, "%s/icons", OnyxLocation);
  ValidProject = False;
}

void ApplicationLoad(int argc, char *argv[]) {
  int  i;
  char *ProjectName;

  /* Set all of the settings that are defaults and must be taken care of before development time. */

  /* Show splash screen while we are busy. */
  SplashScreen->Move((ScreenInfo->Width - SplashScreen->Width) / 2,
                     (ScreenInfo->Height - SplashScreen->Height) / 2);
  SplashScreen->Show();

  /* Verify the XClass version number */
  if (strcmp(XCLASS_VERSION, XClassDesignVersionNumber)) {  /* If they don't match then send warning message. */
    XClassWarning->Move((ScreenInfo->Width - XClassWarning->Width) / 2,
                        (ScreenInfo->Height - XClassWarning->Height) / 2);
    strcpy(XClassWarningLabel4->Text, XClassDesignVersionNumber);
    XClassWarningLabel4->Update();
    strcpy(XClassWarningLabel6->Text, XCLASS_VERSION);
    XClassWarningLabel6->Update();
    XClassWarning->Show();
  }

  /* Change the settings on the about window. */
  AboutWindow->Move((ScreenInfo->Width - AboutWindow->Width) / 2,
                    (ScreenInfo->Height - AboutWindow->Height) / 2);
  sprintf(AboutLabel1->Text, "Version %i.%i.%i", MajorVersionNumber,
                                                 MinorVersionNumber,
                                                 ReleaseVersionNumber);
  AboutLabel1->Update();
  sprintf(AboutLabel5->Text, "Version %s", XCLASS_VERSION);
  AboutLabel5->Update();

  TotalProjectWindows = 0;
  NextProjectID = 1 + IDModifier;
  
  UsableObjects  = NULL;    /* Initialize with no usable objects. */
  ProjectObjects = NULL;    /* Initialize with no objects in project. */
  DeletedObjects = NULL;    /* Initialize with no objects in project. */

  DevelopmentWindow[0] = NULL;    /* No windows being shown and worked on. */
  NextDevelopmentItemID = 0;      /* Next ID for new object. */
  DevelopmentButtonDown = False;  /* User is obviously not clicking on a window being worked on. */

  CCHelpInfo[0] = NULL;    /* No help has been previously given. */

  ToolboxItem[0] = NULL;   /* Toolbox hasn't been created yet. */
  LastToolSelected = 0;    /* Last tool selected is the "Pointer". */

  CustomControlsNeedToBeCompiled  = False;  /* Custom controls are up to date. */
  ControlsLocked = False;

  OpenFileDialog->ClearFileTypes();  /* Initialize open and save dialog boxes. */
  OpenFileDialog->AddFileType("Onyx Project", "*.oxp");

  SaveFileDialog->ClearFileTypes();
  SaveFileDialog->AddFileType("Onyx Project", "*.oxp");

  InternalOxcCode = NULL;
  InternalHeader = NULL;
  InternalMaincc = NULL;
  InternalMakefile = NULL;
  InternalBaseMakefile = NULL;
  InternalOxpCode = NULL;

  SetMainWindowButtonStatus();
  SetMainWindowMenuStatus();

  if (RCError == -2) {
    ExclamationBox(SplashScreen->Self(), "Warning: Could not access resource file.\nDefaults will be used.");
  } else if (RCError == -1) {
    ExclamationBox(SplashScreen->Self(), "Warning: HOME environment variable not set.\nDefaults will be used.");
  } else if (RCError == -3) {
    ExclamationBox(SplashScreen->Self(), "Severe Error! Could not locate Object Repository!");
  }

  if (!OpenButton->PictureFound) {
    ExclamationBox(SplashScreen->Self(), "Could not locate Onyx files.\nPlease check OnyxLocation setting.");
    OptionsWindow->Show();
  }

  // Parse through the args and get the project to work on if it was
  // specified.

  ProjectName = NULL;
  for (i = 1; i < argc; ++i) {
    if (argv[i][0] == '-') {
      if (!strcmp(argv[i], "-display")) ++i;  // skip display name
    } else {
      ProjectName = argv[i];
      break;
    }
  }

  if (ProjectName) {

    /* Go backwards to find the last slash. */
    for (i = strlen(ProjectName); (i > -1) && (ProjectName[i] != '/'); i--) {}

    i++;

    if (i > 0) {    /* If last slash found then a directy was given. */
      strcpy(ProjectFile, &ProjectName[i]);  /* Grab the project file name. */
      ProjectName[i] = '\0';      /* Cut off ProjectName for next strcpy. */
      strcpy(ProjectDirectory, ProjectName);  /* Grab the directory. */
    } else {     /* If last slash not found then ProjectName is the project file name. */
      strcpy(ProjectFile, ProjectName);  /* Grab the project file name. */
      strcpy(ProjectDirectory, "./");  /* Assume it is in the current directory. */
    }

    /* Initialize these lists to NULL. */
    for (i = 0; i < MaxDevelopmentObjects; i++) {
      DevelopmentObject[i] = NULL;
    }
    for (i = 0; i < MaxDevelopmentWindows; i++) {
      DevelopmentWindow[i] = NULL;
    }
    for (i = 0; i < DevelopmentItemIDMax; i++) {
      DevelopmentItem[i] = NULL;
    }

    OpenProject();        /* Open the specified project! */

  }

  MainWindow->Resize(ScreenInfo->Width - 8, MainWindow->Height);
  SplashTimer->Start();
//  MainWindow->Show();    /* Show the main window. */
}


void PropertyWindowEvent(OnyxMessage *msg) {
  int i, j;
  int Item;
  struct ObjectList *UsableObject;
  struct ObjectProperty *UsableProperty;
  struct ProjectObjectList *SelectedObjectLocal;
  char temp[255];

  SelectedObjectLocal = SelectedObject; /* Make local copy to avoid conflicts; */

  if (msg->action == ButtonClicked &&
      msg->id >= PropertyButtonIDModifier) {

    // Browse button of some kind selected.

    Item = msg->id - PropertyButtonIDModifier;

    if (PropertyButton[Item]) {
      /* Locate object type. */
      for (UsableObject = UsableObjects;
           strcmp(UsableObject->Name, SelectedObject->Type);
           UsableObject = UsableObject->Next) {}

      /* Locate PropertySetting info to see if we need anything else. */
      UsableProperty = UsableObject->Property;

      while (strcmp(UsableProperty->Name, PropertyItem[Item]->Text))
        UsableProperty = UsableProperty->Next;

      if (UsableProperty->Type == FileBrowser) {  /* File dialog. */
        OpenFileDialog->ClearFileTypes();
        for (i = 2; i < UsableProperty->Entries; i += 2) {
          OpenFileDialog->AddFileType(UsableProperty->Entry[i], UsableProperty->Entry[i+1]);
        }
        OpenFileDialog->Show();

        if (!OpenFileDialog->Cancelled) {

          /* Clear textbox entry. */
          PropertyValue[Item]->ClearText();

          /* Add what was selected to textbox. */
          
          /* If no preferences given or the word All for entry 0 */
          if (UsableProperty->Entries == 0 || !strcmp(UsableProperty->Entry[0], "All")) {
            sprintf(temp, "%s/%s", OpenFileDialog->DirectorySelected, OpenFileDialog->FileSelected);
            PropertyValue[Item]->AddText(temp);
          } else if (!strcmp(UsableProperty->Entry[0], "File")) {  /* If just file requested */
            PropertyValue[Item]->AddText(OpenFileDialog->FileSelected);
          } else {  /* Otherwise give them a directory. */
            PropertyValue[Item]->AddText(OpenFileDialog->DirectorySelected);
          }

          /* Update what is displayed. */
          PropertyValue[Item]->Update();

          /* If this is linked to another item to share the data with. Locate it. */
          if (UsableProperty->Entries >= 2 && strcmp(UsableProperty->Entry[1], "None")) {
            for (j = 0; PropertyItem[j] != NULL && strcmp(PropertyItem[j]->Text, UsableProperty->Entry[1]); j++) {}

            if(PropertyItem[j] != NULL)  /* If it was found. */
            {
              /* Locate PropertySetting info */
              UsableProperty = UsableObject->Property;

              while(strcmp(UsableProperty->Name,PropertyItem[j]->Text))
              {
                UsableProperty = UsableProperty->Next;
              }

              /* If no preferences given or the word All for entry 0 */
              if(UsableProperty->Entries == 0 || !strcmp(UsableProperty->Entry[0],"All"))
              {
                sprintf(temp,"%s/%s",OpenFileDialog->DirectorySelected,OpenFileDialog->FileSelected);
                PropertyValue[j]->AddText(temp);
              }
              else if(!strcmp(UsableProperty->Entry[0],"File"))  /* If just file requested */
              {
                PropertyValue[j]->AddText(OpenFileDialog->FileSelected);
              }
              else  /* Otherwise give them a directory. */
              {
                PropertyValue[j]->AddText(OpenFileDialog->DirectorySelected);
              }

              /* Update what is displayed. */
              PropertyValue[j]->Update();

              /* Since this is a pseudo second concurrent event */
              /* we have to do it the hard way. */

              j = j + PropertyIDModifier;

              OnyxMessage omsg = *msg;
              omsg.id = j;
              omsg.action = EnterPressed;

              PropertyWindowEvent(&omsg);
            }
          }

          /* Now fake an enter pressed to change internal property value. */
          msg->action = EnterPressed;
        }
      }
      else if(UsableProperty->Type == ColorBrowser)  /* Color selector dialog. */
      {
        strcpy(ColorDialog->CurrentColor, PropertyValue[Item]->Text);
        ColorDialog->Show();
        PropertyValue[Item]->ClearText();
        PropertyValue[Item]->AddText(ColorDialog->CurrentColor);
        PropertyValue[Item]->Update();

        /* Now fake an enter pressed to change internal property value. */
        msg->action = EnterPressed;
      }
      else if(UsableProperty->Type == FontBrowser)  /* Font selector dialog. */
      {
        strcpy(FontDialog->Font, PropertyValue[Item]->Text);
        FontDialog->Show();
        PropertyValue[Item]->ClearText();
        PropertyValue[Item]->AddText(FontDialog->Font);
        PropertyValue[Item]->Update();

        /* Now fake an enter pressed to change internal property value. */
        msg->action = EnterPressed;
      }
    }
  }
  else  /* Event not generated by OnyxButton. Use regular modifier to find Item index. */
  {
    Item = msg->id - PropertyIDModifier;
  }

  if(msg->action == EnterPressed || msg->action == ItemSelected)
  {
    if(PropertyCombo[Item] != NULL)
    {
      /* Clear textbox entry. */
      PropertyValue[Item]->ClearText();
      /* Copy text from combo box to text box. */
      PropertyValue[Item]->AddText(PropertyCombo[Item]->Text);
      PropertyValue[Item]->Update();
    }
    
    if(Item == 0) /* If Object Name being modified */
    {
      if(SelectedObject == NULL) /* If Application is the object being modified. */
      {
        strcpy(ProjectApplicationName,PropertyValue[0]->Text);
      }
      else
      {
        RenameDevelopmentObject(SelectedObjectLocal,PropertyValue[0]->Text);
        if(ProjectExplorer->IsVisible)
        {
          ProjectExplorer_Update();
//          ProjectExplorer->Show();
        }
      }
    }
    else /* Locate the property selected on the internal SelectedObject. */
    {
      SelectedProperty = SelectedObjectLocal->Property;
        /* Select the i'th property to match the i'th text box changed. */
      i = 1;
      while (i < Item)
      {
        if(!strcmp(SelectedObject->Type,"OnyxMenu") || !strcmp(SelectedObject->Type,"OnyxMenuItem"))
        {
          while(!strcmp(SelectedProperty->Name,"MenuParent") || !strcmp(SelectedProperty->Name,"MenuBarParent"))
          {
            SelectedProperty = SelectedProperty->Next;
          }
        }
        else
        {
          // printf("%s\n",SelectedObject->Type); /* Debug statement. */
        }
        SelectedProperty = SelectedProperty->Next;
        i++;
      }

      if(!strcmp(SelectedObject->Type,"OnyxMenu") || !strcmp(SelectedObject->Type,"OnyxMenuItem"))
      {
        while(!strcmp(SelectedProperty->Name,"MenuParent") || !strcmp(SelectedProperty->Name,"MenuBarParent"))
        {
          SelectedProperty = SelectedProperty->Next;
        }
      }

      SelectedValue = i;
            /* Copy in the new value for the selected property. */
      strcpy(SelectedProperty->Value,PropertyValue[SelectedValue]->Text);
//      printf("Set %s to %s\n",SelectedProperty->Name,PropertyValue[SelectedValue]->Text); fflush(stdout);

      /* If it is a window being updated, see if it is visible. If so then update the window visually. */
      if(!strcmp(SelectedObject->Type,"OnyxWindow"))
      {
        /* Fond the window to see if it is visible. */
        for(i = 0; i < MaxDevelopmentWindows; i++)
        {
          /* If the selected object and window to list map agree then we have a visible window. */
          if(SelectedObject == WindowToListMap[i])
          {
            UpdateDevelopmentWindow(i,SelectedProperty);
            i = MaxDevelopmentWindows; /* Leave loop. */
          }
        }
      }
      else  /* Its an object being updated. See if it is visible and if so make the visible change. */
      {
        for(i = 0; i < MaxDevelopmentObjects; i++)
        {
          if(DevelopmentObject[i] != NULL)
          {
            if(DevelopmentObject[i]->ID == SelectedItemID)
            {
              UpdateDevelopmentObject(i,SelectedProperty);
              i = MaxDevelopmentObjects; /* Leave loop. */
            }
          }
        }
      }
    }
  }
}

void ToolboxWindowEvent(OnyxMessage *msg) {
  if (msg->action == ButtonClicked) {
    if (LastToolSelected != msg->id - ToolboxButtonIDModifier) {
      ToolboxItem[LastToolSelected]->Pop();  /* Pop up previous button. */
      /* Make note of which button was selected. */
      LastToolSelected = msg->id - ToolboxButtonIDModifier;
    }
  } else if (msg->action == ButtonReleased && msg->button == 3) {
    /* Right clicking on a button is the same as right clicking on the window. */
    CustomControlsWindow->Show();
  }
}


void DevelopmentWindowEvent(OnyxMessage *msg) {
  int i, j;
  int newx, newy, newWidth, newHeight;

  struct ProjectObjectList *SelectedObjectLocal;
  struct ObjectList *UsableObject;
  struct ProjectObjectList *NewObject;
  struct ProjectObjectPropertyList *SelectedObjectProperty;

  SelectedObjectLocal = SelectedObject; /* Make local copy to avoid conflicts; */

  switch (msg->action) {

    case ButtonPressed:  /* Was a window just selected? */
      SelectedItemID = msg->id;

      MoveBrackets(-10, -10, -10, -10);

      if (LastToolSelected == 0) {  /* Is the pointer selected? */
        SelectedItemID = msg->id;
        CurrentDevelopmentWindow = msg->id - DevelopmentWindowIDModifier;
        SelectedWindow = WindowToListMap[CurrentDevelopmentWindow];

        /* We must reset SelectedWindow in the future. So multiple windows can be developed at the same time. */
        SelectedObject = SelectedWindow;

        if (PropertiesWindow->IsVisible) {  /* If the Properties window is visible. */
          PropertiesWindow_Update();
        }

        if (ProjectExplorer->IsVisible) {
          ExplorerTree->SelectItem(SelectedWindow->Name);
        }

      } else if (ControlsLocked) {  /* If not the pointer and controls locked then beep error. */

        XBell(ApplicationRoot()->GetDisplay(), 0);

      } else if (SelectedWindow == NULL) {

        AsteriskBox(MainWindow->Self(), "Put it where?", "Application is selected. Cannot add objects to it.");

      } else {  /* If controls not locked and application not selected */
                /* then add new object. */
        UsableObject = UsableObjects;  /* Get proper object from list. */
        i = 0;
        while (UsableObject != NULL) {

          if (UsableObject->ToolboxViewable) i++;

          if (i >= LastToolSelected) 
            break;
          else
            UsableObject = UsableObject->Next;

        }

        /* Override the directory entry for an icon so that something appears. */

        if (!strcmp(UsableObject->Name, "OnyxIcon")) {
          NewObject = AddDevelopmentObject(UsableObject, SelectedWindow->Name, msg->x, msg->y, "Directory", OnyxIconLocation);
        } else {
          NewObject = AddDevelopmentObject(UsableObject, SelectedWindow->Name, msg->x, msg->y, "", "");
        }

        CreateDevelopmentItem(NewObject);

        /* Destroy and recreate Brackets so they can be on top of the new objects */
        DestroyBrackets();
        CreateBrackets();

        DevelopmentWindow[CurrentDevelopmentWindow]->Show();

        if (PropertiesWindow->IsVisible) {
          PropertiesWindow_Update();
        }

        if (ProjectExplorer->IsVisible) {
          ProjectExplorer_Update();
        }
        

        if (Toolbox->IsVisible) {
          ToolboxItem[LastToolSelected]->Pop();  /* Pop up previous button. */
          ToolboxItem[0]->Push();      /* Push in the selector. */
        }
        LastToolSelected = 0;
      }
      break;

    case KeyReleased:
    {
      if((SelectedItemID == msg->id) && (msg->keysym == XK_Delete))
      {
        if(ControlsLocked)  /* If controls locked then beep error. */
        {
          XBell(ApplicationRoot()->GetDisplay(), 0);
        }
        else  /* Else delete the object. */
        {
          DeleteDevelopmentObject(SelectedObjectLocal);

          SelectedWindow = SelectedObject; /* Delete just updated selected object. Thus, since */
                   /* we just killed our window we better update. */

          if(PropertiesWindow->IsVisible)
          {
            PropertiesWindow_Update();
          }

          if(ProjectExplorer->IsVisible)
          {
            ProjectExplorer_Update();
          }

          if(DevelopmentWindow[CurrentDevelopmentWindow]->IsVisible)
          {
            ViewWindowButton_ButtonClick();
          }
        }
      }
    } break;

    case WindowMoved:
      SelectedObjectProperty = SelectedWindow->Property;
      while (strcmp(SelectedObjectProperty->Name, "X")) {
        SelectedObjectProperty = SelectedObjectProperty->Next;
      }
      sprintf(SelectedObjectProperty->Value, "%i", msg->x);

      SelectedObjectProperty = SelectedWindow->Property;
      while (strcmp(SelectedObjectProperty->Name, "Y")) {
        SelectedObjectProperty = SelectedObjectProperty->Next;
      }
      sprintf(SelectedObjectProperty->Value, "%i", msg->y);

      if (PropertiesWindow->IsVisible) {
        PropertiesWindow_Update();
      }
      break;

    case WindowResized:
      SelectedObjectProperty = SelectedWindow->Property;
      while (strcmp(SelectedObjectProperty->Name, "Width")) {
        SelectedObjectProperty = SelectedObjectProperty->Next;
      }
      sprintf(SelectedObjectProperty->Value, "%i", msg->width);

      SelectedObjectProperty = SelectedWindow->Property;
      while (strcmp(SelectedObjectProperty->Name, "Height")) {
        SelectedObjectProperty = SelectedObjectProperty->Next;
      }
      sprintf(SelectedObjectProperty->Value, "%i", msg->height);

      if (PropertiesWindow->IsVisible) {
        PropertiesWindow_Update();
      }
      break;

    case ButtonReleased:
      /* If controls arent locked and snap to grid is on then perform snap. */
      if (!ControlsLocked && SnapToGridIsOn) {

        /* Locate X value. */
        SelectedProperty = SelectedWindow->Property;
        while (strcmp(SelectedProperty->Name, "X")) {
          SelectedProperty = SelectedProperty->Next;
        }

        newx = atoi(SelectedProperty->Value);
//        newx = SNAP(newx);
        sprintf(SelectedProperty->Value, "%i", newx);

        /* Locate Y value. */
        SelectedProperty = SelectedWindow->Property;
        while (strcmp(SelectedProperty->Name, "Y")) {
          SelectedProperty = SelectedProperty->Next;
        }

        newy = atoi(SelectedProperty->Value);
//        newy = SNAP(newy);
        sprintf(SelectedProperty->Value, "%i", newy);

        /* Locate Width value. */
        SelectedProperty = SelectedWindow->Property;
        while (strcmp(SelectedProperty->Name, "Width")) {
          SelectedProperty = SelectedProperty->Next;
        }
        newWidth = atoi(SelectedProperty->Value);
//        newWidth = SNAP(newWidth);
        sprintf(SelectedProperty->Value, "%i", newWidth);

        /* Locate Height value. */
        SelectedProperty = SelectedWindow->Property;
        while (strcmp(SelectedProperty->Name, "Height")) {
          SelectedProperty = SelectedProperty->Next;
        }
        newHeight = atoi(SelectedProperty->Value);
//        newHeight = SNAP(newHeight);
        sprintf(SelectedProperty->Value, "%i", newHeight);

        if ((newx != DevelopmentWindow[CurrentDevelopmentWindow]->X) || (newy != DevelopmentWindow[CurrentDevelopmentWindow]->Y)) {
          DevelopmentWindow[CurrentDevelopmentWindow]->Move(newx, newy);
        }

        if ((newWidth != DevelopmentWindow[CurrentDevelopmentWindow]->Width) || (newHeight != DevelopmentWindow[CurrentDevelopmentWindow]->Height)) {
          DevelopmentWindow[CurrentDevelopmentWindow]->Resize(newWidth, newHeight);
        }

        if (PropertiesWindow->IsVisible) {
          PropertiesWindow_Update();
        }
      }
      break;

    case WindowClosed:
      DestroyBrackets();

      j = msg->id - DevelopmentWindowIDModifier;
      SelectedObjectLocal = WindowToListMap[j];

      /* Delete all attached items (not objects) and set their values to NULL. */
      for (i = 0; i < MaxDevelopmentObjects; i++) {
        /* If this item exists and has the closing window as it parent */
        if (DevelopmentItem[i] != NULL && !strcmp(DevelopmentItem[i]->Parent, SelectedObjectLocal->Name)) {
          DevelopmentItem[i] = NULL;
        }
      }
      break;

  }
}

void DevelopmentItemEvent(OnyxMessage *msg) {
  int i;

  int newx, newy, newWidth, newHeight;

  struct ProjectObjectList *SelectedObjectLocal;
  struct ProjectObjectPropertyList *SelectedObjectProperty;

  SelectedObjectLocal = SelectedObject;

  if (LastToolSelected == 0) {   // Is the pointer object selected?

    if (msg->action == ButtonPressed) {  // Was an object just selected?

      i = msg->id - DevelopmentItemIDModifier;  // Update the internal SelectedObject.
      SelectedItemID = msg->id;
      SelectedObject = DevelopmentItem[i];
      SelectedObjectLocal = SelectedObject;

      // Move bracketing corners to surround selected object.
      MoveBrackets(DevelopmentObject[i]->X, DevelopmentObject[i]->Y,
                   DevelopmentObject[i]->Width, DevelopmentObject[i]->Height);

      // Remember where the click was. These coordinates
      // are used later in moving the object.
      ItemClickX = msg->x;
      ItemClickY = msg->y;

      DevelopmentButtonDown = msg->id;  // Button is down. Dragging and resizing is allowed.

      if (PropertiesWindow->IsVisible) {  // If the Properties window is visible.
        PropertiesWindow_Update();
      }

      if (ProjectExplorer->IsVisible) {
        ExplorerTree->SelectItem(SelectedObjectLocal->Name);
      }

    } else if (((msg->action == ButtonReleased) || (msg->id != DevelopmentButtonDown)) &&
                (msg->action != Paint)) {

      // If the button was released or object changed then disallow moving
      // and resizing.

      // If the button was just released over the object we were last
      // working with AND snap to grid is enabled AND Controls arent locked,
      // then perform the snap.

      if (msg->id == DevelopmentButtonDown && SnapToGridIsOn && !ControlsLocked) {
        i = msg->id - DevelopmentItemIDModifier;

        // Record move in the internal SelectedObject
        for (SelectedObjectProperty = SelectedObjectLocal->Property;
             (SelectedObjectProperty != NULL) && strcmp(SelectedObjectProperty->Name, "X");
             SelectedObjectProperty = SelectedObjectProperty->Next) {}

        if (SelectedObjectProperty != NULL) {
          newx = atoi(SelectedObjectProperty->Value);
          newx = SNAP(newx);
          sprintf(SelectedObjectProperty->Value, "%i", newx);
        } else {
          newx = DevelopmentObject[i]->X;
          newx = SNAP(newx);
        }
  
        for (SelectedObjectProperty = SelectedObjectLocal->Property;
             (SelectedObjectProperty != NULL) && strcmp(SelectedObjectProperty->Name, "Y");
             SelectedObjectProperty = SelectedObjectProperty->Next) {}

        if (SelectedObjectProperty != NULL) {
          newy = atoi(SelectedObjectProperty->Value);
          newy = SNAP(newy);
          sprintf(SelectedObjectProperty->Value, "%i", newy);
        } else {
          newy = DevelopmentObject[i]->Y;
          newy = SNAP(newy);
        }

        // Record resize in the internal SelectedObject
        for (SelectedObjectProperty = SelectedObjectLocal->Property;
             (SelectedObjectProperty != NULL) && strcmp(SelectedObjectProperty->Name, "Width");
             SelectedObjectProperty = SelectedObjectProperty->Next) {}

        if (SelectedObjectProperty != NULL) {
          newWidth = atoi(SelectedObjectProperty->Value);
          newWidth = SNAP(newWidth);
          sprintf(SelectedObjectProperty->Value, "%i", newWidth);
        } else {
          newWidth = DevelopmentObject[i]->Width;
        }

        for (SelectedObjectProperty = SelectedObjectLocal->Property;
             (SelectedObjectProperty != NULL) && strcmp(SelectedObjectProperty->Name, "Height");
             SelectedObjectProperty = SelectedObjectProperty->Next) {}

        if (SelectedObjectProperty != NULL) {
          newHeight = atoi(SelectedObjectProperty->Value);
          newHeight = SNAP(newHeight);
          sprintf(SelectedObjectProperty->Value, "%i", newHeight);
        } else {
          newHeight = DevelopmentObject[i]->Height;
          newHeight = SNAP(newHeight);
        }

        // MoveResize the displayed object

        DevelopmentObject[i]->MoveResize(newx, newy, newWidth, newHeight);

        if (!strcmp(SelectedObject->Type, "OnyxButton")) {
          DevelopmentObject[i]->Update();
        }

        // Move bracketing corners to surround selected object.
        MoveBrackets(DevelopmentObject[i]->X, DevelopmentObject[i]->Y,
                     DevelopmentObject[i]->Width, DevelopmentObject[i]->Height);

        if (PropertiesWindow->IsVisible) {
          PropertiesWindow_Update();
        }
      }

      // Now change the status to stop moves and snaps when button isn't down.
      DevelopmentButtonDown = False;
    }

    if (SelectedObjectLocal != NULL) {  // If we aren't using "Application" (though I don't know how this would happen)
      switch (msg->action) {
        case PointerMoved:              // If the mouse was moved and
          if (DevelopmentButtonDown) {  // drag/resize allowed
            if (!ControlsLocked) {

              // Record move in the internal SelectedObject
              for (SelectedObjectProperty = SelectedObjectLocal->Property;
                   (SelectedObjectProperty != NULL) && strcmp(SelectedObjectProperty->Name, "X");
                   SelectedObjectProperty = SelectedObjectProperty->Next) {}

              if (SelectedObjectProperty != NULL) {
                newx = atoi(SelectedObjectProperty->Value) + msg->x - ItemClickX;
                sprintf(SelectedObjectProperty->Value, "%i", newx);
              } else {
                newx = DevelopmentObject[msg->id - DevelopmentItemIDModifier]->X + msg->x - ItemClickX;
              }
  
              for (SelectedObjectProperty = SelectedObjectLocal->Property;
                   (SelectedObjectProperty != NULL) && strcmp(SelectedObjectProperty->Name, "Y");
                   SelectedObjectProperty = SelectedObjectProperty->Next) {}

              if (SelectedObjectProperty != NULL) {
                newy = atoi(SelectedObjectProperty->Value) + msg->y - ItemClickY;
                sprintf(SelectedObjectProperty->Value, "%i", newy);
              } else {
                newy = DevelopmentObject[msg->id - DevelopmentItemIDModifier]->Y + msg->y - ItemClickY;
              }

              // Move the displayed object
              DevelopmentObject[msg->id - DevelopmentItemIDModifier]->Move(newx, newy);

              // Move bracketing corners to surround selected object.
              i = msg->id - DevelopmentItemIDModifier;
              MoveBrackets(DevelopmentObject[i]->X, DevelopmentObject[i]->Y,
                           DevelopmentObject[i]->Width, DevelopmentObject[i]->Height);

            }

            if (PropertiesWindow->IsVisible) {
              PropertiesWindow_Update();
            }

          } // End if development button down
          break; // End case pointer moved

        case KeyReleased:
          if (ControlsLocked) {  // If controls locked then beep error.
            XBell(ApplicationRoot()->GetDisplay(), 0);
          } else if ((SelectedItemID == msg->id) && (msg->keysym == XK_Delete)) {  // Else delete object.
            XSetInputFocus(ApplicationRoot()->GetDisplay(), None, None, CurrentTime);  // ==!==
            DeleteDevelopmentObject(SelectedObjectLocal);

            // Delete the visible object.
            i  = msg->id - DevelopmentItemIDModifier;
            if (DevelopmentObject[i] != NULL) {
              delete DevelopmentObject[i];
              DevelopmentObject[i] = NULL;
              // Hide the brackets since there is no longer the object for them to surround.
              MoveBrackets(-10, -10, -10, -10);
            }

            if (PropertiesWindow->IsVisible) {
              PropertiesWindow_Update();
            }

            if (ProjectExplorer->IsVisible) {
              ProjectExplorer_Update();
            }

            if (DevelopmentWindow[CurrentDevelopmentWindow]->IsVisible) {
              DevelopmentWindow[CurrentDevelopmentWindow]->Update();
            }
          }
          break;
      }
    }
  }
}

void CustomControlsEvent(OnyxMessage *msg) {
  struct ObjectList *UsableObject;

  FILE *oocfile;
  char *Marker;
  char *Marker2;
  char InLine[255];

  if (msg->action == ButtonClicked) {
    if (CCSelected != msg->id - CCIDModifier) {
      CCSelected = msg->id - CCIDModifier;
      CCList[CCSelected]->Toggle(); // Toggle back to allow for a selection before a checking.
    } else {
      oocfile = fopen(CCFileList[CCSelected], "r");
      if (oocfile) {
        fgets(InLine, 255, oocfile); // Name
        fgets(InLine, 255, oocfile); // oob files.
        fclose(oocfile);

        Marker = InLine;
        Marker2 = strchr(InLine, '\n'); // Get rid of newline
        if (Marker2) *Marker2 = '\0';

        Marker2 = strchr(InLine, ' ');  // Find oob file name separation.
        if (Marker2) {
          *Marker2 = '\0';
          Marker2++;
        }

        if (UsableObjects != NULL) {
          for (UsableObject = UsableObjects;
               UsableObject->Next != NULL;
               UsableObject = UsableObject->Next) {}
        }

        while (Marker != NULL) {
          if (CCList[CCSelected]->IsChecked) {
            if (UsableObjects == NULL) {
              UsableObjects = GetNewObjectType(Marker);
              UsableObject = UsableObjects;
            } else {
              UsableObject->Next = GetNewObjectType(Marker);
              UsableObject = UsableObject->Next;
            }
            InitializeInternalBaseMakefile();
          } else {
            RemoveObjectType(Marker);
            InitializeInternalBaseMakefile();
          }

          Marker = Marker2;
          if (Marker2 != NULL) {
            Marker2 = strchr(Marker, ' ');
          }
          if (Marker2) {
            *Marker2 = '\0';
            Marker2++;
          }
        }
      }            
      if (Toolbox->IsVisible) {
        Toolbox->Show();
      }
    }
  }
}

void GeneralEvent(OnyxMessage *msg) {

  // These general events are where we catch the events from the dynamic
  // objects we are creating and destroying in the ToolBox, ProjectExplorer,
  // etc.

  /* Was it a value in the Properties Window? */
  if((msg->id < PropertyIDModifier + PropertyIDMax) && (msg->id >= PropertyIDModifier))
  {
    PropertyWindowEvent(msg);
  }

  /* Was it a button in the Properties Window? */
  else if((msg->id < PropertyButtonIDModifier + PropertyButtonIDMax) && (msg->id >= PropertyButtonIDModifier))
  {
    PropertyWindowEvent(msg);
  }

  /* Was it a ToolBox button? */
  else if((msg->id < ToolboxButtonIDModifier + ToolboxButtonIDMax) && (msg->id >= ToolboxButtonIDModifier))
  {
    ToolboxWindowEvent(msg);
  }

  /* Was it on a window being developed? */
  else if((msg->id < DevelopmentWindowIDModifier + MaxDevelopmentWindows) && (msg->id >= DevelopmentWindowIDModifier))
  {
    DevelopmentWindowEvent(msg);
  }

  /* Was it an item on a window being developed? */
  else if((msg->id < DevelopmentItemIDModifier + DevelopmentItemIDMax) && (msg->id >= DevelopmentItemIDModifier))
  {
    DevelopmentItemEvent(msg);
  }

  /* Was it a check box on the custom controls window? */
  else if((msg->id < CCIDModifier + CCIDMax) && (msg->id >= CCIDModifier))
  {
    CustomControlsEvent(msg);
  }
}


struct ProjectObjectList *AddDevelopmentObject(struct ObjectList *UsableObject, char *Window, long x, long y, char *ParameterOverride, char *POValue)
{
/*ooo*/
  int OverrideActive;

  struct ProjectObjectList *ProjectObject;
  struct ObjectProperty *UsableProperty;
  struct ObjectEvent *UsableEvent;

  struct ProjectObjectPropertyList *ObjectProperty;

  InternalCodeFile *InternalFile;

  if (ProjectObjects == NULL) /* If we have a new or empty project then initialize ProjectObjects */
  {
    ProjectObjects = new ProjectObjectList;
    ProjectObject = ProjectObjects;
    ProjectObject->IsIncorporated = False;
  }
  else  /* Otherwise append to it. */
  {
    ProjectObject = ProjectObjects;
    while (ProjectObject->Next != NULL)  /* Get to the end of the list. */
    {
      ProjectObject = ProjectObject->Next;
    }

            /* Add new ProjectObject */
    ProjectObject->Next = new ProjectObjectList;
    ProjectObject = ProjectObject->Next;
    ProjectObject->IsIncorporated = False;
  }

  ProjectObject->Next = NULL;    /* Terminate the list properly. */
  ProjectObject->Property = NULL;    /* No properties initially. */
  ObjectProperty = NULL;

  OverrideActive = strlen(ParameterOverride);

  sprintf(ProjectObject->Name, "%s%i", UsableObject->Name, NextAvailableNewObjectNumber(UsableObject->Name));

  /* Create new internal oxc file */
  InternalOxcCode = new InternalCodeFile(ProjectObject->Name, NULL, InternalOxcCode);
  InternalFile = InternalOxcCode;
  InternalFile->IsIncorporated = False;
  InternalFile->HasBeenModified = True;

  /* Write the events into the oxc file */

  UsableEvent = UsableObject->Event;
  while (UsableEvent != NULL)
  {
    InternalFile->Append("void %s_%s(", ProjectObject->Name, UsableEvent->Name);
    if (strlen(UsableEvent->Parm1) > 0)
    {
      InternalFile->Append("long %s", UsableEvent->Parm1);
      if (strlen(UsableEvent->Parm2) > 0)
      {
        InternalFile->Append(", long %s", UsableEvent->Parm2);
        if (strlen(UsableEvent->Parm3) > 0)
        {
          InternalFile->Append(", long %s", UsableEvent->Parm3);
          if (strlen(UsableEvent->Parm4) > 0)
          {
            InternalFile->Append(", long %s", UsableEvent->Parm4);
          }
        }
      }
    }
    InternalFile->Append(")\n{\n}\n\n");
    UsableEvent = UsableEvent->Next;
  }

  strcpy(ProjectObject->Type, UsableObject->Name);
  strcpy(ProjectObject->Parent, Window);

  UsableProperty = UsableObject->Property;  /* Cycle through all usable properties available */
                                            /* to this object type. */
  while (UsableProperty != NULL)
  {
    if (ObjectProperty == NULL)
    {
      ProjectObject->Property = new ProjectObjectPropertyList;
      ObjectProperty = ProjectObject->Property;
    }
    else
    {
      ObjectProperty->Next = new ProjectObjectPropertyList;
      ObjectProperty = ObjectProperty->Next;
    }

    ObjectProperty->Next = NULL;

    if (!strcmp(UsableProperty->Name, "X"))
    {
      strcpy(ObjectProperty->Name, "X");
      sprintf(ObjectProperty->Value, "%i", x);
    }
    else if (!strcmp(UsableProperty->Name, "Y"))
    {
      strcpy(ObjectProperty->Name, "Y");
      sprintf(ObjectProperty->Value, "%i", y);
    }
    else
    {
      strcpy(ObjectProperty->Name, UsableProperty->Name);
      if (OverrideActive && !strcmp(ParameterOverride, ObjectProperty->Name))
      {
        strcpy(ObjectProperty->Value, POValue);
      }
      else
      {
        strcpy(ObjectProperty->Value, UsableProperty->Default);
      }
    }

    UsableProperty = UsableProperty->Next;
  }


  SelectedObject = ProjectObject;

  /* Are the following still needed now that IsIncorporated attributes in InternalFile and ProjectObjectList exist? */
  InitializeInternalMakefile();  /* Sync Makefile with project to reflect changes. */
  InitializeInternalProjectFile(ProjectFile);  /* Sync Project file with project to reflect changes. */

  return ProjectObject;
}

void TLCorner_ButtonPressed(int Button, int x, int y)
{
  ItemClickX = x;
  ItemClickY = y;
  DevelopmentButtonDown = True;
}

void TLCorner_ButtonReleased(int Button, int x, int y)
{
  DevelopmentButtonDown = False;
}

void TLCorner_PointerMoved(int x, int y, int rootX, int rootY)
{
  int i;
  
  long newX;
  long newY;
  long newWidth;
  long newHeight;
  struct ProjectObjectList *SelectedObjectLocal; /* Prevents conflicts if selected object is changed by something else */
                                                 /* while we are processing. */
  struct ProjectObjectPropertyList *SelectedObjectProperty;

  if (DevelopmentButtonDown)
  {
    SelectedObjectLocal = SelectedObject;

    /* Adjust affected corners. */
    TLCorner->Move(TLCorner->X + x - ItemClickX,TLCorner->Y + y - ItemClickY);
    TRCorner->Move(TRCorner->X, TRCorner->Y + y - ItemClickY);
    BLCorner->Move(BLCorner->X + x - ItemClickX,BLCorner->Y);

    /* Get DevelopmentItem/Object array position of selected object. */
    i = SelectedItemID - DevelopmentItemIDModifier;

    /* Record move in the internal SelectedObject */
    for(SelectedObjectProperty = SelectedObjectLocal->Property; (SelectedObjectProperty != NULL) && strcmp(SelectedObjectProperty->Name,"X"); SelectedObjectProperty = SelectedObjectProperty->Next)
    {}
    newX = DevelopmentObject[i]->X + x - ItemClickX;
    if(SelectedObjectProperty != NULL)
    {
      sprintf(SelectedObjectProperty->Value,"%i",newX);
    }
  
    for(SelectedObjectProperty = SelectedObjectLocal->Property; (SelectedObjectProperty != NULL) && strcmp(SelectedObjectProperty->Name,"Y"); SelectedObjectProperty = SelectedObjectProperty->Next)
    {}
    newY = DevelopmentObject[i]->Y + y - ItemClickY;
    if(SelectedObjectProperty != NULL)
    {
      sprintf(SelectedObjectProperty->Value,"%i",newY);
    }

    /* Record resize in the internal SelectedObject */
    for(SelectedObjectProperty = SelectedObjectLocal->Property; (SelectedObjectProperty != NULL) && strcmp(SelectedObjectProperty->Name,"Width"); SelectedObjectProperty = SelectedObjectProperty->Next)
    {}
    newWidth = DevelopmentObject[i]->Width - x + ItemClickX;
    if(SelectedObjectProperty != NULL)
    {
      sprintf(SelectedObjectProperty->Value,"%i",newWidth);
    }

    for(SelectedObjectProperty = SelectedObjectLocal->Property; (SelectedObjectProperty != NULL) && strcmp(SelectedObjectProperty->Name,"Height"); SelectedObjectProperty = SelectedObjectProperty->Next)
    {}
    newHeight = DevelopmentObject[i]->Height - y + ItemClickY;
    if(SelectedObjectProperty != NULL)
    {
      sprintf(SelectedObjectProperty->Value,"%i",newHeight);
    }

    /* Resize the displayed object */
    DevelopmentObject[i]->MoveResize(newX,newY,newWidth,newHeight);

    if(PropertiesWindow->IsVisible)
    {
      PropertiesWindow_Update();
    }

  }
}

void TRCorner_ButtonPressed(int Button, int x, int y)
{
  ItemClickX = x;
  ItemClickY = y;
  DevelopmentButtonDown = True;
}

void TRCorner_ButtonReleased(int Button, int x, int y)
{
  DevelopmentButtonDown = False;
}

void TRCorner_PointerMoved(int x, int y, int rootX, int rootY)
{
  int i;
  
  long newX;
  long newY;
  long newWidth;
  long newHeight;
  struct ProjectObjectList *SelectedObjectLocal; /* Prevents conflicts if selected object is chaneg by something else*/
              /* while we are processing. */
  struct ProjectObjectPropertyList *SelectedObjectProperty;

  if(DevelopmentButtonDown)
  {
    SelectedObjectLocal = SelectedObject;

    /* Adjust affected corners. */
    TLCorner->Move(TLCorner->X, TLCorner->Y + y - ItemClickY);
    TRCorner->Move(TRCorner->X + x - ItemClickX,TRCorner->Y + y - ItemClickY);
    BRCorner->Move(BRCorner->X + x - ItemClickX,BRCorner->Y);

    /* Get DevelopmentItem/Object array position of selected object. */
    i = SelectedItemID - DevelopmentItemIDModifier;

    /* Record move in the internal SelectedObject */
    newX = DevelopmentObject[i]->X;
  
    for(SelectedObjectProperty = SelectedObjectLocal->Property; (SelectedObjectProperty != NULL) && strcmp(SelectedObjectProperty->Name,"Y"); SelectedObjectProperty = SelectedObjectProperty->Next)
    {}
    newY = DevelopmentObject[i]->Y + y - ItemClickY;
    if(SelectedObjectProperty != NULL)
    {
      sprintf(SelectedObjectProperty->Value,"%i",newY);
    }

    /* Record resize in the internal SelectedObject */
    for(SelectedObjectProperty = SelectedObjectLocal->Property; (SelectedObjectProperty != NULL) && strcmp(SelectedObjectProperty->Name,"Width"); SelectedObjectProperty = SelectedObjectProperty->Next)
    {}
    newWidth = DevelopmentObject[i]->Width + x - ItemClickX;
    if(SelectedObjectProperty != NULL)
    {
      sprintf(SelectedObjectProperty->Value,"%i",newWidth);
    }

    for(SelectedObjectProperty = SelectedObjectLocal->Property; (SelectedObjectProperty != NULL) && strcmp(SelectedObjectProperty->Name,"Height"); SelectedObjectProperty = SelectedObjectProperty->Next)
    {}
    newHeight = DevelopmentObject[i]->Height - y + ItemClickY;
    if(SelectedObjectProperty != NULL)
    {
      sprintf(SelectedObjectProperty->Value,"%i",newHeight);
    }

    /* Resize the displayed object */
    DevelopmentObject[i]->MoveResize(newX,newY,newWidth,newHeight);

    if(PropertiesWindow->IsVisible)
    {
      PropertiesWindow_Update();
    }

  }
}

void BLCorner_ButtonPressed(int Button, int x, int y)
{
  ItemClickX = x;
  ItemClickY = y;
  DevelopmentButtonDown = True;
}

void BLCorner_ButtonReleased(int Button, int x, int y)
{
  DevelopmentButtonDown = False;
}

void BLCorner_PointerMoved(int x, int y, int rootX, int rootY)
{
  int i;
  
  long newX;
  long newY;
  long newWidth;
  long newHeight;
  struct ProjectObjectList *SelectedObjectLocal; /* Prevents conflicts if selected object is chaneg by something else*/
              /* while we are processing. */
  struct ProjectObjectPropertyList *SelectedObjectProperty;

  if(DevelopmentButtonDown)
  {
    SelectedObjectLocal = SelectedObject;

    /* Adjust affected corners. */
    TLCorner->Move(TLCorner->X + x - ItemClickX,TLCorner->Y);
    BLCorner->Move(BLCorner->X + x - ItemClickX,BLCorner->Y + y - ItemClickY);
    BRCorner->Move(BRCorner->X, BRCorner->Y + y - ItemClickY);

    /* Get DevelopmentItem/Object array position of selected object. */
    i = SelectedItemID - DevelopmentItemIDModifier;

    /* Record move in the internal SelectedObject */
    for(SelectedObjectProperty = SelectedObjectLocal->Property; (SelectedObjectProperty != NULL) && strcmp(SelectedObjectProperty->Name,"X"); SelectedObjectProperty = SelectedObjectProperty->Next)
    {}
    newX = DevelopmentObject[i]->X + x - ItemClickX;
    if(SelectedObjectProperty != NULL)
    {
      sprintf(SelectedObjectProperty->Value,"%i",newX);
    }
  
    newY = DevelopmentObject[i]->Y;

    /* Record resize in the internal SelectedObject */
    for(SelectedObjectProperty = SelectedObjectLocal->Property; (SelectedObjectProperty != NULL) && strcmp(SelectedObjectProperty->Name,"Width"); SelectedObjectProperty = SelectedObjectProperty->Next)
    {}
    newWidth = DevelopmentObject[i]->Width - x + ItemClickX;
    if(SelectedObjectProperty != NULL)
    {
      sprintf(SelectedObjectProperty->Value,"%i",newWidth);
    }

    for(SelectedObjectProperty = SelectedObjectLocal->Property; (SelectedObjectProperty != NULL) && strcmp(SelectedObjectProperty->Name,"Height"); SelectedObjectProperty = SelectedObjectProperty->Next)
    {}
    newHeight = DevelopmentObject[i]->Height + y - ItemClickY;
    if(SelectedObjectProperty != NULL)
    {
      sprintf(SelectedObjectProperty->Value,"%i",newHeight);
    }

    /* Resize the displayed object */
    DevelopmentObject[i]->MoveResize(newX,newY,newWidth,newHeight);

    if(PropertiesWindow->IsVisible)
    {
      PropertiesWindow_Update();
    }

  }
}

void BRCorner_ButtonPressed(int Button, int x, int y)
{
  ItemClickX = x;
  ItemClickY = y;
  DevelopmentButtonDown = True;
}

void BRCorner_ButtonReleased(int Button, int x, int y)
{
  DevelopmentButtonDown = False;
}

void BRCorner_PointerMoved(int x, int y, int rootX, int rootY)
{
  int i;
  long newX;
  long newY;
  long newWidth;
  long newHeight;
  struct ProjectObjectList *SelectedObjectLocal; /* Prevents conflicts if selected object is chaneg by something else*/
              /* while we are processing. */
  struct ProjectObjectPropertyList *SelectedObjectProperty;

  if(DevelopmentButtonDown)
  {
    SelectedObjectLocal = SelectedObject;

    /* Adjust affected corners. */
    TRCorner->Move(TRCorner->X + x - ItemClickX,TRCorner->Y);
    BLCorner->Move(BLCorner->X, BLCorner->Y + y - ItemClickY);
    BRCorner->Move(BRCorner->X + x - ItemClickX,BRCorner->Y + y - ItemClickY);

    /* Get DevelopmentItem/Object array position of selected object. */
    i = SelectedItemID - DevelopmentItemIDModifier;

    newX = DevelopmentObject[i]->X;
    newY = DevelopmentObject[i]->Y;

    /* Record resize in the internal SelectedObject */
    for(SelectedObjectProperty = SelectedObjectLocal->Property; (SelectedObjectProperty != NULL) && strcmp(SelectedObjectProperty->Name,"Width"); SelectedObjectProperty = SelectedObjectProperty->Next)
    {}

    newWidth = DevelopmentObject[i]->Width + x - ItemClickX;
    if(SelectedObjectProperty != NULL)
    {
      sprintf(SelectedObjectProperty->Value,"%i",newWidth);
    }

    for(SelectedObjectProperty = SelectedObjectLocal->Property; (SelectedObjectProperty != NULL) && strcmp(SelectedObjectProperty->Name,"Height"); SelectedObjectProperty = SelectedObjectProperty->Next)
    {}

    newHeight = DevelopmentObject[i]->Height + y - ItemClickY;
    if(SelectedObjectProperty != NULL)
    {
      sprintf(SelectedObjectProperty->Value,"%i",newHeight);
    }

    /* Resize the displayed object */
    DevelopmentObject[i]->MoveResize(newX,newY,newWidth,newHeight);


    if(PropertiesWindow->IsVisible)
    {
      PropertiesWindow_Update();
    }

  }
}

void RenameDevelopmentObject(struct ProjectObjectList *Object, char *NewName)
{
  InternalCodeFile *tmp;
  struct ProjectObjectList *ProjectObject;
  struct ProjectObjectPropertyList *Property;

  tmp = InternalOxcCode;

  while(strcmp(tmp->ObjectName,Object->Name))  /* Locate object */
  {
    tmp = tmp->Next;
  }

  /* Rewrite the procedures and such in the code to reflect the name change. */
  tmp->Replace(Object->Name,strlen(Object->Name), NewName, strlen(NewName));

  /* Rename the code. */
  strcpy(tmp->OldObjectName,tmp->ObjectName);
  strcpy(tmp->ObjectName,NewName);

  /* Rename the object */
  strcpy(Object->Name,NewName);

  /* Note that the code has changed and renamed. */
  tmp->HasBeenModified = True;
  tmp->HasBeenRenamed = True;

  /* Change any children */
  for(ProjectObject = ProjectObjects; ProjectObject != NULL; ProjectObject = ProjectObject->Next)
  {
    if(!strcmp(tmp->OldObjectName,ProjectObject->Parent))
    {
      strcpy(ProjectObject->Parent,tmp->ObjectName);
      /* Do we need to mark modified here ? */
    }
  }

  /* Handle the renaming of menus and menu bars. */
  if(!strcmp(Object->Type,"OnyxMenu") || !strcmp(Object->Type,"OnyxMenuBar"))
  {
    /* Cycle through all objects in the project. */
    for(ProjectObject = ProjectObjects; ProjectObject != NULL; ProjectObject = ProjectObject->Next)
    {
      /* If we find a menu or a menu item its a candidate for a change. */
      if(!strcmp(ProjectObject->Type,"OnyxMenu") || !strcmp(ProjectObject->Type,"OnyxMenuItem"))
      {
        /* Check its properties. */
        for(Property = ProjectObject->Property; Property != NULL; Property = Property->Next)
        {
          /* If this property is a menu parent or menu bar parent. */
          if(!strcmp(Property->Name,"MenuParent") || !strcmp(Property->Name,"MenuBarParent"))
          {
            /* If its value is the old object name */
            if(!strcmp(tmp->OldObjectName,Property->Value))
            {
              /* Update it. */
              strcpy(Property->Value,tmp->ObjectName);
              /* Do we need to mark modified here ? */
            }
          }
        }
      }
    }
  }  
}


void DeleteDevelopmentObject(struct ProjectObjectList *Object)
{
  struct ProjectObjectList *NextObject;
  struct ProjectObjectList *PreviousObject;
  struct ProjectObjectList *ParentObject;
  struct ProjectObjectList *DeletableObject;
  struct ProjectObjectList *EndOfDeletedList;
  struct ProjectObjectPropertyList *Property;
  InternalCodeFile *InternalCode;
  char OName[255];
  int IsMenu;

  IsMenu = 0;  /* Assume not a menu. */

  strcpy(OName, Object->Name); /* Copy the moronic sucker because when we do a delete *Object won't point to jack didly! */
                               /* the above may no longer be true with a delete list implemented. */

  /* We better reset the SelectedObject or we could have problems later. */

  /* If this is a menu item then select the menu parent. */
  if (!strcmp(Object->Type, "OnyxMenuItem"))
  {
    /* Find the name of the menu parent. */
    for (Property = Object->Property; strcmp(Property->Name, "MenuParent"); Property = Property->Next)
    {}

    /* Find the actual object matching the name. */
    for (NextObject = ProjectObjects; (NextObject != NULL) && strcmp(NextObject->Name, Property->Value); NextObject = NextObject->Next)
    {}
  }
  /* If this is a menu entry then select the menu parent or the menu bar parent. */
  else if (!strcmp(Object->Type, "OnyxMenu"))
  {
    IsMenu = 1;

    /* Find the name of the menu parent. */
    for (Property = Object->Property; strcmp(Property->Name, "MenuParent"); Property = Property->Next)
    {}

    /* If the menu parent is null then it is attached to a menu bar. */
    if (!strcmp(Property->Value, "NULL"))
    {
      /* Find the name of the menu bar parent. */
      for (Property = Object->Property; strcmp(Property->Name, "MenuBarParent"); Property = Property->Next)
      {}
    }

    /* Find the actual object matching the name. */
    for (NextObject = ProjectObjects; (NextObject != NULL) && strcmp(NextObject->Name, Property->Value); NextObject = NextObject->Next)
    {}
  }
  else  /* This is just a regular item. Find the parent. */
  {
    /* If this is a menu bar mark it as a menu as well. */
    if (!strcmp(Object->Type, "OnyxMenuBar"))
    {
      IsMenu = 2;
    }

    for (NextObject = ProjectObjects; (NextObject != NULL) && strcmp(NextObject->Name, Object->Parent); NextObject = NextObject->Next)
    {}
  }


  ParentObject = NextObject; /* Note: If NextObject is NULL here then this action will work because */
                             /* a ParentObject = NULL puts us at "Applicaiton". Which is where it should go. */

  NextObject = ProjectObjects;
  PreviousObject = NULL;

  /* Go through the list recursively removing any children. */
  NextObject = ProjectObjects; 
  while (NextObject != NULL)
  {
    DeletableObject = NULL;  /* Set to NULL to indicate nothing found yet. */

    if (!strcmp(NextObject->Parent, OName)) /* If this is a child of the object. */
    {
      DeletableObject = NextObject;
    }
    /* If this delete is being performed on a menu type object and this is a menu or menu item. */
    else if ((IsMenu == 1) && (!strcmp(NextObject->Type, "OnyxMenu") || !strcmp(NextObject->Type, "OnyxMenuItem")))
    {
      /* Find the name of the menu parent. */
      for (Property = NextObject->Property; strcmp(Property->Name, "MenuParent"); Property = Property->Next)
      {}

      /* If the menu parent is our object being deleted. */
      if (!strcmp(Property->Value, OName))
      {
        DeletableObject = NextObject;
      }
    }
    /* If this delete is being performed on a menu bar type object and this is a menu. */
    else if (IsMenu == 2 && !strcmp(NextObject->Type, "OnyxMenu"))
    {
      /* Find the name of the menu bar parent. */
      for (Property = NextObject->Property; strcmp(Property->Name, "MenuBarParent"); Property = Property->Next)
      {}

      /* If the menu bar parent is our object being deleted. */
      if (!strcmp(Property->Value, OName))
      {
        DeletableObject = NextObject;
      }
    }

    NextObject = NextObject->Next;  /* Advance to the next object. */

    if (DeletableObject)  /* If we found something to delete then do it. */
    {
      DeleteDevelopmentObject(DeletableObject);
    }
  }

  /* Go through the list, find the original requested object and delete it. */
  NextObject = ProjectObjects;
  while (NextObject != NULL)
  {
    /* If this is the object. */
    if (!strcmp(NextObject->Name, OName))
    {
      /* Save as deletable. */
      DeletableObject = NextObject;

      /* Move Next Object. */
      NextObject = NextObject->Next;

      /* Remove DeletableObject from the list of project objects. */
      if (PreviousObject != NULL)
      {  /* This basically removes 'b' by setting 'a->next' equal to 'c'. */
        PreviousObject->Next = NextObject;
      }
      else  /* If previous object is still NULL then the object being deleted is at the beginning of */
      {  /*   the list. Move the start of the list to remove the deleteable object from it. */
        ProjectObjects = NextObject;
      }


      /* The following lines replace 'delete DeletableObject'; It will allow for an eventual undelete and such. */

      /* Cut the connection in DeletableObject->Next since it still points into the old list. */
      DeletableObject->Next = NULL;

      /* Toss the object onto the deleted objects list. */
      if (DeletedObjects == NULL)
      {
        DeletedObjects = DeletableObject;
      }
      else
      {
        for (EndOfDeletedList = DeletedObjects; EndOfDeletedList->Next != NULL; EndOfDeletedList = EndOfDeletedList->Next)
        {}
        EndOfDeletedList->Next = DeletableObject;
      }

      /* Mark the internal code as being deleted. */
      for (InternalCode = InternalOxcCode; InternalCode != NULL && strcmp(InternalCode->ObjectName, DeletableObject->Name); InternalCode = InternalCode->Next)
      {}
      if(InternalCode != NULL)
      {
        InternalCode->HasBeenDeleted = True;
      }

      /* Mark the parent's internal code as being modified. */
      for (InternalCode = InternalOxcCode; InternalCode != NULL && strcmp(InternalCode->ObjectName, DeletableObject->Parent); InternalCode = InternalCode->Next)
      {}
      if (InternalCode != NULL)
      {
        InternalCode->HasBeenModified = True;
      }
      break;
    }
    else  /* We don't have a match. Update Previous and Next objects. */
    {
      PreviousObject = NextObject;
      NextObject = NextObject->Next;
    }
  }

  SelectedObject = ParentObject;  /* Update the selected object. Done last to override any setting during recursion. */
}

/* NextAvailableNewObjectNumber:
   Returns the suffix for a new object. IE: Send it "OnyxButton" it will scan all object names. If OnyxButton1 and
   OnyxButton2 exits it would return 3. If nothing exists it returns 1. */

int NextAvailableNewObjectNumber(char *ObjectName)
{
  struct ProjectObjectList *Object;
  int NextNumber;
  int Length;
  int i;
  char *Marker;

  NextNumber = 1;
  Length = strlen(ObjectName);

  Object = ProjectObjects;
  while (Object != NULL) {
    if (!strncmp(Object->Name, ObjectName, Length)) {
      Marker = Object->Name + Length;
      i = atoi(Marker);
      if (i >= NextNumber) {
        NextNumber = i + 1;
      }
    }
    Object = Object->Next;
  }

  return NextNumber;
}

void RemoveObjectType(char *ObjectTypeName)
{
  struct ObjectList *HoldOT;
  struct ObjectList *OT;
  char temp[255];
  struct ObjectProperty *OP;
  struct ObjectProperty *HoldOP;
  struct ObjectEvent *OE;
  struct ObjectEvent *HoldOE;

  if (UsableObjects != NULL)
  {
    if (!strcmp(UsableObjects->Name, ObjectTypeName))
    {
      OT = UsableObjects;
      UsableObjects = UsableObjects->Next;

      OP = OT->Property;
      while (OP != NULL)
      {
        HoldOP = OP;
        OP = OP->Next;
        delete HoldOP;
      }
      OE = OT->Event;
      while (OE != NULL)
      {
        HoldOE = OE;
        OE = OE->Next;
        delete HoldOE;
      }

      delete OT;
    }
    else
    {
      /* Go through the whole list just to make sure I kill any duplicates. */
      /* Shouldn't happen but this is to be safe. */
      HoldOT = UsableObjects;
      OT = UsableObjects->Next; 
      while (OT != NULL)
      {
        if (!strcmp(OT->Name,ObjectTypeName))
        {
          sprintf(temp,"rm -f .onyx/%s.*",ObjectTypeName);
          system(temp);
          HoldOT->Next = OT->Next;

          OP = OT->Property;
          while (OP != NULL)
          {
            HoldOP = OP;
            OP = OP->Next;
            delete HoldOP;
          }
          OE = OT->Event;
          while (OE != NULL)
          {
            HoldOE = OE;
            OE = OE->Next;
            delete HoldOE;
          }

          delete OT;
          OT = HoldOT->Next;
        }
        else
        {
          OT = OT->Next;
          HoldOT = HoldOT->Next;
        }
      }
    }
  }
}

/* Converts a project file from Alpha stage 1 (any release) to Alpha Stage 2, Release 0 (v.0.3.0) */
void ConvertTo_0_4_0(char *ProjectFile)
{
  char temp[128];
  char temp2[255];

  sprintf(temp,".%s",ProjectFile);  /* Change from sample.oxp to .sample.oxp */
  temp[strlen(temp) - 1] = 'd';    /* Change from .sample.oxp to .sample.oxd */
  mkdir(temp,S_IRWXU);      /* Create matching .oxd directory. */
  chmod(temp,S_IRWXU); 

  printf("Converting to version 0.3.0. Please ignore errors.\n");

  sprintf(temp2,"mv * %s",temp);    /* Move everything (except .onyx) to the new directory. */
  system(temp2);

  sprintf(temp2,"mv .onyx %s",temp);  /* Move .onyx to the new directory. */
  system(temp2);

  sprintf(temp2,"mv %s/%s ./",temp,ProjectFile);  /* Move the project file back up. */
  system(temp2);
}

int ConvertOldProject(int OMajor, int OMinor, int ORelease, char *ProjectFile, char *VersionText)
{
  int Converted;

  Converted = 0;

  /* Show conversion message */
  sprintf(ConvertMsgBox->Message,"Convert project from OnyX %s?",VersionText);
  ConvertMsgBox->Show();

  /* If they agreed, then start conversion. */
  if(ConvertMsgBox->Response == YesButton)
  {
    /* The conversion will be in stages. Continue conversion until we are current. */
    while((OMajor != CompatibleMajor) || (OMinor != CompatibleMinor) || (ORelease != CompatibleRelease))
    {
      if(OMajor < 1)  /*Project created with a development release. */
      {
        if(OMinor < 3)  /* Alpha Stage 1 release (pre version number tracking) */
        {
          ConvertTo_0_4_0(ProjectFile);  /* Convert to Alpha Stage 2 */
          OMajor = 0; OMinor = 4; ORelease = 0;  /* Reset current project level. */
          Converted = 1;
        }
      }
    }

  }
  return(Converted);
}

int VersionAcceptable(int OMajor, int OMinor, int ORelease, char *ProjectFile, int *PerformedConversion)
{
  int Acceptable;
  int Age;
  char temp[255];
  char VersionText[32];

  /* Create text representation of the version found. */
  if (OMajor < 1) {
    if (OMinor < 3) {
      sprintf(VersionText, "Alpha - Stage 1");
    } else if (OMinor < 5) {
      sprintf(VersionText, "Alpha - Stage 2, Version %i.%i.%i", OMajor, OMinor, ORelease);
    } else if (OMinor < 7) {
      sprintf(VersionText, "Beta - Stage 1, Version %i.%i.%i", OMajor, OMinor, ORelease);
    } else if (OMinor < 9) {
      sprintf(VersionText, "Beta - Stage 2, Version %i.%i.%i", OMajor, OMinor, ORelease);
    } else {
      sprintf(VersionText, "Gamma, Version %i.%i.%i", OMajor, OMinor, ORelease);
    }
  } else {
    sprintf(VersionText, "Version %i.%i.%i", OMajor, OMinor, ORelease);
  }

  Age = -1; /* Assume project older than current version. */

  if (OMajor > MajorVersionNumber) {
    Age = 1;
  } else if (OMajor == MajorVersionNumber) {
    if (OMinor > MinorVersionNumber) {
      Age = 1;
    } else if (OMinor == MinorVersionNumber) {
      if (ORelease > ReleaseVersionNumber) {  /* Everything here and above is too new. */
        Age = 1;
      } else if (ORelease == ReleaseVersionNumber) {  /* Exact match! */
        Age = 0;
      } else if (ORelease >= CompatibleRelease) {  /* Not exact but close enough. */
        Age = 0;
      }
    } else if ((OMinor >= CompatibleMinor) && (ORelease >= CompatibleRelease)) {  /* Close enough. */
      Age = 0;
    }
  } else if ((OMajor >= CompatibleMajor) && (OMinor >= CompatibleMinor) && (ORelease >= CompatibleRelease)) {
    /* Still close enough. */
    Age = 0;
  }

  if (Age > 0) {  /* Project developed with a newer version of Onyx. We can't use it. */
    Acceptable = False;
    *PerformedConversion = 0;
    sprintf(temp, "This project was created with OnyX %s.\n"
                  "The current version is not new enough to use it.",
                  VersionText);
    StopBox(SplashScreen->Self(), temp);
  } else if (Age == 0) {  /* Everything is just fine. */
    Acceptable = True;
    *PerformedConversion = 0;
  } else {
    Acceptable = ConvertOldProject(OMajor, OMinor, ORelease, ProjectFile, VersionText);
    *PerformedConversion = 1;
    if (!Acceptable) {
      StopBox(SplashScreen->Self(), "Project not opened.");
    }
  }

  return Acceptable;
}


void OpenProject() {
  FILE *OxpFile;
  char InLine[100];
  char *tmp;
  char temp[255];
  extern OnyxWindow *SplashScreen;
  struct ObjectList *UsableObject;
  struct ObjectList *TmpObject;
  char *Marker;
  struct stat junk;
  int OMajor, OMinor, ORelease;
  int NoProblems;
  int ConversionPerformed;

  ProjectObjectList *CurrentObject, *ProjectObject;
  ProjectObjectPropertyList *CurrentProperty;

  struct ObjectProperty *UsableProperty;

  ResetProject();

  UsableObject = NULL; /* Local for usable objects. */
  CurrentObject = NULL;

  ValidProject = False;  /* Assume failure. */
  ConversionPerformed = 0;

  /* Open up the project file. */
  chdir(ProjectDirectory);    
  OxpFile = fopen(ProjectFile, "r");

  if (!OxpFile) {
    /* Ack. We couldn't read the file. Alert user and mark this as an invalid project. */
    ExclamationBox(SplashScreen->Self(), "Could not open project file");
  } else {
    /* Locate the version number of OnyX that this project was crated with. */
    /* This is done all by itself since it may affect how we read the rest of the oxp file. */
    OMajor = 0;
    OMinor = 0;
    ORelease = 0;
    while (fgets(InLine, 255, OxpFile) != NULL) {
      if (!strncmp(InLine, "OnyXVersion ", 12)) {
        /* If this is the onyx version that was used. */
        tmp = InLine;
        tmp = tmp + 12;
        /* Pull out the information */
        if (sscanf(tmp, "%i.%i.%i", &OMajor, &OMinor, &ORelease) != 3) {
          /* Try the old way */
          sscanf(tmp, "%i %i %i", &OMajor, &OMinor, &ORelease);
        }
        break; /* Jump out if the while loop. */
      }
    }
    fclose(OxpFile);

    /* Check the version numbers agains the current OnyX version. */
    /* If it is acceptable then go on. (Conversions may have been performed) */
    NoProblems = 1;
    if (!VersionAcceptable(OMajor, OMinor, ORelease, ProjectFile, &ConversionPerformed)) {
      NoProblems = 0;  
    } else {
      OxpFile = fopen(ProjectFile, "r");
      if (!OxpFile) {
        /* Ack. We couldn't read the file. Alert user and mark this as an invalid project. */
        ExclamationBox(SplashScreen->Self(), "Could not open project file");
        NoProblems = 0;  /* Bail out. */
      }
    }

    if (NoProblems) {
      /* Read through it. */
      while (fgets(InLine, 255, OxpFile) != NULL) {

        /* Be sure everything is null terminated. */
        InLine[strlen(InLine) -1] = '\0';
  
        if (!strncmp(InLine, "ApplicationName", 15)) { /* If this line gives the name of the application */
          tmp = InLine;
          tmp = tmp + 16;
          strcpy(ProjectApplicationName, tmp);
        } else if (!strncmp("ToolboxItem", InLine, 11)) { /* Else if this line gives toolbox information */
          tmp = InLine + 12;
          if (!ObjectTypeKnown(tmp)) {
            if (UsableObjects == NULL) {
              UsableObjects = GetNewObjectType(tmp);
              UsableObject = UsableObjects;
              UsableObject->Next = NULL;
            } else {
              UsableObject->Next = GetNewObjectType(tmp);
              UsableObject = UsableObject->Next;
              UsableObject->Next = NULL;
            }
          }
        } else if (!strncmp("StartObject", InLine, 11)) { /* If this line starts an object description */
          if (CurrentObject == NULL) { /* If this is the first object initialize list */
            ProjectObjects = new ProjectObjectList;
            CurrentObject = ProjectObjects;
          } else {  /* Otherwise append to it. */
            CurrentObject->Next = new ProjectObjectList;
            CurrentObject = CurrentObject->Next;
          }
          CurrentObject->Next = NULL;
          CurrentObject->Property = NULL;
          CurrentObject->IsIncorporated = True;
  
          CurrentProperty = NULL;
  
          /* Grab all the properties/info about the object. */
          while ((fgets(InLine, 255, OxpFile) != NULL) && (strncmp("EndObject", InLine, 9))) {
            InLine[strlen(InLine) -1] = '\0';
            if (!strncmp(InLine, "ObjectName", 10)) {
              /* If this line names the object */
              tmp = InLine;
              tmp = tmp + 11;
              strcpy(CurrentObject->Name, tmp);
            } else if (!strncmp(InLine, "ObjectType", 10)) {
              /* Else if this line says what type of object it is */
              tmp = InLine;
              tmp = tmp + 11;
              strcpy(CurrentObject->Type, tmp);
  
              /* If this is the first window found then select it to be the current selected window */
              if (SelectedWindow == NULL) {
                if (!strcmp(CurrentObject->Type, "OnyxWindow")) {
                  SelectedWindow = CurrentObject;
                  SelectedObject = CurrentObject;
                }
              }
  
              /* If we have never seen this object type then go get it from the repository */
              if (!ObjectTypeKnown(CurrentObject->Type)) {
                if (UsableObjects == NULL) {
                  UsableObjects = GetNewObjectType(CurrentObject->Type);
                  UsableObject = UsableObjects;
                  UsableObject->Next = NULL;
                } else {
                  UsableObject->Next = GetNewObjectType(CurrentObject->Type);
                  UsableObject = UsableObject->Next;
                  UsableObject->Next = NULL;
                }
                TmpObject = UsableObject;
              } else { /* Find the object type. */
                for (TmpObject = UsableObjects;
                     strcmp(TmpObject->Name, CurrentObject->Type);
                     TmpObject = TmpObject->Next) {}
              }
  
              /* Add all of the property slots needed. */
              /* This will help aid in the conversion of older objects to */
              /* updated ones when the .oob files are changed. */
              CurrentProperty = NULL;
              for (UsableProperty = TmpObject->Property;
                   UsableProperty != NULL;
                   UsableProperty = UsableProperty->Next) {
                /* Create property list. */
                if (CurrentProperty == NULL) {
                  CurrentObject->Property = new ProjectObjectPropertyList;
                  CurrentProperty = CurrentObject->Property;
                } else { /* ...or append to it. */
                  CurrentProperty->Next = new ProjectObjectPropertyList;
                  CurrentProperty = CurrentProperty->Next;
                }
                CurrentProperty->Next = NULL;
  
                /* Store property name. */
                strcpy(CurrentProperty->Name, UsableProperty->Name);
                /* Assign the default value. */
                strcpy(CurrentProperty->Value, UsableProperty->Default);
              }
            } else if (!strncmp(InLine, "ObjectParent", 12)) { /* else if this line says what the objects parent is */
              tmp = InLine;
              tmp = tmp + 13;
              strcpy(CurrentObject->Parent, tmp);
            } else { /* else this line must be a property description. */
              /* Separate the property from its value. */
              Marker = strchr(InLine, ' ');  /* Locate the space divider. */
              if (Marker) { /* Ensure that a value is present. */
                *Marker = '\0';  /* Cut it off. */
                Marker++;  /* Move to the begining of the property value. */
              }
  
              /* Locate its proper slot. */
              for (CurrentProperty = CurrentObject->Property;
                   CurrentProperty != NULL && strcmp(CurrentProperty->Name, InLine);
                   CurrentProperty = CurrentProperty->Next) {}
  
              /* If it was found put in the value. */
              if (CurrentProperty != NULL) {
                /* If the property value was not found. */
                if (Marker == NULL) {
                  /* Alert user of missing value. */
                  printf("Warning: Property %s has no value.\n", InLine);
                } else {
                  /* Store property value. */
                  strcpy(CurrentProperty->Value, Marker);
                }
              } else {  /* Alert user of extraneous property setting. */
                printf("Warning: Unknown property %s with value %s removed.\n", InLine, Marker);
              }
            }
          }
        } else if (InLine[0] == '#') {
          /* It's a comment. Just ignore it. */
        } else if (!strncmp(InLine, "OnyXVersion ", 12)) {
          /* Version number. Ignore it at this point. */
        } else { /* We have some weird line. Pitch it out. */
          printf("No match on %s\n", InLine);
        }
      }

      /* Close the project file. */
      fclose(OxpFile);

      sprintf(temp, ".%s", ProjectFile);
      if (strlen(temp) >= 4) {
        Marker = temp + strlen(temp) - 4;       /* Change to .oxd */
        if (!strcmp(Marker, ".oxp")) {
          Marker = Marker + 3;
          *Marker = 'd';
        }
      }
      chdir(temp);  /* Enter the true directory of the project code. */

      // Initialize out internal representation of the file we just read.
      // This is done separately here so that all of the invalid lines
      // will have already been weeded out.

      InitializeInternalProjectFile(ProjectFile);
  
      /* If a conversion was performed then save immediately to write out new version number in oxp file. */
      if(ConversionPerformed) {
        InternalOxpCode->Save();
      }

      /* Read in all of the oxc files belonging to the objects. */
      ProjectObject = ProjectObjects; 
  
      while (ProjectObject != NULL) {
        InternalOxcCode = new InternalCodeFile(ProjectObject->Name, NULL, InternalOxcCode);
        InternalOxcCode->Open();
        InternalOxcCode->IsIncorporated = True;
        ProjectObject = ProjectObject->Next;
      }
  
      /* Lets see if an Application.oxp exists. If it does open it up. Otherwise recreate it. */
      if (stat("Application.oxp", &junk)) {
        InternalOxcCode = new InternalCodeFile("Application", NULL, InternalOxcCode);
        InternalOxcCode->Open();
        InternalOxcCode->IsIncorporated = True;
      } else {
        InitializeInternalApplication();
      }      
  
      InitializeInternalMain_h();
      InitializeInternalEvents_h();
      InitializeInternalObjects_h();
      InitializeInternalIncludes_h();
      InitializeInternalMakefile();
      InitializeInternalBaseMakefile();
  
      /* The user can modify the project now. It is a valid one. */
      ValidProject = True;
    }
  }

  if (ValidProject) {
    /* Modify name of main window to reflect project that is open. */
    sprintf(MainWindow->Name, "OnyX - %s", ProjectFile);
    tmp = MainWindow->Name + strlen(MainWindow->Name) - 4;
    if (!strcmp(tmp, ".oxp")) {
      *tmp = '\0';
    }
    MainWindow->Update();
  } else {
    strcpy(MainWindow->Name, "OnyX");
    MainWindow->Update();
  }

  // Set the menu and button access based upon whether or not we have a
  // valid project to work with.

  SetMainWindowButtonStatus();
  SetMainWindowMenuStatus();
}


/* Free all project structures. Called before opening a new project */

void ResetProject() {

  /* Remove all internal code */
  while (InternalOxcCode != NULL) {  
    InternalOxcCode->Close();
    if (InternalOxcCode->Next == NULL) {
      delete InternalOxcCode;
      InternalOxcCode = NULL;
    } else {
      InternalOxcCode = InternalOxcCode->Next;
      delete InternalOxcCode->Previous;
    }
  }

  while (InternalHeader != NULL) {  
    InternalHeader->Close();
    if (InternalHeader->Next == NULL) {
      delete InternalHeader;
      InternalHeader = NULL;
    } else {
      InternalHeader = InternalHeader->Next;
      delete InternalHeader->Previous;
    }
  }

  while (InternalMaincc != NULL) {  
    InternalMaincc->Close();
    if (InternalMaincc->Next == NULL) {
      delete InternalMaincc;
      InternalMaincc = NULL;
    } else {
      InternalMaincc = InternalMaincc->Next;
      delete InternalMaincc->Previous;
    }
  }

  while (InternalMakefile != NULL) {  
    InternalMakefile->Close();
    if (InternalMakefile->Next == NULL) {
      delete InternalMakefile;
      InternalMakefile = NULL;
    } else {
      InternalMakefile = InternalMakefile->Next;
      delete InternalMakefile->Previous;
    }
  }

  while (InternalBaseMakefile != NULL) {  
    InternalBaseMakefile->Close();
    if (InternalBaseMakefile->Next == NULL) {
      delete InternalBaseMakefile;
      InternalBaseMakefile = NULL;
    } else {
      InternalBaseMakefile = InternalBaseMakefile->Next;
      delete InternalBaseMakefile->Previous;
    }
  }

  while (InternalOxpCode != NULL) {  
    InternalOxpCode->Close();
    if (InternalOxpCode->Next == NULL) {
      delete InternalOxpCode;
      InternalOxpCode = NULL;
    } else {
      InternalOxpCode = InternalOxpCode->Next;
      delete InternalOxpCode->Previous;
    }
  }

  /* Reset project */
  while (UsableObjects != NULL) {
    while (UsableObjects->Property != NULL) {
      ObjectProperty *tmp = UsableObjects->Property;
      UsableObjects->Property = UsableObjects->Property->Next;
      delete tmp;
    }
    while (UsableObjects->Event != NULL) {
      ObjectEvent *tmp = UsableObjects->Event;
      UsableObjects->Event = UsableObjects->Event->Next;
      delete tmp;
    }
    ObjectList *tmp = UsableObjects;
    UsableObjects = UsableObjects->Next;
    delete tmp;
  }
  UsableObjects = NULL;

  while (ProjectObjects != NULL) {
    while (ProjectObjects->Property != NULL) {
      ProjectObjectPropertyList *tmp = ProjectObjects->Property;
      ProjectObjects->Property = ProjectObjects->Property->Next;
      delete tmp;
    }
    ProjectObjectList *tmp = ProjectObjects;
    ProjectObjects = ProjectObjects->Next;
    delete tmp;
  }
  ProjectObjects = NULL;

  ValidProject = False;
}

int WriteOnyxrcFile() {
  FILE *onyxrc;
  char rcfilename[255];
  char *Marker;
  int rcode;

  Marker = getenv("HOME");

  rcode = 0;
  if (!Marker) {
    rcode = -1;
  } else {
    sprintf(rcfilename, "%s/.onyxrc", Marker);
    onyxrc = fopen(rcfilename, "w");
    if (onyxrc) {
      fprintf(onyxrc, "OnyxLocation %s\n", OnyxLocation);
      fprintf(onyxrc, "EditorCommand %s\n", EditorCommand);
      fprintf(onyxrc, "SnapToGridByDefault %i\n", SnapToGridIsDefault);
      fclose(onyxrc);
    } else {
      rcode = -2;
    }
  }

  return rcode;
}
