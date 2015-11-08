/**************************************************************************

    This file is part of xclass.
    Copyright (C) 2000, Harald Radke.

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
#include <string.h>

#include <xclass/utils.h>
#include <xclass/OXMsgBox.h>
#include <xclass/OXMainFrame.h>
#include <xclass/OXPictureButton.h>
#include <xclass/OXListView.h>

#include "back.xpm"


//----------------------------------------------------------------------

class OXTestFrame : public OXMainFrame {
public:
  OXTestFrame(const OXWindow *p, int w, int h);
  virtual int ProcessMessage(OMessage *msg);

protected:
  OXListView *listview;
  OXCompositeFrame *bframe;
  OXPictureButton *bbig, *bsmall, *blist, *bdetails;
  const OPicture *bpic, *spic;
};

//----------------------------------------------------------------------

OXClient *clientX;

int main(int argc, char **argv) {

  clientX = new OXClient;

  OXTestFrame *mainWindow = new OXTestFrame(clientX->GetRoot(), 10, 10);

  mainWindow->MapSubwindows();
  mainWindow->Resize(480, 300);
  mainWindow->Layout();
  mainWindow->MapWindow();

  clientX->Run();
}

OXTestFrame::OXTestFrame(const OXWindow *p, int w, int h) :
  OXMainFrame(p, w, h) {

  OListViewItem *item;
  std::vector<OString *> names;
  char string[14];

  bpic = _client->GetPicture("app.s.xpm");
  spic = _client->GetPicture("app.t.xpm");

  OLayoutHints *layout = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y,
                                          0, 0, 3, 0);

  bframe = new OXCompositeFrame(this, 10, 10, HORIZONTAL_FRAME);
  AddFrame(bframe, new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 0, 0, 3, 0));

  const OPicture *pbig = _client->GetPicture("tb-bigicons.xpm");
  const OPicture *psmall = _client->GetPicture("tb-smicons.xpm");
  const OPicture *plist = _client->GetPicture("tb-list.xpm");
  const OPicture *pdetails = _client->GetPicture("tb-details.xpm");

  bbig = new OXPictureButton(bframe, pbig, 1);
  bsmall = new OXPictureButton(bframe, psmall, 2);
  blist = new OXPictureButton(bframe, plist, 3);
  bdetails = new OXPictureButton(bframe, pdetails, 4);

  bbig->Associate(this);
  bsmall->Associate(this);
  blist->Associate(this);
  bdetails->Associate(this);

  bbig->SetType(BUTTON_STAYDOWN);
  bbig->SetState(BUTTON_ENGAGED);
  bsmall->SetType(BUTTON_STAYDOWN);
  blist->SetType(BUTTON_STAYDOWN);
  bdetails->SetType(BUTTON_STAYDOWN);

  OLayoutHints *bhints = new OLayoutHints(LHINTS_LEFT | LHINTS_CENTER_Y);

  bframe->AddFrame(bbig, bhints);
  bframe->AddFrame(bsmall, bhints);
  bframe->AddFrame(blist, bhints);
  bframe->AddFrame(bdetails, bhints);

  listview = new OXListView(this, 10, 10, 1);
  listview->SetupBackgroundPic(_client->GetPicture("back.xpm", back_xpm));

  const OResourcePool *res = _client->GetResourcePool();
  listview->SetItemFont(res->GetFontPool()->GetFont("times -14"));
  listview->SetHeaderFont(res->GetFontPool()->GetFont("helvetica -12"));

  listview->AddColumn(new OString("Name"), 0, TEXT_LEFT);
  listview->AddColumn(new OString("Column 1"), 1);
  listview->AddColumn(new OString("Column 2"), 2);
  listview->AddColumn(new OString("Column 4"), 4, TEXT_RIGHT);
  listview->AddColumn(new OString("Column 3"), 3, TEXT_LEFT);
  listview->AddColumn(new OString("Column 5"), 5);
  listview->AddColumn(new OString("Column 2"), 2);

  AddFrame(listview, layout);
  listview->SetViewMode(LV_LARGE_ICONS);
  listview->Associate(this);

  for (unsigned int i = 0; i < 1000; i++) {
    sprintf(string, "name %d", i);
    names.push_back(new OString(string));
    sprintf(string, "one %d", i);
    names.push_back(new OString(string));
    sprintf(string, "two %d", i);
    names.push_back(new OString(string));
    sprintf(string, "three %d", i);
    names.push_back(new OString(string));
    sprintf(string, "four %5d", 1000-i);
    names.push_back(new OString(string));
    sprintf(string, "five %d", i);
    names.push_back(new OString(string));
    item = new OListViewItem(listview, i, bpic, spic, names, 
                             listview->GetViewMode());
    listview->AddItem(item);
    names.clear();
  }

}

int OXTestFrame::ProcessMessage(OMessage *msg) {
  switch (msg->type) {
  case MSG_BUTTON:
    switch (msg->action) {
    case MSG_CLICK:
      switch (((OWidgetMessage *)msg)->id) {
      case 1:
	listview->SetViewMode(LV_LARGE_ICONS);
	bsmall->SetState(BUTTON_UP);
	blist->SetState(BUTTON_UP);
	bdetails->SetState(BUTTON_UP);
	break;

      case 2:
	listview->SetViewMode(LV_SMALL_ICONS);
	bbig->SetState(BUTTON_UP);
	blist->SetState(BUTTON_UP);
	bdetails->SetState(BUTTON_UP);
	break;

      case 3:
	listview->SetViewMode(LV_LIST);
	bbig->SetState(BUTTON_UP);
	bsmall->SetState(BUTTON_UP);
	bdetails->SetState(BUTTON_UP);
	break;

      case 4:
	listview->SetViewMode(LV_DETAILS);
	bbig->SetState(BUTTON_UP);
	bsmall->SetState(BUTTON_UP);
	blist->SetState(BUTTON_UP);
	break;

      default:
	break;
      }				// case msg->id
      break;

    default:
      break;
    }				// case msg->action
    break;

  }				// case msg->type

  return True;
}
