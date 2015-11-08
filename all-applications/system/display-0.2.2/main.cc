#include <FVWMconfig.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdarg.h>
#include <signal.h>
#include <unistd.h>
#include <sys/param.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>

#include <fvwm/module.h>
#include <fvwm/version.h>

#include <xclass/utils.h>
#include <xclass/OXMsgBox.h>
#include <xclass/OFileHandler.h>

#include "main.h"
#include "bckgnd_tab.h"
#include "scrnsav_tab.h"
#include "appearance_tab.h"
#include "settings_tab.h"

#include "screen.xpm"
#include "es.xpm"


//--- fvwm95 module stuff

FILE *console;
char *Module;

int  OpenConsole(void);
void ConsoleMessage(char *fmt, ...);
void DeadPipe(int nonsense);


//----------------------------------------------------------------------

// TODO:
// - when used with fvwm95, add a "fvwm95" tab to configure fvwm95 settings
// - fix fvwm95/98 bugs regarding window recoloring
// - load/save schemes in some rc file
// - add support for fvwm95/98 module mode.

// fvwm95 commands:
//   WindowFont <XLFD font>
//   IconFont <XLFD font>
//   MenuFont <XLFD font>
//   DefaultColors winfg winbg titlefg titlebg
//   MenuColors menufg menubg selfg selbg
//   HilightColors titlefg titlebg
// optionally Style "*" BackColor titlebg
//            Style "*" ForeColor titlefg

//----------------------------------------------------------------------

OXMain::OXMain(const OXWindow *p, int w, int h, char *modname,
               int inp, int outp) : OXMainFrame(p, w, h) {

    _pipein = inp;
    _pipeout = outp;

    _fvwmPipe = NULL;

    _moduleName = modname ? StrDup(modname) : NULL;

    if (_moduleName) {
      // Set fvwm Message Mask
      char tmp[50];
      int mask = M_CONFIG_INFO;

      sprintf(tmp, "SET_MASK %lu\n", mask);
      SendFvwmText(tmp);

      _fvwmPipe = new OFileHandler(this, _pipein, XCM_READABLE);
    }

    Energy_Star = _client->GetPicture("es.xpm", es);
    if (!Energy_Star) FatalError("Pixmap not found: es.xpm");

    Monitor = _client->GetPicture("screen.xpm", ecran);
    if (!Monitor) FatalError("Pixmap not found: screen.xpm");

    OXCompositeFrame *Buttons = new OXCompositeFrame(this, 250, 20,
                                               HORIZONTAL_FRAME | FIXED_WIDTH);

    OKButton = new OXTextButton(Buttons, new OHotString("&OK"), ID_OK);
    OKButton->Associate(this);

    CancelButton = new OXTextButton(Buttons, new OHotString("&Cancel"), ID_CANCEL);
    CancelButton->Associate(this);

    ApplyButton = new OXTextButton(Buttons, new OHotString("&Apply"), ID_APPLY);
    ApplyButton->Associate(this);

    SetDefaultAcceptButton(OKButton);
    SetDefaultCancelButton(CancelButton);

    int width = OKButton->GetDefaultWidth();
    width = MAX(width, CancelButton->GetDefaultWidth());
    width = MAX(width, ApplyButton->GetDefaultWidth());

    Buttons->Resize((width+20)*3, Buttons->GetDefaultHeight());

    ButtonLayout = new OLayoutHints(LHINTS_EXPAND_X, 5, 0, 0, 0);

    Buttons->AddFrame(OKButton, ButtonLayout);
    Buttons->AddFrame(CancelButton, ButtonLayout);
    Buttons->AddFrame(ApplyButton, ButtonLayout);

    ButtonFrameLayout = new OLayoutHints(LHINTS_BOTTOM | LHINTS_RIGHT,
                                         0, 5, 0, 5);

    AddFrame(Buttons, ButtonFrameLayout);

    TabLayout = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y,
                                 5, 5, 5, 5);
    TabItemLayout = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y,
                                     5, 5, 5, 5);

    _tab = new OXTab(this, 10, 10);
    AddFrame(_tab, TabLayout);

    AddTabs();

    if (_moduleName)
      SetWindowName("Display properties - fvwm95 module mode");
    else
      SetWindowName("Display properties - standalone mode");

    MapSubwindows();

    ODimension size = ODimension(w, h);  //GetDefaultSize();
    Resize(size);

