SHELL=/bin/sh

CXX=c++
CXXFLAGS=`xc-config --cflags` -g
LIBS=`xc-config --libs`
LDFLAGS=
MAKEDEPEND=@MAKEDEPEND@

# Rules for compiling .cc sources
.SUFFIX: .cc .o
.cc.o:
	$(CXX) $(CXXFLAGS) -c $<

SRCS = btest.cc \
	colorsel.cc \
	combotest.cc \
	docktest.cc \
	exception.cc \
	exec.cc \
	favordlg.cc \
	flies.cc \
	ftest.cc \
	gltest.cc \
	helptest.cc \
	htmltest.cc \
	listview.cc \
	mditest.cc \
	progbar.cc \
	resizer.cc \
	root_flies.cc \
	root_trails.cc \
	spinnertest.cc \
	shutter.cc \
	shutter1.cc \
	test4.cc \
	textedit.cc \
	trails.cc \
	wintest.cc \
	xdnd.cc

OBJS = $(SRCS:%.cc=%.o)

PROG = $(SRCS:%.cc=%)


all: ${PROG}

depend:
	${MAKEDEPEND} ${CXXFLAGS} ${SRCS}

install: 

clean:
	rm -f ${OBJS} $(PROG) *~ core *.bak

distclean: clean

${PROG}: ${OBJS}
	$(CXX) -o $@ $@.o $(LDFLAGS) $(LIBS) $(EXTRALIBS)
	strip $@

test: ${PROG}
	@for i in ${PROG} ; do \
		echo Testing $$i ; \
		$$i ; \
	done
