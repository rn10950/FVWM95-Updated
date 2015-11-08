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

// Note to self, continue developing along the line of "Active events."
// Perhaps expand this to other objects.

#include <X11/X.h>

#include <xclass/OFocusManager.h>

#include "OnyxWindow.h"

extern MessageCenter *CentralMessageCenter;


//----------------------------------------------------------------------

RedirectedMainFrame::RedirectedMainFrame(const OXWindow *p,
    int *x, int *y, int *w, int *h, int ID,
    int *CloseExitsApplication, int *AbortClose, 
    int *WindowExists, int *CloseDestroysWindow, int *IsVisible,
    long ActiveEvents, unsigned long options) :
  OXMainFrame(p, *w, *h, options) {

  InternalID = ID;
  Width = w;
  Height = h;
  X = x;
  Y = y;
  ApplicationShouldClose = CloseExitsApplication;
  WindowShouldBeDestroyed = CloseDestroysWindow;
  ShouldAbortClose = AbortClose;
  CurrentVisibility = IsVisible;
  WinExists = WindowExists;
  *WinExists = *WinExists + 1;

  AddInput(ActiveEvents);

  SetLayoutManager(NULL);

#if 1    // until frames get added() to parents
  if (_focusMgr) delete _focusMgr;
  _focusMgr = NULL;
#endif
}

RedirectedMainFrame::~RedirectedMainFrame() {
}

void RedirectedMainFrame::_Moved() {
  OnyxMessage omsg;

  omsg.onyx_type = ContainerType;
  omsg.id = InternalID;

  OXMainFrame::_Moved();

  if ((*X != _x || *Y != _y)) {
    omsg.action = WindowMoved;
    omsg.old_x = *X;
    omsg.old_y = *Y;
    *X = _x;
    *Y = _y;
    omsg.x = _x;
    omsg.y = _y;
    CentralMessageCenter->ProcessMessage(&omsg);
  }
}

void RedirectedMainFrame::_Resized() {
  OnyxMessage omsg;

  omsg.onyx_type = ContainerType;
  omsg.id = InternalID;

  OXMainFrame::_Resized();

  if (*Width != _w || *Height != _h) {
    omsg.action = WindowResized;
    omsg.old_width = *Width;
    omsg.old_height = *Height;
    *Width = _w;
    *Height = _h;
    omsg.width = _w;
    omsg.height = _h;
    CentralMessageCenter->ProcessMessage(&omsg);
  }
}

int RedirectedMainFrame::CloseWindow() {
  int retc = False;
  OnyxMessage omsg;

  omsg.onyx_type = WindowType;
  omsg.id = InternalID;
  omsg.action = WindowCloseSignaled;

  CentralMessageCenter->ProcessMessage(&omsg);

  if (!(*ShouldAbortClose)) {
    *CurrentVisibility = 0;
    if (*WindowShouldBeDestroyed) {
      *WinExists = *WinExists - 1;
      ForceCloseWindow();
      retc = True;
    } else {
      UnmapWindow();
    }

    omsg.action = WindowClosed;
    CentralMessageCenter->ProcessMessage(&omsg);

    if (*ApplicationShouldClose) {
      ApplicationClose();
    }
  } else {
    *ShouldAbortClose = False;
  }

  return retc;
}

int RedirectedMainFrame::ForceCloseWindow() {
  OnyxMessage omsg;

  omsg.onyx_type = WindowType;
  omsg.id = InternalID;
  omsg.action = WindowClosed;

  CentralMessageCenter->ProcessMessage(&omsg);

  OXMainFrame::CloseWindow(); ////DestroyWindow();

  return True;
}

int RedirectedMainFrame::HandleKey(XKeyEvent *event) {
  OnyxMessage omsg;

  omsg.onyx_type = ContainerType;
  omsg.id = InternalID;

  omsg.keysym = XKeycodeToKeysym(GetDisplay(), event->keycode, 0);

  if (event->type == KeyPress) {
    omsg.action = KeyPressed;
    CentralMessageCenter->ProcessMessage(&omsg);
  } else {
    omsg.action = KeyReleased;
    CentralMessageCenter->ProcessMessage(&omsg);
  }
  return True;
}

int RedirectedMainFrame::HandleButton(XButtonEvent *event) {
  OnyxMessage omsg;

  omsg.onyx_type = ContainerType;
  omsg.id = InternalID;

  omsg.button = event->button;
  omsg.x = event->x;
  omsg.y = event->y;
  omsg.x_root = event->x_root;
  omsg.y_root = event->y_root;

  OXMainFrame::HandleButton(event);

  if (event->type == ButtonPress) {
    omsg.action = ButtonPressed;
    CentralMessageCenter->ProcessMessage(&omsg);
  } else {
    omsg.action = ButtonReleased;
    CentralMessageCenter->ProcessMessage(&omsg);
  }
  return True;
}

