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
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#ifdef __FreeBSD__
#  include <sys/cdio.h>
#else
#  if 0
#    include <linux/ucdrom.h>    // doesn't work for me (kernel 2.2.16)
#  else
#    include <sys/types.h>
#    include <linux/cdrom.h>
#  endif
#endif

#include <xclass/utils.h>

#include "OCd.h"

//----------------------------------------------------------------------

void OCd::Open() {
  int i;
#ifdef __FreeBSD__
  struct ioc_toc_header tochdr;
  struct ioc_read_toc_entry tocentry;
#else
  struct cdrom_tochdr tochdr;
  struct cdrom_tocentry tocentry;
#endif

  cdfile = open(devpath, O_RDONLY);
  if (cdfile == -1)
    // use a message box instead!
    FatalError("Could not open %s, exiting.", devpath);

  status = OMEDIUM_STOPPED;
#ifdef __FreeBSD__
  ioctl(cdfile, CDIOREADTOCHEADER, &tochdr);
#else
  ioctl(cdfile, CDROMREADTOCHDR, &tochdr);
#endif
  current_track = 1;

#ifdef __FreeBSD__
  for (i = tochdr.starting_track - 1;
       i <= (nb_tracks = tochdr.ending_track); i++) {
    tocentry.starting_track = (i == tochdr.ending_track) ? 0 : i + 1;
    tocentry.address_format = CD_MSF_FORMAT;
#else
  for (i = tochdr.cdth_trk0 - 1;
       i <= (nb_tracks = tochdr.cdth_trk1); i++) {
    tocentry.cdte_track  = (i == tochdr.cdth_trk1) ? CDROM_LEADOUT : i + 1;
    tocentry.cdte_format = CDROM_MSF;
#endif

#ifdef __FreeBSD__
    ioctl(cdfile, CDIOREADTOCENTRYS, &tocentry);
    track_db[i+1].info1 = ntohl(tocentry.data[i].addr.lba);
    track_db[i+1].info2 = ntohl(tocentry.data[i+1].addr.lba) - track_db[i+1].info1;
#else
    ioctl(cdfile, CDROMREADTOCENTRY, &tocentry);
    track_db[i+1].info1 =((tocentry.cdte_addr.msf.minute * 60) +
                          (tocentry.cdte_addr.msf.second)) * 75 +
                           tocentry.cdte_addr.msf.frame;
    track_db[i].info2 =((tocentry.cdte_addr.msf.minute * 60) +
                        (tocentry.cdte_addr.msf.second)) * 75 +
                         tocentry.cdte_addr.msf.frame;
#endif
  }

  for (i = 1; i <= nb_tracks; i++) {
    track_db[i].length = track_db[i].info2 - track_db[i].info1;
    sprintf(track_db[i].name, "Track %i", i);
  }

  ResetPlayList();
  current_index = 0;
}

void OCd::Close() {
#ifdef __FreeBSD__
  ioctl(cdfile, CDIOCEJECT);
#else
  ioctl(cdfile, CDROMEJECT);
#endif
  close(cdfile);
  status = OMEDIUM_TRAY_OPEN;
}

void OCd::ReadInfo() {
#ifdef __FreeBSD__
  struct ioc_read_subchannel subchnl;
  struct cd_sub_channel_info csci;
  struct ioc_vol volctrl;
#else
  struct cdrom_subchnl subchnl;
  struct cdrom_volctrl volctrl;
#endif

#ifdef __FreeBSD__
  ioctl(cdfile, CDIOCGETVOL, &volctrl);
  volume = volctrl.vol[0];
#else
  ioctl(cdfile, CDROMVOLREAD, &volctrl);
  volume = volctrl.channel0;
#endif

  if (status != OMEDIUM_TRAY_OPEN) {
#ifdef __FreeBSD__
    subchnl.address_format = CD_MSF_FORMAT;
    subchnl.data = &csci;
    subchnl.data_len = sizeof(csci);
    subchnl.data_format = CD_MEDIA_CATALOG;
    if (ioctl(cdfile, CDIOCREADSUBCHANNEL, &subchnl)) {
#else
    subchnl.cdsc_format = CDROM_MSF;
    if (ioctl(cdfile, CDROMSUBCHNL, &subchnl) == -1) {
#endif
      status = OMEDIUM_ERROR;
    } else {
#ifdef __FreeBSD__
      switch (csci.header.audio_status) {
#else
      switch (subchnl.cdsc_audiostatus) {
#endif

#ifdef __FreeBSD__
        case CD_AS_PLAY_ERROR:
#else
        case CDROM_AUDIO_ERROR:
#endif

#ifdef __FreeBSD__
        case CD_AS_AUDIO_INVALID:
#else
        case CDROM_AUDIO_INVALID:
#endif
          status = OMEDIUM_ERROR;
          break;

#ifdef __FreeBSD__
        case CD_AS_PLAY_IN_PROGRESS:
#else
        case CDROM_AUDIO_PLAY:
#endif
          status = OMEDIUM_PLAYING;
#ifdef __FreeBSD__
          current_track = subchnl.track;
          reltime = csci.what.position.reladdr.msf.minute * 4500 +
                    csci.what.position.reladdr.msf.second * 75 +
                    csci.what.position.reladdr.msf.frame;
#else
          current_track = subchnl.cdsc_trk;
          reltime = subchnl.cdsc_reladdr.msf.minute * 4500 +
                    subchnl.cdsc_reladdr.msf.second * 75 +
                    subchnl.cdsc_reladdr.msf.frame;
#endif
          break;

#ifdef __FreeBSD__
	case CD_AS_PLAY_PAUSED:
#else
        case CDROM_AUDIO_PAUSED:
#endif
          status = OMEDIUM_PAUSED;
#ifdef __FreeBSD__
          current_track = subchnl.track;
          reltime = csci.what.position.reladdr.msf.minute * 4500 +
                    csci.what.position.reladdr.msf.second * 75 +
                    csci.what.position.reladdr.msf.frame;
#else
          current_track = subchnl.cdsc_trk;
          reltime = subchnl.cdsc_reladdr.msf.minute * 4500 +
                    subchnl.cdsc_reladdr.msf.second * 75 +
                    subchnl.cdsc_reladdr.msf.frame;
#endif
          break;

#ifdef __FreeBSD__
        case CD_AS_NO_STATUS:
#else
        case CDROM_AUDIO_NO_STATUS:
#endif

#ifdef __FreeBSD__
        case CD_AS_PLAY_COMPLETED:
#else
        case CDROM_AUDIO_COMPLETED:
#endif
          if ((status == OMEDIUM_PLAYING) || (status == OMEDIUM_COMPLETED))
            status = OMEDIUM_COMPLETED;
          else
            status = OMEDIUM_STOPPED;
          break;
      }
    }
  }
}

void OCd::SetVolume(int vol) {
#ifdef __FreeBSD__
  struct ioc_vol volctrl;

  volctrl.vol[0] = vol;
  volctrl.vol[1] = vol;
  volctrl.vol[2] = 0;
  volctrl.vol[3] = 0;

  ioctl(cdfile, CDIOCSETVOL, &volctrl);
#else
  struct cdrom_volctrl volctrl;

  volctrl.channel0 = vol;
  volctrl.channel1 = vol;

  ioctl(cdfile, CDROMVOLCTRL, &volctrl);
#endif

}

void OCd::_Play(int track, int time) {
#ifdef __FreeBSD__
  struct ioc_play_msf msf;
#else
  struct cdrom_msf msf;
#endif

  int beg_time;

  current_track = track;
  if (time < 0) {
    beg_time = track_db[track].info2 + time;
    reltime = track_db[track].length + time;
  } else {
    beg_time = track_db[track].info1 + time;
    reltime = time;
  }

#ifdef __FreeBSD__
  msf.start_m = beg_time / 4500;
  msf.start_s = (beg_time / 75) % 60;
  msf.start_f = beg_time % 75;

  msf.end_m = track_db[track].info2 / 4500;
  msf.end_s = (track_db[track].info2 / 75) % 60;
  msf.end_f = track_db[track].info2 % 75;

  ioctl(cdfile, CDIOCPLAYMSF, &msf);
#else
  msf.cdmsf_min0 = beg_time / 4500;
  msf.cdmsf_sec0 = (beg_time / 75) % 60;
  msf.cdmsf_frame0 = beg_time % 75;
  
  msf.cdmsf_min1 = track_db[track].info2 / 4500;
  msf.cdmsf_sec1 = (track_db[track].info2 / 75) % 60;
  msf.cdmsf_frame1 = track_db[track].info2 % 75;

  ioctl(cdfile, CDROMPLAYMSF, &msf);
#endif

  status = OMEDIUM_PLAYING;
}

void OCd::Stop() {
#ifdef __FreeBSD__
  ioctl(cdfile, CDIOCSTOP);
#else
  ioctl(cdfile, CDROMSTOP);
#endif
  status = OMEDIUM_STOPPED;
  reltime = 0;
}

void OCd::Pause() {
  if (status == OMEDIUM_PAUSED) {
#ifdef __FreeBSD__
    ioctl(cdfile, CDIOCRESUME);
#else
    ioctl(cdfile, CDROMRESUME);
#endif
    status = OMEDIUM_PLAYING;
  } else if (status == OMEDIUM_PLAYING) {
#ifdef __FreeBSD__
    ioctl(cdfile, CDIOCPAUSE);
#else
    ioctl(cdfile, CDROMPAUSE);
#endif
    status = OMEDIUM_PAUSED;
  } else {
    Play();
#ifdef __FreeBSD__
    ioctl(cdfile, CDIOCPAUSE);
#else
    ioctl(cdfile, CDROMPAUSE);
#endif
    status = OMEDIUM_PAUSED;
  }
}
