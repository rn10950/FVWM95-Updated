/****************************************************************************
 * This is an all new program to set the root window to an Xpm pixmap.
 * Copyright 1993, Rob Nation 
 * You may use this file for anything you want, as long as the copyright
 * is kept intact. No guarantees of any sort are made in any way regarding
 * this program or anything related to it.
 ****************************************************************************/

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/xpm.h>

Display *dpy;
int screen;
Window root;

void SetRootPixmap(char *fname);

int main(int argc, char **argv) {
  char *display_name = NULL;

  if (argc != 2) {
    fprintf(stderr, "Usage: %s xpmfile\n", argv[0]);
    exit(1);
  }

  dpy = XOpenDisplay(display_name);
  if (!dpy) {
    fprintf(stderr, "Xpmroot:  unable to open display '%s'\n",
	    XDisplayName(display_name));
    exit(2);
  }

  screen = DefaultScreen(dpy);
  root = RootWindow(dpy, screen);

  SetRootPixmap(argv[1]);

  XSetCloseDownMode(dpy, RetainPermanent);
  XCloseDisplay(dpy);

  return 0;
}


void SetRootPixmap(char *fname) {
  XWindowAttributes root_attr;
  XpmAttributes xpm_attributes;
  Pixmap shapeMask, rootXpm;
  int retc;
  Atom prop, type;
  int format;
  unsigned long length, after;
  unsigned char *data;

  XGetWindowAttributes(dpy, root, &root_attr);
  xpm_attributes.colormap = root_attr.colormap;
  xpm_attributes.valuemask = XpmSize | XpmReturnPixels | XpmColormap;

  if ((retc = XpmReadFileToPixmap(dpy, root, fname,
                                  &rootXpm, &shapeMask,
                                  &xpm_attributes)) != XpmSuccess) {
    if (retc == XpmOpenFailed)
      fprintf(stderr, "Couldn't open pixmap file\n");
    else if (retc == XpmColorFailed)
      fprintf(stderr, "Couldn't allocate required colors\n");
    else if (retc == XpmFileInvalid)
      fprintf(stderr, "Invalid Format for an Xpm File\n");
    else if (retc == XpmColorError)
      fprintf(stderr, "Invalid Color specified in Xpm FIle\n");
    else if (retc == XpmNoMemory)
      fprintf(stderr, "Insufficient Memory\n");
    exit(1);
  }

  XSetWindowBackgroundPixmap(dpy, root, rootXpm);
  XClearWindow(dpy, root);

  prop = XInternAtom(dpy, "_XSETROOT_ID", False);

  XGetWindowProperty(dpy, root, prop, 0L, 1L, True, AnyPropertyType,
                     &type, &format, &length, &after, &data);

  if ((type == XA_PIXMAP) && (format == 32) && (length == 1)
      && (after == 0)) XKillClient(dpy, *((Pixmap *) data));

  XChangeProperty(dpy, root, prop, XA_PIXMAP, 32, PropModeReplace,
                  (unsigned char *) &rootXpm, 1);
}
