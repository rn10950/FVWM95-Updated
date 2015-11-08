
SHELL=/bin/sh

subdirs = `find . -name Makefile -mindepth 2 -maxdepth 3 -printf "%h\n"`

all:
	@for i in ${subdirs}; do \
		echo Making all in $$i ; \
		(cd $$i; ${MAKE} all) ; \
	done

clean:
	@for i in ${subdirs}; do \
		echo Cleaning in $$i ; \
		(cd $$i; ${MAKE} clean) ; \
	done

install:
	@for i in ${subdirs}; do \
		echo Installing in $$i ; \
		(cd $$i; ${MAKE} install) ; \
	done

test:
	@for i in ${subdirs}; do \
		(cd $$i; ${MAKE} test) ; \
	done
