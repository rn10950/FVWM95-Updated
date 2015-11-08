SHELL=/bin/sh

RGBF = "/usr/X11/lib/X11/rgb.txt"

CXX=c++
CXXFLAGS=`xc-config --cflags` -g -DRGBF=${RGBF}
LIBS=`xc-config --libs`
LDFLAGS=
MAKEDEPEND=@MAKEDEPEND@
INSTALL=install -c
BINDIR=`xc-config --exec-prefix`/bin

# Rules for compiling .cc sources
.SUFFIX: .cc .o
.cc.o:
	$(CXX) $(CXXFLAGS) -c $<

SRCS = main.cc \
	OColorTable.cc \
	OXDialogs.cc \
	OXPaintCanvas.cc \
	OXPaletteFrame.cc

OBJS = $(SRCS:.cc=.o)

PROG = xcpaint

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
	${PROG}
