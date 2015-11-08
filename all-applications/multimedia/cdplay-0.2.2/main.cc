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

#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <limits.h>

#include <xclass/OXButton.h>
#include <xclass/OXMsgBox.h>
#include <xclass/OXFileDialog.h>

#include "main.h"
#include "OCd.h"

#include "icons/volume.xpm"
#include "icons/xcd-continue.xpm"
#include "icons/xcd-diskremain.xpm"
#include "icons/xcd-eject.xpm"
#include "icons/xcd-forward.xpm"
#include "icons/xcd-intro.xpm"
#include "icons/xcd-next.xpm"
#include "icons/xcd-pause.xpm"
#include "icons/xcd-play.xpm"
#include "icons/xcd-previous.xpm"
#include "icons/xcd-random.xpm"
#include "icons/xcd-rewind.xpm"
#include "icons/xcd-select.xpm"
#include "icons/xcd-stop.xpm"
#include "icons/xcd-trackelapsed.xpm"
#include "icons/xcd-trackremain.xpm"


const OPicture *volicon;

char *filetypes[] = { "All files",  "*",
                      NULL,         NULL };

SToolBarData toolbar[] = {
  { "xcd-select.xpm",       xcd_select,       "Edit Play List",       BUTTON_NORMAL,   M_MODIF_SELECT,    NULL },
  { "",                     NULL,             "",                     0,               -10,               NULL },
  { "xcd-trackelapsed.xpm", xcd_trackelapsed, "Track Time Elapsed",   BUTTON_STAYDOWN, M_TRACK_ELAPSED,   NULL },
  { "xcd-trackremain.xpm",  xcd_trackremain,  "Track Time Remaining", BUTTON_STAYDOWN, M_TRACK_REMAINING, NULL },
  { "xcd-diskremain.xpm",   xcd_diskremain,   "Disc Time Remaining",  BUTTON_STAYDOWN, M_DISK_REMAINING,  NULL },
  { "",                     NULL,             "",                     0,               -10,               NULL },
  { "xcd-random.xpm",       xcd_random,       "Random Track Order",   BUTTON_ONOFF,    M_RANDOM,          NULL },
  { "xcd-continue.xpm",     xcd_continue,     "Continuous Play",      BUTTON_ONOFF,    M_CONTINUE,        NULL },
  { "xcd-intro.xpm",        xcd_intro,        "Intro Play",           BUTTON_ONOFF,    M_INTRO,           NULL },
  { NULL,                   NULL,             NULL,                   0,               0,                 NULL }
};

SToolBarData mainbar1[] = {
  { "xcd-play.xpm",  xcd_play,  "Play",  BUTTON_NORMAL, CDPLAY,  NULL },
  { "xcd-pause.xpm", xcd_pause, "Pause", BUTTON_NORMAL, CDPAUSE, NULL },
  { "xcd-stop.xpm",  xcd_stop,  "Stop",  BUTTON_NORMAL, CDSTOP,  NULL },
  { NULL,            NULL,      NULL,    0,             0,       NULL }
};

SToolBarData mainbar2[] = {
  { "xcd-previous.xpm", xcd_previous, "Previous Track", BUTTON_NORMAL, CDPREVIOUS, NULL },
  { "xcd-rewind.xpm",   xcd_rewind,   "Skip Backwards", BUTTON_NORMAL, CDBACKWARD, NULL },
  { "xcd-forward.xpm",  xcd_forward,  "Skip Forwards",  BUTTON_NORMAL, CDFORWARD,  NULL },
  { "xcd-next.xpm",     xcd_next,     "Next Track",     BUTTON_NORMAL, CDNEXT,     NULL },
  { "xcd-eject.xpm",    xcd_eject,    "Eject",          BUTTON_NORMAL, CDEJECT,    NULL },
  { NULL,               NULL,         NULL,             BUTTON_NORMAL, 0,          NULL }
};

int speed, skip_counter;
int skip_state = 0;

