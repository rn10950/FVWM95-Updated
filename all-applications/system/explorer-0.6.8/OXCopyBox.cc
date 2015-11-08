#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include <xclass/utils.h>
#include <xclass/OXMsgBox.h>
#include <xclass/OXTextButton.h>

#include "OXCopyBox.h"

#define POST_TIMER

// TODO:
// - Add support for multiple-file copies.
// - Make it more general, so it could be used for recursive deletes,
//   for example.


//----------------------------------------------------------------------

OXCopyBox::OXCopyBox(const OXWindow *p, const OXWindow *t,
                     const char *frompath, const char *topath) :
  OXTransientFrame(p, t, 10, 10) {
    OString *s;

    _init = False;
    _from_fd = _to_fd = -1;
    _fwrite = _fread = NULL;
    _t = NULL;
    _nread = _nwriten = 0;
    _fsize = _count = 0;
    _from_path = StrDup(frompath);
    _to_path = StrDup(topath);
    _buf = NULL;
    _bufsz = 0;

    _lh1 = new OLayoutHints(LHINTS_LEFT | LHINTS_TOP, 10, 10, 10, 1);
    _lh2 = new OLayoutHints(LHINTS_LEFT | LHINTS_TOP, 10, 10, 1, 1);
    _lh3 = new OLayoutHints(LHINTS_LEFT | LHINTS_TOP, 10, 10, 1, 10);
    _lp = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_CENTER_Y, 10, 10, 0, 8);
    _lb = new OLayoutHints(LHINTS_CENTER_X | LHINTS_CENTER_Y, 0, 0, 0, 5);

    s = new OString("Copying file 1 of 1");
    OXLabel *_fnum = new OXLabel(this, s);
    AddFrame(_fnum, _lh1);

    s = new OString("From: ");
    s->Append(frompath);
    OXLabel *_from = new OXLabel(this, s);
    AddFrame(_from, _lh2);

    s = new OString("To: ");
    s->Append(topath);
    OXLabel *_to = new OXLabel(this, s);
    AddFrame(_to, _lh3);

    _pb = new OXProgressBar(this, 250, 22, PROGBAR_MODE1);
    _pb->SetRange(0, 100);  // we'll change it later...
    _pb->ShowPercentage(True);
//    _pb->SetFloatingText(True);
    AddFrame(_pb, _lp);

    OXButton *cb = new OXTextButton(this, new OString(" Cancel "), ID_CANCEL);
    AddFrame(cb, _lb);

    SetWindowName("Copy");

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

    // not a modal dialog by default...
}

OXCopyBox::~OXCopyBox() {
  if (_t) delete _t;
  if (_fwrite) delete _fwrite;
  if (_fread) delete _fread;
  delete _lh1;
  delete _lh2;
  delete _lh3;
  delete _lb;
  if (_from_fd > 0) close(_from_fd);
  if (_to_fd > 0) close(_to_fd);
  delete[] _from_path;
  delete[] _to_path;
  if (_buf) delete _buf;
}

// abort any copy operation in progress

int OXCopyBox::CloseWindow() {
  // delete the destination file
  if (_to_fd > 0) unlink(_to_path);
  return OXTransientFrame::CloseWindow();
}

// We start the copy operation as soon as the window gets mapped.
// Note that we schedule file events for both the read and write
// operations, since one (or both) of these could be having place
// on a slow or networked device.

int OXCopyBox::HandleMapNotify(XMapEvent *event) {
  int retc;
  struct stat sbuf;

  if (!_init) {

    _from_fd = open(_from_path, O_RDONLY | O_NONBLOCK);
    if (_from_fd == -1) {
      FileError("Cannot open source file", _from_path, errno);
      CloseWindow();
      return True;
    }

    if (fstat(_from_fd, &sbuf) != 0) {
      FileError("Cannot stat source file", _from_path, errno);
      CloseWindow();  // we could go and copy it anyway!
      return True;
    }

    _fsize = sbuf.st_size;
#if 0
    _pb->SetRange(0, _fsize);
#endif

    _bufsz = _fsize / 8;
    if (_bufsz > 32768) _bufsz = 32768;
    if (_bufsz < 4096) _bufsz = 4096;
    _buf = new char[_bufsz];

    // check for the existence of the dest and warn the user...
    // when doing multiple file copies, include "Yes All" button.

    if (access(_to_path, F_OK) == 0) {
      new OXMsgBox(_client->GetRoot(), this,
            new OString("Copy"),
            new OString("A file with the same name already exists. "
                        "Overwrite?"),
            MB_ICONQUESTION, ID_YES | ID_NO, &retc);
      if (retc == ID_NO) {
        CloseWindow();
        return True;
      }
    }

    _to_fd = open(_to_path, O_CREAT | O_TRUNC | O_WRONLY | O_NONBLOCK,
                            sbuf.st_mode & 07777);
    if (_to_fd == -1) {
      FileError("Cannot create destination file", _to_path, errno);
      CloseWindow();  // this takes care of closing _from_fd
      return True;
    }

    // schedule a read operation

    _fread = new OFileHandler(this, _from_fd, XCM_READABLE);
    _init = True;
  }

  return True;
}

int OXCopyBox::HandleFileEvent(OFileHandler *f, unsigned int mask) {

  if (f == _fwrite) {
    int numwr = write(_to_fd, _buf, _nread);
    if (numwr < _nread) {  // write error?
      delete _fwrite;
      _fwrite = NULL;
      FileError("Error writing destination file", _to_path, errno);
      CloseWindow();
      return True;
    }
    _count += numwr;
#if 0
    _pb->SetPosition(_count);
#else
    _pb->SetPosition(_count*100/_fsize);
#endif

    delete _fwrite;
    _fwrite = NULL;
    _fread = new OFileHandler(this, _from_fd, XCM_READABLE);
    return True;
  } 

  if (f == _fread) {
    int numrd = read(_from_fd, _buf, _bufsz);
    if (numrd == 0) {  // end of file?
      close(_to_fd);
      _to_fd = -1;
#ifdef POST_TIMER
      _t = new OTimer(this, 250);
      delete _fread;
      _fread = NULL;
#else
      CloseWindow();
#endif
      return True;
    } else if (numrd < 0) {  // read error?
      delete _fread; // delete the _fread first!
      _fread = NULL;
      FileError("Error reading source file", _from_path, errno);
      CloseWindow();
      return True;
    }
    _nread = numrd;
    delete _fread;
    _fread = NULL;
    _fwrite = new OFileHandler(this, _to_fd, XCM_WRITABLE);
    return True;
  }

  return False;
}

int OXCopyBox::HandleTimer(OTimer *t) {
  if (t == _t) {
#if 0
    _pb->SetPosition(_count);
#else
    _pb->SetPosition(100);
#endif
    delete _t;
    _t = NULL;
    CloseWindow();
    return True;
  }
  return False;
}

int OXCopyBox::FileError(const char *msg, const char *fname, int errc) {
  OString *s = new OString(msg);
  if (fname) {
    s->Append(" \"");
    s->Append(fname);
    s->Append("\"");
  }
  s->Append(":\n");
  s->Append(strerror(errc));

  new OXMsgBox(_client->GetRoot(), this, new OString("Error"), s,
               MB_ICONSTOP, ID_CLOSE);

  return True;
}

int OXCopyBox::ProcessMessage(OMessage *msg) {
  switch (msg->type) {
    case MSG_BUTTON:
      switch(msg->action) {
        case MSG_CLICK:
          CloseWindow();
          break;
      }
      break;
  }
  return True;
}
