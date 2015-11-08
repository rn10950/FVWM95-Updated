SHELL=/bin/sh

CXX=c++
CXXFLAGS=`xc-config --cflags` -g
LIBS=`xc-config --libs`
LDFLAGS=
INSTALL=install -c
BINDIR=`xc-config --exec-prefix`/bin
HELPDIR=`xc-config --prefix`/xclass/docs/rx320

# Rules for compiling .cc sources
.SUFFIX: .cc .o
.cc.o:
	$(CXX) $(CXXFLAGS) -c $<

SRCS = main.cc \
	ORX320.cc \
	OFreqRecord.cc \
	OXSlideRuler.cc \
	OXDisplayPanel.cc \
	OXSMeter.cc \
	OXTuningKnob.cc \
	OXFreqDB.cc \
	OXDialogs.cc \
	OXScan.cc

OBJS = $(SRCS:.cc=.o)

PROG = rx320

all: ${PROG}

${PROG}: ${OBJS}
	$(CXX) -o $@ ${OBJS} $(LDFLAGS) $(LIBS) $(EXTRALIBS)
	strip $@

install: ${PROG}
	mkdir -p ${BINDIR}
	${INSTALL} ${PROG} ${BINDIR}
	mkdir -p ${HELPDIR}
	${INSTALL} -m 0644 help/* ${HELPDIR}

install_user:
	mkdir -p ${HOME}/.xclass/bin
	${INSTALL} ${PROG} ${HOME}/.xclass/bin
	mkdir -p ${HOME}/.xclass/docs/rx320
	${INSTALL} -m 0644 help/* ${HOME}/.xclass/docs/rx320

clean:
	rm -f ${OBJS} $(PROG) *~ core *.bak

distclean: clean

test: ${PROG}
	@echo Testing ${PROG}...
	${PROG}
