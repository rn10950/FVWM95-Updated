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
LIBS=-L${top_srcdir}/libs ${x_libraries} ${FVWM_LIBRARY} ${XPM_LIBRARY} ${XEXT_LIBRARY} ${X_LIBRARY} -ldl

EXTRA_DEFINES=-DXPM -DSHAPE
CC=gcc -rdynamic -pipe
INCLUDES=-I${top_srcdir}/include ${x_includes}
CFLAGS=${INCLUDES} -O ${EXTRA_DEFINES}
MAKEDEPEND=makedepend
INSTALL=/usr/bin/install -c
INSTALL_PROGRAM=${INSTALL}
INSTALL_DATA=${INSTALL} -m 644
RM=rm -f


PROG=FvwmTaskBar

SRCS=FvwmTaskBar.c \
	List.c \
	ButtonArray.c \
	Mallocs.c \
	Colors.c \
	Goodies.c \
	Start.c \
	GoodyLoadable.c

OBJS=FvwmTaskBar.o \
	List.o \
	ButtonArray.o \
	Mallocs.o \
	Colors.o \
	Goodies.o \
	Start.o \
	GoodyLoadable.o

MODULES_SRC=CheckLockModule.c \
	GoodyModule.c \
	ShowDateModule.c \
	MailCheckModule.c \
	BatStatModule.c

MODULES=CheckLockModule.so \
	GoodyModule.so \
	ShowDateModule.so \
	MailCheckModule.so \
	BatStatModule.so


all: ${PROG} modules

depend:
	${MAKEDEPEND} ${CFLAGS} ${SRCS}

install: all
	${INSTALL_PROGRAM} ${PROG} ${fvwm_dir}
	${INSTALL_DATA} ${PROG}.man ${man_dir}/${PROG}.1 
	@mkdir -p ${fvwm_dir}/plugins
	@for i in ${MODULES}; do \
		echo Installing $$i ; \
		(${INSTALL_PROGRAM} $$i  ${fvwm_dir}/plugins/ ) \
	done
uninstall: 
	${RM} ${fvwm_dir}/${PROG}
	${RM} ${man_dir}/${PROG}.1
	@for i in ${MODULES}; do \
		echo Removing $$i ; \
		(${RM} ${fvwm_dir}/plugins/$$i) \
	done

clean:
	${RM} ${PROG} ${OBJS} *~ core *.bak *.so

distclean: clean

modules: ${MODULES}

CheckLockModule.so: CheckLockModule.c GoodyLoadable.h
	${CC} ${INCLUDES} -shared CheckLockModule.c -o CheckLockModule.so

GoodyModule.so: GoodyModule.c GoodyLoadable.h
	${CC} ${INCLUDES} -shared GoodyModule.c -o GoodyModule.so

ShowDateModule.so: ShowDateModule.c GoodyLoadable.h
	${CC} ${INCLUDES} -shared ShowDateModule.c -o ShowDateModule.so

MailCheckModule.so: MailCheckModule.c GoodyLoadable.h
	${CC} ${INCLUDES} -shared MailCheckModule.c -o MailCheckModule.so

BatStatModule.so: BatStatModule.c GoodyLoadable.h
		  ${CC} ${INCLUDES} -shared BatStatModule.c -o BatStatModule.so

${PROG}: ${OBJS}
	${CC} -o ${PROG} ${OBJS} ${LIBS}
