#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <xclass/utils.h>
#include <xclass/OXMsgBox.h>
#include <xclass/OX3dLines.h>
#include <xclass/OXGroupFrame.h>

#include "OXIconSelDialog.h"

// shall we substitute the contents of the OXFSDDListBox with the
// picture pool paths?


//----------------------------------------------------------------------

OXIconSelDialog::OXIconSelDialog(const OXWindow *p, const OXWindow *t,
                                 OString *ret) :
  OXFileDialog(p, t) {

    _FileDialog(FDLG_OPEN, new OFileInfo);

    AddFrame(new OXHorizontal3dLine(this), 
             new OLayoutHints(LHINTS_EXPAND_X | LHINTS_TOP, 3, 3, 4, 4));

    OXHorizontalFrame *hf = new OXHorizontalFrame(this, 10, 10);
    AddFrame(hf, new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y, 3, 3, 0, 3));

    OXVerticalFrame *vf = new OXVerticalFrame(hf, 10, 10);
    hf->AddFrame(vf, new OLayoutHints(LHINTS_EXPAND_X | LHINTS_TOP));

    OXGroupFrame *gf = new OXGroupFrame(hf, new OString("Preview"), HORIZONTAL_FRAME);
    hf->AddFrame(gf, new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y));

    _lh1 = new OLayoutHints(LHINTS_LEFT | LHINTS_TOP, 10, 10, 10, 1);
    _lh2 = new OLayoutHints(LHINTS_LEFT | LHINTS_TOP, 10, 10, 1, 1);
    _lh3 = new OLayoutHints(LHINTS_LEFT | LHINTS_TOP, 10, 10, 1, 10);
    _lp = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_CENTER_Y, 10, 10, 0, 8);
    _lb = new OLayoutHints(LHINTS_CENTER_X | LHINTS_CENTER_Y, 0, 0, 0, 5);

    char cwd[PATH_MAX];
    getcwd(cwd, PATH_MAX);
    _picPool = new OPicturePool(_client, _client->GetResourcePool(), cwd);

    OString *s;

    s = new OString("Name:");
    _name = new OXLabel(vf, s);
    _name->SetTextAlignment(TEXT_LEFT | TEXT_CENTER_Y);
    vf->AddFrame(_name, new OLayoutHints(LHINTS_EXPAND_X | LHINTS_TOP,
                                         5, 2, 5, 5));

    s = new OString("Size:");
    _size = new OXLabel(vf, s);
    _size->SetTextAlignment(TEXT_LEFT | TEXT_CENTER_Y);
    vf->AddFrame(_size, new OLayoutHints(LHINTS_EXPAND_X | LHINTS_TOP,
                                         5, 2, 5, 5));

    _big = _picPool->GetPicture("Notepad.s.xpm", 32, 32);
    _bigIcon = new OXIcon(gf, _big, 32, 32);
    gf->AddFrame(_bigIcon, new OLayoutHints(LHINTS_LEFT | LHINTS_BOTTOM,
                                            10, 20, 20, 10));

    _small = _picPool->GetPicture("Notepad.s.xpm", 16, 16);
    _smallIcon = new OXIcon(gf, _small, 16, 16);
    gf->AddFrame(_smallIcon, new OLayoutHints(LHINTS_LEFT | LHINTS_BOTTOM,
                                              20, 10, 20, 10));

    SetWindowName("Select Icon");

    MapSubwindows();
    Resize(GetDefaultSize());

    //---- position relative to the parent's window

    if (t) {
      int ax, ay;
      Window wdummy;
      XTranslateCoordinates(GetDisplay(),
                            t->GetId(), GetParent()->GetId(),
                            (((OXFrame *) t)->GetWidth() - _w) >> 1,
                            (((OXFrame *) t)->GetHeight() - _h) >> 1,
                            &ax, &ay, &wdummy);
      Move(ax, ay);
      SetWMPosition(ax, ay);
    }

    //---- make the dialog box non-resizable

    SetWMSize(_w, _h);
    SetWMSizeHints(_w, _h, _w, _h, 0, 0);

    SetMWMHints(MWM_DECOR_ALL | MWM_DECOR_MAXIMIZE | MWM_DECOR_MENU,
                MWM_FUNC_ALL | MWM_FUNC_MAXIMIZE | MWM_FUNC_RESIZE,
                MWM_INPUT_MODELESS);

    MapWindow();

    _client->WaitFor(this);
}

OXIconSelDialog::~OXIconSelDialog() {
  delete _lh1;
  delete _lh2;
  delete _lh3;
  delete _lb;
  delete _file_info;
  delete _picPool;
}

int OXIconSelDialog::ProcessMessage(OMessage *msg) {
  OItemViewMessage *cmsg;
  OFileItem *f;
  std::vector<OItem *> items;

  switch (msg->type) {
  case MSG_LISTVIEW:
    cmsg = (OItemViewMessage *) msg;
    switch (cmsg->action) {
    case MSG_CLICK:
      if (cmsg->button == Button1) {
        if (_fv->NumSelected() == 1) {
          items = _fv->GetSelectedItems();
          f = (OFileItem *) items[0];
          if (!S_ISDIR(f->GetFileType())) {

            // hmmmm....
            char tmp[PATH_MAX];
            getcwd(tmp, PATH_MAX);
            _picPool->SetPath(tmp);

            const char *fname = f->GetName()->GetString();

            const OPicture *_pic = _picPool->GetPicture(fname);

            sprintf(tmp, "Name: %s", _pic ? fname : "");
            _name->SetText(new OString(tmp));

            if (_pic)
              sprintf(tmp, "Size: %d x %d", _pic->GetWidth(), _pic->GetHeight());
            else
              sprintf(tmp, "Size:");

            _size->SetText(new OString(tmp));

            if (_big) _picPool->FreePicture(_big);
            _big = _picPool->GetPicture(fname, 32, 32);
            _bigIcon->SetPicture(_big);

            if (_small) _picPool->FreePicture(_small);
            _small = _picPool->GetPicture(fname, 16, 16);
            _smallIcon->SetPicture(_small);

            if (_pic) _picPool->FreePicture(_pic);
          }
        }
      }
      break;
    }
  }

  return OXFileDialog::ProcessMessage(msg);
}
