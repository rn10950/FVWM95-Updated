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
#include <string.h>
#include <limits.h>

#include "playlist.h"

extern OLayoutHints *centerleftlayout;
extern OLayoutHints *toprightlayout;
extern OLayoutHints *topleftlayout;
extern OLayoutHints *topexpandxlayout;
extern OLayoutHints *expandxexpandylayout;

OXEditPlayListFrame::OXEditPlayListFrame(const OXWindow *p,
                       const OXWindow *main, int w, int h, OMedium *medium,
                       unsigned long options) :
  OXTransientFrame(p, main, w, h, options) {

  omedium = medium;

/*
  driveframe = new OXCompositeFrame(this, 10, 10, HORIZONTAL_FRAME);
  drivelabel = new OXLabel(driveframe, new OString("Device:"));
  driveframe->AddFrame(drivelabel, centerleftlayout);
  drive = new OXTextEntry(driveframe, drive_b = new OTextBuffer(100));
  drive->SetWidth(300);
  drive->ChangeOptions(FIXED_WIDTH | SUNKEN_FRAME | DOUBLE_BORDER);
  driveframe->AddFrame(drive, toprightlayout);
*/

  authorframe = new OXCompositeFrame(this, 10, 10, HORIZONTAL_FRAME);
  authorlabel = new OXLabel(authorframe, new OString("Author:"));
  authorframe->AddFrame(authorlabel, centerleftlayout);
  author = new OXTextEntry(authorframe, author_b = new OTextBuffer(100));
  author->SetWidth(300);
  author->ChangeOptions(FIXED_WIDTH | SUNKEN_FRAME | DOUBLE_BORDER);
  authorframe->AddFrame(author, toprightlayout);

  titleframe = new OXCompositeFrame(this, 10, 10, HORIZONTAL_FRAME);
  titlelabel = new OXLabel(titleframe, new OString("Title:"));
  titleframe->AddFrame(titlelabel, centerleftlayout);
  title = new OXTextEntry(titleframe, title_b = new OTextBuffer(100));
  title->SetWidth(300);
  title->ChangeOptions(FIXED_WIDTH | SUNKEN_FRAME | DOUBLE_BORDER);
  titleframe->AddFrame(title, toprightlayout);

  //AddFrame(driveframe, topexpandxlayout);
  AddFrame(authorframe, topexpandxlayout);
  AddFrame(titleframe, topexpandxlayout); 

  playlistzone = new OXCompositeFrame(this, 10, 10, HORIZONTAL_FRAME);
  playlistbuttons = new OXCompositeFrame(playlistzone, 10, 10, VERTICAL_FRAME);

  addbutton = new OXTextButton(playlistbuttons,
                               new OHotString("  <- Add  "), B_ADD);
  addbutton->Associate(this);
  removebutton = new OXTextButton(playlistbuttons,
                                  new OHotString("  Remove ->  "), B_REMOVE);
  removebutton->Associate(this);
  clearbutton = new OXTextButton(playlistbuttons,
                                 new OHotString("  Clear List  "), B_CLEAR);
  clearbutton->Associate(this);
  initbutton = new OXTextButton(playlistbuttons,
                                new OHotString("  Initialize  "), B_RESET);
  initbutton->Associate(this);
  playlistbuttons->AddFrame(addbutton, topexpandxlayout);
  playlistbuttons->AddFrame(removebutton, topexpandxlayout);
  playlistbuttons->AddFrame(clearbutton, topexpandxlayout);
  playlistbuttons->AddFrame(initbutton, topexpandxlayout);

  playlist = new OXScrolledPlayList(playlistzone, 100, 100,
                                    omedium, SUNKEN_FRAME | DOUBLE_BORDER);
  playlistref = new OXScrolledPlayList(playlistzone, 100, 100,
                                       omedium, SUNKEN_FRAME | DOUBLE_BORDER);
  playlist->Clear();
  playlistref->Clear();
  for(int i=1;i <= omedium->nb_tracks; i++) {
    playlistref->AddTrack(i);
  }
  playlist->GetPlayListFromMedium();

  playlistzone->AddFrame(playlist, expandxexpandylayout);
  playlistzone->AddFrame(playlistbuttons, centerleftlayout);
  playlistzone->AddFrame(playlistref, expandxexpandylayout);
  AddFrame(playlistzone, expandxexpandylayout);

  tracktitleframe = new OXCompositeFrame(this, 10, 10, HORIZONTAL_FRAME);
  tracktitlelabel = new OXLabel(tracktitleframe, new OString("Track x   "));
  tracktitle = new OXTextEntry(tracktitleframe, 
                               tracktitle_b = new OTextBuffer(100));
  tracktitle->Associate(this);
  settitlebutton = new OXTextButton(tracktitleframe,
                              new OHotString(" Set Title "), B_OK);
  settitlebutton->Associate(this);
  tracktitleframe->AddFrame(tracktitlelabel, centerleftlayout);
  tracktitleframe->AddFrame(tracktitle, topexpandxlayout);
  tracktitleframe->AddFrame(settitlebutton, topleftlayout);
  AddFrame(tracktitleframe, topexpandxlayout);

  bottombuttons = new OXCompositeFrame(this, 10, 10, HORIZONTAL_FRAME);
  okbutton = new OXTextButton(bottombuttons,
                              new OHotString("           OK           "), B_OK);
  okbutton->Associate(this);
  cancelbutton = new OXTextButton(bottombuttons,
                                new OHotString("  Cancel  "), B_CANCEL);
  cancelbutton->Associate(this);
  bottombuttons->AddFrame(okbutton, topexpandxlayout);
  bottombuttons->AddFrame(cancelbutton, topexpandxlayout);
  AddFrame(bottombuttons, topexpandxlayout);

  MapSubwindows();
  Resize(400, 300);
  Layout();

  CenterOnParent();

  SetWMSize(_w, _h);
  SetWMSizeHints(_w, _h, 640, 480, 1, 1);

  SetWindowName("Play List");
}

OXEditPlayListFrame::~OXEditPlayListFrame() {
}

int OXEditPlayListFrame::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;

  switch(msg->action) {
    case MSG_CLICK:
      switch(msg->type) {
        case MSG_BUTTON:
          switch(wmsg->id) {
             case B_ADD:
               for(int i = playlistref->GetFirstSelectedTrack();
                   i != -1;
                   i = playlistref->GetFirstSelectedTrack()) {
                 if (i != -1) playlist->AddTrack(i);
               }
               break;
             case B_REMOVE:
               playlist->RemoveSelection();
               break;
             case B_CLEAR:
               playlist->Clear();
               break;
             case B_RESET:
               playlist->Clear();
               for(int i=1;i <= omedium->nb_tracks; i++) {
                 playlist->AddTrack(i);
               }
               break;
             case B_OK:
               playlist->SavePlayListToMedium();
               CloseWindow();
               break;
             case B_CANCEL:
               CloseWindow();
               break;
            default:
               break;
          }
          break;
      }
      break;
  }
  return True;
}
