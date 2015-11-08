/**************************************************************************
 
    This file is part of xcdiff, a front-end to the diff command.              
    Copyright (C) 1998-2002 Matzka Gerald, Hector Peraza.            

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

#ifndef __OXDIALOGS_H
#define __OXDIALOGS_H

#include <xclass/OXTransientFrame.h>
#include <xclass/OXColorSelect.h>


//----------------------------------------------------------------------

struct ODiffColors {
  ODiffColors();

  OColor normal_fg,  normal_bg,
         changed_fg, changed_bg,
         added_fg,   added_bg,
         deleted_fg, deleted_bg;
  int normal_default;
};

class OXDiffColorsDialog : public OXTransientFrame {
public:
  OXDiffColorsDialog(const OXWindow *p, const OXWindow *t,
                     ODiffColors *colors, int *retc = NULL);
  virtual ~OXDiffColorsDialog();
    
  virtual int ProcessMessage(OMessage *msg);

protected:
  OLayoutHints *bly, *bfly, *ly1, *ly2, *ly3;
  
  int *_retc;
  ODiffColors *_colors;
  OXColorSelect *nfg, *nbg, *cfg, *cbg, *afg, *abg, *dfg, *dbg;
};


#endif  // __OXDIALOGS_H
