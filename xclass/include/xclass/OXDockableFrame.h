/**************************************************************************

    This file is part of xclass.
    Copyright (C) 1997-2000, Harald Radke, Hector Peraza.

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

#ifndef __OXDOCKABLEFRAME_H
#define __OXDOCKABLEFRAME_H

#include <xclass/OXTransientFrame.h>
#include <xclass/OXWidget.h>
#include <xclass/OXButton.h>


class OXDockableFrame;


//----------------------------------------------------------------------

class OXDockButton : public OXButton {
public:
  OXDockButton(const OXCompositeFrame *p, int id = 1);
  virtual ~OXDockButton();
  
  virtual int HandleCrossing(XCrossingEvent *event);

protected:
  virtual void DrawBorder();
  virtual void _DoRedraw();
  
  bool _mouseOn;
  unsigned long _normBg, _hiBg;
};

class OXDockHideButton : public OXDockButton {
public:
  OXDockHideButton(const OXCompositeFrame *p);

  void SetAspect(int a) { _aspect = a; _DoRedraw(); }

protected:
  virtual void _DoRedraw();

  int _aspect;
};

class OXUndockedFrame : public OXTransientFrame {
public:
  OXUndockedFrame(const OXWindow *p, OXDockableFrame *dockable);
  virtual ~OXUndockedFrame();
  
  void FixSize();

protected:
  OXDockableFrame *_dockable;
};


//----------------------------------------------------------------------

class OXDockableFrame : public OXCompositeFrame, public OXWidget {
public:
  OXDockableFrame(const OXWindow *p, int id = -1,
                  unsigned int options = HORIZONTAL_FRAME);
  virtual ~OXDockableFrame();

  virtual int  ProcessMessage(OMessage *msg);
  virtual void AddFrame(OXCompositeFrame *f, OLayoutHints *hints);

  void UndockContainer();
  void DockContainer(int del = True);
  
  void HideContainer();
  void ShowContainer();
  
  void EnableUndock(int onoff);
  void EnableHide(int onoff);
  
  void SetWindowName(const char *name);
  
  bool IsUndocked() const { return (_tframe != NULL); }
  bool IsHidden() const { return _hidden; }

  OXCompositeFrame *GetContainer() const { return _container; }

protected:
  OXCompositeFrame *_container, *_buttons;
  OXDockButton *_dockbutton;
  OXDockHideButton *_hidebutton;
  OXUndockedFrame *_tframe;
  OLayoutHints *_l1, *_l2, *_lb, *_cl, *_hints;

  int _hidden, _enableHide, _enableUndock;
  char *_dockName;
};


#endif  // __OXDOCKABLEFRAME_H
