#include <fcntl.h> 
#include <unistd.h> 
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <xclass/utils.h>
#include <xclass/OXMsgBox.h>
#include <xclass/OXLabel.h>
#include <xclass/OXFont.h>
#include <xclass/OString.h>
#include <xclass/OFileHandler.h>
#include <xclass/OXProgressBar.h>

#include "OXIrc.h"
#include "OXDCCFile.h"

#define DCC_MODE_CLIENT  0
#define DCC_MODE_SERVER  1

#define DCC_BLOCK_SIZE   1024

extern char *filetypes[];


//----------------------------------------------------------------------

OXDCCFileConfirm::OXDCCFileConfirm(const OXWindow *p, const OXWindow *main,
                              const char *nick, const char *filename,
                              const char *size, OFileInfo *retn, int *reti) :
  OXTransientFrame(p, main, 10, 10) {

    int width = 0, height = 0;

    if (retn) fi = retn;
    if (reti) iret = reti;

    if (iret) *iret = ID_DCC_REJECT;

    L1 = new OLayoutHints(LHINTS_CENTER_Y | LHINTS_EXPAND_X, 3, 3, 0, 0);
    L2 = new OLayoutHints(LHINTS_CENTER_X | LHINTS_CENTER_Y);

    OString *temp = new OString("");
    temp->Append(nick);
    temp->Append(" wishes to send you ");
    temp->Append(filename);
    temp->Append("\nwhich is ");
    temp->Append(size);
    temp->Append(" bytes long.");

    OXLabel *lb = new OXLabel(this, temp);
    lb->SetTextJustify(TEXT_JUSTIFY_CENTER);
    AddFrame(lb, L2);

    // button frame
    OXCompositeFrame *btnf = new OXHorizontalFrame(this, 60, 20, FIXED_WIDTH);

    // ok button 
    _ok = new OXTextButton(btnf, new OHotString("&Save"), ID_DCC_SAVE);
    btnf->AddFrame(_ok, L1);
    _ok->Associate(this);

    width = max(width, _ok->GetDefaultWidth());

    // saveas button
    _saveas = new OXTextButton(btnf, new OHotString("S&ave As..."), ID_DCC_SAVE_AS);
    _saveas->Associate(this);
    btnf->AddFrame(_saveas, L1);

    width = max(width, _saveas->GetDefaultWidth());

    // reject button
    _reject = new OXTextButton(btnf, new OHotString("&Reject"), ID_DCC_REJECT);
    btnf->AddFrame(_reject, L1);
    _reject->Associate(this);

    width = max(width, _reject->GetDefaultWidth());

    // ignore button
    _ignore = new OXTextButton(btnf, new OHotString("&Ignore"), ID_DCC_IGNORE);
    btnf->AddFrame(_ignore, L1);
    _ignore->Associate(this);

    width = max(width, _ignore->GetDefaultWidth());

    // add button frame and resize it allowing a 20 pixel border
    AddFrame(btnf, new OLayoutHints(LHINTS_BOTTOM | LHINTS_CENTER_X, 0, 0, 0, 5));
    btnf->Resize(((width + 6) * 4), btnf->GetDefaultHeight());

    _ok->Resize(width, _ok->GetDefaultHeight());
    _saveas->Resize(width, _saveas->GetDefaultHeight());
    _reject->Resize(width, _reject->GetDefaultHeight());
    _ignore->Resize(width, _ignore->GetDefaultHeight());

    //_focusMgr->SetFocusOwner(_ok);
    SetFocusOwner(_ok);

    MapSubwindows();
    width  = GetDefaultWidth() + 20;
    height = GetDefaultHeight() + 20;

    Resize(width, height);

    CenterOnParent();

    //---- make the dialog box non-resizable

    SetWMSize(width, height);
    SetWMSizeHints(width, height, width, height, 0, 0);

    SetWindowName("DCC File Confirm");
    SetIconName("DCC File Confirm");
    SetClassHints("fOXIrc", "dialog");

    MapWindow();

    if (filename) {
      name = StrDup(filename);
      if (fi->filename) delete[] fi->filename;
      fi->filename = StrDup(filename);
    }

    _client->WaitFor(this); 
}

