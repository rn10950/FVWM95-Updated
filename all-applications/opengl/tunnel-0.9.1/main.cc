/**************************************************************************

    This is an example xclass OpenGL application.
    Copyright (C) 2000, 2001, Hector Peraza.

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
#include <unistd.h>
#include <limits.h>

#include <xclass/OXMsgBox.h>
#include <xclass/OXAboutDialog.h>
#include <xclass/version.h>

#include "main.h"
#include "tunnel.h"

#include "menudef.h"


//----------------------------------------------------------------------

int main(int argc, char **argv) {
  OXClient *clientX = new OXClient(argc, argv);

  MainFrame *mainw = new MainFrame(clientX->GetRoot(), 10, 10);
  mainw->Resize(600, 400);

  mainw->MapWindow();

  clientX->Run();

  exit(0);
}

MainFrame::MainFrame(const OXWindow *p, int w, int h) :
  OXMainFrame(p, w, h) {

  _menuBarLayout = new OLayoutHints(LHINTS_TOP | LHINTS_LEFT | LHINTS_EXPAND_X, 
                                    0, 0, 1, 1);
  _menuBarItemLayout = new OLayoutHints(LHINTS_TOP | LHINTS_LEFT,
                                    0, 4, 0, 0);

  _menuFile = _MakePopup(&file_popup);
  _menuView = _MakePopup(&view_popup);
  _menuHelp = _MakePopup(&help_popup);

  _menuFile->Associate(this);
  _menuView->Associate(this);
  _menuHelp->Associate(this);

  //------ menu bar

  _menuBar = new OXMenuBar(this, 1, 1, HORIZONTAL_FRAME);
  _menuBar->AddPopup(new OHotString("&File"), _menuFile, _menuBarItemLayout);
  _menuBar->AddPopup(new OHotString("&View"), _menuView, _menuBarItemLayout);
  _menuBar->AddPopup(new OHotString("&Help"), _menuHelp, _menuBarItemLayout);

  AddFrame(_menuBar, _menuBarLayout);

  //------ GL frame

  OXCompositeFrame *glf = new OXCompositeFrame(this, 10, 10,
                                               SUNKEN_FRAME | DOUBLE_BORDER);
  AddFrame(glf, new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y));

  _tunnel = new Tunnel(glf);
  glf->AddFrame(_tunnel, new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y));

  SetWindowName("Tunnel");
  SetClassHints("XCLASS", "XCLASS");

  UpdateMenu();
  SetFocusOwner(_tunnel);

  MapSubwindows();
}

MainFrame::~MainFrame() {
  delete _menuBarLayout;
  delete _menuBarItemLayout;

  delete _menuFile;
  delete _menuView;
  delete _menuHelp;
}

OXPopupMenu *MainFrame::_MakePopup(struct _popup *p) {

  OXPopupMenu *popup = new OXPopupMenu(_client->GetRoot());

  for (int i=0; p->popup[i].name != NULL; ++i) {
    if (strlen(p->popup[i].name) == 0) {
      popup->AddSeparator();
    } else {
      if (p->popup[i].popup_ref == NULL) {
        popup->AddEntry(new OHotString(p->popup[i].name), p->popup[i].id);
      } else {
        struct _popup *p1 = p->popup[i].popup_ref;
        popup->AddPopup(new OHotString(p->popup[i].name), p1->ptr);
      }
      if (p->popup[i].state & MENU_DISABLED) popup->DisableEntry(p->popup[i].id);
      if (p->popup[i].state & MENU_CHECKED) popup->CheckEntry(p->popup[i].id);
      if (p->popup[i].state & MENU_RCHECKED) popup->RCheckEntry(p->popup[i].id,
                                                                p->popup[i].id,
                                                                p->popup[i].id);
    }
  }
  p->ptr = popup;

  return popup;
}


int MainFrame::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;

  switch (msg->type) {

    case MSG_MENU:
    case MSG_BUTTON:
      switch (msg->action) {

        case MSG_CLICK:
          switch (wmsg->id) {

            //--------------------------------------- File

            case M_FILE_EXIT:
              CloseWindow();
              break;

            //--------------------------------------- View

            case M_VIEW_FOG:
              _tunnel->fog = !_tunnel->fog;
              UpdateMenu();
              break;

            case M_VIEW_TEXTURE:
              _tunnel->usetex = !_tunnel->usetex;
              UpdateMenu();
              break;

            //--------------------------------------- Help

            case M_HELP_KEYS:
              _tunnel->help = !_tunnel->help;
              UpdateMenu();
              break;

            case M_HELP_ABOUT:
              DoAbout();
              break;

            default:
              break;

          } // switch (wmsg->id)
          break;

        default:
          break;

      } // switch (msg->action)
      break;

    default:
      break;

  } // switch (msg->type)

  return True;
}

//----------------------------------------------------------------------

void MainFrame::UpdateMenu() {
  _menuView->CheckEntry(M_VIEW_FOG, _tunnel->fog);
  _menuView->CheckEntry(M_VIEW_TEXTURE, _tunnel->usetex);
  _menuHelp->CheckEntry(M_HELP_KEYS, _tunnel->help);
}

void MainFrame::DoAbout() {
  OAboutInfo info;

  info.wname = "About OpenGL Test Application";
  info.title = "OpenGL Test Application\n"
               "Compiled with xclass version "XCLASS_VERSION;
  info.copyright = "Adapted from David Bucciarell code by Hector Peraza";
  info.text = "This program is free software; you can redistribute it "
              "and/or modify it under the terms of the GNU "
              "General Public License.\n\n"
              "http://xclass.sourceforge.net";

  new OXAboutDialog(_client->GetRoot(), this, &info);
}