OXClient *clientX;
OXCDPlayerMainFrame *mainWindow;
OXEditPlayListFrame *editWindow;

//#define DISABLE_BUTTONS

//----------------------------------------------------------------------

OLayoutHints *toprightlayout =
    new OLayoutHints(LHINTS_TOP | LHINTS_RIGHT,1,1,1,1);

OLayoutHints *topleftlayout =
    new OLayoutHints(LHINTS_TOP | LHINTS_LEFT,1,1,1,1);

OLayoutHints *centerleftlayout =
    new OLayoutHints(LHINTS_CENTER_Y | LHINTS_LEFT,1,1,1,1);

OLayoutHints *centerrightlayout =
    new OLayoutHints(LHINTS_CENTER_Y | LHINTS_RIGHT,1,1,1,1);

OLayoutHints *topexpandxlayout =
    new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X,1,1,1,1);

OLayoutHints *topcenterxlayout =
    new OLayoutHints(LHINTS_TOP | LHINTS_CENTER_X,1,1,1,1);

OLayoutHints *centerxcenterylayout =
    new OLayoutHints(LHINTS_CENTER_Y | LHINTS_CENTER_X,1,1,1,1);

OLayoutHints *expandxexpandylayout =
    new OLayoutHints(LHINTS_EXPAND_Y | LHINTS_EXPAND_X,1,1,1,1);

//----------------------------------------------------------------------

int main(int argc, char **argv) {

  clientX = new OXClient;

  volicon = clientX->GetPicture("volume.xpm", xcd_volume);

  mainWindow = new OXCDPlayerMainFrame(clientX->GetRoot(), 10, 10);
  mainWindow->MapWindow();

  clientX->Run();
}