int OXDCCFileConfirm::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;

  switch (msg->action) {
    case MSG_CLICK:
      switch (msg->type) {
        case MSG_BUTTON:
          switch (wmsg->id) {
            case ID_DCC_SAVE:
              if (iret) *iret = ID_DCC_SAVE;
              CloseWindow();
              break;

            case ID_DCC_SAVE_AS:
              {
              OFileInfo f2;
              f2.filename = StrDup(fi->filename);
              f2.file_types = filetypes;
              f2.MimeTypesList = NULL;
              if (fi->filename) f2.filename = StrDup(fi->filename);
              new OXFileDialog(_client->GetRoot(), this, FDLG_SAVE, &f2);
              if (f2.filename) {
                if (fi->filename) delete[] fi->filename;
                fi->filename = StrDup(f2.filename);
                if (fi->ini_dir) delete[] fi->ini_dir;
                fi->ini_dir = StrDup(f2.ini_dir);
                if (iret) *iret = ID_DCC_SAVE;
                CloseWindow();
              }
              }
              return true;
              break;

            case ID_DCC_REJECT:
            case ID_DCC_IGNORE:
              if (iret) *iret = wmsg->id;
              if (fi->filename) delete[] fi->filename;
              fi->filename = 0;
              CloseWindow();
              break;

            default:
              break;
          }

        default:
          break;
      }
      break;

    default:
      break;
  }

  return true;
}

OXDCCFileConfirm::~OXDCCFileConfirm() {
  delete L1;
  delete L2;
}


//----------------------------------------------------------------------

// This constructor is for receive (client) mode

OXDCCFile::OXDCCFile(const OXWindow *p, OXIrc *irc,
                     const char *nick, const char *filename,
                     char *ip, char *port, char *size, int *retc) :
  OXMainFrame(p, 100, 100) {
    char name[PATH_MAX];
    int  ste, width;

    _mode = DCC_MODE_CLIENT;

    _retc = retc;
    if (_retc) *_retc = ID_DCC_REJECT;

    _filesize = strtoul(size, NULL, 10);
    _irc = irc;
    _tcp = new OTcp();
    _connected = false;
    _fh = NULL;
    _file = -1;
    _serverSocket = false;
    _bytesread = 0;

    if (fi.filename) delete[] fi.filename;
    if (filename)
      fi.filename = StrDup(filename);
    else
      fi.filename = 0;

    fi.file_types = filetypes;
    fi.MimeTypesList = NULL;

    if (filename)
      _filename = StrDup(filename);
    else
      _filename = 0;

    if (getcwd(_dir, PATH_MAX))
      fi.ini_dir = StrDup(_dir);
    else
      fi.ini_dir = StrDup("/");

    OString *str = new OString("Saving as: ");
    str->Append(_filename);

    L1 = new OLayoutHints(LHINTS_NORMAL | LHINTS_EXPAND_X, 2, 2, 2, 2);
    L2 = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_CENTER_Y, 2, 2, 2, 2);

    _t1 = new OXLabel(this, str);
    width = _t1->GetDefaultWidth();

    _t2 = new OXLabel(this, new OString("Bytes Received:"));
    _t2->SetTextAlignment(TEXT_LEFT);

    width = max(width, _t2->GetDefaultWidth());
    _t3 = new OXLabel(this, new OString("Bytes Remaining:"));
    _t3->SetTextAlignment(TEXT_LEFT);

    width = max(width, _t3->GetDefaultWidth());

    AddFrame(_t1, L1);
    AddFrame(_t2, L1);
    AddFrame(_t3, L1);

    _prog = new OXProgressBar(this, 250, 20);
    _prog->SetRange(0, _filesize);
    _prog->SetPosition(0);
    _prog->ShowPercentage(true);
    AddFrame(_prog, L2);

    width = max(width, _prog->GetDefaultWidth());

    _cancel = new OXTextButton(this, new OHotString("&Cancel"), -1);
    _cancel->Associate(this);
    AddFrame(_cancel, new OLayoutHints(LHINTS_CENTER_X, 2, 2, 2, 2));

    Resize(width + 80, GetDefaultHeight() + 20);

    SetWindowName("DCC File");
    SetIconName("DCC File");
    SetClassHints("XDCC", "XDCC");

    MapSubwindows();
    Layout();
    MapWindow();

    new OXDCCFileConfirm(_client->GetRoot(), this, nick, filename, size,
                         &fi, &ste);

    if (_retc) *_retc = ste;

    if (ste == ID_DCC_SAVE) {
      snprintf(name, PATH_MAX, "%s/%s", fi.ini_dir, fi.filename);
      if (_OpenFile(name)) {
        if (_tcp->Connect(strtoul(ip, NULL, 10), atoi(port), true) > 0) {
          _fh = new OFileHandler(this, _tcp->GetFD(), XCM_WRITABLE);
          OString *str = new OString("Saving as: ");
          str->Append(fi.filename);
          _t1->SetText(str);
        } else {
          OString stitle("Connect");
          sprintf(name, "Failed to connect to %s (%s)",
                        _tcp->GetAddress(), strerror(errno));
          OString smsg(name);
          new OXMsgBox(_client->GetRoot(), this, &stitle, new OString(&smsg),
                       MB_ICONSTOP, ID_OK);
          if (_retc) *_retc = ID_DCC_REJECT;
          CloseWindow();
        }
      } else {
        if (_retc) *_retc = ID_DCC_REJECT;
        CloseWindow();
      }
    } else {
      CloseWindow();
    }
}

