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


#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xproto.h>
#include <X11/cursorfont.h>

#include "OXGhostView.h"

Atom ATOM_GHOSTVIEW;
Atom ATOM_GHOSTVIEW_COLORS;
Atom ATOM_NEXT;
Atom ATOM_PAGE;
Atom ATOM_DONE;


//----------------------------------------------------------------------

OXGhostView::OXGhostView(const OXWindow *p, int w, int h, int ID,
                         unsigned int options, unsigned long back) :
  OXFrame(p, w, h, options, back), OXWidget(ID, "OXGhostView") {
    XGCValues gcval;
    unsigned int mask;

    _msgObject = p;
    _widgetFlags = 0;

    _fgndPixel = _blackPixel;
    _bgndPixel = back;

    gcval.foreground = back;
    mask = GCForeground;
    _gc = XCreateGC(GetDisplay(), _id, mask, &gcval);

    XSetWindowBorder(GetDisplay(), _id, _blackPixel);
    XSetWindowBorderWidth(GetDisplay(), _id, 1);

    // Get atoms needed to communicate with ghostscript.
    ATOM_GHOSTVIEW = XInternAtom(GetDisplay(), "GHOSTVIEW", False);
    ATOM_GHOSTVIEW_COLORS = XInternAtom(GetDisplay(), 
                                        "GHOSTVIEW_COLORS", False);
    ATOM_NEXT = XInternAtom(GetDisplay(), "NEXT", False);
    ATOM_PAGE = XInternAtom(GetDisplay(), "PAGE", False);
    ATOM_DONE = XInternAtom(GetDisplay(), "DONE", False);

    // configurable
    _arguments = NULL;
    _busy_cursor = XCreateFontCursor(GetDisplay(), XC_watch);
    _cursor = XCreateFontCursor(GetDisplay(), XC_crosshair);
    _filename = NULL;
    //SetFilename("-");
    _interpreter = StrDup("gs");
    _llx = _lly = 0;
    _urx = 612; _ury = 792;
    _orientation = PageOrientationPortrait;
    _palette = PaletteColor;
    _quiet = True;
    _safer = True;
    _left_margin =
    _right_margin =
    _top_margin =
    _bottom_margin = 0;
    _use_bpixmap = True;
    // end configurable

    _mwin = None;

    _disable_start = True; //False;

    _ps_input = NULL;
    _input_buffer = NULL;
    _input_buffer_ptr = NULL;

    _bytes_left = _buffer_bytes_left = 0;

    _output_buffer = new char[4096];

    _interpreter_pid       = -1;
    _interpreter_input     = -1;
    _interpreter_output    = -1;
    _interpreter_error     = -1;
    _interpreter_input_fh  = NULL;
    _interpreter_output_fh = NULL;
    _interpreter_error_fh  = NULL;

    _background_pixmap = None;

    _gs_width  = 0;
    _gs_height = 0;

    _changed = False;
    _busy = False;

    _xdpi = ComputeXdpi();
    _ydpi = ComputeYdpi();

    //Setup();
    XDefineCursor(GetDisplay(), _id, _cursor);
}

OXGhostView::~OXGhostView() {

  StopInterpreter(True);   // kill the interpreter

  XFreeGC(GetDisplay(), _gc);

  if (_input_buffer) delete[] _input_buffer;
  if (_output_buffer) delete[] _output_buffer;
  if (_background_pixmap != None)
    XFreePixmap(GetDisplay(), _background_pixmap);

  delete[] _interpreter;
}


// Procedures that compute the default xdpi and ydpi from display parameters

float OXGhostView::ComputeXdpi() {
  float xdpi;
  Screen *scr;

  scr = ScreenOfDisplay(GetDisplay(), DefaultScreen(GetDisplay()));
  xdpi = 25.4 * WidthOfScreen(scr) / WidthMMOfScreen(scr);

//  return xdpi;
  return 72.0;
}

