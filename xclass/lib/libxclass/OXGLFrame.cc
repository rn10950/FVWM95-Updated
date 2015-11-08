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

#include <xclass/OXGLFrame.h>


#ifdef HAVE_GL_GLX_H
GLXContext OXGLFrame::_current_glxc = NULL;
#endif

//----------------------------------------------------------------------

OXGLFrame::OXGLFrame(const OXWindow *p, int w, int h, int *GLattrib) :
  OXFrame(p, w, h) {

#if defined(HAVE_GL_GLX_H) && defined(HAVE_GL_GL_H) && defined(HAVE_GL_GLU_H)
  static int defaultAttribList[] = {
    GLX_RGBA,
    GLX_DOUBLEBUFFER,
    GLX_RED_SIZE,     1,
    GLX_GREEN_SIZE,   1,
    GLX_BLUE_SIZE,    1,
    GLX_STENCIL_SIZE, 1,
    None
  };

  // this is ugly, but since we cannot call virtual functions
  // in the constructor...

  _client->UnregisterWindow(this);
  XDestroyWindow(GetDisplay(), _id);

  XSetWindowAttributes wattr;

  if (!GLattrib) GLattrib = defaultAttribList;
  _glvi = glXChooseVisual(GetDisplay(), _client->GetScreenNumber(),
                          GLattrib);
  if (!_glvi) {
    FatalError("Could not find a suitable GLX visual");
  }

  _glxc = glXCreateContext(GetDisplay(), _glvi, 0, GL_TRUE);
  if (!_glxc) {
    FatalError("Could not create GLX context");
  }

  Colormap cmap = XCreateColormap(GetDisplay(), _client->GetRoot()->GetId(),
                                  _glvi->visual, AllocNone);

  wattr.colormap = cmap;
  wattr.border_pixel = 0;
  wattr.event_mask = _eventMask = ExposureMask | StructureNotifyMask;

  _id = XCreateWindow(GetDisplay(), _parent->GetId(), 0, 0,
                      max(w, 1), max(h, 1), 0,
                      _glvi->depth, InputOutput, _glvi->visual,
                      CWEventMask | CWColormap, &wattr);
  _windowExists = (_id != None);
  _client->RegisterWindow(this);

  _doubleBuf = false;
  int retval;
  if (glXGetConfig(GetDisplay(), _glvi,
                   GLX_DOUBLEBUFFER, &retval) == Success) {
    _doubleBuf = (retval == True);
  }
#else
  FatalError("No OpenGL support was included in the xclass library\n"
             "at compile time.\n");
#endif
}

OXGLFrame::~OXGLFrame() {
#ifdef HAVE_GL_GLX_H
  if (_current_glxc == _glxc) {
    glXMakeCurrent(GetDisplay(), None, NULL);
    _current_glxc = NULL;
  }
  // the window has been already destroyed
  //glXDestroyContext(GetDisplay(), _glxc);
#endif
}

int OXGLFrame::HandleMapNotify(XMapEvent *event) {
  OXFrame::HandleMapNotify(event);

#ifdef HAVE_GL_GLX_H
  glXMakeCurrent(GetDisplay(), _id, _glxc);
  _current_glxc = _glxc;
#endif

  // initialize OpenGL
  InitGL();

  return True;
}

void OXGLFrame::_Resized() {
  OXFrame::_Resized();
  RestoreContext();
#ifdef HAVE_GL_GL_H
  // setup the viewing area: select the full client area
  glViewport(0, 0, _w, _h);
#endif
}

void OXGLFrame::_DoRedraw() {
  RestoreContext();
#ifdef HAVE_GL_GL_H
  // clear out the color and depth buffers
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // flush the OpenGL pipeline
  glFinish();
  if (_doubleBuf) glXSwapBuffers(GetDisplay(), _id);
#endif
}

void OXGLFrame::InitGL() {
#ifdef HAVE_GL_GL_H
  // specify clear (background) color
  glClearColor(0.2, 0.3, 0.4, 1);
    
  // specify the back of the buffer as clear depth
  glClearDepth(1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
  // enable depth testing
  glDepthFunc(GL_LEQUAL);
  glEnable(GL_DEPTH_TEST);

  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
#endif
}

void OXGLFrame::RestoreContext() {
#ifdef HAVE_GL_GL_H
#if 0
  if (_current_glxc != _glxc) {
    glXMakeCurrent(GetDisplay(), _id, _glxc);
    _current_glxc = _glxc;
  }
#else
  if (glXGetCurrentContext() != _glxc) {
    glXMakeCurrent(GetDisplay(), _id, _glxc);
  }
#endif
#endif
}
