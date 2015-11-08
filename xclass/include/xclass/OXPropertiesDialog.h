/**************************************************************************

    This file is part of xclass.
    Copyright (C) 1996-2000 David Barth, Hector Peraza.

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

#ifndef __OXPROPERTIESDIALOG_H
#define __OXPROPERTIESDIALOG_H

#include <sys/stat.h>

#include <xclass/utils.h>
#include <xclass/OXTransientFrame.h>
#include <xclass/OXIcon.h>
#include <xclass/OXTextButton.h>
#include <xclass/OXCheckButton.h>
#include <xclass/OXRadioButton.h>
#include <xclass/OXLabel.h>
#include <xclass/OXTextEntry.h>
#include <xclass/OXTab.h>


//----------------------------------------------------------------------

class OXFileIcon : public OXIcon {
public:
  OXFileIcon(const OXWindow *p, const OPicture *pic, const OPicture *lpic,
             unsigned int options = CHILD_FRAME,
             unsigned long back = _defaultDocumentBackground) :
    OXIcon(p, pic, options, back) { _lpic = lpic; }

  void SetPics(const OPicture *pic, const OPicture *lpic) {
    OXIcon::SetPicture(pic);
    _lpic = lpic;
    NeedRedraw();
  }

protected:
  virtual void _DoRedraw() {
    OXIcon::_DoRedraw();
    if (_lpic) _lpic->Draw(_client->GetDisplay(), _id, _bckgndGC, 0, 0);
  }

  const OPicture *_lpic;
};

class OXPropertiesDialog : public OXTransientFrame {
public:
  OXPropertiesDialog(const OXWindow *p, const OXWindow *main, OString *fname,
                     unsigned long options = MAIN_FRAME | VERTICAL_FRAME);
  virtual ~OXPropertiesDialog();

  virtual int ProcessMessage(OMessage *msg);

protected:
  void StatFile();
  void UpdateName();
  void UpdatePics();
  
  void AddGeneralTab();
  void AddAttributesTab();
  void AddTargetTab();

  void InitControls();
  void ApplyProperties();
  void GetFilePictures(const OPicture **pic, const OPicture **lpic,
                       char *fname, int ftype, int is_link);

  OString *_fname, *_tname;
  int _type, _is_link;

  const OPicture *_folder, *_app, *_doc, *_slink;

  //--- "OK Cancel Apply" buttons:
  OXButton *Ok, *Cancel, *Apply;

  OLayoutHints *bly, *bfly;
  OLayoutHints *Ltab;

  OXTab *_tab;

  //--- "General" Tab (1)
  OXFileIcon *_fileIcon;
  OXTextEntry *_fileName;
  OXLabel *ltype;    // file type
  OXLabel *ldev;     // device
  OXLabel *lusr;     // owner
  OXLabel *lgrp;     // group
  OXLabel *llnks;    // hard links
  OXLabel *lsz;      // file size
  OXLabel *lacc;     // last access
  OXLabel *lmod;     // last modification
  OXLabel *lchg;     // last change
  OLayoutHints *Vly, *lhf1, *Tly, *Ily, *Lr, *Lc;

  //--- "Attributes" Tab (2)
  OXCheckButton *_ur, *_uw, *_ux,
                *_gr, *_gw, *_gx,
                *_or, *_ow, *_ox,
                *_sticky, *_suid, *_sgid;  // access permission controls
  OLayoutHints *Lc1, *Lg, *Ls;

  //--- "Target" Tab (3)
  OXFileIcon *_targetIcon;
  OXTextEntry *_targetName;
  OXLabel *lttype;   // file type
  OXLabel *ltdev;    // device
  OXLabel *ltusr;    // owner
  OXLabel *ltgrp;    // group
  OXLabel *ltlnks;   // hard links
  OXLabel *ltsz;     // file size
  OXLabel *ltacc;    // last access
  OXLabel *ltmod;    // last modification
  OXLabel *ltchg;    // last change

  struct stat _sbuf, _lsbuf;
  int _stat_errno, _lstat_errno;
};


#endif  // __OXPROPERTIESDIALOG_H