// This constructor is for sender (server) mode

OXDCCFile::OXDCCFile(const OXWindow *p, OXIrc *irc,
                     const char *nick, const char *filename) :
  OXMainFrame(p, 100, 100) {
    struct stat stbuf;
    int width = 0;

    _mode = DCC_MODE_SERVER;

    if (!filename || stat(filename, &stbuf) != 0) {
      CloseWindow();
      return;
    }

    _tcp = new OTcp();
    _irc = irc;
    _connected = false;
    _fh = NULL;
    _file = -1;
    _serverSocket = false;
    _bytessent = 0;
    _acksize = 0;

    _filename = StrDup(filename);
    _filesize = stbuf.st_size;

    OString *str = new OString("Sending ");
    str->Append(_filename);

    L1 = new OLayoutHints(LHINTS_NORMAL | LHINTS_EXPAND_X, 2, 2, 2, 2);
    L2 = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_CENTER_Y, 2, 2, 2, 2);

    _t1 = new OXLabel(this, str);
    width = _t1->GetDefaultWidth();

    _t2 = new OXLabel(this, new OString("Bytes Sent:"));
    _t2->SetTextAlignment(TEXT_LEFT);

    width = max(width, _t2->GetDefaultWidth());
    _t3 = new OXLabel(this, new OString("Bytes Remaining:"));
    _t3->SetTextAlignment(TEXT_LEFT);

    width = max(width, _t3->GetDefaultWidth());

    AddFrame(_t1, L1);
    AddFrame(_t2, L1);
    AddFrame(_t3, L1);

    _prog = new OXProgressBar(this, 250, 20);
    _prog->SetRange(0, _filesize);
    _prog->SetPosition(0);
    _prog->ShowPercentage(true);
    AddFrame(_prog, L2);

    width = max(width, _prog->GetDefaultWidth());

    _cancel = new OXTextButton(this, new OHotString("&Cancel"), -1);
    _cancel->Associate(this);
    AddFrame(_cancel, new OLayoutHints(LHINTS_CENTER_X, 2, 2, 2, 2));

    Resize(width + 80, GetDefaultHeight() + 20);

    SetWindowName("DCC File");
    SetIconName("DCC File");
    SetClassHints("XDCC", "XDCC");

    MapSubwindows();
    Layout();
    MapWindow();

    _file = open(_filename, O_RDONLY /* | O_NONBLOCK */);

    if (_file < 0) {
      OString stitle("Open");
      OString smsg("Failed to open file: ");
      smsg.Append(strerror(errno));
      new OXMsgBox(_client->GetRoot(), this, &stitle, new OString(&smsg),
                   MB_ICONSTOP, ID_OK);
      CloseWindow();
      return;
    }

}

