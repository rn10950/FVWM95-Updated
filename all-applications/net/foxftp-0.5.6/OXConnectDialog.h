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

#ifndef __OXCONNECTDIALOG_H
#define __OXCONNECTDIALOG_H

#include <xclass/OXTransientFrame.h>

class OLayoutHints;
class OPicture;
class OSiteConfig;
class OGeneralConfig;

class OXButton;
class OXIcon;
class OXLabel;
class OXTextButton;
class OXCheckButton;
class OXSecretTextEntry;
class OXTextEntry;


//----------------------------------------------------------------------

class OXConnectDialog : public OXTransientFrame {
public:
  OXConnectDialog(const OXWindow *p, const OXWindow *main, int w, int h,
                  int *ret_val, OSiteConfig *ret_site,
                  unsigned long options = MAIN_FRAME | VERTICAL_FRAME);
  virtual ~OXConnectDialog();

  virtual int ProcessMessage(OMessage *msg);

protected:
  int *_ret_val;
  OSiteConfig *_ret_site;
  OXTextEntry *_siteEntry, *_loginEntry, *_portEntry;
  OXSecretTextEntry *_passwdEntry;
  OXLabel *_siteLabel, *_loginLabel, *_passwdLabel, *_portLabel;
  OXTextButton *_tbOk, *_tbCancel;
  OXCheckButton *_useAnonymous, *_useProxy;
  OXIcon *_connectIcon;
  const OPicture *_connectPicture;
  OXCompositeFrame *_frame, *_infoFrame, *_buttonFrame, *_allFrame;
  OLayoutHints *_l1, *_l2, *_l3, *_l4, *_l5, *_l6, *_l7;
};


#endif  // __OXCONNECTDIALOG_H
