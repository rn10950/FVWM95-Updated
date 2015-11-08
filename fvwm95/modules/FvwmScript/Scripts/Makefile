# Generated automatically from Makefile.in by configure.

SHELL=/bin/sh

prefix=/usr/local
fvwm_dir=/usr/local/lib/X11/fvwm95


INSTALL = /usr/bin/install -c
INSTALL_DATA = ${INSTALL} -m 644
RM=rm -f
SCRIPTS=BellSetup Buttons Date DeskSetup FileBrowser KeyboardSetup \
	PointerSetup Quit ScreenDump ScreenSetup 


all:

install:
	@mkdir -p ${fvwm_dir}/scripts
	@for i in ${SCRIPTS}; do \
		echo Installing $$i ; \
		(${INSTALL_DATA} $$i ${fvwm_dir}/scripts) \
	done

uninstall:
	@for i in ${SCRIPTS}; do \
		echo Removing $$i ; \
		(${RM} ${fvwm_dir}/scripts/$$i) \
	done

clean: 

distclean: 

