#ifndef __TUNNEL_H
#define __TUNNEL_H

#include <xclass/OXGLView.h>
#include <xclass/OXFont.h>

class Tunnel : public OXGLView {
public:
  Tunnel(const OXWindow *p);
  virtual ~Tunnel();

  virtual int HandleKey(XKeyEvent *event);
  virtual int HandleTimer(OTimer *t);
  
  virtual void SetupFrustum();
  virtual void SetupModelView();
  virtual void SetupTextures();
  virtual void SetupLighting();
  virtual void CreateObjects();
  virtual void RenderScene();

public:
  bool fog, bfcull, usetex, cstrip, help;
  
protected:
  void _DrawObjects(int *l, float *f);
  void _CalcPosObs();
  void _ShowHelp();
  void _PrintString(char *str);

  virtual void _DoRedraw();

  float  obs[3], dir[3], v, alpha, beta;
  GLuint t1id, t2id;

  OTimer *_walkTimer;
  OXFont *_font;
};


#endif  // __TUNNEL_H
