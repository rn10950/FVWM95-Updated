/**************************************************************************

    This file is part of xclass, a Win95-looking GUI toolkit.
    Copyright (C) 1996, 1997 David Barth, Hector Peraza.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

**************************************************************************/

#ifndef __OXMSGBOX_H
#define __OXMSGBOX_H

#include <X11/Xlib.h>

#include <xclass/utils.h>
#include <xclass/OXTransientFrame.h>
#include <xclass/OXButton.h>
#include <xclass/OXIcon.h>
#include <xclass/OXLabel.h>
#include <xclass/OString.h>
#include <xclass/OPicture.h>


//--- Icon types

#define MB_ICONSTOP         1
#define MB_ICONQUESTION     2
#define MB_ICONEXCLAMATION  3
#define MB_ICONASTERISK     4

//--- Button types and return codes

#define ID_YES          (1<<0)
#define ID_YESALL       (1<<1)
#define ID_NO           (1<<2)
#define ID_OK           (1<<3)
#define ID_APPLY        (1<<4)
#define ID_RETRY        (1<<5)
#define ID_IGNORE       (1<<6)
#define ID_CANCEL       (1<<7)
#define ID_CLOSE        (1<<8)
#define ID_DISMISS      (1<<9)


//----------------------------------------------------------------------

class OXMsgBox : public OXTransientFrame {
protected:
  void _MsgBox(const OXWindow *main,
               OString *title, OString *msg, const OPicture *icon,
               int buttons, int *ret_code);

public:
  OXMsgBox(const OXWindow *p, const OXWindow *main,
           OString *title, OString *msg, const OPicture *icon,
           int buttons = ID_DISMISS, int *ret_code = NULL,
           unsigned long options = MAIN_FRAME | VERTICAL_FRAME);
  OXMsgBox(const OXWindow *p, const OXWindow *main,
           OString *title, OString *msg, int icon,
           int buttons = ID_DISMISS, int *ret_code = NULL,
           unsigned long options = MAIN_FRAME | VERTICAL_FRAME);
  virtual ~OXMsgBox();

  virtual int CloseWindow();
  virtual int ProcessMessage(OMessage *msg);

protected:
  OXButton *Yes, *YesAll, *No, *OK, *Apply, 
           *Retry, *Ignore, *Cancel,
           *Close, *Dismiss;
  OXIcon   *Icon;
  OXLabel  *Label;
  OXHorizontalFrame *ButtonFrame;
  OXHorizontalFrame *IconFrame;
  OLayoutHints *L1, *L2, *L3, *L4, *L5;
  int *_ret_code;
};


#endif  // __OXMSGBOX_H