OXCDPlayerMainFrame::OXCDPlayerMainFrame(const OXWindow *p, int w, int h) :
  OXMainFrame(p, w, h) {
  char tmp[BUFSIZ];


  //--- Prepare the font for the Display

  font = _client->GetFont("-adobe-utopia-medium-r-normal-*-24-*-*-*-*-*-iso8859-1");

  if (font == NULL)
    FatalError("Unable to find font\n");

  //--- Open the medium and set the defaults

  omedium = new OCd();

  omedium->playing_mode = 0;
  omedium->intro_length = 10;

  omedium->Open("/dev/cdrom");

  blink = 0;

  //--- The GUI

  _menuBarLayout = new OLayoutHints(LHINTS_TOP | LHINTS_LEFT | LHINTS_EXPAND_X, 
                                    0, 0, 1, 1);
  _menuBarItemLayout = new OLayoutHints(LHINTS_TOP | LHINTS_LEFT,
                                    0, 4, 0, 0);

  _menuDisk = new OXPopupMenu(_client->GetRoot());
  _menuDisk->AddEntry(new OHotString("&Edit Play List..."), M_MODIF_SELECT);
  _menuDisk->AddSeparator();
  _menuDisk->AddEntry(new OHotString("E&xit"), M_EXIT);

  _menuDisplay = new OXPopupMenu(_client->GetRoot());
  _menuDisplay->AddEntry(new OHotString("&Toolbar"), M_VIEW_TOOLBAR);
  //_menuDisplay->AddEntry(new OHotString("&Disk/Track info."), M_VIEW_INFO);
  _menuDisplay->AddEntry(new OHotString("&Status bar"), M_VIEW_STATUSBAR);
  _menuDisplay->AddSeparator();
  _menuDisplay->AddEntry(new OHotString("Track Time &Elapsed"), M_TRACK_ELAPSED);
  _menuDisplay->AddEntry(new OHotString("Track Time &Remaining"), M_TRACK_REMAINING);
  _menuDisplay->AddEntry(new OHotString("Dis&c Time Remaining"), M_DISK_REMAINING);
  _menuDisplay->AddSeparator();
  _menuDisplay->AddEntry(new OHotString("&Volume control"), M_VOL_CONTROL);
  _menuDisplay->CheckEntry(M_VIEW_TOOLBAR);
  _menuDisplay->CheckEntry(M_VIEW_STATUSBAR);

  _menuOptions = new OXPopupMenu(_client->GetRoot());
  _menuOptions->AddEntry(new OHotString("&Random Order"), M_RANDOM);
  _menuOptions->AddEntry(new OHotString("&Continuous Play"), M_CONTINUE);
  _menuOptions->AddEntry(new OHotString("&Intro Play"), M_INTRO);
  _menuOptions->AddSeparator();
  _menuOptions->AddEntry(new OHotString("&Preferences..."), M_PREFERENCES);

  _menuHelp = new OXPopupMenu(_client->GetRoot());
  _menuHelp->AddEntry(new OHotString("&Help Topics..."), M_HELP_CONTENTS);
  _menuHelp->AddSeparator();
  _menuHelp->AddEntry(new OHotString("&About CD Player..."), M_HELP_ABOUT);
  _menuHelp->DisableEntry(M_HELP_CONTENTS);
  _menuHelp->DisableEntry(M_HELP_SEARCH);
  _menuHelp->DisableEntry(M_HELP_ABOUT);

  _menuDisk->Associate(this);
  _menuDisplay->Associate(this);
  _menuOptions->Associate(this);
  _menuHelp->Associate(this);

  _menuBar = new OXMenuBar(this, 1, 1, HORIZONTAL_FRAME);
  _menuBar->AddPopup(new OHotString("&Disk"), _menuDisk, _menuBarItemLayout);
  _menuBar->AddPopup(new OHotString("&View"), _menuDisplay, _menuBarItemLayout);
  _menuBar->AddPopup(new OHotString("&Options"), _menuOptions, _menuBarItemLayout);
  _menuBar->AddPopup(new OHotString("&Help"), _menuHelp, _menuBarItemLayout);

  AddFrame(_menuBar, _menuBarLayout);

  _toolBarSep = new OXHorizontal3dLine(this);

  _toolBar = new OXToolBar(this);
  _toolBar->Associate(this);
  _toolBar->AddButtons(toolbar);

  _volume = new OXHSlider(_toolBar, 80, SLIDER_1 | SCALE_DOWNRIGHT, 5);
  _volume->Associate(this);
  _volume->SetScale(20);
  _volume->SetRange(0, 100);
  _volume->SetTip("CD Volume");
  _toolBar->AddFrame(_volume, new OLayoutHints(LHINTS_RIGHT | LHINTS_CENTER_Y));
  _toolBar->AddFrame(new OXIcon(_toolBar, volicon, 16, 16),
                     new OLayoutHints(LHINTS_CENTER_Y | LHINTS_RIGHT, 20, 0, 0, 10));

  AddFrame(_toolBarSep, topexpandxlayout);
  AddFrame(_toolBar, topexpandxlayout);

  _main = new OXCompositeFrame(this, 60, 20, HORIZONTAL_FRAME);

  _display = new OXCompositeFrame(_main, 180, 44,
                                  /*FIXED_WIDTH | FIXED_HEIGHT | */
  				  SUNKEN_FRAME);
  _display->SetBackgroundColor(_blackPixel);
  TimeLabel = new OXLabel(_display, new OString("fOX CD Player"));
  TimeLabel->SetBackgroundColor(_blackPixel);
  TimeLabel->SetTextColor(_client->GetColorByName("cyan"));
  TimeLabel->SetFont(font);
  _display->AddFrame(TimeLabel, new OLayoutHints(LHINTS_CENTER_X |
  						LHINTS_CENTER_Y, 5, 5, 7, 7));
  _main->AddFrame(_display, new OLayoutHints(LHINTS_EXPAND_X | LHINTS_CENTER_Y, 0, 3, 0, 0));

  _buttons = new OXCompositeFrame(_main, 60, 20, VERTICAL_FRAME);
  _buttons1 = new OXToolBar(_buttons);
  _buttons1->Associate(this);
  _buttons1->AddButtons(mainbar1);
  _buttons->AddFrame(_buttons1, new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X));

