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

#include "OnyxFileDialog.h"

extern MessageCenter *CentralMessageCenter;


//----------------------------------------------------------------------

OnyxFileDialog::OnyxFileDialog() : OnyxObject("OnyxFileDialog") {
  TotalFileTypes = 0;
}

void OnyxFileDialog::Show() {
  int i;

  // our pointer to the array of pointers

  // allocate the array of pointers: we need 2 pointers per file type plus 2
  // NULL pointers for the end of list

  FileTypes = new (char *)[(TotalFileTypes + 1) * 2];

  // then allocate and initialize the strings:
  for (i = 0; i < TotalFileTypes; ++i) {

    // allocate space for strings
    FileTypes[i*2]   = new char[strlen(FileDescription[i]) + 1];
    FileTypes[i*2+1] = new char[strlen(FilePattern[i]) + 1];

    strcpy(FileTypes[i*2], FileDescription[i]);
    strcpy(FileTypes[i*2+1], FilePattern[i]);
  }

  // the two last elements are just NULL
  FileTypes[i*2] = FileTypes[i*2+1] = NULL;

  fi = new OFileInfo;

  fi->file_types = FileTypes;
  new OXFileDialog(DisplayWindow, Parent, DialogType, fi);

  if (fi->filename) {
    strcpy(FileSelected, fi->filename);
    getcwd(DirectorySelected, 254);      // ==!==
    Cancelled = 0;
  } else {
    Cancelled = 1;
    strcpy(FileSelected, "");
    strcpy(DirectorySelected, "");
  }

  // free all allocated memory...
  delete fi;

  for (i = 0; i < TotalFileTypes; ++i) {
    delete[] FileTypes[i*2];
    delete[] FileTypes[i*2+1];
  }

  delete[] FileTypes;
}

int OnyxFileDialog::Create() {
  OnyxObject::Create();
  ClearFileTypes();
  AddFileType("All Files", "*");
  return True;
}

int OnyxFileDialog::AddFileType(char *Description, char *Pattern) {
  if (TotalFileTypes < OnyxMaxFileTypes) {
    strcpy(FileDescription[TotalFileTypes], Description);
    strcpy(FilePattern[TotalFileTypes], Pattern);
    TotalFileTypes++;
    return 0;
  } else {
    return 1;
  }
}

void OnyxFileDialog::ClearFileTypes() {
  TotalFileTypes = 0;
}

OnyxFileDialog::~OnyxFileDialog() {
}
