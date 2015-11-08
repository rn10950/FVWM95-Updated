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

#include <FVWMconfig.h>

#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <limits.h>
#include <dlfcn.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xproto.h>
#include <X11/cursorfont.h>

#include <fvwm/module.h>
#include <fvwm/version.h>

#include <xclass/OXClient.h>
#include <xclass/OXWindow.h>
#include <xclass/OFileHandler.h>
#include <xclass/OIdleHandler.h>
#include <xclass/OFocusManager.h>
#include <xclass/OResourcePool.h>
#include <xclass/OXMsgBox.h>
#include <xclass/OXAboutDialog.h>
#include <xclass/OTimer.h>
#include <xclass/OIniFile.h>

#include "OXTaskBar.h"
#include "OXTButton.h"
#include "OXSwallowedFrame.h"
#include "OTButtonLayout.h"

#include "envvar.h"
#include "plugins.h"

#define CLICK_ACTION_1 "Iconify -1, Raise, Focus"
#define CLICK_ACTION_2 "Iconify +1, Lower"
#define CLICK_ACTION_3 "Nop"
#define ENTER_ACTION   "Nop"

#ifndef PLUGINS_DIR
#define PLUGINS_DIR "/var/X11R6/lib/fvwm95-2/plugins"
#endif

#undef DEBUG_PLUGINS

#undef USE_FVWM98_WORKSPACE
#define USE_WORKSPACE_EXTENSION

#ifndef isblank
#define isblank(c) (((c) == ' ') || ((c) == '\t'))
#endif

//----------------------------------------------------------------------

extern void ConsoleMessage(char *fmt, ...);

void UpdateString(char **dst, char *src) {
  int  len;
  char *start;

  if (*dst) delete[] *dst;

  while ((isspace(*src) && (*src != '\n')) && *src) ++src;

  len = 0;
  start = src;
  while ((*src != '\n') && *src) ++len, ++src;

  --src;
  while (isspace(*src) && *src && (len > 0)) --len, --src;

  *dst = new char[len+1];
  strncpy(*dst, start, len);
  (*dst)[len] = 0;
}


//----------------------------------------------------------------------

