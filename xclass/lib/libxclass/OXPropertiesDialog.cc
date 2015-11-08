/**************************************************************************

    This file is part of xclass.
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
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <time.h>
#include <grp.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <xclass/utils.h>
#include <xclass/OXClient.h>
#include <xclass/OResourcePool.h>
#include <xclass/OMimeTypes.h>
#include <xclass/OXTransientFrame.h>
#include <xclass/OX3dLines.h>
#include <xclass/OXTab.h>
#include <xclass/OXButton.h>
#include <xclass/OXLabel.h>
#include <xclass/OXIcon.h>
#include <xclass/OXTextEntry.h>
#include <xclass/OXMsgBox.h>
#include <xclass/OXGroupFrame.h>
#include <xclass/O2ColumnsLayout.h>
#include <xclass/OString.h>
#include <xclass/OPicture.h>
#include <xclass/OXPropertiesDialog.h>

#include "icons/folder.s.xpm"
#include "icons/app.s.xpm"
#include "icons/doc.s.xpm"
#include "icons/slink.s.xpm"


//-------------------------------------------------------------------

OXPropertiesDialog::OXPropertiesDialog(const OXWindow *p, const OXWindow *main,
                    OString *fname, unsigned long options) :
  OXTransientFrame(p, main, 100, 100, options) {
    int ax, ay, width;
    Window wdummy;
    char name[PATH_MAX], wname[PATH_MAX];

    _folder = _client->GetPicture("folder.s.xpm");
    if (!_folder)
      _folder = _client->GetPicture("folder.s.xpm", XCP_folder_s_xpm);

    _app = _client->GetPicture("app.s.xpm");
    if (!_app)
      _app = _client->GetPicture("app.s.xpm", XCP_app_s_xpm);

    _doc = _client->GetPicture("doc.s.xpm");
    if (!_doc)
      _doc = _client->GetPicture("doc.s.xpm", XCP_doc_s_xpm);

    _slink = _client->GetPicture("slink.s.xpm");
    if (!_slink)
      _slink = _client->GetPicture("slink.s.xpm", XCP_slink_s_xpm);

    if (!_folder || !_app || !_doc || !_slink)
      FatalError("OXPropertiesDialog: Missing required pixmap(s)");

    _fname = fname;
    _tname = new OString("");
    strcpy(name, _fname->GetString());

    StatFile();

    //---- Tab widget

    _tab = new OXTab(this, 320, 300);
    Ltab = new OLayoutHints(LHINTS_EXPAND_Y | LHINTS_EXPAND_X, 5, 5, 5, 5);

    Vly = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y, 10, 10, 10, 50);
    Lr  = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_TOP, 0, 0, 10, 10);

    // fill-in the tab widget:

    AddGeneralTab();
    AddAttributesTab();
    if (_is_link) AddTargetTab();

    AddFrame(_tab, Ltab);

    //---- frame with "OK Cancel Apply" buttons

    OXHorizontalFrame *bf = new OXHorizontalFrame(this, 60, 20, FIXED_WIDTH);

    // create the buttons
    Ok     = new OXTextButton(bf, new OHotString("OK"), ID_OK);
    Cancel = new OXTextButton(bf, new OHotString("Cancel"), ID_CANCEL);
    Apply  = new OXTextButton(bf, new OHotString("&Apply"), ID_APPLY);

    // buttons send messages to this dialog

    Ok->Associate(this);
    Cancel->Associate(this);
    Apply->Associate(this);

    // layout for buttons: top align, equally expand horizontally
    bly = new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 5, 0, 0, 0);

    // layout for the frame: place at bottom, some gap at the left
    bfly = new OLayoutHints(LHINTS_BOTTOM | LHINTS_RIGHT, 0, 5, 0, 4);

    bf->AddFrame(Ok, bly);
    bf->AddFrame(Cancel, bly);
    bf->AddFrame(Apply, bly);

    width = Ok->GetDefaultWidth();   
    width = max(width, Cancel->GetDefaultWidth());
    width = max(width, Apply->GetDefaultWidth());
    bf->Resize((width + 20) * 3, bf->GetDefaultHeight());

    // the "Apply" button is initially disabled,
    // "Ok" is the default enter action

    Apply->Disable();

    SetDefaultAcceptButton(Ok);
    SetDefaultCancelButton(Cancel);
    SetFocusOwner(Ok);

    AddFrame(bf, bfly);

    //---- Map subwindows, init the layout...

    MapSubwindows();
    Resize(GetDefaultSize());

    //---- position relative to the parent's window

    if (main) {
      XTranslateCoordinates(GetDisplay(),
                            main->GetId(), GetParent()->GetId(),
                            50, 50, &ax, &ay, &wdummy);

      int dw = _client->GetDisplayWidth();
      int dh = _client->GetDisplayHeight();

      if (ax < 10) ax = 10; else if (ax + _w + 10 > dw) ax = dw - _w - 10;
      if (ay < 20) ay = 20; else if (ay + _h + 50 > dh) ay = dh - _h - 50;

      Move(ax, ay);
      SetWMPosition(ax, ay);
    }

    //---- make dialog non-resizable

    SetWMSize(_w, _h);
    SetWMSizeHints(_w, _h, _w, _h, 0, 0);

    sprintf(wname, "%s Properties", name);
    SetWindowName(wname);
    SetIconName("Properties");
    SetClassHints("XCLASS", "dialog");

    SetMWMHints(MWM_DECOR_ALL | MWM_DECOR_RESIZEH | MWM_DECOR_MAXIMIZE /*|
                                MWM_DECOR_MINIMIZE*/ | MWM_DECOR_MENU,
                MWM_FUNC_ALL | MWM_FUNC_RESIZE | MWM_FUNC_MAXIMIZE /*| 
                               MWM_FUNC_MINIMIZE*/,
                MWM_INPUT_MODELESS);

    InitControls();

    MapWindow();

    // This is not a modal dialog, applications like explorer might
    // want to have several instances of the dialog running simultaneously.
    // Therefore we do not call _client->WaitFor(this) here.
}

