/**************************************************************************

    This file is part of xclass.
    Copyright (C) 1996-2000 David Barth, Hector Peraza.

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
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include <X11/Xlib.h>
#include <X11/keysym.h>

#include <xclass/OXFontDialog.h>

#define FDLG_OK          1
#define FDLG_CANCEL      2

#define FDLG_FONTNAMES   21
#define FDLG_FONTSIZES   22
#define FDLG_FONTSTYLES  23

#define FDLG_DEFAULTSAMPLE  "AaBbCcYyZz 1234567890"


char *fontsizes[] = {  "8",  "9", "10", "11", "12", "13", "14", "16",
                      "18", "20", "22", "24", "26", "28", "30", "32",
                      "34", "36", "48", "72", NULL };

// shouldn't we use check buttons for this?

char *fontstyles[] = { "Normal", "Bold", "Italic", "Bold Italic", NULL };

//----------------------------------------------------------------------

static char *Capitalize(char *str) {
  char *p = str;
  if (!p) return NULL;
  if (*p) *p = toupper(*p);
  while ((p = strchr(p, ' ')) != NULL) {
    while (*p == ' ') ++p;
    if (*p) *p = toupper(*p);
  }
  return str;
}

static int CompareNames(const void *item1, const void *item2) {
  return strcmp(*((char **)item1), *((char **)item2));
}


static void SortFontNames(char **names) {
  int n;

  for (n = 0; names[n] != NULL; ++n) ;

  qsort(names, n, sizeof(char *), CompareNames);
}

//----------------------------------------------------------------------

// nret is a pointer to an OString object, on return it will have its
// contents replaced with the name of the selected font; if the dialog was
// cancelled, the string remains unchanged.

// shouldn't we return an OXFont object instead?

OXFontDialog::OXFontDialog(const OXWindow *p, const OXWindow *t,
                           OString *nret, OString *sample) :
  OXTransientFrame(p, t, 100, 100) {
    OXLabel *lbl;
    OXHorizontalFrame *hf;
    OXVerticalFrame *vf;
    int i, w;

    OLayoutHints *lh1 = new OLayoutHints(LHINTS_LEFT | LHINTS_EXPAND_Y);
    OLayoutHints *lh2 = new OLayoutHints(LHINTS_LEFT | LHINTS_TOP, 2, 0, 2, 2);
    OLayoutHints *lhb = new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 5, 5, 0, 5);

    hf = new OXHorizontalFrame(this, 10, 10);
    AddFrame(hf, new OLayoutHints(LHINTS_NORMAL, 5, 5, 5, 5));

    //--------------------- font names listbox

    vf = new OXVerticalFrame(hf, 10, 10);

    lbl = new OXLabel(vf, new OHotString("&Font"));
    vf->AddFrame(lbl, lh2);

    _fontNames = new OXListBox(vf, FDLG_FONTNAMES);
    _fontNames->Resize(120, 120/*_fontNames->GetDefaultHeight()*/);
    _fontNames->Associate(this);
    vf->AddFrame(_fontNames, lh1);

    hf->AddFrame(vf, new OLayoutHints(LHINTS_LEFT | LHINTS_EXPAND_Y,
                                      0, 10, 2, 2));

    //--------------------- font styles listbox

    vf = new OXVerticalFrame(hf, 10, 10);

    lbl = new OXLabel(vf, new OHotString("St&yle"));
    vf->AddFrame(lbl, lh2);

    _fontStyles = new OXListBox(vf, FDLG_FONTSTYLES);
    _fontStyles->Resize(80, _fontStyles->GetDefaultHeight());
    _fontStyles->Associate(this);
    vf->AddFrame(_fontStyles, lh1);

    hf->AddFrame(vf, new OLayoutHints(LHINTS_LEFT | LHINTS_EXPAND_Y,
                                      0, 10, 2, 2));

    //--------------------- font sizes listbox

    vf = new OXVerticalFrame(hf, 10, 10);

    lbl = new OXLabel(vf, new OHotString("&Size"));
    vf->AddFrame(lbl, lh2);

    _fontSizes = new OXListBox(vf, FDLG_FONTSIZES);
    _fontSizes->Resize(50, _fontSizes->GetDefaultHeight());
    _fontSizes->Associate(this);
    vf->AddFrame(_fontSizes, lh1);

    hf->AddFrame(vf, new OLayoutHints(LHINTS_LEFT | LHINTS_EXPAND_Y,
                                      0, 0, 2, 2));

    //--------------------- OK and Cancel buttons

    vf = new OXVerticalFrame(hf, 10, 10, FIXED_WIDTH);

    _ok = new OXTextButton(vf, new OHotString("OK"), FDLG_OK);
    _ok->Associate(this);
    vf->AddFrame(_ok, lhb);

    _cancel = new OXTextButton(vf, new OHotString("Cancel"), FDLG_CANCEL);
    _cancel->Associate(this);
    vf->AddFrame(_cancel, lhb);

    vf->Resize(_cancel->GetDefaultWidth()+30, vf->GetDefaultHeight());

    w = hf->GetDefaultWidth();
    i = lbl->GetDefaultHeight() + 6;

    hf->AddFrame(vf, new OLayoutHints(LHINTS_LEFT | LHINTS_EXPAND_Y,
                                      10, 0, i, 0));

    SetDefaultAcceptButton(_ok);
    SetDefaultCancelButton(_cancel);
    SetFocusOwner(_fontNames);

    //--------------------- sample box

    _gf = new OXGroupFrame(this, new OString("Sample"), FIXED_SIZE);
    _gf->OldInsets(False);
    AddFrame(_gf, new OLayoutHints(LHINTS_NORMAL, 5, 5, 0, 5));

    _cf = new OXCompositeFrame(_gf, 10, 10, CHILD_FRAME);
    _gf->AddFrame(_cf, new OLayoutHints(LHINTS_EXPAND_ALL));
    
    const char *stext = sample ? sample->GetString() : FDLG_DEFAULTSAMPLE;
    _sample = new OXLabel(_cf, new OString(stext));
    _cf->AddFrame(_sample, new OLayoutHints(LHINTS_CENTER));

    _gf->Resize(w, 80);

    //--------------------- initialize controls

    Resize(GetDefaultSize());

    char **fonts = _client->GetFontPool()->GetFontFamilies();

    SortFontNames(fonts);

    for (i = 0; fonts[i] != NULL; ++i)
      _fontNames->AddEntry(new OString(Capitalize(fonts[i])), i);

    for (i = 0; fontsizes[i] != NULL; ++i)
      _fontSizes->AddEntry(new OString(fontsizes[i]), i);

    for (i = 0; fontstyles[i] != NULL; ++i)
      _fontStyles->AddEntry(new OString(fontstyles[i]), i);

    for (i = 0; fonts[i] != NULL; ++i)
      if (strcasecmp(fonts[i], "helvetica") == 0) _fontNames->Select(i);
    _fontSizes->Select(4);
    _fontStyles->Select(0);

    if (nret) {
      OFontAttributes fa;
      OFontPool *fp = _client->GetFontPool();

      if (fp->ParseFontName(nret->GetString(), &fa) == 0) {
        for (i = 0; fonts[i] != NULL; ++i)
          if (strcasecmp(fonts[i], fa.family) == 0) {
            _fontNames->Select(i);
            break;
          }
        for (i = 0; fontsizes[i] != NULL; ++i)
          if (atoi(fontsizes[i]) >= abs(fa.pointsize)) {
            _fontSizes->Select(i);
            break;
          }
        i = 0;
        if (fa.slant == FONT_FS_ITALIC) i = 2;
        if (fa.weight == FONT_FW_BOLD) ++i;
        _fontStyles->Select(i);
      }
    }

    _client->GetFontPool()->FreeFontFamilies(fonts);

    _font = NULL;
    _ret = nret;
    _GetFontName();

    MapSubwindows();
    Resize(GetDefaultSize());

    CenterOnParent();

    //---- make the dialog box non-resizable

    SetWMSize(_w, _h);
    SetWMSizeHints(_w, _h, _w, _h, 0, 0);

    SetWindowName("Font Selection");
    SetIconName("Font Selection");
    SetClassHints("XCLASS", "dialog");

    SetMWMHints(MWM_DECOR_ALL | MWM_DECOR_RESIZEH | MWM_DECOR_MAXIMIZE /*|
                                MWM_DECOR_MINIMIZE*/ | MWM_DECOR_MENU,
                MWM_FUNC_ALL | MWM_FUNC_RESIZE | MWM_FUNC_MAXIMIZE /*| 
                               MWM_FUNC_MINIMIZE*/,
                MWM_INPUT_MODELESS);


    //---- grab accelerator keys for listboxes

    _GrabAltKey(XK_f);
    _GrabAltKey(XK_y);
    _GrabAltKey(XK_s);


    MapWindow();

    _client->WaitFor(this);
}