#if 1
    SetWMSize(size.w, size.h);
    SetWMSizeHints(size.w, size.h, size.w, size.h, 0, 0);
    SetMWMHints(MWM_DECOR_ALL | MWM_DECOR_RESIZEH | MWM_DECOR_MAXIMIZE |
                                MWM_DECOR_MINIMIZE | MWM_DECOR_MENU,
                MWM_FUNC_ALL | MWM_FUNC_RESIZE | MWM_FUNC_MAXIMIZE | 
                               MWM_FUNC_MINIMIZE,
                MWM_INPUT_MODELESS);
#endif

    Layout();   /////
    MapWindow();
}

OXMain::~OXMain() {
  delete ButtonLayout;
  delete ButtonFrameLayout;
  delete TabLayout;
  delete TabItemLayout;

  XSync(GetDisplay(), False);

  if (_moduleName) delete[] _moduleName;
  if (_fvwmPipe) delete _fvwmPipe;

  delete[] _tabs;
}

void OXMain::AddTabs() {
  OXCompositeFrame *f, *vf;

  _tabs = new OXFrame*[4];
  _ntabs = 0;

  f = _tab->AddTab(new OString("Background"));
  vf = new OXBackgroundTab(f, Monitor);
  f->AddFrame(vf, TabItemLayout);
  vf->Associate(this);
  _tabs[_ntabs++] = vf;

  f = _tab->AddTab(new OString("Screen saver"));
  vf = new OXScreenSaversTab(f, Monitor);
  f->AddFrame(vf, TabItemLayout);
  vf->Associate(this);
  _tabs[_ntabs++] = vf;

  f = _tab->AddTab(new OString("Appearance"));
  vf = new OXAppearanceTab(f, Monitor);
  f->AddFrame(vf, TabItemLayout);
  vf->Associate(this);
  _tabs[_ntabs++] = vf;

  f = _tab->AddTab(new OString("Settings"));
  vf = new OXSettingsTab(f, Monitor);
  f->AddFrame(vf, TabItemLayout);
  vf->Associate(this);
  _tabs[_ntabs++] = vf;
}

int OXMain::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;

  switch (msg->type) {
    case MSG_BUTTON:
      switch (msg->action) {
        case MSG_CLICK:
          switch (wmsg->id) {
            case ID_OK:
            case ID_CANCEL:
              CloseWindow();
              break;

            case ID_APPLY:
              for (int i = 0; i < _ntabs; ++i) {
                SendMessage(_tabs[i], msg);
              }
              break;
          }
          break;

        default:
          break;

      }
      break;

    case MSG_FVWM:
      switch (msg->action) {
        case MSG_SEND:
          SendFvwmText(((OFvwmMessage *) msg)->text);
          break;
      }
      break;

    default:
      break;

  }

  return True;
}

//----------------------------------------------------------------------

// Communication with fvwm...

int OXMain::ReadFvwmPacket(unsigned long *header, unsigned long **body) {
  int count, total, count2, body_length;
  char *cbody;
  extern void DeadPipe(int);

  if ((count = read(_pipein, header,
                    HEADER_SIZE*sizeof(unsigned long))) > 0) {
    if (header[0] == START_FLAG) {
      body_length = header[2] - HEADER_SIZE;
      *body = new unsigned long [body_length];
      cbody = (char *) *body;
      total = 0;
      while (total < body_length * sizeof(unsigned long)) {
        if ((count2 = read(_pipein, &cbody[total],
                      body_length * sizeof(unsigned long) - total)) > 0) {
          total += count2;
        } else if (count2 < 0) {
          DeadPipe(1);
        }
      }
    } else {
      count = 0;
    }
  }
  if (count <= 0) DeadPipe(1);
  return count;
}

