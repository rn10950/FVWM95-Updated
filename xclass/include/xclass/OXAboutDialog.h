/**************************************************************************

    This file is part of xclass.
    Copyright (C) 1998, 1999, Hector Peraza.

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

#ifndef __OXABOUTDIALOG_H
#define __OXABOUTDIALOG_H


#include <xclass/OXTransientFrame.h>
#include <xclass/OPicture.h>
#include <xclass/OXTextButton.h>
#include <xclass/OXFont.h>


//--------------------------------------------------------------------

class OAboutInfo : public OBaseObject {
public:
  OAboutInfo();

public:
  char *wname, *title, *copyright, *text;
  int title_justify, cprt_justify, text_justify;
  OXFont *title_font, *cprt_font, *text_font;
};


//--------------------------------------------------------------------

class OXAboutDialog : public OXTransientFrame {
public:
  OXAboutDialog(const OXWindow *p, const OXWindow *main, OAboutInfo *info,
                unsigned long options = MAIN_FRAME | VERTICAL_FRAME);
  ~OXAboutDialog();

  virtual int ProcessMessage(OMessage *msg);

protected:
  OXButton *_ok;
  const OPicture *_pic;
  OLayoutHints *_lh1, *_lh2, *_lh3, *_lhi, *_lhe, *_lhb, *_lhf;
};


#endif  // __OXABOUTDIALOG_H
