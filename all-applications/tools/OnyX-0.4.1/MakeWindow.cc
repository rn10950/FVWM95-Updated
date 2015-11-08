/**************************************************************************

    This file is part of OnyX, a visual development environment
    using the xclass toolkit. Copyright (C) 1997, 1998 Frank Hall.

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

#include "Objects.h"
#include "Events.h"
#include "GlobalExtern.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>

FILE *MakeFileOut;
int MakePID;
char MakeType[10];
int CustomControlsNeedToBeCompiled;
int MakeRunActive = False;
int ExecutableUpToDate;

struct stat tmp;
time_t mtime;

void MakeWindow_CloseSignaled() {
  MakeTimer->Stop();
  MakeRunActive = False;
}

void MakeWindow_Closed() {
  char command[255];

  /* We shouldn't automatically assume the custom control compilation worked. But right now we do. */
  CustomControlsNeedToBeCompiled = False;

  sprintf(command, "rm %s.make", ProjectApplicationName);
  system(command);
}


void MakeWindow_Load() {
  char parms[255];

  /* Record the last modification time of the executable. */
  if (stat(ProjectApplicationName, &tmp) != 0) {
    mtime = 0;
  } else {
    mtime = tmp.st_mtime;
  }  

  ExecutableUpToDate = False;

  /* Remove any stale make file outputs */
  sprintf(parms, "%s.make", ProjectApplicationName);
  remove(parms);  

  MakePID = fork();

  if (MakePID == 0) {
    sprintf(parms, "make %s > %s.make 2>> %s.make", MakeType, ProjectApplicationName, ProjectApplicationName);
    execlp("sh", "sh", "-c", parms, NULL);
  }

  /* Need to check if fork and execlp sucessful. */

  MakeFileOut = NULL;
  MakeTimer->Start();
}

void MakeWindow_WindowResized(int OldWidth, int OldHeight, int NewWidth, int NewHeight) {
  //MakeText->MoveResize(10, 10, MakeWindow->Width - 20, MakeWindow->Height - 20);
  MakeText->MoveResize(0, 0, MakeWindow->Width, MakeWindow->Height);
}

void MakeTimer_TimerElapsed() {
  char FileName[255];
  char InLine[255];
  char ShortLine[100];
  char *Marker;
  int i;

  int l;

  l = 72;

  if (!MakeFileOut) {
    sprintf(FileName, "%s.make", ProjectApplicationName);
    MakeFileOut = fopen(FileName, "r");
    for (i = 0; i < 100; i++) {
      ShortLine[i] = ' ';
    }
    MakeText->Clear();
  }

  if (MakeFileOut) {

    if (fgets(InLine, 255, MakeFileOut)) {  /* Pull in the Make results. */

      if (strstr(InLine, "is up to date") != NULL) {  /* This allows for a noncompilation make successful. */
        ExecutableUpToDate = True;
      }

      Marker = &InLine[0];

      while (strlen(Marker) > l) {  /* Cut lines to fit length of l. "ell". */
        strncpy(ShortLine, Marker, l);
        ShortLine[l] = '\0';
        MakeText->AddText(ShortLine);  /* Add line to the display. */
        Marker = Marker + l;
      }
      strcpy(ShortLine, Marker);
      MakeText->AddText(ShortLine);  /* Add remainder to the display. */

    } else {

      if (waitpid(MakePID, NULL, WNOHANG) != 0) {  /* If the make has finished. */
        MakeTimer->Stop();      /* Stop the timer. */

        fclose(MakeFileOut);    /* Close the make results file. */
        MakeText->AddText("");  /* Add a line space. */

        /* Check the executable and see if it has changed. */
        if (stat(ProjectApplicationName, &tmp) != 0) {
          if (strcmp(MakeType, "base") && strcmp(MakeType, "clean")) {
            /* It doesn't exist. Things definitively failed. */
            MakeText->AddText("Make Failed. :(");
          }
        } else if(mtime != tmp.st_mtime || ExecutableUpToDate) {
          /* Executable modified. Wahoo! It compiled! */
          MakeText->AddText("Make Complete.");

          if (MakeRunActive) {
            /* They want to run immediately. Okey dokey! */
            RunButton_ButtonClick();
            MakeWindow->Close();
          }
        } else {
          if (strcmp(MakeType, "base")) {
            /* Nothing changed. Something didn't compile. */
            MakeText->AddText("Make Failed. :(");
          }
        }
      }

    }
  }
}
