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

#include <math.h>

#include <xclass/OXGLView.h>


#define ANIM_ROTATE  1
#define ANIM_SHIFT   2
#define ANIM_ZOOM    4

#define ANIM_DELAY   20
#define ANIM_STEPS   8


//----------------------------------------------------------------------

OXGLView::OXGLView(const OXWindow *p, int w, int h, int *GLattrib) :
  OXGLFrame(p, w, h, GLattrib) {

  // initialize transformation variables
  _center[0] = 0.0f;
  _center[1] = 0.0f;
  _center[2] = 0.0f;

  _initialZoom = 200.0f;

  // grab left button for scene move/rotate operations
  XGrabButton(GetDisplay(), Button1, AnyModifier, _id, True,
              ButtonPressMask | ButtonReleaseMask | ButtonMotionMask,
              GrabModeAsync, GrabModeAsync, None, None);

  // this is for the mouse wheel (buttons 4 and 5)
  AddInput(ButtonPressMask | ButtonReleaseMask);

  _timer = NULL;
  _animate = true;
  _animStep = 0;
}

OXGLView::~OXGLView() {
  if (_timer) delete _timer;
  DeleteObjects();
}

void OXGLView::InitGL() {
#ifdef HAVE_GL_GL_H
  _zoom = _initialZoom;

  // specify clear (background) color
  //glClearColor(0.1, 0.1, 0.2, 1);
  glClearColor(0.2, 0.3, 0.4, 1);
    
  // specify the back of the buffer as clear depth
  glClearDepth(1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
  // enable depth testing
  glDepthFunc(GL_LEQUAL);
  glEnable(GL_DEPTH_TEST);

  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

  // initialize transformation matrices
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  SetupFrustum();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  SetupModelView();

  // initialize trackball
  _curQuat = OQuaternion::FromAxis(1.0f, 0.0f, 0.0f, -30.0f * M_PI / 180.0f) *
             OQuaternion::FromAxis(0.0f, 1.0f, 0.0f, 30.0f * M_PI / 180.0f);

  _curQuat.RotMatrix(_matrix);

  // initialize scene as required (lights, materials, textures, objects, etc.)
  SetupLighting();
  SetupMaterials();
  SetupTextures();
  CreateObjects();
#endif
}

void OXGLView::SetupLighting() {
#ifdef HAVE_GL_GL_H
  // by default, setup a single white light behind and
  // slightly to the left of the viewing point
  GLfloat light_ambient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
  GLfloat light_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
  GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
  GLfloat light_position[] = { 0.0f, 25.0f, 50.0f, 0.0f };

  glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
  glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
#endif
}

void OXGLView::SetupMaterials() {
#ifdef HAVE_GL_GL_H
  // setup the default material
  float ambient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
  float diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
  float specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
  GLfloat emission[] = { 0.0f, 0.0f, 0.0f, 1.0f };
  float shininess = 100.0f;

  glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
  glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
  glMaterialf(GL_FRONT, GL_SHININESS, shininess);
  glMaterialfv(GL_FRONT, GL_EMISSION, emission);

  glEnable(GL_COLOR_MATERIAL);
#endif
}

void OXGLView::SetupTextures() {
}

void OXGLView::SetupModelView() {
#ifdef HAVE_GL_GL_H
  // setup the viewing transform:
  // view the objects from the specified distance
  glTranslatef(0.0f, 0.0f, -_zoom);
#endif
}

void OXGLView::SetupFrustum() {
#ifdef HAVE_GL_GLU_H
  // setup the viewing frustum:
  // select a default perspective view
  float aspectRatio = (float) _w / (float) _h;
  gluPerspective(40.0f, aspectRatio, 1.0f, 5000.0f);
#endif
}

void OXGLView::CreateObjects() {
}

void OXGLView::DeleteObjects() {
}

void OXGLView::_Resized() {
  OXFrame::_Resized();
  RestoreContext();

#ifdef HAVE_GL_GL_H
  // setup the viewing area: select the full client area
  glViewport(0, 0, _w, _h);

  // initialize the projection matrix
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  // select the viewing volume
  SetupFrustum();

  // initialize the modelview matrix and leave it as the default mode
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // perform default viewing transformations
  SetupModelView();
#endif
}

void OXGLView::_DoRedraw() {
  RestoreContext();

#ifdef HAVE_GL_GL_H
  // clear out the color and depth buffers
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#if 1
  // save the model matrix
  glPushMatrix();

  // apply the current transformations
  glMultMatrixf(&_matrix[0][0]);
  glTranslatef(-_center[0], -_center[1], -_center[2]);

  // render the scene
  RenderScene();

  // restore the model matrix
  glPopMatrix();
#else
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  SetupFrustum();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  SetupModelView();
  glMultMatrixf(&_matrix[0][0]);
  glTranslatef(-_center[0], -_center[1], -_center[2]);
  RenderScene();
#endif

  // flush the OpenGL pipeline
  glFinish();
  if (_doubleBuf) glXSwapBuffers(GetDisplay(), _id);
#endif
}

int OXGLView::HandleButton(XButtonEvent *event) {

  OXGLFrame::HandleButton(event);
  if (!IsEnabled()) return True;

  RestoreContext();

#ifdef HAVE_GL_GL_H
  if (event->type == ButtonPress) {
    // handle the mouse wheel: zoom in and out
    if (event->button == Button4) {
      float delta = (event->state & ShiftMask) ? 1.99f : 0.91f;
      _zoom *= delta;

      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      SetupFrustum();
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();
      SetupModelView();
      NeedRedraw();

      return True;
    } else if (event->button == Button5) {
      float delta = (event->state & ShiftMask) ? 1.01f : 1.1f;
      _zoom *= delta;

      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      SetupFrustum();
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();
      SetupModelView();
      NeedRedraw();

      return True;
    }
  }
#endif

  _click.x = event->x;
  _click.y = event->y;

  return True;
}

int OXGLView::HandleMotion(XMotionEvent *event) {

  OXGLFrame::HandleMotion(event);
  if (!IsEnabled()) return True;

  RestoreContext();

#ifdef HAVE_GL_GL_H
  if (event->state & ControlMask) {

    // CTRL + drag left button up/down: change zoom
    // (just for the case the user does not have a wheel mouse)
    _zoom += (float) (event->y - _click.y) / 2.0f;

    // initialize the projection matrix, etc... (we know the drill)
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    SetupFrustum();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    SetupModelView();

  } else if (event->state & ShiftMask) {

    // SHIFT + drag left button: shift the scene position
    int dx = event->x - _click.x;
    int dy = event->y - _click.y;

    float cx = (float) -dx * _zoom / 1000.0f;
    float cy = (float) dy * _zoom / 1000.0f;
    float cz = 0.0f;

    // unproject the translation vector before shifting
    float tx = cx - _matrix[3][0];
    float ty = cy - _matrix[3][1];
    float tz = cz - _matrix[3][2];
    cx = tx * _matrix[0][0] + ty * _matrix[0][1] + tz * _matrix[0][2];
    cy = tx * _matrix[1][0] + ty * _matrix[1][1] + tz * _matrix[1][2];
    cz = tx * _matrix[2][0] + ty * _matrix[2][1] + tz * _matrix[2][2];

    _center[0] += cx;
    _center[1] += cy;
    _center[2] += cz;

  } else {

    // in all other cases, rotate the scene
    _curQuat = _trackball.Track((2.0f * _click.x - _w) / _w,
                                (_h - 2.0f * _click.y) / _h,
                                (2.0f * event->x - _w) / _w,
                                (_h - 2.0f * event->y) / _h) * _curQuat;
    _curQuat.RotMatrix(_matrix);

  }
#endif

  _click.x = event->x;
  _click.y = event->y;

  NeedRedraw();

  return True;
}

void OXGLView::RenderScene() {
}

//----------------------------------------------------------------------

// The following functions can be used to orient the scene into
// one of the standard views. The orientation is animated if
// the variable _animate is True;

void OXGLView::LeftSideView() {
  _slerpFrom = _curQuat;
  _slerpTo = OQuaternion::FromAxis(0.0f, 1.0f, 0.0f, -M_PI / 2.0f);

  if (_animate) {
    _animStep = 0;
    _animMode = ANIM_ROTATE;
    if (_timer) delete _timer;
    _timer = new OTimer(this, ANIM_DELAY);
  } else {
    _curQuat = _slerpTo;
    _curQuat.RotMatrix(_matrix);
    NeedRedraw();
  }
}

void OXGLView::RightSideView() {
  _slerpFrom = _curQuat;
  _slerpTo = OQuaternion::FromAxis(0.0f, 1.0f, 0.0f, M_PI / 2.0f);

  if (_animate) {
    _animStep = 0;
    _animMode = ANIM_ROTATE;
    if (_timer) delete _timer;
    _timer = new OTimer(this, ANIM_DELAY);
  } else {
    _curQuat = _slerpTo;
    _curQuat.RotMatrix(_matrix);
    NeedRedraw();
  }
}

void OXGLView::FrontView() {
  _slerpFrom = _curQuat;
  _slerpTo = OQuaternion::FromAxis(1.0f, 0.0f, 0.0f, 0.0f);

  if (_animate) {
    _animStep = 0;
    _animMode = ANIM_ROTATE;
    if (_timer) delete _timer;
    _timer = new OTimer(this, ANIM_DELAY);
  } else {
    _curQuat = _slerpTo;
    _curQuat.RotMatrix(_matrix);
    NeedRedraw();
  }
}

void OXGLView::RearView() {
  _slerpFrom = _curQuat;
  _slerpTo = OQuaternion::FromAxis(1.0f, 0.0f, 0.0f, M_PI) *
             OQuaternion::FromAxis(0.0f, 0.0f, 1.0f, M_PI);

  if (_animate) {
    _animStep = 0;
    _animMode = ANIM_ROTATE;
    if (_timer) delete _timer;
    _timer = new OTimer(this, ANIM_DELAY);
  } else {
    _curQuat = _slerpTo;
    _curQuat.RotMatrix(_matrix);
    NeedRedraw();
  }
}

void OXGLView::TopView() {
  _slerpFrom = _curQuat;
  _slerpTo = OQuaternion::FromAxis(0.0f, 1.0f, 0.0f, M_PI / 2.0f) *
             OQuaternion::FromAxis(0.0f, 0.0f, 1.0f, M_PI / 2.0f);

  if (_animate) {
    _animStep = 0;
    _animMode = ANIM_ROTATE;
    if (_timer) delete _timer;
    _timer = new OTimer(this, ANIM_DELAY);
  } else {
    _curQuat = _slerpTo;
    _curQuat.RotMatrix(_matrix);
    NeedRedraw();
  }
}

void OXGLView::BottomView() {
  _slerpFrom = _curQuat;
  _slerpTo = OQuaternion::FromAxis(0.0f, 1.0f, 0.0f, M_PI / 2.0f) *
             OQuaternion::FromAxis(0.0f, 0.0f, 1.0f, -M_PI / 2.0f);

  if (_animate) {
    _animStep = 0;
    _animMode = ANIM_ROTATE;
    if (_timer) delete _timer;
    _timer = new OTimer(this, ANIM_DELAY);
  } else {
    _curQuat = _slerpTo;
    _curQuat.RotMatrix(_matrix);
    NeedRedraw();
  }
}

void OXGLView::PerspectiveView() {
  _slerpFrom = _curQuat;
  _slerpTo = OQuaternion::FromAxis(1.0f, 0.0f, 0.0f, -30.0f * M_PI / 180.0f) *
             OQuaternion::FromAxis(0.0f, 1.0f, 0.0f, 30.0f * M_PI / 180.0f);

  if (_animate) {
    _animStep = 0;
    _animMode = ANIM_ROTATE;
    if (_timer) delete _timer;
    _timer = new OTimer(this, ANIM_DELAY);
  } else {
    _curQuat = _slerpTo;
    _curQuat.RotMatrix(_matrix);
    NeedRedraw();
  }
}

void OXGLView::ResetView() {

  _shiftFrom[0] = _center[0];
  _shiftFrom[1] = _center[1];
  _shiftFrom[2] = _center[2];
  _shiftTo[0] = 0;
  _shiftTo[1] = 0;
  _shiftTo[2] = 0;

  _zoomFrom = _zoom;
  _zoomTo = _initialZoom;

  _animMode = ANIM_SHIFT | ANIM_ZOOM;

  bool perspective = true;
  if (perspective) {
    _slerpFrom = _curQuat;
    _slerpTo = OQuaternion::FromAxis(1.0f, 0.0f, 0.0f, -30.0f * M_PI / 180.0f) *
               OQuaternion::FromAxis(0.0f, 1.0f, 0.0f, 30.0f * M_PI / 180.0f);

    _animMode |= ANIM_ROTATE;
  }

  if (_animate) {
    _animStep = 0;
    if (_timer) delete _timer;
    _timer = new OTimer(this, ANIM_DELAY);
  } else {
    _center[0] = _shiftTo[0];
    _center[1] = _shiftTo[1];
    _center[2] = _shiftTo[2];
    _zoom = _zoomTo;
    RestoreContext();
#ifdef HAVE_GL_GL_H
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    SetupFrustum();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    SetupModelView();
#endif
    if (perspective) {
      _curQuat = _slerpTo;
      _curQuat.RotMatrix(_matrix);
    }
    NeedRedraw();
  }
}

// Animation timer handling

int OXGLView::HandleTimer(OTimer *t) {

  if (t != _timer) return False;

  ++_animStep;

  if (_animMode & ANIM_ROTATE) {
    //_moving = true;
    _curQuat = OQuaternion::Slerp(_slerpFrom, _slerpTo,
                                  (float) _animStep / (float) ANIM_STEPS);
    _curQuat.RotMatrix(_matrix);
  }

  if (_animMode & ANIM_SHIFT) {
    //_moving = true;
    _center[0] = _shiftFrom[0] + (float) _animStep / (float) ANIM_STEPS * (_shiftTo[0] - _shiftFrom[0]);
    _center[1] = _shiftFrom[1] + (float) _animStep / (float) ANIM_STEPS * (_shiftTo[1] - _shiftFrom[1]);
    _center[2] = _shiftFrom[2] + (float) _animStep / (float) ANIM_STEPS * (_shiftTo[2] - _shiftFrom[2]);
  }

  if (_animMode & ANIM_ZOOM) {
    //_moving = true;
    _zoom = _zoomFrom + (float) _animStep / (float) ANIM_STEPS * (_zoomTo - _zoomFrom);
    RestoreContext();
#ifdef HAVE_GL_GL_H
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    SetupFrustum();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    SetupModelView();
#endif
  }

  delete _timer;

  if (_animStep >= ANIM_STEPS) {
    _timer = NULL;
    //_moving = false;
  } else {
    _timer = new OTimer(this, ANIM_DELAY);
  }

  NeedRedraw();

  return True;
}
