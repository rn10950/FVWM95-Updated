
SHELL=/bin/sh

prefix=/usr/local
exec_prefix=${prefix}
config_dir=${exec_prefix}/bin
INSTALL=/usr/bin/install -c
INSTALL_DATA=${INSTALL} -m 644

all:

shared:

install_shared: install

install:
	${INSTALL} xc-config ${config_dir}

clean:
	rm -f core *.bak *~

distclean:
	rm -f core *.bak *~ xc-config