#ifdef DISABLE_BUTTONS
  mainbar1[1].button->Disable();
  mainbar1[2].button->Disable();
#endif

  _buttons2 = new OXToolBar(_buttons, 60, 20, HORIZONTAL_FRAME);
  _buttons2->Associate(this);
  _buttons2->AddButtons(mainbar2);
  _buttons->AddFrame(_buttons2, new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X));
  _main->AddFrame(_buttons, new OLayoutHints(LHINTS_CENTER_Y,
                                     0, 1, 0, 0));

  AddFrame(_main, new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X |
					LHINTS_EXPAND_Y));

  sllayout = new OLayoutHints(LHINTS_LEFT, 3, 1, 1, 1);
  lsblayout = new OLayoutHints(LHINTS_EXPAND_X, 0, 1, 0, 0);
  rsblayout = new OLayoutHints(LHINTS_EXPAND_X, 2, 0, 0, 0);

  _status = new OXCompositeFrame(this, 60, 20, HORIZONTAL_FRAME);
  _lstat = new OXCompositeFrame(_status, 60, 10, HORIZONTAL_FRAME | SUNKEN_FRAME);
  _lstatus = new OXLabel(_lstat, new OString(""));
  _lstat->AddFrame(_lstatus, sllayout);
  _status->AddFrame(_lstat, lsblayout);
  _rstat = new OXCompositeFrame(_status, 60, 10, HORIZONTAL_FRAME|SUNKEN_FRAME);
  _rstatus = new OXLabel(_rstat, new OString(""));
  _rstat->AddFrame(_rstatus, sllayout);
  _status->AddFrame(_rstat, rsblayout);
  AddFrame(_status, new OLayoutHints(LHINTS_EXPAND_X, 0, 0, 3, 0));

  SetViewMode(M_TRACK_ELAPSED);

  SetWindowName("fOX CD Player");
  SetClassHints("CDPlay", "CDPlay");

  SetMWMHints(MWM_DECOR_ALL | MWM_DECOR_MAXIMIZE | MWM_DECOR_MENU,
	      MWM_FUNC_ALL | MWM_FUNC_MAXIMIZE | MWM_FUNC_RESIZE,
	      MWM_INPUT_MODELESS);

  omedium->ReadInfo();
  RedrawStatusBar();

  timer = new OTimer(this, 1000);

  MapSubwindows();
  Resize(GetDefaultSize());

  SetWMSize(_w, _h);
  SetWMSizeHints(_w, _h, _w, _h, 0, 0);
}

OXCDPlayerMainFrame::~OXCDPlayerMainFrame() {
  delete _menuDisk;
  delete _menuDisplay;
  delete _menuOptions;
  delete _menuHelp;
}

void OXCDPlayerMainFrame::SetViewMode(int mode) {
  _menuDisplay->RCheckEntry(mode, M_TRACK_ELAPSED, M_DISK_REMAINING);
  toolbar[2].button->SetState(mode == M_TRACK_ELAPSED ? BUTTON_ENGAGED : BUTTON_UP);
  toolbar[3].button->SetState(mode == M_TRACK_REMAINING ? BUTTON_ENGAGED : BUTTON_UP);
  toolbar[4].button->SetState(mode == M_DISK_REMAINING ? BUTTON_ENGAGED : BUTTON_UP);
  ViewMode = mode;
}

void OXCDPlayerMainFrame::SetPlayMode(int mode) {
  int bnum;

  switch (mode) {
    case M_RANDOM:   bnum = 6; break;
    case M_CONTINUE: bnum = 7; break;
    case M_INTRO:    bnum = 8; break;
  }

  if (_menuOptions->IsEntryChecked(mode)) {
    toolbar[bnum].button->SetState(BUTTON_UP);
    _menuOptions->UnCheckEntry(mode);
  } else {
    toolbar[bnum].button->SetState(BUTTON_ENGAGED);
    _menuOptions->CheckEntry(mode);
  }

  omedium->playing_mode = 
    ((toolbar[6].button->GetState() != BUTTON_UP) ? 1 : 0) |
    ((toolbar[7].button->GetState() != BUTTON_UP) ? 2 : 0) |
    ((toolbar[8].button->GetState() != BUTTON_UP) ? 4 : 0);
}