OXDCCFile::~OXDCCFile() {
  if (_tcp) delete _tcp;
  if (_fh) delete _fh;
  if (_file >= 0) close(_file);
  delete L1;
  delete L2;
}

int OXDCCFile::CloseWindow() {
  // Tell OXIrc to remove us from the chain...
  _irc->RemoveDCC(this);
  return OXMainFrame::CloseWindow();
}

int OXDCCFile::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;

  switch (msg->action) {
    case MSG_CLICK:
      switch (msg->type) {
        case MSG_BUTTON:
          CloseWindow();  // cancel pressed...
          break;

        default:
          break;
      }
      break;

    default:
      break;
  }
  return false;
}

bool OXDCCFile::_OpenFile(char *name) {
  struct stat st;
  char *fname = name;
  OFileInfo f2;

  while (stat(fname, &st) == 0) {
    int rt;
    OString stitle("Save");
    OString smsg("A file with the same name already exists. Overwrite?");
    new OXMsgBox(_client->GetRoot(), this, &stitle, new OString(&smsg),
                 MB_ICONQUESTION, ID_YES | ID_NO, &rt);

    if (rt == ID_YES) break;

    f2.filename = StrDup(name);
    f2.file_types = filetypes;
    f2.MimeTypesList = NULL;
    new OXFileDialog(_client->GetRoot(), this, FDLG_SAVE, &f2);
    if (!f2.filename) return false;
    fname = f2.filename;
  }

  _file = open(fname, O_CREAT | O_WRONLY | O_TRUNC, 0644);

  if (_file < 0) {
    OString stitle("Save");
    OString smsg("Failed to create file: ");
    smsg.Append(strerror(errno));
    new OXMsgBox(_client->GetRoot(), this, &stitle, new OString(&smsg),
                 MB_ICONSTOP, ID_OK);
    return false;
  }

  return true;
}

int OXDCCFile::Listen(unsigned long *host, unsigned short *port) {
  int ret;

  if (_connected) _tcp->Close(); //Disconnect();
  ret = _tcp->Listen(host, port, _irc->GetOIrc()->GetOTcp());

  if (_fh) delete _fh;
  _fh = NULL;

  if (ret < 0) return ret;
  _fh = new OFileHandler(this, _tcp->GetFD(), XCM_READABLE);
  _serverSocket = True;
  //Log("Waiting for connection...", P_COLOR_SERVER_1);

  return ret;
}

int OXDCCFile::HandleFileEvent(OFileHandler *fh, unsigned int mask) {
  if (fh != _fh) return False;

  if (_mode == DCC_MODE_SERVER) {

    if (_serverSocket) {
      if (_fh) { delete _fh; _fh = NULL; }
      OTcp *newconn = new OTcp();
      int ret = newconn->Accept(_tcp->GetFD());  // write this better...
      delete _tcp;  // we do not have to continue
                    // listening on that port...
      _tcp = newconn;
      _tcp->Associate(this);
      if (ret >= 0) {
        char s[256];

        _fh = new OFileHandler(this, _tcp->GetFD(),
                               XCM_READABLE | XCM_WRITABLE);
        //sprintf(s, "Connection to %s:%d established",
        //            _dccServer->GetAddress(), _dccServer->GetPort());
        //Log(s, "green");
        _connected = True;
        _acksize = 0;
      } else {
        //Log("Connection failed", "red");
      }
      _serverSocket = False;

    } else {

      switch (mask) {
        case XCM_WRITABLE:
          if (_acksize == _bytessent) _SendSomeData();
          break;

        case XCM_READABLE:
          if (_acksize < _bytessent) {
            unsigned long bytestemp;

            if (_tcp->BinaryReceive((char *) &bytestemp,
                                    sizeof(unsigned long)) < 0) {
              // error
            } else {
              if (_fh) delete _fh;  // here and not below to avoid receiving file
              _fh = 0;              // events while the message box is active
              bytestemp = ntohl(bytestemp);
              //_acksize += bytestemp;
              _acksize = bytestemp;
              if (_acksize > _bytessent) {
                OString stitle("Sending File");
                OString smsg("Transfer error: "
                             "bad ACK size received from remote end.\n"
                             "Connection will be closed");
                new OXMsgBox(_client->GetRoot(), this, &stitle,
                             new OString(&smsg), MB_ICONSTOP, ID_OK);
                CloseWindow();
                return False;
              }
              _fh = new OFileHandler(this, _tcp->GetFD(),
                                     XCM_READABLE | XCM_WRITABLE);
            }
          }
          break;
      }
    }

  } else if (_mode == DCC_MODE_CLIENT) {

    switch (mask) {
      case XCM_WRITABLE:
        if (!_connected) {
          _connected = True;
          if (_fh) delete _fh;
          _fh = new OFileHandler(this, _tcp->GetFD(), XCM_READABLE);
        }
        break;

      case XCM_READABLE:
        if (!_serverSocket) _FetchSomeData();
        break;
    }
  }

  return True;
}

