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
	URL.cc \
	OXExplorer.cc \
	OXDirectoryTree.cc \
	OXCopyBox.cc \
	OXOptions.cc \
	OXEditMime.cc \
	OXIconSelDialog.cc \
	OXOpenWith.cc \
	ORecycledFiles.cc

OBJS = $(SRCS:.cc=.o)

PROG = explorer

all: ${PROG}

${PROG}: ${OBJS}
	$(CXX) -o $@ ${OBJS} $(LDFLAGS) $(LIBS) $(EXTRALIBS)
	strip $@

install: ${PROG}
	mkdir -p ${BINDIR}
	${INSTALL} ${PROG} ${BINDIR}
#	${INSTALL} -m 644 mime.types `xc-config --prefix`/xclass/etc

clean:
	rm -f ${OBJS} $(PROG) *~ core *.bak

distclean: clean

test: ${PROG}
	@echo Testing ${PROG}...
	${PROG}
