/**************************************************************************

    This file is part of explorer95, a file manager for fvwm95.
    Copyright (C) 1996, 1997 David Barth, Hector Peraza.

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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <xclass/utils.h>
#include <xclass/version.h>
#include <xclass/OMimeTypes.h>
#include <xclass/OXClient.h>
#include <xclass/OResourcePool.h>

#include "OXExplorer.h"


OXClient *clientX;
OXExplorer *mainWindow;

OMimeTypes *MimeTypeList;

char *AppName, *AppPath;

//-------------------------------------------------------------------

int main(int argc, char *argv[]) {
  char *p, *startDir, *homeDir;
  int  MainMode;

  MatchXclassVersion(XCLASS_VERSION, XCLASS_RELEASE_DATE);

  AppPath = argv[0];

  if ((p = strrchr(AppPath, '/')) == NULL)
    AppName = AppPath;
  else
    AppName = ++p;

  if (argc > 1) {
    startDir = argv[1];
    MainMode = FILE_MGR_MODE;
  } else {
    startDir = ".";
    MainMode = EXPLORER_MODE;
  }

  homeDir = getenv("HOME");

  if (!homeDir) homeDir = "/";

  clientX = new OXClient(argc, argv);

  //--- read rc types...

  MimeTypeList = clientX->GetResourcePool()->GetMimeTypes();

  mainWindow = new OXExplorer(clientX->GetRoot(), startDir, MainMode);

  clientX->Run();

  return 0;
}