OXPropertiesDialog::~OXPropertiesDialog() {

  _client->FreePicture(_folder);
  _client->FreePicture(_app);
  _client->FreePicture(_doc);
  _client->FreePicture(_slink);

  // Need to delete OLayoutHints objects only, OXCompositeFrame
  // takes care of the rest...

  delete bly; delete bfly;
  delete Ily; delete Tly;

  delete lhf1;
  delete Lr;
  delete Vly;

  delete Lc; delete Lc1;
  delete Lg; delete Ls;

  delete Ltab;
  delete _fname;
  delete _tname;
}

void OXPropertiesDialog::StatFile() {

  _lstat_errno = _stat_errno = 0;
  _type = 0;

  if (lstat(_fname->GetString(), &_lsbuf) == -1) {
    _lstat_errno = _stat_errno = errno;
    return;
  }

  _is_link = S_ISLNK(_lsbuf.st_mode);

  if (_is_link) {
    char tname[PATH_MAX];
    int len;

    len = readlink(_fname->GetString(), tname, PATH_MAX-1);
    if (len > 0) {
      tname[len] = '\0';
      _tname = new OString(tname);
    }

    if (stat(_fname->GetString(), &_sbuf) == -1) {
      _stat_errno = errno;
      return;
    }
  } else {
    _sbuf = _lsbuf;
  }
  _type = _sbuf.st_mode;

}

