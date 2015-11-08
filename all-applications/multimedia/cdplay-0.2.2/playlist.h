/**************************************************************************

   This file is part of cdplay, a cdplayer for fvwm95.
   Copyright (C) 1997 Rodolphe Suescun.

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

#include <stdlib.h>
#include <stdio.h>

#include <xclass/OXClient.h>
#include <xclass/OXWindow.h>
#include <xclass/OXTransientFrame.h>
#include <xclass/OXTextEntry.h>
#include <xclass/OXLabel.h>
#include <xclass/OXTextButton.h>
#include <xclass/OXMsgBox.h>
#include <xclass/OString.h>

#include "OXPlayList.h"
#include "OMedium.h"

#define B_ADD              0
#define B_REMOVE           1
#define B_CLEAR            2
#define B_RESET            3
#define B_OK               4
#define B_CANCEL           5


//----------------------------------------------------------------------

class OXEditPlayListFrame : public OXTransientFrame {
public:
  OXEditPlayListFrame(const OXWindow *p, const OXWindow *main, int w, int h,
                      OMedium * medium,
                      unsigned long options = MAIN_FRAME | VERTICAL_FRAME);
  virtual ~OXEditPlayListFrame();

  virtual int ProcessMessage(OMessage *msg);

protected:
  OMedium *omedium;

  OXCompositeFrame *driveframe, *authorframe, *titleframe, *tracktitleframe;
  OXLabel *drivelabel, *authorlabel, *titlelabel, *tracktitlelabel;
  OXTextEntry *drive, *author, *title, *tracktitle;
  OTextBuffer *drive_b, *author_b, *title_b, *tracktitle_b;
  OXCompositeFrame *playlistzone, *playlistbuttons, *bottombuttons;
  OXTextButton *addbutton, *removebutton, *clearbutton, *initbutton;
  OXTextButton *settitlebutton, *okbutton, *cancelbutton;
  OXScrolledPlayList *playlist, *playlistref;
};
