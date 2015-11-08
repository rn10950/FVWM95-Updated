SHELL=/bin/sh

CXX=c++
CXXFLAGS=`xc-config --cflags` -g
LIBS=`xc-config --libs`
LDFLAGS=
MAKEDEPEND=@MAKEDEPEND@
INSTALL=install -c
BINDIR=`xc-config --exec-prefix`/bin

# Rules for compiling .cc sources
.SUFFIX: .cc .o
.cc.o:
	$(CXX) $(CXXFLAGS) -c $<

SRCS = OXGifFrame.cc main.cc

OBJS = $(SRCS:.cc=.o)

PROG = gifview

all: ${PROG}

depend:
	${MAKEDEPEND} ${CXXFLAGS} ${SRCS}

${PROG}: ${OBJS}
	$(CXX) -o $@ ${OBJS} $(LDFLAGS) $(LIBS) $(EXTRALIBS)
	strip $@

install: ${PROG}
	mkdir -p ${BINDIR}
	${INSTALL} ${PROG} ${BINDIR}

clean:
	rm -f ${OBJS} $(PROG) *~ core *.bak

distclean: clean

test: ${PROG}
	@echo Testing ${PROG}...
	${PROG} smile.gif
