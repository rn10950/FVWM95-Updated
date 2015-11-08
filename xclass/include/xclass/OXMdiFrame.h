/**************************************************************************
 
    This file is part of OXMdi, an extension to the xclass toolkit.
    Copyright (C) 1998-2002 by Harald Radke, Hector Peraza.
 
    This application is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
 
    This application is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.
 
    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 
**************************************************************************/

#ifndef __OXMDIFRAME_H
#define __OXMDIFRAME_H

#include <xclass/OXCompositeFrame.h>

class OPicture;
class OXMdiMainFrame;
class OXMdiDecorFrame;


//----------------------------------------------------------------------

class OXMdiFrame : public OXCompositeFrame {
public:
  OXMdiFrame(OXMdiMainFrame *main, int w, int h,
             unsigned int options = 0,
             unsigned long back =_defaultFrameBackground);
  virtual ~OXMdiFrame();

  virtual void Move(int x, int y);
  virtual int CloseWindow();
  virtual int Help() { return False; }

  virtual void SetMdiHints(unsigned long mdihints);
  unsigned long GetMdiHints() const { return _mdiHints; }

  void SetWindowName(const char *name);
  void SetWindowIcon(const OPicture *pic);
  const char *GetWindowName();
  const OPicture *GetWindowIcon();

  friend class OXMdiMainFrame;
  friend class OXMdiDecorFrame;

protected:
  OXMdiMainFrame *_main;
  unsigned long _mdiHints;
};


#endif  // __OXMDIFRAME_H
