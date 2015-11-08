/**************************************************************************

    This file is part of xclass, a Win95-looking GUI toolkit.
    Copyright (C) 1996, 1997 David Barth, Ricky Ralston, Hector Peraza.

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

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <sys/stat.h>

#include <X11/keysym.h>

#include <xclass/utils.h>
#include <xclass/OXMsgBox.h>
#include <xclass/OXFileDialog.h>
#include <xclass/OResourcePool.h>

#include "icons/tb-uplevel.xpm"
#include "icons/tb-newfolder.xpm"
#include "icons/tb-list.xpm"
#include "icons/tb-details.xpm"
#include "icons/tb-addfavor.xpm"
#include "icons/tb-favor.xpm"


#define IDF_CDUP        0
#define IDF_NEW_FOLDER  1
#define IDF_LIST        2
#define IDF_DETAILS     3
#define IDF_ADDFAVOR    4
#define IDF_FAVOR       5

#define IDF_FSLB        6
#define IDF_FTYPESLB    7


#define TYPE_MASK       (FDLG_OPEN | FDLG_SAVE | FDLG_BROWSE)

static char *defaultFiletypes[] = { "All files",      "*",
                                    "Document files", "*.doc",
                                    "Text files",     "*.txt",
                                    NULL,             NULL };


//--------------------------------------------------------------

OFileInfo::OFileInfo() { 
  filename = NULL;
  ini_dir = NULL;
  file_types = NULL;
  file_types_index = 0;
}
  
OFileInfo::~OFileInfo() { 
  if (filename) delete[] filename;
  if (ini_dir) delete[] ini_dir;
}


//--------------------------------------------------------------

OXFileDialog::OXFileDialog(const OXWindow *p, const OXWindow *main,
                           int dlg_type, OFileInfo *file_info) :
  OXTransientFrame(p, main, 10, 10, MAIN_FRAME | VERTICAL_FRAME) {

    _FileDialog(dlg_type, file_info);

    MapSubwindows();
    Resize(GetDefaultSize());

    //---- position relative to the parent's window

    CenterOnParent(False);

    //---- make the message box non-resizable

    SetWMSize(_w, _h);
    SetWMSizeHints(_w, _h, _w, _h, 0, 0);

    SetWindowName(_windowNameString);
    SetIconName(_windowNameString);
    SetClassHints("XCLASS", "dialog");

    SetMWMHints(MWM_DECOR_ALL | MWM_DECOR_RESIZEH | MWM_DECOR_MAXIMIZE |
                                MWM_DECOR_MINIMIZE | MWM_DECOR_MENU,
                MWM_FUNC_ALL | MWM_FUNC_RESIZE | MWM_FUNC_MAXIMIZE |
                               MWM_FUNC_MINIMIZE,
                MWM_INPUT_MODELESS);

    MapWindow();
    _client->WaitFor(this);
}

OXFileDialog::OXFileDialog(const OXWindow *p, const OXWindow *main) :
  OXTransientFrame(p, main, 10, 10, MAIN_FRAME | VERTICAL_FRAME) {
}

OXFileDialog::~OXFileDialog() {
  delete _lb; delete _lvbf;
  delete _lb1; delete _lb2; delete _lb3;
  delete _lhl; delete _lht; delete _lht1;
  delete _lvf; delete _lmain;
  if (_favorDir) delete[] _favorDir;
}

void OXFileDialog::_FileDialog(int dlg_type, OFileInfo *file_info) {
  int i;
  const OPicture *pcdup, *pnewf, *plist, *pdetails, *paddfavor, *pfavor;
  char *lookinString;
  char *okButtonString;

  _init = False;
  _file_info = file_info;
  _favorDir = NULL;

  if (!_file_info->file_types)
    _file_info->file_types = defaultFiletypes;

  _htop = new OXHorizontalFrame(this, 10, 10);

  _lmain = new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 4, 4, 3, 1);

  //--- top toolbar elements
  _dlg_type = dlg_type;

  switch (_dlg_type & TYPE_MASK) {
    default:
    case FDLG_OPEN:
      lookinString      = "Look &in";
      okButtonString    = "&Open";
      _windowNameString = "Open";
      break;

    case FDLG_SAVE:
      lookinString      = "Save &in";
      okButtonString    = "&Save";
      _windowNameString = "Save as...";
      break;

    case FDLG_BROWSE:
      lookinString      = "Look &in";
      okButtonString    = "&Ok";
      _windowNameString = "Browse";
      break;
   }

  _lookin = new OXLabel(_htop, new OHotString(lookinString));
  _tree_lb = new OXFileSystemDDListBox(_htop, IDF_FSLB);
  _tree_lb->Associate(this);

  // try user-defined toolbar pixmaps first...

  pcdup = _client->GetPicture("tb-uplevel.xpm");
  pnewf = _client->GetPicture("tb-newfolder.xpm");
  plist = _client->GetPicture("tb-list.xpm");
  pdetails = _client->GetPicture("tb-details.xpm");

  // use our default pixmaps for the missing ones...

  if (!pcdup)
    pcdup = _client->GetPicture("tb-uplevel.xpm", XCP_tb_uplevel_xpm);
  if (!pnewf)
    pnewf = _client->GetPicture("tb-newfolder.xpm", XCP_tb_newfolder_xpm);
  if (!plist)
    plist = _client->GetPicture("tb-list.xpm", XCP_tb_list_xpm);
  if (!pdetails)
    pdetails = _client->GetPicture("tb-details.xpm", XCP_tb_details_xpm);

  if (!(pcdup && pnewf && plist && pdetails))
    FatalError("OXFileDialog: missing toolbar pixmap(s).");

  _cdup = new OXPictureButton(_htop, pcdup, IDF_CDUP);
  _newf = new OXPictureButton(_htop, pnewf, IDF_NEW_FOLDER);
  _list = new OXPictureButton(_htop, plist, IDF_LIST);
  _details = new OXPictureButton(_htop, pdetails, IDF_DETAILS);

  _cdup->SetTip("Up one level");
  _newf->SetTip("Create new folder");
  _list->SetTip("List");
  _details->SetTip("Details");

  _cdup->Associate(this);
  _newf->Associate(this);
  _list->Associate(this);
  _details->Associate(this);

  _cdup->TakeFocus(False);
  _newf->TakeFocus(False);
  _list->TakeFocus(False);
  _details->TakeFocus(False);

  if (_dlg_type & FDLG_FAVOURITES) {
    paddfavor = _client->GetPicture("tb-addfavor.xpm");
    pfavor = _client->GetPicture("tb-favor.xpm");

    if (!paddfavor)
      paddfavor = _client->GetPicture("tb-addfavor.xpm", XCP_tb_addfavor_xpm);
    if (!pfavor)
      pfavor = _client->GetPicture("tb-favor.xpm", XCP_tb_favor_xpm);

    if (!(paddfavor && pfavor))
      FatalError("OXFileDialog: missing toolbar pixmap(s).");

    const char *userRoot = _client->GetResourcePool()->GetUserRoot();
    _favorDir = new char[strlen(userRoot) + 12];

    sprintf(_favorDir, "%s/favourites", userRoot);

    if (access(_favorDir, R_OK | W_OK | X_OK) != 0) {
      if (MakePath(_favorDir, 0777) != 0) {
        char tmp[256];
        sprintf(tmp, "Error creating favourites folder \"%s\":\n%s.",
                _favorDir, strerror(errno));
        new OXMsgBox(_client->GetRoot(), _toplevel,
                     new OString("Error"), new OString(tmp),
                     MB_ICONSTOP, ID_OK);
        // shall we default to normal file dialog?
      }
    }
    _addFavor = new OXPictureButton(_htop, paddfavor, IDF_ADDFAVOR);
    _favor = new OXPictureButton(_htop, pfavor, IDF_FAVOR);

    _addFavor->SetTip("Add to favourites");
    _favor->SetTip("Go to favourites folder");

    _addFavor->Associate(this);
    _favor->Associate(this);

    _addFavor->TakeFocus(False);
    _favor->TakeFocus(False);
  }

  _list->SetType(BUTTON_STAYDOWN);
  _details->SetType(BUTTON_STAYDOWN);

  _lhl = new OLayoutHints(LHINTS_LEFT | LHINTS_CENTER_Y,  5,  5, 2, 2); 
  _lht = new OLayoutHints(LHINTS_LEFT | LHINTS_EXPAND_Y, 10,  0, 2, 2); 
  _lb1 = new OLayoutHints(LHINTS_LEFT | LHINTS_CENTER_Y,  8,  0, 2, 2); 
  _lb2 = new OLayoutHints(LHINTS_LEFT | LHINTS_CENTER_Y,  0, 15, 2, 2);
  _lb3 = new OLayoutHints(LHINTS_LEFT | LHINTS_CENTER_Y,  0,  0, 2, 2);

  _tree_lb->Resize(200, _tree_lb->GetDefaultHeight());

  _htop->AddFrame(_lookin, _lhl);
  _htop->AddFrame(_tree_lb, _lht); 
  _htop->AddFrame(_cdup, _lb1);
  if (_dlg_type & FDLG_FAVOURITES) {
    _htop->AddFrame(_favor, _lb1);
    _htop->AddFrame(_addFavor, _lb3);
  }
  _htop->AddFrame(_newf, _lb1);
  _htop->AddFrame(_list, _lb1);
  _htop->AddFrame(_details, _lb2);

  AddFrame(_htop, _lmain);

  //--- file view

  _fv = new OXFileList(this, -1, GetResourcePool()->GetMimeTypes(),
                       NULL, 400, 161);
  _fv->Associate(this);

  // The initialization of OXFileList and OXFileSystemDDlistBox
  // contents is done in HandleMapNotify()

  _fv->SetViewMode(LV_LIST);
  _fv->Sort(SORT_BY_TYPE);

  _list->SetState(BUTTON_ENGAGED);

  AddFrame(_fv, _lmain);

  //--- file name and types

  int fw = (_dlg_type & FDLG_FAVOURITES) ? 260 : 220;

  _hf = new OXHorizontalFrame(this, 10, 10);

  _vf = new OXVerticalFrame(_hf, 10, 10);
  _lvf = new OLayoutHints(LHINTS_LEFT | LHINTS_EXPAND_Y |
                          LHINTS_EXPAND_X);

  _hfname = new OXHorizontalFrame(_vf, 10, 10);

  _lfname = new OXLabel(_hfname, new OHotString("File &name:"));
  _fname = new OXTextEntry(_hfname, new OTextBuffer(PATH_MAX+10));
  _fname->Resize(fw, _fname->GetDefaultHeight());

  _lht1 = new OLayoutHints(LHINTS_RIGHT | LHINTS_EXPAND_Y, 0, 20, 2, 2); 

  _hfname->AddFrame(_lfname, _lhl);
  _hfname->AddFrame(_fname, _lht1);

  _vf->AddFrame(_hfname, _lvf);

  _hftype = new OXHorizontalFrame(_vf, 10, 10);

  _lftypes = new OXLabel(_hftype, new OHotString("Files of &type:"));
  _ftypes = new OXDDListBox(_hftype, IDF_FTYPESLB);
  _ftypes->Associate(this);
  _ftypes->Resize(fw, /*_fname*/_ftypes->GetDefaultHeight());

  if (_file_info->file_types) {
    for (i = 0; _file_info->file_types[i] != NULL; i += 2)
      _ftypes->AddEntry(new OString(_file_info->file_types[i]), i);

    int index = _file_info->file_types_index;
    if (index >= i / 2)
      index = (i / 2) - 1;
    if (index < 0)
      index = 0;

    _ftypes->Select(index);

    _fname->Clear();
    switch (_dlg_type & TYPE_MASK) {
      case FDLG_OPEN:
      case FDLG_BROWSE:
        _fname->AddText(0, _file_info->file_types[2 * index + 1]);
        break;

      case FDLG_SAVE:
        if (_file_info->filename) {
          _fname->AddText(0, _file_info->filename);
          _defName = StrDup(_file_info->filename);
        }
        break;
    }
    _fv->SetFileFilter(_file_info->file_types[2 * index + 1], False);
  }

  _hftype->AddFrame(_lftypes, _lhl);
  _hftype->AddFrame(_ftypes, _lht1);

  _vf->AddFrame(_hftype, _lvf);

  _hf->AddFrame(_vf, _lvf);

  //--- Open/Save and Cancel buttons

  _vbf = new OXVerticalFrame(_hf, 10, 10, FIXED_WIDTH);
  _lvbf = new OLayoutHints(LHINTS_LEFT | LHINTS_CENTER_Y);

  _lb = new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 0, 0, 2, 2);

  _ok = new OXTextButton(_vbf, new OHotString(okButtonString), ID_OK);
  _cancel = new OXTextButton(_vbf, new OHotString("Cancel"), ID_CANCEL);

  _ok->Associate(this);
  _cancel->Associate(this);

  SetDefaultAcceptButton(_ok);
  SetDefaultCancelButton(_cancel);
  SetFocusOwner(_fname);

  _vbf->AddFrame(_ok, _lb);
  _vbf->AddFrame(_cancel, _lb);

  int width = max(_ok->GetDefaultWidth(), _cancel->GetDefaultWidth()) + 20;
  _vbf->Resize(width, _vbf->GetDefaultHeight());

  _hf->AddFrame(_vbf, _lvbf);

  AddFrame(_hf, _lmain);

  //---- grab accelerators

  _GrabAltKey(XK_i);
  _GrabAltKey(XK_n);
  _GrabAltKey(XK_t);
}

