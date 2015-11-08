# Generated automatically from Makefile.in by configure.
#
SHELL=/bin/sh



top_srcdir=..
srcdir=.
prefix=/usr/local
exec_prefix=${prefix}
bin_dir=${prefix}/bin
fvwm_name=fvwm95
fvwm_dir=${prefix}/lib/X11/${fvwm_name}

INSTALL=/usr/bin/install -c
INSTALL_PROGRAM=${INSTALL}
RM=rm -f

SCRIPTS=fvwmrc_convert quantize_pixmaps

all: 

depend:

install: 
	@for i in ${SCRIPTS} ; do \
		${INSTALL_PROGRAM} ./$$i ${bin_dir} ; \
	done

uninstall: 
	@for i in ${SCRIPTS} ; do \
		${RM}  ${bin_dir}/$$i ; \
	done

clean:
	${RM} *~ core *.bak

distclean: clean


#