// SendFvwmPipe - Send a message to fvwm 

void OXMain::SendFvwmPipe(const char *message, unsigned long window) {
  int   len;
  const char *hold, *temp;
  char  *temp_msg;

  hold = message;
  
  while (1) {
    temp = strchr(hold, ',');
    if (temp != NULL) {
      temp_msg = new char[temp-hold+1];
      strncpy(temp_msg, hold, temp-hold);
      temp_msg[temp-hold] = '\0';
      hold = temp+1;
    } else {
      temp_msg = (char *) hold;
    }

    write(_pipeout, &window, sizeof(unsigned long));
    
    len = strlen(temp_msg);
    write(_pipeout, &len, sizeof(int));
    write(_pipeout, temp_msg, len);

    // keep going
    len = 1;
    write(_pipeout, &len, sizeof(int));

    if (temp_msg != hold)
      delete[] temp_msg;
    else
      break;
  }
}

void OXMain::SendFvwmText(const char *message, unsigned long window) {
  int w;
  
  if (message) {
    write(_pipeout, &window, sizeof(unsigned long));
      
    w = strlen(message);
    write(_pipeout, &w, sizeof(int));
    write(_pipeout, message, w);
      
    // keep going
    w = 1;
    write(_pipeout, &w, sizeof(int));
  }
}

// ProcessFvwmMessage - Process the message coming from Fvwm

void OXMain::ProcessFvwmMessage(unsigned long type, unsigned long *body) {
  OFvwmMessage msg(MSG_FVWM, MSG_RECEIVED, 0, type, body);
  for (int i = 0; i < _ntabs; ++i) SendMessage(_tabs[i], &msg);
}

int OXMain::HandleFileEvent(OFileHandler *fh, unsigned int mask) {
  unsigned long header[HEADER_SIZE], *body;

  if (fh == _fvwmPipe) {
    if (ReadFvwmPacket(header, &body) > 0) {
      ProcessFvwmMessage(header[1], body);
      delete[] body;
    }
    return True;
  }
  return False;
}


//-----------------------------------------------------------------------

int main(int argc, char *argv[]) {
  int  inp, outp;
  char *temp, *s;

  // Save the program name for error messages and config parsing
  temp = argv[0];
  s = strrchr(argv[0], '/');
  if (s) temp = s + 1;

  // Open the console for messages
  OpenConsole();

  if ((argc != 6) && (argc != 7)) {
    Module = NULL;  // not in module mode...
    inp = outp = -1;
  } else {
    // Setup the module name
    Module = StrDup(temp);
    // and pipe numbers
    inp = atoi(argv[2]);
    outp = atoi(argv[1]);
  }

  signal(SIGPIPE, DeadPipe);

  OXClient *clientX = new OXClient("");

  OXMain *mainWindow = new OXMain(clientX->GetRoot(), 420, 450,
                                  Module, inp, outp);
  mainWindow->MapWindow();

  clientX->Run();
  return 0;
}


//----------------------------------------------------------------------

// Detected a broken fvwm pipe - exit 

void DeadPipe(int nonsense) {
  ConsoleMessage("Received SIGPIPE signal, exiting.\n");
  FatalError("Received SIGPIPE, exiting.");
}


//----------------------------------------------------------------------

// OpenConsole - Open the console as a way of sending messages

int OpenConsole() {
#ifndef NO_CONSOLE
  if ((console = fopen("/dev/console","w")) == NULL) {
    fprintf(stderr, "%s: cannot open console\n", Module);
    return 0;
  }
#endif
  return 1;
}


//----------------------------------------------------------------------

// ConsoleMessage - Print a message on the console.  Works like printf.

void ConsoleMessage(char *fmt, ...) {
#ifndef NO_CONSOLE
  va_list args;
  FILE *filep;

  if (console == NULL)
    filep = stderr;
  else
    filep = console;

  fprintf(filep, "%s: ", Module);
  va_start(args, fmt);
  vfprintf(filep, fmt, args);
  va_end(args);
  fflush(filep);
#endif
}
