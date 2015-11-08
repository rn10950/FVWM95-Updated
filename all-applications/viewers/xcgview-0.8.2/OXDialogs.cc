#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>
#include <xclass/OXMsgBox.h>

#include "OXDialogs.h"

#include "printer-icon.xpm"

extern OXOutput *output_dlg;


//-----------------------------------------------------------------

OXOutput::OXOutput(const OXWindow *p, const OXWindow *t, int w, int h) :
  OXTransientFrame(p, t, w, h) {

  int ax, ay;
  Window wdummy;

  _closeButton = new OXTextButton(this, new OHotString("  &Close  "),
                                  ID_CLOSE);
  _closeButton->Associate(this);

  SetDefaultAcceptButton(_closeButton);
  SetDefaultCancelButton(_closeButton);

  //------ Canvas frame and text widget

  _text = new OXTextView(this, 100, 400,
                         SUNKEN_FRAME | DOUBLE_BORDER | OWN_BKGND,
                         _defaultDocumentBackground);

  AddFrame(_text, new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y));
  AddFrame(_closeButton, new OLayoutHints(LHINTS_CENTER_X | LHINTS_BOTTOM,
                                          0, 0, 3, 0));

  SetWindowName("Stdout / Stderr");
  SetClassHints("XCGhostView", "XCGhostView");

  //Resize(GetDefaultSize());
  MapSubwindows();
  Resize(360, 230);

  //---- position relative to the parent's window
 
  XTranslateCoordinates(GetDisplay(),
                        t->GetId(), GetParent()->GetId(),
                        50, 50, &ax, &ay, &wdummy);
  Move(ax, ay);

  MapWindow();
}

OXOutput::~OXOutput() {
}

int OXOutput::CloseWindow() {
  delete this;
  output_dlg = NULL;
  return True;
}

int OXOutput::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;

  switch (msg->action) {

    case MSG_CLICK:
      switch(msg->type) {

        case MSG_BUTTON:
          switch (wmsg->id) {
            case ID_CLOSE:
              CloseWindow();
              break;

            default:
              break;

          } // switch(id)
          break;

        default:
          break;

      } // switch(type)
      break;


    default:
      break;

  } // switch(action)

  return True;
}

int OXOutput::printf(char *fmt, ...) {
  va_list argptr;
  char str[1024];
  int cnt;

  va_start(argptr, fmt);
  cnt = vsnprintf(str, 1024, fmt, argptr);
  _text->AddText(str);
  va_end(argptr);
  return cnt;
}

//-----------------------------------------------------------------

