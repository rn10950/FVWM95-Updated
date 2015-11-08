/**************************************************************************

    This is an example xclass OpenGL application.
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

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <xclass/utils.h>
#include <xclass/OXMainFrame.h>
#include <xclass/OXGLView.h>
#include <xclass/OTimer.h>


//----------------------------------------------------------------------

class GLView : public OXGLView {
public:
  GLView(const OXWindow *p);
  virtual ~GLView();

  virtual int HandleTimer(OTimer *t);
  
  virtual void SetupLighting();
  virtual void CreateObjects();
  virtual void RenderScene();

protected:
  struct GEAR {
    GLuint  list;
    int     teeth;
    float   width;
    float   angpos;
    GLfloat color[4];
  };
  GEAR   *gears;
  int    numgears;
  float  R;
  float  angle, delta;
  OTimer *step;
};

class MainFrame : public OXMainFrame {
public:
  MainFrame(const OXWindow *p, int w, int h);
  virtual ~MainFrame();

protected:
  GLView *_glView;
};


//----------------------------------------------------------------------

int main(int argc, char **argv) {
  OXClient *clientX = new OXClient(argc, argv);

  MainFrame *mainw = new MainFrame(clientX->GetRoot(), 10, 10);
  mainw->Resize(600, 400);

  mainw->MapWindow();

  clientX->Run();

  exit(0);
}

MainFrame::MainFrame(const OXWindow *p, int w, int h) :
  OXMainFrame(p, w, h) {

  //------ GL frame

  OXCompositeFrame *glf = new OXCompositeFrame(this, 10, 10,
                                               SUNKEN_FRAME | DOUBLE_BORDER);
  AddFrame(glf, new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y));

  _glView = new GLView(glf);
  glf->AddFrame(_glView, new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y));

  SetWindowName("OpenGL test application");
  SetClassHints("XCLASS", "XCLASS");

  MapSubwindows();
}

MainFrame::~MainFrame() {
}

//----------------------------------------------------------------------

// 3-D gear wheels.  This program is in the public domain.
//
// Brian Paul
//
// Xclass version by Hector Peraza

#ifndef M_PI
#define M_PI 3.14159265
#endif

//  Draw a gear wheel.  You'll probably want to call this function when
//  building a display list since we do a lot of trig here.
// 
//  Input:  inner_radius - radius of hole at center
//          outer_radius - radius at center of teeth
//          width - width of gear
//          teeth - number of teeth
//          tooth_depth - depth of tooth

static void gear(GLfloat inner_radius, GLfloat outer_radius, GLfloat width,
                 GLint teeth, GLfloat tooth_depth) {
  GLint i;
  GLfloat r0, r1, r2;
  GLfloat angle, da, a0;
  GLfloat u, v, len;

  r0 = inner_radius;
  r1 = outer_radius - tooth_depth / 2.0;
  r2 = outer_radius + tooth_depth / 2.0;

  da = 2.0 * M_PI / teeth / 4.0;
  a0 = 2.0 * M_PI / teeth * 5.0 / 8.0;

  glShadeModel(GL_FLAT);

  glNormal3f(0.0, 0.0, 1.0);

  // draw front face
  glBegin(GL_QUAD_STRIP);
  for (i = 0; i <= teeth; i++) {
    angle = a0 + i * 2.0 * M_PI / teeth;
    glVertex3f(r0 * cos(angle), r0 * sin(angle), width * 0.5);
    glVertex3f(r1 * cos(angle), r1 * sin(angle), width * 0.5);
    glVertex3f(r0 * cos(angle), r0 * sin(angle), width * 0.5);
    glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), width * 0.5);
  }
  glEnd();

  // draw front sides of teeth
  glBegin(GL_QUADS);
  for (i = 0; i < teeth; i++) {
    angle = a0 + i * 2.0 * M_PI / teeth;
    glVertex3f(r1 * cos(angle), r1 * sin(angle), width * 0.5);
    glVertex3f(r2 * cos(angle + da), r2 * sin(angle + da), width * 0.5);
    glVertex3f(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da), width * 0.5);
    glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), width * 0.5);
  }
  glEnd();

  glNormal3f(0.0, 0.0, -1.0);

  // draw back face
  glBegin(GL_QUAD_STRIP);
  for (i = 0; i <= teeth; i++) {
    angle = a0 + i * 2.0 * M_PI / teeth;
    glVertex3f(r1 * cos(angle), r1 * sin(angle), -width * 0.5);
    glVertex3f(r0 * cos(angle), r0 * sin(angle), -width * 0.5);
    glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), -width * 0.5);
    glVertex3f(r0 * cos(angle), r0 * sin(angle), -width * 0.5);
  }
  glEnd();

  // draw back sides of teeth
  glBegin(GL_QUADS);
  for (i = 0; i < teeth; i++) {
    angle = a0 + i * 2.0 * M_PI / teeth;
    glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), -width * 0.5);
    glVertex3f(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da), -width * 0.5);
    glVertex3f(r2 * cos(angle + da), r2 * sin(angle + da), -width * 0.5);
    glVertex3f(r1 * cos(angle), r1 * sin(angle), -width * 0.5);
  }
  glEnd();

  // draw outward faces of teeth
  glBegin(GL_QUAD_STRIP);
  for (i = 0; i < teeth; i++) {
    angle = a0 + i * 2.0 * M_PI / teeth;
    glVertex3f(r1 * cos(angle), r1 * sin(angle), width * 0.5);
    glVertex3f(r1 * cos(angle), r1 * sin(angle), -width * 0.5);
    u = r2 * cos(angle + da) - r1 * cos(angle);
    v = r2 * sin(angle + da) - r1 * sin(angle);
    len = sqrt(u * u + v * v);
    u /= len;
    v /= len;
    glNormal3f(v, -u, 0.0);
    glVertex3f(r2 * cos(angle + da), r2 * sin(angle + da), width * 0.5);
    glVertex3f(r2 * cos(angle + da), r2 * sin(angle + da), -width * 0.5);
    glNormal3f(cos(angle), sin(angle), 0.0);
    glVertex3f(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da), width * 0.5);
    glVertex3f(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da), -width * 0.5);
    u = r1 * cos(angle + 3 * da) - r2 * cos(angle + 2 * da);
    v = r1 * sin(angle + 3 * da) - r2 * sin(angle + 2 * da);
    glNormal3f(v, -u, 0.0);
    glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), width * 0.5);
    glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), -width * 0.5);
    glNormal3f(cos(angle), sin(angle), 0.0);
  }

  glVertex3f(r1 * cos(a0), r1 * sin(a0), width * 0.5);
  glVertex3f(r1 * cos(a0), r1 * sin(a0), -width * 0.5);

  glEnd();

  glShadeModel(GL_SMOOTH);

  // draw inside radius cylinder
  glBegin(GL_QUAD_STRIP);
  for (i = 0; i <= teeth; i++) {
    angle = a0 + i * 2.0 * M_PI / teeth;
    glNormal3f(-cos(angle), -sin(angle), 0.0);
    glVertex3f(r0 * cos(angle), r0 * sin(angle), -width * 0.5);
    glVertex3f(r0 * cos(angle), r0 * sin(angle), width * 0.5);
  }
  glEnd();
}

//----------------------------------------------------------------------

#define ANIM_DELAY  20

GLView::GLView(const OXWindow *p) : OXGLView(p) {
  numgears = 4;
  R = 4.0;

  gears = new GEAR[numgears];

  gears[0].teeth = 20;    // gear 0 is the master
  gears[0].width = 1.0;
  gears[0].angpos = 0.0;
  gears[0].color[0] = 0.8;
  gears[0].color[1] = 0.1;
  gears[0].color[2] = 0.0;
  gears[0].color[3] = 1.0;

  gears[1].teeth = 11;
  gears[1].width = 2.0;
  gears[1].angpos = 0.0;
  gears[1].color[0] = 0.0;
  gears[1].color[1] = 0.8;
  gears[1].color[2] = 0.2;
  gears[1].color[3] = 1.0;

  gears[2].teeth = 10;
  gears[2].width = 0.5;
  gears[2].angpos = 90.0;
  gears[2].color[0] = 0.2;
  gears[2].color[1] = 0.2;
  gears[2].color[2] = 1.0;
  gears[2].color[3] = 1.0;

  gears[3].teeth = 6;
  gears[3].width = 1.5;
  gears[3].angpos = 225.0;
  gears[3].color[0] = 0.8;
  gears[3].color[1] = 0.8;
  gears[3].color[2] = 0.2;
  gears[3].color[3] = 1.0;

  angle = 0.0;
  delta = 1.5;

  step = NULL;
  _initialZoom = 25.0f;
}

GLView::~GLView() {
  if (step) delete step;
  delete[] gears;
}

void GLView::CreateObjects() {
  int   i;
  float ri, ro;

  glDisable(GL_COLOR_MATERIAL);

  // make the gears
  for (i = 0; i < numgears; ++i) {
    gears[i].list = glGenLists(1);
    glNewList(gears[i].list, GL_COMPILE);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, gears[i].color);
    ro = R * (float) gears[i].teeth / (float) gears[0].teeth;
    ri = ro / 4.0f;
    gear(ri, ro, gears[i].width, gears[i].teeth, 0.7);
    glEndList();
  }

  glEnable(GL_NORMALIZE);

  step = new OTimer(this, ANIM_DELAY);
//  FrontView();
}

void GLView::RenderScene() {
  int   i;
  float a, x, y, ratio, ar;

  for (i = 0; i < numgears; ++i) {
    glPushMatrix();
    if (i == 0) {
      glRotatef(angle, 0.0, 0.0, 1.0);
      glCallList(gears[i].list);
    } else {
      ratio = (float) gears[0].teeth / (float) gears[i].teeth;
      ar = angle * ratio;
      a = (1.0 + ratio) * gears[i].angpos;
      if ((gears[i].teeth & 1) == 0) a += 180.0 / (float) gears[i].teeth;
      x = (R + R / ratio + 0.1) * cos(gears[i].angpos * M_PI / 180.0);
      y = (R + R / ratio + 0.1) * sin(gears[i].angpos * M_PI / 180.0);
      glTranslatef(x, y, 0.0);
      glRotatef(-ar + a, 0.0, 0.0, 1.0);
      glCallList(gears[i].list);
    }
    glPopMatrix();
  }
}

int GLView::HandleTimer(OTimer *t) {
  if (t != step) return OXGLView::HandleTimer(t);
  delete step;
  step = new OTimer(this, ANIM_DELAY);
  angle += delta;
  if (angle > 360.0) angle -= 360.0;
  NeedRedraw();
}

void GLView::SetupLighting() {
#if 0
  OXGLView::SetupLighting();
#else
  static GLfloat pos[4] = { 5.0, 5.0, 10.0, 0.0 };

  glLightfv(GL_LIGHT0, GL_POSITION, pos);
  glEnable(GL_CULL_FACE);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_DEPTH_TEST);
#endif
}
