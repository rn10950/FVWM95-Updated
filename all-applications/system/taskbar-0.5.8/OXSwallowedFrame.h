/* 
 *  taskbar: xclass version of FvwmTaskBar module for Fvwm95
 *
 *  Copyright (C) 2000, Hector Peraza
 *
 */

#ifndef __OXSWALLOWEDFRAME_H
#define __OXSWALLOWEDFRAME_H

#include <xclass/OXClient.h>
#include <xclass/OComponent.h>
#include <xclass/OXFrame.h>

#define MSG_SWALLOWED       (MSG_USERMSG+1700)

#define SWALLOWED_HIDE      1
#define SWALLOWED_SHOW      2
#define SWALLOWED_KILLED    3
#define SWALLOWED_CHANGED   4


//----------------------------------------------------------------------

struct SListSwallowedElt {
public:
  char *name;
  int id, aspect_x, aspect_y;
  OXFrame *frame;
};

class OXSwallowedFrame : public OXFrame {
public:
  OXSwallowedFrame(OXClient *c, Window w, OXWindow *p,
                   OComponent *msgObj, int id);
  virtual ~OXSwallowedFrame();

  virtual int HandleEvent(XEvent *event);

  virtual int HandleUnmapNotify(XUnmapEvent *event);
  virtual int HandleMapNotify(XMapEvent *event);
  virtual int HandleDestroyNotify(XDestroyWindowEvent *event);
  virtual int HandleConfigureNotify(XConfigureEvent *event);

protected:
  int _sID;
  OComponent *_msgObject;
};

#endif  // __OXSWALLOWEDFRAME_H
