# Generated automatically from Makefile.in by configure.
#
SHELL=/bin/sh



top_srcdir=..
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

STATIC_LIB=lib${fvwm_name}.a

SRCS=GetFdWidth.c \
       GetFont.c \
       GetLine.c \
       Picture.c \
       ReadPacket.c \
       SendInfo.c \
       SendText.c \
       SetMask.c \
       envvar.c \
       findIconFile.c \
       hostname.c \
       mygetostype.c \
       safemalloc.c \
       sleep.c \
       strutils.c \
       wild.c

OBJS=GetFdWidth.o \
       GetFont.o \
       GetLine.o \
       Picture.o \
       ReadPacket.o \
       SendInfo.o \
       SendText.o \
       SetMask.o \
       envvar.o \
       findIconFile.o \
       hostname.o \
       mygetostype.o \
       safemalloc.o \
       sleep.o \
       strutils.o \
       wild.o


all: ${STATIC_LIB}

depend:
	${MAKEDEPEND} ${CFLAGS} ${SRCS}

install: all
	mkdir -p ${lib_dir}
	${INSTALL_DATA} ${STATIC_LIB} ${lib_dir}

uninstall:
	rm -f ${lib_dir}/${STATIC_LIB}

clean:
	${RM} ${OBJS} ${STATIC_LIB} *~ core *.bak

distclean: clean

${STATIC_LIB}: ${OBJS}
	${AR} ${STATIC_LIB} ${OBJS}
	${RANLIB} ${STATIC_LIB}

#
