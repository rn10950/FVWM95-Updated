#ifndef __VIEW_H
#define __VIEW_H

#include <xclass/OXGLView.h>

//----------------------------------------------------------------------

class GLView : public OXGLView {
public:
  GLView(const OXWindow *p);
  virtual ~GLView();
  
  virtual void CreateObjects();
  virtual void SetupTextures();
  virtual void RenderScene();
  virtual void DeleteObjects();
  
  void CrossSection(bool onoff);
  void Wireframe(bool onoff);
  
protected:
  void _DrawObjects();
  void _DrawCap();

  bool _crossSection;
  bool _wireframe;
  double _clipPlane[4];
};

#endif  // __VIEW_H