OXTaskBar::OXTaskBar(const OXWindow *p, char *modname, int inp, int outp) :
  OXMainFrame(p, 10, 10) {
    OFontMetrics fm;
    int win_width, win_height;
    char *rcfile;

    delete _focusMgr;
    _focusMgr = NULL;
    RemoveInput(FocusChangeMask);

    SetLayoutManager(new OHorizontalLayout(this));

    _pipein = inp;
    _pipeout = outp;

    _moduleName = StrDup(modname);

    _buttons   = new OXSList(GetDisplay(), "Buttons");
    _plugins   = new OXSList(GetDisplay(), "Plug-ins");
    _swallowed = new OXSList(GetDisplay(), "Swallowed");

    // Set the defaults

    _startName     = StrDup("Start");
    _startPopup    = StrDup("StartMenu");
    _startIconName = StrDup("mini-start.xpm");

    _belayHide      = False;
    _useSkipList    = False;
    _useIconNames   = False;
    _showTransients = False;
    _autoStick      = False;
    _autoHide       = False;
    _highlightFocus = False;
    _deskOnly       = False;

    _clickAction[0] = StrDup(CLICK_ACTION_1);
    _clickAction[1] = StrDup(CLICK_ACTION_2);
    _clickAction[2] = StrDup(CLICK_ACTION_3);
    _enterAction    = StrDup(ENTER_ACTION);

    _fontString    = StrDup("fixed");
    _selfontString = NULL;

    _geometry = NULL;

    _rows = 1;
    _rowHeight  = 16;
    _deskNumber = 0;
    _deskOnly   = False;

    _iconDir = NULL;
    _pluginsDir = StrDup(PLUGINS_DIR);

    // Read in the rc file

    rcfile = _client->GetResourcePool()->FindIniFile("taskbarrc", INI_READ);

    if (rcfile) ReadIniFile(rcfile);

    _scrWidth  = _client->GetDisplayWidth();
    _scrHeight = _client->GetDisplayHeight();

    _midLine = (int) (_scrHeight >> 1);

    if (!_selfontString) _selfontString = StrDup(_fontString);

    if ((_normFont = _client->GetFont(_fontString)) == NULL) {
      if ((_normFont = _client->GetFont("fixed")) == NULL) {
        ConsoleMessage("Couldn't load fixed font, exiting...\n");
        FatalError("Couldn't load fixed font, exiting.");
      }
    }
    if ((_hiFont = _client->GetFont(_selfontString)) == NULL) {
      if ((_hiFont = _client->GetFont("fixed")) == NULL) {
        ConsoleMessage("Couldn't load fixed font, exiting...\n");
        FatalError("Couldn't load fixed font, exiting.");
      }
    }

    _hiFont->GetFontMetrics(&fm);

    int fontheight = fm.ascent + fm.descent;

    _rowHeight = fontheight + 7;

    _decorBorder = 4; // default decoration border width

    win_height = _rowHeight;
    win_width  = _scrWidth - (_decorBorder << 1);

    int ret = XParseGeometry(_geometry, &_sizeHints.x, &_sizeHints.y,
                             (unsigned int *) &_sizeHints.width,
                             (unsigned int *) &_sizeHints.height);

    _sizeHints.flags = USPosition | PPosition | USSize | PSize |
                       PResizeInc | PMinSize | PMaxSize | PBaseSize |
                       PWinGravity;

    if (ret & YNegative)
      _sizeHints.y = _scrHeight - 
                     (_autoHide ? 1 : win_height + (_decorBorder << 1));
    else 
      _sizeHints.y = _autoHide ? 1 - win_height : 0;
   
    _sizeHints.x           = _decorBorder;
    _sizeHints.width       = win_width;
    _sizeHints.height      = _rowHeight;
    _sizeHints.width_inc   = win_width;
    _sizeHints.height_inc  = _rowHeight + 2;
    _sizeHints.min_width   = win_width;
    _sizeHints.min_height  = _rowHeight;
    _sizeHints.min_height  = win_height;
    _sizeHints.max_width   = win_width;
    _sizeHints.max_height  = _rowHeight + 7 * (_rowHeight + 2) + 1;
    _sizeHints.base_width  = win_width;
    _sizeHints.base_height = _rowHeight;
    _sizeHints.win_gravity = NorthWestGravity;

    Move(_sizeHints.x, _sizeHints.y);

    XSetWMNormalHints(GetDisplay(), _id, &_sizeHints);
   
    if (_iconDir)
      _client->GetResourcePool()->GetPicturePool()->SetPath(_iconDir);

    _stl = new OLayoutHints(LHINTS_TOP | LHINTS_LEFT, 0, 4, 0, 0);
    _start = new OXTButton(this, -1, _hiFont, _hiFont, _startName,
                           _client->GetPicture(_startIconName));
    _start->AutoTip(False);
    _start->SetTip("Click Here to Start");
    AddFrame(_start, _stl);

    _arl = new OLayoutHints(LHINTS_EXPAND_Y | LHINTS_EXPAND_X, 0, 0, 0, 0);
    _buttonArray = new OXCompositeFrame(this, 10, 10);
    _buttonArray->SetLayoutManager(new OTButtonLayout(_buttonArray,
                                                      _rowHeight, _rows));
    AddFrame(_buttonArray, _arl);

    _drl = new OLayoutHints(LHINTS_TOP | LHINTS_RIGHT, 10, 0, 0, 0);
    _tray = new OXHorizontalFrame(this, 100, _rowHeight, 
                                  FIXED_HEIGHT | SUNKEN_FRAME);
    AddFrame(_tray, _drl);

    _pl = new OLayoutHints(LHINTS_RIGHT | LHINTS_CENTER_Y, 1, 1, 0, 0);

    if (rcfile) LoadPlugins(rcfile);
    if (_plugins->NoOfItems() == 0) HideFrame(_tray);

    if (rcfile) LaunchSwallow(rcfile);

    SetWindowName(_moduleName);
    SetIconName(_moduleName);

    _firstDeskMsg = True; // we need to skip the very first M_NEW_DESK message

    // Set fvwm Message Mask
    char tmp[50];
    int mask = M_ADD_WINDOW |
               M_CONFIGURE_WINDOW |
               M_DESTROY_WINDOW |
               M_WINDOW_NAME |
               M_ICON_NAME |
               M_RES_NAME |
               M_DEICONIFY |
               M_ICONIFY |
               M_END_WINDOWLIST |
               M_FOCUS_CHANGE |
               M_FUNCTION_END |
               M_CONFIG_INFO |
               M_MINI_ICON |
               M_SCROLLREGION |
               M_NEW_DESK;

    sprintf(tmp, "SET_MASK %lu\n", mask);
    SendFvwmText(tmp);

    // Request a list of all windows, wait for ConfigureWindow packets.

    SendFvwmPipe("Send_WindowList");

    _hideTimer = NULL;

    _fvwmPipe = new OFileHandler(this, _pipein, XCM_READABLE);

    Resize(win_width, _rowHeight);
    Layout();
    MapSubwindows();
    if (_plugins->NoOfItems() == 0) HideFrame(_tray);
    MapWindow();

    if (rcfile) delete[] rcfile;
}

OXTaskBar::~OXTaskBar() {
  const OXSNode *e;

  delete _stl;
  delete _arl;
  delete _drl;
  delete _pl;
  if (_moduleName) delete[] _moduleName;

  while (e = _plugins->GetHead()) {
    SListPluginElt *p = (SListPluginElt *) e->data;
    // not neccessary: p->frame->DestroyWindow();
    _plugins->Remove(p->id);
    dlclose(p->dlhandle);
    delete p;
  }
  delete _plugins;

  while (e = _swallowed->GetHead()) {
    SListSwallowedElt *s = (SListSwallowedElt *) e->data;
    _swallowed->Remove(s->id);
    XKillClient(GetDisplay(), s->frame->GetId());
    XSync(GetDisplay(), False);
    //s->frame->DestroyWindow();
    delete[] s->name;
    delete s;
  }
  delete _swallowed;
}

int OXTaskBar::CloseWindow() {
  // We should not exit in response to an accidental Alt-F4 or other
  // WM close action. Perhaps we should ask the user for a confirmation?
  return False;
}


//----------------------------------------------------------------------

// Communication with fvwm...

