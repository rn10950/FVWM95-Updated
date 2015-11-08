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

ob = ./.onyx/

onyxo = ${ob}OnyxBase.o \
	${ob}OnyxButton.o \
	${ob}OnyxCheckBox.o \
	${ob}OnyxColorDialog.o \
	${ob}OnyxComboBox.o \
	${ob}OnyxDraw.o \
	${ob}OnyxFileDialog.o \
	${ob}OnyxFontDialog.o \
	${ob}OnyxHScrollBar.o \
	${ob}OnyxIcon.o \
	${ob}OnyxLabel.o \
	${ob}OnyxListBox.o \
	${ob}OnyxMenu.o \
	${ob}OnyxMenuBar.o \
	${ob}OnyxMenuItem.o \
	${ob}OnyxMsgBox.o \
	${ob}OnyxProgressBar.o \
	${ob}OnyxTextBox.o \
	${ob}OnyxTextCanvas.o \
	${ob}OnyxTimer.o \
	${ob}OnyxTree.o \
	${ob}OnyxRadioButton.o \
	${ob}OnyxScreen.o \
	${ob}OnyxSpinButton.o \
	${ob}OnyxVScrollBar.o \
	${ob}OnyxWindow.o

SRCS =	main.cc \
	AboutWindow.cc \
	Application.cc \
	CCHelpWindow.cc \
	CustomControlsWindow.cc \
	MainWindow.cc \
	MakeWindow.cc \
	MenuEditor.cc \
	MenuEditor2.cc \
	NewProjectWindow.cc \
	OptionsWindow.cc \
	ProjectExplorer.cc \
	PropertiesWindow.cc \
	SplashScreen.cc \
	Toolbox.cc 

OBJS = $(SRCS:.cc=.o)

PROG = OnyX

# Rules for compiling .cc sources

.SUFFIX: .cc .o
.cc.o:
	@echo Compiling $<
	@$(CXX) $(CXXFLAGS) -c $<

all: complete

libOnyX.a: base

${PROG}: ${OBJS} libOnyX.a
	@echo Linking all objects
	@$(CXX) -o $@ ${OBJS} ${onyxo} $(LDFLAGS) $(LIBS) $(EXTRALIBS)
	@echo Stripping $@
	@strip $@
	@echo $@ is ready.

strip: ${PROG}
	@echo Stripping ${PROG}
	@strip ${PROG}
	@echo ${PROG} is ready.

complete: base ${PROG}

base: 
	@for i in ${ob}; do \
		(cd $$i; ${MAKE} all); \
	done

depend:
	${MAKEDEPEND} ${CXXFLAGS} ${SRCS}

install: 

clean:
	${RM} ${OBJS} $(PROG) core *.bak *~ setup.log temp.log

baseclean:
	@for i in ${ob}; do \
		(cd $$i; ${MAKE} clean); \
	done

completeclean: baseclean clean

distclean: baseclean

allclean: completeclean