bool OXDCCFile::_FetchSomeData() {
  char buf[DCC_BLOCK_SIZE], char1[256];
  unsigned long bytestemp;

  int size = _tcp->BinaryReceive(buf, DCC_BLOCK_SIZE);

  if (size > 0) {
    write(_file, buf, size);
    _bytesread += size;
    bytestemp = htonl(_bytesread);

    sprintf(char1, "Bytes Received: %ld", _bytesread);
    _t2->SetText(new OString(char1));
    
    sprintf(char1, "Bytes Remaining: %ld", _filesize - _bytesread);
    _t3->SetText(new OString(char1));

    _prog->SetPosition(_bytesread);
    if (_tcp->BinarySend((char *) &bytestemp, sizeof(unsigned long)) < 0) {
      OString stitle("Reading File");
      OString smsg("Remote end closed connection.");
      new OXMsgBox(_client->GetRoot(), this, &stitle, new OString(&smsg),
                   MB_ICONSTOP, ID_OK);
      return false;
    }

    if (_fh) delete _fh;
    _fh = new OFileHandler(this, _tcp->GetFD(), XCM_READABLE);

  } else {
    if (_fh) delete _fh;  // here and not below to avoid recursive file events
    _fh = 0;              // while the message box is active
    if (_bytesread != _filesize) {
      OString stitle("Error Reading File");
      OString smsg("File size is different than reported");
      new OXMsgBox(_client->GetRoot(), this, &stitle, new OString(&smsg),
                   MB_ICONSTOP, ID_OK);
      return false;
    }
    close(_file);
    _file = -1;
    _tcp->Close();

    CloseWindow();
  }

  return true;
}

bool OXDCCFile::_SendSomeData() {
  char buf[DCC_BLOCK_SIZE], char1[256];

  int size = read(_file, &buf, DCC_BLOCK_SIZE);

  if (size > 0) {

    if (_tcp->BinarySend(buf, size) < 0) {
      OString stitle("Sending File");
      OString smsg("Remote end closed connection.");
      new OXMsgBox(_client->GetRoot(), this, &stitle, new OString(&smsg),
                   MB_ICONSTOP, ID_OK);
      return false;
    }

    _bytessent += size;

    sprintf(char1, "Bytes Sent: %ld", _bytessent);
    _t2->SetText(new OString(char1));
    
    sprintf(char1, "Bytes Remaining: %ld", _filesize - _bytessent);
    _t3->SetText(new OString(char1));

    _prog->SetPosition(_bytessent);
    if (_fh) delete _fh;
    _fh = new OFileHandler(this, _tcp->GetFD(), XCM_READABLE | XCM_WRITABLE);

  } else {
    if (_bytessent != _filesize) {  // we shouldn't bother about this...
      OString stitle("Error Sending File");
      OString smsg("File size changed during transfer?");
      new OXMsgBox(_client->GetRoot(), this, &stitle, new OString(&smsg),
                   MB_ICONSTOP, ID_OK);
      return false;
    }
    close(_file);
    _file = -1;
    if (_fh) delete _fh;
    _fh = 0;
    _tcp->Close();

    CloseWindow();
  }

  return true;
}
