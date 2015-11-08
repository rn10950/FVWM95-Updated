
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

SRCS = main.cc

OBJS = $(SRCS:.cc=.o)

PROG = example

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
	$(CXX) -o $@ $(OBJS) $(LDFLAGS) $(LIBS) $(EXTRALIBS)
	strip $@
