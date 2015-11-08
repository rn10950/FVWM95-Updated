/**************************************************************************

    This file is part of xcpaint, a XPM pixmap editor.
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

#ifndef __OXPALETTEFRAME_H
#define __OXPALETTEFRAME_H

#include <xclass/OXClient.h>
#include <xclass/OXCompositeFrame.h>
#include <xclass/OXWidget.h>
#include <xclass/OMessage.h>
#include <xclass/OPicture.h>
#include <xclass/OString.h>
#include <xclass/utils.h>


#define MSG_PALETTE  (MSG_USERMSG+50)


//---------------------------------------------------------------------

class OXColorEntry : public OXFrame {
public:
  OXColorEntry(const OXWindow *p, int w, int h,
               unsigned int options = 0,
               unsigned long back = _defaultFrameBackground,
               int id = -1) :
    OXFrame(p, w, h, options, back) { _ID = id; }
  virtual ~OXColorEntry() {}

  int ID() const { return _ID; }

protected:
  int _ID;
};

class OXPaletteFrame : public OXCompositeFrame, public OXWidget {
public:
  OXPaletteFrame(const OXWindow *p, int w, int h,
                unsigned int options = 0,
                unsigned long back = _defaultFrameBackground);
  virtual ~OXPaletteFrame();

  virtual int HandleButton(XButtonEvent *event);

  void UpdateColors(const OColorTable *ct);
  const OPicture *GetBgndPic() const { return _tpic; }
  unsigned long GetColorByIndex(int ix);

protected:
  void _RemoveEntries();
  void _AddEntry(const OColorInfo *ci);
  void _AddBTreeEntries(const OColorInfo *root);

  int colors;
  const OPicture *_tpic;
};


#endif  // __OXPALETTEFRAME_H
