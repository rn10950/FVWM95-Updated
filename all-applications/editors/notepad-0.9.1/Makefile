SHELL=/bin/sh

CXX=c++
CXXFLAGS=`xc-config --cflags` -g
LIBS=`xc-config --libs`
EXTRALIBS=
LDFLAGS= 
MAKEDEPEND=@MAKEDEPEND@
INSTALL=install -c
BINDIR=`xc-config --exec-prefix`/bin
HELPDIR=`xc-config --prefix`/xclass/docs/notepad

SRCS = main.cc \
	OXDialogs.cc

OBJS = $(SRCS:.cc=.o)

PROG = notepad

.SUFFIX: .cc .o
.cc.o:
	$(CXX) $(CXXFLAGS) -c $<

all: ${PROG}

depend:
	${MAKEDEPEND} ${CXXFLAGS} ${SRCS}

install: ${PROG}
	mkdir -p ${BINDIR}
	${INSTALL} ${PROG} ${BINDIR}
	mkdir -p ${HELPDIR}
	${INSTALL} -m 0644 help/* ${HELPDIR}

install_user: ${PROG}
	mkdir -p ${HOME}/.xclass/bin
	${INSTALL} ${PROG} ${HOME}/.xclass/bin
	mkdir -p ${HOME}/.xclass/docs/notepad
	${INSTALL} -m 0644 help/* ${HOME}/.xclass/docs/notepad

clean:
	${RM} ${OBJS} $(PROG) *~ core *.bak

distclean: clean

${PROG}: ${OBJS}
	$(CXX) -o $@ ${OBJS} $(LDFLAGS) $(LIBS) $(EXTRALIBS)
	strip $@

test: ${PROG}
	@echo Testing ${PROG}...
	${PROG}
