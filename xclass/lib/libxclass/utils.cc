/**************************************************************************

    This file is part of xclass, a Win95-looking GUI toolkit.
    Copyright (C) 1996, 1997 David Barth, Hector Peraza.

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

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>

#include <xclass/utils.h>
#include <xclass/version.h>

//int _debugMask = DBG_REDRAW | DBG_MVRESIZE | DBG_INFO | DBG_MISC;
//int _debugMask = 0;
int _debugMask = DBG_ERR | DBG_WARN;


void FatalError(const char *fmt, ...) {
  va_list args;

  fprintf(stderr, "Fatal error: ");
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);

  fprintf(stderr, "\n");
  exit (-1);
}

void Debug(int level, const char *fmt, ...) {
  va_list args;

  if (level & _debugMask) {
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fflush(stderr);
  }
}

char *StrDup(const char *str) {
  char *p = new char[strlen(str)+1];
  strcpy(p, str);
  return p;
}

int MakePath(const char *path, mode_t mode) {
  char *p, sp[PATH_MAX];
  int errc;
  struct stat sbuf;

  strcpy(sp, path);
  p = sp;

  while (p) {
    p = strchr(p, '/');
    if (p) {
      while (*p == '/') ++p;
      *(--p) = '\0';
    }
    if (*sp) {
      errc = mkdir(sp, mode);
      if (errc != 0) {
        if (errno == EEXIST) {
          if (stat(sp, &sbuf) != 0) return errno;
          if (!S_ISDIR(sbuf.st_mode)) return errno;
        } else {
          return errno;
        }
      }
    }
    if (p) *p++ = '/';
  }
  return 0;
}

int MatchXclassVersion(const char *version, const char *rlsdate) {
  if (strcmp(XCLASS_VERSION, version) == 0) return 1;
  Debug(DBG_WARN,
  "Warning: This program was linked to a library version that does not match\n"
  "the include files used at compile time:\n"
  " library  version %s, release date %s\n"
  " includes version %s, release date %s\n",
  (char *) XCLASS_VERSION, (char *) XCLASS_RELEASE_DATE,
  (char *) version, (char *) rlsdate);
  return 0;
}

#if !defined(HAVE_STRERROR) || HAVE_STRERROR == 0
char *strerror(int num) {
  extern int sys_nerr;
  extern char *sys_errlist[];

  if (num >= 0 && num < sys_nerr)
    return(sys_errlist[num]);
  else
    return "Unknown error number";
}
#endif


//#define TRACE_MEM

#ifdef TRACE_MEM

#include <malloc.h>
#include <signal.h>
#include <unistd.h>

static FILE *__f = NULL;

void OpenLog() {
  if (!__f) {
    __f = fopen("mem.log", "a");
    if (!__f) FatalError("Can't open log file");
    fprintf(__f, "******** Log started ********\n"); fflush(__f);
  }
}

void* operator new(size_t n) {
  OpenLog();
  fprintf(__f, "Allocating %d bytes. ", n);
  void *p = malloc(n);
  fprintf(__f, "Returning ptr = %#x\n", p);
  return p;
}

void* operator new[](size_t n) {
  OpenLog();
  fprintf(__f, "Allocating %d bytes[]. ", n); fflush(__f);
  void *p = malloc(n);
  fprintf(__f, "Returning ptr[] = %#x\n", p); fflush(__f);
  return p;
}

void operator delete(void *p) {
  OpenLog();
  fprintf(__f, "Freeing ptr = %#x\n", p); fflush(__f);
  if (p == NULL) {
    raise(SIGSEGV);
    pause();
  }
  free(p);
}

void operator delete[](void *p) {
  OpenLog();
  fprintf(__f, "Freeing ptr[] = %#x\n", p); fflush(__f);
  if (p == NULL) {
    raise(SIGSEGV);
    pause();
  }
  free(p);
}

#endif // TRACE_MEM
