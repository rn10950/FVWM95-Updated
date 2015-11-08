/**************************************************************************

    This file is part of taskbar.
    Copyright (C) 2000, Hector Peraza.

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

#ifndef __OXTASKBAR_H
#define __OXTASKBAR_H

#include "fvwmdefs.h"

#include <xclass/OXSList.h>
#include <xclass/OXClient.h>
#include <xclass/OXWindow.h>
#include <xclass/OXMainFrame.h>
#include <xclass/OXMsgBox.h>
#include <xclass/OXAboutDialog.h>
#include <xclass/OString.h>
#include <xclass/OXSlider.h>
#include <xclass/utils.h>
#include <xclass/version.h>


//XErrorHandler ErrorHandler(Display *d, XErrorEvent *event);

class OTimer;
class OFileHandler;
class OXTButton;
class OIniFile;
struct SListSwallowedElt;

//----------------------------------------------------------------------

struct SListPluginElt {
public:
  int  id;
  void *dlhandle;
  OXFrame *frame;
};

//----------------------------------------------------------------------

class OXTaskBar : public OXMainFrame {
public:
  OXTaskBar(const OXWindow *p, char *modname, int inp, int outp);
  virtual ~OXTaskBar();

  virtual int ProcessMessage(OMessage *msg);
  virtual int HandleConfigureNotify(XConfigureEvent *event);
  virtual int HandleFileEvent(OFileHandler *fh, unsigned int mask);
  virtual int HandleTimer(OTimer *t);
  virtual int HandleIdleEvent(OIdleHandler *ih);
  virtual int CloseWindow();

  void ReadIniFile(char *filename);
  void LoadPlugins(char *inifile);
  void LoadPlugin(char *plgname, OIniFile *rcfile);

  int  ReadFvwmPacket(unsigned long *header, unsigned long **body);
  void SendFvwmPipe(char *message, unsigned long window = 0);
  void SendFvwmText(char *message, unsigned long window = 0);
  void ProcessFvwmMessage(unsigned long type, unsigned long *body);

  void SwitchDesktops();

  void LaunchSwallow(char *inifile);
  void Swallow(unsigned long *body);
  void ConstrainSize(XSizeHints *hints, int *widthp, int *heightp);

  void ShowButton(int num);
  void HideButton(int num);
  OXTButton *AddButton(char *title, const OPicture *p, int state, int num);
  void RemoveButton(int num);
  void RadioButton(int num, int state = BUTTON_ENGAGED);
  OXTButton *FindButton(long wid);
  SListSwallowedElt *FindSwallowed(char *name);

  void AdjustWindow(int width, int height);
  void Warp(int x, int y, int new_width, int new_height);
  void Reveal();
  void Hide();

protected:
  OXTButton *_start;
  OXCompositeFrame *_buttonArray, *_tray;

  //OXPopupMenu *_startMenu, **_subMenu;

  FILE *_console;
  int _pipein, _pipeout;  // file descriptors for Fvwm message pipe

  int _scrWidth, _scrHeight, _midLine, _decorBorder, _rows, _rowHeight;

  int _belayHide, 
      _useSkipList, _useIconNames, _showTransients,
      _autoStick, _autoHide, _highlightFocus,
      _deskOnly, _deskNumber, _firstDeskMsg;
  
  OXFont *_normFont, *_hiFont;
  OLayoutHints *_stl, *_arl, *_drl, *_pl;
  
  OTimer *_hideTimer;
  OFileHandler *_fvwmPipe;
  
  char *_moduleName, *_clickAction[3], *_enterAction,
       *_startName, *_startIconName, *_startPopup, 
       *_geometry, *_fontString, *_selfontString,
       *_iconDir, *_pluginsDir;

  OXSList *_buttons, *_plugins, *_swallowed;
};

#endif  // __OXTASKBAR_H
