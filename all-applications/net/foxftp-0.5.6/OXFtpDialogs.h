/**************************************************************************

    This file is part of foxftp, a ftp client for fOX.
    Copyright (C) 1998, 1999, G. Matzka.

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

#ifndef __OXFTPDIALOGS_H
#define __OXFTPDIALOGS_H


class OLayoutHints;
class OPicture;
class OSiteConfig;
class OGeneralConfig;

class OXButton;
class OXIcon;
class OXLabel;
class OXTextButton;
class OXCheckButton;
class OXPopupMenu;
class OXSecretTextEntry;
class OXTextEntry;
class OXVScrollBar;

#include <xclass/OXTransientFrame.h>


//----------------------------------------------------------------------

class OXPasswdDialog : public OXTransientFrame {
public:
  OXPasswdDialog(const OXWindow *p, const OXWindow *main, int w, int h,
                 int *ret, OSiteConfig *ret_info,
                 unsigned long options = MAIN_FRAME | VERTICAL_FRAME);
  virtual ~OXPasswdDialog();

  virtual int ProcessMessage(OMessage *msg);

protected:
  int *_ret_val;
  OSiteConfig *_ret_site;

  OXTextEntry *_loginEntry;
  //OXPwTextEntry *_passwdEntry;
  OXSecretTextEntry *_passwdEntry;
  OXLabel *_textLabel;
  OXLabel *_loginLabel, *_passwdLabel;
  OXButton *_tbOk, *_tbCancel;
  OXIcon *_connectIcon;
  const OPicture *_connectPicture;
  OXCompositeFrame *_frame1, *_frame2, *_frame3, *_frame4;
  OLayoutHints *_l1, *_l2, *_l3, *_l4;
};


//----------------------------------------------------------------------

class OXSetDirectory : public OXTransientFrame {
public:
  OXSetDirectory(const OXWindow *p, const OXWindow *main, int w, int h,
                 char *path, int *rtc,
                 unsigned long options = MAIN_FRAME | VERTICAL_FRAME);
  virtual ~OXSetDirectory();

  virtual int ProcessMessage(OMessage *);
 
protected:
  char *_path;
  int *_rtc;
 
  OXCompositeFrame *_bframe;
  OXTextButton *_tbOk, *_tbCancel;
  OXLabel *_lpath;
  OXTextEntry *_tepath;
  OLayoutHints *_l1, *_l2, *_l3;
};


//----------------------------------------------------------------------

class OXChooseType : public OXTransientFrame {
public:
  OXChooseType(const OXWindow *p, const OXWindow *main, int w, int h,
               int *rtc, unsigned long options = MAIN_FRAME | VERTICAL_FRAME);
  virtual ~OXChooseType();

  virtual int ProcessMessage(OMessage *);
 
protected:
  int *_rtc;

  OLayoutHints *_l1, *_l2, *_l3, *_l4;
  const OPicture *_binPic, *_asciiPic;
  OXButton *_binRb, *_asciiRb;
  OXIcon *_binIcon, *_asciiIcon;
};


#endif  // __OXFTPDIALOGS_H
