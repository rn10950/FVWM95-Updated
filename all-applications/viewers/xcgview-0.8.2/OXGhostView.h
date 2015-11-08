/**************************************************************************

    This file is part of xcgview, a xclass port of Ghostview 1.5
    Copyright (C) 1992  Timothy O. Theisen.
    Copyright (C) 1996  Hector Peraza.

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

      Author: Tim Theisen           Systems Programmer
    Internet: tim@cs.wisc.edu       Department of Computer Sciences
        UUCP: uwvax!tim             University of Wisconsin-Madison
       Phone: (608)262-0438         1210 West Dayton Street
         FAX: (608)262-9777         Madison, WI   53706

    Xclass port by: Hector Peraza <peraza@mitac11.uia.ac.be>

**************************************************************************/

#ifndef __OXGHOSTVIEW_H
#define __OXGHOSTVIEW_H

#include <stdio.h>

#include <xclass/utils.h>
#include <xclass/OXFrame.h>
#include <xclass/OXWidget.h>
#include <xclass/OXClient.h>
#include <xclass/OFileHandler.h>
#include <xclass/OMessage.h>

#include <signal.h>
#ifdef SIGNALRETURNSINT
#define SIGVAL int
#else
#define SIGVAL void
#endif

#define NON_BLOCKING_IO

#ifdef NON_BLOCKING_IO
#include <fcntl.h>
/* if POSIX O_NONBLOCK is not available, use O_NDELAY */
#if !defined(O_NONBLOCK) && defined(O_NDELAY)
#define O_NONBLOCK O_NDELAY
#endif
#endif

#include <errno.h>
/* BSD 4.3 errno.h does not declare errno */
extern "C" int errno;

/* Both error returns are checked for non-blocking I/O. */
/* Manufacture the other error code if only one exists. */
#if !defined(EWOULDBLOCK) && defined(EAGAIN)
#define EWOULDBLOCK EAGAIN
#endif
#if !defined(EAGAIN) && defined(EWOULDBLOCK)
#define EAGAIN EWOULDBLOCK
#endif

// GV_BUFSIZ is set to the minimum POSIX PIPE_BUF to ensure that
// nonblocking writes to ghostscript will work properly.

#define GV_BUFSIZ 512


// Number represents clockwise rotation of the paper in degrees

typedef enum {
  PageOrientationPortrait   =   0,  // Normal portrait orientation
  PageOrientationLandscape  =  90,  // Normal landscape orientation
  PageOrientationUpsideDown = 180,  // Don't think this will be used much
  PageOrientationSeascape   = 270   // Landscape rotated the other way
} XCPageOrientation;

typedef enum {
  PaletteMonochrome,
  PaletteGrayscale,
  PaletteColor
} XCPalette;


//--- structure to describe section of file to send to ghostscript

struct SRecordList {
  FILE *fp;
  long begin;
  unsigned int len;
  int  seek_needed;
  int  close;
  struct SRecordList *next;
};


//--- Message type and actions sent to the application
//    We send OWidgetMessage objects

#define MSG_GHOSTVIEW  (MSG_USERMSG+10)

#define GV_PAGE        1
#define GV_DONE        2
#define GV_REFRESH     3
#define GV_BADALLOC    4
#define GV_FAILED      5
#define GV_OUTPUT      6


//--------------------------------------------------------------------

class OXGhostView : public OXFrame, public OXWidget {
protected:
  static int broken_pipe;
  static int alloc_error;
  static XErrorHandler oldhandler;

  static SIGVAL CatchPipe(int i);
  static int CatchAlloc(Display *dpy, XErrorEvent *err);

public:
  OXGhostView(const OXWindow *p, int w, int h, int ID,
                           unsigned int options = 0,
                           unsigned long back = _defaultFrameBackground);
  virtual ~OXGhostView();

  virtual int HandleClientMessage(XClientMessageEvent *event);
  virtual int HandleFileEvent(OFileHandler *fh, unsigned int mask);

  virtual ODimension GetDefaultSize() const;

  float ComputeXdpi();
  float ComputeYdpi();

  void  SetInterpreterCommand(const char *cmd);
  void  SetFilename(const char *filename);

  void  EnableInterpreter();
  void  DisableInterpreter();
  int   SendPS(FILE* fp, long begin, unsigned int len, int close);
  int   NextPage();
  int   SetOrientation(XCPageOrientation orientation);
  int   SetBoundingBox(int llx, int lly, int urx, int ury);
  int   SetDpi(float xdpi, float ydpi);

  char *GetOutputBuffer() const { return _output_buffer; }
  void  ClearOutputBuffer() { if (_output_buffer) *_output_buffer = '\0'; }

  // Returns True if the interpreter is ready for new input.
  int IsInterpreterReady() const {
    return ((_interpreter_pid != -1) && (!_busy) && (_ps_input == NULL));
  }
  int IsInterpreterRunning() const { return _interpreter_pid != -1; }
  Pixmap GetBackingPixmap() const { return _background_pixmap; }

protected:
  virtual void _Resized();

  void _Input(int source);
  void _Output(int source);
  int  Setup();
  void StartInterpreter();
  void StopInterpreter(int dying = False);
  void InterpreterFailed();

  //--- resources:

  Cursor _cursor, _busy_cursor;
  Pixmap _background_pixmap;
  unsigned int _bgndPixel, _fgndPixel;
  char *_interpreter;
  int  _quiet, _safer, _use_bpixmap;
  char *_arguments;
  char *_filename;
  XCPageOrientation _orientation;
  XCPalette _palette;
  float _xdpi, _ydpi;
  int _llx, _lly, _urx, _ury;
  int _left_margin, _bottom_margin, _right_margin, _top_margin;

private:
  GC _gc;		           // GC used to clear window
  Window _mwin;		           // destination of ghostsript messages
  int _disable_start;              // whether to fork ghostscript
  int _interpreter_pid;            // pid of ghostscript, -1 if none
  SRecordList *_ps_input;          // pointer in gs input queue
  char *_input_buffer;             // pointer to input buffer
  unsigned int _bytes_left;	   // bytes left in section
  char *_input_buffer_ptr;         // pointer into input buffer
  unsigned int _buffer_bytes_left; // bytes left in buffer
  int _interpreter_input;          // fd gs stdin, -1 if None
  int _interpreter_output;         // fd gs stdout, -1 if None
  int _interpreter_error;          // fd gs stderr, -1 if None
  OFileHandler *_interpreter_input_fh;   // file event handlers for above
  OFileHandler *_interpreter_output_fh;
  OFileHandler *_interpreter_error_fh;
  int _gs_width;                   // Width of window at last Setup()
  int _gs_height;                  // Height of window at last Setup()
  int _busy;                       // Is gs busy drawing?
  int _changed;                    // something changed since Setup()?
  char *_output_buffer;
};

/* The structure returned by the regular callback */

typedef struct _GhostviewReturnStruct {
    int width, height;
    int	psx, psy;
    float xdpi, ydpi;
} GhostviewReturnStruct;


#endif  // __OXGHOSTVIEW_H
