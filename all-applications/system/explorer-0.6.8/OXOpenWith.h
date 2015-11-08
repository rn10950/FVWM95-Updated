/**************************************************************************

    This file is part of explorer95, a file manager for fvwm95.
    Copyright (C) 1996, 1997 David Barth, Hector Peraza.

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

#ifndef __OXOPENWITH_H
#define __OXOPENWITH_H


#include <xclass/OXClient.h>
#include <xclass/OXTransientFrame.h>
#include <xclass/OXTextButton.h>
#include <xclass/OXCheckButton.h>
#include <xclass/OXLabel.h>
#include <xclass/OXListBox.h>
#include <xclass/OXIcon.h>
#include <xclass/utils.h>


//---------------------------------------------------------------------

class OXOpenWithDialog : public OXTransientFrame {
public:
  OXOpenWithDialog(const OXWindow *p, const OXWindow *main,
                   OString *docname, OString *progname,
                   unsigned long options = MAIN_FRAME | VERTICAL_FRAME);
  virtual ~OXOpenWithDialog();

  virtual int ProcessMessage(OMessage *msg);

protected:
  void UpdateListBox();
  void AddOther();
  const OPicture *GetFilePic(const char *name);

  OXButton *Ok, *Cancel, *Other;
  OLayoutHints *bly, *bfly, *ly1, *ly2, *ly3;
  OXListBox *lbf;
  OString *_progname;
  int _maxEntry;
};


#endif   // __OXOPENWITH_H
