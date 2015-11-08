/**************************************************************************

    This is file is part of xclass.
    Copyright (C) 2000, 2001, Hector Peraza.

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

#ifndef __OXGLVIEW_H
#define __OXGLVIEW_H

#include <xclass/XCconfig.h>

#if defined(HAVE_GL_GLX_H) && defined(HAVE_GL_GL_H) && defined(HAVE_GL_GLU_H)
#include <GL/glx.h>
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "OXGLFrame.h"
#include <xclass/OTimer.h>

#include "OVector3D.h"
#include "OTrackball.h"
#include "OQuaternion.h"

//----------------------------------------------------------------------

class OXGLView : public OXGLFrame {
public:
  OXGLView(const OXWindow *p, int w = 10, int h = 10, int *GLattrib = NULL);
  virtual ~OXGLView();

  virtual int HandleButton(XButtonEvent *event);
  virtual int HandleMotion(XMotionEvent *event);
  
  virtual int HandleTimer(OTimer *t);
  
  virtual void InitGL();
  virtual void SetupLighting();
  virtual void SetupMaterials();
  virtual void SetupTextures();
  virtual void SetupModelView();
  virtual void SetupFrustum();
  virtual void CreateObjects();
  virtual void RenderScene();
  virtual void DeleteObjects();
  
  void LeftSideView();
  void RightSideView();
  void FrontView();
  void RearView();
  void TopView();
  void BottomView();
  void PerspectiveView();
  void ResetView();
  
protected:
  virtual void _Resized();
  virtual void _DoRedraw();

  float _initialZoom, _zoom, _zoomFrom, _zoomTo;
  float _center[3], _shiftFrom[3], _shiftTo[3];
  float _matrix[4][4];
  OQuaternion _curQuat, _slerpFrom, _slerpTo;
  OTrackball _trackball;
  OPosition _click;
  OTimer *_timer;
  bool _animate;
  int _animMode, _animStep;
};

#endif  // __OXGLVIEW_H
