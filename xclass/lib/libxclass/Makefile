#
# Makefile.in for the XClass library
#

top_srcdir=../..
srcdir=.
prefix=/usr/local
exec_prefix=${prefix}
lib_dir=${prefix}/lib
x_includes=
x_libraries= -lXpm -lXext -lX11

lib_name=xclass

CXX=g++
INCLUDES=-I${top_srcdir}/include ${x_includes}
DEFINES=-DOX_DEFAULT_POOL=\"/usr/local/xclass-icons\" \
	-DOX_DEFAULT_ROOT=\"/usr/local/xclass\"
CXXFLAGS=${INCLUDES} ${DEFINES}  -g
EXTRALIBS=${x_libraries}
RANLIB=ranlib
AR=ar
INSTALL=/usr/bin/install -c
INSTALL_DATA=${INSTALL} -m 644

STATIC_LIB=lib${lib_name}.a
SHARED_LIB=lib${lib_name}.so.0.9.2
#SO_NAME=lib${lib_name}.so.0.9
SO_NAME=lib${lib_name}.so.0.9.2

SRCS = O2ColumnsLayout.cc \
	OColor.cc \
	ODNDmanager.cc \
	OExec.cc \
	OFileHandler.cc \
	OFocusManager.cc \
	OGC.cc \
	OGifImage.cc \
	OHashTable.cc \
	OHtmlElement.cc \
	OHtmlTokenMap.cc \
	OHtmlUri.cc \
	OIdleHandler.cc \
	OIniFile.cc \
	OLayout.cc \
	OListViewLayout.cc \
	OMatrixLayout.cc \
	OMimeTypes.cc \
	OPicture.cc \
	OQuaternion.cc \
	OResourcePool.cc \
	ORowColumnLayout.cc \
	OSelectedPicture.cc \
	OString.cc \
	OTextBuffer.cc \
	OTimer.cc \
	OTrackball.cc \
	OVector3D.cc \
	OXAboutDialog.cc \
	OXButton.cc \
	OXCanvas.cc \
	OXCheckButton.cc \
	OXClient.cc \
	OXColorDialog.cc \
	OXColorSelect.cc \
	OXComboBox.cc \
	OXCompositeFrame.cc \
	OXDDListBox.cc \
	OXDockableFrame.cc \
	OXDragWindow.cc \
	OXFSDDListBox.cc \
	OXFileDialog.cc \
	OXFileList.cc \
	OXFont.cc \
	OXFontDialog.cc \
	OXFrame.cc \
	OXGLFrame.cc \
	OXGLView.cc \
	OXGroupFrame.cc \
	OXHelpDoc.cc \
	OXHelpPopup.cc \
	OXHelpWindow.cc \
	OXHtml.cc \
	OXHtmlDraw.cc \
	OXHtmlForm.cc \
	OXHtmlImage.cc \
	OXHtmlIndex.cc \
	OXHtmlLayout.cc \
	OXHtmlParse.cc \
	OXHtmlSizer.cc \
	OXHtmlTable.cc \
	OXIcon.cc \
	OXImageView.cc \
	OXItemView.cc \
	OXLabel.cc \
	OXListBox.cc \
	OXListTree.cc \
	OXListView.cc \
	OXMainFrame.cc \
	OXMdiDecorFrame.cc \
	OXMdiFrame.cc \
	OXMdiMainFrame.cc \
	OXMdiMenu.cc \
	OXMenu.cc \
	OXMsgBox.cc \
	OXPictureButton.cc \
	OXProgressBar.cc \
	OXPropertiesDialog.cc \
	OXRadioButton.cc \
	OXResizer.cc \
	OXRootWindow.cc \
	OXSList.cc \
	OXScrollBar.cc \
	OXSecretTextEntry.cc \
	OXShutter.cc \
	OXSlider.cc \
	OXSpinner.cc \
	OXStatusBar.cc \
	OXTab.cc \
	OXTextButton.cc \
	OXTextEdit.cc \
	OXTextEntry.cc \
	OXTip.cc \
	OXToolBar.cc \
	OXToolBarButton.cc \
	OXTransientFrame.cc \
	OXView.cc \
	OXWindow.cc \
	utils.cc

OBJS=$(SRCS:.cc=.o)

all: ${STATIC_LIB}
#all: ${STATIC_LIB} ${SHARED_LIB}

both: ${STATIC_LIB} ${SHARED_LIB}

shared: CXXFLAGS += -fPIC
shared: ${SHARED_LIB}

install: ${STATIC_LIB}
	${INSTALL_DATA} ${STATIC_LIB} ${lib_dir}

install_shared: ${SHARED_LIB}
	${INSTALL_DATA} ${SHARED_LIB} ${lib_dir}
	ldconfig

clean:
	rm -f ${OBJS} ${STATIC_LIB} ${SHARED_LIB} lib$(lib_name).so* *~ core *.bak

distclean: clean

depend: ${SRCS}
	makedepend -- ${CXXFLAGS} -- ${SRCS}

# Rules for building the static library

${STATIC_LIB}: ${OBJS}
	${AR} cr ${STATIC_LIB} ${OBJS}
	${RANLIB} ${STATIC_LIB}

# Rules for building the shared library

${SHARED_LIB}: ${OBJS}
	g++ -shared -Wl,-soname,${SO_NAME} -o ${SHARED_LIB} $(OBJS) ${EXTRALIBS}
	ln -sf $(SHARED_LIB) lib$(lib_name).so.0.9
	ln -sf $(SHARED_LIB) lib$(lib_name).so.0
	ln -sf $(SHARED_LIB) lib$(lib_name).so


include ${top_srcdir}/config/C++.rules.in

#