float OXGhostView::ComputeYdpi() {
  float ydpi;
  Screen *scr;

  scr = ScreenOfDisplay(GetDisplay(), DefaultScreen(GetDisplay()));
  ydpi = 25.4 * HeightOfScreen(scr) / HeightMMOfScreen(scr);

//  return ydpi;
  return 72.0;
}


// Message action routine, passes ghostscript message events back to
// application. It also marks the interpreter as being not busy at the
// end of page, and stops the interpreter when it sends a "done" message.

int OXGhostView::HandleClientMessage(XClientMessageEvent *event) {

  if (OXFrame::HandleClientMessage(event) == True) return True;

  if (event->message_type == ATOM_PAGE) {
    _mwin = event->data.l[0];
    _busy = False;
    XDefineCursor(GetDisplay(), _id, _cursor);
    OWidgetMessage message(MSG_GHOSTVIEW, GV_PAGE, _widgetID);
    SendMessage(_msgObject, &message);
    return True;
  } else if (event->message_type == ATOM_DONE) {
    _mwin = event->data.l[0];
    StopInterpreter();
    OWidgetMessage message(MSG_GHOSTVIEW, GV_DONE, _widgetID);
    SendMessage(_msgObject, &message);
    return True;
  }

  return False;
}

// Process resize request, start a new interpreter by calling Setup()
// if size changed. If Setup() actually started a new interpreter and
// it is taking input from stdin, send a refresh message to the
// application. This is the only way that the application can be
// notified that it needs to resend the input because someone forced
// a new window size on the widget.

void OXGhostView::_Resized() {
  OXFrame::_Resized();
  if (Setup())
    if (!_filename) {
      OWidgetMessage message(MSG_GHOSTVIEW, GV_REFRESH, _widgetID);
      SendMessage(_msgObject, &message);
    }
}

void OXGhostView::SetFilename(const char *filename) {
  if (_filename) delete[] _filename;
  if (filename) {
    _filename = StrDup(filename);
  } else {
    _filename = NULL;
  }
}

void OXGhostView::SetInterpreterCommand(const char *cmd) {
  if (cmd) {
    delete[] _interpreter;
    _interpreter = StrDup(cmd);
    StopInterpreter();
    StartInterpreter();
  }
}

int OXGhostView::SetOrientation(XCPageOrientation orientation) {
  int need_setup = (_orientation != orientation);
  _orientation = orientation;
  if (need_setup) {
    _changed = True; _w = _h = 0;  // force a resize and a Setup()
    Resize(GetDefaultSize());
  }
  return need_setup;
}

int OXGhostView::SetBoundingBox(int llx, int lly, int urx, int ury) {
  int need_setup = (_llx != llx) || (_lly != lly) ||
                   (_urx != urx) || (_ury != ury);
  _llx = llx;
  _lly = lly;
  _urx = urx;
  _ury = ury;
  if (need_setup) {
    _changed = True; _w = _h = 0;
    Resize(GetDefaultSize());
  }

  return need_setup;
}

int OXGhostView::SetDpi(float xdpi, float ydpi) {
  int need_setup = (_xdpi != xdpi) || (_ydpi != ydpi);

  _xdpi = xdpi;
  _ydpi = ydpi;
  if (need_setup) {
    _changed = True; _w = _h = 0;
    Resize(GetDefaultSize());
  }

  return need_setup;
}

