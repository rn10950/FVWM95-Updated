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

#ifndef __ONYXBASE_H
#define __ONYXBASE_H

#include <xclass/OXClient.h>
#include <xclass/OXMainFrame.h>
#include <xclass/OXFont.h>
#include <xclass/OResourcePool.h>
#include <xclass/OMessage.h>
#include <xclass/utils.h>
#include <xclass/version.h>


#define MSG_ONYX              (MSG_USERMSG+9662)

#define NoType                0
#define MenuType              MSG_MENU
#define ButtonType            MSG_BUTTON
#define TextBoxType           MSG_TEXTENTRY
#define HorizontalScrollType  MSG_HSCROLL
#define VerticalScrollType    MSG_VSCROLL
#define ContainerType         MSG_CONTAINER
#define RadioButtonType       MSG_RADIOBUTTON
#define SpinButtonType        MSG_SPINNER
#define CheckButtonType       MSG_CHECKBUTTOM
#define ListBoxType           MSG_LISTBOX
#define ComboBoxType          MSG_COMBOBOX
#define WindowType            MSG_CONTAINER

#define DefaultAction         0
#define ButtonClicked         100
#define ButtonPressed         110
#define ButtonReleased        120
#define ButtonDoubleClicked   130
#define KeyClicked            200
#define KeyPressed            210
#define KeyReleased           220
#define EnterPressed          230
#define TextChanged           240
#define ValueChanged          250
#define PointerMoved          300
#define ItemSelected          400
#define Paint                 500

#define NoBorder              0
#define SunkenSingle          SUNKEN_FRAME
#define SunkenDouble          (SUNKEN_FRAME | DOUBLE_BORDER)
#define RaisedSingle          RAISED_FRAME
#define RaisedDouble          (RAISED_FRAME | DOUBLE_BORDER)
#define BorderBits            (RAISED_FRAME | SUNKEN_FRAME | DOUBLE_BORDER)

#define AlignTop              1
#define AlignBottom           2
#define AlignLeft             1
#define AlignRight            2
#define AlignCenter           3


//----------------------------------------------------------------------

class OnyxMessage : public OWidgetMessage {
public:
  OnyxMessage(int typ = MSG_ONYX, int act = DefaultAction, int wid = -1) :
    OWidgetMessage(typ, act, wid) {}

public:
  int onyx_type;
  int x, y, x_root, y_root, old_x, old_y;
  int direction, position;
  unsigned int keysym, button;
  unsigned int old_width, old_height, width, height;
  int item_number;
};


//----------------------------------------------------------------------

class OnyxObject {
public:
  OnyxObject(char *ObjectType);
  virtual ~OnyxObject();
  
  char *ObjectType() { return OType; }
  
  virtual int  Create();
  virtual void Move(int x, int y);
  virtual void Resize(int width, int height);
  virtual void MoveResize(int x, int y, int width, int height);
  virtual void Update();
  virtual void Enable();
  virtual void Disable();

  int ID;
  int X;
  int Y;
  int Width;
  int Height;
  int Size;

  int  BorderStyle;
  int  VerticalAlignment;
  int  HorizontalAlignment;

  int  IsChecked;
  int  IsEnabled;
  int  IsShaped;
  int  EnableToolTip;
  char Text[256];
  char Picture[256];
  char ToolTip[256];
  char Font[256];

  const OXWindow *Parent;

private:
  char OType[32];
};


//----------------------------------------------------------------------

class MessageCenter : public OXMainFrame {
public:
  MessageCenter(const OXWindow *p) : OXMainFrame(p, 10, 10) {}

  virtual int ProcessMessage(OMessage *msg);

  char ApplicationName[255];
};

void Initialize(char *ApplicationName, int argc, char *argv[]);
const OXWindow *ApplicationRoot();
const OXWindow *Application();
OXClient *ApplicationClient();
OPicturePool *ApplicationPicturePool();
void ApplicationRun();
void ApplicationClose();

#endif  // __ONYXBASE_H
