/**************************************************************************

    This is file is part of xclass.
    Copyright (C) 2000-2006, Hector Peraza.

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

#ifndef __OXGLFRAME_H
#define __OXGLFRAME_H

#include <xclass/XCconfig.h>

#if defined(HAVE_GL_GLX_H) && defined(HAVE_GL_GL_H)
#include <GL/glx.h>
#include <GL/gl.h>
#endif

#include <xclass/OXFrame.h>

//----------------------------------------------------------------------

class OXGLFrame : public OXFrame {
public:
  OXGLFrame(const OXWindow *p, int w, int h, int *GLattrib = 0);
  virtual ~OXGLFrame();

  virtual int HandleMapNotify(XMapEvent *event);
  
  virtual void InitGL();
  
  void RestoreContext();
  
protected:
  virtual void _Resized();
  virtual void _DoRedraw();
                            
  bool _doubleBuf;
  XVisualInfo *_glvi;
#ifdef HAVE_GL_GLX_H
  GLXContext _glxc;
  static GLXContext _current_glxc;
#endif
};

#endif  // __OXGLFRAME_H
