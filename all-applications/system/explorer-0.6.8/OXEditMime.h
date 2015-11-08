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

#ifndef __OXEDITMIME_H
#define __OXEDITMIME_H


#include <xclass/utils.h>
#include <xclass/OXTransientFrame.h>
#include <xclass/OMimeTypes.h>

class OXLabel;
class OXIcon;
class OXTextEntry;
class OXTextButton;


//---------------------------------------------------------------------

class OXEditMimeDialog : public OXTransientFrame {
public:
  OXEditMimeDialog(const OXWindow *p, const OXWindow *main,
                   OMime *mime, int *retc = NULL,
                   unsigned long options = MAIN_FRAME | VERTICAL_FRAME);
  virtual ~OXEditMimeDialog();
  
  virtual int ProcessMessage(OMessage *msg);

  int UpdateMime();
  
protected:
  //=== dialog widgets and frames:
  
  //--- "OK Cancel Apply" buttons:
  OXTextButton *Ok, *Cancel, *SetIcon;
  OXHorizontalFrame *bframe;
  OLayoutHints *bly, *bfly;
  
  OXIcon *icon1;
  OXLabel *lbl1, *lbl2, *lbl3, *lbl4;
  OXTextEntry *txt1, *txt2, *txt3, *txt4;
  OXHorizontalFrame *Ihf, *hf1, *hf2, *hf3, *hf4;
  OLayoutHints *Ily, *lyl, *lyr, *lyln, *lv, *llv;

  int *_retc;
  OMime *EditingMime;
};

#endif   // __OXEDITMIME_H
