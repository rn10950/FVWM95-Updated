#include <limits.h>

#include "view.h"
#include "teapot.h"

static GLuint texName;


//----------------------------------------------------------------------

GLView::GLView(const OXWindow *p) : OXGLView(p, 10, 10) {
  _crossSection = false;
  _clipPlane[0] = -1.0;
  _clipPlane[1] =  0.0;
  _clipPlane[2] =  0.0;
  _clipPlane[3] =  0.0;
  _wireframe = false;
}

GLView::~GLView() {
}

void GLView::SetupTextures() {
  // load a sample texture
  FILE *f = fopen("env.ppm", "r");
  if (f) {
    char tmp[PATH_MAX];
    int  xres, yres, zres;

    fgets(tmp, 20, f);
    if (strcmp(tmp, "P6\n") != 0) {
      // invalid file type
      fclose(f);
      return;
    }

    xres = yres = zres = -1;

    for (;;) {
      fgets(tmp, PATH_MAX, f);
      if (tmp[0] == '#') continue;
      sscanf(tmp, "%d %d", &xres, &yres);
      break;
    }

    for (;;) {
      fgets(tmp, PATH_MAX, f);
      if (tmp[0] == '#') continue;
      sscanf(tmp, "%d", &zres);
      break;
    }

    // TODO: check image size, must be a power of two, and smaller
    // than the largest value allowed

    if (xres < 4 || xres > 5000 || yres < 4 || yres > 5000) {
      // too small or too large
      fclose(f);
      return;
    }

    if (zres <= 0)  {
      // invalid depth
      fclose(f);
      return;
    }

    GLubyte *image = new GLubyte[xres * yres * 4];

    for (int y = yres - 1; y >= 0; --y) {
      for (int x = 0; x < xres; ++x) {
        int ix = 4 * (y * xres + x);
        image[ix + 0] = getc(f);
        image[ix + 1] = getc(f);
        image[ix + 2] = getc(f);
        image[ix + 3] = 255;
      }
    }

    fclose(f);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glGenTextures(1, &texName);
    glBindTexture(GL_TEXTURE_2D, texName);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, xres, yres, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, image);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

    delete[] image;
  }
}

void GLView::CreateObjects() {
  // nothing to do here for this simple example;
  // for complex scenes you might want to create display lists here.
  // this is also a good place to create the object tree.
}

void GLView::DeleteObjects() {
  // this might be a good place to free display lists and memory
  // allocated by objects, structures, trees, etc.
}

void GLView::CrossSection(bool onoff) {
  _crossSection = onoff;
  NeedRedraw();
}

void GLView::Wireframe(bool onoff) {
  _wireframe = onoff;
  NeedRedraw();
}

void GLView::RenderScene() {
  if (_crossSection) {

    glDisable(GL_LIGHTING);

    glClipPlane(GL_CLIP_PLANE0, _clipPlane);

    glClearStencil(0);
    glDisable(GL_STENCIL_TEST);

    // clear the stencil and depth buffers (the color buffer has been
    // already cleared)
    glClear(GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // disable color buffer writes
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    // render the capping polygon into the depth buffer
    _DrawCap();

    // disable depth buffer writes
    glDepthMask(GL_FALSE);

    // Use the stencil buffer to find out where the cap cuts our model:
    // set stencil op to incr stencil value where depth test passes
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 1, 1);
    glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);

    // draw model with glCullFace(GL_BACK)
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    _DrawObjects();

    // set stencil op to decr stencil value where depth test passes
    glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);

    // draw model with glCullFace(GL_FRONT)
    glCullFace(GL_FRONT);
    _DrawObjects();
    glDisable(GL_CULL_FACE);

    // clear depth buffer
    glDepthMask(GL_TRUE);
    glClear(GL_DEPTH_BUFFER_BIT);

    // enable color buffer writes
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    // draw the capping polygon with stencil function set to GL_EQUAL
    // and the reference value set to 1
    glStencilFunc(GL_EQUAL, 1, 1);
    _DrawCap();

    // disable stencil
    glDisable(GL_STENCIL_TEST);

    // apply OpenGL clipping plane
    glEnable(GL_CLIP_PLANE0);

    // draw model with color and depth enabled:
    glEnable(GL_LIGHTING);
    _DrawObjects(); //RenderModel();

    glDisable(GL_CLIP_PLANE0);

  } else {

    _DrawObjects();

  }
}

void GLView::_DrawObjects() {

  glPushMatrix();
  glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);

  if (_wireframe) {
    glColor4f(0.8f, 0.75f, 0.4f, 0.0f);
    renderWireTeapot(50.0);
  } else {
    // setup environment mapping
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);

    glEnable(GL_TEXTURE_2D);
    glColor4f(0.8f, 0.75f, 0.4f, 0.0f);
    renderSolidTeapot(50.0);

    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);

    glDisable(GL_TEXTURE_2D);
  }

  glPopMatrix();
}

void GLView::_DrawCap() {
  float extent = 1000.0f;

  OVector3D n(_clipPlane[0], _clipPlane[1], _clipPlane[2]);
  n.normalize();
  OVector3D v(1.0f, 0.0f, 0.0f);
  if ((n.getY() == 0.0f) && (n.getZ() == 0.0f))
    v = OVector3D(0.0f, 1.0f, 0.0f);

  OVector3D b1 = OVector3D::cross(v, n);
  OVector3D b2 = OVector3D::cross(n, b1);

  float w = -_clipPlane[3];
  float x = w * n.getX();
  float y = w * n.getY();
  float z = w * n.getZ();

  b1.normalize();
  b2.normalize();

  OVector3D v1 = b1 + b2;
  OVector3D v2 = b1 - b2;

  float x1 = x + extent * v1.getX();
  float y1 = y + extent * v1.getY();
  float z1 = z + extent * v1.getZ();
  float x2 = x + extent * v2.getX();
  float y2 = y + extent * v2.getY();
  float z2 = z + extent * v2.getZ();
  float x3 = x - extent * v1.getX();
  float y3 = y - extent * v1.getY();
  float z3 = z - extent * v1.getZ();
  float x4 = x - extent * v2.getX();
  float y4 = y - extent * v2.getY();
  float z4 = z - extent * v2.getZ();

  //LoadTexture(_HATCH);
  //glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  //glEnable(GL_TEXTURE_2D);
  glColor4f(0.6f, 0.6f, 0.0f, 1.0f);

  glBegin(GL_QUADS);
  glNormal3f(-n.getX(), -n.getY(), -n.getZ());
  glTexCoord2f(200.0f, 0.0f);
  glVertex3f(x1, y1, z1);
  glTexCoord2f(200.0f, 200.0f);
  glVertex3f(x2, y2, z2);
  glTexCoord2f(0.0f, 200.0f);
  glVertex3f(x3, y3, z3);
  glTexCoord2f(0.0f, 0.0f);
  glVertex3f(x4, y4, z4);
  glEnd();

  //glDisable(GL_TEXTURE_2D);
}
