#include <limits.h>

#include "view.h"


//----------------------------------------------------------------------

GLView::GLView(const OXWindow *p) : OXGLView(p, 10, 10) {

  _model = NULL;
  _list = 0;

  Scale = 1.0;
  SmoothingAngle = 90.0;
  WeldDistance = 0.00001;
  FacetNormal = false;
  BoundingBox = false;
  MaterialMode = 0;
  CullFaces = false;
  Wireframe = false;
  TwoSidedLighting = false;

  _initialZoom = 4.0f;
}

GLView::~GLView() {
  if (_list) glDeleteLists(_list, 1);
  if (_model) glmDelete(_model);
}

bool GLView::LoadModel(char *filename) {

  if (_list) glDeleteLists(_list, 1);
  _list = 0;

  if (_model) glmDelete(_model);
  _model = glmReadOBJ(filename);

  if (_model) {
    Scale = 1.0;
    SmoothingAngle = 90.0;
    WeldDistance = 0.00001;
    MaterialMode = 0;
    glmUnitize(_model);
    glmFacetNormals(_model);
    glmVertexNormals(_model, 90.0);
    GLuint mode = FacetNormal ? GLM_FLAT : GLM_SMOOTH;
    if (_model->nummaterials > 0) MaterialMode = 2;
    if (MaterialMode == 1)
      mode |= GLM_COLOR;
    else if (MaterialMode == 2)
      mode |= GLM_MATERIAL;
    _list = glmList(_model, mode);
    NeedRedraw();
    return true;
  } else {
    return false;
  }
}

void GLView::SetupLighting() {
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
//  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

  glEnable(GL_DEPTH_TEST);
}

void GLView::SetupMaterials() {
  GLfloat ambient[] = { 0.2, 0.2, 0.2, 1.0 };
  GLfloat diffuse[] = { 0.8, 0.8, 0.8, 1.0 };
  GLfloat specular[] = { 0.0, 0.0, 0.0, 1.0 };
  GLfloat shininess = 65.0;

  glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
  glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
  glMaterialf(GL_FRONT, GL_SHININESS, shininess);
}

void GLView::SetupTextures() {
}

void GLView::CreateObjects() {
}

void GLView::DeleteObjects() {
}

void GLView::SetupFrustum() {
#if 0
  float aspectRatio = (float) _w / (float) _h;
  ::glOrtho(-_zoom / 2.5f * aspectRatio,
             _zoom / 2.5f * aspectRatio,
            -_zoom / 2.5f,
             _zoom / 2.5f,
            -1.0,
             2000.0);
#else
  OXGLView::SetupFrustum();
#endif
}

void GLView::SetupModelView() {
  OXGLView::SetupModelView();
}

void GLView::ReverseWinding() {
  if (_model) glmReverseWinding(_model);
  NeedRedraw();
}

void GLView::RenderScene() {
  if (CullFaces)
    glEnable(GL_CULL_FACE);
  else
    glDisable(GL_CULL_FACE);

  if (Wireframe)
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  else
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  if (TwoSidedLighting)
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
  else
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);

  if (_model) {
#if 0
    glmDraw(_model, GLM_SMOOTH | GLM_MATERIAL);
#else
    glCallList(_list);
#endif
  }

  if (BoundingBox) {
    glDisable(GL_LIGHTING);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glColor4f(1.0, 0.0, 0.0, 0.25);
#if 0
    //solidCube(2.0);
#else
    float size = 2.0;
    // rear end
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(-size / 2.0f, -size / 2.0f, -size / 2.0f);
    glVertex3f(-size / 2.0f,  size / 2.0f, -size / 2.0f);
    glVertex3f( size / 2.0f,  size / 2.0f, -size / 2.0f);
    glVertex3f( size / 2.0f, -size / 2.0f, -size / 2.0f);
    glEnd();

    // walls
    glBegin(GL_QUAD_STRIP);
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(-size / 2.0f, -size / 2.0f,  size / 2.0f);
    glVertex3f(-size / 2.0f, -size / 2.0f, -size / 2.0f);
    glVertex3f( size / 2.0f, -size / 2.0f,  size / 2.0f);
    glVertex3f( size / 2.0f, -size / 2.0f, -size / 2.0f);
    glNormal3f(1.0f, 0.0f, 0.0f);
    glVertex3f( size / 2.0f,  size / 2.0f,  size / 2.0f);
    glVertex3f( size / 2.0f,  size / 2.0f, -size / 2.0f);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-size / 2.0f,  size / 2.0f,  size / 2.0f);
    glVertex3f(-size / 2.0f,  size / 2.0f, -size / 2.0f);
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(-size / 2.0f, -size / 2.0f,  size / 2.0f);
    glVertex3f(-size / 2.0f, -size / 2.0f, -size / 2.0f);
    glEnd();

    // front end
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-size / 2.0f, -size / 2.0f, size / 2.0f);
    glVertex3f( size / 2.0f, -size / 2.0f, size / 2.0f);
    glVertex3f( size / 2.0f,  size / 2.0f, size / 2.0f);
    glVertex3f(-size / 2.0f,  size / 2.0f, size / 2.0f);
    glEnd();
#endif
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
  }
}