void OXPropertiesDialog::AddGeneralTab() {

  OXCompositeFrame *tf = _tab->AddTab(new OString("General"));

  OXVerticalFrame *vf1 = new OXVerticalFrame(tf, 10, 10);
  OXHorizontalFrame *hf1 = new OXHorizontalFrame(vf1, 10, 10);

  lhf1 = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_TOP, 0, 0, 10, 0);

  vf1->AddFrame(hf1, lhf1);

  _fileIcon = new OXFileIcon(hf1, _doc, NULL, CHILD_FRAME,
                             _defaultFrameBackground);

  _fileName = new OXTextEntry(hf1, NULL, 101);
  _fileName->ChangeOptions(_fileName->GetOptions() & 
                           ~(RAISED_FRAME | SUNKEN_FRAME | DOUBLE_BORDER));
  _fileName->SetBackgroundColor(_defaultFrameBackground);
  _fileName->Associate(this);

  Tly = new OLayoutHints(LHINTS_CENTER_Y | LHINTS_EXPAND_X, 10, 0, 0, 0);
  Ily = new OLayoutHints(LHINTS_CENTER_Y | LHINTS_LEFT, 5, 0, 3, 3);

  hf1->AddFrame(_fileIcon, Ily);
  hf1->AddFrame(_fileName, Tly);

  vf1->AddFrame(new OXHorizontal3dLine(vf1), Lr);

  //--- type, location, owner, group...

  Lc = new OLayoutHints(LHINTS_LEFT | LHINTS_TOP);

  OXCompositeFrame *d1 = new OXCompositeFrame(vf1, 10, 10);
  d1->SetLayoutManager(new O2ColumnsLayout(d1, 15, 0));
  vf1->AddFrame(d1, Lc);

  d1->AddFrame(new OXLabel(d1, new OString("Type:")));
  d1->AddFrame(ltype = new OXLabel(d1, new OString("Unknown")));
  d1->AddFrame(new OXLabel(d1, new OString("Location:")));
  d1->AddFrame(ldev = new OXLabel(d1, new OString("Unknown")));
  d1->AddFrame(new OXLabel(d1, new OString("Owner:")));
  d1->AddFrame(lusr = new OXLabel(d1, new OString("Unknown")));
  d1->AddFrame(new OXLabel(d1, new OString("Group:")));
  d1->AddFrame(lgrp = new OXLabel(d1, new OString("Unknown")));
  d1->AddFrame(new OXLabel(d1, new OString("Links:")));
  d1->AddFrame(llnks = new OXLabel(d1, new OString("Unknown")));
  d1->AddFrame(new OXLabel(d1, new OString("Size:")));
  d1->AddFrame(lsz = new OXLabel(d1, new OString("Unknown")));

  vf1->AddFrame(new OXHorizontal3dLine(vf1), Lr);

  //--- last accessed, last modified...

  OXCompositeFrame *d2 = new OXCompositeFrame(vf1, 10, 10);
  d2->SetLayoutManager(new O2ColumnsLayout(d2, 15, 0));
  vf1->AddFrame(d2, Lc);

  d2->AddFrame(new OXLabel(d2, new OString("Last accessed:")));
  d2->AddFrame(lacc = new OXLabel(d2, new OString("Unknown")));
  d2->AddFrame(new OXLabel(d2, new OString("Last modified:")));
  d2->AddFrame(lmod = new OXLabel(d2, new OString("Unknown")));
  d2->AddFrame(new OXLabel(d2, new OString("Last changed:")));
  d2->AddFrame(lchg = new OXLabel(d2, new OString("Unknown")));

  //--- add the vertical frame to the tab

  tf->AddFrame(vf1, Vly);

}

void OXPropertiesDialog::AddAttributesTab() {

  OXCompositeFrame *tf = _tab->AddTab(new OString("Attributes"));

  OXVerticalFrame *vf2 = new OXVerticalFrame(tf, 10, 10);

  //--- file access permissions controls

  Lc1 = new OLayoutHints(LHINTS_LEFT | LHINTS_TOP, 0, 0, 5, 0);
  vf2->AddFrame(new OXLabel(vf2, new OString("Access permissions:")), Lc1);
  OXHorizontalFrame *hf2 = new OXHorizontalFrame(vf2, 10, 10);
  vf2->AddFrame(hf2, Lr);

  OXGroupFrame *umode = new OXGroupFrame(hf2, new OString("User"));
  OXGroupFrame *gmode = new OXGroupFrame(hf2, new OString("Group"));
  OXGroupFrame *omode = new OXGroupFrame(hf2, new OString("Others"));
  Lg = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y, 1, 1, 0, 0);
  hf2->AddFrame(umode, Lg);
  hf2->AddFrame(gmode, Lg);
  hf2->AddFrame(omode, Lg);

  _ur = new OXCheckButton(umode, new OHotString("Read"),    71);
  _uw = new OXCheckButton(umode, new OHotString("Write"),   72);
  _ux = new OXCheckButton(umode, new OHotString("Execute"), 73);
  _gr = new OXCheckButton(gmode, new OHotString("Read"),    74);
  _gw = new OXCheckButton(gmode, new OHotString("Write"),   75);
  _gx = new OXCheckButton(gmode, new OHotString("Execute"), 76);
  _or = new OXCheckButton(omode, new OHotString("Read"),    77);
  _ow = new OXCheckButton(omode, new OHotString("Write"),   78);
  _ox = new OXCheckButton(omode, new OHotString("Execute"), 79);

  _ur->Associate(this);
  _uw->Associate(this);
  _ux->Associate(this);
  _gr->Associate(this);
  _gw->Associate(this);
  _gx->Associate(this);
  _or->Associate(this);
  _ow->Associate(this);
  _ox->Associate(this);

  umode->AddFrame(_ur, Lc1);
  umode->AddFrame(_uw, Lc1);
  umode->AddFrame(_ux, Lc1);
  gmode->AddFrame(_gr, Lc1);
  gmode->AddFrame(_gw, Lc1);
  gmode->AddFrame(_gx, Lc1);
  omode->AddFrame(_or, Lc1);
  omode->AddFrame(_ow, Lc1);
  omode->AddFrame(_ox, Lc1);

  Ls = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_TOP, 0, 0, 5, 0);
  vf2->AddFrame(_sticky = new OXCheckButton(vf2,
                              new OHotString("Sticky"), 80), Ls);
  vf2->AddFrame(_suid = new OXCheckButton(vf2,
                            new OHotString("Set user ID on exec"), 81), Ls);
  vf2->AddFrame(_sgid = new OXCheckButton(vf2,
                            new OHotString("Set group ID on exec"), 82), Ls);

  _sticky->Associate(this);
  _suid->Associate(this);
  _sgid->Associate(this);

  //--- add the vertical frame to the tab

  tf->AddFrame(vf2, Vly);

}

