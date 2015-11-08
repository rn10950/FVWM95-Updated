/**************************************************************************

    This file is part of xcpaint, a pixmap editor for xclass
    Copyright (C) 1997, Hector Peraza.

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


//---------------------------------------------------------------------

OXSizeDialog::OXSizeDialog(const OXWindow *p, const OXWindow *main,
                           int w, int h, unsigned long options,
                           int dlg_type, int *retw, int *reth) :
  OXTransientFrame(p, main, w, h, options) {

    _dlgType = dlg_type;

    ret_width  = retw;
    ret_height = reth;

    ChangeOptions((GetOptions() & ~VERTICAL_FRAME) | HORIZONTAL_FRAME);
  
    _f1 = new OXCompositeFrame(this, 60, 20, VERTICAL_FRAME | FIXED_WIDTH);
    _f2 = new OXCompositeFrame(this, 60, 20, HORIZONTAL_FRAME);

    _f2->SetLayoutManager(new O2ColumnsLayout(_f2, 5, 5));

    _okButton = new OXTextButton(_f1, new OHotString("&OK"), ID_OK);
    _cancelButton = new OXTextButton(_f1, new OHotString("&Cancel"), ID_CANCEL);

    _f1->Resize(_okButton->GetDefaultWidth()+40, GetDefaultHeight());
              
    _okButton->Associate(this);
    _cancelButton->Associate(this);

    SetDefaultAcceptButton(_okButton);
    SetDefaultCancelButton(_cancelButton);
              
    _l1 = new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 2, 2, 0, 3);
    _l2 = new OLayoutHints(LHINTS_TOP | LHINTS_RIGHT, 5, 5, 15, 10);
    _l3 = new OLayoutHints(LHINTS_TOP | LHINTS_LEFT, 10, 10, 15, 15);

    _f1->AddFrame(_okButton, _l1);
    _f1->AddFrame(_cancelButton, _l1);
    AddFrame(_f1, _l2);

    _widthLabel  = new OXLabel(_f2, new OHotString("&Width:"));
    _heightLabel = new OXLabel(_f2, new OHotString("&Height:"));

    _widthTE = new OXTextEntry(_f2, new OTextBuffer(10));
    _widthTE->Associate(this);
    _widthTE->Resize(60, _widthTE->GetDefaultHeight());

    _heightTE = new OXTextEntry(_f2, new OTextBuffer(10));
    _heightTE->Associate(this);
    _heightTE->Resize(60, _heightTE->GetDefaultHeight());

    if (ret_width) {
      char tmp[20];
      sprintf(tmp, "%d", *ret_width);
      _widthTE->AddText(0, tmp);
    }
    if (ret_height) {
      char tmp[20];
      sprintf(tmp, "%d", *ret_height);
      _heightTE->AddText(0, tmp);
    }

    _f2->AddFrame(_widthLabel, NULL);
    _f2->AddFrame(_widthTE, NULL);
    _f2->AddFrame(_heightLabel, NULL);
    _f2->AddFrame(_heightTE, NULL);
    AddFrame(_f2, _l3);

    SetFocusOwner(_widthTE);

    SetMWMHints(MWM_DECOR_ALL | MWM_DECOR_MAXIMIZE | MWM_DECOR_MENU,
              MWM_FUNC_ALL | MWM_FUNC_MAXIMIZE | MWM_FUNC_RESIZE,
              MWM_INPUT_MODELESS);
    MapSubwindows();
    Resize(GetDefaultSize());
    
    CenterOnParent();

    switch (_dlgType) {
      case DIALOG_RESIZE:    SetWindowName("Resize image to..."); break;
      case DIALOG_RESCALE:   SetWindowName("Rescale image to..."); break;
      case DIALOG_NEW_IMAGE: SetWindowName("Create new image..."); break;
    }

    MapWindow();
    _client->WaitFor(this);
}
    
OXSizeDialog::~OXSizeDialog() {
  delete _l1;
  delete _l2;
  delete _l3;
}

int OXSizeDialog::CloseWindow() {
  if (ret_width) *ret_width = -1;
  if (ret_height) *ret_height = -1;
  return OXTransientFrame::CloseWindow();
}

int OXSizeDialog::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;
  const char *str;

  switch(msg->action) {
    case MSG_CLICK:
        
      switch(msg->type) {
        case MSG_BUTTON:
          switch(wmsg->id) {
            case ID_OK:
              str = _widthTE->GetString();
              if (ret_width) *ret_width = atoi(str);
              str = _heightTE->GetString();
              if (ret_height) *ret_height = atoi(str);
              delete this;
              break;

            case ID_CANCEL:
              if (ret_width) *ret_width = -1;
              if (ret_height) *ret_height = -1;
              delete this;
              break;
          }
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

//---------------------------------------------------------------------

OXTextDialog::OXTextDialog(const OXWindow *p, const OXWindow *main,
                           char *rtxt, unsigned long options) :
  OXTransientFrame(p, main, 10, 10, options) {

    ret_text = rtxt;

    ChangeOptions((GetOptions() & ~VERTICAL_FRAME) | HORIZONTAL_FRAME);
  
    _f1 = new OXCompositeFrame(this, 60, 20, VERTICAL_FRAME | FIXED_WIDTH);
    _f2 = new OXCompositeFrame(this, 60, 20, HORIZONTAL_FRAME);

    _okButton = new OXTextButton(_f1, new OHotString("&OK"), ID_OK);
    _cancelButton = new OXTextButton(_f1, new OHotString("&Cancel"), ID_CANCEL);

    _f1->Resize(_okButton->GetDefaultWidth()+40, GetDefaultHeight());
              
    _okButton->Associate(this);
    _cancelButton->Associate(this);
              
    SetDefaultAcceptButton(_okButton);
    SetDefaultCancelButton(_cancelButton);
              
    _l1 = new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 2, 2, 0, 3);
    _l2 = new OLayoutHints(LHINTS_TOP | LHINTS_RIGHT, 5, 5, 15, 10);
    _l3 = new OLayoutHints(LHINTS_TOP | LHINTS_LEFT, 10, 10, 15, 15);

    _f1->AddFrame(_okButton, _l1);
    _f1->AddFrame(_cancelButton, _l1);
    AddFrame(_f1, _l2);

    _textLabel  = new OXLabel(_f2, new OHotString("&Text:"));

    _textTE = new OXTextEntry(_f2, new OTextBuffer(200));
    _textTE->Associate(this);
    _textTE->Resize(200, _textTE->GetDefaultHeight());

    if (ret_text) _textTE->AddText(0, ret_text);

    _f2->AddFrame(_textLabel, new OLayoutHints(LHINTS_LEFT | LHINTS_CENTER_Y,
                                               0, 5, 0, 0));
    _f2->AddFrame(_textTE, new OLayoutHints(LHINTS_LEFT | LHINTS_CENTER_Y));
    AddFrame(_f2, _l3);

    SetFocusOwner(_textTE);

    SetMWMHints(MWM_DECOR_ALL | MWM_DECOR_MAXIMIZE | MWM_DECOR_MENU,
              MWM_FUNC_ALL | MWM_FUNC_MAXIMIZE | MWM_FUNC_RESIZE,
              MWM_INPUT_MODELESS);

    MapSubwindows();
    Resize(GetDefaultSize());
    
    CenterOnParent();

    SetWindowName("Enter text...");

    MapWindow();
    _client->WaitFor(this);
}

OXTextDialog::~OXTextDialog() {
  delete _l1;
  delete _l2;
  delete _l3;
}

int OXTextDialog::CloseWindow() {
  if (ret_text) *ret_text = '\0';
  return OXTransientFrame::CloseWindow();
}

int OXTextDialog::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;
  const char *str;
          
  switch(msg->action) {
    case MSG_CLICK:
        
      switch(msg->type) {
        case MSG_BUTTON:
          switch(wmsg->id) {
            case ID_OK:
              str = _textTE->GetString();
              if (ret_text) strcpy(ret_text, str);
              delete this;
              break;

            case ID_CANCEL:
              if (ret_text) *ret_text = '\0';
              delete this;
              break;
          }
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
