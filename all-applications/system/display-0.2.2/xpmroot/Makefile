# Generated automatically from Makefile.in by configure.
#
SHELL=/bin/sh



top_srcdir=..
srcdir=.
prefix=/usr/local
exec_prefix=${prefix}
bin_dir=${prefix}/bin
x_includes=-I/usr/X11R6/include
x_libraries=-L/usr/X11R6/lib

XPM_LIBRARY=-lXpm
X_LIBRARY=-lX11 

LIBS=${x_libraries} ${XPM_LIBRARY} ${X_LIBRARY}

CC=gcc
INCLUDES=-I${top_srcdir}/include ${x_includes}
CFLAGS=${INCLUDES} -O ${PATH_DEFINES} 
MAKEDEPEND=makedepend
INSTALL=/usr/bin/ginstall -c
INSTALL_PROGRAM=${INSTALL}
RM=rm -f

PROG=xpmroot

SRCS = xpmroot.c
OBJS = xpmroot.o

all: ${PROG}

depend:
	${MAKEDEPEND} ${CFLAGS} ${SRCS}

install: all
	${INSTALL_PROGRAM} ${PROG} ${bin_dir}

clean:
	${RM} ${PROG} ${OBJS} *~ core *.bak

distclean: clean

${PROG}: ${OBJS}
	${CC} -o ${PROG} ${OBJS} ${LIBS}
