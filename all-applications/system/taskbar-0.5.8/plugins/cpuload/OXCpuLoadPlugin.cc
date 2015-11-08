#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <xclass/OXClient.h>
#include <xclass/OPicture.h>
#include <xclass/OXFrame.h>
#include <xclass/OXMenu.h>
#include <xclass/OTimer.h>
#include <xclass/OXFont.h>
#include <xclass/OGC.h>
#include <xclass/OIniFile.h>

#include "OXCpuLoadPlugin.h"
#include "OProc.h"
#include "../../plugins.h"


#define DLY100         500
#define DLY250         501
#define DLY500         502
#define DLY1000        503
#define DLY5000        504
#define DLY10000       505

#define LINES          300
#define BARS           301
#define SHADES         302

#define BLUE           0
#define GREEN          1
#define ORANGE         2
#define PURPLE         3
#define RED            4
#define TURQUOISE      5
#define YELLOW         6

#define COLOR0         1500
#define COLOR1         1600

#define M_SMP_MODE     100
#define M_SHOW_LABELS  101
#define M_SHOW_GRID    102
#define M_CLEAR        103
#define M_EXIT         104


OXCpuLoadPlugin *CreatePlugin(const OXWindow *p, OComponent *c,
                              OIniFile *rcfile, int id) {
  return new OXCpuLoadPlugin(p, c, rcfile, id);
}

        
OXCpuLoadPlugin::OXCpuLoadPlugin(const OXWindow *p, OComponent *c,
                                 OIniFile *rcfile, int id) : 
  OXFrame(p, 16, 32, CHILD_FRAME | SUNKEN_FRAME | OWN_BKGND) {
    XGCValues gcval;
    unsigned long gcm;

    SetBackgroundColor(_client->GetColorByName("darkslategray"));

    _msgObject = c;
    _pluginID = id;

    _proc = new OProc();
    _hasSMP = _proc->CheckSMP();

    _font = _client->GetFont("Helvetica -8");

    _pixel0 = _client->GetColorByName("green");
    _pixel1 = _client->GetColorByName("red");

    gcm = GCForeground | GCBackground | GCFont | GCGraphicsExposures;
    gcval.foreground = _pixel0;
    gcval.background = _backPixel;
    gcval.font = _font->GetId();
    gcval.graphics_exposures = False;
    _gc = new OXGC(GetDisplay(), _id, gcm, &gcval);

    // defaults
    _upd = 5000;
    _color0 = 4;
    _color1 = 1;
    _grid = False;
    _labels = True;
    _style = 1;
    _SMPmode = True;

    _hlen = 32;
    _hbfr0 = new int[_hlen];
    _hbfr1 = new int[_hlen];
    for (int i = 0; i < _hlen; ++i) _hbfr0[i] = _hbfr1[i] = 0;

    _ParseConfig(rcfile);
    if (_hasSMP)
      SetTip("CPU usage: ?% (CPU0: ?%, CPU1: ?%)");
    else
      SetTip("CPU usage: ?%");

    _umenu = new OXPopupMenu(_client->GetRoot());
    _umenu->AddEntry(new OHotString("100 msec"), DLY100);
    _umenu->AddEntry(new OHotString("250 msec"), DLY250);
    _umenu->AddEntry(new OHotString("500 msec"), DLY500);
    _umenu->AddEntry(new OHotString("1 sec"), DLY1000);
    _umenu->AddEntry(new OHotString("5 sec"), DLY5000);
    _umenu->AddEntry(new OHotString("10 sec"), DLY10000);
    _umenu->RCheckEntry(DLY5000, DLY100, DLY10000);

    _c0menu = new OXPopupMenu(_client->GetRoot());
    _c0menu->AddEntry(new OHotString("Blue"), COLOR0+BLUE);
    _c0menu->AddEntry(new OHotString("Green"), COLOR0+GREEN);
    _c0menu->AddEntry(new OHotString("Orange"), COLOR0+ORANGE);
    _c0menu->AddEntry(new OHotString("Purple"), COLOR0+PURPLE);
    _c0menu->AddEntry(new OHotString("Red"), COLOR0+RED);
    _c0menu->AddEntry(new OHotString("Turquoise"), COLOR0+TURQUOISE);
    _c0menu->AddEntry(new OHotString("Yellow"), COLOR0+YELLOW);
    _c0menu->RCheckEntry(COLOR0+_color0, COLOR0+BLUE, COLOR0+YELLOW);

    if (_hasSMP) {
      _c1menu = new OXPopupMenu(_client->GetRoot());
      _c1menu->AddEntry(new OHotString("Blue"), COLOR1+BLUE);
      _c1menu->AddEntry(new OHotString("Green"), COLOR1+GREEN);
      _c1menu->AddEntry(new OHotString("Orange"), COLOR1+ORANGE);
      _c1menu->AddEntry(new OHotString("Purple"), COLOR1+PURPLE);
      _c1menu->AddEntry(new OHotString("Red"), COLOR1+RED);
      _c1menu->AddEntry(new OHotString("Turquoise"), COLOR1+TURQUOISE);
      _c1menu->AddEntry(new OHotString("Yellow"), COLOR1+YELLOW);
      _c1menu->RCheckEntry(COLOR1+_color1, COLOR1+BLUE, COLOR1+YELLOW);
    }

    _dmenu = new OXPopupMenu(_client->GetRoot());
    _dmenu->AddEntry(new OHotString("&Lines"), LINES);
    _dmenu->AddEntry(new OHotString("&Bars"), BARS);
    _dmenu->AddEntry(new OHotString("&Shades"), SHADES);
    _dmenu->RCheckEntry(LINES+_style, LINES, SHADES);

    _menu = new OXPopupMenu(_client->GetRoot());
    if (_hasSMP) {
      _menu->AddEntry(new OHotString("S&MP mode"), M_SMP_MODE);
      if (_SMPmode) _menu->CheckEntry(M_SMP_MODE);
    }
    _menu->AddEntry(new OHotString("Show &Labels"), M_SHOW_LABELS);
    _menu->AddEntry(new OHotString("Show &Grid"), M_SHOW_GRID);
    _menu->AddSeparator();
    _menu->AddEntry(new OHotString("&Clear"), M_CLEAR);
    _menu->AddSeparator();
    _menu->AddPopup(new OHotString("&Update interval"), _umenu);
    _menu->AddPopup(new OHotString("&Style"), _dmenu);
    if (_hasSMP) {
      _menu->AddPopup(new OHotString("Color &0"), _c0menu);
      _menu->AddPopup(new OHotString("Color &1"), _c1menu);
    } else {
      _menu->AddPopup(new OHotString("C&olor"), _c0menu);
    }
    _menu->AddSeparator();
    _menu->AddEntry(new OHotString("E&xit"), M_EXIT);

    if (_grid) _menu->CheckEntry(M_SHOW_GRID);
    if (_labels) _menu->CheckEntry(M_SHOW_LABELS);

    _umenu->Associate(this);
    _dmenu->Associate(this);
    _menu->Associate(this);
    _c0menu->Associate(this);
    if (_hasSMP) _c1menu->Associate(this);

    _timer = new OTimer(this, _upd);

    AddInput(ButtonPressMask | ButtonReleaseMask);
}

