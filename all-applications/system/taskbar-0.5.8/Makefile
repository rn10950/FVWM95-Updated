SHELL=/bin/sh

fvwm_includes=-I./fvwm

CXX=c++
CXXFLAGS=`xc-config --cflags` -g ${fvwm_includes}
LIBS=`xc-config --libs`
EXTRALIBS=-ldl
LDFLAGS=-rdynamic
INSTALL=install -c
BINDIR=`xc-config --exec-prefix`/bin
RCDIR=`xc-config --prefix`/xclass/etc

# Rules for compiling .cc sources
.SUFFIX: .cc .o
.cc.o:
	$(CXX) $(CXXFLAGS) -c $<

SUBDIRS =

SRCS = main.cc \
	envvar.cc \
	OTButtonLayout.cc \
	OXTButton.cc \
	OXSwallowedFrame.cc \
	OXTaskBar.cc

OBJS = $(SRCS:.cc=.o)

PROG = taskbar

all: ${PROG}
	@for i in ${SUBDIRS}; do \
		echo Making all in $$i ; \
		(cd $$i; ${MAKE} all) ; \
	done

#depend:
#	${MAKEDEPEND} ${CXXFLAGS} ${SRCS}
#	@for i in ${SUBDIRS}; do \
#		echo Making dependencies in $$i ; \
#		(cd $$i; ${MAKE} depend) ; \
#	done

${PROG}: ${OBJS}
	$(CXX) -o $@ ${OBJS} $(LDFLAGS) $(LIBS) $(EXTRALIBS)
	strip $@

install: 
	mkdir -p ${BINDIR}
	${INSTALL} ${PROG} ${BINDIR}
	mkdir -p ${RCDIR}
	${INSTALL} -m 644 EXAMPLE.taskbarrc ${RCDIR}/taskbarrc
	@for i in ${SUBDIRS}; do \
		echo Installing in $$i ; \
		(cd $$i; ${MAKE} install) ; \
	done

clean:
	rm -f ${OBJS} $(PROG) *~ core *.bak
	@for i in ${SUBDIRS}; do \
		echo Cleaning in $$i ; \
		(cd $$i; ${MAKE} clean) ; \
	done
