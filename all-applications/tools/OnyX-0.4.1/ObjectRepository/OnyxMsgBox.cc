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

#include "OnyxMsgBox.h"

extern MessageCenter *CentralMessageCenter;


//----------------------------------------------------------------------

OnyxMsgBox::OnyxMsgBox() : OnyxObject("OnyxMsgBox") {
  Icon = 0;
  Buttons = 0;
  strcpy(Title, CentralMessageCenter->ApplicationName);
}

int OnyxMsgBox::Create() {
  OnyxObject::Create();
  Buttons += UseYesButton     * YesButton    |
             UseNoButton      * NoButton     |
             UseOkButton      * OkButton     |
             UseCancelButton  * CancelButton |
             UseRetryButton   * RetryButton  |
             UseIgnoreButton  * IgnoreButton |
             UseApplyButton   * ApplyButton  |
             UseCloseButton   * CloseButton  |
             UseDismissButton * DismissButton;

  return True;
}

void OnyxMsgBox::Show() {
  InternalMsgBox = new OXMsgBox(DisplayWindow, Parent, new OString(Title),
                                new OString(Message), Icon, Buttons, &Response);
}

void OnyxMsgBox::AddButton(int Button) {
  /* Add it in. */
  Buttons = Buttons | Button;

  /* Modify current listing chart. */
  switch (Button) {
    case YesButton:
      UseYesButton = 1;
      break;

    case NoButton:
      UseNoButton = 1;
      break;

    case OkButton:
      UseOkButton = 1;
      break;

    case CancelButton:
      UseCancelButton = 1;
      break;

    case RetryButton:
      UseRetryButton = 1;
      break;

    case IgnoreButton:
      UseIgnoreButton = 1;
      break;

    case ApplyButton:
      UseApplyButton = 1;
      break;

    case CloseButton:
      UseCloseButton = 1;
      break;

    case DismissButton:
      UseDismissButton = 1;
      break;
  }
}

void OnyxMsgBox::RemoveButton(int Button) {
  /* Toss it in so we can safely pull it out (in case it wasn't there) */
  Buttons = Buttons | Button;
  Buttons = Buttons - Button;

  /* Modify current listing chart. */
  switch (Button) {
    case YesButton:
      UseYesButton = 0;
      break;

    case NoButton:
      UseNoButton = 0;
      break;

    case OkButton:
      UseOkButton = 0;
      break;

    case CancelButton:
      UseCancelButton = 0;
      break;

    case RetryButton:
      UseRetryButton = 0;
      break;

    case IgnoreButton:
      UseIgnoreButton = 0;
      break;

    case ApplyButton:
      UseApplyButton = 0;
      break;

    case CloseButton:
      UseCloseButton = 0;
      break;

    case DismissButton:
      UseDismissButton = 0;
      break;
  }
}

OnyxMsgBox::~OnyxMsgBox() {
  delete InternalMsgBox;
}

void MsgBox(const OXWindow *Parent, char *Title, char *Message) {
  new OXMsgBox(ApplicationRoot(), Parent, new OString(Title),
               new OString(Message), 0, ID_OK);
}

void MsgBox(const OXWindow *Parent, char *Message) {
  new OXMsgBox(ApplicationRoot(), Parent,
               new OString(CentralMessageCenter->ApplicationName),
               new OString(Message), 0, ID_OK);
}

void StopBox(const OXWindow *Parent, char *Title, char *Message) {
  new OXMsgBox(ApplicationRoot(), Parent, new OString(Title),
               new OString(Message), MB_ICONSTOP, ID_OK);
}

void StopBox(const OXWindow *Parent, char *Message) {
  new OXMsgBox(ApplicationRoot(), Parent,
               new OString(CentralMessageCenter->ApplicationName),
               new OString(Message), MB_ICONSTOP, ID_OK);
}

void QuestionBox(const OXWindow *Parent, char *Title, char *Message) {
  new OXMsgBox(ApplicationRoot(), Parent, new OString(Title),
               new OString(Message), MB_ICONQUESTION, ID_OK);
}

void QuestionBox(const OXWindow *Parent, char *Message) {
  new OXMsgBox(ApplicationRoot(), Parent,
               new OString(CentralMessageCenter->ApplicationName),
               new OString(Message), MB_ICONQUESTION, ID_OK);
}

void ExclamationBox(const OXWindow *Parent, char *Title, char *Message) {
  new OXMsgBox(ApplicationRoot(), Parent, new OString(Title),
               new OString(Message), MB_ICONEXCLAMATION, ID_OK);
}

void ExclamationBox(const OXWindow *Parent, char *Message) {
  new OXMsgBox(ApplicationRoot(), Parent,
               new OString(CentralMessageCenter->ApplicationName),
               new OString(Message), MB_ICONEXCLAMATION, ID_OK);
}

void AsteriskBox(const OXWindow *Parent, char *Title, char *Message) {
  new OXMsgBox(ApplicationRoot(), Parent, new OString(Title),
               new OString(Message), MB_ICONASTERISK, ID_OK);
}

void AsteriskBox(const OXWindow *Parent, char *Message) {
  new OXMsgBox(ApplicationRoot(), Parent,
               new OString(CentralMessageCenter->ApplicationName),
               new OString(Message), MB_ICONASTERISK, ID_OK);
}
