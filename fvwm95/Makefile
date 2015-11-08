# Generated automatically from Makefile.in by configure.
#
# Makefile for FVWM95
#
# Copyright (C) 1996 Peter G. Williams
#
#
SHELL=/bin/sh



prefix=/usr/local
exec_prefix=${prefix}
subdirs=libs fvwm modules utils sample.fvwmrc xpmroot pixmaps mini-icons docs

RM=rm -f

CFLAGS = -O

all:
	@for i in ${subdirs}; do \
		echo Making all in $$i ; \
		(cd $$i; ${MAKE} all) \
	done

install:
	@for i in ${subdirs}; do \
		echo Installing in $$i ; \
		(cd $$i; ${MAKE} install) \
	done

uninstall:
	@for i in ${subdirs}; do \
		echo Uninstalling in $$i ; \
		(cd $$i; ${MAKE} uninstall) \
	done

depend:
	@for i in ${subdirs}; do \
		echo Depending in $$i ; \
		(cd $$i; ${MAKE} depend) \
	done

clean:
	@for i in ${subdirs}; do \
		echo Cleaning in $$i ; \
		(cd $$i; ${MAKE} clean) \
	done

distclean:
	@for i in ${subdirs}; do \
		echo Distribution Cleaning in $$i ; \
		(cd $$i; ${MAKE} distclean) \
	done
	${RM} config.cache config.status config.log sample.fvwmrc/system.fvwm95rc
	find . -name Makefile -exec rm {} \; -print