int OXTaskBar::ReadFvwmPacket(unsigned long *header, unsigned long **body) {
  int count, total, count2, body_length;
  char *cbody;
  extern void DeadPipe(int);

  if ((count = read(_pipein, header,
                    HEADER_SIZE*sizeof(unsigned long))) > 0) {
    if (header[0] == START_FLAG) {
      body_length = header[2] - HEADER_SIZE;
      *body = new unsigned long [body_length];
      cbody = (char *) *body;
      total = 0;
      while (total < body_length * sizeof(unsigned long)) {
        if ((count2 = read(_pipein, &cbody[total],
                      body_length * sizeof(unsigned long) - total)) > 0) {
          total += count2;
        } else if (count2 < 0) {
          DeadPipe(1);
        }
      }
    } else {
      count = 0;
    }
  }
  if (count <= 0) DeadPipe(1);
  return count;
}

// SendFvwmPipe - Send a message to fvwm 

void OXTaskBar::SendFvwmPipe(char *message, unsigned long window) {
  int  len;
  char *hold, *temp, *temp_msg;

  hold = message;
  
  while (1) {
    temp = strchr(hold, ',');
    if (temp != NULL) {
      temp_msg = new char[temp-hold+1];
      strncpy(temp_msg, hold, temp-hold);
      temp_msg[temp-hold] = '\0';
      hold = temp+1;
    } else {
      temp_msg = hold;
    }

    write(_pipeout, &window, sizeof(unsigned long));
    
    len = strlen(temp_msg);
    write(_pipeout, &len, sizeof(int));
    write(_pipeout, temp_msg, len);

    // keep going
    len = 1;
    write(_pipeout, &len, sizeof(int));

    if (temp_msg != hold)
      delete[] temp_msg;
    else
      break;
  }
}

void OXTaskBar::SendFvwmText(char *message, unsigned long window) {
  int w;
  
  if (message) {
    write(_pipeout, &window, sizeof(unsigned long));
      
    w = strlen(message);
    write(_pipeout, &w, sizeof(int));
    write(_pipeout, message, w);
      
    // keep going
    w = 1;
    write(_pipeout, &w, sizeof(int));
  }
}

// ProcessFvwmMessage - Process the message coming from Fvwm

void OXTaskBar::ProcessFvwmMessage(unsigned long type, unsigned long *body) {
  int  i;
  long flags;
  char *string;
  OTPicture *p;
  OXTButton *b;
  SListSwallowedElt *s;

  switch (type) {
  case M_FOCUS_CHANGE:
    if (body[0] == _id) break;
    b = FindButton(body[0]);
    if (b) {
      i = b->WidgetID();
      if (b->GetWindowFlags() & ICONIFIED) i = -1;
    } else {
      i = -1;
    }
    RadioButton(i);
    break;

  case M_ADD_WINDOW:
  case M_CONFIGURE_WINDOW:
    // Matched only at startup when the default border width and
    // actual border width differ.
    if (body[0] == _id) {
      if (_decorBorder != (int) body[10]) {
        int nx, ny;

	nx = _decorBorder = (int) body[10];

        if (_y > _midLine)
          ny = _scrHeight - (_autoHide ? 1 : _h + _decorBorder);
        else 
          ny = _autoHide ? 1 - _h : _decorBorder;

        MoveResize(nx, ny, _scrWidth - (_decorBorder << 1), _h);
      }

#ifdef USE_WORKSPACE_EXTENSION

#ifdef USE_FVWM98_WORKSPACE
      int wsX0 = 0;
      int wsY0 = 0;
      int wsX1 = _scrWidth - 1;
      int wsY1 = _scrHeight - 1;

      if (_y > _midLine)
        wsY1 -= (_autoHide ? 1 : _h + _decorBorder * 2 + 1);
      else
        wsY0 += (_autoHide ? 1 : _h + _decorBorder * 2 + 1);

      char tmp[255];
      sprintf(tmp, "SetWorkspace %i %i %i %i", wsX0, wsY0, wsX1, wsY1);
      SendFvwmText(tmp, _id);
#else
      int wsX = 0;
      int wsY = 0;
      int wsW = _scrWidth;
      int wsH = _scrHeight;

      if (_y < _midLine)
        wsY = (_autoHide ? 1 : _h + _decorBorder * 2);

      wsH -= (_autoHide ? 1 : _h + _decorBorder * 2);

      char tmp[255];
      sprintf(tmp, "SetWorkspace %i %i %i %i", wsX, wsY, wsW, wsH);
      SendFvwmText(tmp, _id);
#endif

#endif

      break;
    }

    if (!_showTransients && (body[8] & TRANSIENT)) break;

    b = FindButton(body[0]);
    if (b) {
      i = b->WidgetID();
      if (_deskOnly) {
        if (_deskNumber != b->GetDesk() && _deskNumber == body[7]) {
          // window moving to current desktop
          ShowButton(i);
        }
        if (_deskNumber == b->GetDesk() && _deskNumber != body[7]) {
          // window moving to another desktop
          HideButton(i);
        }
      }
      b->SetDesk(body[7]);
      b->SetWindowFlags(body[8]);
    } else if (!(body[8] & WINDOWLISTSKIP) || !_useSkipList) {
      b = AddButton(NULL, NULL, BUTTON_UP, body[0]);
      b->SetDesk(body[7]);
      b->SetWindowFlags(body[8]);
      if (_deskOnly && _deskNumber != body[7]) HideButton(b->WidgetID());
    }
    break;

  case M_DESTROY_WINDOW:
    b = FindButton(body[0]);
    if (b) RemoveButton(b->WidgetID());
    break;

  case M_MINI_ICON:
    b = FindButton(body[0]);
    if (b) {
      OTPicture *p = new OTPicture((Pixmap) body[1], (Pixmap) body[2],
                                   body[3], body[4], body[5]);
      delete (OTPicture *) b->GetIcon();
      b->SetIcon(p);
    }
    break;

  case M_WINDOW_NAME:
  case M_ICON_NAME:
    if ((type == M_ICON_NAME   && !_useIconNames) || 
        (type == M_WINDOW_NAME &&  _useIconNames)) break;
    string = (char *) &body[3];
    s = FindSwallowed(string);
    if (s && !s->frame) {
      Swallow(body);
      break;
    }
    b = FindButton(body[0]);
    if (b) {
      b->SetName(string);
      if (!_deskOnly || b->GetDesk() == _deskNumber) {
        ShowButton(b->WidgetID());
      }
    }
    break;

  case M_DEICONIFY:
  case M_ICONIFY:
    b = FindButton(body[0]);
    if (b) {
      flags = b->GetWindowFlags();
      if (type == M_DEICONIFY && !(flags & ICONIFIED)) break;
      if (type == M_ICONIFY   &&   flags & ICONIFIED) break;
      flags ^= ICONIFIED;
      if (type == M_ICONIFY) RadioButton(-1, BUTTON_UP);
      b->SetWindowFlags(flags);
    }
    break;

  case M_END_WINDOWLIST:
    MapRaised();
    break;

  case M_FUNCTION_END:
    _start->SetState(BUTTON_UP);
    
    // We don't want the taskbar to hide
    // after a Focus or Iconify function

    if (_autoHide && !_belayHide) {
      if (_hideTimer) delete _hideTimer;
      _hideTimer = new OTimer(this, 1000);
    }
    break;

  // Added a new Fvwm event because scrolling regions interfere 
  // with EnterNotify event when taskbar is hidden.
  case M_SCROLLREGION:
    if (_autoHide && ((_y <  _midLine && body[1] < 4) ||
                      (_y >= _midLine && body[1] > _scrHeight-4)))
      Reveal();
    break;

  case M_NEW_DESK:
    _deskNumber = body[0];
    if (!_firstDeskMsg && _deskOnly)
      SwitchDesktops();
    else
      _firstDeskMsg = False;
    break;

  case M_NEW_PAGE:
    break;

  }
}


