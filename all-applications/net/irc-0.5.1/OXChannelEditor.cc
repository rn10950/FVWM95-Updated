#include <xclass/utils.h>
#include <xclass/OXFileDialog.h>
#include <xclass/OXGroupFrame.h>

#include "OXChannelEditor.h"
#include "OXServerDlg.h"

#define CH_ED_AUTO_LOG		3000
#define CH_ED_AUTO_RAISE	3001
#define CH_ED_AUTO_BELL		3002
#define CH_ED_TRANSIENT		3003
#define CH_ED_BACKGROUND	3004
#define CH_ED_LOGFILE		3005
#define CH_ED_OK		3006
#define CH_ED_CANCEL		3007


extern OSettings *foxircSettings;

char *backtypes[] = { "Pictures",  "*.xpm|*.xbm",
                      NULL,        NULL };

char *logtypes[]  = { "Logfile",   "*.log",
                      "Any file",  "*",
                      NULL,        NULL };


//----------------------------------------------------------------------

OXChannelEditor::OXChannelEditor(const OXWindow *p, const OXWindow *main,
				 OChannelInfo *ci, int *retc) :
  OXTransientFrame(p, main, 10, 10) {

    int width = 0, height = 0;

    _ci = ci;
    _ret = retc;

    if (_ret) *_ret = false;
    
    _l1 = new OLayoutHints(LHINTS_EXPAND_Y | LHINTS_EXPAND_X, 5, 5, 5, 5);
    _l2 = new OLayoutHints(LHINTS_NORMAL);
    _l3 = new OLayoutHints(LHINTS_NORMAL, 5);
    _l4 = new OLayoutHints(LHINTS_CENTER_Y | LHINTS_CENTER_X);
    _l5 = new OLayoutHints(LHINTS_NORMAL, 3, 3, 1, 1);
    _l6 = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_CENTER_Y, 3, 3, 5, 5);
    _l7 = new OLayoutHints(LHINTS_CENTER_Y | LHINTS_EXPAND_X, 2, 2, 0, 0);

    OXCompositeFrame *f1 = new OXHorizontalFrame(this, 0, 0, 0);
    f1->SetLayoutManager(new O2ColumnsLayout(f1, 5, 5));
    AddFrame(f1, _l1);
    
    f1->AddFrame(new OXLabel(f1, new OString("Name")), NULL);
    _nameTE = new OXTextEntry(f1, new OTextBuffer(100));
    _nameTE->Associate(this);
    _nameTE->Resize(200, _nameTE->GetDefaultHeight());
    f1->AddFrame(_nameTE, NULL);

    const OPicture *pic = _client->GetPicture("openbutton.xpm");

    f1->AddFrame(new OXLabel(f1, new OString("Logfile")), NULL);

    OXHorizontalFrame *h1 = new OXHorizontalFrame(f1, 150, 20);
    _logfileTE = new OXTextEntry(h1, new OTextBuffer(100));
    _logfileTE->Associate(this);
    if (!pic) {
      _logfileBT = new OXTextButton(h1, new OString("Load..."), CH_ED_LOGFILE);
    } else {
      _logfileBT = new OXPictureButton(h1, pic, CH_ED_LOGFILE);
    }
    _logfileTE->Resize(195 - _logfileBT->GetDefaultWidth(),
                             _logfileTE->GetDefaultHeight());
    _logfileBT->Associate(this);
    h1->AddFrame(_logfileTE, _l2);
    h1->AddFrame(_logfileBT, _l3);
    f1->AddFrame(h1, NULL);

    f1->AddFrame(new OXLabel(f1, new OString("Background")), NULL);
    OXHorizontalFrame *h2 = new OXHorizontalFrame(f1, 150, 20);
    _backTE = new OXTextEntry(h2, new OTextBuffer(100));
    _backTE->Associate(this);
    if (!pic) {
      _fileBT = new OXTextButton(h2, new OString("Load..."), CH_ED_BACKGROUND);
    } else {
      _fileBT = new OXPictureButton(h2, pic, CH_ED_BACKGROUND);
    }
    _fileBT->Associate(this);
    _backTE->Resize(195 - _fileBT->GetDefaultWidth(),
                          _backTE->GetDefaultHeight());
    h2->AddFrame(_backTE, _l2);
    h2->AddFrame(_fileBT, _l3);
    f1->AddFrame(h2, NULL);

    OXCompositeFrame *f2 = new OXHorizontalFrame(this, 0, 0, 0);
    AddFrame(f2, _l6);

    OXGroupFrame *gf = new OXGroupFrame(f2, new OString("On Text"));
    gf->OldInsets(False);
    f2->AddFrame(gf, _l7);
    _raiseCB = new OXCheckButton(gf, new OHotString("Raise Window"), CH_ED_AUTO_RAISE);
    _raiseCB->Associate(this);
    gf->AddFrame(_raiseCB, _l5);

    _beepCB = new OXCheckButton(gf, new OHotString("Sound Bell"), CH_ED_AUTO_BELL);
    _beepCB->Associate(this);
    gf->AddFrame(_beepCB, _l5);

    OXGroupFrame *gf2 = new OXGroupFrame(f2, new OString("Options"));
    gf2->OldInsets(False);
    f2->AddFrame(gf2, _l7);
    
    _autoCB = new OXCheckButton(gf2, new OHotString("Auto Log"), CH_ED_AUTO_LOG);
    _autoCB->Associate(this);
    gf2->AddFrame(_autoCB, _l5);
    
    _transCB = new OXCheckButton(gf2, new OHotString("Transient"), CH_ED_TRANSIENT);
    _transCB->Associate(this);
    gf2->AddFrame(_transCB, _l5);

    AddFrame(new OXHorizontal3dLine(this), _l6);

    OXCompositeFrame *ButtonFrame = new OXHorizontalFrame(this,
                                                    60, 20, FIXED_WIDTH);
    AddFrame(ButtonFrame, _l4);
