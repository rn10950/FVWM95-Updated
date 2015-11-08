/**************************************************************************

    This file is part of taskbar.
    Copyright (C) 2000, Hector Peraza.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

**************************************************************************/

#include <FVWMconfig.h>

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <stdarg.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xproto.h>
#include <X11/Xatom.h>

#include <xclass/OXClient.h>
#include <xclass/OXWindow.h>

#include "OXTaskBar.h"


FILE *console;
OXClient *clientX;
OXTaskBar *TaskBar;
char *Module;

XErrorHandler ErrorHandler(Display *d, XErrorEvent *event);

int  OpenConsole(void);
void ConsoleMessage(char *fmt, ...);
void DeadPipe(int nonsense);


//----------------------------------------------------------------------

int main(int argc, char *argv[]) {
  char *temp, *s;

  // Save the program name for error messages and config parsing
  temp = argv[0];
  s = strrchr(argv[0], '/');
  if (s) temp = s + 1;

  // Setup the module name
  Module = StrDup(temp);

  // Open the console for messages
  OpenConsole();

  if ((argc != 6) && (argc != 7)) {
    fprintf(stderr, "%s should be started only by fvwm!\n", Module);
    ConsoleMessage("Should be started only by fvwm!\n");
    exit(1);
  }

  signal(SIGPIPE, DeadPipe);

  // Setup the XConnection

  clientX = new OXClient("");
  XSetErrorHandler((XErrorHandler) ErrorHandler);

  TaskBar = new OXTaskBar(clientX->GetRoot(), Module, 
                          atoi(argv[2]), atoi(argv[1]));

  // Receive and process messages from X and Fvwm
  clientX->Run();

  return 0;
}


//----------------------------------------------------------------------

// Detected a broken fvwm pipe - exit 

void DeadPipe(int nonsense) {
  ConsoleMessage("Received SIGPIPE signal, exiting.\n");
  delete clientX;
  FatalError("Received SIGPIPE, exiting.");
  exit(1);
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


//----------------------------------------------------------------------

// X Error Handler, needed mostly for the case when the shared pixmap
// is destroyed by the window manager without notifying us...

XErrorHandler ErrorHandler(Display *d, XErrorEvent *event) {
  char errmsg[256];

  XGetErrorText(d, event->error_code, errmsg, 256);
  ConsoleMessage("Failed request: %s\n", errmsg);
  ConsoleMessage("Major opcode: 0x%x, resource id: 0x%x\n",
                  event->request_code, event->resourceid);
}
