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

#ifndef __OXFTPTRANSFER_H
#define __OXFTPTRANSFER_H

class OFileSystem;
class ODir;
class OFile;
template <class T> class TDDLList;

class OLayoutHints;
class OPicture;
class OTimer;
class OXButton;
class OXIcon;
class OXLabel;
class OXProgressBar;
class OXStatusBar;
class OXSiteView;
class OXWindow;

#include <xclass/OXTransientFrame.h>


//----------------------------------------------------------------------

class OXFtpTransfer : public OXTransientFrame {
public:
  OXFtpTransfer(const OXWindow *p, const OXWindow *main, int w, int h,
              OXSiteView *from, OXSiteView *to, TDDLList<ODir *> *list,
              unsigned long options = MAIN_FRAME | VERTICAL_FRAME);
  virtual ~OXFtpTransfer();

  virtual int HandleTimer(OTimer *);
  virtual int HandleFileEvent(OFileHandler *fh, unsigned int mask);
  virtual int ProcessMessage(OMessage *msg);

protected:
  void DoFinish();
  void DoStart();

  char *_ioBuf;
  int _npic, _pending, _transfered, _total, _stall;
  struct timeval _startTime;

  TDDLList<ODir *> *_list;
  TDDLList<OFile *> *_cList;
  ODir *_cDelem;
  OFile *_cFelem;

  OFileHandler *_rh, *_wh;
  OXSiteView *_from, *_to;
  OTimer *_timer;

  const OPicture *_pics[10], *_filepic;
  OXButton *_cancelButton;
  OXIcon *_ftpIcon, *_fileIcon;
  OXLabel *_dirLabel, *_dirName, *_fileLabel, *_fileName;
  OXProgressBar *_fileProg, *_ftpProg;
  OXCompositeFrame *_frame1, *_frame2, *_frame3, *_frame4;
  OXStatusBar *_statusBar;
  OLayoutHints *_l1, *_l2, *_l3, *_l4, *_l5, *_l6;

  const OXWindow *_main;
};


#endif