int OXFileDialog::CloseWindow() {
  if (_file_info->filename) delete[] _file_info->filename;
  _file_info->filename = NULL;
  return OXTransientFrame::CloseWindow();
}

int OXFileDialog::HandleKey(XKeyEvent *event) {
  if ((event->type == KeyPress) && (event->state & Mod1Mask)) {
    int keysym = XKeycodeToKeysym(GetDisplay(), event->keycode, 0);
    switch (keysym) {
      case XK_i: _tree_lb->RequestFocus(); break;
      case XK_n: _fname->RequestFocus(); break;
      case XK_t: _ftypes->RequestFocus(); break;
      default: return OXTransientFrame::HandleKey(event);
    }
    return True;
  }
  return OXTransientFrame::HandleKey(event);
}

int OXFileDialog::HandleMapNotify(XMapEvent *event) {

  OXTransientFrame::HandleMapNotify(event);

  if (!_init) {
    char path[PATH_MAX];

    DefineCursor(GetResourcePool()->GetWaitCursor());
    XFlush(GetDisplay());

    if (_file_info->ini_dir) {
      _fv->ChangeDirectory(_file_info->ini_dir);
      delete[] _file_info->ini_dir;  // in case ChangeDirectory fails...
    } else {
      _fv->ChangeDirectory(".");
    }

    _tree_lb->UpdateContents(getcwd(path, PATH_MAX));
    _file_info->ini_dir = StrDup(path);

    DefineCursor(None);

    _init = True;
  }

  return True;
}

