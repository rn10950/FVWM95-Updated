/**************************************************************************

    This file is part of xclass' desktop manager.
    Copyright (C) 1996-2000 David Barth, Hector Peraza.

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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <sys/stat.h>

#include <xclass/OXClient.h>
#include <xclass/OXWindow.h>
#include <xclass/OXMainFrame.h>
#include <xclass/OXMenu.h>
#include <xclass/OXMsgBox.h>
#include <xclass/OString.h>
#include <xclass/OPicture.h>
#include <xclass/OResourcePool.h>
#include <xclass/OXPropertiesDialog.h>
#include <xclass/utils.h>

#include <X11/keysym.h>

#include "OXDesktopIcon.h"
#include "OXDesktopContainer.h"
#include "OXDesktopRoot.h"

#include "main.h"


//-------------------------------------------------------------------

OXPopupMenu *objectMenu;
OXPopupMenu *sendToMenu;

Atom URI_list = None;

//#define TRAP_ERRORS


//----------------------------------------------------------------------

#ifdef TRAP_ERRORS
int ErrorHandler(Display *dpy, XErrorEvent *event) {
  char text[2048];

  XGetErrorText(dpy, event->error_code, text, 2048);
  fprintf(stderr, "Request %d, Error: %s\n",
                  event->request_code, text);

  return 0;
}
#endif

int main(int argc, char *argv[]) {

  OXClient *clientX = new OXClient(argc, argv);

#ifdef TRAP_ERRORS
  XSynchronize(clientX->GetDisplay(), True);
  XSetErrorHandler(ErrorHandler);
#endif

  OXDesktopMain *mainWindow = new OXDesktopMain(clientX->GetRoot(), 400, 200);
  mainWindow->MapWindow();

  clientX->Run();

  return 0;
}


//----------------------------------------------------------------------

OXDesktopMain::OXDesktopMain(const OXWindow *p, int w, int h) :
  OXMainFrame(p, w, h) {
    int retc;

    _t = NULL;

    _dndTypeList = new Atom[2];

    _dndTypeList[0] = XInternAtom(GetDisplay(), "text/uri-list", False);
    _dndTypeList[1] = None;

    URI_list = _dndTypeList[0];

    _dndManager = new ODNDmanager(_client, this, _dndTypeList);

    if (!_dndManager->SetRootProxy()) {
      new OXMsgBox(_client->GetRoot(), NULL,
                   new OString("Desktop Manager"),
                   new OString("Could not grab the desktop window for "
                               "drag-and-drop operations.\n"
                               "Another desktop manager is probably running.\n"
                               "Continue anyway?"),
                   MB_ICONSTOP, ID_YES | ID_NO, &retc);

      if (retc != ID_YES) exit(1);
    }

    OXDesktopRoot *dr = new OXDesktopRoot(_client, this, &retc);
    dr->Associate(this);

    _parent = dr;

    if (retc == BadAccess) {
      new OXMsgBox(_client->GetRoot(), NULL,
                   new OString("Desktop Manager"),
                   new OString("Could not grab the desktop window to capture mouse events.\n"
                               "Another desktop manager is probably running,\n"
                               "or you have conflicting window manager settings.\n"
                               "Continue anyway?"),
                   MB_ICONSTOP, ID_YES | ID_NO, &retc);

      if (retc != ID_YES) exit(1);
    }

    const char *user_root = _client->GetResourcePool()->GetUserRoot();

    char *desktop_dir = new char[strlen(user_root)+9];
    sprintf(desktop_dir, "%s/desktop", user_root);

    if (access(desktop_dir, F_OK)) {
      // probably this is the first time this program is run under this
      // user account: create the necessary dirs here, put some
      // useful stuff into them, warn the user, etc...
      int errc = _CreateDesktop(desktop_dir);

      if (errc) {
        char msg[256];

        sprintf(msg, "Could not create desktop directory\n"
                     "\"%s\":\n%s\nExiting.", desktop_dir, strerror(errc));
        new OXMsgBox(_client->GetRoot(), NULL,
                     new OString("Desktop Manager"),
                     new OString(msg), MB_ICONSTOP, ID_CLOSE);
        FatalError(msg);
      }
    }

    _MakeMenus();

    _container = new OXDesktopContainer(this, this, _dndManager);
    _container->Associate(this);

    _container->ChangeDirectory(desktop_dir);

    _container->Init();
    // We call Save() immediately after Init() in order to re-create the
    // contents of the desktoprc file in case the user or any other
    // program was messing around with the contents of the desktop
    // directory. Unused entries will get deleted, new entries will be
    // added for any new files...
    _container->Save();

    _container->ArrangeIcons();

    MapSubwindows();

    // This window must remain hidden (and mapped!), its only purpose
    // is to grab the WM focus when an icon is clicked (icons are
    // override-redirect windows and therefore cannot be used for that
    // purpose).

    MoveResize(-100, -100, 90, 40);
    SetWMPosition(-100, -100);

    SetWindowName("desktop");
    SetClassHints("XCLASS", "Desktop");

    SetMWMHints(MWM_DECOR_ALL, MWM_FUNC_ALL, MWM_INPUT_MODELESS);

    Resize(GetDefaultSize());

    AddInput(FocusChangeMask | KeyPressMask | KeyReleaseMask);
}

OXDesktopMain::~OXDesktopMain() {
  delete _container;  // must delete explicitely, since it was not Added()
 
  delete _newMenu;
  delete _sortMenu;

  delete objectMenu;
  delete sendToMenu;

  delete[] _dndTypeList;

  _dndManager->RemoveRootProxy();

  if (_t) delete _t;
}

void OXDesktopMain::_MakeMenus() {

  sendToMenu = new OXPopupMenu(_client->GetRoot());
  sendToMenu->AddEntry(new OHotString("Fax Recipient"),  21, _client->GetPicture("fax.t.xpm"));
  sendToMenu->AddEntry(new OHotString("Mail Recipient"), 22, _client->GetPicture("mail.t.xpm"));
  sendToMenu->AddEntry(new OHotString("My Briefcase"),   23, _client->GetPicture("briefcase.t.xpm"));

  objectMenu = new OXPopupMenu(_client->GetRoot());
  objectMenu->AddEntry(new OHotString("&Open"),    M_OBJECT_OPEN);
  //objectMenu->AddEntry(new OHotString("&Explore"), M_OBJECT_EXPLORE);
  //objectMenu->AddEntry(new OHotString("&Find..."), M_OBJECT_FIND);
  objectMenu->AddSeparator();
  objectMenu->AddPopup(new OHotString("Se&nd To"), sendToMenu);
  objectMenu->AddSeparator();
  objectMenu->AddEntry(new OHotString("Cu&t"),   M_EDIT_CUT);
  objectMenu->AddEntry(new OHotString("&Copy"),  M_EDIT_COPY);
  objectMenu->AddEntry(new OHotString("&Paste"), M_EDIT_PASTE);
  objectMenu->AddSeparator();
  //objectMenu->AddEntry(new OHotString("Create &Shortcut"), M_FILE_NEWSHORTCUT);
  objectMenu->AddEntry(new OHotString("&Delete"), M_FILE_DELETE);
  objectMenu->AddEntry(new OHotString("Rena&me"), M_FILE_RENAME);
  objectMenu->AddEntry(new OHotString("Change &Icon..."), 888);
  objectMenu->AddSeparator();
  objectMenu->AddEntry(new OHotString("P&roperties..."), M_FILE_PROPS);
  objectMenu->SetDefaultEntry(1501);

  sendToMenu->Associate(this);
  objectMenu->Associate(this);

  _newMenu = new OXPopupMenu(_client->GetRoot());
  _newMenu->AddEntry(new OHotString("&Folder"),       M_FILE_NEWFOLDER);
  _newMenu->AddEntry(new OHotString("&Shortcut"),     M_FILE_NEWSHORTCUT);
  _newMenu->AddSeparator();
  _newMenu->AddEntry(new OHotString("Bitmap Image"),  M_FILE_NEWIMAGE);
  _newMenu->AddEntry(new OHotString("Text Document"), M_FILE_NEWTEXTDOC);
  _newMenu->AddEntry(new OHotString("Briefcase"),     M_FILE_NEWBRIEFCASE);

  _newMenu->DisableEntry(M_FILE_NEWSHORTCUT);

  _sortMenu = new OXPopupMenu(_client->GetRoot());
  _sortMenu->AddEntry(new OHotString("By &Name"),      M_VIEW_ARRANGE_BYNAME);
  _sortMenu->AddEntry(new OHotString("By &Type"),      M_VIEW_ARRANGE_BYTYPE);
  _sortMenu->AddEntry(new OHotString("By &Size"),      M_VIEW_ARRANGE_BYSIZE);
  _sortMenu->AddEntry(new OHotString("By &Date"),      M_VIEW_ARRANGE_BYDATE);
  _sortMenu->AddSeparator();
  _sortMenu->AddEntry(new OHotString("&Auto Arrange"), M_VIEW_ARRANGE_AUTO);
  _sortMenu->CheckEntry(M_VIEW_ARRANGE_AUTO);
  _sortMenu->RCheckEntry(M_VIEW_ARRANGE_BYNAME, 
                         M_VIEW_ARRANGE_BYNAME, M_VIEW_ARRANGE_BYDATE);

  _sortMenu->Associate(this);

  _rootMenu = new OXPopupMenu(_client->GetRoot());
  _rootMenu->AddPopup(new OHotString("&New"), _newMenu);
  _rootMenu->AddSeparator();
  _rootMenu->AddEntry(new OHotString("Cu&t"),   M_EDIT_CUT);
  _rootMenu->AddEntry(new OHotString("&Copy"),  M_EDIT_COPY);
  _rootMenu->AddEntry(new OHotString("&Paste"), M_EDIT_PASTE);
  _rootMenu->AddSeparator();
  _rootMenu->AddPopup(new OHotString("Arrange &Icons"), _sortMenu);
  _rootMenu->AddEntry(new OHotString("Lin&e up Icons"), M_VIEW_LINEUP);
  _rootMenu->AddSeparator();
  _rootMenu->AddEntry(new OHotString("P&roperties..."), M_ROOT_PROPS);

  _rootMenu->DisableEntry(M_EDIT_CUT);
  _rootMenu->DisableEntry(M_EDIT_COPY);
  _rootMenu->DisableEntry(M_EDIT_PASTE);

  _rootMenu->Associate(this);
}

int OXDesktopMain::_CreateDesktop(const char *path) {

  // make the path...

  if (MakePath(path, S_IRWXU)) return errno;

  // ...and create some useful objects

  if (chdir(path)) return errno;

  const char *home = getenv("HOME");
  if (home) {

    // create a link to the user's home directory
    symlink(home, "Home");

    // create a link to the recycle bin
    char recycle[PATH_MAX];
    const char *user_root = _client->GetResourcePool()->GetUserRoot();
    sprintf(recycle, "%s/recycle", user_root);
    if (access(recycle, F_OK)) MakePath(recycle, S_IRWXU);
    if (access(recycle, F_OK)) {
      // could not create the recycle bin directory, notify the user?
    } else {
      symlink(recycle, "Recycle Bin");
    }

    // shall we examine $PATH and add a few links to commonly used
    // applications? (X/xclass/kde/gnome):
    // xterm, xcgview, notepad, xcpaint, netscape, lock screen,
    // acroread, xv, calculator, kcalc...

  }

  return 0;
}

int OXDesktopMain::HandleConfigureNotify(XConfigureEvent *event) {
  OXMainFrame::HandleConfigureNotify(event);
  if ((_x + _w > 0) && (_y + _h > 0)) {
    // the hidden window became visible for some reason,
    // hide it again after a second.
    if (_t) delete _t;
    _t = new OTimer(this, 1000);
  }
  return True;
}

int OXDesktopMain::HandleFocusChange(XFocusChangeEvent *event) {
  if ((event->mode == NotifyNormal) &&
      (event->detail != NotifyPointer)) {
    if (event->type == FocusIn) {
      // perhaps we should remember the selection and highlight
      // the icons again here
    } else {
      _container->UnselectAll();
    }
  }
  return True;
}

int OXDesktopMain::HandleKey(XKeyEvent *event) {
  if (event->type == KeyPress) {

    KeySym keysym = XKeycodeToKeysym(GetDisplay(), event->keycode, 0);

    switch (keysym) {
      case XK_Left:
        break;

      case XK_Right:
        break;

      case XK_Up:
        break;

      case XK_Down:
        break;

      case XK_KP_Enter:
      case XK_Return: {
        void *iterator = NULL;

        while (1) {
          OXDesktopIcon *f = (OXDesktopIcon *) _container->GetNextSelected(&iterator);
          if (!f) break;
          _container->DoAction(f);
        }
        break;
      }

      case XK_space:
        break;

      case XK_Delete:
        _container->DeleteSelectedFiles();
        break;

      default: {
        char input[2] = { 0, 0 };
        int  charcount;
        KeySym keysym;
        XComposeStatus compose = { NULL, 0 };
        charcount = XLookupString(event, input, sizeof(input)-1,
                                  &keysym, &compose);
        if (charcount > 0) {
        }
        break;
      }

    }

  }

  return True;
}

int OXDesktopMain::HandleTimer(OTimer *t) {
  if (t == _t) {
    MoveResize(-100, -100, 90, 40);
    delete _t;
    _t = NULL;
    return True;
  }
  return False;
}

int OXDesktopMain::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;

  switch (msg->type) {

    case MSG_MENU:
      switch (msg->action) {

        case MSG_CLICK:
          switch (wmsg->id) {

            case M_VIEW_ARRANGE_BYNAME:
              _sortMenu->RCheckEntry(M_VIEW_ARRANGE_BYNAME,
                                     M_VIEW_ARRANGE_BYNAME, M_VIEW_ARRANGE_BYDATE);
              _container->Sort(SORT_BY_NAME);
              break;

            case M_VIEW_ARRANGE_BYTYPE:
              _sortMenu->RCheckEntry(M_VIEW_ARRANGE_BYTYPE,
                                     M_VIEW_ARRANGE_BYNAME, M_VIEW_ARRANGE_BYDATE);
              _container->Sort(SORT_BY_TYPE);
              break;

            case M_VIEW_ARRANGE_BYSIZE:
              _sortMenu->RCheckEntry(M_VIEW_ARRANGE_BYSIZE,
                                     M_VIEW_ARRANGE_BYNAME, M_VIEW_ARRANGE_BYDATE);
              _container->Sort(SORT_BY_SIZE);
              break;

            case M_VIEW_ARRANGE_BYDATE:
              _sortMenu->RCheckEntry(M_VIEW_ARRANGE_BYDATE,
                                     M_VIEW_ARRANGE_BYNAME, M_VIEW_ARRANGE_BYDATE);
              _container->Sort(SORT_BY_DATE);
              break;

            case M_VIEW_REFRESH:
              _container->DisplayDirectory();
              break;

            case M_FILE_DELETE:
              _container->DeleteSelectedFiles();
              break;

            case M_FILE_RENAME:
              if (_container->NumSelected() == 1) {
                void *iterator = NULL;

                OXDesktopIcon *f = (OXDesktopIcon *) _container->GetNextSelected(&iterator);
                if (f) _container->Rename(f);
              }
              break;

            case M_FILE_PROPS:
              if (_container->NumSelected() == 1) {
                const OXDesktopIcon *f;
                void *iterator = NULL;

                f = _container->GetNextSelected(&iterator);
                new OXPropertiesDialog(_client->GetRoot(), NULL,
                                       new OString(f->GetName()));
              } else {
                // group file properties: chmod only?
                XBell(GetDisplay(), 0);
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

    case MSG_DESKTOPROOT:
      if (msg->action == MSG_CLICK) {
        ODesktopRootMessage *dmsg = (ODesktopRootMessage *) msg;

        if (dmsg->button == Button3) {
          int choice = _rootMenu->PopupMenu(dmsg->x_root, dmsg->y_root);
          switch (choice) {
            case M_FILE_NEWFOLDER:
            case M_FILE_NEWIMAGE:
            case M_FILE_NEWTEXTDOC:
            case M_FILE_NEWBRIEFCASE:
              _container->CreateObject(dmsg->x_root, dmsg->y_root, choice);
              break;

            default:
              break;

          }
        }
      }
      break;

    default:
      break;

  }

  return True;
}

OXFrame *OXDesktopMain::GetFrameFromPoint(int x, int y) {
  int x_root, y_root;
  Window child;

  XTranslateCoordinates(GetDisplay(), _id, _client->GetRoot()->GetId(),
                        x, y, &x_root, &y_root, &child);

  return _container->GetFrameFromPoint(x_root, y_root);
}
