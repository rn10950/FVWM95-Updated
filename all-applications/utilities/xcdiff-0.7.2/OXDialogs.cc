/**************************************************************************
 
    This file is part of xcdiff, a front-end to the diff command.              
    Copyright (C) 1998-2002 Matzka Gerald, Hector Peraza.            
 
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

#include <xclass/OX3dLines.h>
#include <xclass/OXCheckButton.h>
#include <xclass/OXMsgBox.h>

#include "OXDialogs.h"


//----------------------------------------------------------------------

ODiffColors::ODiffColors() {
  normal_default = True;
  normal_fg = OColor(0, 0, 0);
  normal_bg = OColor(255, 255, 255);
  changed_fg = OColor(0, 0, 0);
  changed_bg = OColor(0, 255, 0);
  added_fg = OColor(0, 0, 0);
  added_bg = OColor(0, 0, 255);
  deleted_fg = OColor(0, 0, 0);
  deleted_bg = OColor(255, 0, 0);
}

//----------------------------------------------------------------------

OXDiffColorsDialog::OXDiffColorsDialog(const OXWindow *p, const OXWindow *t,
                                       ODiffColors *colors, int *retc) :
  OXTransientFrame(p, t, 100, 100) {
    int width;

    _colors = colors;
    _retc = retc;
    if (_retc) *_retc = ID_CANCEL;

    //--- OK and Cancel buttons
    OXHorizontalFrame *bf = new OXHorizontalFrame(this, 60, 20, FIXED_WIDTH);

    OXTextButton *Ok, *Cancel;
    Ok = new OXTextButton(bf, new OHotString("OK"), ID_OK);
    Cancel = new OXTextButton(bf, new OHotString("Cancel"), ID_CANCEL);

    Ok->Associate(this);
    Cancel->Associate(this);

    SetDefaultAcceptButton(Ok);
    SetDefaultCancelButton(Cancel);

    //--- layout for buttons: top align, equally expand horizontally
    bly = new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 5, 0, 0, 0);

    //--- layout for the frame: place at bottom, right-aligned
    bfly = new OLayoutHints(LHINTS_BOTTOM | LHINTS_RIGHT, 0, 5, 5, 4);

    bf->AddFrame(Ok, bly);
    bf->AddFrame(Cancel, bly);

    width = Ok->GetDefaultWidth();
    width = max(width, Cancel->GetDefaultWidth());
    bf->Resize((width + 20) * 2, bf->GetDefaultHeight());

    AddFrame(bf, bfly);

    //--- color selection buttons

    OXHorizontalFrame *hf;
    ly1 = new OLayoutHints(LHINTS_LEFT | LHINTS_CENTER_Y, 0, 20, 0, 0);
    ly2 = new OLayoutHints(LHINTS_RIGHT | LHINTS_CENTER_Y, 10, 0, 0, 0);
    ly3 = new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 5, 5, 5, 0);

    hf = new OXHorizontalFrame(this, 20, 20, CHILD_FRAME);
    hf->AddFrame(new OXLabel(hf, new OString("Normal text")), ly1);
    hf->AddFrame(nbg = new OXColorSelect(hf, _colors->normal_bg, 1001), ly2);
    hf->AddFrame(new OXLabel(hf, new OString("on")), ly2);
    hf->AddFrame(nfg = new OXColorSelect(hf, _colors->normal_fg, 1002), ly2);
    AddFrame(hf, ly3);

    hf = new OXHorizontalFrame(this, 20, 20, CHILD_FRAME);
    hf->AddFrame(new OXLabel(hf, new OString("Changed text")), ly1);
    hf->AddFrame(cbg = new OXColorSelect(hf, _colors->changed_bg, 1001), ly2);
    hf->AddFrame(new OXLabel(hf, new OString("on")), ly2);
    hf->AddFrame(cfg = new OXColorSelect(hf, _colors->changed_fg, 1002), ly2);
    AddFrame(hf, ly3);

    hf = new OXHorizontalFrame(this, 20, 20, CHILD_FRAME);
    hf->AddFrame(new OXLabel(hf, new OString("Added text")), ly1);
    hf->AddFrame(abg = new OXColorSelect(hf, _colors->added_bg, 1001), ly2);
    hf->AddFrame(new OXLabel(hf, new OString("on")), ly2);
    hf->AddFrame(afg = new OXColorSelect(hf, _colors->added_fg, 1002), ly2);
    AddFrame(hf, ly3);
                 
    hf = new OXHorizontalFrame(this, 20, 20, CHILD_FRAME);
    hf->AddFrame(new OXLabel(hf, new OString("Deleted text")), ly1);
    hf->AddFrame(dbg = new OXColorSelect(hf, _colors->deleted_bg, 1001), ly2);
    hf->AddFrame(new OXLabel(hf, new OString("on")), ly2);
    hf->AddFrame(dfg = new OXColorSelect(hf, _colors->deleted_fg, 1002), ly2);
    AddFrame(hf, ly3);

    AddFrame(new OXHorizontal3dLine(this), ly3);
                 
    //--- setup toplevel: non-resizable, modal, etc.

    MapSubwindows();
    ODimension size = GetDefaultSize();
    Resize(size);

    CenterOnParent();

    SetWMSize(size.w, size.h);
    SetWMSizeHints(size.w, size.h, size.w, size.h, 0, 0);

    SetWindowName("Select colors");
    SetIconName("Select colors");
    SetClassHints("XCLASS", "xcdiff");

    SetMWMHints(MWM_DECOR_ALL | MWM_DECOR_RESIZEH | MWM_DECOR_MAXIMIZE |
                                MWM_DECOR_MINIMIZE | MWM_DECOR_MENU,
                MWM_FUNC_ALL | MWM_FUNC_RESIZE | MWM_FUNC_MAXIMIZE | 
                               MWM_FUNC_MINIMIZE,
                MWM_INPUT_MODELESS);

    SetFocusOwner(Ok);

    MapSubwindows();
    Layout();
    MapWindow();

    _client->WaitFor(this);
}

OXDiffColorsDialog::~OXDiffColorsDialog() {
  delete bly;
  delete bfly;
  delete ly1;
  delete ly2;
  delete ly3;
}

int OXDiffColorsDialog::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;

  switch (msg->action) {
    case MSG_CLICK:

      switch (msg->type) {
        case MSG_BUTTON:
          switch (wmsg->id) {
            case ID_OK:
              _colors->normal_fg  = nfg->GetColor();
              _colors->normal_bg  = nbg->GetColor();
              _colors->changed_fg = cfg->GetColor();
              _colors->changed_bg = cbg->GetColor();
              _colors->added_fg   = afg->GetColor();
              _colors->added_bg   = abg->GetColor();
              _colors->deleted_fg = dfg->GetColor();
              _colors->deleted_bg = dbg->GetColor();
              if (_retc) *_retc = ID_OK;
            case ID_CANCEL:
              CloseWindow();
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
