/**************************************************************************

    This is a test program for xclass OXResizers.
    Copyright (C) 1997, Harald Radke.

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

#include <stdio.h>

#include <xclass/OXMenu.h>
#include <xclass/OXMainFrame.h>
#include <xclass/OXListBox.h>
#include <xclass/OXResizer.h>

#define MVS_FILE_SAVE             1
#define MVS_FILE_PRINT            2
#define MVS_FILE_EXIT             3

#define MVS_EDIT_COPY             4
#define MVS_EDIT_SELECTALL        5
#define MVS_EDIT_SEARCH           6
#define MVS_EDIT_SEARCHAGAIN      7

#define MVS_HELP_CONTENTS         8
#define MVS_HELP_ABOUT            9

//---------------------------------------------------------------------

class OXAppMainFrame : public OXMainFrame {
public:
  OXAppMainFrame(const OXWindow *p, int w, int h);

  void UpdateListBox();

protected:
  OXCompositeFrame * _v1;
  OXListBox *_lb1, *_lb2, *_lb3;
  OXVerticalResizer *_vr;
  OXHorizontalResizer *_hr;
};


//----------------------------------------------------------------------

OXClient *clientX;

int main(int argc, char **argv) {

  clientX = new OXClient;

  OXAppMainFrame *mainWindow = new OXAppMainFrame(clientX->GetRoot(), 10, 10);

  mainWindow->UpdateListBox();
  mainWindow->Resize(400, 400);

  clientX->Run();
}

OXAppMainFrame::OXAppMainFrame(const OXWindow *p, int w, int h) :
  OXMainFrame(p, w, h, HORIZONTAL_FRAME) {

  SetWindowName("test");
  SetClassHints("test", "test");
  _lb1 = new OXListBox(this, 500);
  AddFrame(_lb1, new OLayoutHints(LHINTS_EXPAND_Y, 0, 0, 0, 0));
  _vr = new OXVerticalResizer(this);
  AddFrame(_vr, new OLayoutHints(LHINTS_EXPAND_Y, 0, 0, 0, 0));
  _v1 = new OXCompositeFrame(this, 10, 10, VERTICAL_FRAME);
  AddFrame(_v1, new OLayoutHints(LHINTS_EXPAND_Y | LHINTS_EXPAND_X, 0, 0, 0, 0));

  _lb2 = new OXListBox(_v1, 50);
  _lb2->IntegralHeight(False);
  _v1->AddFrame(_lb2, new OLayoutHints(LHINTS_EXPAND_X, 0, 0, 0, 0));
  _hr = new OXHorizontalResizer(_v1, -1, RESIZER_OPAQUE);
  _v1->AddFrame(_hr, new OLayoutHints(LHINTS_EXPAND_X, 0, 0, 0, 0));
  _lb3 = new OXListBox(_v1, 800);
  _lb3->IntegralHeight(False);
  _v1->AddFrame(_lb3, new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y, 0, 0, 0, 0));
  _vr->SetPrev(_lb1); //, 50);
  _vr->SetNext(_v1); //, 100);
  _hr->SetPrev(_lb2); //, 20);
  _hr->SetNext(_lb3); //, 80);

  SetMWMHints(MWM_DECOR_ALL | MWM_DECOR_MAXIMIZE | MWM_DECOR_MENU,
	      MWM_FUNC_MINIMIZE | MWM_FUNC_CLOSE,
	      MWM_INPUT_MODELESS);

  Layout();
  MapSubwindows();
  MapWindow();
}


void OXAppMainFrame::UpdateListBox() {
  int i;
  char strtemp[256];

  _lb1->AutoUpdate(False);
  _lb2->AutoUpdate(False);
  _lb3->AutoUpdate(False);

  for (i = 0; i < 18; i++) {
    sprintf(strtemp, "test %d", i);
    _lb1->AddEntry(new OString(strtemp), _lb1->GetId());
    _lb2->AddEntry(new OString(strtemp), _lb2->GetId());
    _lb3->AddEntry(new OString(strtemp), _lb3->GetId());
  }

  _lb1->Update();
  _lb2->Update();
  _lb3->Update();

//   _lbf1->Associate(this);
  //   _lbf2->Associate(this);
  //   _lbf3->Associate(this);               
}