OXFontDialog::~OXFontDialog() {
}

int OXFontDialog::HandleKey(XKeyEvent *event) {
  if ((event->type == KeyPress) && (event->state & Mod1Mask)) {
    int keysym = XKeycodeToKeysym(GetDisplay(), event->keycode, 0);
    switch (keysym) {
      case XK_f: _fontNames->RequestFocus(); break;
      case XK_y: _fontStyles->RequestFocus(); break;
      case XK_s: _fontSizes->RequestFocus(); break;
      default: return OXTransientFrame::HandleKey(event);
    }
    return True;
  }
  return OXTransientFrame::HandleKey(event);
}

int OXFontDialog::ProcessMessage(OMessage *msg) {
  //OListBoxMessage *lbmsg = (OListBoxMessage *) msg;
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;

  switch (msg->type) {
    case MSG_BUTTON:
      if (msg->action == MSG_CLICK) {
        if (wmsg->id == FDLG_OK) {
          if (_ret) {
            _ret->Clear();
            _ret->Append(_fname);
          }
          CloseWindow();
        } else if (wmsg->id == FDLG_CANCEL) {
          CloseWindow();
        }
      }
      break;

    case MSG_LISTBOX:
      if (msg->action == MSG_CLICK) {
        _GetFontName();
      }
      break;
  }

  return True;
}

