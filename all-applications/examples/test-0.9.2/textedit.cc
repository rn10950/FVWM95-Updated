/**************************************************************************

    This is a test program for the OXTextEdit widget.
    Copyright (C) 2000, Hector Peraza.

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
#include <string.h>

#include <xclass/utils.h>
#include <xclass/OXMsgBox.h>
#include <xclass/OXMainFrame.h>
#include <xclass/OXTextEdit.h>

#include "bgnd.xpm"


//----------------------------------------------------------------------

class OXTestFrame : public OXMainFrame {
public:
  OXTestFrame(const OXWindow *p, char *fname);

protected:
  OXTextEdit *textview;
};

//----------------------------------------------------------------------

OXClient *clientX;

int main(int argc, char **argv) {

  clientX = new OXClient;

  char *fname = "textedit.cc";
  if (argc > 1) fname = argv[1];

  OXTestFrame *mainWindow = new OXTestFrame(clientX->GetRoot(), fname);

  mainWindow->MapSubwindows();
  mainWindow->Resize(560, 400);
  mainWindow->Layout();
  mainWindow->MapWindow();

  clientX->Run();
}

OXTestFrame::OXTestFrame(const OXWindow *p, char *fname) :
  OXMainFrame(p, 100, 100) {

  OLayoutHints *layout = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y,
                                          0, 0, 1, 0);

  textview = new OXTextEdit(this, 10, 10, 2);
  textview->SetupBackgroundPic(_client->GetPicture("bgnd.xpm", bgnd_xpm));

  textview->LoadFromFile(fname);

  AddFrame(textview, layout);
}