/*
// Notify action routine.
// Calculates where the user clicked in the default user coordinate system.
// Call the callbacks with the point of click and size of zoom window
// requested.

void OXGhostView::Notify(XEvent *event, char **params, int *num_params) {
  GhostviewReturnStruct ret_val;

  // notify takes zero to four parameters. The first two give the width and
  // height of the zoom requested in the default user coordinate system.
  // If they are omitted, a default value of 72 is provided. If the second
  // parameter is omitted, the zoom area is assumed to be a square.
  // The next two parameters give the desired resolution of the zoom window.
  // If they are omitted, a default value of 300 is provided. If the four
  // parameter is omitted, the y resolution is assumed to be equal to the
  // x resolution.

  switch (*num_params) {
    case 0:
	ret_val.width = ret_val.height = 72;
	ret_val.xdpi = ret_val.ydpi = 300;
	break;

    case 1:
	ret_val.width = ret_val.height = atoi(params[0]);
	ret_val.xdpi = ret_val.ydpi = 300;
	break;

    case 2:
	ret_val.width = atoi(params[0]);
	ret_val.height = atoi(params[1]);
	ret_val.xdpi = ret_val.ydpi = 300;
	break;

    case 3:
	ret_val.width = atoi(params[0]);
	ret_val.height = atoi(params[1]);
	ret_val.xdpi = ret_val.ydpi = atoi(params[2]);
	break;

    default:
	ret_val.width = atoi(params[0]);
	ret_val.height = atoi(params[1]);
	ret_val.xdpi = atoi(params[2]);
	ret_val.ydpi = atoi(params[3]);
	break;
  }

  switch (_orientation) {
    case PageOrientationPortrait:
	ret_val.psx = _llx + event->xbutton.x * 72.0 / _xdpi;
	ret_val.psy = _ury - event->xbutton.y * 72.0 / _ydpi;
	break;

    case PageOrientationLandscape:
	ret_val.psx = _llx + event->xbutton.y * 72.0 / _ydpi;
	ret_val.psy = _lly + event->xbutton.x * 72.0 / _xdpi;
	break;

    case PageOrientationUpsideDown:
	ret_val.psx = _urx - event->xbutton.x * 72.0 / _xdpi;
	ret_val.psy = _lly + event->xbutton.y * 72.0 / _ydpi;
	break;

    case PageOrientationSeascape:
	ret_val.psx = _urx - event->xbutton.y * 72.0 / _ydpi;
	ret_val.psy = _ury - event->xbutton.x * 72.0 / _xdpi;
	break;
  }

  //    XtCallCallbackList(w, _callback, (XtPointer) &ret_val);
}

*/

#ifndef SEEK_SET
#define SEEK_SET 0
#endif

int OXGhostView::broken_pipe = False;

SIGVAL OXGhostView::CatchPipe(int i) {
  broken_pipe = True;
#ifdef SIGNALRETURNSINT
  return 0;
#endif
}

int OXGhostView::HandleFileEvent(OFileHandler *fh, unsigned int mask) {
  if (fh == _interpreter_input_fh && mask == XCM_WRITABLE) {
    _Input(fh->GetFd());
    return True;
  } else if (fh == _interpreter_output_fh ||
             fh == _interpreter_error_fh  || mask == XCM_READABLE) {
    _Output(fh->GetFd());
    return True;
  }
  return False;
}

// Input - Feed data to ghostscript's stdin.
// Write bytes to ghostscript using non-blocking I/O.
// Also, pipe signals are caught during writing. The return
// values are checked and the appropriate action is taken. I do
// this at this low level, because it may not be appropriate for
// SIGPIPE to be caught for the overall application.