void OXFontDialog::_GetFontName() {
  OXTextLBEntry *e;
  const char *name, *size, *style;

  e = (OXTextLBEntry *) _fontNames->GetSelectedEntry();
  name = e ? e->GetText()->GetString() : "";

  e = (OXTextLBEntry *) _fontSizes->GetSelectedEntry();
  size = e ? e->GetText()->GetString() : "";

  e = (OXTextLBEntry *) _fontStyles->GetSelectedEntry();
  style = e ? e->GetText()->GetString() : "";

  sprintf(_fname, "\"%s\" -%sp %s", name, size, style);

  // do not free the old font, since it now "belongs" to the label

  _font = _client->GetFont(_fname);
  _sample->SetFont(_font);
  _cf->Layout();

  //printf("\"%s\" (%#x)\n", _fname, _font);
}

void OXFontDialog::_GrabAltKey(int keysym) {

  int keycode = XKeysymToKeycode(GetDisplay(), keysym);

  XGrabKey(GetDisplay(), keycode, Mod1Mask,
           _id, True, GrabModeAsync, GrabModeAsync);
  XGrabKey(GetDisplay(), keycode, Mod1Mask | Mod2Mask,
           _id, True, GrabModeAsync, GrabModeAsync);
  XGrabKey(GetDisplay(), keycode, Mod1Mask | LockMask,
           _id, True, GrabModeAsync, GrabModeAsync);
  XGrabKey(GetDisplay(), keycode, Mod1Mask | Mod2Mask | LockMask,
           _id, True, GrabModeAsync, GrabModeAsync);
}
