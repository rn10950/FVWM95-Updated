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

#ifndef __OXFTPCONFIG_H
#define __OXFTPCONFIG_H

#include <xclass/OXTransientFrame.h>
#include <xclass/OXDDListBox.h>
#include <xclass/OXTextEntry.h>
#include <xclass/OXCheckButton.h>
#include <xclass/OXIcon.h>


class OIniFile;
class OLayoutHints;
class OGeneralConfig;

class OXDDListBox;
class OXTextEntry;
class OXCheckButton;
class OXSpinner;


//----------------------------------------------------------------------

class OXFtpConfig : public OXTransientFrame {
public:
  OXFtpConfig(const OXWindow *p, const OXWindow *main, char *homepath,
              int w, int h, 
              unsigned long options = MAIN_FRAME | VERTICAL_FRAME);

  virtual ~OXFtpConfig();

  void OpenWindow();

  virtual int CloseWindow();
  virtual int ProcessMessage(OMessage *);

  OGeneralConfig *GetGeneralConfig() { return _genConfig; }

protected:
  int ReadFile(char *file);
  int WriteFile(char *file);
  void UpdateMimeTypeLB();
  void SetWidgets();
  void GetWidgets();

  char *_currentFile;
  OGeneralConfig *_genConfig;

  OXDDListBox *_transferModeLb, *_overwriteModeLb, *_proxyModeLb;
  OXListBox *_mimeTypeLb;
  OXTextEntry *_anoPwdTe, *_downDirTe, *_cacheDirTe;
  OXTextEntry *_proxyHostTe, *_proxyPortTe;
  OXTextEntry *_recvBufferTe, *_sendBufferTe;
  OXCheckButton *_disCloseCb, *_closeDisCb;
  OXTextButton *_newTb, *_removeTb, *_editTb;
  OXSpinner *_siteRetrySp, *_retryDelaySp;
  OXLabel *_mimeTypeLa, *_mimeActionLa;
  OXIcon *_mimeIcon;

  OLayoutHints *_l1, *_l2, *_l3, *_l4, *_l5, *_l6, *_l7, *_l8, *_l9, *_l10;
  OLayoutHints *_l11, *_l12;
};


#endif  // __OXFTPCONFIG_H
