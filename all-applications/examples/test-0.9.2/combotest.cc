/**************************************************************************

    This is a program intended for testing xclass combo boxes.
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
#include <xclass/OXMainFrame.h>
#include <xclass/OString.h>
#include <xclass/OXComboBox.h>

main() {

  OXClient *ClientX = new OXClient();

  OXMainFrame *m = new OXMainFrame(ClientX->GetRoot(), 300, 100);

  OXComboBox *cb = new OXComboBox(m, "test", 100);

  m->AddFrame(cb, new OLayoutHints(LHINTS_CENTER_X | LHINTS_CENTER_Y));
  cb->AddEntry(new OString("Testing 1"), 1);
  cb->AddEntry(new OString("Testing 2"), 2);
  cb->AddEntry(new OString("Testing 3"), 3);
  cb->AddEntry(new OString("Testing 4"), 4);
  cb->AddEntry(new OString("Testing 5"), 5);
  cb->AddEntry(new OString("Testing 6"), 6);
  cb->AddEntry(new OString("Testing 7"), 7);
  cb->AddEntry(new OString("Testing 8"), 8);

  m->MapSubwindows();
  m->MapWindow();
  m->Layout();

  ClientX->Run();
}
