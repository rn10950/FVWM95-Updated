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

#include <X11/X.h>

#include "OnyxButton.h"

extern MessageCenter *CentralMessageCenter;


//----------------------------------------------------------------------

RedirectedButtonFrame::RedirectedButtonFrame(const OXWindow *p,
    int w, int h, int ID) : OXCompositeFrame(p, w, h) {
  InternalID = ID;
}

int RedirectedButtonFrame::ProcessMessage(OMessage *msg) {
  return CentralMessageCenter->ProcessMessage(msg);
}


//----------------------------------------------------------------------

RedirectedTextButton::RedirectedTextButton(const OXWindow *p,
    OHotString *text, int ID, int *IsSticky, int *IsToggle,
    int *DisableStopsClick) :
  OXTextButton(p, text, ID) {

  InternalID = ID;
  ButtonIsSticky = IsSticky;
  ButtonIsToggle = IsToggle;
  DisableClick = DisableStopsClick;
  SetType((*ButtonIsSticky + *ButtonIsToggle * 2) % 3);
  State = 0;
  Clicked = 0;

  AddInput(StructureNotifyMask | ButtonPressMask |  ButtonReleaseMask |
           PointerMotionMask | ExposureMask | KeyPressMask | KeyReleaseMask |
           EnterWindowMask | LeaveWindowMask);
}

int RedirectedTextButton::HandleCrossing(XCrossingEvent *event) {
  OXTextButton::HandleCrossing(event);

  if (event->type == LeaveNotify) {
    if (!State) Clicked = 0;
  } else {
    if (State) Clicked = 1;
  }

  return True;
}

int RedirectedTextButton::HandleMotion(XMotionEvent *event) {
  OnyxMessage omsg;

  OXTextButton::HandleMotion(event);

  omsg.onyx_type = ButtonType;
  omsg.id = InternalID;
  omsg.action = PointerMoved;
  omsg.x = event->x;
  omsg.y = event->y;
  omsg.x_root = event->x_root;
  omsg.y_root = event->y_root;

  return CentralMessageCenter->ProcessMessage(&omsg);
}

int RedirectedTextButton::HandleButton(XButtonEvent *event) {
  OnyxMessage omsg;

  omsg.onyx_type = ButtonType;
  omsg.id = InternalID;

  omsg.button = event->button;
  omsg.x = event->x;
  omsg.y = event->y;
  omsg.x_root = event->x_root;
  omsg.y_root = event->y_root;

  if (event->type == ButtonPress) {
    Clicked = 1;

    omsg.action = ButtonPressed;
    CentralMessageCenter->ProcessMessage(&omsg);
    if (event->button == 1) {
      OXTextButton::HandleButton(event);
    }
  } else {
    omsg.action = ButtonReleased;
    CentralMessageCenter->ProcessMessage(&omsg);
    if (event->button == 1) {
      OXTextButton::HandleButton(event);
      if (Clicked) {
        // Send event so long as we arent disabled on the button and the event
        if (!*DisableClick || IsEnabled()) {
          omsg.action = ButtonClicked;
          CentralMessageCenter->ProcessMessage(&omsg);
        } else if (*DisableClick && !IsEnabled()) {
          // If the disable is on both just beep to signal an error to the user.
          XBell(GetDisplay(), 0);
        }
      }
    }
    Clicked = 0;
  }

  return True;
}

