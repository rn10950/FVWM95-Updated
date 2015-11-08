/**************************************************************************

    This is a test program for xclass.
    Copyright (C) 2002, Hector Peraza.

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

#include <stdio.h>

#include <xclass/OXClient.h>
#include <xclass/OXRootWindow.h>
#include <xclass/OXFrame.h>
#include <xclass/OPicture.h>
#include <xclass/OTimer.h>
#include <xclass/utils.h>

#include <X11/extensions/shape.h>

#include "trail1.xpm"
#include "trail2.xpm"
#include "trail3.xpm"
#include "trail4.xpm"
#include "trail5.xpm"
#include "trail6.xpm"

#define NUM_TRAILS  8
#define NUM_IMAGES  6

const OPicture *pic[NUM_IMAGES];


//----------------------------------------------------------------------

class OXSprite : public OXFrame {
public:
  OXSprite(const OXWindow *p, int w, int h, const OPicture *pic = NULL);
  virtual ~OXSprite();

  void SetPicture(const OPicture *p);

protected:
  virtual void _DoRedraw();

  const OPicture *_pic;
};

OXSprite::OXSprite(const OXWindow *p, int w, int h, const OPicture *pic) :
  OXFrame(p, w, h, CHILD_FRAME) {

    XSetWindowAttributes wattr;
    unsigned long mask;

    mask = CWOverrideRedirect | CWSaveUnder;
    wattr.override_redirect = True;

    // The combination "shaped window" + "save under" is fatal for most
    // XFree86 servers: they die a miserably death... and anyway,
    // "save under" alone is broken in so many X servers that is better
    // not to request it.

    wattr.save_under = False;  //True;

    XChangeWindowAttributes(GetDisplay(), _id, mask, &wattr);

    SetPicture(pic);
}

OXSprite::~OXSprite() {
}

void OXSprite::SetPicture(const OPicture *pic) {
  _pic = pic;
  if (_pic) {
    XShapeCombineMask(GetDisplay(), _id, ShapeBounding, 0, 0,
                      _pic->GetMask(), ShapeSet);
  } else {
    XShapeCombineMask(GetDisplay(), _id, ShapeBounding, 0, 0,
                      None, ShapeSet);
  }
  NeedRedraw();
}

void OXSprite::_DoRedraw() {
  if (_pic) {
    _pic->Draw(GetDisplay(), _id, _bckgndGC, 0, 0);
  }
}


//----------------------------------------------------------------------

class OTrailSprite : public OBaseObject {
public:
  OTrailSprite(const OXWindow *p);
  virtual ~OTrailSprite();

  void Animate();

public:
  int currentFrame;
  OXSprite *f;
};

OTrailSprite::OTrailSprite(const OXWindow *p) {
  f = new OXSprite(p, 10, 10);
  currentFrame = 0;
}

OTrailSprite::~OTrailSprite() {
  f->DestroyWindow();
  delete f;
}

void OTrailSprite::Animate() {
  if (currentFrame < NUM_IMAGES)
    f->SetPicture(pic[currentFrame++]);
  else
    f->UnmapWindow();
}


//----------------------------------------------------------------------

class OXMain : public OXRootWindow {
public:
  OXMain(OXClient *c);
  virtual ~OXMain();

  virtual int HandleEvent(XEvent *event);
  virtual int HandleTimer(OTimer *t);

protected:
  virtual void _DoRedraw() {}

  OTrailSprite *ShuffleQueue();

  OTrailSprite *queue[NUM_TRAILS];
  OTimer *_timer;

  int delay;
};

OXMain::OXMain(OXClient *c) :
  OXRootWindow(c, c->GetRoot()->GetId(), NULL) {

  pic[0] = _client->GetPicture("trail1.xpm", trail1_xpm);
  pic[1] = _client->GetPicture("trail2.xpm", trail2_xpm);
  pic[2] = _client->GetPicture("trail3.xpm", trail3_xpm);
  pic[3] = _client->GetPicture("trail4.xpm", trail4_xpm);
  pic[4] = _client->GetPicture("trail5.xpm", trail5_xpm);
  pic[5] = _client->GetPicture("trail6.xpm", trail6_xpm);

  for (int i = 0; i < NUM_TRAILS; ++i) queue[i] = new OTrailSprite(this);

  delay = 40;

  XSelectInput(GetDisplay(), _id, PointerMotionMask);

  _timer = new OTimer(this, delay);
}

OXMain::~OXMain() {
  int i;

  delete _timer;
  for (i = 0; i < NUM_TRAILS; ++i) delete queue[i];
  for (i = 0; i < NUM_IMAGES; ++i) _client->FreePicture(pic[i]);
}

int lastx = -10, lasty = -10;
int newx = -10, newy = -10;

int OXMain::HandleTimer(OTimer *t) {
  if (t != _timer) return False;

  if ((newx != lastx) || (newy != lasty)) {
    OTrailSprite *currentObj = ShuffleQueue();
    if (currentObj) {
      currentObj->currentFrame = 0;
      currentObj->f->Move(newx - 3, newy - 3);
      currentObj->f->MapRaised();
    }
    lastx = newx;
    lasty = newy;
  }

  for (int i = 0; i < NUM_TRAILS; ++i) queue[i]->Animate();

  delete _timer;
  _timer = new OTimer(this, delay);

  return True;
}

int OXMain::HandleEvent(XEvent *event) {

  if (event->xany.type != MotionNotify)
    return OXRootWindow::HandleEvent(event);

  newx = event->xmotion.x;
  newy = event->xmotion.y;

  return True;
}

OTrailSprite *OXMain::ShuffleQueue() {
  int i, lastItemPos = NUM_TRAILS - 1;
  OTrailSprite *lastItem = queue[lastItemPos];

  if (lastItem->currentFrame < NUM_IMAGES) return NULL;

  for (i = lastItemPos; i > 0; --i) queue[i] = queue[i-1];
  queue[0] = lastItem;

  return queue[0];
}


//----------------------------------------------------------------------

int main(int argc, char *argv[]) {
  OXMain *mainWindow;

  OXClient *clientX = new OXClient(argc, argv);
  mainWindow = new OXMain(clientX);

  clientX->WaitFor(mainWindow);

  return 0;
}