OXPrintBox::OXPrintBox(const OXWindow *p, const OXWindow *main, int w, int h, 
                       char **printerName, char **printProg, int *ret_val, 
                       unsigned long options) :
  OXTransientFrame(p, main, w, h, options) {
    int i, ax, ay;
    Window wdummy;  

    pprinter = printerName;
    pprintCommand = printProg;
    ret = ret_val;
    ChangeOptions((GetOptions() & ~VERTICAL_FRAME) | HORIZONTAL_FRAME);

    f1 = new OXCompositeFrame(this, 60, 20, VERTICAL_FRAME | FIXED_WIDTH);
    f6 = new OXCompositeFrame(this, 60, 20, VERTICAL_FRAME);

    f5 = new OXCompositeFrame(f6, 60, 20, HORIZONTAL_FRAME);
    f2 = new OXCompositeFrame(f5, 60, 20, VERTICAL_FRAME);

    f3 = new OXCompositeFrame(f2, 60, 20, HORIZONTAL_FRAME);
    f4 = new OXCompositeFrame(f2, 60, 20, HORIZONTAL_FRAME);

    PrintButton = new OXTextButton(f1, new OHotString("&Print"), PB_OK);
    CancelButton = new OXTextButton(f1, new OHotString("&Cancel"), PB_CANCEL);
    f1->Resize(PrintButton->GetDefaultWidth()+40, GetDefaultHeight());

    PrintButton->Associate(this);
    CancelButton->Associate(this);

    SetDefaultAcceptButton(PrintButton);
    SetDefaultCancelButton(CancelButton);

    L1 = new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 2, 2, 2, 0);
    L2 = new OLayoutHints(LHINTS_TOP | LHINTS_RIGHT | LHINTS_EXPAND_X);
    L21 = new OLayoutHints(LHINTS_TOP | LHINTS_RIGHT, 2, 5, 10, 0);

    f1->AddFrame(PrintButton, L1);
    f1->AddFrame(CancelButton, L1);

    lprintCommand = new OXLabel(f3, new OHotString("Print &command:"));
    tbprintCommand = new OTextBuffer(20);
    tbprintCommand->AddText(0, *printProg);
    printCommand = new OXTextEntry(f3, tbprintCommand);
    printCommand->Associate(this);
    printCommand->Resize(100, printCommand->GetDefaultHeight());
    L5 = new OLayoutHints(LHINTS_LEFT | LHINTS_CENTER_Y, 3, 5, 0, 0);
    L6 = new OLayoutHints(LHINTS_RIGHT | LHINTS_CENTER_Y, 0, 2, 0, 0);

    f3->AddFrame(lprintCommand, L5);
    f3->AddFrame(printCommand, L6);

    lprinter = new OXLabel(f4, new OHotString("&Printer name:"));
    tbprinter = new OTextBuffer(20);
    tbprinter->AddText(0, *printerName);
    printer = new OXTextEntry(f4, tbprinter); 
    printer->Associate(this);
    printer->Resize(100, printer->GetDefaultHeight());
    f4->AddFrame(lprinter, L5);
    f4->AddFrame(printer, L6);

    f2->AddFrame(f3, L1);
    f2->AddFrame(f4, L1);
 
    L7 = new OLayoutHints(LHINTS_LEFT | LHINTS_CENTER_Y, 10, 10, 10, 0);

    printerPicture = _client->GetPicture("printer.xpm", printerIconData);
    printerIcon = new OXIcon(f5, printerPicture, 32, 34);
    f5->AddFrame(printerIcon, L7);
    f5->AddFrame(f2, L21);

    f6->AddFrame(f5, L2);

    L4 = new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 2, 2, 10, 0);

    _hline = new OXHorizontal3dLine(f6, 10, 10);
    f6->AddFrame(_hline, L4);

    _all = new OXRadioButton(f6, new OHotString("Print the &whole document"), PB_PRINTALL);
    _marked = new OXRadioButton(f6, new OHotString("Print only the &selected range"), PB_PRINTMARKED);

    _all->SetState(BUTTON_DOWN);
    _marked->SetState(BUTTON_UP);

    if (!(ret && (*ret & PB_PRINTALL))) _all->Disable();
    if (!(ret && (*ret & PB_PRINTMARKED))) _marked->Disable();

    _all->Associate(this);
    _marked->Associate(this);

    L3 = new OLayoutHints(LHINTS_TOP | LHINTS_LEFT, 10, 0, 0, 5);

    f6->AddFrame(_all, L3);
    f6->AddFrame(_marked, L3);

    AddFrame(f1, L21);
    AddFrame(f6, L2);

    MapSubwindows();
    Resize(GetDefaultSize());
    
    // position relative to the parent's window
    XTranslateCoordinates(GetDisplay(),
                          main->GetId(), GetParent()->GetId(),
                          (((OXFrame *) main)->GetWidth() - _w) >> 1,
                          (((OXFrame *) main)->GetHeight() - _h) >> 1,
                          &ax, &ay, &wdummy);
    Move(ax, ay);
    SetMWMHints(MWM_DECOR_ALL | MWM_DECOR_MAXIMIZE | MWM_DECOR_MENU,
                MWM_FUNC_ALL  | MWM_FUNC_MAXIMIZE  | MWM_FUNC_RESIZE,
                MWM_INPUT_MODELESS);
    SetWindowName("Print");

    MapWindow();
    _client->WaitFor(this);
}
    
OXPrintBox::~OXPrintBox() {
  delete L1; delete L2; delete L3;
  delete L4; delete L5; delete L6;
  delete L7; delete L21;
}
                          
int OXPrintBox::ProcessMessage(OMessage *msg) {
  const char *string;
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;
          
  switch(msg->action) {
    case MSG_CLICK:
        
      switch(msg->type) {
        case MSG_BUTTON:
        case MSG_RADIOBUTTON:
          switch(wmsg->id) {
            case PB_OK:
              if (ret) {
                *ret = PB_OK;
                if (_all->GetState() == BUTTON_DOWN) *ret |= PB_PRINTALL;
                if (_marked->GetState() == BUTTON_DOWN) *ret |= PB_PRINTMARKED;
              }
              string = tbprinter->GetString();
              delete[] *pprinter;
              *pprinter = new char[strlen(string)+1];
              strcpy(*pprinter, string);

              string = tbprintCommand->GetString();
              delete[] *pprintCommand;
              *pprintCommand = new char[strlen(string)+1];
              strcpy(*pprintCommand, string);
              CloseWindow();
              break;

            case PB_CANCEL:
              if (ret) *ret = PB_CANCEL;
              CloseWindow();
              break;

            case PB_PRINTALL:
              if (_marked->IsEnabled())
                _marked->SetState(BUTTON_UP);
              break;

            case PB_PRINTMARKED:
              if (_all->IsEnabled())
                _all->SetState(BUTTON_UP);
              break;
          }
          break;
       } 
       break;

    default:
      break;
  }

  return True;
}         
