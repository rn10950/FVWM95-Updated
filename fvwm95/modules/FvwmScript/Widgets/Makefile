# Generated automatically from Makefile.in by configure.
#
SHELL=/bin/sh



top_srcdir=../../..
srcdir=.
prefix=/usr/local
exec_prefix=${prefix}
lib_dir=${prefix}/lib
x_includes=-INONE
x_libraries=-LNONE
fvwm_icons=NONE/X11/bitmaps:NONE/X11/pixmaps:NONE/icons
fvwm_rc=.fvwm95rc
fvwm_name=fvwm95

EXTRA_DEFINES=-DXPM -DSHAPE
CC=gcc
INCLUDES=-I${top_srcdir}/include ${x_includes}
CFLAGS=${INCLUDES} -O ${EXTRA_DEFINES}
MAKEDEPEND=makedepend
INSTALL=/usr/bin/install -c
INSTALL_DATA=${INSTALL} -m 644
RM=rm -f
AR=ar cr
RANLIB=ranlib



SRCS = \
	Tools.c \
	CheckBox.c \
	HScrollBar.c \
	VScrollBar.c \
	ItemDraw.c \
	PushButton.c \
	RadioButton.c \
	TextField.c \
	PopupMenu.c \
	Rectangle.c \
	MiniScroll.c \
	Swallow.c \
	HDipstick.c \
	VDipstick.c \
	List.c \
	Menu.c \
	Widget.c

OBJS = \
	Tools.o \
	CheckBox.o \
	HScrollBar.o \
	VScrollBar.o \
	ItemDraw.o \
	PushButton.o \
	RadioButton.o \
	TextField.o \
	PopupMenu.o \
	Rectangle.o \
	MiniScroll.o \
	Swallow.o \
	HDipstick.o \
	VDipstick.o \
	List.o \
	Menu.o \
	Widget.o

all:: libWidgets.a

libWidgets.a: $(OBJS) 
	$(RM) $@
	$(AR) $@ $(OBJS)
	$(RANLIB) $@


clean:
	${RM}  ${OBJS} *~ core *.bak *.a

distclean: clean

