
SHELL=/bin/sh



top_srcdir=..
srcdir=.
prefix=/usr/local
exec_prefix=${prefix}
x_includes=
x_libraries= -lXpm -lXext -lX11

CXX=g++
INCLUDES=-I${top_srcdir}/include ${x_includes}
CXXFLAGS=${INCLUDES}  -g
LIBS=-L${top_srcdir}/lib/libxclass -lxclass ${x_libraries}
EXTRALIBS=-lXpm -lXext -lX11
LDFLAGS=
MAKEDEPEND=@MAKEDEPEND@

SRCS = btest.cc \
	colorsel.cc \
	combotest.cc \
	docktest.cc \
	exception.cc \
	exec.cc \
	favordlg.cc \
	flies.cc \
	ftest.cc \
	gltest.cc \
	helptest.cc \
	htmltest.cc \
	listview.cc \
	mditest.cc \
	progbar.cc \
	resizer.cc \
	root_flies.cc \
	root_trails.cc \
	spinnertest.cc \
	shutter.cc \
	shutter1.cc \
	test4.cc \
	textedit.cc \
	trails.cc \
	wintest.cc \
	xdnd.cc

OBJS = $(SRCS:%.cc=%.o)

PROG = $(SRCS:%.cc=%)


include ${top_srcdir}/config/C++.rules.in


all: ${PROG}

shared: all

depend:
	${MAKEDEPEND} ${CXXFLAGS} ${SRCS}

install_shared: install

install: 

clean:
	${RM} ${OBJS} $(PROG) *~ core *.bak

distclean: clean

${PROG}: ${OBJS}
	$(CXX) -o $@ $@.o $(LDFLAGS) $(LIBS) $(EXTRALIBS)
	strip $@

