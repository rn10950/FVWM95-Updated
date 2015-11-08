/**************************************************************************

    This file is part of xclauncher.
    Copyright (C) 1998-2002 Hector Peraza.

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

#ifndef __OXICONSELDIALOG_H
#define __OXICONSELDIALOG_H

#include <xclass/utils.h>
#include <xclass/OXFileDialog.h>
#include <xclass/OXGroupFrame.h>
#include <xclass/OXLabel.h>
#include <xclass/OXIcon.h>
#include <xclass/OMessage.h>


//----------------------------------------------------------------------

class OXIconSelDialog : public OXFileDialog {
public:
  OXIconSelDialog(const OXWindow *p, const OXWindow *t, OString *ret);
  virtual ~OXIconSelDialog();
  
  virtual int ProcessMessage(OMessage *msg);
  
protected:
  OXLabel *_name, *_size;
  OXGroupFrame *_gf;
  OXIcon *_icon;
  const OPicture *_pic;
  OLayoutHints *_lh1, *_lh2, *_lh3, *_lb, *_lp;
  OPicturePool *_picPool;
  OString *_picstr;
};


#endif  // __OXICONSELDIALOG_H
