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

#include "OnyxBase.h"

OXClient *ClientX;

MessageCenter *CentralMessageCenter;

extern int OnyxProcessMessage(OnyxMessage *msg);


//----------------------------------------------------------------------

OnyxObject::OnyxObject(char *ObjectType) {
  strcpy(OType, ObjectType);
  X = 0;
  Y = 0;
  Width = 32;
  Height = 32;
  Size = 255;
  BorderStyle = NoBorder;
  VerticalAlignment = AlignCenter;
  HorizontalAlignment = AlignCenter;
  IsEnabled = True;
  IsChecked = False;
  IsShaped = False;
  EnableToolTip = False;
  Parent = NULL;
  strcpy(Text, "");
  strcpy(Picture, "");
  strcpy(ToolTip, "");
  strcpy(Font, "Helvetica -12");
}

OnyxObject::~OnyxObject() {
}

int OnyxObject::Create() {
  return True;
}

void OnyxObject::Move(int x, int y) {
  X = x;
  Y = y;
}

void OnyxObject::Resize(int width, int height) {
  Width = width;
  Height = height;
}

void OnyxObject::MoveResize(int x, int y, int width, int height) {
  X = x;
  Y = y;
  Width = width;
  Height = height;
}

void OnyxObject::Update() {
}

void OnyxObject::Enable() {
  IsEnabled = True;
}

void OnyxObject::Disable() {
  IsEnabled = False;
}

//----------------------------------------------------------------------

int MessageCenter::ProcessMessage(OMessage *msg) {
  if (msg->type == MSG_ONYX)
    return OnyxProcessMessage((OnyxMessage *) msg);
  else
    return OXMainFrame::ProcessMessage(msg);
}


//----------------------------------------------------------------------

void Initialize(char *ApplicationName, int argc, char *argv[]) {
  ClientX = new OXClient(argc, argv);
  CentralMessageCenter = new MessageCenter(ClientX->GetRoot());
  strcpy(CentralMessageCenter->ApplicationName, ApplicationName);
}

const OXWindow *ApplicationRoot() {
  return ClientX->GetRoot();
}

const OXWindow *Application() {
  return ClientX->GetRoot();
}

OXClient *ApplicationClient() {
  return ClientX;
}

OPicturePool *ApplicationPicturePool() {
  return ClientX->GetResourcePool()->GetPicturePool();
}

void ApplicationRun() {
  ClientX->Run();
}

void ApplicationClose() {
  CentralMessageCenter->CloseWindow();
  exit(0);
}