void OXGhostView::_Input(int source) {
  int bytes_written;
  SIGVAL (*oldsig)(int);

  oldsig = signal(SIGPIPE, CatchPipe);

#ifdef NON_BLOCKING_IO
  do {
#endif

    if (_buffer_bytes_left == 0) {

      // Get a new section if required
      if (_ps_input && _bytes_left == 0) {
        SRecordList *ps_old = _ps_input;
        _ps_input = ps_old->next;
        if (ps_old->close) fclose(ps_old->fp);
        delete ps_old;
      }

      // Have to seek at the beginning of each section
      if (_ps_input && _ps_input->seek_needed) {
        if (_ps_input->len > 0)
          fseek(_ps_input->fp, _ps_input->begin, SEEK_SET);
          _ps_input->seek_needed = False;
          _bytes_left = _ps_input->len;
      }

      if (_bytes_left > GV_BUFSIZ) {
        _buffer_bytes_left = fread(_input_buffer, sizeof (char),
                                   GV_BUFSIZ, _ps_input->fp);
      } else if (_bytes_left > 0) {
        _buffer_bytes_left = fread(_input_buffer, sizeof (char),
                                   _bytes_left, _ps_input->fp);
      } else {
        _buffer_bytes_left = 0;
      }

      if (_bytes_left > 0 && _buffer_bytes_left == 0) {
        InterpreterFailed();   // Error occurred
      }

      _input_buffer_ptr = _input_buffer;
      _bytes_left -= _buffer_bytes_left;
    }

    if (_buffer_bytes_left > 0) {
      bytes_written = write(_interpreter_input, _input_buffer_ptr,
                            _buffer_bytes_left);

      if (broken_pipe) {
        broken_pipe = False;
        InterpreterFailed();   // Something bad happened
      } else if (bytes_written == -1) {
        if ((errno != EWOULDBLOCK) && (errno != EAGAIN)) {
          InterpreterFailed(); // Something bad happened
        }
      } else {
        _buffer_bytes_left -= bytes_written;
        _input_buffer_ptr += bytes_written;
      }
    }
#ifdef NON_BLOCKING_IO
  } while (_ps_input && _buffer_bytes_left == 0);
#endif

  signal(SIGPIPE, oldsig);
  if (_ps_input == NULL && _buffer_bytes_left == 0) {
    if (_interpreter_input_fh) {
      delete _interpreter_input_fh;
      _interpreter_input_fh = NULL;
    }
  }
}

// Output - receive I/O from ghostscript's stdout and stderr.
// Pass this to the application via the output_callback.

void OXGhostView::_Output(int source) {
  char buf[GV_BUFSIZ+1];
  int bytes = 0;

  if (source == _interpreter_output) {

    bytes = read(_interpreter_output, buf, GV_BUFSIZ);
    if (bytes == 0) { // EOF occurred
      close(_interpreter_output);
      _interpreter_output = -1;
      delete _interpreter_output_fh;
      _interpreter_output_fh = NULL;
      return;
    } else if (bytes == -1) {
      InterpreterFailed();  // Something bad happened
      return;
    }

  } else if (source == _interpreter_error) {

    bytes = read(_interpreter_error, buf, GV_BUFSIZ);
    if (bytes == 0) { // EOF occurred
      close(_interpreter_error);
      _interpreter_error = -1;
      delete _interpreter_error_fh;
      _interpreter_error_fh = NULL;
      return;
    } else if (bytes == -1) {
      InterpreterFailed();  // Something bad happened
      return;
    }

  }

  if (bytes > 0) {
    buf[bytes] = '\0';

    if (strlen(_output_buffer)+bytes >= 4095)
      ClearOutputBuffer();

    strcat(_output_buffer, buf);
    OWidgetMessage message(MSG_GHOSTVIEW, GV_OUTPUT, _widgetID);
    SendMessage(_msgObject, &message);
  }

}


ODimension OXGhostView::GetDefaultSize() const {
  ODimension size;

  switch (_orientation) {
    default:
    case PageOrientationPortrait:
    case PageOrientationUpsideDown:
      size.w = (int) ((_urx - _llx) / 72.0 * _xdpi + 0.5);
      size.h = (int) ((_ury - _lly) / 72.0 * _ydpi + 0.5);
      break;

    case PageOrientationLandscape:
    case PageOrientationSeascape:
      size.w = (int) ((_ury - _lly) / 72.0 * _xdpi + 0.5);
      size.h = (int) ((_urx - _llx) / 72.0 * _ydpi + 0.5);
      break;
  }

  return size;
}


// Catch the alloc error when there is not enough resources for the
// backing pixmap.  Automatically shut off backing pixmap and let the
// user know when this happens.

int OXGhostView::alloc_error;
XErrorHandler OXGhostView::oldhandler;

int OXGhostView::CatchAlloc(Display *dpy, XErrorEvent *err) {
  if (err->error_code == BadAlloc) {
    alloc_error = True;
  }
  if (alloc_error) return 0;
  return oldhandler(dpy, err);
}

// Setup - sets up the backing pixmap, and GHOSTVIEW property and
// starts interpreter if needed.
// NOTE: the widget must be realized before calling Setup().
// Returns True if a new interpreter was started, False otherwise.
 