void OXPropertiesDialog::AddTargetTab() {

  OXCompositeFrame *tf = _tab->AddTab(new OString("Target"));

  OXVerticalFrame *vf1 = new OXVerticalFrame(tf, 10, 10);
  OXHorizontalFrame *hf1 = new OXHorizontalFrame(vf1, 10, 10);

  vf1->AddFrame(hf1, lhf1);

  _targetIcon = new OXFileIcon(hf1, _doc, NULL, CHILD_FRAME,
                               _defaultFrameBackground);

  _targetName = new OXTextEntry(hf1, NULL, 102);
  _targetName->ChangeOptions(_targetName->GetOptions() & 
                             ~(RAISED_FRAME | SUNKEN_FRAME | DOUBLE_BORDER));
  _targetName->SetBackgroundColor(_defaultFrameBackground);
  _targetName->Associate(this);

  hf1->AddFrame(_targetIcon, Ily);
  hf1->AddFrame(_targetName, Tly);

  vf1->AddFrame(new OXHorizontal3dLine(vf1), Lr);

  //--- type, location, owner, group...

  OXCompositeFrame *d1 = new OXCompositeFrame(vf1, 10, 10);
  d1->SetLayoutManager(new O2ColumnsLayout(d1, 15, 0));
  vf1->AddFrame(d1, Lc);

  d1->AddFrame(new OXLabel(d1, new OString("Target type:")));
  d1->AddFrame(lttype = new OXLabel(d1, new OString("Unknown")));
  d1->AddFrame(new OXLabel(d1, new OString("Location:")));
  d1->AddFrame(ltdev = new OXLabel(d1, new OString("Unknown")));
  d1->AddFrame(new OXLabel(d1, new OString("Owner:")));
  d1->AddFrame(ltusr = new OXLabel(d1, new OString("Unknown")));
  d1->AddFrame(new OXLabel(d1, new OString("Group:")));
  d1->AddFrame(ltgrp = new OXLabel(d1, new OString("Unknown")));
  d1->AddFrame(new OXLabel(d1, new OString("Links:")));
  d1->AddFrame(ltlnks = new OXLabel(d1, new OString("Unknown")));
  d1->AddFrame(new OXLabel(d1, new OString("Size:")));
  d1->AddFrame(ltsz = new OXLabel(d1, new OString("Unknown")));

  vf1->AddFrame(new OXHorizontal3dLine(vf1), Lr);

  //--- last accessed, last modified...

  OXCompositeFrame *d2 = new OXCompositeFrame(vf1, 10, 10);
  d2->SetLayoutManager(new O2ColumnsLayout(d2, 15, 0));
  vf1->AddFrame(d2, Lc);

  d2->AddFrame(new OXLabel(d2, new OString("Last accessed:")));
  d2->AddFrame(ltacc = new OXLabel(d2, new OString("Unknown")));
  d2->AddFrame(new OXLabel(d2, new OString("Last modified:")));
  d2->AddFrame(ltmod = new OXLabel(d2, new OString("Unknown")));
  d2->AddFrame(new OXLabel(d2, new OString("Last changed:")));
  d2->AddFrame(ltchg = new OXLabel(d2, new OString("Unknown")));

  //--- add the vertical frame to the tab

  tf->AddFrame(vf1, Vly);

}

