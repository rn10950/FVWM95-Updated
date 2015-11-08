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

#ifndef __ONYXMSGBOX_H
#define __ONYXMSGBOX_H

#include <xclass/OXMsgBox.h>

#include "OnyxBase.h"


#define StopIcon MB_ICONSTOP
#define QuestionIcon MB_ICONQUESTION
#define ExclamationIcon MB_ICONEXCLAMATION
#define AsteriskIcon MB_ICONASTERISK

#define YesButton ID_YES
#define NoButton ID_NO
#define OkButton ID_OK
#define CancelButton ID_CANCEL
#define RetryButton ID_RETRY
#define IgnoreButton ID_IGNORE
#define ApplyButton ID_APPLY
#define CloseButton ID_CLOSE
#define DismissButton ID_DISMISS


//----------------------------------------------------------------------

class OnyxMsgBox : public OnyxObject {
public:
  OnyxMsgBox();
  virtual ~OnyxMsgBox();

  virtual int Create();
  virtual void Show();
  virtual void AddButton(int Button);
  virtual void RemoveButton(int Button);

  char Title[255];
  char Message[255];

  const OXWindow *DisplayWindow;

  int Icon;
  int Response;

  int UseYesButton;
  int UseNoButton;
  int UseOkButton;
  int UseCancelButton;
  int UseRetryButton;
  int UseIgnoreButton;
  int UseApplyButton;
  int UseCloseButton;
  int UseDismissButton;

protected:
  OXMsgBox *InternalMsgBox;
  int MsgBoxExists;
  int Buttons;
};


void MsgBox(const OXWindow *Parent, char *Title, char *Message);
void StopBox(const OXWindow *Parent, char *Title, char *Message);
void QuestionBox(const OXWindow *Parent, char *Title, char *Message);
void ExclamationBox(const OXWindow *Parent, char *Title, char *Message);
void AsteriskBox(const OXWindow *Parent, char *Title, char *Message);

void MsgBox(const OXWindow *Parent, char *Message);
void StopBox(const OXWindow *Parent, char *Message);
void QuestionBox(const OXWindow *Parent, char *Message);
void ExclamationBox(const OXWindow *Parent, char *Message);
void AsteriskBox(const OXWindow *Parent, char *Message);

#endif  // __ONYXMSGBOX_H
