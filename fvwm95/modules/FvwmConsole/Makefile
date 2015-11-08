# Generated automatically from Makefile.in by configure.
#
SHELL=/bin/sh



top_srcdir=../..
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
XPM_LIBRARY=-lXpm
XEXT_LIBRARY=-lXext
FVWM_LIBRARY=-l${fvwm_name}
LIBS=-L${top_srcdir}/libs ${x_libraries} ${FVWM_LIBRARY} ${XPM_LIBRARY} ${XEXT_LIBRARY} ${X_LIBRARY} 

PATH_DEFINES = -DFVWMDIR=\"${fvwm_dir}\"
EXTRA_DEFINES=${PATH_DEFINES} -DXPM -DSHAPE

CC=gcc
INCLUDES=-I${top_srcdir}/include ${x_includes}
CFLAGS=${INCLUDES} -O ${EXTRA_DEFINES} 
MAKEDEPEND=makedepend
INSTALL=/usr/bin/install -c
INSTALL_PROGRAM=${INSTALL}
INSTALL_DATA=${INSTALL} -m 644
RM=rm -f

PROG=FvwmConsole
PROGC=FvwmConsoleC

SRCS=FvwmConsole.c FvwmConsoleC.c getline.c
OBJS=FvwmConsole.o
OBJSC=FvwmConsoleC.o getline.o


all: ${PROG} ${PROGC}

depend:
	${MAKEDEPEND} ${CFLAGS} ${SRCS}

install: all
	${INSTALL_PROGRAM} ${PROG} ${fvwm_dir}
	${INSTALL_PROGRAM} ${PROGC} ${fvwm_dir}
	${INSTALL_DATA} ${PROG}.man ${man_dir}/${PROG}.1 

uninstall: 
	${RM} ${fvwm_dir}/${PROG}
	${RM} ${fvwm_dir}/${PROGC}
	${RM} ${man_dir}/${PROG}.1

clean:
	${RM} ${PROG} ${OBJS} *~ core *.bak

distclean: clean


${PROG}: ${OBJS}
	${CC} -o ${PROG} ${OBJS} ${LIBS}

${PROGC}: ${OBJSC}
	${CC} -o ${PROGC} ${OBJSC} ${LIBS}

