SHELL=/bin/sh



prefix=/usr/local
exec_prefix=${prefix}
subdirs=include lib config doc icons test example-app

all:
	@for i in ${subdirs}; do \
		echo Making all in $$i ; \
		(cd $$i; ${MAKE} all) ; \
	done

shared:
	@for i in ${subdirs}; do \
		echo Making shared library in $$i ; \
		(cd $$i; ${MAKE} shared) ; \
	done

install:
	@for i in ${subdirs}; do \
		echo Installing in $$i ; \
		(cd $$i; ${MAKE} install) ; \
	done

install_shared:
	@for i in ${subdirs}; do \
		echo Installing in $$i ; \
		(cd $$i; ${MAKE} install_shared) ; \
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
	rm -f *~ *.bak core

distclean:
	@for i in ${subdirs}; do \
		echo Distribution Cleaning in $$i ; \
		(cd $$i; ${MAKE} distclean) ; \
	done
	rm -f *~ *.bak core config.cache config.status config.log 
	find ${subdirs} -name Makefile -exec rm {} \; -print
	rm ./Makefile
