#ifndef __VIEW_H
#define __VIEW_H

#include <xclass/OXGLView.h>
#include "glm.h"

//----------------------------------------------------------------------

class GLView : public OXGLView {
public:
  GLView(const OXWindow *p);
  virtual ~GLView();
  
  virtual void CreateObjects();
  virtual void SetupLighting();
  virtual void SetupMaterials();
  virtual void SetupTextures();
  virtual void RenderScene();
  virtual void DeleteObjects();
  virtual void SetupFrustum();
  virtual void SetupModelView();
  
  bool LoadModel(char *filename);
  
  void ReverseWinding();
  
public:
  int   MaterialMode;
  float Scale;
  float SmoothingAngle;
  float WeldDistance;
  bool  FacetNormal;
  bool  BoundingBox;
  bool  CullFaces;
  bool  Wireframe;
  bool  TwoSidedLighting;

protected:
  GLMmodel *_model;
  GLuint _list;
};

#endif  // __VIEW_H
