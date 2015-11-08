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

#ifndef __GLOBALEXTERN_H
#define __GLOBALEXTERN_H

#include <sys/stat.h>
#include <unistd.h>

#define MajorVersionNumber    0
#define MinorVersionNumber    4
#define ReleaseVersionNumber  1

#define CompatibleMajor       0
#define CompatibleMinor       3
#define CompatibleRelease     0

#define XClassDesignVersionNumber "0.9.2"

#define IDModifier                 10000
#define PropertyIDModifier          4000
#define PropertyIDMax                100
#define PropertyButtonIDModifier    4200
#define PropertyButtonIDMax          100
#define ToolboxButtonIDModifier     6000
#define ToolboxButtonIDMax           100
#define MaxDevelopmentWindows         50
#define MaxDevelopmentObjects        999
#define DevelopmentWindowIDModifier 7000
#define DevelopmentItemIDModifier   8000
#define DevelopmentItemIDMax         999 // Warning: in CreateDevelopmentObjects
                                         // (MainWindow.cc) this and MaxDevelopmentObjects
                                         // seem to get linked. Eliminate one?
#define CCIDModifier               20000
#define CCIDMax                       50

enum PropertyEntryType  {
  TextBox,
  ComboBox,
  ListBox,
  ColorBrowser,
  FontBrowser,
  FileBrowser
};

enum PropertyAccessType {
  ReadOnly,
  ReadWrite,
  WriteOnly
};

struct ObjectProperty {
  struct ObjectProperty *Next;

  char Name[50];
  char Assignment[50];
  char Default[50];

  PropertyAccessType Access;  // ReadOnly, ReadWrite, or WriteOnly
  PropertyEntryType Type;     // TextBox, ComboBox, ListBox, ColorBrowser, FontBrowser, FileBrowser
  int Entries;                // Entries available. (0-10)
  char Entry[10][255];
};

struct ObjectEvent {
  struct ObjectEvent *Next;

  char Name[50];
  char Parm1[20];
  char Parm2[20];
  char Parm3[20];
  char Parm4[20];
};

struct ObjectList {
  struct ObjectList *Next;
  struct ObjectProperty *Property;
  struct ObjectEvent *Event;

  char Name[100];
  char ToolTip[100];
  int ToolboxViewable;
};

struct ProjectObjectPropertyList {
  struct ProjectObjectPropertyList *Next;

  char Name[100];
  char Value[100];
  char NewValue[100];
};

struct ProjectObjectList {
  struct ProjectObjectList *Next;
  struct ProjectObjectPropertyList *Property;

  char Name[100];
  char Type[100];
  char Parent[255];
  int IsIncorporated;
};

class InternalCodeFile {
public:
  InternalCodeFile(char *ObjectNam, InternalCodeFile *Prev, InternalCodeFile *Nex);
  virtual ~InternalCodeFile();

  virtual void Open();
  virtual void Append(const char *fmt, ...);
  virtual void AppendString(char *Data, long Size);
  virtual void AppendToFile(char *NewFileName);
  virtual void Replace(char *Original, long OriginalSize, char *New, long NewSize);
  virtual void WriteForEdit();
  virtual void Edit(char *EditorCommand);
  virtual void Check();
  virtual void Save();
  virtual void Close();

  char ObjectName[255];
  char OldObjectName[255];
  int HasBeenModified;
  int HasBeenRenamed;
  int HasBeenDeleted;
  int IsIncorporated;
  int CodeStyle;

  InternalCodeFile *Next;
  InternalCodeFile *Previous;

  int EditorPID;
  int Breaker;

private:
  char *Buffer;
  struct stat FileStats;
};


extern int TotalProjectWindows;
extern int NextProjectID;

extern int NextDevelopmentItemID;

extern int RunPID;
extern char ProjectFile[50];
extern char ProjectDirectory[255];
extern char ProjectApplicationName[50];
extern char OnyxLocation[255];
extern char EditorCommand[255];
extern char OnyxIconLocation[255];

extern struct ObjectList *UsableObjects;
extern struct ProjectObjectList *ProjectObjects;
extern struct ProjectObjectList *DeletedObjects;
extern struct ProjectObjectList *SelectedObject;
extern struct ProjectObjectList *SelectedWindow;
extern struct ProjectObjectPropertyList *SelectedProperty;
extern struct ProjectObjectList *DevelopmentItem[DevelopmentItemIDMax];

extern OnyxWindow *DevelopmentWindow[MaxDevelopmentWindows];
extern struct ProjectObjectList *WindowToListMap[MaxDevelopmentWindows];

extern OnyxObject *DevelopmentObject[MaxDevelopmentObjects];

extern OnyxLabel *PropertyItem[50];
extern OnyxTextBox *PropertyValue[50];
extern OnyxComboBox *PropertyCombo[50];
extern OnyxButton *PropertyButton[50];
extern int SelectedValue;

extern int ObjectTypeKnown(char *ObjectTypeName);
extern ObjectList *GetNewObjectType(char *ObjectTypeName);


#endif  // __GLOBALEXTERN_H