int OXGhostView::Setup() {
  char buf[GV_BUFSIZ];
  Pixmap bpixmap;
  XSetWindowAttributes xswa;

  if (!_changed && (_w == _gs_width) && (_h == _gs_height)) return False;

  StopInterpreter();

  if ((_w != _gs_width) || (_h != _gs_height) || (!_use_bpixmap)) {
    if (_background_pixmap != None) {
      XFreePixmap(GetDisplay(), _background_pixmap);
      _background_pixmap = None;
      SetBackgroundPixmap(None);
    }
  }

  if (_use_bpixmap) {
    if (_background_pixmap == None) {
      // Get a Backing Pixmap, but be ready for the BadAlloc.
      XSync(GetDisplay(), False);      // Get to known state
      oldhandler = XSetErrorHandler(OXGhostView::CatchAlloc);
      alloc_error = False;
      bpixmap = XCreatePixmap(GetDisplay(), _id, _w, _h,
                              DefaultDepth(GetDisplay(),
                                           DefaultScreen(GetDisplay())));
      XSync(GetDisplay(), False);      // Force the error
      if (alloc_error) {
        OWidgetMessage message(MSG_GHOSTVIEW, GV_BADALLOC, _widgetID);
        SendMessage(_msgObject, &message);
        if (bpixmap != None) {
          XFreePixmap(GetDisplay(), bpixmap);
          XSync(GetDisplay(), False);
          bpixmap = None;
        }
      }

      oldhandler = XSetErrorHandler(oldhandler);
      if (bpixmap != None) {
        _background_pixmap = bpixmap;
        SetBackgroundPixmap(_background_pixmap);
      }
    } else {
      bpixmap = _background_pixmap;
    }
  } else {
    if (_background_pixmap != None) {
      XFreePixmap(GetDisplay(), _background_pixmap);
      _background_pixmap = None;
      SetBackgroundPixmap(None);
    }
    bpixmap = None;
  }

  if (bpixmap != None) {
    xswa.backing_store = NotUseful;
    XChangeWindowAttributes(GetDisplay(), _id, CWBackingStore, &xswa);
  } else {
    xswa.backing_store = Always;
    XChangeWindowAttributes(GetDisplay(), _id, CWBackingStore, &xswa);
  }

  _gs_width = _w;
  _gs_height = _h;

  sprintf(buf, "%d %d %d %d %d %d %g %g %d %d %d %d",
               bpixmap, _orientation,
               _llx, _lly, _urx, _ury, _xdpi, _ydpi,
	       _left_margin, _bottom_margin,
	       _right_margin, _top_margin);
  XChangeProperty(GetDisplay(), _id, ATOM_GHOSTVIEW,
                  XA_STRING, 8, PropModeReplace,
                  (unsigned char *)buf, strlen(buf));

  sprintf(buf, "%s %d %d",
               _palette == PaletteMonochrome ? "Monochrome" :
               _palette == PaletteGrayscale  ? "Grayscale" :
               _palette == PaletteColor      ? "Color" : "?",
               _fgndPixel, _bgndPixel);
  XChangeProperty(GetDisplay(), _id, ATOM_GHOSTVIEW_COLORS,
                  XA_STRING, 8, PropModeReplace,
                  (unsigned char *)buf, strlen(buf));

  XSync(GetDisplay(), False);  // Be sure to update properties
  StartInterpreter();

  return True;
}

// This routine starts the interpreter.  It sets the DISPLAY and 
// GHOSTVIEW environment variables.  The GHOSTVIEW environment variable
// contains the Window that ghostscript should write on.

// This routine also opens pipes for stdout and stderr and initializes
// application input events for them.  If input to ghostscript is not
// from a file, a pipe for stdin is created.  This pipe is setup for
// non-blocking I/O so that the user interface never "hangs" because of
// a write to ghostscript.