//    OLayoutHints *L1 = new OLayoutHints(LHINTS_CENTER_Y | LHINTS_EXPAND_X, 3, 3, 0, 5);

    _ok = new OXTextButton(ButtonFrame, new OHotString("&OK"), CH_ED_OK);
    _ok->Associate(this);
    ButtonFrame->AddFrame(_ok, _l6);
    width = max(width, _ok->GetDefaultWidth());

    _cancel = new OXTextButton(ButtonFrame, new OHotString("&Cancel"), CH_ED_CANCEL);
    _cancel->Associate(this);
    ButtonFrame->AddFrame(_cancel, _l6);
    width = max(width, _cancel->GetDefaultWidth());
    ButtonFrame->Resize((width + 20) * 2, ButtonFrame->GetDefaultHeight());

    Layout();
    MapSubwindows();
    
    width  = GetDefaultWidth();
    height = GetDefaultHeight();
    Resize(width, height);

    CenterOnParent();

    _LoadInfo();

    SetWMSize(width, height);
    SetWMSizeHints(width, height, width, height, 0, 0);
    SetMWMHints(MWM_DECOR_ALL | MWM_DECOR_RESIZEH | MWM_DECOR_MAXIMIZE |
                                MWM_DECOR_MINIMIZE | MWM_DECOR_MENU,
                MWM_FUNC_ALL | MWM_FUNC_RESIZE | MWM_FUNC_MAXIMIZE |
                               MWM_FUNC_MINIMIZE,
                MWM_INPUT_MODELESS);

    SetWindowName("Channel Editor");
    SetIconName("Channel Editor");
    SetClassHints("fOXIrc", "dialog");

    SetDefaultAcceptButton(_ok);
    SetDefaultCancelButton(_cancel);
    SetFocusOwner(_nameTE);

    MapSubwindows();
    Layout();
    MapWindow();
    _client->WaitFor(this);
}

OXChannelEditor::~OXChannelEditor() {
  delete _l1;
  delete _l2;
  delete _l3;
  delete _l4;
  delete _l5;
  delete _l6;
  delete _l7;
}

int OXChannelEditor::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;

  switch (msg->type) {
    case MSG_BUTTON:
      switch (msg->action) {
        case MSG_CLICK:
          switch (wmsg->id) {
            case CH_ED_OK:
              _SaveInfo();
            case CH_ED_CANCEL:
              CloseWindow();
              break;

            case CH_ED_BACKGROUND:
              _GetBack();
              break;

            case CH_ED_LOGFILE:
              _GetLog();
              break;

          }
          break;
      }
      break;
  }

  return True;
}

