# Generated automatically from Makefile.in by configure.

SHELL=/bin/sh

prefix=/usr/local
indexdir=/usr/local/lib/X11/fvwm95
htmldir=/usr/local/lib/X11/fvwm95/html

INSTALL = /usr/bin/install -c
INSTALL_DATA = ${INSTALL} -m 644
RM=rm -f




all:

install:
	@echo Installing documentation in ${htmldir}  
	@mkdir -p ${htmldir}
	@${INSTALL_DATA} index.html ${indexdir}
	@${INSTALL_DATA} GPL.html ${indexdir}
	@${INSTALL_DATA} html/*.html ${htmldir}  


uninstall:
	@echo Uninstalling documentation
	@${RM} ${indexdir}/index.html
	@${RM} ${indexdir}/GPL.html
	@${RM} ${htmldir}/*.html  


clean:

distclean:

