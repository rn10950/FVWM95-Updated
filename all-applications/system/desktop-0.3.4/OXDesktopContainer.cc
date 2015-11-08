/**************************************************************************

    This file is part of a xclass desktop manager.
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

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <limits.h>

#include <vector>

#include <X11/Xatom.h>

#include <xclass/utils.h>
#include <xclass/OXCompositeFrame.h>
#include <xclass/OXClient.h>
#include <xclass/OString.h>
#include <xclass/OIniFile.h>
#include <xclass/OResourcePool.h>
#include <xclass/OXCanvas.h>
#include <xclass/OExec.h>
#include <xclass/OGC.h>
#include <xclass/OXFont.h>
#include <xclass/OXMsgBox.h>
#include <xclass/ODNDmanager.h>

#include "URL.h"
#include "OXDesktopIcon.h"
#include "OXDesktopContainer.h"
#include "ODesktopLayout.h"
#include "ORecycledFiles.h"
#include "main.h"


OXGC *OXDesktopContainer::_lineGC = NULL;

extern Atom URI_list;



// TODO:
// + watch desktop directory for changes
// - implement a correct layout manager which tracks desktop size changes
//   (taskbar and other sticking toolbars)
// - implement special-case icons (Recycle Bin, My Computer, etc).
// + make the recycle-bin look empty or full depending on the corresponding
//   dir contents (need to save two icons in the .ini file?)
// - implement auto-arrange, snap-to-grid, and line-up icons
// - capture stderr to show error messages while trying to execute programs
// + add support for "shortcut" pixmaps
// - lots of some other things
// + exec: use absolute path, do not rely on "." in $PATH
// - executable icons: add "start in" (directory) property
// + capture mouse events on the root window
// - process keyboard events
// - correct small icon moves (currently xdnd interferes)
// - edit icon labels
// - wrap/truncate long icon labels


//#define DEBUG


// NewIcon() actions:

#define ACTION_MOVE     0
#define ACTION_COPY     1
#define ACTION_SYMLINK  2
#define ACTION_CREATE   3


//---------------------------------------------------------------------------

OXDesktopContainer::OXDesktopContainer(const OXWindow *p, 
                                       OXDesktopMain *main, 
                                       ODNDmanager *dnd) :
  OXCompositeFrame(p, 1, 1, CHILD_FRAME) {

    if (!_lineGC) {
      unsigned long gmask;
      XGCValues gval;

      unsigned int _white = GetResourcePool()->GetWhiteColor();
      unsigned int _black = GetResourcePool()->GetBlackColor();

      gmask = GCForeground | GCBackground | GCFunction | GCFillStyle |  
              GCLineWidth  | GCLineStyle  | GCSubwindowMode |
              GCGraphicsExposures;
      gval.foreground = _white ^ _black;
      gval.background = _white;
      gval.function   = GXxor;
      gval.line_width = 0;
      gval.line_style = LineOnOffDash;
      gval.fill_style = FillSolid;
      gval.subwindow_mode = IncludeInferiors;
      gval.graphics_exposures = False;
      _lineGC = new OXGC(GetDisplay(), _id, gmask, &gval);
      XSetDashes(GetDisplay(), _lineGC->GetGC(), 0, "\x1\x1", 2); 
    }

    _desktopMain = main;
    _dndManager = dnd;

    MimeTypesList = _client->GetResourcePool()->GetMimeTypes();

    _folder  = _client->GetPicture("folder.s.xpm");
    _app     = _client->GetPicture("app.s.xpm");
    _doc     = _client->GetPicture("doc.s.xpm");
    _slink   = _client->GetPicture("slink.s.xpm");
    _rbempty = _client->GetPicture("recycle-empty.s.xpm");
    _rbfull  = _client->GetPicture("recycle-full.s.xpm");

    if (!_folder || !_app || !_doc || !_slink || !_rbempty || !_rbfull)
      FatalError("OXDesktopContainer: Missing required pixmap(s)\n");

    const char *uroot = _client->GetResourcePool()->GetUserRoot();
    if (!uroot) uroot = "";

    char recyclerc[PATH_MAX];
    char recyclebin[PATH_MAX];

    sprintf(recyclerc, "%s/etc/recycle.files", uroot);
    sprintf(recyclebin, "%s/recycle", uroot);

    _recycled = new ORecycledFiles(recyclebin, recyclerc);

    _msgObject = NULL;

    _last_active = NULL;
    _dragging = False;
    _total = _selected = 0;
    _popup = objectMenu; // _MakeMenu();
    _sortType = SORT_BY_NAME;

    _dndpopup = new OXPopupMenu(_client->GetRoot());
    _dndpopup->AddEntry(new OHotString("&Move here"), ACTION_MOVE);
    _dndpopup->AddEntry(new OHotString("&Copy here"), ACTION_COPY);
    _dndpopup->AddEntry(new OHotString("Create &soft link here"), ACTION_SYMLINK);
    _dndpopup->AddSeparator();
    _dndpopup->AddEntry(new OHotString("Cancel"), -1);
    _dndpopup->SetDefaultEntry(ACTION_MOVE);

#ifdef DEBUG
    _inifile = "desktoprc-1";
#else
    _inifile = "desktoprc";
#endif

    _refresh = new OTimer(this, 5000);

    SetLayoutManager(new ODesktopLayout(this, _client, 8));

    _fl = new OLayoutHints(LHINTS_EXPAND_Y | LHINTS_CENTER_X);

    XGrabButton(GetDisplay(), AnyButton, AnyModifier, _id, True,
                ButtonPressMask | ButtonReleaseMask |
                PointerMotionMask,
                GrabModeAsync, GrabModeAsync, None, None);

    _dndx = _dndy = 0;
}

OXDesktopContainer::~OXDesktopContainer() {
  if (_refresh) delete _refresh;
  _client->FreePicture(_folder);
  _client->FreePicture(_app);
  _client->FreePicture(_doc);
  _client->FreePicture(_slink);
  _client->FreePicture(_rbempty);
  _client->FreePicture(_rbfull);
  delete _fl;
  delete _recycled;
}

//----------------------------------------------------------------------

int OXDesktopContainer::HandleTimer(OTimer *t) {
  struct stat sbuf;

  if (t != _refresh) return False;

  if (stat(".", &sbuf) == 0) {
    if (_st_mtime != sbuf.st_mtime) {
      DisplayDirectory();
    } else if (stat(_recycled->GetRecycleBin(), &sbuf) == 0) {
      if (_rb_mtime != sbuf.st_mtime) DisplayDirectory();
    }
  }

  delete _refresh;
  _refresh = new OTimer(this, 5000);

  return True;
}

int OXDesktopContainer::HandleButton(XButtonEvent *event) {
  SListFrameElt *ptr;
  OXDesktopIcon *f;
  int total, selected, inv;

//printf("OXDesktopContainer: click!\n");

  if (event->type == ButtonPress) {

    XSetInputFocus(GetDisplay(), _desktopMain->GetId(),
                   RevertToParent, CurrentTime);

    inv = event->state & ControlMask;
    _xp = event->x;
    _yp = event->y;

    if (_last_active) {
      _last_active->Activate(False);
      _last_active = NULL;
    }
    total = selected = 0;
    for (ptr = _flist; ptr != NULL; ptr = ptr->next) {   
      f = (OXDesktopIcon *) ptr->frame;
      ++total;
      if (f->GetId() == event->subwindow) {
        f->Activate(True);
        ++selected;
        _last_active = f;
      } else {
        f->Activate(False);
      }
    }

    if (_total != total || _selected != selected) {
      _total = total;
      _selected = selected;
      //OContainerMessage message(MSG_CONTAINER, MSG_SELCHANGED, -1, 0,
      //                          _total, _selected);
      //SendMessage(_msgObject, &message);
    }

    if (selected == 0) {
      _dragging = True;
      _x0 = _xf = _xp;
      _y0 = _yf = _yp;
      DrawRectangle(_lineGC->GetGC(), _x0, _y0, _xf-_x0, _yf-_y0);
    }
  }  

  if (event->type == ButtonRelease) {
    if (_dragging) {
      _dragging = False;
      DrawRectangle(_lineGC->GetGC(), _x0, _y0, _xf-_x0, _yf-_y0);
    } else if (event->button == Button3) {
      _popup->PlaceMenu(event->x_root, event->y_root, False, True);
    }

else
    if ((_selected == 1) && _last_active) {
      if (_last_active->IsInsideLabel(OPosition(event->x, event->y))) {
        Rename(_last_active);
      }
    }

  }

  return True;
}  

int OXDesktopContainer::HandleMotion(XMotionEvent *event) {
  SListFrameElt *ptr;
  OXDesktopIcon *f;
  int xf0, yf0, xff, yff, total, selected, inv;

  if (_dragging) {
    inv = event->state & ControlMask;
    DrawRectangle(_lineGC->GetGC(), _x0, _y0, _xf-_x0, _yf-_y0);
    _x0 = min(_xp, event->x);
    _xf = max(_xp, event->x);
    _y0 = min(_yp, event->y);
    _yf = max(_yp, event->y);

    total = selected = 0;
    for (ptr = _flist; ptr != NULL; ptr = ptr->next) {
      f = (OXDesktopIcon *) ptr->frame;
      ++total;
      xf0 = f->GetX() + (f->GetWidth() >> 3);
      yf0 = f->GetY() + (f->GetHeight() >> 3);
      xff = xf0 + f->GetWidth() - (f->GetWidth() >> 2);
      yff = yf0 + f->GetHeight() - (f->GetHeight() >> 2);

      if (((xf0 > _x0 && xf0 < _xf) ||
           (xff > _x0 && xff < _xf)) &&
          ((yf0 > _y0 && yf0 < _yf) ||
           (yff > _y0 && yff < _yf))) {
        f->Activate(True);
        ++selected;
      } else {
        f->Activate(False);
      }
    }

    DrawRectangle(_lineGC->GetGC(), _x0, _y0, _xf-_x0, _yf-_y0);

    if (_total != total || _selected != selected) {
      _total = total;
      _selected = selected;
      //OContainerMessage message(MSG_CONTAINER, MSG_SELCHANGED, -1, 0,
      //                          _total, _selected);
      //SendMessage(_msgObject, &message);
    }

  }

  return True;
}  

void OXDesktopContainer::SelectInRectangle(ORectangle rect, int add) {
  SListFrameElt *ptr;
  OXDesktopIcon *f;
  int xf0, yf0, xff, yff, total, selected, inv;

  inv = add; //event->state & ControlMask;

  total = selected = 0;
  for (ptr = _flist; ptr != NULL; ptr = ptr->next) {
    f = (OXDesktopIcon *) ptr->frame;
    ++total;
    xf0 = f->GetX();
    yf0 = f->GetY();
    xff = xf0 + f->GetWidth();
    yff = yf0 + f->GetHeight();

    if ((xf0 < rect.x + rect.w) && (xff > rect.x) &&
        (yf0 < rect.y + rect.h) && (yff > rect.y)) {
      f->Activate(True);
      ++selected;
    } else {
      f->Activate(False);
    }
  }

  if (_selected > 0)
    XSetInputFocus(GetDisplay(), _desktopMain->GetId(),
                   RevertToParent, CurrentTime);


  if (_total != total || _selected != selected) {
    _total = total;
    _selected = selected;
    //OContainerMessage message(MSG_CONTAINER, MSG_SELCHANGED, -1, 0,
    //                          _total, _selected);
    //SendMessage(_msgObject, &message);
  }
}  

int OXDesktopContainer::HandleDoubleClick(XButtonEvent *event) {
  SListFrameElt *ptr;
  OXDesktopIcon *f;

  for (ptr = _flist; ptr != NULL; ptr = ptr->next) {   

    f = (OXDesktopIcon *) ptr->frame;

    if (f->GetId() == event->subwindow) {
      DoAction(f);
      break;
    }
  }

  return True;  // required if the window processes double-click!
}


//----------------------------------------------------------------------

//--- Delete selected files

void OXDesktopContainer::DeleteSelectedFiles() {
  const OXDesktopIcon *f;  
  struct stat  inode;
  const  char *basename;
  char         dirname[PATH_MAX];
  char         newfilename[PATH_MAX];
  char         filename[PATH_MAX];
  char         recycle[PATH_MAX];
  char         recfilename[PATH_MAX];
  int          index = 0;
  time_t t;
  int    i, sel, retval;
  const  OPicture *pic;
  char  *title, prompt[256];
  void  *iterator = NULL;
  std::vector<const OXDesktopIcon *> selected;

  if ((sel = NumSelected()) == 0) return;

  // we must collect the selected items before calling OXMsgBox, since
  // the last causes the selection to be lost when it gets the focus.

  selected.clear();
  while (1) {
    f = GetNextSelected(&iterator);
    if (!f) break;
    selected.push_back(f);
  }

  if (sel == 1) {
    pic = _client->GetPicture("srecycle.xpm");
    sprintf(prompt, "Are you sure you want to send \"%s\" to the Recycle Bin?",
            selected[0]->GetName()->GetString());
    title = "Confirm File Delete";
  } else {
    pic = _client->GetPicture("mrecycle.xpm");
    sprintf(prompt, "Are you sure you want to send these %d items to the Recycle Bin?",
            sel);
    title = "Confirm Multiple File Delete";
  }

  new OXMsgBox(_client->GetRoot(), NULL,
               new OString(title), new OString(prompt), pic,
               ID_YES | ID_NO, &retval);

  if (retval != ID_YES) return;

  getcwd(dirname, PATH_MAX);
  strcpy(recycle, _recycled->GetRecycleBin());

  for (i = 0; i < selected.size(); ++i) {
    f = selected[i];
    basename = f->GetName()->GetString();
    if ((strcmp(basename, ".") != 0) && (strcmp(basename, "..") != 0)) {
      sprintf(filename, "%s/%s", dirname, basename);
      sprintf(recfilename, "%s/%s", recycle, basename);
      sprintf(newfilename, "%s", basename);
      index = 0;
      while (stat(recfilename, &inode) == 0) {
        sprintf(newfilename, "%s.%d", basename, index++);
        sprintf(recfilename, "%s/%s", recycle, newfilename);
      }
      time(&t);
      _recycled->AddFile(basename, newfilename,
                         ctime((const time_t *) &t), filename);

      // warning! possible cross-device rename attempt!

      if (rename(filename, recfilename) != 0) {
        sprintf(prompt, "Cannot delete file \"%s\": %s.",
                basename, strerror(errno));
        new OXMsgBox(_client->GetRoot(), NULL,
                     new OString("Desktop Manager"), new OString(prompt),
                     MB_ICONSTOP, ID_OK);
        
      }
    }
  }

  selected.clear(); // not really neccessary...

  // this will remove deleted files from the container
  DisplayDirectory();
}

//--- Execute action for the specified icon

void OXDesktopContainer::DoAction(OXDesktopIcon *f) {
  int  ftype;
  char action[PATH_MAX];
  char fullaction[PATH_MAX];
  char command[PATH_MAX];
  char filename[PATH_MAX];   
  char path[PATH_MAX];
  char *argv[PATH_MAX];
  int  argc = 0;
  char *argptr;

  if (!f) return;

  // get file type
  ftype = f->GetType();

  // is it a directory?
  if (S_ISDIR(ftype)) {

    argv[0] = "explorer";
    argv[1] = (char *) f->GetName()->GetString();
    argv[2] = NULL;
    OExec exec(_client, argv[0], argv, NULL, False, True);

  // is it an executable file?
  } else if (S_ISREG(ftype) && (ftype & S_IXUSR)) {

    sprintf(path, "./%s", f->GetName()->GetString());
    //sprintf(path, "%s/%s", getcwd(...), f->GetName()->GetString());
    argv[0] = (char *) f->GetName()->GetString();
    argv[1] = NULL;
    OExec exec(_client, path, argv, NULL, False, True);

  // MIME
  } else if (MimeTypesList->GetAction(f->GetName()->GetString(), action)) {

    getcwd(path, PATH_MAX);
    sprintf(filename, "%s/%s", path, f->GetName()->GetString());
    argptr = strtok(action, " ");
    while (argptr) {
      if (strcmp(argptr, "%s") == 0) {
        argv[argc] = StrDup(filename);
        argc++;
      } else {
        argv[argc] = StrDup(argptr);
        argc++;
      }
      argptr = strtok(NULL, " ");
    }
    argv[argc] = NULL;
    OExec exec(_client, argv[0], argv, NULL, False, True);

    while (argc) delete[] argv[--argc];

  }

}

void OXDesktopContainer::Rename(OXDesktopIcon *f) {
  OString *new_name = f->EditLabel();
  if (new_name) {
    if (strcmp(f->GetName()->GetString(), new_name->GetString()) == 0) {
      // same name, not changed.
      delete new_name;
      return;
    }
    if (access(new_name->GetString(), F_OK) == 0) {
      char tmp[PATH_MAX];
      sprintf(tmp, "Cannot rename file \"%s\" to \"%s\": File already exists.",
              f->GetName()->GetString(), new_name->GetString());
      new OXMsgBox(_client->GetRoot(), NULL,
                   new OString("Desktop Manager"), new OString(tmp),
                   MB_ICONSTOP, ID_OK);
      delete new_name;
      return;
    }
    int retc = rename(f->GetName()->GetString(), new_name->GetString());
    if (retc != 0) {
      char tmp[PATH_MAX];
      sprintf(tmp, "Cannot rename file \"%s\" to \"%s\": %s.",
              f->GetName()->GetString(), new_name->GetString(), strerror(errno));
      new OXMsgBox(_client->GetRoot(), NULL,
                   new OString("Desktop Manager"), new OString(tmp),
                   MB_ICONSTOP, ID_OK);
      delete new_name;
      return;
    } else {
      f->SetName(new_name);
      Save();
    }
  }
}


//----------------------------------------------------------------------

OXDesktopIcon *OXDesktopContainer::AddIcon(const OPicture *pic,
                                           const OPicture *lpic,
                                           OString *name, int type,
                                           unsigned long size) {
  OXDesktopIcon *f;

  f = new OXDesktopIcon(_client->GetRoot(), pic, lpic, name, type, size);
  OXCompositeFrame::AddFrame(f, _fl);

  f->SetFileWindow(this);

  return f;
}

void OXDesktopContainer::Init() {
  SListFrameElt *ptr;
  char line[1024], arg[256], *inipath;
  int x, y;

  inipath = GetResourcePool()->FindIniFile(_inifile, INI_READ);
  if (!inipath) inipath = _inifile;

  OIniFile ini(inipath, INI_READ);
  while (ini.GetNext(line)) {
    for (ptr = _flist; ptr != NULL; ptr = ptr->next) {
      OXDesktopIcon *f = (OXDesktopIcon *) ptr->frame;
      if (strcmp(line, f->GetName()->GetString()) == 0) {
        ini.GetItem("icon", arg);
        ini.GetItem("position", arg);
        if (sscanf(arg, "%d,%d", &x, &y) == 2) f->Move(x, y);
        break;
      }
    }
  }
  if (inipath != _inifile) delete[] inipath;
}

void OXDesktopContainer::Save() {
  SListFrameElt *ptr;
  char tmp[256], *inipath;

  inipath = GetResourcePool()->FindIniFile(_inifile, INI_WRITE);
  if (!inipath) inipath = _inifile;

  OIniFile ini(inipath, INI_WRITE);
  for (ptr = _flist; ptr != NULL; ptr = ptr->next) {
    OXDesktopIcon *f = (OXDesktopIcon *) ptr->frame;
    ini.PutNext((char *) f->GetName()->GetString());
    ini.PutItem("icon", (char *) f->GetPicture()->GetName());
    sprintf(tmp, "%d,%d", f->GetX(), f->GetY());
    ini.PutItem("position", tmp);
    ini.PutNewLine();
  }
  if (inipath != _inifile) delete[] inipath;
}

void OXDesktopContainer::Sort(int sortType) {
  OXCanvas *canvas;
  SListFrameElt *ptr;
  int n;

  _sortType = sortType;

  for (n = 0, ptr = _flist; ptr != NULL; n++, ptr = ptr->next);

  _doSort(&_flist, n);

  // ==!== we should change the sort routine to update the prev ptr instead...

  if (_flist == NULL) {
    _ftail = NULL;
  } else {
    for (ptr = _flist; ptr->next != NULL; ptr = ptr->next) {
      ptr->next->prev = ptr;
    }
    _ftail = ptr;
  }

  // ==!==

  //canvas = (OXCanvas *) this->GetParent()->GetParent();
  //canvas->Layout();
}

//---- Original list sort code by David Kastrup (c) 1992, Germany.

SListFrameElt **OXDesktopContainer::_doSort(SListFrameElt **head, int n) {
  SListFrameElt *p1, *p2, **h2, **t2;
  int m;

  switch (n) {
  case 0:
    return head;

  case 1:
    return &((*head)->next);

  case 2:
    p2 = (p1 = *head)->next;
    if (_compare(p1, p2)) return &(p2->next);
    p1->next = (*head = p2)->next;
    return &((p2->next = p1)->next);
  }

  n -= m = n / 2;
  t2 = _doSort(h2 = _doSort(head, n), m);

  if (_compare(p1 = *head, p2 = *h2)) {
    do {
      if (!--n) return *h2 = p2, t2;
    } while (_compare(p1 = *(head = &(p1->next)), p2));
  }

  while(1) {
    *head = p2;
    do {
      if (!--m) return *h2 = *t2, *t2 = p1, h2;
    } while (!_compare(p1, p2 = *(head = &(p2->next))));
      *head = p1;
      do {
      if (!--n) return *h2 = p2, t2;
    } while (_compare(p1 = *(head = &(p1->next)), p2));
  }
}

int OXDesktopContainer::_compare(SListFrameElt *p1, SListFrameElt *p2) {
  int type1, type2;

  OXDesktopIcon *f1 = (OXDesktopIcon *) p1->frame;
  OXDesktopIcon *f2 = (OXDesktopIcon *) p2->frame;

  switch (_sortType) {
  default:
  case SORT_BY_NAME:
    break;

  case SORT_BY_TYPE:
    type1 = f1->GetType();
    type2 = f2->GetType();

    //--- use posix macros

    if (S_ISDIR(type1)) type1 = 1;
    #if defined(S_IFLNK)
    else if ((type1 & S_IFMT) == S_IFLNK) type1 = 2;
    #endif
    #if defined(S_IFSOCK)
    else if ((type1 & S_IFMT) == S_IFSOCK) type1 = 3;
    #endif
    else if (S_ISFIFO(type1)) type1 = 4;
    else if (S_ISREG(type1) &&
            (type1 & S_IXUSR)) type1 = 5;
    else type1 = 6;
   
    if (S_ISDIR(type2)) type2 = 1;
    #if defined(S_IFLNK)
    else if ((type2 & S_IFMT) == S_IFLNK) type2 = 2;
    #endif
    #if defined(S_IFSOCK)
    else if ((type2 & S_IFMT) == S_IFSOCK) type2 = 3;
    #endif
    else if (S_ISFIFO(type2)) type2 = 4;
    else if (S_ISREG(type2) &&
            (type2 & S_IXUSR)) type2 = 5;
    else type2 = 6;

    if (type1 != type2) return (type1 <= type2);
    break;

  case SORT_BY_SIZE:
    if (f1->GetSize() != f2->GetSize()) return (f1->GetSize() <= f2->GetSize());
    break;

  }
  return (strcmp(f1->GetName()->GetString(), f2->GetName()->GetString()) <= 0);
}


//---- Determine the file picture for the given file type.

void OXDesktopContainer::GetIconPics(const OPicture **pic,
                                     const OPicture **lpic,
                                     char *fname, int ftype, int is_link) {
  char temp[BUFSIZ];

  *pic = MimeTypesList->GetIcon(fname, False);
  if (*pic == NULL) {
    *pic = _doc;
    if (S_ISREG(ftype) && ((ftype) & S_IXUSR)) {
      sprintf(temp, "%s.s.xpm", fname);
      *pic = _client->GetPicture(temp);
      if (*pic == NULL) *pic = _app;
    }
    if (S_ISDIR(ftype)) *pic = _folder;
  }

  *lpic = is_link ? _slink : NULL;
}  


//---- Change current directory

void OXDesktopContainer::ChangeDirectory(const char *path) {
  if (chdir(path) == 0) DisplayDirectory();
}


//---- Display the contents of the current directory in the desktop.

void OXDesktopContainer::DisplayDirectory() {

  RemoveAllIcons();
  CreateIcons();

  //---- this automatically calls layout
  Sort(_sortType);

  //---- notify application about the changes
  OContainerMessage message(MSG_CONTAINER, MSG_SELCHANGED, -1, 0,
                            _total, _selected);
  SendMessage(_msgObject, &message);

  MapSubwindows();
  Init();           // read positions back from desktoprc file
  ArrangeIcons();   // calls InitPos(), which in order calls MapWindow()
}


//---- This function creates icons from current dir.

void OXDesktopContainer::CreateIcons() {
  DIR *dirp;
  struct stat sbuf, lbuf;
  struct dirent *dp;
  int type, is_link;
  unsigned long size;
  char *t, *name, filename[PATH_MAX];
  const OPicture *pic, *lpic;

  if (stat(_recycled->GetRecycleBin(), &sbuf) == 0)
    _rb_mtime = sbuf.st_mtime;

  if (stat(".", &sbuf) == 0)
    _st_mtime = sbuf.st_mtime;

  if ((dirp = opendir(".")) == NULL) return;
  
  while ((dp = readdir(dirp)) != NULL) {
    name = dp->d_name;
    if (strcmp(name, ".") && strcmp(name, "..")) {
      strcpy(filename, name);
      type = 0;
      size = 0;
      is_link = False;
//      if (lstat(name, &sbuf) == 0) {
      if ((stat(name, &sbuf) == 0) || (lstat(name, &sbuf) == 0)) {
        if (lstat(name, &lbuf) != 0) continue;
        is_link = S_ISLNK(lbuf.st_mode);
        type = lbuf.st_mode;
        size = lbuf.st_size;
        if (is_link) {
          type = sbuf.st_mode;
          size = sbuf.st_size;
        }
      } else {
        char msg[256];

        sprintf(msg, "Can't read file attributes of \"%s\": %s.",
                     name, strerror(errno));
        new OXMsgBox(_client->GetRoot(), NULL /*_toplevel*/,
                     new OString("Desktop Manager"), new OString(msg),
                     MB_ICONSTOP, ID_OK);
      }

      pic = NULL;

      if (S_ISDIR(sbuf.st_mode)) {
        if (_isRecycleBin(filename))
          pic = _isEmptyDir(filename) ? _rbempty : _rbfull;
        lpic  = is_link ? _slink : NULL;
      }

      if (!pic)
        GetIconPics(&pic, &lpic, filename, type, is_link);

      AddIcon(pic, lpic, new OString(name), type, size);
      _total++;
    }
  } 

  closedir (dirp);

  SetLayoutManager(new ODesktopLayout(this, _client, 8));
  Layout();
}

//--- This adds a new icon to the desktop
//    mode can be link, copy, move or create.
//    'ftype' is used only on create

OXDesktopIcon *OXDesktopContainer::NewIcon(int x, int y, URL *url,
                                           int ftype, int action) {
  struct stat sbuf, lbuf;
  int type, is_link;
  unsigned long size;
  char *t, *name, filename[PATH_MAX];
  char msg[256];
  const OPicture *pic, *lpic;
  OXDesktopIcon *f;

  switch (action) {
    case ACTION_MOVE:
      return NULL;
      break;

    case ACTION_COPY:
      return NULL;
      break;

    case ACTION_SYMLINK:
      if (symlink(url->full_path, url->filename) != 0) {
        sprintf(msg, "Can't create soft link to \"%s\": %s.",
                     url->full_path, strerror(errno));
        new OXMsgBox(_client->GetRoot(), NULL,
                     new OString("Desktop Manager"), new OString(msg),
                     MB_ICONSTOP, ID_OK);
        return NULL;
      }
      break;

    case ACTION_CREATE:
      if (S_ISDIR(ftype)) {
        if (MakePath(url->filename, 0755) != 0) {
          sprintf(msg, "Can't create folder \"%s\": %s.",
                       url->filename, strerror(errno));
          new OXMsgBox(_client->GetRoot(), NULL,
                       new OString("Desktop Manager"), new OString(msg),
                       MB_ICONSTOP, ID_OK);
          return NULL;
        }
      } else {  // regular file
        int fd = creat(url->filename, 0644);
        if (fd < 0) {
          sprintf(msg, "Can't create file \"%s\": %s.",
                       url->filename, strerror(errno));
          new OXMsgBox(_client->GetRoot(), NULL,
                       new OString("Desktop Manager"), new OString(msg),
                       MB_ICONSTOP, ID_OK);
          return NULL;
        } else {
          close(fd);
        }
      }
      break;

    default:
      return NULL;
  }

  if (stat(".", &sbuf) == 0) _st_mtime = sbuf.st_mtime;

  name = url->filename;

  strcpy(filename, name);
  type = 0;
  size = 0;
  is_link = False;

  if ((stat(name, &sbuf) == 0) || (lstat(name, &sbuf) == 0)) {
    if (lstat(name, &lbuf) != 0) return NULL;
    is_link = S_ISLNK(lbuf.st_mode);
    type = lbuf.st_mode;
    size = lbuf.st_size;
    if (is_link) {
      type = sbuf.st_mode;
      size = sbuf.st_size;
    }
  } else {
    sprintf(msg, "Can't read file attributes of \"%s\": %s.",
                 name, strerror(errno));
    new OXMsgBox(_client->GetRoot(), NULL,
                 new OString("Desktop Manager"), new OString(msg),
                 MB_ICONSTOP, ID_OK);
  }

  pic = NULL;

  if (S_ISDIR(sbuf.st_mode)) {
    if (_isRecycleBin(filename))
      pic = _isEmptyDir(filename) ? _rbempty : _rbfull;
    lpic  = is_link ? _slink : NULL;
  }

  if (!pic)
    GetIconPics(&pic, &lpic, filename, type, is_link);

  f = AddIcon(pic, lpic, new OString(name), type, size);
  f->Move(x, y);
  f->InitPos();
  f->PlaceIcon(x, y);

  _total++;

  return f;
} 

int OXDesktopContainer::CreateObject(int x, int y, int objtype) {
  int i, retc = False;
  char name[100];
  URL url("file://localhost/dummy");

  switch (objtype) {
    case M_FILE_NEWFOLDER:
      strcpy(name, "New Folder");
      i = 1;
      while (access(name, F_OK) == 0) sprintf(name, "New Folder (%d)", i++);
      if (url.filename) delete[] url.filename;
      url.filename = StrDup(name);
      if (NewIcon(x, y, &url, S_IFDIR, ACTION_CREATE)) retc = True;
      break;

    default:
      break;
  }

  return retc;
}


int OXDesktopContainer::_isRecycleBin(const char *path) {
  char current[PATH_MAX], tmp[PATH_MAX];

  // shouldn't we use realpath() here?

  getcwd(current, PATH_MAX);
  if (chdir(path) != 0) return False;

  getcwd(tmp, PATH_MAX);
  chdir(current);

  return (strcmp(tmp, _recycled->GetRecycleBin()) == 0);
}

int OXDesktopContainer::_isEmptyDir(const char *dir) {
  DIR *d;
  struct dirent *dp;

  d = opendir(dir);
  if (!d) return True;

  while ((dp = readdir(d)) != NULL) {
    if (strcmp(dp->d_name, ".") && strcmp(dp->d_name, "..")) {
      closedir(d);
      return False;
    }
  }

  closedir(d);
  return True;
}

const OXDesktopIcon *OXDesktopContainer::GetNextSelected(void **current) {
  SListFrameElt *p;
  OXDesktopIcon *f;

  p = (SListFrameElt *) *current;
  p = (p == NULL) ? _flist : p->next;
  for (; p != NULL; p = p->next) {   
    f = (OXDesktopIcon *) p->frame;
    if (f->IsActive()) {
      *current = (void *) p;
      return f;
    }
  }

  return NULL;
}

void OXDesktopContainer::RemoveAllIcons() {
  SListFrameElt *ptr, *next;

  ptr = _flist;
  while(ptr) {
    next = ptr->next;
    ptr->frame->DestroyWindow();
    delete ptr->frame;
    // the layout is shared, thus don't delete ptr->layout;
    delete ptr;
    ptr = next;
  }
  _flist = _ftail = NULL;
  _selected = _total = 0;
  _last_active = NULL;
}

void OXDesktopContainer::UnselectAll() {
  SListFrameElt *ptr;
  OXDesktopIcon *f;

  for (ptr = _flist; ptr != NULL; ptr = ptr->next) {
    f = (OXDesktopIcon *) ptr->frame;
    f->Activate(False);  
  }
  _last_active = NULL;
}

void OXDesktopContainer::ArrangeIcons() {
  SListFrameElt *ptr;
  OXDesktopIcon *f;

  for (ptr = _flist; ptr != NULL; ptr = ptr->next) {
    f = (OXDesktopIcon *) ptr->frame;
    f->InitPos();
  }
}

OXFrame *OXDesktopContainer::GetFrameFromPoint(int x, int y) {
  SListFrameElt *ptr;

  for (ptr = _flist; ptr != NULL; ptr = ptr->next) {
    if (ptr->frame->IsVisible()) {
      if (ptr->frame->Contains(x - ptr->frame->GetX(),
                               y - ptr->frame->GetY()))
        return ptr->frame;
    }
  }

  return this;
}

//----------------------------------------------------------------------

//--- XDND stuff:

Atom OXDesktopContainer::HandleDNDenter(Atom *typelist) {

  for (int i = 0; typelist[i] != None; ++i)
    if (typelist[i] == URI_list) return URI_list;

  return None;
}

int OXDesktopContainer::HandleDNDleave() {
  return True;
}

Atom OXDesktopContainer::HandleDNDposition(int x, int y, Atom action,
                                           int xroot, int yroot) {
  _dndx = xroot;
  _dndy = yroot;

  // we accept any of the standard xdnd actions...
  return action;
}

int OXDesktopContainer::HandleDNDdrop(ODNDdata *data) {
  SListFrameElt *ptr;
  OXDesktopIcon *i;

printf("OXDesktopContainer: dnd drop (%s)\n", (char *) data->data);

  // First, check whether the dropped object is an existing desktop icon
  // that was being dragged to a new position. If so, just move the icon
  // to the new position and return.

  for (ptr = _flist; ptr; ptr = ptr->next) {
    i = (OXDesktopIcon *) ptr->frame;
    if (i->GetId() == _dndManager->GetSource()) {
      i->PlaceIcon();
      return True;
    }
  }

  // Next, check whether the dropped file exists already on the desktop
  // (full paths coincide) in case somebody was dragging from desktop
  // directory using explorer. If so, just move the icon to the last
  // position cached in HandleDNDposition().

  char *str = (char *) data->data;
  if (str) {
    char cwd[PATH_MAX];

    getcwd(cwd, PATH_MAX);

    URL url(str);

    if (strcmp(url.directory, cwd) == 0) {
      for (ptr = _flist; ptr; ptr = ptr->next) {
        i = (OXDesktopIcon *) ptr->frame;
        if (strcmp(i->GetName()->GetString(), url.filename) == 0) {
          i->PlaceIcon(_dndx, _dndy);
          return True;
        }
      }

      // If we are here it means that the file was not found, but in the
      // other hand it must have existed... Shall we create it here!?

      return True;

    } else {

      // Create a new desktop icon (copy, move or make shortcut according to
      // data->action). Note that NewIcon() will fail if the file already
      // exists.

      int action;

      if (data->action == ODNDmanager::DNDactionMove) {
        action = ACTION_MOVE;
      } else if (data->action == ODNDmanager::DNDactionCopy) {
        action = ACTION_COPY;
      } else if (data->action == ODNDmanager::DNDactionLink) {
        action = ACTION_SYMLINK;
      } else if (data->action == ODNDmanager::DNDactionAsk) {
        action = _dndpopup->PopupMenu(_dndx, _dndy);
      }

      NewIcon(_dndx, _dndy, &url, 0, action);

    }
  }

  return True;
}

int OXDesktopContainer::HandleDNDfinished() {
  DisplayDirectory();
  return True;
}
