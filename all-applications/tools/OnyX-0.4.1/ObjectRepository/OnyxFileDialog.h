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

#ifndef __ONYXFILEDIALOG_H
#define __ONYXFILEDIALOG_H

#include <xclass/OXFileDialog.h>
#include <xclass/OMimeTypes.h>

#include "OnyxBase.h"


#define OnyxOpenFileDialog FDLG_OPEN;
#define OnyxSaveFileDialog FDLG_SAVE;

#define OnyxMaxFileTypes 10


//----------------------------------------------------------------------

class OnyxFileDialog : public OnyxObject {
public:
  OnyxFileDialog();
  virtual ~OnyxFileDialog();

  virtual int Create();
  virtual void Show();
  virtual int AddFileType(char *Description, char *Pattern);
#if 0
  virtual int RemoveFileType(char *Description, char *Pattern);
  virtual int RemoveFileType(char *Description);
  virtual int RemoveFileType(char *Pattern);
#endif
  virtual void ClearFileTypes();

  const OXWindow *DisplayWindow;

  int DialogType;

  char DirectorySelected[255];
  char FileSelected[255];
  int Cancelled;

protected:
  int TotalFileTypes;
  OFileInfo *fi;
  char **FileTypes;

  char FileDescription[OnyxMaxFileTypes][100];
  char FilePattern[OnyxMaxFileTypes][15];
};


#endif  // __ONYXFILEDIALOG_H
