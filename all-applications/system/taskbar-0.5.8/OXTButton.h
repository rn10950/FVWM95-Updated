/* 
 *  taskbar: xclass version of FvwmTaskBar module for Fvwm95
 *
 *  Copyright (C) 2000, Hector Peraza
 *
 */

#ifndef __OXTBUTTON_H
#define __OXTBUTTON_H

#include <xclass/OXClient.h>
#include <xclass/OXButton.h>
#include <xclass/OPicture.h>
#include <xclass/OMessage.h>
#include <xclass/OGC.h>


#define MSG_TBUTTON  (MSG_USERMSG+888)

//----------------------------------------------------------------------

class OTButtonMessage : public OWidgetMessage {
public:
  OTButtonMessage(int typ, int act, int wid, int b = -1) :
    OWidgetMessage(typ, act, wid) {
      button = b;
  }

  int button;
};

//----------------------------------------------------------------------

class OTPicture : public OPicture {
public:
  OTPicture(Pixmap pic, Pixmap mask, int width, int height, int depth) :
    OPicture(NULL) {
      _pic = pic;
      _mask = mask;
      _attributes.width = width;
      _attributes.height = height;
      _count = 1;
      _hash = NULL;
    }
};

class OXTButton : public OXButton {
protected:
  static OXGC *_gc;

public:
  OXTButton(const OXWindow *parent, int ID,
            const OXFont *norm, const OXFont *bold,
            char *title = NULL, const OPicture *pic = NULL);
  virtual ~OXTButton();

  virtual ODimension GetDefaultSize() const;

  virtual int  HandleButton(XButtonEvent *event);
  virtual void DrawBorder();

  char *GetName() const { return _title; }
  void  SetName(char *title);

  const OPicture *GetIcon() const { return _p; }
  void  SetIcon(const OPicture *p);

  int   GetDesk() const { return _desk; }
  void  SetDesk(int desk) { _desk = desk; }

  int   GetWindowFlags() const { return _winFlags; }
  void  SetWindowFlags(int flags) { _winFlags = flags; }
  
  void  AutoTip(int onoff) { _autoTip = onoff; }

protected:
  virtual void _DoRedraw();

  char  *_title;
  int   _desk, _winFlags, _autoTip;
  const OPicture *_p;
  const OXFont *_normFont, *_hiFont;
};

#endif  // __OXTBUTTON_H