int OXFileDialog::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg;
  OItemViewMessage *cmsg;
  OXTreeLBEntry *e;
  OXTextLBEntry *te;
  OFileItem *f;
  struct stat sbuf;
  std::vector<OItem *> items;

  switch (msg->type) {
  case MSG_BUTTON:
  case MSG_DDLISTBOX:
    wmsg = (OButtonMessage *) msg;
    switch (wmsg->action) { 
    case MSG_CLICK:
      switch (wmsg->id) {

      //---- buttons

      case ID_OK:
#if 0
        if (_fv->HasFocus() && (_fv->NumSelected() == 1)) {
          items = _fv->GetSelectedItems();
          f = (OFileItem *) items[0];
          if (S_ISDIR(f->GetFileType())) {
            _ChangeDirectory(f->GetName()->GetString());
            break;
          }
        }
#endif
        if (stat(_fname->GetString(), &sbuf) == 0 && S_ISDIR(sbuf.st_mode)) {
          _ChangeDirectory(_fname->GetString());
        } else if (strspn(_fname->GetString(), "?*") > 0) {
          _fv->SetFileFilter(_fname->GetString());
        } else {
          if (_file_info->filename) delete[] _file_info->filename;
          _file_info->filename = StrDup(_fname->GetString());
          delete this;
        }
        break;

      case ID_CANCEL:
        if (_file_info->filename) delete[] _file_info->filename;
        _file_info->filename = NULL;
        delete this;
        break;

      case IDF_CDUP:
        _ChangeDirectory("..");
        break;

      case IDF_NEW_FOLDER:
        break;

      case IDF_LIST:
        _fv->SetViewMode(LV_LIST);
        _details->SetState(BUTTON_UP);
        break;

      case IDF_DETAILS:
        _fv->SetViewMode(LV_DETAILS);
        _list->SetState(BUTTON_UP);
        break;

      case IDF_FAVOR:
        if (_dlg_type & FDLG_FAVOURITES) {  // paranoia check
          _ChangeDirectory(_favorDir);
        }
        break;

      case IDF_ADDFAVOR:
        if (_dlg_type & FDLG_FAVOURITES) {  // paranoia check
          if (_fv->NumSelected() == 1) { // otherwise use current dir?
            char path[PATH_MAX], oldpath[PATH_MAX], tmp[256];

            items = _fv->GetSelectedItems();
            f = (OFileItem *) items[0];

            sprintf(oldpath, "%s/%s", _file_info->ini_dir,
                    f->GetName()->GetString());
            sprintf(path, "%s/%s", _favorDir, f->GetName()->GetString());
            if (symlink(oldpath, path) != 0) {
              sprintf(tmp, "Error adding \"%s\" to favourites:\n%s.",
                      oldpath, strerror(errno));
              new OXMsgBox(_client->GetRoot(), _toplevel,
                           new OString("Error"), new OString(tmp),
                           MB_ICONSTOP, ID_OK);
              return False;
            }
          }
        }
        break;

      //---- drop-down list boxes

      case IDF_FSLB:
        e = (OXTreeLBEntry *) _tree_lb->GetSelectedEntry();
        if (e) _ChangeDirectory(e->GetPath()->GetString());
        break;

      case IDF_FTYPESLB:
        te = (OXTextLBEntry *) _ftypes->GetSelectedEntry();
        if (te) {
          if ((_dlg_type & TYPE_MASK) != FDLG_SAVE) {
            _fname->Clear();
            _fname->AddText(0, _file_info->file_types[te->ID()+1]);
          }
          _file_info->file_types_index = te->ID();
          _fv->SetFileFilter(_file_info->file_types[te->ID()+1]);
          _client->NeedRedraw(_fname);
        }
        break;
      }
      break;

    default:
      break;
    } // switch (msg->action)
    break;

  case MSG_LISTVIEW:
    cmsg = (OItemViewMessage *) msg;
    switch (cmsg->action) {
    case MSG_CLICK:
      if (cmsg->button == Button1) {
        if (_fv->NumSelected() == 1) {
          items = _fv->GetSelectedItems();
	  f = (OFileItem *) items[0];
          if (/*(_dlg_type & TYPE_MASK) == FDLG_SAVE && */S_ISDIR(f->GetFileType()))
            return True;
          _fname->Clear();
          _fname->AddText(0, f->GetName()->GetString());
          _client->NeedRedraw(_fname);
        }
      }
      break;

    case MSG_DBLCLICK:
      if (cmsg->button == Button1) {
        if (_fv->NumSelected() == 1) {
          items = _fv->GetSelectedItems();
          f = (OFileItem *) items[0];
          if (S_ISDIR(f->GetFileType())) {
            _ChangeDirectory(f->GetName()->GetString());
          } else {
            if (_file_info->filename) delete[] _file_info->filename;
            _file_info->filename = StrDup(f->GetName()->GetString());
            delete this;
          }
        }
      }
      break;

    default:
      break;

    } // switch (msg->action)
    break;

  default:
    break;

  } // switch (msg->type)

  return True;
}