void OXGhostView::StartInterpreter() {
  int  std_in[2];
  int  std_out[2];
  int  std_err[2];
  char buf[GV_BUFSIZ];
#define NUM_ARGS 100
  char *argv[NUM_ARGS];
  char *arguments = NULL;
  char *cptr;
  int  argc = 0;
  int  ret;

  StopInterpreter();

  // Clear the window before starting a new interpreter.
  if (_background_pixmap != None)
    XFillRectangle(GetDisplay(), _background_pixmap, _gc, 0, 0, _w, _h);

  XClearArea(GetDisplay(), _id, 0, 0, _w, _h, False);

  if (_disable_start) return;

  argv[argc++] = _interpreter;
  argv[argc++] = "-sDEVICE=x11";
  argv[argc++] = "-dNOPAUSE";
  if (_quiet) argv[argc++] = "-dQUIET";
  if (_safer) argv[argc++] = "-dSAFER";
  if (_arguments) {
    cptr = arguments = StrDup(_arguments);
    while (isspace(*cptr)) cptr++;
    while (*cptr) {
      argv[argc++] = cptr;
      while (*cptr && !isspace(*cptr)) cptr++;
      if (*cptr) *cptr++ = '\0';
      if (argc + 2 >= NUM_ARGS) {
        FatalError("Too many arguments to interpreter.");
        exit(1);
      }
      while (isspace(*cptr)) cptr++;
    }
  }
  argv[argc++] = "-";
  argv[argc++] = NULL;

  if (_filename == NULL) {
    ret = pipe(std_in);
    if (ret == -1) {
      //perror("Could not create pipe");
      FatalError("Could not create pipe");
      exit(1);
    }
  } else if (strcmp(_filename, "-")) {
    std_in[0] = open(_filename, O_RDONLY, 0);
  }
  ret = pipe(std_out);
  if (ret == -1) {
    //perror("Could not create pipe");
    FatalError("Could not create pipe");
    exit(1);
  }
  ret = pipe(std_err);
  if (ret == -1) {
    //perror("Could not create pipe");
    FatalError("Could not create pipe");
    exit(1);
  }

  _changed = False;
  _busy = True;
  XDefineCursor(GetDisplay(), _id, _busy_cursor);
#if defined(SYSV) || defined(USG)
#define vfork fork
#endif
  _interpreter_pid = vfork();

  if (_interpreter_pid == 0) {   // child
    close(std_out[0]);
    close(std_err[0]);
    dup2(std_out[1], 1);
    close(std_out[1]);
    dup2(std_err[1], 2);
    close(std_err[1]);
    sprintf(buf, "%d", _id);
    setenv("GHOSTVIEW", buf, True);
    setenv("DISPLAY", XDisplayString(GetDisplay()), True);
    if (_filename == NULL) {
      close(std_in[1]);
      dup2(std_in[0], 0);
      close(std_in[0]);
    } else if (strcmp(_filename, "-")) {
      dup2(std_in[0], 0);
      close(std_in[0]);
    }
    execvp(argv[0], argv);
    sprintf(buf, "Exec of %s failed", argv[0]);
    perror(buf);
    _exit(1);
  } else {
    if (_filename == NULL) {
#ifdef NON_BLOCKING_IO
      int result;
#endif
      close(std_in[0]);

#ifdef NON_BLOCKING_IO
      result = fcntl(std_in[1], F_GETFL, 0);
      result = result | O_NONBLOCK;
      result = fcntl(std_in[1], F_SETFL, result);
#endif
      _interpreter_input = std_in[1];
      _interpreter_input_fh = NULL;
    } else if (strcmp(_filename, "-")) {
      close(std_in[0]);
    }
    close(std_out[1]);
    _interpreter_output = std_out[0];
    _interpreter_output_fh = new OFileHandler(this, std_out[0],
                                              XCM_READABLE);  // _Output()
    close(std_err[1]);
    _interpreter_error = std_err[0];
    _interpreter_error_fh = new OFileHandler(this, std_err[0],
                                             XCM_READABLE);  // _Output()
  }

  if (arguments) delete[] arguments;
}

// Stop the interpreter, if present, and remove any Input sources.
// Also reset the busy state.