//----------------------------------------------------------------------

// Handling of xclass events...

int OXTaskBar::HandleFileEvent(OFileHandler *fh, unsigned int mask) {
  unsigned long header[HEADER_SIZE], *body;

  if (fh == _fvwmPipe) {
    if (ReadFvwmPacket(header, &body) > 0) {
      ProcessFvwmMessage(header[1], body);
      delete[] body;
    }
    return True;
  }
  return False;
}

int OXTaskBar::HandleTimer(OTimer *t) {
  if (t == _hideTimer) { 
    Hide();
    delete _hideTimer;
    _hideTimer = NULL;
  } else {
    return False;
  }
  return True;
}

int OXTaskBar::HandleIdleEvent(OIdleHandler *ih) {
  const OXSNode *e;
  
  for (e = _plugins->GetHead(); e != NULL; ) {
    SListPluginElt *p = (SListPluginElt *) e->data;
    e = e->next;
    if (!p->frame) {
      _plugins->Remove(p->id);
      //delete p->frame;
      dlclose(p->dlhandle);
      delete p;
    }
  }

  delete ih;

  return True;
}

int OXTaskBar::HandleConfigureNotify(XConfigureEvent *event) {

  if ((event->width != _w) || (event->height != _h)) {
    AdjustWindow(event->width, event->height);
    if (_autoStick) Warp(_x, _y, event->width, event->height);
  } else if ((event->x != _x) || (event->y != _y)) {
    if (_autoStick) {
      _x = event->x;
      _y = event->y;
      Warp(event->x, event->y, _w, _h);
    }
  }
  return OXMainFrame::HandleConfigureNotify(event);
}