int RedirectedTextButton::HandleKey(XKeyEvent *event) {
  OnyxMessage omsg;

  omsg.onyx_type = ButtonType;
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

void RedirectedTextButton::AlterState(int NewState) {
  State = NewState;
  SetState(NewState);
}

RedirectedTextButton::~RedirectedTextButton() {
}


//----------------------------------------------------------------------

RedirectedPictureButton::RedirectedPictureButton(const OXWindow *p,
    const OPicture *pic, int ID, int *IsSticky, int *IsToggle,
    int *DisableStopsClick) :
  OXPictureButton(p, pic, ID) {

  InternalID = ID;
  ButtonIsSticky = IsSticky;
  ButtonIsToggle = IsToggle;
  SetType((*ButtonIsSticky + *ButtonIsToggle * 2) % 3);
  DisableClick = DisableStopsClick;
  State = 0;
  Clicked = 0;

  AddInput(StructureNotifyMask | ButtonPressMask |  ButtonReleaseMask |
           PointerMotionMask | ExposureMask | KeyPressMask | KeyReleaseMask |
           EnterWindowMask | LeaveWindowMask);
}

int RedirectedPictureButton::HandleMotion(XMotionEvent *event) {
  OnyxMessage omsg;

  OXPictureButton::HandleMotion(event);

  omsg.onyx_type = ButtonType;
  omsg.id = InternalID;
  omsg.action = PointerMoved;
  omsg.x = event->x;
  omsg.y = event->y;
  omsg.x_root = event->x_root;
  omsg.y_root = event->y_root;

  return CentralMessageCenter->ProcessMessage(&omsg);
}

int RedirectedPictureButton::HandleCrossing(XCrossingEvent *event) {
  if (event->type == LeaveNotify) {
//    if (!State) Clicked = 2;

    if (Clicked > 0) {
      switch (Clicked) {
        case 1:
          if (event->state == Button1Mask) {
            Clicked *= -1;
          } else {
            Clicked = 0;
          }
          break;

        case 2:
          if (event->state == Button2Mask) {
            Clicked *= -1;
          } else {
            Clicked = 0;
          }
          break;

        case 3:
          if (event->state == Button3Mask) {
            Clicked *= -1;
          } else {
            Clicked = 0;
          }
          break;

        case 4:
          if (event->state == Button4Mask) {
            Clicked *= -1;
          } else {
            Clicked = 0;
          }
          break;

        case 5:
          if (event->state == Button5Mask) {
            Clicked *= -1;
          } else {
            Clicked = 0;
          }
          break;

        default:
          Clicked = 0;
          break;
      }
    }
    OXPictureButton::HandleCrossing(event);
  } else {
    if (Clicked < 0) {
      switch (Clicked) {
        case -1:
          if (event->state == Button1Mask) {
            Clicked *= -1;
          } else {
            Clicked = 0;
          }
          break;

        case -2:
          if (event->state == Button2Mask) {
            Clicked *= -1;
          } else {
            Clicked = 0;
          }
          break;

        case -3:
          if (event->state == Button3Mask) {
            Clicked *= -1;
          } else {
            Clicked = 0;
          }
          break;

        case -4:
          if (event->state == Button4Mask) {
            Clicked *= -1;
          } else {
            Clicked = 0;
          }
          break;

        case -5:
          if (event->state == Button5Mask) {
            Clicked *= -1;
          } else {
            Clicked = 0;
          }
          break;

        default:
          Clicked = 0;
          break;
      }

      if (Clicked > 0) {
        OXPictureButton::HandleCrossing(event);
      }
    } else {
      OXPictureButton::HandleCrossing(event);
    }
  }

  return True;
}

int RedirectedPictureButton::HandleButton(XButtonEvent *event) {
  OnyxMessage omsg;

  omsg.onyx_type = ButtonType;
  omsg.id = InternalID;

  omsg.button = event->button;
  omsg.x = event->x;
  omsg.y = event->y;
  omsg.x_root = event->x_root;
  omsg.y_root = event->y_root;

  if (event->type == ButtonPress) {
    Clicked = event->button;
    omsg.action = ButtonPressed;
    CentralMessageCenter->ProcessMessage(&omsg);
    if (event->button == 1) {
      OXPictureButton::HandleButton(event);
    }
  } else {
    omsg.action = ButtonReleased;
    CentralMessageCenter->ProcessMessage(&omsg);
    if (event->button == 1) {
      if (Clicked > 0) {
        // Send event so long as we aren't disabled on the button and the event
        if (!*DisableClick || IsEnabled()) {
          omsg.action = ButtonClicked;
          CentralMessageCenter->ProcessMessage(&omsg);
        } else if (*DisableClick && !IsEnabled()) {
          // If the disable is on both just beep to signal an error to the user.
          XBell(GetDisplay(), 0);
        }
      }
      OXPictureButton::HandleButton(event);
    }
    Clicked = 0;
  }

  return True;
}

int RedirectedPictureButton::HandleKey(XKeyEvent *event) {
  OnyxMessage omsg;

  omsg.onyx_type = ButtonType;
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

void RedirectedPictureButton::AlterState(int NewState) {
  State = NewState;
  SetState(NewState);
}

RedirectedPictureButton::~RedirectedPictureButton() {
}


//----------------------------------------------------------------------

OnyxButton::OnyxButton() : OnyxObject("OnyxButton") {
  ButtonExists = 0;
  Width = 100;
  Height = 100;
  X = 0;
  Y = 15;
  IsSticky = False;
  IsToggle = False;
  IsTextButton = True;
  PictureFound = True;
  DisableStopsClick = True;
  InternalButton = NULL;
  InternalPictureButton = NULL;
}

int OnyxButton::Create() {
  OnyxObject::Create();
  if (!ButtonExists) {
    InternalFrame = new RedirectedButtonFrame(Parent, Width, Height, ID);

    if (!strlen(Picture)) {
      IsTextButton = True;
      InternalButton = new RedirectedTextButton(InternalFrame,
                             new OHotString(Text), ID, &IsSticky, &IsToggle,
                             &DisableStopsClick);
      InternalFrame->AddFrame(InternalButton, new OLayoutHints(LHINTS_EXPAND_ALL));
      InternalButton->Associate(InternalFrame);
      InternalButton->SetFont(ApplicationClient()->GetFont(Font));
    } else {
      IsTextButton = False;
      InternalPicture = NULL;
      InternalPicture = ApplicationPicturePool()->GetPicture(Picture);
      if (InternalPicture == NULL) {
        // Could not locate picture!!!
        // Load blank text button to cover in emergency!!!
        PictureFound = False;
        IsTextButton = True;
        InternalButton = new RedirectedTextButton(InternalFrame,
                               new OHotString(""), ID, &IsSticky, &IsToggle,
                               &DisableStopsClick);
        InternalFrame->AddFrame(InternalButton, new OLayoutHints(LHINTS_EXPAND_ALL));
        InternalButton->Associate(InternalFrame);
      } else {
        // Everything ok. Keep going.
        InternalPictureButton = new RedirectedPictureButton(InternalFrame,
                                      InternalPicture, ID, &IsSticky, &IsToggle,
                                      &DisableStopsClick);
        InternalFrame->AddFrame(InternalPictureButton, new OLayoutHints(LHINTS_EXPAND_ALL));
        InternalPictureButton->Associate(InternalFrame);
      }
    }

    if (!IsEnabled) Disable();

    InternalFrame->MapSubwindows();
    InternalFrame->Layout();
    InternalFrame->Move(X, Y);

    if (EnableToolTip) {
      if (IsTextButton) {
        InternalButton->SetTip(ToolTip);
      } else {
        InternalPictureButton->SetTip(ToolTip);
      }
    }

    ButtonExists++;
  }

  return ButtonExists;
}

void OnyxButton::Push() {
  if (IsTextButton) {
    if (((IsSticky + IsToggle) % 3) == 2) {
      InternalButton->AlterState(BUTTON_DOWN);
    } else {
      InternalButton->AlterState(BUTTON_ENGAGED);
    }
  } else {
    if (((IsSticky + IsToggle) % 3) == 2) {
      InternalPictureButton->AlterState(BUTTON_DOWN);
    } else {
      InternalPictureButton->AlterState(BUTTON_ENGAGED);
    }
  }
}

void OnyxButton::Pop() {
  if (IsTextButton) {
    InternalButton->AlterState(BUTTON_UP);
  } else {
    InternalPictureButton->AlterState(BUTTON_UP);
  }
}

void OnyxButton::Move(int newX, int newY) {
  OnyxObject::Move(newX, newY);
  InternalFrame->Move(newX, newY);
}

void OnyxButton::Resize(int newWidth, int newHeight) {
  OnyxObject::Resize(newWidth, newHeight);
  InternalFrame->Resize(newWidth, newHeight);
}

void OnyxButton::MoveResize(int newX, int newY, int newWidth, int newHeight) {
  OnyxObject::MoveResize(newX, newY, newWidth, newHeight);
  InternalFrame->MoveResize(newX, newY, newWidth, newHeight);
}

void OnyxButton::Update() {
  OnyxObject::Update();
  if (strlen(Picture)) {
    IsTextButton = False;

    if (InternalPicture) {
      ApplicationPicturePool()->FreePicture(InternalPicture);
      InternalPicture = NULL;
    }

    InternalPicture = ApplicationPicturePool()->GetPicture(Picture);
    if (InternalPicture == NULL) {
      // Could not locate picture!!!
      // Load blank text button to cover in emergency!!!
      PictureFound = False;
      IsTextButton = True;

      if (InternalButton == NULL) {
        InternalFrame->RemoveFrame(InternalPictureButton);
        delete InternalPictureButton;
        InternalPictureButton = NULL;

        InternalButton = new RedirectedTextButton(InternalFrame,
                               new OHotString(""), ID, &IsSticky, &IsToggle,
                               &DisableStopsClick);
        InternalFrame->AddFrame(InternalButton, new OLayoutHints(LHINTS_EXPAND_ALL));
        InternalButton->Associate(InternalFrame);

        if (!IsEnabled) {
          Disable();
        } else {
          Enable();
        }

        InternalFrame->MapSubwindows();
        InternalFrame->Layout();
        InternalFrame->Move(X, Y);
      } else {
        InternalButton->SetText(new OHotString(""));
      }
    } else {
      /* Everything ok. Keep going. */
      PictureFound = True;

      if (InternalPictureButton == NULL) {
        InternalFrame->RemoveFrame(InternalButton);
        delete InternalButton;
        InternalButton = NULL;
      } else {
        InternalFrame->RemoveFrame(InternalPictureButton);
        delete InternalPictureButton;
      }

      InternalPictureButton = new RedirectedPictureButton(InternalFrame,
                                    InternalPicture, ID, &IsSticky, &IsToggle,
                                    &DisableStopsClick);
      InternalFrame->AddFrame(InternalPictureButton, new OLayoutHints(LHINTS_EXPAND_ALL));
      InternalPictureButton->Associate(InternalFrame);

      if (!IsEnabled) {
        Disable();
      } else {
        Enable();
      }

      InternalFrame->MapSubwindows();
      InternalFrame->Layout();
      InternalFrame->Move(X, Y);
      //_on->Draw(GetDisplay(), _id, _normGC, 0, y0);
    }
  } else {
    IsTextButton = True;

    if (InternalButton == NULL) {
      InternalFrame->RemoveFrame(InternalPictureButton);
      delete InternalPictureButton;
      InternalPictureButton = NULL;

      InternalButton = new RedirectedTextButton(InternalFrame,
                             new OHotString(""), ID, &IsSticky, &IsToggle,
                             &DisableStopsClick);
      InternalFrame->AddFrame(InternalButton, new OLayoutHints(LHINTS_EXPAND_ALL));
      InternalButton->Associate(InternalFrame);

      if (!IsEnabled) {
        Disable();
      } else {
        Enable();
      }

      InternalFrame->MapSubwindows();
      InternalFrame->Layout();
      InternalFrame->Move(X, Y);
    } else {
      InternalButton->SetText(new OHotString(Text));
    }
    InternalButton->SetFont(ApplicationClient()->GetFont(Font));
  }

  if (IsTextButton) {
    if (EnableToolTip) {
      InternalButton->SetTip(ToolTip);
    } else {
      InternalButton->RemoveTip();
    }
    InternalButton->NeedRedraw();
  } else {
    if (EnableToolTip) {
      InternalPictureButton->SetTip(ToolTip);
    } else {
      InternalPictureButton->RemoveTip();
    }
    InternalPictureButton->NeedRedraw();
  }
}

void OnyxButton::Enable() {
  OnyxObject::Enable();
  if (IsTextButton) {
    InternalButton->Enable();
  } else {
    InternalPictureButton->Enable();
  }
}

void OnyxButton::Disable() {
  OnyxObject::Disable();
  if (IsTextButton) {
    InternalButton->Disable();
  } else {
    InternalPictureButton->Disable();
  }
}

OnyxButton::~OnyxButton() {
  InternalFrame->DestroyWindow();
  delete InternalFrame;  // this deletes InternalButton or InternalPictureButton
}
