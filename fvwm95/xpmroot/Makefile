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

man_dir=/usr/local/man/man1

XPM_LIBRARY=-lXpm
X_LIBRARY=-lX11 

LIBS=${x_libraries} ${XPM_LIBRARY} ${X_LIBRARY}

CC=gcc
INCLUDES=-I${top_srcdir}/include ${x_includes}
CFLAGS=${INCLUDES} -O ${PATH_DEFINES} 
MAKEDEPEND=makedepend
INSTALL=/usr/bin/install -c
INSTALL_PROGRAM=${INSTALL}
INSTALL_DATA=${INSTALL} -m 644
RM=rm -f

PROG=xpmroot

SRCS = xpmroot.c
OBJS = xpmroot.o

all: ${PROG}

depend:
	${MAKEDEPEND} ${CFLAGS} ${SRCS}

install: all
	${INSTALL_PROGRAM} ${PROG} ${bin_dir}
	${INSTALL_DATA} ${PROG}.man ${man_dir}/${PROG}.1 

uninstall:
	${RM} ${bin_dir}/${PROG}
	${RM} ${man_dir}/${PROG}.1

clean:
	${RM} ${PROG} ${OBJS} *~ core *.bak

distclean: clean

${PROG}: ${OBJS}
	${CC} -o ${PROG} ${OBJS} ${LIBS}
