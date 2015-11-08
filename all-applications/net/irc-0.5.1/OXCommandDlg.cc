#include <xclass/OXMsgBox.h>
#include <xclass/OX3dLines.h>
#include <xclass/O2ColumnsLayout.h>
#include <xclass/OXFont.h>

#include "OXCommandDlg.h"


//----------------------------------------------------------------------

OXCommandDlg::OXCommandDlg(const OXWindow *p, const OXWindow *main,
                           OString *wname, OString *text,
                           OString *cmdlb, OString *command,
                           OString *argslb, OString *args, int *retc) :
  OXTransientFrame(p, main, 10, 10) {

    int width = 0, height = 0;

    _retc = retc;
    _command = command;
    _args = args;

    if (_retc) *_retc = ID_CANCEL;

    L1 = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_TOP, 3, 3, 5, 5);

    //--- create the label for the top text

    L3 = new OLayoutHints(LHINTS_CENTER_X | LHINTS_TOP, 5, 5, 5, 0);
    OXLabel *lb = new OXLabel(this, text);
    lb->SetTextJustify(TEXT_JUSTIFY_CENTER);
    AddFrame(lb, L3);

    //--- create the frame for the text entry boxes

    OXCompositeFrame *cf = new OXCompositeFrame(this, 1, 1);
    cf->SetLayoutManager(new O2ColumnsLayout(cf, 8, 2));

    OXLabel *cmdLabel = new OXLabel(cf, cmdlb);
    _cmdEntry = new OXTextEntry(cf, new OTextBuffer(128), 11);
    if (_command) _cmdEntry->AddText(0, _command->GetString());

    OXLabel *argsLabel = new OXLabel(cf, argslb);
    _argsEntry = new OXTextEntry(cf, new OTextBuffer(128), 12);
    //if (_args) _argsEntry->AddText(0, _args->GetString());

    _cmdEntry->Resize(200, _cmdEntry->GetDefaultHeight());
    _argsEntry->Resize(200, _argsEntry->GetDefaultHeight());
    _cmdEntry->Associate(this);
    _argsEntry->Associate(this);

    cf->AddFrame(cmdLabel, NULL);
    cf->AddFrame(_cmdEntry, NULL);
    cf->AddFrame(argsLabel, NULL);
    cf->AddFrame(_argsEntry, NULL);

    AddFrame(cf, L1);

    //--- create the buttons and button frame

    OXCompositeFrame *ButtonFrame = new OXHorizontalFrame(this,
                                                    60, 20, FIXED_WIDTH);

    OXButton *ok = new OXTextButton(ButtonFrame, new OHotString("&OK"), ID_OK);
    ok->Associate(this);
    ButtonFrame->AddFrame(ok, L1);
    width = max(width, ok->GetDefaultWidth());
    SetDefaultAcceptButton(ok);

    OXButton *ca = new OXTextButton(ButtonFrame, new OHotString("&Cancel"), ID_CANCEL);
    ca->Associate(this);
    ButtonFrame->AddFrame(ca, L1);
    width = max(width, ca->GetDefaultWidth());
    SetDefaultCancelButton(ca);

    AddFrame(new OXHorizontal3dLine(this), L1);

    //--- place buttons at the bottom

    L2 = new OLayoutHints(LHINTS_BOTTOM | LHINTS_CENTER_X);
    AddFrame(ButtonFrame, L2);

    //--- Keep the buttons centered and with the same width

    ButtonFrame->Resize((width + 20) * 2, GetDefaultHeight());

    SetFocusOwner(_cmdEntry);

    MapSubwindows();
    
    width  = GetDefaultWidth();
    height = GetDefaultHeight();

    Resize(width, height);

    CenterOnParent();

    //---- make the dialog box non-resizable

    SetWMSize(width, height);
    SetWMSizeHints(width, height, width, height, 0, 0);

    //---- set names

    SetWindowName(wname->GetString());
    SetIconName(wname->GetString());  
    SetClassHints("XCLASS", "dialog");

    SetMWMHints(MWM_DECOR_ALL | MWM_DECOR_RESIZEH | MWM_DECOR_MAXIMIZE |
                                MWM_DECOR_MINIMIZE | MWM_DECOR_MENU,
                MWM_FUNC_ALL | MWM_FUNC_RESIZE | MWM_FUNC_MAXIMIZE |
                               MWM_FUNC_MINIMIZE,
                MWM_INPUT_MODELESS);

    MapWindow();
    _client->WaitFor(this);
}

OXCommandDlg::~OXCommandDlg() {
  delete L1;
  delete L2;
  delete L3;
}

int OXCommandDlg::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;

  switch (msg->type) {
    case MSG_BUTTON:
      switch (msg->action) {
        case MSG_CLICK:
          switch (wmsg->id) {
            case ID_OK:
              if (_command) {
                _command->Clear();
                _command->Append(_cmdEntry->GetString());
              }
              if (_args) {
                _args->Clear();
                _args->Append(_argsEntry->GetString());
              }
            case ID_CANCEL:
              if (_retc) *_retc = wmsg->id;
              CloseWindow();
              break;
          }
          break;
      }
      break;

  }

  return true;
}