void OXGhostView::StopInterpreter(int dying) {

  if (_interpreter_pid >= 0) {
    kill(_interpreter_pid, SIGTERM);
    wait(0);
    _interpreter_pid = -1;
  }

  if (_interpreter_input >= 0) {
    close(_interpreter_input);
    _interpreter_input = -1;
    if (_interpreter_input_fh) {
      delete _interpreter_input_fh;
      _interpreter_input_fh = NULL;
    }
    while (_ps_input) {
      SRecordList *ps_old = _ps_input;
      _ps_input = ps_old->next;
      if (ps_old->close) fclose(ps_old->fp);
      delete ps_old;
    }
  }

  if (_interpreter_output >= 0) {
    close(_interpreter_output);
    _interpreter_output = -1;
    delete _interpreter_output_fh;
    _interpreter_output_fh = NULL;
  }

  if (_interpreter_error >= 0) {
    close(_interpreter_error);
    _interpreter_error = -1;
    delete _interpreter_error_fh;
    _interpreter_error_fh = NULL;
  }

  _busy = False;
  if (!dying) XDefineCursor(GetDisplay(), _id, _cursor);
}

// The interpeter failed, Stop what's left and notify application

void OXGhostView::InterpreterFailed() {
  StopInterpreter();
  OWidgetMessage message(MSG_GHOSTVIEW, GV_FAILED, _widgetID);
  SendMessage(_msgObject, &message);
}

//-------------------------------------------------------------------------

//   Public Routines

// DisableInterpreter:
// Stop any interpreter and disable new ones from starting.

void OXGhostView::DisableInterpreter() {
  _disable_start = True;
  StopInterpreter();
}

// EnableInterpreter:
// Allow an interpreter to start and start one.

void OXGhostView::EnableInterpreter() {
  _disable_start = False;
  StartInterpreter();
}

// SendPS:
//   Queue a portion of a PostScript file for output to ghostscript.
//   fp: FILE * of the file in question.  NOTE: if you have several
//   Ghostview widgets reading from the same file, you must open
//   a unique FILE * for each widget.
//   SendPS does not actually send the PostScript, it merely queues it
//   for output.
//   begin: position in file (returned from ftell()) to start.
//   len:   number of bytes to write.

//   If an interpreter is not running, nothing is queued and
//   False is returned.

int OXGhostView::SendPS(FILE *fp, long begin, unsigned int len, int close) {
  SRecordList *ps_new;

  if (_interpreter_input < 0) return False;
  ps_new = new SRecordList;
  ps_new->fp = fp;
  ps_new->begin = begin;
  ps_new->len = len;
  ps_new->seek_needed = True;
  ps_new->close = close;
  ps_new->next = NULL;

  if (_input_buffer == NULL) {
    _input_buffer = new char [GV_BUFSIZ];
  }

  if (_ps_input == NULL) {
    _input_buffer_ptr = _input_buffer;
    _bytes_left = len;
    _buffer_bytes_left = 0;
    _ps_input = ps_new;
    _interpreter_input_fh = new OFileHandler(this, _interpreter_input,
			                     XCM_WRITABLE); // _Input()
  } else {
    SRecordList *p = _ps_input;
    while (p->next != NULL) p = p->next;
    p->next = ps_new;
  }

  return True;
}

// NextPage:
//   Tell ghostscript to start the next page.
//   Returns False if ghostscript is not running, or not ready to start
//   another page.
//   If another page is started.  Sets the busy flag and cursor.

int OXGhostView::NextPage() {
  XEvent event;

  if (_interpreter_pid < 0) return False;
  if (_mwin == None) return False;

  if (!_busy) {
    _busy = True;
    XDefineCursor(GetDisplay(), _id, _busy_cursor);

    event.xclient.type = ClientMessage;
    event.xclient.display = GetDisplay();
    event.xclient.window = _mwin;
    event.xclient.message_type = ATOM_NEXT;
    event.xclient.format = 32;
    XSendEvent(GetDisplay(), _mwin, False, 0, &event);
    XFlush(GetDisplay());   // And push it out
    return True;
  } else {
    return False;
  }
}
