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

#ifndef __ONYXWINDOW_H
#define __ONYXWINDOW_H

#include <xclass/OXMainFrame.h>
#include <xclass/OMessage.h>
#include <xclass/OPicture.h>

#include "OnyxBase.h"


#define WindowGroup           (MSG_USERMSG+100)

#define WindowClosed          1   // Define Close Event ID
#define WindowCloseSignaled   2   // Define Close Signaled Event ID
#define WindowLoad            3   // Define Load Event ID
#define WindowMoved           4   // Define Move Event ID
#define WindowResized         5   // Define Resize Event ID


//----------------------------------------------------------------------

class RedirectedMainFrame : public OXMainFrame {
public:
  RedirectedMainFrame(const OXWindow *p,
                      int *x, int *y, int *w, int *h,
                      int ID, int *CloseExitsApplication,
                      int *ShouldAbortClose, 
                      int *WindowExists,
                      int *CloseDestroysWindow,
                      int *IsVisible,
                      long ActiveEvents,
                      unsigned long options = MAIN_FRAME | VERTICAL_FRAME);
  virtual ~RedirectedMainFrame();

  virtual int CloseWindow();
  virtual int ForceCloseWindow();

  virtual int HandleButton(XButtonEvent *event);
  virtual int HandleMotion(XMotionEvent *event);
  virtual int HandleKey(XKeyEvent *event);

private:
  virtual void _Moved();
  virtual void _Resized();

  int InternalID;
  int *X;
  int *Y;
  int *Width;
  int *Height;
  int *ApplicationShouldClose;
  int *ShouldAbortClose;
  int *WindowShouldBeDestroyed;
  int *CurrentVisibility;
  int *WinExists;
};


//----------------------------------------------------------------------

class OnyxWindow : public OnyxObject {
public:
  OnyxWindow();
  virtual ~OnyxWindow();

  virtual int  Create();
  virtual void Move(int newX, int newY);
  virtual void Resize(int newWidth, int newHeight);
  virtual void MoveResize(int newX, int newY, int newWidth, int newHeight);
  virtual void Show();
  virtual void Hide();
  virtual void UnHide();
  virtual void SetBackgroundColor();
  virtual void SetBackgroundPicture();
  virtual void Close();
  virtual void Update();
  virtual void ResetMinimumSize(int NewWidth, int NewHeight);
  virtual void ResetMaximumSize(int NewWidth, int NewHeight);
  virtual void ResetResizeIncrement(int NewWidth, int NewHeight);

  virtual const OXMainFrame *Self();

  int MinimumWidth;
  int MinimumHeight;

  int MaximumWidth;
  int MaximumHeight;

  int ResizeWidthIncrement;
  int ResizeHeightIncrement;

  char Name[255];
  char Class[255];
  char Resource[255];
  char Directory[255];
  char BackgroundColor[255];

  int IsVisible;

  int CloseDestroysWindow;
  int CloseExitsApplication;

  int AbortClose;

  int WindowExists;

  int ButtonPressActive;
  int ButtonReleaseActive;
  int PointerMotionActive;
  int KeyPressActive;
  int KeyReleaseActive;
  int ExposureActive;
  int StructureNotifyActive;

  int HasBorder;
  int HasTitleBar;
  int HasMenu;
  int HasMaximizeButton;
  int HasMinimizeButton;

  int HasMinimizeOption;
  int HasMaximizeOption;
  int HasCloseOption;
  int HasResizeOption;
  int HasMoveOption;

protected:
  RedirectedMainFrame *InternalWindow;
  const OPicture *InternalPicture;

  long ActiveEvents;
};


#endif  // __ONYXWINDOW_H
