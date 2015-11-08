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

#ifndef __OXPROPERTIES_H
#define __OXPROPERTIES_H

#include <xclass/OXTransientFrame.h>
#include <xclass/OString.h>
#include <xclass/OPicture.h>
#include <xclass/utils.h>

class OFile;
class OLayoutHints;
class OPicture;
class OTextBuffer;

class OXButton;
class OXCheckButton;
class OXFileIcon;
class OXGroupFrame;
class OXHorizontal3dLine;
class OXLabel;
class OXTextEntry;


//----------------------------------------------------------------------

class OXFtpPropertiesDialog : public OXTransientFrame {
public:
  OXFtpPropertiesDialog(const OXWindow *p, const OXWindow *main, int *rtc,
                        char *path, OFile *fitem, const OPicture *pic,
                        int local, unsigned long options = MAIN_FRAME | VERTICAL_FRAME);
  virtual ~OXFtpPropertiesDialog();

  virtual int ProcessMessage(OMessage *msg);

  void InitControls();
  void ApplyProperties();

protected:
  OFile *_fitem;
  const OPicture *_pic, *_link;
  char *_path;
  int _local, *_rtc;

  //--- "OK Cancel Apply" buttons:
  OLayoutHints *_bl1, *_bl2, *_l1, *_l2, *_l3, *_l4, *_l5, *_l6, *_l7, *_l8;
  OXButton *_apply;

  OXFileIcon *_fileIcon;
  OXTextEntry *_fileName;
  // file description and misc controls
  OXLabel *ltype, *ldev, *lusr, *lgrp, *llnks, *lsz, *lmod;

  // access permission controls
  OXCheckButton *_ur, *_uw, *_ux,
                *_gr, *_gw, *_gx,
                *_or, *_ow, *_ox;
};


#endif  // __OXPROPERTIES_H