void OXPropertiesDialog::InitControls() {
  unsigned long bsize, fsize;
  char *ftype, tmp[256], *tformat;
  struct tm *tms;
  struct group *grp;
  struct passwd *pwd;

  UpdateName();
  UpdatePics();

  if (_lstat_errno) {
    sprintf(tmp, "Can't read file attributes of \"%s\": %s.",
                 _fname->GetString(), strerror(_lstat_errno));
    new OXMsgBox(_client->GetRoot(), this,
                 new OString("Error"), new OString(tmp),
                 MB_ICONSTOP, ID_OK);
    return;
  }

  if (_is_link) {
    if (_stat_errno) {
      sprintf(tmp, "Can't read target of soft link \"%s\": %s.",
                   _fname->GetString(), strerror(_stat_errno));
      new OXMsgBox(_client->GetRoot(), this,
                   new OString("Error"), new OString(tmp),
                   MB_ICONSTOP, ID_OK);
    }
  }

  // type of file...
  if (S_ISLNK(_sbuf.st_mode)) ftype = "Soft Link";
  else if (S_ISREG(_sbuf.st_mode)) ftype = "Regular file";
  else if (S_ISDIR(_sbuf.st_mode)) ftype = "File folder";
  else if (S_ISCHR(_sbuf.st_mode)) ftype = "Character device";
  else if (S_ISBLK(_sbuf.st_mode)) ftype = "Block device";
  else if (S_ISFIFO(_sbuf.st_mode)) ftype = "FIFO";
  else if (S_ISSOCK(_sbuf.st_mode)) ftype = "Socket";
  else ftype = "Unknown";

  sprintf(tmp, "%s%s", _is_link ? "Soft Link to " : "", ftype);
  ltype->SetText(new OString(tmp));

  sprintf(tmp, "device %d,%d", (int) (_lsbuf.st_dev/256),
                               (int) (_lsbuf.st_dev%256));
  ldev->SetText(new OString(tmp));

  // file ownership...
  pwd = getpwuid(_lsbuf.st_uid);
  if (pwd) {
    lusr->SetText(new OString(pwd->pw_name));
  } else {
    sprintf(tmp, "%d", _lsbuf.st_uid);
    lusr->SetText(new OString(tmp));
  }
  grp = getgrgid(_lsbuf.st_gid);
  if (grp) {
    lgrp->SetText(new OString(grp->gr_name));
  } else {
    sprintf(tmp, "%d", _lsbuf.st_gid);
    lgrp->SetText(new OString(tmp));
  }

  // links
  sprintf(tmp, "%d", _lsbuf.st_nlink);
  llnks->SetText(new OString(tmp));

  // file size
  fsize = bsize = _lsbuf.st_size;
  if (fsize > 1024) {
    fsize /= 1024;
    if (fsize > 1024) {
      // 3.7MB is more informative than just 3MB
      sprintf(tmp, "%ld.%ld MB (%ld bytes)", fsize/1024, (fsize%1024)/103, bsize);
    } else {
      sprintf(tmp, "%ld.%ld KB (%ld bytes)", bsize/1024, (bsize%1024)/103, bsize);
    }
  } else {
    sprintf(tmp, "%ld bytes", bsize);
  }
  lsz->SetText(new OString(tmp));

  // access, change and modification times...
  tformat = "%b %d, %Y  %R";

  tms = localtime(&_lsbuf.st_atime);
  strftime(tmp, 255, tformat, tms);
  lacc->SetText(new OString(tmp));

  tms = localtime(&_lsbuf.st_mtime);
  strftime(tmp, 255, tformat, tms);
  lmod->SetText(new OString(tmp));

  tms = localtime(&_lsbuf.st_ctime);
  strftime(tmp, 255, tformat, tms);
  lchg->SetText(new OString(tmp));

  if (_is_link && !_stat_errno) {
    // target type...
    if (S_ISLNK(_sbuf.st_mode)) ftype = "Soft Link";
    else if (S_ISREG(_sbuf.st_mode)) ftype = "Regular file";
    else if (S_ISDIR(_sbuf.st_mode)) ftype = "File folder";
    else if (S_ISCHR(_sbuf.st_mode)) ftype = "Character device";
    else if (S_ISBLK(_sbuf.st_mode)) ftype = "Block device";
    else if (S_ISFIFO(_sbuf.st_mode)) ftype = "FIFO";
    else if (S_ISSOCK(_sbuf.st_mode)) ftype = "Socket";
    else ftype = "Unknown";

    lttype->SetText(new OString(ftype));

    sprintf(tmp, "device %d,%d", (int) (_sbuf.st_dev/256),
                                 (int) (_sbuf.st_dev%256));
    ltdev->SetText(new OString(tmp));

    // target ownership...
    pwd = getpwuid(_sbuf.st_uid);
    if (pwd) {
      ltusr->SetText(new OString(pwd->pw_name));
    } else {
      sprintf(tmp, "%d", _sbuf.st_uid);
      ltusr->SetText(new OString(tmp));
    }
    grp = getgrgid(_sbuf.st_gid);
    if (grp) {
      ltgrp->SetText(new OString(grp->gr_name));
    } else {
      sprintf(tmp, "%d", _sbuf.st_gid);
      ltgrp->SetText(new OString(tmp));
    }

    // target links
    sprintf(tmp, "%d", _sbuf.st_nlink);
    ltlnks->SetText(new OString(tmp));

    // target size
    fsize = bsize = _sbuf.st_size;
    if (fsize > 1024) {
      fsize /= 1024;
      if (fsize > 1024) {
        // 3.7MB is more informative than just 3MB
        sprintf(tmp, "%ld.%ld MB (%ld bytes)", fsize/1024, (fsize%1024)/103, bsize);
      } else {
        sprintf(tmp, "%ld.%ld KB (%ld bytes)", bsize/1024, (bsize%1024)/103, bsize);
      }
    } else {
      sprintf(tmp, "%ld bytes", bsize);
    }
    ltsz->SetText(new OString(tmp));

    // target access, change and modification times...
    tformat = "%b %d, %Y  %R";

    tms = localtime(&_sbuf.st_atime);
    strftime(tmp, 255, tformat, tms);
    ltacc->SetText(new OString(tmp));

    tms = localtime(&_sbuf.st_mtime);
    strftime(tmp, 255, tformat, tms);
    ltmod->SetText(new OString(tmp));

    tms = localtime(&_sbuf.st_ctime);
    strftime(tmp, 255, tformat, tms);
    ltchg->SetText(new OString(tmp));
  }

  // access permissions...
  _ur->SetState((_lsbuf.st_mode & S_IRUSR) ? BUTTON_DOWN : BUTTON_UP);
  _uw->SetState((_lsbuf.st_mode & S_IWUSR) ? BUTTON_DOWN : BUTTON_UP);
  _ux->SetState((_lsbuf.st_mode & S_IXUSR) ? BUTTON_DOWN : BUTTON_UP);
  _gr->SetState((_lsbuf.st_mode & S_IRGRP) ? BUTTON_DOWN : BUTTON_UP);
  _gw->SetState((_lsbuf.st_mode & S_IWGRP) ? BUTTON_DOWN : BUTTON_UP);
  _gx->SetState((_lsbuf.st_mode & S_IXGRP) ? BUTTON_DOWN : BUTTON_UP);
  _or->SetState((_lsbuf.st_mode & S_IROTH) ? BUTTON_DOWN : BUTTON_UP);
  _ow->SetState((_lsbuf.st_mode & S_IWOTH) ? BUTTON_DOWN : BUTTON_UP);
  _ox->SetState((_lsbuf.st_mode & S_IXOTH) ? BUTTON_DOWN : BUTTON_UP);

  _sticky->SetState((_lsbuf.st_mode & S_ISVTX) ? BUTTON_DOWN : BUTTON_UP);
  _suid->SetState((_lsbuf.st_mode & S_ISUID) ? BUTTON_DOWN : BUTTON_UP);
  _sgid->SetState((_lsbuf.st_mode & S_ISGID) ? BUTTON_DOWN : BUTTON_UP);

  Layout();
}

