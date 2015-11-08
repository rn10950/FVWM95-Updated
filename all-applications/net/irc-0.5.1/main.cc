#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <signal.h>

#include "OXIrc.h"
#include "OXPreferences.h"

OXClient *clientX;
OXIrc *mainWindow;
OSettings *foxircSettings;


//----------------------------------------------------------------------

int main(int argc, char **argv) {
  char server[50], nick[15];
  int  port = -1;

  clientX = new OXClient(argc, argv);

  foxircSettings = new OSettings(clientX);
  foxircSettings->Load();

  nick[0] = 0;
  server[0] = 0;

  if (getenv("IRCSERVER")) strncpy(server, getenv("IRCSERVER"), 14);
  if (getenv("IRCNICK")) strncpy(nick, getenv("IRCNICK"), 49);

  if (argc > 1) strncpy(nick, argv[1], 14);
  if (argc > 2) strncpy(server, argv[2], 49); 
  if (argc > 3) port = atoi(argv[3]);

  if (port < 0) port = 6667;

  nick[14] = 0;
  server[49] = 0;

  // Ignore SIGPIPE errors so that the application won't die when the
  // remote end closes the connection. This will cause write (send)
  // operations on sockets to return with errno = EPIPE in these cases.

  signal(SIGPIPE, SIG_IGN);

  mainWindow = new OXIrc(clientX->GetRoot(), nick, server, port);
  mainWindow->MapWindow();

  clientX->Run();

  return 0;
}