int OXCDPlayerMainFrame::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;

  switch (msg->type) {
    case MSG_HSLIDER:
      omedium->SetVolume(_volume->GetPosition());
      break;

    case MSG_MENU:
    case MSG_BUTTON:
      switch(msg->action) {
        case MSG_CLICK:
          switch(wmsg->id) {
            case CDPLAY:
              omedium->Play();
              RedrawDisplay();
#ifdef DISABLE_BUTTONS
              mainbar1[0].button->Disable();
              mainbar1[1].button->Enable();
              mainbar1[2].button->Enable();
#endif
              break;

            case CDPAUSE:
              omedium->Pause();
              blink = 1;
              RedrawDisplay();
#ifdef DISABLE_BUTTONS
              if (omedium->status == OMEDIUM_PAUSED) {
                mainbar1[0].button->Enable();
                mainbar1[1].button->Enable();
                mainbar1[2].button->Enable();
              } else {
                mainbar1[0].button->Disable();
                mainbar1[1].button->Enable();
                mainbar1[2].button->Enable();
              }
#endif
              break;

            case CDSTOP:
              omedium->Stop();
              RedrawDisplay();
#ifdef DISABLE_BUTTONS
              mainbar1[0].button->Enable();
              mainbar1[1].button->Disable();
              mainbar1[2].button->Disable();
#endif
              break;

            case CDEJECT:
              if (omedium->status == OMEDIUM_TRAY_OPEN)
                omedium->Open("/dev/cdrom");
              else
                omedium->Close();
              RedrawDisplay();
              RedrawStatusBar();
              if (omedium->status == OMEDIUM_TRAY_OPEN) {
                mainbar1[0].button->Disable();
                mainbar1[1].button->Disable();
                mainbar1[2].button->Disable();
                mainbar2[0].button->Disable();
                mainbar2[1].button->Disable();
                mainbar2[2].button->Disable();
                mainbar2[3].button->Disable();
              } else {
                mainbar1[0].button->Enable();
                mainbar1[1].button->Enable();
                mainbar1[2].button->Enable();
                mainbar2[0].button->Enable();
                mainbar2[1].button->Enable();
                mainbar2[2].button->Enable();
                mainbar2[3].button->Enable();
              }
              break;

            case CDPREVIOUS:
              omedium->PreviousTrack();
              RedrawDisplay();
              RedrawStatusBar();
              break;

            case CDNEXT:
              omedium->NextTrack();
              RedrawDisplay();
              RedrawStatusBar();
              break;

            case M_MODIF_SELECT:
              editWindow = new OXEditPlayListFrame(clientX->GetRoot(), this,
                                                   400, 200, omedium);
              editWindow->MapWindow();
              clientX->WaitFor(editWindow);
              break;

            case M_EXIT:
              delete _client;
              exit(0);
              break;

            case M_VIEW_TOOLBAR:
              if (_toolBar->IsVisible()) {
                HideFrame(_toolBar);
                HideFrame(_toolBarSep);
                _menuDisplay->UnCheckEntry(M_VIEW_TOOLBAR);
              } else {
                ShowFrame(_toolBar);
                ShowFrame(_toolBarSep);
                _menuDisplay->CheckEntry(M_VIEW_TOOLBAR);
              }
              Resize(GetDefaultSize());
              break;

            case M_VIEW_STATUSBAR:
              if (_status->IsVisible()) {
                HideFrame(_status);
                _menuDisplay->UnCheckEntry(M_VIEW_STATUSBAR);
              } else {
                ShowFrame(_status);
                _menuDisplay->CheckEntry(M_VIEW_STATUSBAR);
              }
              Resize(GetDefaultSize());
              break;

            case M_TRACK_ELAPSED:
            case M_TRACK_REMAINING:
            case M_DISK_REMAINING:
              SetViewMode(wmsg->id);
              RedrawDisplay();
              break;

            case M_CONTINUE:
            case M_RANDOM:
            case M_INTRO:
              SetPlayMode(wmsg->id);
              break;

            default:
              break;

          } // switch(id)
          break;

        default:
          break;

      } // switch(action)
      break;

    default:
      break;

  } // switch(type)

  return True;
}

