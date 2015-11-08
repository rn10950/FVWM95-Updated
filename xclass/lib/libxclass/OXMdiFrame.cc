/**************************************************************************
 
    This file is part of OXMdi an extension to the xclass toolkit.
    Copyright (C) 1998-2002 by Harald Radke, Hector Peraza.
 
    This application is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
 
    This application is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.
 
    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 
**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include <xclass/OXMdiFrame.h>
#include <xclass/OXMdiMainFrame.h>
#include <xclass/OXMdiDecorFrame.h>


//----------------------------------------------------------------------

OXMdiFrame::OXMdiFrame(OXMdiMainFrame *main, int w, int h,
		       unsigned int options, unsigned long back) :
  OXCompositeFrame(main->GetContainer(), w, h,
                   options | OWN_BKGND | MDI_FRAME, back) {

  _main = main;
  _main->AddMdiFrame(this);  // this reparents the window
}

OXMdiFrame::~OXMdiFrame() {
  _main->RemoveMdiFrame(this);
}

int OXMdiFrame::CloseWindow() {
  delete this;
  return True;
}

void OXMdiFrame::SetMdiHints(unsigned long mdihints) {
  _mdiHints = mdihints;
  ((OXMdiDecorFrame *)_parent)->SetMdiButtons(mdihints);
}

void OXMdiFrame::SetWindowName(const char *name) {
  ((OXMdiDecorFrame *)_parent)->SetWindowName(name);
  _main->UpdateWinListMenu();
}

void OXMdiFrame::SetWindowIcon(const OPicture *pic) {
  ((OXMdiDecorFrame *)_parent)->SetWindowIcon(pic);
  _main->UpdateWinListMenu();
}

const char *OXMdiFrame::GetWindowName() {
  return ((OXMdiDecorFrame *)_parent)->GetWindowName();
}

const OPicture *OXMdiFrame::GetWindowIcon() {
  return ((OXMdiDecorFrame *)_parent)->GetWindowIcon();
}

void OXMdiFrame::Move(int x, int y) {
  ((OXMdiDecorFrame *)_parent)->Move(x, y);
}
