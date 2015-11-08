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

#ifndef __OXFONTDIALOG_H
#define __OXFONTDIALOG_H


#include <xclass/OXTransientFrame.h>
#include <xclass/OXGroupFrame.h>
#include <xclass/OXTextButton.h>
#include <xclass/OXLabel.h>
#include <xclass/OXListBox.h>
#include <xclass/OXFont.h>
#include <xclass/OMessage.h>


//----------------------------------------------------------------------

class OXFontDialog : public OXTransientFrame {
public:
  OXFontDialog(const OXWindow *p, const OXWindow *t,
               OString *nret = NULL, OString *sample = NULL);
  virtual ~OXFontDialog();
  
  virtual int ProcessMessage(OMessage *msg);
  virtual int HandleKey(XKeyEvent *event);
  
protected:
  void _GetFontName();
  void _GrabAltKey(int keysym);

  OXListBox *_fontNames, *_fontSizes, *_fontStyles;
  OXTextButton *_ok, *_cancel;
  OXGroupFrame *_gf;
  OXCompositeFrame *_cf;
  OXLabel *_sample;
  OXFont *_font;
  OString *_ret;
  char _fname[256];
};


#endif  // __OXFONTDIALOG_H