void OXChannelEditor::_LoadInfo() {
  
  if (!_ci) return;

  _nameTE->Clear();
  if (_ci->name) _nameTE->AddText(0, _ci->name);

  _logfileTE->Clear();
  if (_ci->logfile) _logfileTE->AddText(0, _ci->logfile);

  _backTE->Clear();
  if (_ci->background) _backTE->AddText(0, _ci->background);

  if (_ci->flags & TRANSIENT_WINDOW)
    _transCB->SetState(BUTTON_DOWN);
  else
    _transCB->SetState(BUTTON_UP);

  if (_ci->flags & AUTO_RAISE_WINDOW)
    _raiseCB->SetState(BUTTON_DOWN);
  else
    _raiseCB->SetState(BUTTON_UP);

  if (_ci->flags & AUTO_BEEP_WINDOW)
    _beepCB->SetState(BUTTON_DOWN);
  else
    _beepCB->SetState(BUTTON_UP);

  if (_ci->flags & AUTO_LOG)
    _autoCB->SetState(BUTTON_DOWN);
  else
    _autoCB->SetState(BUTTON_UP);

}

void OXChannelEditor::_SaveInfo(){

  if (_ci->name) delete[] _ci->name;
  _ci->name = NULL;
  if (_nameTE->GetTextLength() > 0)
    _ci->name = StrDup(_nameTE->GetString());

  if (_ci->logfile) delete[] _ci->logfile;
  _ci->logfile = NULL;
  if (_logfileTE->GetTextLength() > 0)
    _ci->logfile = StrDup(_logfileTE->GetString());

  if (_ci->background) delete[] _ci->background;
  if (_backTE->GetTextLength() > 0) {
    _ci->background = StrDup(_backTE->GetString());
    _ci->flags |= USES_BACKGROUND_PIX;
  } else {
    _ci->background = NULL;
    _ci->flags &= ~USES_BACKGROUND_PIX;
  }

  if (_autoCB->GetState() == BUTTON_DOWN)
    _ci->flags |= AUTO_LOG;
  else
    _ci->flags &= ~AUTO_LOG;

  if (_transCB->GetState() == BUTTON_DOWN)
    _ci->flags |= TRANSIENT_WINDOW;
  else
    _ci->flags &= ~TRANSIENT_WINDOW;

  if (_raiseCB->GetState() == BUTTON_DOWN)
    _ci->flags |= AUTO_RAISE_WINDOW;
  else
    _ci->flags &= ~AUTO_RAISE_WINDOW;

  if (_beepCB->GetState() == BUTTON_DOWN)
    _ci->flags |= AUTO_BEEP_WINDOW;
  else
    _ci->flags &= ~AUTO_BEEP_WINDOW;

  if (_ret) *_ret = true;
}

void OXChannelEditor::_GetBack() {
  OFileInfo fi;

  fi.MimeTypesList = NULL;
  fi.file_types = backtypes;
  new OXFileDialog(_client->GetRoot(), this, FDLG_OPEN, &fi);
  if (fi.filename) {
    if (_ci->background) delete[] _ci->background;
    _ci->background = new char[strlen(fi.filename) + strlen(fi.ini_dir) + 2];
    sprintf(_ci->background, "%s/%s", fi.ini_dir, fi.filename);
    _backTE->Clear();
    if (_ci->background) _backTE->AddText(0, _ci->background);
    //_LoadInfo();
  }
}

void OXChannelEditor::_GetLog() {
  OFileInfo fi;

  fi.MimeTypesList = NULL;
  fi.file_types = logtypes;
  new OXFileDialog(_client->GetRoot(), this, FDLG_OPEN, &fi);
  if (fi.filename) {
    if (_ci->logfile) delete[] _ci->logfile;
    _ci->logfile = new char[strlen(fi.filename) + strlen(fi.ini_dir) + 2];
    sprintf(_ci->logfile, "%s/%s", fi.ini_dir, fi.filename);
    _logfileTE->Clear();
    if (_ci->logfile) _logfileTE->AddText(0, _ci->logfile);
  }
}