int OXTaskBar::ProcessMessage(OMessage *msg) {
  OXTButton *b;
  OPluginMessage *pmsg;
  OWidgetMessage *wmsg;
  OTButtonMessage *tbmsg;
  const OXSNode *e;
  int x, y, wid;

  switch (msg->type) {

    case MSG_TBUTTON:
      tbmsg = (OTButtonMessage *) msg;
      switch (msg->action) {
        case MSG_CLICK:
          if (tbmsg->id == -1 && tbmsg->button != -1) {
            // Start button
	    _start->SetState(BUTTON_DOWN);
            x = _x;
            if (_y < _midLine) {
              // bar in top half of the screen
              y = _y + _rowHeight;
            } else {
              // bar in bottom of the screen
              y = _y - _scrHeight;
            }
            char tmp[256];
            sprintf(tmp, "Popup %s %d %d", _startPopup, x, y);
            SendFvwmPipe(tmp);
          } else {
            b = FindButton(tbmsg->id);
            if (b) {
              wid = b->WidgetID();
              if (tbmsg->button == -1) {
                RadioButton(wid, BUTTON_DOWN);
              } else {

                // Fvwm usually defers the assignment of the focus to a window
                // until it receives a ButtonRelease event. Here we have to
                // send a synthetic one since the taskbar buttons eat them
                // before they can be delivered to fvwm. Without this we end up
                // with fvwm failing to give the focus to some windows after
                // the taskbar button is released.

                XEvent ev;

                ev.type = ButtonRelease;
                ev.xany.type = ButtonRelease;
                ev.xbutton.window = _id;
                ev.xbutton.root = _parent->GetId();
                ev.xbutton.subwindow = _id;
                ev.xbutton.time = CurrentTime;
                ev.xbutton.button = 1;

                XSendEvent(GetDisplay(), _parent->GetId(), True,
                           ButtonReleaseMask, &ev);
                XSync(GetDisplay(), False);

                RadioButton(wid, BUTTON_DOWN);
                _belayHide = True;    // Don't AutoHide when function ends
                SendFvwmPipe(_clickAction[tbmsg->button-1], wid);
              }

              //if (_highlightFocus) {
              //  RadioButton(wid, BUTTON_DOWN);
              //  SendFvwmPipe("Focus 0", wid);
              //}
            }
          }
          break;

        default:
          break;
      }
      break;

    case MSG_MENU:
      wmsg = (OWidgetMessage *) msg;
      switch (msg->action) {

        case MSG_CLICK:
          switch (wmsg->id) {
            case 888:
              {
              OAboutInfo info;

              info.wname = "About TaskBar";
              info.title = "TaskBar\n"
                           "A taskbar for fvwm95\n"
                           "Compiled with xclass version "
                           XCLASS_VERSION;
              info.copyright = "Copyleft © 1998-2000 by H. Peraza";
              info.text = "This program is free software; you can "
                          "redistribute it and/or modify it under the "
                          "terms of the GNU General Public License.";

              new OXAboutDialog(_client->GetRoot(), this, &info);
              }
              break;

            default:
              break;
          }
          break;

        default:
          break;
      }
      break;

    case MSG_PLUGIN:
      pmsg = (OPluginMessage *) msg;
      switch (msg->action) {
        case PLUGIN_CHANGED:
          Layout();
          break;

        case PLUGIN_DOACTION:
          SendFvwmPipe((char *) pmsg->command->GetString());
          break;

        case PLUGIN_HIDE:
          e = _plugins->GetNode(pmsg->id);
          if (e) {
            SListPluginElt *p = (SListPluginElt *) e->data;
            _tray->HideFrame(p->frame);
            Layout();
          }
          break;

        case PLUGIN_SHOW:
          e = _plugins->GetNode(pmsg->id);
          if (e) {
            SListPluginElt *p = (SListPluginElt *) e->data;
            _tray->ShowFrame(p->frame);
            Layout();
          }
          break;

        case PLUGIN_UNLOAD:
          e = _plugins->GetNode(pmsg->id);
          if (e) {
            SListPluginElt *p = (SListPluginElt *) e->data;
            OXFrame *f = (OXFrame *) e->data;
            _tray->RemoveFrame(p->frame);
            p->frame->DestroyWindow();
            delete p->frame;

            // We can't close the dl handler right here, since we're still
            // inside the caller's plugin context: if we close the handle,
            // we would segfault on return from ProcessMessage. Instead
            // we'll schedule an idle event to close the handle safely. Note
            // that we don't need to save the idle handle object, as it is
            // kept in OXClient's internal list.
#if 0
            dlclose(p->dlhandle);
            _plugins->Remove(pmsg->id);
            delete p;
#else
            p->frame = NULL;
            new OIdleHandler(this);
#endif
            Layout();
          }
          break;

        default:
          break;
      }
      break;

    case MSG_SWALLOWED:
      wmsg = (OWidgetMessage *) msg;
      switch (msg->action) {
        case SWALLOWED_CHANGED:
          Layout();
          break;

        case SWALLOWED_HIDE:
          e = _swallowed->GetNode(wmsg->id);
          if (e) {
            SListSwallowedElt *s = (SListSwallowedElt *) e->data;
            _tray->HideFrame(s->frame);
            Layout();
          }
          break;

        case SWALLOWED_SHOW:
          e = _swallowed->GetNode(wmsg->id);
          if (e) {
            SListSwallowedElt *s = (SListSwallowedElt *) e->data;
            _tray->ShowFrame(s->frame);
            Layout();
          }
          break;

        case SWALLOWED_KILLED:
          e = _swallowed->GetNode(wmsg->id);
          if (e) {
            SListSwallowedElt *s = (SListSwallowedElt *) e->data;
            _tray->RemoveFrame(s->frame);
            delete s->frame;
            delete[] s->name;
            _swallowed->Remove(s->id);
            delete s;
            Layout();
          }
          break;

        default:
          break;
      }
      break;

    default:
      break;

  } // type

  return True;
}


//----------------------------------------------------------------------

