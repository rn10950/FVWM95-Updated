
SHELL=/bin/sh

prefix=/usr/local
lib_dir=/usr/local/lib
doc_dir=/usr/local/xclass/docs
etc_dir=/usr/local/xclass/etc

INSTALL = /usr/bin/install -c
INSTALL_DATA = ${INSTALL} -m 644


XCLASSRC = xclassrc
MIMETYPES = mime.types
DOCS = GPL \
	LGPL \
	LICENSE \
	tcl.license.terms \
	tk.license.terms \
	ChangeLog \
	INSTALL \
	Programming.notes \
	Layout.notes \
	MimeTypes.README \
	Drag-and-Drop \
	OpenGL \
	MDI.notes

all:

shared:

install_shared: install

install:
	mkdir -p ${etc_dir}
	${INSTALL_DATA} ${XCLASSRC} ${etc_dir}
	${INSTALL_DATA} ${MIMETYPES} ${etc_dir}
	mkdir -p ${doc_dir}
	for i in ${DOCS} ; do \
		${INSTALL_DATA} ./$$i ${doc_dir} ; \
	done

clean:
	rm -f core *~ *.bak

distclean: clean
