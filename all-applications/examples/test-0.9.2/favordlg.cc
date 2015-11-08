/**************************************************************************

    This program is used to test the File Open dialog.
    Copyright (C) 1997-2001 Mike McDonald, Hector Peraza.

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

#include <xclass/OXClient.h>
#include <xclass/OXFileDialog.h>

char *filetypes[] = {
  "C/C++ Files",      "*.c|*.cc|*.h",
  "JavaScript Files", "*.js",
  "All files",        "*",
  0,		      0
};

int main() {
  OFileInfo fi;
  OXClient *client;

  client = new OXClient();
  
  fi.file_types = filetypes;
 
  new OXFileDialog(client->GetRoot(), 0, FDLG_OPEN | FDLG_FAVOURITES, &fi);

  if (fi.filename)
    printf("File Selected is %s/%s\n", fi.ini_dir, fi.filename);

  new OXFileDialog(client->GetRoot(), 0, FDLG_OPEN, &fi);

  if (fi.filename)
    printf("File Selected is %s/%s\n", fi.ini_dir, fi.filename);
}
