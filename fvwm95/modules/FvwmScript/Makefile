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
LOCAL_LIBRARY = ./Widgets/libWidgets.a
LIBS=-L${top_srcdir}/libs ${x_libraries} ${FVWM_LIBRARY} ${XPM_LIBRARY} ${XEXT_LIBRARY} ${X_LIBRARY} ${LOCAL_LIBRARY}

EXTRA_DEFINES=-DXPM -DSHAPE
CC=gcc
INCLUDES=-I${top_srcdir}/include ${x_includes}
CFLAGS=${INCLUDES} -O ${EXTRA_DEFINES} 
MAKEDEPEND=makedepend
INSTALL=/usr/bin/install -c
INSTALL_PROGRAM=${INSTALL}
INSTALL_DATA=${INSTALL} -m 644
RM=rm -f

subdirs=Widgets

PROG=FvwmScript

SRCS = libyywrap.c lex.yy.c y.tab.c Instructions.c FvwmScript.c

OBJS = libyywrap.o lex.yy.o y.tab.o Instructions.o FvwmScript.o



all: widgets ${PROG}

depend:
	${MAKEDEPEND} ${CFLAGS} ${SRCS}

install: all
	${INSTALL_PROGRAM} ${PROG} ${fvwm_dir}
	${INSTALL_DATA} ${PROG}.man ${man_dir}/${PROG}.1 
	@cd Scripts; ${MAKE} install

uninstall: 
	${RM} ${fvwm_dir}/${PROG}
	${RM} ${man_dir}/${PROG}.1
	@cd Scripts; ${MAKE} uninstall

clean:
	${RM} ${PROG} ${OBJS} *~ core *.bak
	@for i in ${subdirs}; do \
		echo Cleaning in $$i ; \
		(cd $$i; ${MAKE} clean) \
	done

distclean: clean


widgets:
	@for i in ${subdirs}; do \
		echo Making all in $$i ; \
		(cd $$i; ${MAKE} all) \
	done

${PROG}: ${OBJS}
	${CC} -o ${PROG} ${OBJS} ${LIBS}