OXCpuLoadPlugin::~OXCpuLoadPlugin() {
  //if (_tipMsg) delete[] _tipMsg;
  delete _menu;
  delete _umenu;
  delete _dmenu;
  delete _c0menu;
  if (_hasSMP) delete _c1menu;
  delete _timer;
  delete _gc;
  delete[] _hbfr0;
  delete[] _hbfr1;
}

void OXCpuLoadPlugin::_ParseConfig(OIniFile *rcfile) {
  char arg[256];

  if (rcfile->GetItem("update interval", arg)) {
  }

  if (rcfile->GetItem("smp mode", arg)) {
  }

  if (rcfile->GetItem("color 0", arg)) {
  }

  if (rcfile->GetItem("color 1", arg)) {
  }

  if (rcfile->GetItem("style", arg)) {
  }

  if (rcfile->GetItem("show grid", arg)) {
  }

  if (rcfile->GetItem("show labels", arg)) {
  }
}


void OXCpuLoadPlugin::_DoRedraw() {
  int i, x, y, h;

  OXFrame::_DoRedraw();

  _gc->SetForeground(_pixel0);
  h = _h - _insets.t - _insets.b;
  for (i = 0; i < _hlen; ++i) {
    if (_hbfr1[i] > 0) {
      x = _hlen - 1 + _insets.l - i;
      y = _h - _insets.b - 1 - (_hbfr0[i] * h / 100);
      if (y < _insets.t) y = _insets.t;
      DrawLine(_gc->GetGC(), x, y, x, _h - _insets.b - 1);
    }
  }
  if (_hasSMP && _SMPmode) {
    _gc->SetForeground(_pixel1);
    for (i = 0; i < _hlen; ++i) {
      if (_hbfr1[i] > 0) {
        x = _hlen + _hlen - 1 + _insets.l - i;
        y = _h - _insets.b - 1 - (_hbfr1[i] * h / 100);
        if (y < _insets.t) y = _insets.t;
        DrawLine(_gc->GetGC(), x, y, x, _h - _insets.b - 1);
      }
    }
  }
}

