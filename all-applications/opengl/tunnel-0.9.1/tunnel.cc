/*
 * This program is under the GNU GPL.
 * Use at your own risk.
 *
 * written by David Bucciarelli (tech.hmw@plus.it)
 *            Humanware s.r.l.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <X11/keysym.h>

#include "main.h"
#include "tunnel.h"
#include "image.h"
#include "sources.h"

#if defined(GL_VERSION_1_1)
/* Routines called directly. */
#elif defined(GL_EXT_texture_object) && defined(GL_EXT_copy_texture) && defined(GL_EXT_subtexture)
#define glBindTexture(A,B)     glBindTextureEXT(A,B)
#define glGenTextures(A,B)     glGenTexturesEXT(A,B)
#define glDeleteTextures(A,B)  glDeleteTexturesEXT(A,B)
#else
#define glBindTexture(A,B)
#define glGenTextures(A,B)
#define glDeleteTextures(A,B)
#endif


#define NUMBLOC 6

/* Some <math.h> files do not define M_PI... */
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

//----------------------------------------------------------------------

Tunnel::Tunnel(const OXWindow *p) : OXGLView(p) {

  obs[0] = 1000.0;
  obs[1] = 0.0;
  obs[2] = 2.0;

  v = 0.05;

  alpha = 90.0;
  beta = 90.0;

  fog = true;
  bfcull = true;
  usetex = true;
  cstrip = false;
  help = true;
  
  _walkTimer = NULL;
  AddInput(KeyPressMask);

  //_font = _client->GetFont("Helvetica -14 bold");
  _font = _client->GetFont("Lucidatypewriter -14 bold");

  TakeFocus(True);
}

Tunnel::~Tunnel() {
  if (_walkTimer) delete _walkTimer;
  _client->FreeFont((OXFont *) _font);
}

void Tunnel::SetupTextures() {
  IMAGE *img;
  GLenum gluerr;
  
  glGenTextures(1, &t1id);
  glBindTexture(GL_TEXTURE_2D, t1id);

  if (!(img = ImageLoad("tile.rgb"))) {
    FatalError("Error reading texture \"tile.rgb\"\n");
  }

  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  if ((gluerr = gluBuild2DMipmaps(GL_TEXTURE_2D, 3,
                                  img->sizeX, img->sizeY, GL_RGB,
			          GL_UNSIGNED_BYTE, (GLvoid *) (img->data)))) {
    FatalError("GLU error: %s\n", gluErrorString(gluerr));
  }

  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

  glGenTextures(1, &t2id);
  glBindTexture(GL_TEXTURE_2D, t2id);

  if (!(img = ImageLoad("bw.rgb"))) {
    FatalError("Error reading texture \"bw.rgb\"\n");
  }

  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  if ((gluerr = gluBuild2DMipmaps(GL_TEXTURE_2D, 3,
                                  img->sizeX, img->sizeY, GL_RGB,
			          GL_UNSIGNED_BYTE, (GLvoid *) (img->data)))) {
    FatalError("GLU error: %s\n", gluErrorString(gluerr));
  }

  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

void Tunnel::_DrawObjects(int *l, float *f) {
  int mend, j;

  if (cstrip) {
    float r = 0.33, g = 0.33, b = 0.33;

    for (; *l; ) {
      mend = *l++;

      r += 0.33;
      if (r > 1.0) {
	r = 0.33;
	g += 0.33;
	if (g > 1.0) {
	  g = 0.33;
	  b += 0.33;
	  if (b > 1.0) {
	    b = 0.33;
          }
	}
      }

      glColor3f(r, g, b);
      glBegin(GL_TRIANGLE_STRIP);
      for (j = 0; j < mend; j++) {
	f += 4;
	glTexCoord2fv(f);
        f += 2;
	glVertex3fv(f);
        f += 3;
      }
      glEnd();
    }
  } else {
    for ( ; *l; ) {
      mend = *l++;

      glBegin(GL_TRIANGLE_STRIP);
      for (j = 0; j < mend; j++) {
	glColor4fv(f);
        f += 4;
	glTexCoord2fv(f);
        f += 2;
	glVertex3fv(f);
        f += 3;
      }
      glEnd();
    }
  }
}

void Tunnel::_CalcPosObs() {
  dir[0] = sin(alpha * M_PI / 180.0);
  dir[1] = cos(alpha * M_PI / 180.0) * sin(beta * M_PI / 180.0);
  dir[2] = cos(beta * M_PI / 180.0);

  obs[0] += v * dir[0];
  obs[1] += v * dir[1];
  obs[2] += v * dir[2];
}

int Tunnel::HandleKey(XKeyEvent *event) {
  KeySym keysym;
  char input[4] = { 0, 0, 0, 0 };
  static XComposeStatus compose = { NULL, 0 };
  int  chrcnt;
  
  if (event->type == KeyPress) {
    chrcnt = XLookupString(event, input, sizeof(input)-1, &keysym, &compose);
  
    switch (keysym) {
      case XK_Left:  alpha -= 2.0; break;
      case XK_Right: alpha += 2.0; break;
      case XK_Down:  beta  -= 2.0; break;
      case XK_Up:    beta  += 2.0; break;

      default:
        if (chrcnt == 1) {
          switch (*input) {
            case 'a': v += 0.01; break;
            case 'z': v -= 0.01; break;
            case 'h': help = !help; break;
            case 'f': fog = !fog; break;
            case 't': usetex = !usetex; break;
            case 'b':
              if (bfcull) {
                glDisable(GL_CULL_FACE);
                bfcull = false;
              } else {
                glEnable(GL_CULL_FACE);
                bfcull = true;
              }
              break;
            case 'm': cstrip = !cstrip; break;
            case 'd':
              glDeleteTextures(1, &t1id);
              glDeleteTextures(1, &t2id);
              SetupTextures();
              break;

            case 'r':
              obs[0] = 1000.0;
              obs[1] = 0.0;
              obs[2] = 2.0;
              v = 0.05;
              alpha = 90.0;
              beta = 90.0;
              break;
          }
          MainFrame *main = (MainFrame *) GetTopLevel();
          if (main) main->UpdateMenu();
        }
        break;
    }
  }
}

void Tunnel::SetupFrustum() {
  gluPerspective(80.0, (float) _w / (float) _h, 1.0, 50.0);
}

void Tunnel::SetupModelView() {
}

void Tunnel::_PrintString(char *string) {
  static int fontList = 0;

  if (!fontList) {
    fontList = glGenLists(1);
    glXUseXFont(_font->GetId(), 0, 256, fontList);
  }
  glListBase(fontList);

  if (*string) {
    glCallLists(strlen(string), GL_UNSIGNED_BYTE, string);
  }
}

void Tunnel::_ShowHelp() {
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glColor4f(0.0, 0.0, 0.0, 0.5);
  glRecti(40, 40, 600, 440);
  glDisable(GL_BLEND);

  glColor3f(1.0, 0.0, 0.0);
  glRasterPos2i(300, 420);
  _PrintString("Help");

  glRasterPos2i(60, 390);
  _PrintString("h - Togle Help");
  glRasterPos2i(60, 360);
  _PrintString("t - Togle Textures");
  glRasterPos2i(60, 330);
  _PrintString("f - Togle Fog");
  glRasterPos2i(60, 300);
  _PrintString("m - Togle strips");
  glRasterPos2i(60, 270);
  _PrintString("b - Togle Back face culling");
  glRasterPos2i(60, 240);
  _PrintString("r - Reset");
  glRasterPos2i(60, 210);
  _PrintString("Arrow Keys - Rotate");
  glRasterPos2i(60, 180);
  _PrintString("a - Increase velocity");
  glRasterPos2i(60, 150);
  _PrintString("z - Decrease velocity");
}

void Tunnel::_DoRedraw() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  RenderScene();
  glFinish();
  glXSwapBuffers(GetDisplay(), _id);
}