void OXFileDialog::_ChangeDirectory(const char *path) {
  char tmp[PATH_MAX];

  DefineCursor(GetResourcePool()->GetWaitCursor());
  XFlush(GetDisplay());

  if (_fv->ChangeDirectory(path) != 0) {
    sprintf(tmp, "Can't read directory \"%s\": %s.", path, strerror(errno));
    new OXMsgBox(_client->GetRoot(), _toplevel, new OString("Error"),
                 new OString(tmp), MB_ICONSTOP, ID_OK);
  }

  _tree_lb->UpdateContents(getcwd(tmp, PATH_MAX));
  if (_file_info->ini_dir) delete[] _file_info->ini_dir;
  _file_info->ini_dir = StrDup(tmp);

  DefineCursor(None);
  XFlush(GetDisplay());
}

void OXFileDialog::_GrabAltKey(int keysym) {

  int keycode = XKeysymToKeycode(GetDisplay(), keysym);

  XGrabKey(GetDisplay(), keycode, Mod1Mask,
           _id, True, GrabModeAsync, GrabModeAsync);
  XGrabKey(GetDisplay(), keycode, Mod1Mask | Mod2Mask,
           _id, True, GrabModeAsync, GrabModeAsync);
  XGrabKey(GetDisplay(), keycode, Mod1Mask | LockMask,
           _id, True, GrabModeAsync, GrabModeAsync);
  XGrabKey(GetDisplay(), keycode, Mod1Mask | Mod2Mask | LockMask,
           _id, True, GrabModeAsync, GrabModeAsync);
}
