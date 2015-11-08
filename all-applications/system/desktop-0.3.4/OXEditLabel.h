/**************************************************************************

    This file is part of xclass.
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

#ifndef __OXEDITLABEL_H
#define __OXEDITLABEL_H

#include <X11/Xlib.h>

#include <xclass/utils.h>
#include <xclass/OXCompositeFrame.h>
#include <xclass/OXTextEntry.h>
#include <xclass/OString.h>


//----------------------------------------------------------------------

class OXEditLabel : public OXCompositeFrame {
public:
  OXEditLabel(const OXWindow *p, OString *text);
  virtual ~OXEditLabel();

  virtual int ProcessMessage(OMessage *msg);

  virtual int HandleButton(XButtonEvent *event);
  virtual int HandleDoubleClick(XButtonEvent *event);
  virtual int HandleMotion(XMotionEvent *event);
  virtual int HandleKey(XKeyEvent *event);

  virtual void DrawBorder();
  virtual void Reconfig();

  void SetText(OString *text);
  void Show(int x, int y, int w, int h, bool autosize = false);
  void Hide();
  const char *GetString() const { return _te->GetString(); }
  
protected:
  OXTextEntry *_te;
  OLayoutHints *_ll;
  
  unsigned long _bg, _fg;
  bool _bdown, _autosize;
  OString *_oldName;
};


#endif  // __OXEDITLABEL_H
