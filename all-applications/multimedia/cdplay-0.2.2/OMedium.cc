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

#include <stdio.h>

#include "OMedium.h"


//----------------------------------------------------------------------

void OMedium::Open(char *path) {
  sprintf(devpath, "%s", path);
  Open();
}

int OMedium::Update() {
  int answer = 0;

  ReadInfo();
  if (((status == OMEDIUM_PLAYING) &&
       (playing_mode & OMEDIUM_MODE_INTRO) &&
       (reltime > intro_length * 75)) ||
      (status == OMEDIUM_COMPLETED)) {
    NextTrack();
    answer = 1;
  }

  return(answer);
}

int OMedium::TrackTimeElapsed() {
  return (reltime / 75);
}

int OMedium::TrackTimeRemaining() {
  return ((track_db[current_track].length - reltime) / 75);
}

int OMedium::DiscTimeRemaining() {
  return((track_db[nb_tracks].info2 -
          track_db[current_track].info1 -
          reltime) / 75);
}

int OMedium::DiscTime() {
  return (0);
}

int OMedium::TrackTime(int track) {
  return (track_db[track].length / 75);
}

void OMedium::Play() {
  if (status == OMEDIUM_PAUSED)
    Pause();
  else if (status != OMEDIUM_PLAYING)
    _Play(current_track, 0);
}

void OMedium::NextTrack() {
  int play = 1;

  if (current_index < play_list_length - 1) {
    current_track = play_list[++current_index];
  } else if (playing_mode & OMEDIUM_MODE_CONTINUOUS) {
    current_track = play_list[(current_index = 0)];
  } else {
    play = 0;
  }

  if (play) {
    if (status == OMEDIUM_PAUSED) {
      _Play(current_track, 0);
      Pause();
    } else if ((status == OMEDIUM_PLAYING) || (status == OMEDIUM_COMPLETED))
      _Play(current_track, 0);
  }
}

void OMedium::PreviousTrack() {

  if ((current_index > 0) && (reltime < 75))
    current_track = play_list[--current_index];

  if (status == OMEDIUM_PAUSED) {
    _Play(current_track, 0);
    Pause();
  } else if (status == OMEDIUM_PLAYING) {
    _Play(current_track, 0);
  }
}

int OMedium::SkipForward(int speed) {
  int newtrack, newtime, changes_track = 0;

  if ((status == OMEDIUM_PAUSED) ||
      (status == OMEDIUM_PLAYING)) {
    newtime = reltime + 75 * speed;
    newtrack = current_track;

    if (newtime > track_db[newtrack].length) {
      if (current_index < play_list_length - 1) {
        newtime -= track_db[newtrack].length;
        newtrack = play_list[++current_index];
        changes_track = 1;
      } else {
        Stop();
        return(0);
      }
    }

    if (status == OMEDIUM_PAUSED) {
      _Play(newtrack, newtime);
      Pause();
    } else {
      _Play(newtrack, newtime);
    }
  }
  return (changes_track);
}

int OMedium::SkipBackward(int speed) {
  int newtrack, newtime, changes_track = 0;

  if ((status == OMEDIUM_PAUSED) ||
      (status == OMEDIUM_PLAYING)) {
    newtime = reltime - 75 * speed;
    newtrack = current_track;

    if (newtime < 0) {
      if (current_index > 0) {
        newtrack = play_list[--current_index];
        newtime += track_db[newtrack].length;
        changes_track = 1;
      } else {
        Stop();
        return(0);
      }
    }

    if (status == OMEDIUM_PAUSED) {
      _Play(newtrack, newtime);
      Pause();
    } else {
      _Play(newtrack, newtime);
    }
  }

  return (changes_track);
}

void OMedium::ClearPlayList() {
  play_list_length = 0;
}

void OMedium::AddToPlayList(int track) {
  play_list[play_list_length++] = track;
}

void OMedium::ResetPlayList() {
  ClearPlayList();
  for(int i = 0; i < nb_tracks; i++)
    AddToPlayList(i + 1);
}
