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

#ifndef __OMEDIUM_H
#define __OMEDIUM_H

#define OMEDIUM_ERROR           0
#define OMEDIUM_PLAYING         1
#define OMEDIUM_PAUSED          2
#define OMEDIUM_STOPPED         3
#define OMEDIUM_COMPLETED       4
#define OMEDIUM_TRAY_OPEN       5

#define OMEDIUM_MODE_RANDOM     1
#define OMEDIUM_MODE_CONTINUOUS 2
#define OMEDIUM_MODE_INTRO      4


//----------------------------------------------------------------------

// The OMedium class is a superclass for all media (cda, audio and video
// files). It is meant to play the "tracks" of a medium which can thus be a
// cd-audio, or a collection of files, and to provide random, programmable,
// intro and continuous playing modes, and handle names and info for each
// track and each "disk".

class OXScrolledPlayList;

class Track {
public:
  char name[60];
  char filename[60];
  int  length;
  int  info1;
  int  info2;
};

class OMedium {
public:
  OMedium() {}
  virtual ~OMedium() {}

  Track track_db[100];
  int nb_tracks;

  int reltime;
  int current_track;
  int status;

  int playing_mode;
  int intro_length;

  int Update();

  int TrackTimeElapsed();
  int TrackTimeRemaining();
  int DiscTimeRemaining();
  int DiscTime();
  int TrackTime(int track);

  virtual int HasVolumeControl() { return 0; }
  virtual int GetVolume() { return 0; }
  virtual void SetVolume(int vol) {}

  void Play();
  virtual void Stop() {}
  virtual void Pause() {}
  void Open(char *path);
  virtual void Open() {}
  virtual void Close() {}
  void NextTrack();
  void PreviousTrack();
  int SkipForward(int speed);
  int SkipBackward(int speed);

  // The following methods are medium - dependant.

  virtual void ReadInfo() {}

  void ClearPlayList();
  void AddToPlayList(int track);
  void ResetPlayList();
  void PlayListChanged();

  friend class OXScrolledPlayList;

protected:
  char devpath[200];

  int playing_list[100];
  int play_list[100];
  int play_list_length;
  int current_index;

  virtual void _Play(int track, int time) {}
};
      
#endif  // __OMEDIUM_H
