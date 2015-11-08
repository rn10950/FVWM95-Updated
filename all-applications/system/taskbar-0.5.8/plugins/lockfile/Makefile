SHELL=/bin/sh

CXX=c++
CXXFLAGS=`xc-config --cflags` -I../.. -g -rdynamic -fPIC
LIBS=`xc-config --libs`
EXTRALIBS=-ldl
LDFLAGS=-rdynamic -shared
INSTALL=install -c
BINDIR=`xc-config --prefix`/xclass/apps/taskbar

# Rules for compiling .cc sources
.SUFFIX: .cc .o
.cc.o:
	$(CXX) $(CXXFLAGS) -c $<

SRCS = OXLockfilePlugin.cc

OBJS = $(SRCS:.cc=.o)

PROG = lockfile.so

all: ${PROG}

${PROG}: ${OBJS}
	$(CXX) -o $@ ${OBJS} $(LDFLAGS) -Wl,-soname,${PROG} $(LIBS)
	strip $@

install: 
	mkdir -p ${BINDIR}
	${INSTALL} ${PROG} ${BINDIR}

clean:
	rm -f ${OBJS} $(PROG) *~ core *.bak