int RedirectedMainFrame::HandleMotion(XMotionEvent *event) {
  OnyxMessage omsg;

  OXMainFrame::HandleMotion(event);

  omsg.onyx_type = ContainerType;
  omsg.id = InternalID;
  omsg.action = PointerMoved;
  omsg.x = event->x;
  omsg.y = event->y;
  omsg.x_root = event->x_root;
  omsg.y_root = event->y_root;

  return CentralMessageCenter->ProcessMessage(&omsg);
}


//----------------------------------------------------------------------

OnyxWindow::OnyxWindow() : OnyxObject("OnyxWindow") {
  WindowExists = False;
  Width = 100;
  Height = 100;
  X = 0;
  Y = 0;

  ID = 0;
  Parent = ApplicationRoot();

  AbortClose = False;
  CloseExitsApplication = False;
  IsVisible = False;
  CloseDestroysWindow = False;

  MinimumWidth = 0;
  MinimumHeight = 0;

  MaximumWidth = 32767;
  MaximumHeight = 32767;

  ResizeWidthIncrement = 0;
  ResizeHeightIncrement = 0;

  Name[0] = '\0';
  Class[0] = '\0';
  Resource[0] = '\0';

  Picture[0] = '\0';
  Directory[0] = '\0';
  strcpy(BackgroundColor, "#c0c0c0");

  ButtonPressActive = True;
  ButtonReleaseActive = True;
  PointerMotionActive = True;
  KeyPressActive = True;
  KeyReleaseActive = True;
  ExposureActive = True;
  StructureNotifyActive = True;

  HasBorder = True;
  HasTitleBar = True;
  HasMenu = True;
  HasMinimizeButton = True;
  HasMaximizeButton = True;

  HasMinimizeOption = True;
  HasMaximizeOption = True;
  HasResizeOption = True;
  HasCloseOption = True;
  HasMoveOption = True;
}

int OnyxWindow::Create() {
  unsigned int Color;

  OnyxObject::Create();

  if (!WindowExists) {
    // Form event mask to be used by window.
    ActiveEvents = ButtonPressActive * ButtonPressMask |
                   ButtonReleaseActive * ButtonReleaseMask |
                   PointerMotionActive * PointerMotionMask |
                   KeyPressActive * KeyPressMask | 
                   KeyReleaseActive * KeyReleaseMask |
                   ExposureActive * ExposureMask | 
                   StructureNotifyActive * StructureNotifyMask;

    InternalWindow = new RedirectedMainFrame(Parent, &X, &Y, &Width, &Height,
                                             ID, &CloseExitsApplication,
                                             &AbortClose, &WindowExists,
                                             &CloseDestroysWindow, &IsVisible,
                                             ActiveEvents,
                                             MAIN_FRAME | HORIZONTAL_FRAME);

    Color = ApplicationClient()->GetColorByName(BackgroundColor);
    InternalWindow->SetBackgroundColor(Color);

    // Add in background pixmap if given.
    if (strlen(Directory)) {
      ApplicationPicturePool()->AddPath(Directory);
    }
    if (strlen(Picture)) {
      InternalPicture = ApplicationPicturePool()->GetPicture(Picture);
      InternalWindow->SetBackgroundPixmap(InternalPicture->GetPicture());
    }
  }

  InternalWindow->SetWindowName(Name);
  if (strlen(Class) == 0) strcpy(Class, Name);
  if (strlen(Resource) == 0) strcpy(Resource, Name);
  InternalWindow->SetClassHints(Class, Resource);

  IsVisible = False;

  // New stuff being tested out.
  InternalWindow->SetIconName(Name);
  InternalWindow->Move(X, Y);
  InternalWindow->SetWMPosition(X, Y);
  InternalWindow->SetWMSize(Width, Height);
  InternalWindow->SetWMSizeHints(MinimumWidth, MinimumHeight,
                                 MaximumWidth, MaximumHeight,
                                 ResizeWidthIncrement, ResizeHeightIncrement);

  InternalWindow->SetMWMHints(
    MWM_DECOR_ALL |                     // All decorations on except those explicately turned off.
    MWM_DECOR_BORDER * !HasBorder |     // Client Window Border
    MWM_DECOR_RESIZEH * 0 |             // Resize Frame Handles
    MWM_DECOR_TITLE * !HasTitleBar |    // Title Bar
    MWM_DECOR_MENU * !HasMenu |         // Window Menu Button
    MWM_DECOR_MINIMIZE * !HasMinimizeButton |  // Minimize Window Button
    MWM_DECOR_MAXIMIZE * !HasMaximizeButton,   // Maximize Window Button

    MWM_FUNC_ALL |                         // All functiuons except those explicately turned off
    MWM_FUNC_RESIZE * !HasResizeOption |   // Resize capability
    MWM_FUNC_MOVE * !HasMoveOption |       // Move capability
    MWM_FUNC_MINIMIZE * !HasMinimizeOption |   // Minimize capability
    MWM_FUNC_MAXIMIZE * !HasMaximizeOption |   // Maximize capability
    MWM_FUNC_CLOSE * !HasCloseOption,          // Accepts close window

    MWM_INPUT_MODELESS);

/*************************************************************************************************************************/
/* For future refrence                                                                                                   */
/*            MWM_INPUT_MODELESS - Input goes to any window                                                              */
/*            MWM_INPUT_PRIMARY_APPLICATION_MODAL - Input does not go to ancestors of this window                        */
/*            MWM_INPUT_SYSTEM_MODAL - Input goes only to this window                                                    */
/*            MWM_INPUT_FULL_APPLICATION_MODAL - Input does not go to other windows in this application                  */
/*************************************************************************************************************************/

  return WindowExists;
}

