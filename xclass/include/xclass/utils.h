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

#ifndef __UTILS_H
#define __UTILS_H

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include <xclass/XCconfig.h>

//--- Text justification modes for OXLabel, OXButton, etc...
  
#define TEXT_LEFT         (1<<0)
#define TEXT_RIGHT        (1<<1)
#define TEXT_CENTER_X     (1<<2)

#define TEXT_TOP          (1<<3)
#define TEXT_BOTTOM       (1<<4)
#define TEXT_CENTER_Y     (1<<5)

//--- Debug masks

#define DBG_REDRAW   (1<<0)
#define DBG_MVRESIZE (1<<1)
#define DBG_EVENT    (1<<2)
#define DBG_INFO     (1<<3)
#define DBG_ERR      (1<<4)
#define DBG_WARN     (1<<5)
#define DBG_MISC     (1<<6)

#undef NULL
#define NULL 0

#ifndef min
#define min(a,b) (((a)<(b)) ? (a) : (b))
#endif

#ifndef max
#define max(a,b) (((a)>(b)) ? (a) : (b))
#endif

char *StrDup(const char *s);
int  MakePath(const char *path, mode_t mode);
void FatalError(const char *fmt, ...);
void Debug(int level, const char *fmt, ...);
int  MatchXclassVersion(const char *version, const char *rlsdate);

#if !defined(HAVE_STRERROR) || HAVE_STRERROR == 0
char *strerror(int num);
#endif  // HAVE_STRERROR


#endif  // __UTILS_H
