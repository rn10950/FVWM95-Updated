SHELL=/bin/sh

CXX=c++
CXXFLAGS=`xc-config --cflags` -g 
LIBS=`xc-config --libs`
EXTRALIBS=
LDFLAGS= 
MAKEDEPEND=@MAKEDEPEND@
INSTALL=install -c
BINDIR=`xc-config --exec-prefix`/bin
RM=rm -f

SRCS =	OFolder.cc \
	OMail.cc \
	OMailList.cc \
	OMimeAtt.cc \
	OSession.cc \
	OSmtp.cc \
	OText.cc \
	OXBodyView.cc \
	OXComposer.cc \
	OXFolderDelete.cc \
	OXFolderProp.cc \
	OXMailList.cc \
	OXMoveMail.cc \
	OXPreferences.cc \
	OXPrintBox.cc \
	OXSearchBox.cc \
	OXViewSrc.cc \
	main.cc

OBJS = $(SRCS:.cc=.o)

PROG = nrxmail

.SUFFIX: .cc .o
.cc.o:
	$(CXX) $(CXXFLAGS) -c $<

all: ${PROG}

depend:
	${MAKEDEPEND} ${CXXFLAGS} ${SRCS}

install: ${PROG}
	mkdir -p ${BINDIR}
	${INSTALL} ${PROG} ${BINDIR}

clean:
	${RM} ${OBJS} $(PROG) *~ core *.bak

distclean: clean

${PROG}: ${OBJS}
	$(CXX) -o $@ ${OBJS} $(LDFLAGS) $(LIBS) $(EXTRALIBS)
	strip $@

test: ${PROG}
	@echo Testing ${PROG}...
	${PROG}
