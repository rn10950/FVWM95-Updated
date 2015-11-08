#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <X11/X.h>
#include <X11/Xlib.h>

#include <xclass/utils.h>
#include <xclass/OXFrame.h>
#include <xclass/OMessage.h>

#include "OXSwallowedFrame.h"


//----------------------------------------------------------------------

OXSwallowedFrame::OXSwallowedFrame(OXClient *c, Window w, OXWindow *p,
                                   OComponent *msgObj, int id) :
  OXFrame(c, w, p) {
    _sID = id;
    _msgObject = msgObj;
    AddInput(PropertyChangeMask | StructureNotifyMask);
}

OXSwallowedFrame::~OXSwallowedFrame() {
}

int OXSwallowedFrame::HandleEvent(XEvent *event) {
  switch (event->xany.type) {
  case DestroyNotify:
    return HandleDestroyNotify((XDestroyWindowEvent *) event);

  default:
    return OXFrame::HandleEvent(event);
  }

  return False;
}

int OXSwallowedFrame::HandleUnmapNotify(XUnmapEvent *event) {
  OWidgetMessage msg(MSG_SWALLOWED, SWALLOWED_HIDE, _sID);
  SendMessage(_msgObject, &msg);
}

int OXSwallowedFrame::HandleMapNotify(XMapEvent *event) {
  OWidgetMessage msg(MSG_SWALLOWED, SWALLOWED_SHOW, _sID);
  SendMessage(_msgObject, &msg);
}

int OXSwallowedFrame::HandleDestroyNotify(XDestroyWindowEvent *event) {
  OWidgetMessage msg(MSG_SWALLOWED, SWALLOWED_KILLED, _sID);
  SendMessage(_msgObject, &msg);
}

int OXSwallowedFrame::HandleConfigureNotify(XConfigureEvent *event) {
  return OXFrame::HandleConfigureNotify(event);
}