void OXTaskBar::ReadIniFile(char *filename) {
  char line[1024], arg[256];
  int  i, j, n;

  OIniFile rcfile(filename, INI_READ);

  while (rcfile.GetNext(line)) {
    if (strcasecmp(line, "defaults") == 0) {

      if (rcfile.GetItem("font", arg))
        UpdateString(&_fontString, arg);

      if (rcfile.GetItem("selfont", arg))
        UpdateString(&_selfontString, arg);

      if (rcfile.GetItem("font", arg))
        UpdateString(&_fontString, arg);

      if (rcfile.GetItem("geometry", arg))
        UpdateString(&_geometry, arg);

      if (rcfile.GetItem("fvwm_action_click1", arg))
        UpdateString(&_clickAction[0], arg);

      if (rcfile.GetItem("fvwm_action_click2", arg))
        UpdateString(&_clickAction[1], arg);

      if (rcfile.GetItem("fvwm_action_click3", arg))
        UpdateString(&_clickAction[2], arg);

      _useSkipList = rcfile.GetBool("use_skip_list");

      _autoStick = rcfile.GetBool("auto_stick");

      _autoHide = rcfile.GetBool("auto_hide");
      if (_autoHide) _autoStick = True;

      _useIconNames = rcfile.GetBool("use_icon_names");

      _showTransients = rcfile.GetBool("show_transients");

      _deskOnly = rcfile.GetBool("desk_only");

      _highlightFocus = rcfile.GetBool("highlight_focus");

      if (rcfile.GetItem("icon_dir", arg)) {
        if (_iconDir) delete[] _iconDir;
        _iconDir = envDupExpand(arg);
      }

      if (rcfile.GetItem("plugins_dir", arg)) {
        if (_pluginsDir) delete[] _pluginsDir;
        _pluginsDir = envDupExpand(arg);
      }

      // Start button:

      if (rcfile.GetItem("start_button_name", arg))
        UpdateString(&_startName, arg);

      if (rcfile.GetItem("start_button_icon", arg))
        UpdateString(&_startIconName, arg);

      if (rcfile.GetItem("start_fvwm_menu", arg))
        UpdateString(&_startPopup, arg);

    }
  }
}


//----------------------------------------------------------------------

// Loadable plugin support...

void OXTaskBar::LoadPlugin(char *plgname, OIniFile *rcfile) {
  const char *error;
  static int count = 1;
  char msg[256], fullName[PATH_MAX];

  sprintf(fullName, "%s/%s", _pluginsDir, plgname);

  OString stitle("TaskBar Error");

  void *dlhandle = dlopen(fullName, RTLD_NOW);
  if (!dlhandle) {
    error = dlerror();
    sprintf(msg, "Could not open plugin \"%s\"\n%s", fullName, error);
    ConsoleMessage("Error opening plugin \"%s\": %s\n", plgname, error);
    new OXMsgBox(_client->GetRoot(), NULL, &stitle, new OString(msg),
                 MB_ICONSTOP, ID_OK);
    return;
  }

  OXFrame *(*dlcreate)(const OXWindow *, OComponent *, OIniFile *, int) =
       (OXFrame *(*)(const OXWindow *, OComponent *, OIniFile *, int))
       dlsym(dlhandle, "CreatePlugin");
  error = dlerror();
  if (error) {
    sprintf(msg, "Could not load plugin \"%s\"\n%s", fullName, error);
    ConsoleMessage("Error loading plugin \"%s\": %s\n", plgname, error);
    new OXMsgBox(_client->GetRoot(), NULL, &stitle, new OString(msg),
                 MB_ICONSTOP, ID_OK);
    return;
  }

  OXFrame *plugin = (*dlcreate)(_tray, this, rcfile, count);
  _tray->AddFrame(plugin, _pl);
  Layout();

  SListPluginElt *p = new SListPluginElt;

  p->id = count;
  p->frame = plugin;
  p->dlhandle = dlhandle;

  _plugins->Add(count++, (XPointer) p);

#ifdef DEBUG_PLUGINS
  ConsoleMessage("OXTaskBar::LoadPlugin(): plugin \"%s\" loaded.\n", plgname);
#endif
}

void OXTaskBar::LoadPlugins(char *inifile) {
  char *p, line[1024], arg[256];

  OIniFile rcfile(inifile, INI_READ);

  while (rcfile.GetNext(line)) {
    if (strcasecmp(line, "plugin") == 0)
      if (rcfile.GetItem("file", arg)) LoadPlugin(arg, &rcfile);
  }
}


//----------------------------------------------------------------------

// Swallowed window support...

void OXTaskBar::LaunchSwallow(char *inifile) {
  int  i, j;
  char line[1024], arg[256];
  static int count = 1;

  OIniFile rcfile(inifile, INI_READ);

  while (rcfile.GetNext(line)) {
    if (strcasecmp(line, "swallow") == 0) {
      char *cmd = NULL, *wname = NULL;
      int ax = 1, ay = 1;

      if (rcfile.GetItem("command", arg))
        cmd = StrDup(arg);

      if (rcfile.GetItem("window_name", arg))
        wname = StrDup(arg);

      if (rcfile.GetItem("aspect", arg))
        if (sscanf(arg, "%dx%d", &ax, &ay) != 2) ax = ay = 1;

      if (cmd) {
        if (!wname) {
          if (strncasecmp(cmd, "Exec", 4) == 0) {
            j = 4;
          } else if (strncasecmp(cmd, "Module", 6) == 0) {
            j = 6;
          } else {
            j = 0;
          }
          for (i = j; cmd[i] && isblank(cmd[i]); ++i) {}
          for (j = i; cmd[j] && !isblank(cmd[j]); ++j) {}
          if (j > i) {
            wname = new char[j-i+1];
            strncpy(wname, &cmd[i], j-i);
            wname[j-i] = '\0';
          }
        }
        if (wname) {
          // tell fvwm to launch the application or module for us
#ifdef DEBUG_PLUGINS
          ConsoleMessage("Swallow: \"%s\"\n", cmd);
#endif
          SendFvwmPipe(cmd);
#ifdef DEBUG_PLUGINS
          ConsoleMessage("Looking for window \"%s\"\n", wname);
#endif
          SListSwallowedElt *s = new SListSwallowedElt;
          s->id = count++;
          s->name = wname;
          wname = NULL;     // now the strings belongs to s
          s->frame = NULL;
          s->aspect_x = ax;
          s->aspect_y = ay;
          _swallowed->Add(s->id, (XPointer) s);
        }
      }

      if (cmd) delete[] cmd;
      if (wname) delete[] wname;

    }
  }

}

