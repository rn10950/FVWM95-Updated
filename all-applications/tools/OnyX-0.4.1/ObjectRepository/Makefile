# /**************************************************************************
# 
#         This file is part of OnyX, a visual development environment
#    using the xclass toolkit. Copyright (C) 1997, 1998 Frank Hall.
# 
#         This program is free software; you can redistribute it and/or
#    modify it under the terms of the GNU General Public License as published 
#    by the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
# 
#         This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
#    Public License for more details.
# 
#         You should have received a copy of the GNU General Public License
#    along with this program; if not, write to the Free Software Foundation,
#    Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
# 
# **************************************************************************/

SHELL=/bin/sh

prefix=/usr/local
exec_prefix=${prefix}

CXX=c++
CXXFLAGS=`xc-config --cflags` -g
LIBS=`xc-config --libs`
EXTRALIBS=
LDFLAGS= 
MAKEDEPEND=@MAKEDEPEND@


SRCS =	OnyxBase.cc \
	OnyxButton.cc \
	OnyxCheckBox.cc \
	OnyxColorDialog.cc \
	OnyxComboBox.cc \
	OnyxDraw.cc \
	OnyxFileDialog.cc \
	OnyxFontDialog.cc \
	OnyxHScrollBar.cc \
	OnyxIcon.cc \
	OnyxLabel.cc \
	OnyxListBox.cc \
	OnyxMenu.cc \
	OnyxMenuBar.cc \
	OnyxMenuItem.cc \
	OnyxMsgBox.cc \
	OnyxProgressBar.cc \
	OnyxRadioButton.cc \
	OnyxScreen.cc \
	OnyxSpinButton.cc \
	OnyxTextBox.cc \
	OnyxTextCanvas.cc \
	OnyxTimer.cc \
	OnyxTree.cc \
	OnyxVScrollBar.cc \
	OnyxWindow.cc

OBJS = $(SRCS:.cc=.o)


# Rules for compiling .cc sources

.SUFFIX: .cc .o
.cc.o:
	@echo Compiling base object $<
	@$(CXX) $(CXXFLAGS) -c $<


all: onyx

depend:
	${MAKEDEPEND} ${CXXFLAGS} ${SRCS}

install: 

clean:
	${RM} ${OBJS} libOnyX.a core *.bak *~

distclean: clean

onyx: libOnyX.a ${OBJS}

libOnyX.a: ${OBJS}
	@echo Creating library $@
	@ar cr $@ ${OBJS}
	@ranlib $@

black:
	$(CXX) -o $@ ${OBJS} $(LDFLAGS) $(LIBS) $(EXTRALIBS)