void OXPropertiesDialog::UpdateName() {
  _fileName->Clear();
  _fileName->AddText(0, _fname->GetString());
  if (_is_link) {
    _targetName->Clear();
    _targetName->AddText(0, _tname->GetString());
  }
  Layout();
}

void OXPropertiesDialog::UpdatePics() {
  const OPicture *pic, *lpic;

  GetFilePictures(&pic, &lpic, (char *) _fname->GetString(), _type,
                  _is_link);

  _fileIcon->SetPics(pic, lpic);

  if (_is_link && !_stat_errno) {
    GetFilePictures(&pic, &lpic, (char *) _tname->GetString(), _sbuf.st_mode,
                    False);

    _targetIcon->SetPics(pic, lpic);
  }

  Layout();
}

void OXPropertiesDialog::GetFilePictures(const OPicture **pic,
                                         const OPicture **lpic,
                                         char *fname, int ftype,
                                         int is_link) {
  char temp[BUFSIZ];

  OMimeTypes *MimeTypesList = _client->GetResourcePool()->GetMimeTypes();

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

void OXPropertiesDialog::ApplyProperties() {
  mode_t newstate = 0;
  char msg[256];

  const char *fname = _fileName->GetTextBuffer()->GetString();
  if (strcmp(_fname->GetString(), fname) != 0) {
    if (access(fname, F_OK) == 0) {
      int retval;

      new OXMsgBox(_client->GetRoot(), this,
            new OString("Rename"),
            new OString("A file with the same name already exists. Overwrite?"),
            MB_ICONQUESTION, ID_YES | ID_NO, &retval);

      if (retval != ID_YES) {
        UpdateName(); //InitControls();  // restore old filename
        return;
      }
    }

    if (rename(_fname->GetString(), fname) != 0) {
      sprintf(msg, "Can't rename file: %s.", strerror(errno));
      new OXMsgBox(_client->GetRoot(), this,
                   new OString("Error"), new OString(msg),
                   MB_ICONSTOP, ID_OK);
      UpdateName(); //InitControls();
      return;
    }
    delete _fname;
    _fname = new OString(fname);
  }

  if (_ur->GetState() == BUTTON_DOWN) newstate = newstate | S_IRUSR;
  if (_uw->GetState() == BUTTON_DOWN) newstate = newstate | S_IWUSR;
  if (_ux->GetState() == BUTTON_DOWN) newstate = newstate | S_IXUSR;
  if (_gr->GetState() == BUTTON_DOWN) newstate = newstate | S_IRGRP;
  if (_gw->GetState() == BUTTON_DOWN) newstate = newstate | S_IWGRP;
  if (_gx->GetState() == BUTTON_DOWN) newstate = newstate | S_IXGRP;
  if (_or->GetState() == BUTTON_DOWN) newstate = newstate | S_IROTH;
  if (_ow->GetState() == BUTTON_DOWN) newstate = newstate | S_IWOTH;
  if (_ox->GetState() == BUTTON_DOWN) newstate = newstate | S_IXOTH;
  if (_sticky->GetState() == BUTTON_DOWN) newstate = newstate | S_ISVTX;
  if (_suid->GetState() == BUTTON_DOWN) newstate = newstate | S_ISUID;
  if (_sgid->GetState() == BUTTON_DOWN) newstate = newstate | S_ISGID;

  if (chmod(_fname->GetString(), newstate) != 0){
    sprintf(msg, "Can't change file mode: %s.", strerror(errno));
    new OXMsgBox(_client->GetRoot(), this,
                 new OString("Error"), new OString(msg),
                 MB_ICONSTOP, ID_OK);
    InitControls();
  }
  StatFile();  // update attrib bits, etc...
}

int OXPropertiesDialog::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;

  switch (msg->action) {
    case MSG_CLICK: // same as MSG_TEXTCHANGED for the text entry below...
      switch (msg->type) {
        case MSG_BUTTON:
          switch (wmsg->id) {
            case ID_OK:
              ApplyProperties();
              CloseWindow();
              break;

            case ID_CANCEL:
              CloseWindow();
              break;

            case ID_APPLY:
              ApplyProperties();
              Apply->Disable();
              break;
          }
          break;

        case MSG_RADIOBUTTON:
        case MSG_CHECKBUTTON:
          switch (wmsg->id) {

          }
          Apply->Enable();
          break;

        case MSG_TEXTENTRY:
          if (wmsg->id == 101) Apply->Enable();
          break;

        default:
          break;
      }
      break;

    default:
      break;
  }

  return True;
}