// Swallow a process window

void OXTaskBar::Swallow(unsigned long *body) {
  OXFrame *f;
  SListSwallowedElt *s;
  Window win;
  XSizeHints hints;
  long supplied;
  int  w, h;

  win = (Window) body[0];

  s = FindSwallowed((char *) &body[3]);
  if (!s) return;
  if (s->frame) return;

  if (!XGetWMNormalHints(GetDisplay(), win, &hints, &supplied))
    hints.flags = 0;

  // Swallow the window
  XUnmapWindow(GetDisplay(), win);

  XSetWindowBorderWidth(GetDisplay(), win, 0);

  h = _tray->GetHeight() - 4;
  w = h * s->aspect_x / s->aspect_y;
  ConstrainSize(&hints, &w, &h);

  XReparentWindow(GetDisplay(), win, _tray->GetId(), 0, 0);

  f = new OXSwallowedFrame(_client, win, _tray, this, s->id);
  _tray->AddFrame(f, _pl);
  Layout();
  
  // Do not swallow it next time
  s->frame = f;

  f->Resize(w, h);
  f->MapWindow();

#ifdef DEBUG_PLUGINS
  ConsoleMessage("Swallowed: %s\n", (char *) &body[3]);
#endif

  Layout();
}

SListSwallowedElt *OXTaskBar::FindSwallowed(char *name) {
  const OXSNode *e;
  SListSwallowedElt *s;
  int i;

  for (e = _swallowed->GetHead(); e != NULL; e = e->next) {
    s = (SListSwallowedElt *) e->data;
    if (strcmp(s->name, name) == 0) return s;
  }
  return NULL;
}

// Adjust the given width and height to account for the constraints
// imposed by size hints
//
// The general algorithm, especially the aspect ratio stuff, is borrowed
// from uwm's CheckConsistency routine.

void OXTaskBar::ConstrainSize(XSizeHints *hints, int *widthp, int *heightp) {

#define makemult(a,b) ((b == 1) ? (a) : (((int)((a)/(b))) * (b)))
#define _min(a,b) (((a) < (b)) ? (a) : (b))

  int minWidth, minHeight, maxWidth, maxHeight, xinc, yinc, delta;
  int baseWidth, baseHeight;
  int dwidth = *widthp, dheight = *heightp;

  if (hints->flags & PMinSize) {

    minWidth = hints->min_width;
    minHeight = hints->min_height;

    if (hints->flags & PBaseSize) {
      baseWidth = hints->base_width;
      baseHeight = hints->base_height;
    } else {
      baseWidth = hints->min_width;
      baseHeight = hints->min_height;
    }

  } else if (hints->flags & PBaseSize) {

    minWidth = hints->base_width;
    minHeight = hints->base_height;
    baseWidth = hints->base_width;
    baseHeight = hints->base_height;

  } else {

    minWidth = 1;
    minHeight = 1;
    baseWidth = 1;
    baseHeight = 1;

  }
  
  if (hints->flags & PMaxSize) {
    maxWidth = hints->max_width;
    maxHeight = hints->max_height;
  } else {
    maxWidth = 10000;
    maxHeight = 10000;
  }

  if (hints->flags & PResizeInc) {
    xinc = hints->width_inc;
    yinc = hints->height_inc;
  } else {
    xinc = 1;
    yinc = 1;
  }
  
  // First, clamp to min and max values

  if (dwidth  < minWidth)  dwidth = minWidth;
  if (dheight < minHeight) dheight = minHeight;
  
  if (dwidth  > maxWidth)  dwidth = maxWidth;
  if (dheight > maxHeight) dheight = maxHeight;
  
  
  // Second, fit to base + N * inc

  dwidth  = ((dwidth - baseWidth) / xinc * xinc) + baseWidth;
  dheight = ((dheight - baseHeight) / yinc * yinc) + baseHeight;

  // Third, adjust for aspect ratio

#define maxAspectX hints->max_aspect.x
#define maxAspectY hints->max_aspect.y
#define minAspectX hints->min_aspect.x
#define minAspectY hints->min_aspect.y

  // The math looks like this:
  //
  // minAspectX    dwidth     maxAspectX
  // ---------- <= ------- <= ----------
  // minAspectY    dheight    maxAspectY
  //
  // If that is multiplied out, then the width and height are
  // invalid in the following situations:
  //
  // minAspectX * dheight > minAspectY * dwidth
  // maxAspectX * dheight < maxAspectY * dwidth
  
  if (hints->flags & PAspect) {
    if (minAspectX * dheight > minAspectY * dwidth) {
      delta = makemult(minAspectX * dheight / minAspectY - dwidth, xinc);
      if (dwidth + delta <= maxWidth) {
        dwidth += delta;
      } else {
        delta = makemult(dheight - dwidth*minAspectY/minAspectX, yinc);
        if (dheight - delta >= minHeight) dheight -= delta;
      }
    }
      
    if (maxAspectX * dheight < maxAspectY * dwidth) {
      delta = makemult(dwidth * maxAspectY / maxAspectX - dheight, yinc);
      if (dheight + delta <= maxHeight) {
        dheight += delta;
      } else {
        delta = makemult(dwidth - maxAspectX*dheight/maxAspectY, xinc);
        if (dwidth - delta >= minWidth) dwidth -= delta;
      }
    }
  }
  
  *widthp = dwidth;
  *heightp = dheight;

  return;
}