void Tunnel::RenderScene() {
  int i;
  float base, offset;

  if (usetex)
    glEnable(GL_TEXTURE_2D);
  else
    glDisable(GL_TEXTURE_2D);
  
  if (fog)
    glEnable(GL_FOG);
  else
    glDisable(GL_FOG);
  
  glShadeModel(GL_SMOOTH);

  glPushMatrix();
  _CalcPosObs();

  gluLookAt(obs[0], obs[1], obs[2],
	    obs[0] + dir[0], obs[1] + dir[1], obs[2] + dir[2],
	    0.0, 0.0, 1.0);

  if (dir[0] > 0) {
    offset = 8.0;
    base = obs[0] - fmod(obs[0], 8.0);
  } else {
    offset = -8.0;
    base = obs[0] + (8.0 - fmod(obs[0], 8.0));
  }

  glPushMatrix();
  glTranslatef(base - offset / 2.0, 0.0, 0.0);
  for (i = 0; i < NUMBLOC; i++) {
    glTranslatef(offset, 0.0, 0.0);
    glBindTexture(GL_TEXTURE_2D, t1id);
    _DrawObjects(striplength_skin_11, stripdata_skin_11);
    glBindTexture(GL_TEXTURE_2D, t2id);
    _DrawObjects(striplength_skin_12, stripdata_skin_12);
    _DrawObjects(striplength_skin_9, stripdata_skin_9);
    _DrawObjects(striplength_skin_13, stripdata_skin_13);
  }
  glPopMatrix();
  glPopMatrix();

  glDisable(GL_TEXTURE_2D);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_FOG);
  glShadeModel(GL_FLAT);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-0.5, 639.5, -0.5, 479.5, -1.0, 1.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  if (help) _ShowHelp();

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  SetupFrustum();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  SetupModelView();
}

void Tunnel::SetupLighting() {
  float fogcolor[4] = { 0.7, 0.7, 0.7, 1.0 };

  glEnable(GL_FOG);
  glFogi(GL_FOG_MODE, GL_EXP2);
  glFogfv(GL_FOG_COLOR, fogcolor);

  glFogf(GL_FOG_DENSITY, 0.06);
  glHint(GL_FOG_HINT, GL_NICEST);

  glClearColor(fogcolor[0], fogcolor[1], fogcolor[2], fogcolor[3]);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Tunnel::CreateObjects() {

#if 0
  _matrix[0][0] = 1.0f; _matrix[0][1] = 0.0f; _matrix[0][2] = 0.0f; _matrix[0][3] = 0.0f;
  _matrix[1][0] = 0.0f; _matrix[1][1] = 1.0f; _matrix[1][2] = 0.0f; _matrix[1][3] = 0.0f;
  _matrix[2][0] = 0.0f; _matrix[2][1] = 0.0f; _matrix[2][2] = 1.0f; _matrix[2][3] = 0.0f;
  _matrix[3][0] = 0.0f; _matrix[3][1] = 0.0f; _matrix[3][2] = 0.0f; _matrix[3][3] = 1.0f;
#endif

  glShadeModel(GL_SMOOTH);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glEnable(GL_TEXTURE_2D);

  _CalcPosObs();

//  _idle = new OIdleHandler();
  _walkTimer = new OTimer(this, 20);
}

int Tunnel::HandleTimer(OTimer *t) {
  if (t == _walkTimer) {
    delete _walkTimer;
    _walkTimer = new OTimer(this, 20);
    NeedRedraw();
  } else {
    return OXGLView::HandleTimer(t);
  }
  return True;
}