const OXMainFrame *OnyxWindow::Self() {
  return InternalWindow;
}

void OnyxWindow::Move(int newX, int newY) {
  //OnyxObject::Move(newX, newY);
  InternalWindow->Move(newX, newY);
}

void OnyxWindow::Resize(int newWidth, int newHeight) {
  //OnyxObject::Resize(newWidth, newHeight);
  InternalWindow->Resize(newWidth, newHeight);
}

void OnyxWindow::MoveResize(int newX, int newY, int newWidth, int newHeight) {
  //OnyxObject::MoveResize(newX, newY, newWidth, newHeight);
  InternalWindow->MoveResize(newX, newY, newWidth, newHeight);
}

void OnyxWindow::Update() {
  OnyxObject::Update();
  InternalWindow->SetWindowName(Name);
  InternalWindow->SetClassHints(Class, Resource);
  InternalWindow->SetIconName(Name);
  InternalWindow->MapSubwindows();
//  InternalWindow->Layout();
//  InternalWindow->MapWindow();
  InternalWindow->SetWMPosition(X, Y);
  InternalWindow->Move(X, Y);
  InternalWindow->SetWMSizeHints(MinimumWidth, MinimumHeight,
                                 MaximumWidth, MaximumHeight,
                                 ResizeWidthIncrement, ResizeHeightIncrement);
  ApplicationClient()->NeedRedraw(InternalWindow);
}

void OnyxWindow::Show() {

  if (!WindowExists) Create();

  OnyxMessage omsg;
  omsg.onyx_type = WindowType;
  omsg.action = WindowLoad;
  omsg.id = ID;
  CentralMessageCenter->ProcessMessage(&omsg);

  InternalWindow->MapSubwindows();
  InternalWindow->MapWindow();

  IsVisible = True;
}

void OnyxWindow::Hide() {
  InternalWindow->UnmapWindow();
  IsVisible = False;
}

void OnyxWindow::UnHide() {
  InternalWindow->MapWindow();
  IsVisible = True;
}

void OnyxWindow::SetBackgroundColor() {
  unsigned int Color;

  Color = ApplicationClient()->GetColorByName(BackgroundColor);
  InternalWindow->SetBackgroundColor(Color);
}

void OnyxWindow::SetBackgroundPicture() {
  // If picture given.
  if (strlen(Picture)) {
    if (InternalPicture)
      ApplicationPicturePool()->FreePicture(InternalPicture);
    InternalPicture = ApplicationPicturePool()->GetPicture(Picture);
    if (InternalPicture) 
      InternalWindow->SetBackgroundPixmap(InternalPicture->GetPicture());
  }
}

void OnyxWindow::ResetMinimumSize(int NewWidth, int NewHeight) {
  MinimumWidth = NewWidth;
  MinimumHeight = NewHeight;
  InternalWindow->SetWMSizeHints(MinimumWidth, MinimumHeight,
                                 MaximumWidth, MaximumHeight,
                                 ResizeWidthIncrement, ResizeHeightIncrement);
}

void OnyxWindow::ResetMaximumSize(int NewWidth, int NewHeight) {
  MaximumWidth = NewWidth;
  MaximumHeight = NewHeight;
  InternalWindow->SetWMSizeHints(MinimumWidth, MinimumHeight,
                                 MaximumWidth, MaximumHeight,
                                 ResizeWidthIncrement, ResizeHeightIncrement);
}

void OnyxWindow::ResetResizeIncrement(int NewWidth, int NewHeight) {
  ResizeWidthIncrement = NewWidth;
  ResizeHeightIncrement = NewHeight;
  InternalWindow->SetWMSizeHints(MinimumWidth, MinimumHeight,
                                 MaximumWidth, MaximumHeight,
                                 ResizeWidthIncrement, ResizeHeightIncrement);
}

void OnyxWindow::Close() {
  InternalWindow->CloseWindow();
}

OnyxWindow::~OnyxWindow() {
  if (WindowExists) {
    InternalWindow->ForceCloseWindow();
  }
}