//----------------------------------------------------------------------

// Button stuff...

OXTButton *OXTaskBar::AddButton(char *title, const OPicture *p,
                          int state, int num) {
  OXTButton *b;

  b = new OXTButton(_buttonArray, num, _normFont, _hiFont, title, p);
  _buttonArray->AddFrame(b, NULL);
  b->Associate(this);

  _buttons->Add(num, (XPointer) b);

  _buttonArray->Layout();

  return b;
}

void OXTaskBar::RemoveButton(int num) {
  OXSNode *e;
  OXTButton *b;

  e = _buttons->GetNode(num);
  if (!e) return;

  b = (OXTButton *) e->data;
  _buttons->Remove(num);         // unlink from list of buttons

  _buttonArray->RemoveFrame(b);  // unlink from parent's list
  b->DestroyWindow();            // destroy the X window
  delete b;

  _buttonArray->Layout();
}

void OXTaskBar::ShowButton(int num) {
  OXTButton *b = FindButton(num);
  if (b) _buttonArray->ShowFrame(b);
}

void OXTaskBar::HideButton(int num) {
  OXTButton *b = FindButton(num);
  if (b) _buttonArray->HideFrame(b);
}

// RadioButton - Engage button 'num' and release all others

void OXTaskBar::RadioButton(int num, int state) {
  const OXSNode *e;
  OXTButton *b;

  for (e = _buttons->GetHead(); e != NULL; e = e->next) {
    b = (OXTButton *) e->data;
    if (b->WidgetID() == num) {
      b->SetState(state);
    } else {
      b->SetState(BUTTON_UP);
    }
  }
}

OXTButton *OXTaskBar::FindButton(long wid) {
  const OXSNode *e;
  OXTButton *b;

  for (e = _buttons->GetHead(); e != NULL; e = e->next) {
    b = (OXTButton *) e->data;
    if (b->WidgetID() == wid) return b;
  }
  return NULL;
}


//----------------------------------------------------------------------

// Some misc stuff...

// Show only the buttons corresponding to the windows in the current
// desktop, hide all the rest.

void OXTaskBar::SwitchDesktops() {
  OXTButton *b;
  const OXSNode *e;

  HideFrame(_buttonArray);

  for (e = _buttons->GetHead(); e != NULL; e = e->next) {
    b = (OXTButton *) e->data;
    if ((b->GetDesk() == _deskNumber) || (b->GetWindowFlags() & STICKY)) {
      _buttonArray->ShowFrame(b);
      b->SetState(BUTTON_UP);
    } else {
      _buttonArray->HideFrame(b);
    }
  }

  //_buttonArray()->Layout();
  ShowFrame(_buttonArray);
    
  //_buttonArray->NeedRedraw();
}

void OXTaskBar::AdjustWindow(int width, int height) {

  int newrows = (height+2) / _rowHeight;
  if (newrows < 1) newrows = 1;
  if (newrows != _rows) {
    _rows = newrows;
    _buttonArray->SetLayoutManager(new OTButtonLayout(_buttonArray,
                                                      _rowHeight, _rows));
  }

  Layout();
}

// Warp taskbar -- Enforce AutoStick feature

void OXTaskBar::Warp(int x, int y, int new_width, int new_height) {
  int nx, ny;

  nx = _decorBorder;

  if (!_autoHide) {
    if (y < _midLine)
      ny = _decorBorder;
    else
      ny = (int) _scrHeight - new_height - _decorBorder;

    Move(nx, ny);
  }
  if (_autoHide) {
    if (_hideTimer) delete _hideTimer;
    _hideTimer = new OTimer(this, 1000);
  }

  // Prevent oscillations caused by race with 
  // time delayed TaskBarHide().  Is there any way
  // to prevent these Xevents from being sent
  // to the server in the first place?

  //PurgeConfigEvents(); 
}

// Reveal taskbar -- Make taskbar fully visible

void OXTaskBar::Reveal() {
  int y, new_win_y;

  if (_hideTimer) {
    delete _hideTimer;
    _hideTimer = NULL;
  }

  if (_y < _midLine) {
    new_win_y = _decorBorder;
    for (y = _y; y <= new_win_y; y += 2)
      Move(_x, y);
  } else {
    new_win_y = (int) _scrHeight - _h - _decorBorder;
    for (y = _y; y >= new_win_y; y -= 2)
      Move(_x, y);
  }

  y = new_win_y;
  Move(_x, y);
  _belayHide = False; 
}

// Hide taskbar -- Make taskbar partially visible

void OXTaskBar::Hide() {
  int y, new_win_y;

  if (_hideTimer) {
    delete _hideTimer;
    _hideTimer = NULL;
  }

  if (_y < _midLine) {
    new_win_y = 1 - _h;
    for (y = _y; y >= new_win_y; --y)
      Move(_x, y);
  } else {
    new_win_y = (int) _scrHeight - 1;
    for (y = _y; y <= new_win_y; ++y)
      Move(_x, y);
  }

  y = new_win_y;
  // Move(_x, y);
}
