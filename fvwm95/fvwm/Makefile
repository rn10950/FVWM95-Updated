# Generated automatically from Makefile.in by configure.
#
SHELL=/bin/sh



top_srcdir=..
srcdir=.
prefix=/usr/local
exec_prefix=${prefix}
bin_dir=${prefix}/bin
x_includes=-INONE
x_libraries=-LNONE
fvwm_icons=NONE/X11/bitmaps:NONE/X11/pixmaps:NONE/icons
fvwm_rc=.fvwm95rc
fvwm_name=fvwm95
#
# This is where the modules will be installed
fvwm_dir=/usr/local/lib/X11/fvwm95
man_dir=/usr/local/man/man1

#
# Libs
X_LIBRARY=-lX11 
XEXT_LIBRARY=-lXext
XPM_LIBRARY=-lXpm
FVWM_LIBRARY=-l${fvwm_name}
LIBS=-L${top_srcdir}/libs ${x_libraries} ${FVWM_LIBRARY} ${XPM_LIBRARY} ${XEXT_LIBRARY} ${X_LIBRARY}

PATH_DEFINES=-DFVWM_ICONDIR=\"${fvwm_icons}\" \
               -DFVWMDIR=\"${fvwm_dir}\" \
               -DFVWMRC=\"${fvwm_rc}\"

EXTRA_DEFINES=-DXPM -DSHAPE ${PATH_DEFINES}

CC=gcc
INCLUDES=-I${top_srcdir}/include ${x_includes}
CFLAGS=${INCLUDES} -O ${EXTRA_DEFINES} 
MAKEDEPEND=makedepend
INSTALL=/usr/bin/install -c
INSTALL_PROGRAM=${INSTALL}
INSTALL_DATA=${INSTALL} -m 644
RM=rm -f

PROG=${fvwm_name}

SRCS=add_window.c \
       bindings.c \
       borders.c \
       builtins.c \
       colormaps.c \
       colors.c \
       complex.c \
       decorations.c \
       events.c \
       focus.c \
       functions.c \
       fvwm.c \
       icons.c \
       menus.c \
       misc.c \
       module.c \
       move.c \
       placement.c \
       read.c \
       resize.c \
       style.c \
       virtual.c \
       windows.c
 
OBJS=add_window.o \
       bindings.o \
       borders.o \
       builtins.o \
       colormaps.o \
       colors.o \
       complex.o \
       decorations.o \
       events.o \
       focus.o \
       functions.o \
       fvwm.o \
       icons.o \
       menus.o \
       misc.o \
       module.o \
       move.o \
       placement.o \
       read.o \
       resize.o \
       style.o \
       virtual.o \
       windows.o

all: ${PROG}

depend:
	${MAKEDEPEND} ${CFLAGS} ${SRCS}

install: all
	@mkdir -p ${bin_dir}
	${INSTALL_PROGRAM} ${PROG} ${bin_dir}
	@mkdir -p ${man_dir}
	${INSTALL_DATA} ${PROG}.man ${man_dir}/${PROG}.1 

uninstall: 
	${RM} ${bin_dir}/${PROG}
	${RM} ${man_dir}/${PROG}.1

clean:
	${RM} ${PROG} ${OBJS} *~ core *.bak

distclean: clean


${PROG}: ${OBJS}
	${CC} -o ${PROG} ${OBJS} ${LIBS}

#
