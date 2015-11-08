/**************************************************************************

    This file is part of notepad, a simple text editor.
    Copyright (C) 1997-2001, Harald Radke, Hector Peraza.

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

#include "OXDialogs.h"

#include "printer-icon.xpm"


//----------------------------------------------------------------------

OXGotoBox::OXGotoBox(const OXWindow *p, const OXWindow *main,
                     int w, int h, long *ret_val, unsigned long options) :
  OXTransientFrame(p, main, w, h, options) {

    ret = ret_val;

    ChangeOptions((GetOptions() & ~VERTICAL_FRAME) | HORIZONTAL_FRAME);
  
    f1 = new OXCompositeFrame(this, 60, 20, VERTICAL_FRAME | FIXED_WIDTH);
    f2 = new OXCompositeFrame(this, 60, 20, HORIZONTAL_FRAME);

    GotoButton = new OXTextButton(f1, new OHotString("&Goto"), 1);
    CancelButton = new OXTextButton(f1, new OHotString("&Cancel"), 2);
    GotoButton->Disable();      
    f1->Resize(GotoButton->GetDefaultWidth()+40, GetDefaultHeight());
              
    GotoButton->Associate(this);
    CancelButton->Associate(this);

    L1 = new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 2, 2, 0, 3);
    L21 = new OLayoutHints(LHINTS_TOP | LHINTS_RIGHT, 2, 5, 3, 0);

    f1->AddFrame(GotoButton, L1);
    f1->AddFrame(CancelButton, L1);
    AddFrame(f1, L21);

    OXLabel *lgoTo = new OXLabel(f2, new OHotString("&Goto Line:"));
    goTo = new OXTextEntry(f2, tbgoTo = new OTextBuffer(50));
    goTo->Associate(this);          
    goTo->Resize(150, goTo->GetDefaultHeight());
    
    L5 = new OLayoutHints(LHINTS_LEFT | LHINTS_CENTER_Y, 3, 5, 0, 0);
    L6 = new OLayoutHints(LHINTS_LEFT | LHINTS_CENTER_Y, 0, 2, 0, 0);
     
    f2->AddFrame(lgoTo, L5);   
    f2->AddFrame(goTo, L5);   
    AddFrame(f2, L21);

    SetDefaultAcceptButton(GotoButton);
    SetDefaultCancelButton(CancelButton);
    SetFocusOwner(goTo);
              
    SetMWMHints(MWM_DECOR_ALL | MWM_DECOR_MAXIMIZE | MWM_DECOR_MENU,
                MWM_FUNC_ALL | MWM_FUNC_MAXIMIZE | MWM_FUNC_RESIZE,
                MWM_INPUT_MODELESS);

    MapSubwindows();
    Resize(GetDefaultSize());

    CenterOnParent();

    SetWMSize(_w, _h);
    SetWMSizeHints(_w, _h, _w, _h, 0, 0);
    
    SetWindowName("Goto");
     
    MapWindow();
    _client->WaitFor(this);
}
    
OXGotoBox::~OXGotoBox() {
  delete L1; delete L5; delete L6;
  delete L21;
}
                          
int OXGotoBox::ProcessMessage(OMessage *msg) {
 OWidgetMessage *wmsg = (OWidgetMessage *) msg;
          
  switch (msg->type) {
    case MSG_BUTTON:
        
      switch (msg->action) {
        case MSG_CLICK:
          switch (wmsg->id) {
            case 1:
              *ret = (long) atof(tbgoTo->GetString());
              CloseWindow();
              break;

            case 2:
              *ret = -1;
              CloseWindow();
              break;
          }
          break;

        default:
          break;
      }
      break;  
    
    case MSG_TEXTENTRY:
      switch (msg->action) {
        case MSG_TEXTCHANGED:
          if (strlen(tbgoTo->GetString()) == 0)
            GotoButton->Disable();
          else
            GotoButton->Enable();
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


//----------------------------------------------------------------------

OXSetTabsBox::OXSetTabsBox(const OXWindow *p, const OXWindow *main,
                           int w, int h, int *ret_val,
                           unsigned long options) :
  OXTransientFrame(p, main, w, h, options) {

    ret = ret_val;

    ChangeOptions((GetOptions() & ~VERTICAL_FRAME) | HORIZONTAL_FRAME);
  
    f1 = new OXCompositeFrame(this, 60, 20, VERTICAL_FRAME | FIXED_WIDTH);
    f2 = new OXCompositeFrame(this, 60, 20, HORIZONTAL_FRAME);

    OkButton = new OXTextButton(f1, new OHotString("&OK"), 1);
    CancelButton = new OXTextButton(f1, new OHotString("&Cancel"), 2);

    f1->Resize(CancelButton->GetDefaultWidth()+20, GetDefaultHeight());

    OkButton->Associate(this);
    CancelButton->Associate(this);

    L1 = new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 2, 2, 0, 3);
    L21 = new OLayoutHints(LHINTS_TOP | LHINTS_RIGHT, 2, 5, 3, 0);
    
    f1->AddFrame(OkButton, L1);
    f1->AddFrame(CancelButton, L1);
    AddFrame(f1, L21);
              
    OXLabel *label = new OXLabel(f2, new OHotString("&Tab Width:"));
    _te = new OXTextEntry(f2, _tb = new OTextBuffer(5));
    _te->Associate(this);
    _te->Resize(150, _te->GetDefaultHeight());

    char tmp[20];
    sprintf(tmp, "%d", *ret);
    _te->AddText(0, tmp);
    
    L5 = new OLayoutHints(LHINTS_LEFT | LHINTS_CENTER_Y, 3, 5, 0, 0);
    L6 = new OLayoutHints(LHINTS_LEFT | LHINTS_CENTER_Y, 0, 2, 0, 0);
     
    f2->AddFrame(label, L5);
    f2->AddFrame(_te, L5);
    AddFrame(f2, L21);

    SetDefaultAcceptButton(OkButton);
    SetDefaultCancelButton(CancelButton);
    SetFocusOwner(_te);
              
    SetMWMHints(MWM_DECOR_ALL | MWM_DECOR_MAXIMIZE | MWM_DECOR_MENU,
                MWM_FUNC_ALL | MWM_FUNC_MAXIMIZE | MWM_FUNC_RESIZE,
                MWM_INPUT_MODELESS);

    MapSubwindows();
    Resize(GetDefaultSize());

    CenterOnParent();
    
    SetWMSize(_w, _h);
    SetWMSizeHints(_w, _h, _w, _h, 0, 0);
    
    SetWindowName("Set Tab Width");
     
    MapWindow();
    _client->WaitFor(this);
}
    
OXSetTabsBox::~OXSetTabsBox() {
  delete L1; delete L5; delete L6;
  delete L21;
}

int OXSetTabsBox::ProcessMessage(OMessage *msg) {
 OWidgetMessage *wmsg = (OWidgetMessage *) msg;

  switch (msg->type) {
    case MSG_BUTTON:
        
      switch (msg->action) {
        case MSG_CLICK:
          switch (wmsg->id) {
            case 1:
              *ret = atoi(_te->GetString());
              CloseWindow();
              break;

            case 2:
              *ret = -1;
              CloseWindow();
              break;
          }
          break;

        default:
          break;
      }
      break;  
    
    case MSG_TEXTENTRY:
      switch (msg->action) {
        case MSG_TEXTCHANGED:
          if (strlen(_te->GetString()) == 0)
            OkButton->Disable();
          else
            OkButton->Enable();
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


//----------------------------------------------------------------------

OXSearchBox::OXSearchBox(const OXWindow *p, const OXWindow *main,
                         int w, int h, search_struct *sstruct, int *ret_val, 
                         unsigned long options) :
  OXTransientFrame(p, main, w, h, options) {

    ret = ret_val;
    s = sstruct;

    ChangeOptions((GetOptions() & ~VERTICAL_FRAME) | HORIZONTAL_FRAME);
  
    f1 = new OXCompositeFrame(this, 60, 20, VERTICAL_FRAME | FIXED_WIDTH);
    f2 = new OXCompositeFrame(this, 60, 20, VERTICAL_FRAME);
    f3 = new OXCompositeFrame(f2, 60, 20, HORIZONTAL_FRAME);
    f4 = new OXCompositeFrame(f2, 60, 20, HORIZONTAL_FRAME);

    SearchButton = new OXTextButton(f1, new OHotString("&Find Next"), 1);
    CancelButton = new OXTextButton(f1, new OHotString("Cancel"), 2);
    SearchButton->Disable();      
    f1->Resize(SearchButton->GetDefaultWidth()+20, GetDefaultHeight());
              
    SearchButton->Associate(this);
    CancelButton->Associate(this);

    SetDefaultAcceptButton(SearchButton);
    SetDefaultCancelButton(CancelButton);
              
    L1 = new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 2, 2, 0, 3);
    L2 = new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 2, 5, 5, 0);
    L21 = new OLayoutHints(LHINTS_TOP | LHINTS_RIGHT, 2, 5, 5, 0);
    
    f1->AddFrame(SearchButton, L1);
    f1->AddFrame(CancelButton, L1);
    AddFrame(f1, L21);
              
    lsearch = new OXLabel(f3, new OHotString("Fi&nd what:"));
    search = new OXTextEntry(f3, tbsearch = new OTextBuffer(100));
    search->Associate(this);        
    search->Resize(200, search->GetDefaultHeight());
    
    L5 = new OLayoutHints(LHINTS_LEFT | LHINTS_CENTER_Y, 3, 5, 0, 0);
    L6 = new OLayoutHints(LHINTS_LEFT | LHINTS_CENTER_Y);
 
    f3->AddFrame(lsearch, L5);
    f3->AddFrame(search, L6);

    g2 = new OXGroupFrame(f4, new OString("Direction"), HORIZONTAL_FRAME);
    
    L3 = new OLayoutHints(LHINTS_TOP | LHINTS_RIGHT);
    L9 = new OLayoutHints(LHINTS_BOTTOM | LHINTS_LEFT, 3, 5, 0, 0);
    L4 = new OLayoutHints(LHINTS_BOTTOM | LHINTS_LEFT, 0, 5, 7, 0);
    L10 = new OLayoutHints(LHINTS_BOTTOM | LHINTS_RIGHT, 0, 0, 7, 0);

    case_check = new OXCheckButton(f4, new OHotString("Match &case"), 1);
    case_check->Associate(this);
    f4->AddFrame(case_check, L9);
     
    direction_radio[0] = new OXRadioButton(g2, new OHotString("&Up"), 1);
    direction_radio[1] = new OXRadioButton(g2, new OHotString("&Down"), 2);
    
    g2->AddFrame(direction_radio[0], L4);
    g2->AddFrame(direction_radio[1], L10);
    direction_radio[0]->Associate(this); 
    direction_radio[1]->Associate(this); 

    if (s->caseSensitive == False)
      case_check->SetState(BUTTON_UP);
    else 
      case_check->SetState(BUTTON_DOWN);

    if (s->direction)
      direction_radio[1]->SetState(BUTTON_DOWN);
    else
      direction_radio[0]->SetState(BUTTON_DOWN);

    f4->AddFrame(g2, L3);
    
    f2->AddFrame(f3, L1);
    f2->AddFrame(f4, L1);

    AddFrame(f2, L2);

    SetDefaultAcceptButton(SearchButton);
    SetDefaultCancelButton(CancelButton);
    SetFocusOwner(search);
              
    SetMWMHints(MWM_DECOR_ALL | MWM_DECOR_MAXIMIZE | MWM_DECOR_MENU,
                MWM_FUNC_ALL | MWM_FUNC_MAXIMIZE | MWM_FUNC_RESIZE,
                MWM_INPUT_MODELESS);
    MapSubwindows();
    Resize(GetDefaultSize());

    CenterOnParent();

    SetWMSize(_w, _h);
    SetWMSizeHints(_w, _h, _w, _h, 0, 0);
    
    SetWindowName("Find");
     
    MapWindow();
    _client->WaitFor(this);
}
    
OXSearchBox::~OXSearchBox() {
  delete L1; delete L2; delete L3; delete L4; delete L5; delete L6;
  delete L21; delete L9; delete L10;
}
                          
int OXSearchBox::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;

  switch (msg->type) {
    case MSG_BUTTON:
        
      switch (msg->action) {
        case MSG_CLICK:
          switch (wmsg->id) {
            case 1:
              s->buffer = StrDup(tbsearch->GetString());
              *ret = True;
              CloseWindow();
              break;

            case 2:
              *ret = False;
              CloseWindow();
              break;
          }
        break;
      }
      break;

    case MSG_CHECKBUTTON:
      if (msg->action == MSG_CLICK)
        s->caseSensitive = !s->caseSensitive;
      break;           

    case MSG_RADIOBUTTON:
      if (msg->action == MSG_CLICK) {
        switch (wmsg->id) {
          case 1:
            s->direction = False;   
            direction_radio[1]->SetState(BUTTON_UP);
            break;

          case 2:
            s->direction = True;
            direction_radio[0]->SetState(BUTTON_UP);
            break;
        }
      }
      break;

    case MSG_TEXTENTRY:
      switch (msg->action) {
        case MSG_TEXTCHANGED:
          if (strlen(tbsearch->GetString()) == 0)
            SearchButton->Disable();
          else
            SearchButton->Enable();
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


//----------------------------------------------------------------------

OXPrintBox::OXPrintBox(const OXWindow *p, const OXWindow *main, int w, int h, 
                       char **printerName, char **printProg, int *ret_val, 
                       unsigned long options) :
  OXTransientFrame(p, main, w, h, options) {
              
    pprinter = printerName;
    pprintCommand = printProg;
    ret = ret_val;

    ChangeOptions((GetOptions() & ~VERTICAL_FRAME) | HORIZONTAL_FRAME);
  
    f1 = new OXCompositeFrame(this, 60, 20, VERTICAL_FRAME | FIXED_WIDTH);
    f5 = new OXCompositeFrame(this, 60,20, HORIZONTAL_FRAME);
    f2 = new OXCompositeFrame(this, 60, 20, VERTICAL_FRAME);
    f3 = new OXCompositeFrame(f2, 60, 20, HORIZONTAL_FRAME);
    f4 = new OXCompositeFrame(f2, 60, 20, HORIZONTAL_FRAME);

    PrintButton = new OXTextButton(f1, new OHotString("&Print"), 1);
    CancelButton = new OXTextButton(f1, new OHotString("&Cancel"), 2);
    f1->Resize(PrintButton->GetDefaultWidth()+40, GetDefaultHeight());
              
    PrintButton->Associate(this);
    CancelButton->Associate(this);
              
    L1 = new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 2, 2, 2, 0);
    L2 = new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 2, 5, 0, 2);
    L21 = new OLayoutHints(LHINTS_TOP | LHINTS_RIGHT, 2, 5, 10, 0);
    
    f1->AddFrame(PrintButton, L1);
    f1->AddFrame(CancelButton, L1);
    AddFrame(f1, L21);
              
    lprintCommand = new OXLabel(f3, new OHotString("Print &command:"));
    tbprintCommand = new OTextBuffer(20);
    tbprintCommand->AddText(0,*printProg);
    printCommand = new OXTextEntry(f3, tbprintCommand);
    printCommand->Associate(this);          
    printCommand->Resize(100,printCommand->GetDefaultHeight());

    L5 = new OLayoutHints(LHINTS_LEFT | LHINTS_CENTER_Y, 3, 5, 0, 0);
    L6 = new OLayoutHints(LHINTS_RIGHT | LHINTS_CENTER_Y, 0, 2, 0, 0);
 
    f3->AddFrame(lprintCommand, L5);
    f3->AddFrame(printCommand, L6);

    lprinter   = new OXLabel(f4, new OHotString("&Printer:"));
    tbprinter = new OTextBuffer(20);
    tbprinter->AddText(0,*printerName);
    printer= new OXTextEntry(f4, tbprinter); 
    printer->Associate(this);
    printer->Resize(100, printer->GetDefaultHeight());
    f4->AddFrame(lprinter, L5);
    f4->AddFrame(printer, L6);

    L3 = new OLayoutHints(LHINTS_TOP | LHINTS_RIGHT, 2, 2, 2, 2);
    
    f2->AddFrame(f3, L1);
    f2->AddFrame(f4, L1);
 
    printerPicture = _client->GetPicture("printer.xpm", printerIconData);
    printerIcon = new OXIcon(f5, printerPicture, 32,32);
    f5->AddFrame(printerIcon, new OLayoutHints(LHINTS_LEFT, 10, 10, 10, 10));
    f5->AddFrame(f2,L21);
    AddFrame(f5, L21);
    
    MapSubwindows();
    Resize(GetDefaultSize());
    
    CenterOnParent();

    SetMWMHints(MWM_DECOR_ALL | MWM_DECOR_MAXIMIZE | MWM_DECOR_MENU,
                MWM_FUNC_ALL | MWM_FUNC_MAXIMIZE | MWM_FUNC_RESIZE,
                MWM_INPUT_MODELESS);

    SetWindowName("Print");
     
    MapWindow();
    _client->WaitFor(this);
}
    
OXPrintBox::~OXPrintBox() {
  delete L1; delete L2; delete L3; delete L5; delete L6;
  delete L21;
}

int OXPrintBox::ProcessMessage(OMessage *msg) { 
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;

  switch (msg->type) {
    case MSG_BUTTON:
        
      switch (msg->action) {
        case MSG_CLICK:
          switch (wmsg->id) {
            case 1:
              *ret = True;
              delete[] *pprinter;
              *pprinter = StrDup(tbprinter->GetString());
              delete[] *pprintCommand;
              *pprintCommand = StrDup(tbprintCommand->GetString());
              CloseWindow();
              break;

            case 2:
              *ret = False;
              CloseWindow();
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
