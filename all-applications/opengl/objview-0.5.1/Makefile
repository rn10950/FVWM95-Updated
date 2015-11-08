SHELL=/bin/sh

CXX=c++
CXXFLAGS=`xc-config --cflags` -g -I../OXGLView
LIBS=`xc-config --libs` -lGLU -lGL
LDFLAGS=

# Rules for compiling .cc sources
.SUFFIX: .cc .o
.cc.o:
	$(CXX) $(CXXFLAGS) -c $<

SRCS = main.cc \
	view.cc \
	glm.cc

OBJS = $(SRCS:.cc=.o)

PROG = objview

all: ${PROG}

${PROG}: ${OBJS}
	$(CXX) -o $@ ${OBJS} $(LDFLAGS) $(LIBS) $(EXTRALIBS)
	strip $@

install: 

clean:
	rm -f ${OBJS} $(PROG) *~ core *.bak

distclean: clean

test: ${PROG}
	@echo Testing ${PROG}...
	${PROG}
