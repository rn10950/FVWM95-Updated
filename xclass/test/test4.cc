/**************************************************************************

    This is a test program for xclass.
    Copyright (C) 1997-2000, Hector Peraza.

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

#include <xclass/OXClient.h>
#include <xclass/OXMainFrame.h>

main() {
  OXClient *clientX = new OXClient();

  OXMainFrame *main1 = new OXMainFrame(clientX->GetRoot(), 100, 100);
  main1->MapWindow();
  main1->SetWindowName("1");

  OXMainFrame *main2 = new OXMainFrame(clientX->GetRoot(), 100, 100);
  main2->MapWindow();
  main2->SetWindowName("2");

  clientX->Run();
  printf("OXClient returned (1,2)\n");

  OXMainFrame *main3 = new OXMainFrame(clientX->GetRoot(), 100, 100);
  main3->MapWindow();
  main3->SetWindowName("3");

  clientX->Run();
  printf("OXClient returned (3)\n");

  OXMainFrame *main4 = new OXMainFrame(clientX->GetRoot(), 100, 100);
  main4->MapWindow();
  main4->SetWindowName("4");

  clientX->Run();
  printf("OXClient returned (4)\n");

  return 0;
}
