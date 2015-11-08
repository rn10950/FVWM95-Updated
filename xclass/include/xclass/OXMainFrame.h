/**************************************************************************

    This file is part of xclass, a Win95-looking GUI toolkit.
    Copyright (C) 1996, 1997 David Barth, Hector Peraza.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

**************************************************************************/

#ifndef __OXMAINFRAME_H
#define __OXMAINFRAME_H

#include <xclass/OXCompositeFrame.h>


class OFocusManager;
class ODNDmanager;
class OXButton;


//---- MWM Hints stuff

// hints

#define MWM_HINTS_FUNCTIONS     (1L << 0)
#define MWM_HINTS_DECORATIONS   (1L << 1)
#define MWM_HINTS_INPUT_MODE    (1L << 2)

// functions

#define MWM_FUNC_ALL            (1L << 0)
#define MWM_FUNC_RESIZE         (1L << 1)
#define MWM_FUNC_MOVE           (1L << 2)
#define MWM_FUNC_MINIMIZE       (1L << 3)
#define MWM_FUNC_MAXIMIZE       (1L << 4)
#define MWM_FUNC_CLOSE          (1L << 5)       

// input mode

#define MWM_INPUT_MODELESS                   0
#define MWM_INPUT_PRIMARY_APPLICATION_MODAL  1
#define MWM_INPUT_SYSTEM_MODAL               2
#define MWM_INPUT_FULL_APPLICATION_MODAL     3         

// decorations

#define MWM_DECOR_ALL                 (1L << 0)
#define MWM_DECOR_BORDER              (1L << 1)
#define MWM_DECOR_RESIZEH             (1L << 2)
#define MWM_DECOR_TITLE               (1L << 3)
#define MWM_DECOR_MENU                (1L << 4)
#define MWM_DECOR_MINIMIZE            (1L << 5)
#define MWM_DECOR_MAXIMIZE            (1L << 6)

#define PROP_MOTIF_WM_HINTS_ELEMENTS  4
#define PROP_MWM_HINTS_ELEMENTS       PROP_MOTIF_WM_HINTS_ELEMENTS


//----------------------------------------------------------------------
// This class is used to create top-level windows:

class OXMainFrame : public OXCompositeFrame {
public:
  OXMainFrame(const OXWindow *p, int w, int h,
	      unsigned long options = MAIN_FRAME | VERTICAL_FRAME);
  virtual ~OXMainFrame();

  virtual int HandleKey(XKeyEvent *event);
  virtual int HandleConfigureNotify(XConfigureEvent *event);
  virtual int HandleClientMessage(XClientMessageEvent *event);
  virtual int HandleSelection(XSelectionEvent *event);
  virtual int HandleSelectionRequest(XSelectionRequestEvent *event);
  virtual int HandleSelectionClear(XSelectionClearEvent *event);
  virtual int CloseWindow();
  
  virtual void Reconfig();

  void SetWindowName(const char *Name);
  void SetIconName(const char *Name);
  void SetClassHints(const char *ClassName, const char *ResourceName);
  void SetMWMHints(unsigned int value, unsigned int funcs, unsigned int input);
  void SetWMPosition(int x, int y);
  void SetWMSize(int w, int h);
  void SetWMMinSize(int wmin, int hmin);
  void SetWMMaxSize(int wmax, int hmax);
  void SetWMResizeIncrement(int winc, int hinc);
  void SetWMSizeHints(int wmin, int hmin, int wmax, int hmax, 
                      int winc, int hinc);
  void SetWMGravity(int gravity);
  void SetWMState(int state);

  int  RegisterTransient(OXWindow *w);
  int  UnregisterTransient(OXWindow *w);

  int  BindKey(const OXWindow *w, int keycode, int modifier) const;
  void RemoveBind(const OXWindow *w, int keycode, int modifier) const;

  int  RegisterButton(OXButton *b);
  int  UnregisterButton(OXButton *b);

  void SetDefaultAcceptButton(OXButton *b);
  void SetDefaultCancelButton(OXButton *b);

  void SetFocusOwner(OXFrame *f);
  
  ODNDmanager *GetDNDmanager() const { return _dndManager; }

protected:
  virtual void _GotFocus();

  typedef struct _MWMHintsProperty {
    CARD32 flags;
    CARD32 functions;
    CARD32 decorations;
    INT32  inputMode;
  } MWMHintsProperty;
  XSizeHints _sizeHints;
  OXSList *_bindlist, *_tlist, *_buttonlist;

  OFocusManager *_focusMgr;
  ODNDmanager *_dndManager;
  OXButton *_defaultAccept, *_defaultCancel, *_currentAccept;
};


#endif  // __OXMAINFRAME_H