ODimension OXCpuLoadPlugin::GetDefaultSize() const {
  int w, h;

  w = _hlen;
  h = 18;

  if (_hasSMP && _SMPmode) w *= 2;

  return ODimension(w + _insets.l + _insets.r, h);
}

int OXCpuLoadPlugin::HandleTimer(OTimer *t) {
  int  ld, ld0, ld1;
  char ldmsg[100];

  if (t != _timer) return False;

  _proc->ReadLoad(&ld, &ld0, &ld1);

  for (int i = _hlen-1; i > 0; --i) {
    _hbfr0[i] = _hbfr0[i-1];
    if (_hasSMP) _hbfr1[i] = _hbfr1[i-1];
  }

    if (_hasSMP)
      SetTip("CPU usage: 0% (CPU0: 0%, CPU1: 0%)");
    else
      SetTip("CPU usage: 0%");

  if (_hasSMP && _SMPmode) {
    _hbfr0[0] = (ld0 + _hbfr0[0]) / 2;
    _hbfr1[0] = (ld1 + _hbfr1[0]) / 2;
    sprintf(ldmsg, "CPU usage: %d%% (CPU0: %d%%, CPU1: %d%%)", ld, ld0, ld1);
  } else {
    _hbfr0[0] = (ld + _hbfr0[0]) / 2;
    sprintf(ldmsg, "CPU usage: %d%%", ld);
  }
  SetTip(ldmsg);

  NeedRedraw();

  delete _timer;
  _timer = new OTimer(this, _upd);

  return True;
}

int OXCpuLoadPlugin::HandleButton(XButtonEvent *event) {
  if ((event->type == ButtonRelease) && 
      (event->button == Button3)) {
    _menu->PlaceMenu(event->x_root, event->y_root, False, True);
  }
  return True;
}

int OXCpuLoadPlugin::ProcessMessage(OMessage *msg) {
  if ((msg->type == MSG_MENU) && (msg->action == MSG_CLICK)) {
    OWidgetMessage *wmsg = (OWidgetMessage *) msg;

    if (wmsg->id >= COLOR0) {
      //SetColor(wmsg->id);
      return True;
    }

    if (wmsg->id >= DLY100) {
      //SetDelay(wmsg->id);
      return True;
    }

    switch (wmsg->id) {
      case M_SMP_MODE:
        if (_hasSMP) ToggleSMP();
        break;

      case M_SHOW_LABELS:
        ToggleLabels();
        break;

      case M_SHOW_GRID:
        ToggleGrid();
        break;

      case M_CLEAR:
        Clear();
        break;

      case M_EXIT:
        Unload();
        break;

      default:
        break;
    }
  }

  return True;
}

void OXCpuLoadPlugin::ToggleSMP() {
  _SMPmode = !_SMPmode;
  if (_SMPmode)
    _menu->CheckEntry(M_SMP_MODE);
  else
    _menu->UnCheckEntry(M_SMP_MODE);
  OPluginMessage msg(MSG_PLUGIN, PLUGIN_CHANGED, _pluginID);
  SendMessage(_msgObject, &msg);
  Clear();
}

void OXCpuLoadPlugin::ToggleGrid() {
  _grid = !_grid;
  if (_grid)
    _menu->CheckEntry(M_SHOW_GRID);
  else
    _menu->UnCheckEntry(M_SHOW_GRID);
  NeedRedraw();
}

void OXCpuLoadPlugin::ToggleLabels() {
  _labels = !_labels;
  if (_labels)
    _menu->CheckEntry(M_SHOW_LABELS);
  else
    _menu->UnCheckEntry(M_SHOW_LABELS);
  NeedRedraw();
}

void OXCpuLoadPlugin::Clear() {
  for (int i = 0; i < _hlen; ++i) {
    _hbfr0[i] = 0;
    _hbfr1[i] = 0;
  }
  NeedRedraw();
}

void OXCpuLoadPlugin::Unload() {
  // Request unload
  OPluginMessage msg(MSG_PLUGIN, PLUGIN_UNLOAD, _pluginID);
  SendMessage(_msgObject, &msg);
}
