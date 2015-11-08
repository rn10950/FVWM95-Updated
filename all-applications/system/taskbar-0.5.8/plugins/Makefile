SHELL=/bin/sh

subdirs = clock \
	mailcheck \
	icon \
	lockfile \
	cpuload \
	battstat

all:
	@for i in ${subdirs}; do \
		echo Making all in $$i ; \
		(cd $$i; ${MAKE} all) ; \
	done

install:
	@for i in ${subdirs}; do \
		echo Installing in $$i ; \
		(cd $$i; ${MAKE} install) ; \
	done

depend:
	@for i in ${subdirs}; do \
		echo Depending in $$i ; \
		(cd $$i; ${MAKE} depend) ; \
	done

clean:
	@for i in ${subdirs}; do \
		echo Cleaning in $$i ; \
		(cd $$i; ${MAKE} clean) ; \
	done

distclean:
	@for i in ${subdirs}; do \
		echo Distribution Cleaning in $$i ; \
		(cd $$i; ${MAKE} distclean) ; \
	done
