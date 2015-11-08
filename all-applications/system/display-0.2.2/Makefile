SHELL=/bin/sh

fvwm_includes=-I./fvwm

CXX=c++
CXXFLAGS=`xc-config --cflags` -g -g ${fvwm_includes}
LIBS=`xc-config --libs`
LDFLAGS=
INSTALL=install -c
BINDIR=`xc-config --exec-prefix`/bin

# Rules for compiling .cc sources
.SUFFIX: .cc .o
.cc.o:
	$(CXX) $(CXXFLAGS) -c $<

SRCS = main.cc \
	bckgnd_tab.cc \
	scrnsav_tab.cc \
	appearance_tab.cc \
	settings_tab.cc \
	scheme.cc

OBJS = $(SRCS:%.cc=%.o)

PROG = display

all: ${PROG}

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
	@echo Run ${PROG} as an fvwm95 module for best results!
	${PROG}