int OXCDPlayerMainFrame::HandleTimer(OTimer *t) {
  if (t != timer) return False;

  blink = 1-blink;

  RedrawDisplay();

  delete timer;
  if (mainbar2[1].button->GetState() == BUTTON_ENGAGED) {
    if (skip_state != 1) {
      speed = 1;
      skip_state = 1;
      skip_counter = 0;
    } else {
      if ((skip_counter++) == 6) speed = 3;
    }
    if (omedium->SkipBackward(speed))
      RedrawStatusBar();
    timer = new OTimer(this, 250);
  } else if (mainbar2[2].button->GetState() == BUTTON_ENGAGED) {
    if (skip_state != 2) {
      speed = 1;
      skip_state = 2;
      skip_counter = 0;
    } else {
      if ((skip_counter++) == 6) speed = 3;
    }
    if (omedium->SkipForward(speed))
      RedrawStatusBar();
    timer = new OTimer(this, 250);
  } else {
    timer = new OTimer(this, 1000);

    skip_state = 0;
    if (omedium->Update())
      RedrawStatusBar();
    _volume->SetPosition(omedium->GetVolume());
  }

  return True;
}

void OXCDPlayerMainFrame::RedrawDisplay() {
  int time;
  char str[40];

  if ((omedium->status == OMEDIUM_PLAYING) ||
             (omedium->status == OMEDIUM_PAUSED)) {
    if ((omedium->status == OMEDIUM_PAUSED) && (blink == 0)) {
      sprintf(str, "                       ");
    } else {
      switch (ViewMode) {
        case M_TRACK_ELAPSED :
          time = omedium->TrackTimeElapsed();
          sprintf(str,"  [%02u] %02u:%02u  ",
		  omedium->current_track, time/60, time%60);
          break;
        case M_TRACK_REMAINING :
          time = omedium->TrackTimeRemaining();
          sprintf(str,"  [%02u]<%02u:%02u>  ",
		  omedium->current_track, time/60, time%60);
          break;
        case M_DISK_REMAINING :
          time = omedium->DiscTimeRemaining();
          sprintf(str,"   [--]<%02u:%02u>   ",
		  time/60, time%60);
          break;
      }
    }
  } else if (omedium->status == OMEDIUM_TRAY_OPEN) {
      sprintf(str,"    Tray Open    ", omedium->current_track);
  } else if (omedium->status == OMEDIUM_ERROR) {
      sprintf(str,"    No Disc    ", omedium->current_track);
  } else {
      sprintf(str,"    [%02u] --:--    ", omedium->current_track);
  }
  TimeLabel->SetText(new OString(str));
  _display->Layout();
}

int OXCDPlayerMainFrame::RedrawStatusBar() {
  char str[40];
  int  time;

  if ((omedium->status == OMEDIUM_ERROR) ||
      (omedium->status == OMEDIUM_TRAY_OPEN)) {
    sprintf(str, "Total play: --:-- m:s");
    _lstatus->SetText(new OString(str));

    sprintf(str, "Track: --:-- m:s");
    _rstatus->SetText(new OString(str));
  } else {
    time = omedium->DiscTime();
    sprintf(str, "Total play: %02u:%02u m:s", time/60, time%60);
    _lstatus->SetText(new OString(str));

    time = omedium->TrackTime(omedium->current_track);
    sprintf(str, "Track: %02u:%02u m:s", time/60, time%60);
    _rstatus->SetText(new OString(str));
  }

  Layout();
}
