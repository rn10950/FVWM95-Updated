/**************************************************************************

    This file is part of foxirc.
    Copyright (C) 2000-2002, Hector Peraza.

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
#include <ctype.h>
#include <errno.h>

#include <xclass/utils.h>
#include <xclass/OXGroupFrame.h>
#include <xclass/OXLabel.h>
#include <xclass/OXTextButton.h>
#include <xclass/O2ColumnsLayout.h>

#include "OIrcMessage.h"
#include "OXServerTree.h"
#include "OXIrc.h"


//----------------------------------------------------------------------

OServerLink::OServerLink(const char *srvname, const char *conn,
                         const char *msg, int hopcnt) {
  serverName = StrDup(srvname);
  connectedTo = StrDup(conn);
  serverMsg = StrDup(msg);
  hop = hopcnt;
}

OServerLink::~OServerLink() {
  delete[] serverName;
  delete[] connectedTo;
  delete[] serverMsg;
}

//----------------------------------------------------------------------

OLinkTreeItem::OLinkTreeItem(OXClient *_client, const char *name, int i) :
  OListTreeItem(_client, name,
                _client->GetPicture("ofolder.t.xpm"),
                _client->GetPicture("folder.t.xpm")) {
  id = i;
}


//----------------------------------------------------------------------

OXServerTree::OXServerTree(const OXWindow *p, const OXWindow *main,
                           OXIrc *irc, int w, int h) :
  OXTransientFrame(p, main, w, h) {

  _irc = irc;
  _links.clear();
  _clearPending = False;

  OLayoutHints *layout = new OLayoutHints(LHINTS_EXPAND_ALL, 5, 5, 5, 0);

  OXCompositeFrame *hf = new OXHorizontalFrame(this);
  AddFrame(hf, layout);

  //---- list tree

  _listTree = new OXListTree(hf, 10, 10, 1, SUNKEN_FRAME | DOUBLE_BORDER);
  _listTree->Associate(this);

  hf->AddFrame(_listTree, new OLayoutHints(LHINTS_EXPAND_ALL));

  //---- buttons

  OXCompositeFrame *vf = new OXVerticalFrame(hf, 10, 10, FIXED_WIDTH);

  OLayoutHints *bl = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_TOP, 0, 0, 0, 5);

  _conn = new OXTextButton(vf, new OHotString("&Connect"), 101);
  _conn->Associate(this);
  vf->AddFrame(_conn, bl);

  _cl = new OXTextButton(vf, new OHotString("C&lose"), 102);
  _cl->Associate(this);
  vf->AddFrame(_cl, bl);

  vf->Resize(max(_cl->GetDefaultWidth(), _conn->GetDefaultWidth()) * 4 / 3,
             vf->GetDefaultHeight());

  hf->AddFrame(vf, new OLayoutHints(LHINTS_TOP | LHINTS_RIGHT, 5, 0, 0, 0));

  //---- description labels

  OXGroupFrame *gf = new OXGroupFrame(this, new OString("Description"));
  gf->OldInsets(False);

  OXCompositeFrame *cf = new OXVerticalFrame(gf);
  gf->AddFrame(cf, new OLayoutHints(LHINTS_EXPAND_ALL, 10, 5, 5, 5));

  cf->SetLayoutManager(new O2ColumnsLayout(cf, 5, 2));

  _servName = new OXLabel(cf, new OString(""));
  _connTo = new OXLabel(cf, new OString(""));
  _servMsg = new OXLabel(cf, new OString(""));

  cf->AddFrame(new OXLabel(cf, new OString("Server Name:")), NULL);
  cf->AddFrame(_servName, NULL);
  cf->AddFrame(new OXLabel(cf, new OString("Connected To:")), NULL);
  cf->AddFrame(_connTo, NULL);
  cf->AddFrame(new OXLabel(cf, new OString("Message:")), NULL);
  cf->AddFrame(_servMsg, NULL);

  AddFrame(gf, new OLayoutHints(LHINTS_EXPAND_X | LHINTS_BOTTOM, 5, 5, 5, 5));

  SetWindowName("Server Links");
  SetClassHints("fOXIrc", "dialog");

  SetDefaultAcceptButton(_conn);
  SetDefaultCancelButton(_cl);

  _conn->Disable();

  MapSubwindows();
  Resize(460, 300);
  Layout();
  MapWindow();
}

OXServerTree::~OXServerTree() {
  Clear();
}

int OXServerTree::CloseWindow() {
  _irc->ServerTreeClosed();
  return OXTransientFrame::CloseWindow();
}

int OXServerTree::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;
  OItemViewMessage *vmsg = (OItemViewMessage *) msg;
  OLinkTreeItem *item;

  switch (msg->type) {
    case MSG_BUTTON:
      switch (msg->action) {

        case MSG_CLICK:
          switch (wmsg->id) {
            case 101:
              item = (OLinkTreeItem *) _listTree->GetSelected();
              if (item) {
                _irc->Connect(_links[item->id]->serverName);
                _conn->Enable();
              }
              break;

            case 102:
              CloseWindow();
              break;

            default:
              break;

          }

        default:
          break;

      }
      break;

    case MSG_LISTTREE:
      switch (msg->action) {
        case MSG_CLICK:
          item = (OLinkTreeItem *) _listTree->GetSelected();
          if (item) {
            _UpdateLabels(_links[item->id]);
            _conn->Enable();
          } else {
            _conn->Disable();
          }
          break;

        case MSG_SELCHANGED:
          // enable/disable connect buttons...
          break;
      }
      break;

    default:
      break;

  }

  return True;
}

//----------------------------------------------------------------------

void OXServerTree::Clear() {
  OListTreeItem *next, *item = _listTree->GetFirstItem();
  while (item) {
    next = item->nextsibling;
    _listTree->DeleteChildren(item);
    _listTree->DeleteItem(item);
    item = next;
  }
  _UpdateLabels(NULL);
  for (int i = 0; i < _links.size(); ++i) delete _links[i];
  _links.clear();
  _clearPending = False;
}

void OXServerTree::AddLink(OServerLink *link) {
  if (_clearPending) Clear();
  _links.push_back(link);
}

void OXServerTree::BuildTree() {
  int i, hopcnt, found, total;
  OListTreeItem *item;
  OLinkTreeItem *e;

  total = 0;
  hopcnt = 0;
  found = True;

  while (found) {
    found = False;
    for (i = 0; i < _links.size(); ++i) {
      if (_links[i]->hop == hopcnt) {
        found = True;
        ++total;
        if (hopcnt == 0) {   // this is a root server
          e = new OLinkTreeItem(_client, _links[i]->serverName, i);
          _listTree->AddItem(NULL, e);
          _listTree->OpenNode(e);
        } else {
          item = _FindServer(NULL, _links[i]->connectedTo, hopcnt - 1);
          if (item) {
            e = new OLinkTreeItem(_client, _links[i]->serverName, i);
            _listTree->AddItem(item, e);
            _listTree->OpenNode(e);
          }
        }
      }
    }
    ++hopcnt;
  }

  //printf("size = %d, total = %d\n", _links.size(), total);

  _listTree->Layout();
  _clearPending = True;
}

OListTreeItem *OXServerTree::_FindServer(OListTreeItem *root,
                                         const char *name, int hopcnt) {
  OListTreeItem *item, *ret;

  if (!root) root = _listTree->GetFirstItem();

  for (item = root; item; item = item->nextsibling) {
    if ((strcmp(item->text, name) == 0) && (hopcnt == 0)) return item;
    if (item->firstchild) {
      ret = _FindServer(item->firstchild, name, hopcnt-1);
      if (ret) return ret;
    }
  }

  return NULL;
}

void OXServerTree::_UpdateLabels(OServerLink *link) {
  char tmp[256];

  if (link) {
    _servName->SetText(new OString(link->serverName));
    sprintf(tmp, "%s [%d]", link->connectedTo, link->hop);
    _connTo->SetText(new OString(tmp));
    const char *p = link->serverMsg;
    char *d = tmp;
    while (*p) {
      if (*p == 0x07 || *p == 0x02 || *p == 0x16 || *p == 0x1F || *p == 0x0F) {
        ++p;
      } else if (*p == 0x03) {
        ++p;
        if (isdigit(*p)) ++p;
        if (isdigit(*p)) ++p;
        if (*p == ',') ++p;
        if (isdigit(*p)) ++p;
        if (isdigit(*p)) ++p;
      } else if (*p == 0x1B) {
        ++p;
        if (*p) {
          if (*p++ == '[') {
            while (1) {
              while (isdigit(*p)) ++p;
              if (*p != ';') break;
              ++p;
            }
            if (*p) ++p;
          }
        }
      } else {
        *d++ = *p++;
      }
    }
    *d = '\0';
    _servMsg->SetText(new OString(tmp));
    Layout();
  } else {
    _servName->SetText(new OString(""));
    _connTo->SetText(new OString(""));
    _servMsg->SetText(new OString(""));
  }
}
