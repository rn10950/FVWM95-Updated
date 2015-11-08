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

#ifndef __OCD_H
#define __OCD_H

#ifdef __FreeBSD__
#include <sys/cdio.h>
#else
#include <linux/cdrom.h>
#endif

#include "OMedium.h"


#ifndef False
#define False 0
#endif

#ifndef True
#define True (!False)
#endif


//----------------------------------------------------------------------

class OCd : public OMedium {
public:
  OCd() {}
  ~OCd() {}

  void ReadInfo();
  void Stop();
  void Pause();
  void Open();
  void Close();
  int  HasVolumeControl() { return True; }
  int  GetVolume() { return volume; }
  void SetVolume(int vol);

protected:
  int  cdfile;
  int  volume;
  void _Play(int track, int time);
};
      

#endif  // __OCD_H
