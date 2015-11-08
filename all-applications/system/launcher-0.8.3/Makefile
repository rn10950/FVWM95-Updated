SHELL=/bin/sh

CXX=c++
CXXFLAGS=`xc-config --cflags` -g
LIBS=`xc-config --libs`
LDFLAGS=
INSTALL=install -c
BINDIR=`xc-config --exec-prefix`/bin

# Rules for compiling .cc sources
.SUFFIX: .cc .o
.cc.o:
	$(CXX) $(CXXFLAGS) -c $<

SRCS = main.cc \
	OXLaunchButton.cc \
	OXRenameDlg.cc \
	OXEditDlg.cc \
	OXIconSelDlg.cc

OBJS = $(SRCS:.cc=.o)

PROG = xclauncher

all: ${PROG}

${PROG}: ${OBJS}
	$(CXX) -o $@ ${OBJS} $(LDFLAGS) $(LIBS) $(EXTRALIBS)
	strip $@

install: ${PROG}
	mkdir -p ${BINDIR}
	${INSTALL} ${PROG} ${BINDIR}

clean:
	rm -f ${OBJS} $(PROG) *~ core *.bak

test: ${PROG}
	@echo Testing ${PROG}...
	${PROG}
